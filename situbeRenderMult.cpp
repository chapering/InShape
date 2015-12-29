// ----------------------------------------------------------------------------
// situbeRenderMult.h: An extension to CSitubeRender, serving precise
//						interaction synchronization in CMitubeRenderView and 
//						CMitubeRenderWindow. For example: 
//
//						support class friendship with these two classes and; 
//						allow deliberate control of the inner logic control;
//						more to go 
//
// Creation : June. 14th 2011
//
// Revision:
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "situbeRenderMult.h"

using std::ostream_iterator;
using std::copy;
using std::ofstream;
using std::ifstream;
using std::ends;

//////////////////////////////////////////////////////////////////////////////
//
//	implementation of the Class CSituberRenderMult
//
CSitubeRenderMult::CSitubeRenderMult(int argc, char **argv) : 
	CSitubeRender(argc, argv)
{
}

CSitubeRenderMult::~CSitubeRenderMult()
{
}

int CSitubeRenderMult::loadGeometry() 
{
	if ( m_nRunNum == 0) {
		if ( 0 != m_loader.load(m_strfnsrc) ) {
			m_cout << "Loading geometry failed - GLApp aborted abnormally.\n";
			return -1;
		}
		m_nRunNum ++;
		return -1;
	}

	if ( -1 == projectSkeleton(-1,-1) ) {
		return -1;
	}

	findMaxMinSegLen();

	m_fbdRadius = m_loader.getBoundRadius();

	// generate streamtube, each for a streamline
	unsigned long szTotal = m_loader.getSize();
	m_alltubevertices.resize( szTotal );
	m_alltubenormals.resize ( szTotal );
	m_alltubecolors.resize ( szTotal );
	m_alltubefaceIdxs.resize( szTotal );
	m_encodedcolors.resize ( szTotal );

	m_ctxtubevertices.resize( szTotal );
	m_ctxtubecolors.resize ( szTotal );

	m_cout << "Generating streamtube meshes .... ";
	for (unsigned long idx = 0; idx < szTotal; ++idx) {
		buildTubefromLine( idx );
	}

	if ( "" != m_strfnFiberIdx ) {
		if ( 0 != _loadFiberIdx() ) {
			cerr << "Failed to load task key : the fiber indices of "
				"the bundle to be marked, see -h for usage.\n";
			return -1;
		}

		for (unsigned long idx = 0; idx < szTotal; ++idx) {
			buildTubefromLine( idx );
		}
	}
	m_cout << " finished.\n";

	m_cout << "X : " << m_minCoord[0] << " - " << m_maxCoord[0] << "\n";
	m_cout << "Y : " << m_minCoord[1] << " - " << m_maxCoord[1] << "\n";
	m_cout << "Z : " << m_minCoord[2] << " - " << m_maxCoord[2] << "\n";

	// for primary geometry
	m_edgeflags.resize( szTotal );
	m_edgeflags.assign( szTotal, GL_TRUE );

	// for streamtube context
	m_ctxedgeflags.resize( szTotal );
	m_ctxedgeflags.assign(  szTotal, GL_TRUE );
	m_brushedoutflags.resize( szTotal );
	m_brushedoutflags.assign( szTotal, GL_TRUE );

	return 0;
}

int CSitubeRenderMult::mainstay() 
{
	// geometry must be loaded successfully in the first place in order the
	// openGL pipeline could be launched.
	if ( m_nRunNum < 2 ) {
		if ( 0 != loadGeometry() ) {
			return -1;
		}

		m_nRunNum ++;
		return -1;
	}

	// serialize the streamtube geometry if asked for
	if ( m_strfnobj.length() >= 1 ) {
		serializeTubes( m_strfnobj );
	}	

	setMinMax(m_minCoord[0], m_minCoord[1], m_minCoord[2],
			m_maxCoord[0], m_maxCoord[1], m_maxCoord[2]);

	//addButton("c", "bicons/6.bmp","n"); // show context or no context
	addButton("NoCx", "", "Ctx"); // show context or no context
#ifdef DWI_EMBEDDING
	addButton("d", "bicons/7.bmp","x"); 
	addButton(">"); 
	addButton("<"); 
#endif
	//addButton("b", "bicons/6.bmp","r"); // brushing or non-brushing
	addButton("Excl", "","Brsh"); // brushing or non-brushing

	m_nselbox = m_strfnTumorBoxes.size();
	// add initial number of selection boxes
	for (int i=0; i<m_nselbox; ++i) {
		addBox();
	}

	if ( 0 != m_nselbox && 0 != _loadTumorBoxpos() ) {
		cerr << "FATAL: failed to load tumor bounding box information.\n";
		return -1;
	}

	// add a color map sphere
	m_pcmGadget->setVertexCoordRange(
			( m_minCoord[0] + m_maxCoord[0] )/2.5,
			( m_minCoord[1] + m_maxCoord[1] )/2.5,
			( m_minCoord[2] + m_maxCoord[2] )/2.5);
	addGadget( m_pcmGadget );
	((CSphereColorMap*)m_pcmGadget)->setColorScheme( m_colorschemeIdx );
	//m_pcmGadget->switchTranslucent();

	// add an anatomical coordinate system gadget
	m_paxesGagdet->setVertexCoordRange(
			( m_minCoord[0] + m_maxCoord[0] )/2,
			( m_minCoord[1] + m_maxCoord[1] )/2,
			( m_minCoord[2] + m_maxCoord[2] )/2);
	addGadget( m_paxesGagdet );
	((CAnatomyAxis*)m_paxesGagdet)->setColorScheme( m_colorschemeIdx );

	// turn back on gadget feature 
	m_bGadgetEnabled = true;
	
	// associate with a task list if requested
	if ( "" != m_strfntask ) { // yes, requested
		// loading a task list is also optional
		if ( 0 < m_taskbox.loadFromFile( m_strfntask.c_str() ) ) {
			m_taskbox.setVertexCoordRange(
				( m_minCoord[0] + m_maxCoord[0] )/2,
				( m_minCoord[1] + m_maxCoord[1] )/2,
				( m_minCoord[2] + m_maxCoord[2] )/2);
			m_taskbox.setColor(1.0, 1.0, 1.0);
			m_taskbox.turncover(true);
			m_bIboxEnabled = false;
			m_bGadgetEnabled = false;
		}
	}

#ifdef DWI_EMBEDDING
	// get DWI embedding prepared if requested
	if ( "" != m_strdwidir ) {// yes, requested 
		if ( 0 != m_dcmexplorer.setDir( m_strdwidir.c_str() ) ) {
			cerr << "Failed in DWI image loading, aborted now.\n";
			return -1;
		}

		m_dcmexplorer.setVertexCoordRange(
				( m_minCoord[0] + m_maxCoord[0] )/2,
				( m_minCoord[1] + m_maxCoord[1] )/2,
				( m_minCoord[2] + m_maxCoord[2] )/2);
		if  (! m_dcmexplorer.start() ) {
			return -1;
		}
	}
#endif

	setPrjInfo(25.0f, 
			ABS(m_minCoord[2])/4.0,
			(ABS(m_maxCoord[2]) + ABS(m_minCoord[2]))*8);

	setViewInfo(0.0, 0.0,
			( m_minCoord[2] + m_maxCoord[2] )*3,
			0.0, 0.0, 0.0,
			0, 1, 0);

	//setClearColor( 0.5, 0.5, 0.5 );
	setClearColor( 128/255.0, 128/255.0, 128/255.0 );

	m_psitInstance = this;
	// if multiple process support is enable
	if ( m_nSiblings >= 1 ) {
		_initSharedInfo();
		// catching signal SIGUSR1 to receive parent's post on the update of the
		// number of alive sibling processes
		signal(SIGUSR1, _on_sibling_exit);
	}

	signal (SIGTERM, _on_killed);
	signal (SIGINT, _on_killed);

	if ( m_bSuspended ) {
		m_cout << "Event loop entrance suspened.\n";
		return 0;
	}

	return	CGLIBoxApp::mainstay();
}

void CSitubeRenderMult::_realkeyResponse(unsigned char key, int x, int y)
{
	switch (key) {
		case 32: // Space bar to trigger next task if any
			if ( "" != m_strfntask ) {
				if ( m_taskbox.iscovered() && !m_taskbox.isPrompt() ) {
					m_bIboxEnabled = true;
					m_bGadgetEnabled = true;
					m_taskbox.turncover(false);
					m_cout << "Get into another task.\n";
				}
				else {
					m_taskbox.turncover(true);
					if ( m_taskbox.next() ) {
						m_bIboxEnabled = false;
						m_bGadgetEnabled = false;
					}
					else { // all task listed out already
						m_bTaskDone = true;
						m_bIboxEnabled = true;
						m_bGadgetEnabled = true;
						m_taskbox.turncover(false);
					}
				}
				break;
			}
			CGLIBoxApp::keyResponse(key, x, y);
			return;
		default:
			CSitubeRender::keyResponse(key, x, y);
			return;
	}
	glutPostRedisplay();
}

void CSitubeRenderMult::_calibrateLineGeometry(const point_t & minpt, const point_t & maxpt)
{
	unsigned long szTotal = m_loader.getSize();
	for (unsigned long idx = 0; idx < szTotal; ++idx) {
		vector<GLfloat> & line = m_loader.getElement( idx );
		unsigned long szPts = static_cast<unsigned long>( line.size()/6 );

		for (unsigned long idx1 = 0; idx1 < szPts; idx1++) {
			line [ idx1*6 + 3 ] *= maxpt.x / m_loader.getMaxCoord(0);
			line [ idx1*6 + 4 ] *= maxpt.y / m_loader.getMaxCoord(1);
			line [ idx1*6 + 5 ] *= maxpt.z / m_loader.getMaxCoord(2);
		}
	}

	for (int j = 0;  j < 3; j++) {
		m_loader.getMinCoord(j) = minpt[j];
		m_loader.getMaxCoord(j) = maxpt[j];
	}
}

void CSitubeRenderMult::_calibrateTubeGeometry(const point_t & minpt, const point_t & maxpt)
{
	for (int j = 0;  j < 3; j++) {
		m_minCoord[j] = minpt[j];
		m_maxCoord[j] = maxpt[j];
	}
}

/* set ts=4 sts=4 tw=80 sw=4 */

