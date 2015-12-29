// ----------------------------------------------------------------------------
// GLiboxmoth.cpp: an extension to CGLApp that supports multiple box selection
//			 on the basis of the simplest collision detection 
//
// Creation : Feb. 12th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "GLiboxmoth.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// class implementation of CGLIBoxApp
//
CGLIBoxApp::CGLIBoxApp() : CGLApp(),
	m_minx(0xffffffff),	
	m_miny(0xffffffff),	
	m_minz(0xffffffff),	
	m_maxx(-0xffffffff),	
	m_maxy(-0xffffffff),	
	m_maxz(-0xffffffff),
	m_curselboxidx(-1),
	m_curhlboxidx(-1),
	m_pvertices(NULL),
	m_bIboxEnabled(true),
	m_bIboxCulling(true),
	m_bMouseOnlyForBox(false)
{
}

CGLIBoxApp::CGLIBoxApp(int argc, char **argv) : CGLApp(argc, argv),
	m_minx(0xffffffff),	
	m_miny(0xffffffff),	
	m_minz(0xffffffff),	
	m_maxx(-0xffffffff),	
	m_maxy(-0xffffffff),	
	m_maxz(-0xffffffff),
	m_curselboxidx(-1),
	m_curhlboxidx(-1),
	m_pvertices(NULL),
	m_bIboxEnabled(true),
	m_bIboxCulling(true),
	m_bMouseOnlyForBox(false)
{
	// in selection box application, gadgets are often needed since
	// application in this catalog mostly involves 3D interaction
	m_bGadgetEnabled = true;
	m_cout.switchtime(true);
}

CGLIBoxApp::~CGLIBoxApp()
{
}

int CGLIBoxApp::associateObj(vector<GLdouble>* pvertices, bool bcalminmax,
						GLdouble dx, GLdouble dy, GLdouble dz)
{
	if ( !pvertices ) {
		return -1;
	}

	size_t szVertex = pvertices->size()/3;

	if ( szVertex <= 0 ) { // guard against the trivial request
		return 0;
	}

	m_pvertices = pvertices;

	// establish edge flags for vertices so that to ease the visibility
	// controls and initialize it as such that all vertices are visible
	m_edgeflags.resize ( szVertex );
	m_edgeflags.assign ( szVertex, GL_TRUE );

	m_dx = dx, m_dy = dy, m_dz = dz;
	updateGadgets();

	// if the clientele did not call setMinMax before, it might want calculate
	// the bounding corners here
	if ( !bcalminmax ) {
		return 0;
	}

	// C++ does not support array of references, which sounds reasonable
	// however. so we only to employ array of pointers again
	GLdouble *mins[3] = { &m_minx, &m_miny, &m_minz },
			*maxs[3] = { &m_maxx, &m_maxy, &m_maxz };
	for (size_t idx = 0; idx < szVertex; ++idx) {
		for (size_t j = 0; j < 3; ++j) {
			if ( (*pvertices)[ idx*3 + j ] < *mins[ j ] ) {
				*mins [ j ] = (*pvertices)[ idx*3 + j ];
			}
			if ( (*pvertices)[ idx*3 + j ] > *maxs[ j ] ) {
				*maxs [ j ] = (*pvertices)[ idx*3 + j ];
			}
		}
	}

	return 0;
}

void CGLIBoxApp::setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
				GLdouble maxx, GLdouble maxy, GLdouble maxz)
{
	m_minx = minx, m_miny = miny, m_minz = minz;
	m_maxx = maxx, m_maxy = maxy, m_maxz = maxz;
	setObjOffset( -(m_maxx + m_minx)/2, 
				  -(m_maxy + m_miny)/2, 
				  -(m_maxz + m_minz)/2);

	// in selection box application, gadgets are often needed since
	// application in this catalog mostly involves 3D interaction
	if (m_bGadgetEnabled) {
		addGadget( new CGLAxis() );
	}
	updateGadgets();
}

void CGLIBoxApp::updateGadgets()
{
	for (size_t i=0; i<m_gadgets.size();i++) {
		m_gadgets[i]->setVertexCoordRange(
				fabs(m_dx), fabs(m_dy), fabs(m_dz));
	}
}

void CGLIBoxApp::drawBoxes()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glDisable(GL_LIGHTING);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glFrontFace(GL_CCW);
	//glEnable(GL_CULL_FACE);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	size_t szBoxes = m_boxes.size();
	for (size_t idx = 0; idx < szBoxes; idx++) {
		m_boxes[idx].drawFrame();
	}

	glPopMatrix();
	glPopAttrib();
}

int CGLIBoxApp::addBox(const CIselbox& selbox)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	// from the programmability standpoint, any number of selection box is
	// acceptable but in actuality, too much, over 10 for instance, should be
	// mischievous and declined therefore
	if ( m_boxes.size() >= 10 ) {
		m_cout << "Too many selection boxes while Upper limit is 10.\n";
		return -1;
	}
	m_boxes.push_back( selbox );
	// the responsiblity of setting the layout for the new selection box is
	// attributed to the user who creates the instance selbox
	return 0;
}

int CGLIBoxApp::addBox()
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	CIselbox selbox;
	if ( m_cout.isswitchon() ) {
		m_cout << "debug output switched on\n";
		selbox.m_cout.switchoff(false);
		selbox.m_cout.switchtime(true);
	}
	else {
		selbox.m_cout.switchoff(true);
	}	
	selbox.associateObj(m_pvertices, &m_edgeflags, m_dx, m_dy, m_dz);
	addBox( selbox );
	return _updateDefaultLayout();
}

int CGLIBoxApp::removeBox(int idx)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	// trivial check against invalid index
	if (idx < 0 || idx >= static_cast<int>(m_boxes.size()) ) {
		if ( -1 == idx ) {
			if ( -1 != m_curhlboxidx ) {
				idx = m_curhlboxidx;
			}
			else if ( -1 != m_curselboxidx ) {
				idx = m_curselboxidx;
			}
			else if (m_boxes.size() >= 1) {
				idx = int(m_boxes.size()) - 1;
			}
			else {
				return -1;
			}
		}
		else {
			return -1;
		}
	}

	m_boxes.erase( m_boxes.begin() + idx );
	// now since the box store has been changed, index to it will also need
	// changing - elements after idx have been moved forwards by just one
	// slot, and "This invalidates all iterator and references to elements after
	// position or first."
	if ( m_curselboxidx >= idx ) {
		m_curselboxidx --;
	}
	else if ( m_curselboxidx == idx ) {
		m_curselboxidx = -1;
	}

	if ( m_curhlboxidx >= idx ) {
		m_curhlboxidx --;
	}
	else if ( m_curhlboxidx == idx ) {
		m_curhlboxidx = -1;
	}

	return 0;
}

int CGLIBoxApp::duplicateBox(GLdouble dx, GLdouble dy, GLdouble dz)
{
	if ( !m_bIboxEnabled ) {
		return -1;
	}
	if ( m_boxes.size() < 1 ) {
		return addBox();
	}

	int idx = -1;
	if ( -1 != m_curhlboxidx ) {
		idx = m_curhlboxidx;
	}
	else if ( -1 != m_curselboxidx ) {
		idx = m_curselboxidx;
	}
	else if (m_boxes.size() >= 1) {
		idx = int(m_boxes.size()) - 1;
	}

	if (idx >= 0 && idx < static_cast<int>(m_boxes.size()) ) {
		return addBox( m_boxes[idx].clone(dx, dy, dz) );
	}

	return idx;
}

bool CGLIBoxApp::isPointInBox(GLdouble x, GLdouble y, GLdouble z)
{
	if (!m_bIboxCulling) {
		return true;
	}

	// judge if (x,y,z) in any box
	//
	size_t szBoxes = m_boxes.size();
	for (size_t idx = 0; idx < szBoxes; idx++) {
		if (m_boxes[idx].isInside(x, y, z)) {
			return true;
		}
	}
	return false;
}

void CGLIBoxApp::glInit(void)
{
	CGLApp::glInit();
}

void CGLIBoxApp::keyResponse(unsigned char key, int x, int y)
{
	switch (key) {
		case 'b':
			if ( !m_bIboxEnabled ) {
				return;
			}
			_genBoxColors();
			glutPostRedisplay();
			return;
		default:
			break;
	}
	CGLApp::keyResponse(key, x, y);
}

void CGLIBoxApp::specialResponse(int key, int x, int y)
{
	switch (key) {
		case GLUT_KEY_F10:
			m_bIboxEnabled = !m_bIboxEnabled;
			m_cout << "selection box " << 
				(m_bIboxEnabled?"Enabled":"Disabled") << "\n";
			glutPostRedisplay();
			return;
		case GLUT_KEY_F9:
			if ( !m_bIboxEnabled ) {
				return;
			}
			m_bIboxCulling = !m_bIboxCulling;
			m_cout << "box culling " << 
				(m_bIboxCulling?"on":"off") << "\n";
			glutPostRedisplay();
			return;
		case GLUT_KEY_F8:
			if ( !m_bIboxEnabled ) {
				return;
			}
			for (size_t idx = 0; idx < m_boxes.size(); idx++) {
				m_boxes[idx].switchLimit();
			}
			return;
		case GLUT_KEY_F7:
			if ( !m_bIboxEnabled ) {
				return;
			}
			for (size_t idx = 0; idx < m_boxes.size(); idx++) {
				m_boxes[idx].switchHint();
			}
			glutPostRedisplay();
			return;
		case GLUT_KEY_F3: // duplicate current selection box
			if ( !m_bIboxEnabled ) {
				return;
			}
			if (0 == duplicateBox(10,10,10)) {
				m_cout << "A box duplicated and added.\n";
			}
			glutPostRedisplay();
			return;
		case GLUT_KEY_F4: // remove current selection box
			if ( !m_bIboxEnabled ) {
				return;
			}
			if (0 == removeBox() ) {
				m_cout << "Box removed.\n";
			}
			glutPostRedisplay();
			return;
		default:
			break;
	}
	CGLApp::specialResponse(key, x, y);
}

void CGLIBoxApp::mouseResponse(int button, int state, int x, int y)
{
	if ( !m_bIboxEnabled ) {
		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mouseResponse(button, state, x, y);
		}
		return;
	}

	switch (state) {
	case GLUT_DOWN: 
		{
			m_curselboxidx = -1;
			// the first selection box that accepts the mouse event is regarded
			// as the currently focused
			size_t szBoxes = m_boxes.size();
			for (size_t idx = 0; idx < szBoxes; idx++) {
				if ( 0 == m_boxes[idx].onMouseClicked(button, state, x, y) ) {
					m_curselboxidx = idx;
					return;
				}
			}
		}
		break;
	case GLUT_UP:
		{
			if ( -1 != m_curselboxidx ) {
				m_boxes[ m_curselboxidx ].onMouseClicked( 
						button, state, 0,0);
				return;
			}
		}
		break;
	default:
		break;
	}

	if ( !m_bMouseOnlyForBox ) {
		CGLApp::mouseResponse(button, state, x, y);
	}
	return;
}

void CGLIBoxApp::mouseMotionResponse(int x, int y)
{
	if ( !m_bIboxEnabled || -1 == m_curselboxidx ) {
		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mouseMotionResponse(x, y);
		}
		return;
	} 

	// forward the mouse input to the selected box
	if ( 0 == m_boxes[ m_curselboxidx ].onMouseMove( x, y ) ) {
		glutPostRedisplay();
		return;
	}

	if ( !m_bMouseOnlyForBox ) {
		CGLApp::mouseMotionResponse(x, y);
	}
}

void CGLIBoxApp::mousePassiveMotionResponse(int x, int y)
{
	if ( !m_bIboxEnabled ) {
		if ( !m_bMouseOnlyForBox ) {
			CGLApp::mousePassiveMotionResponse(x, y);
		}
		return;
	}

	int orghlidx = m_curselboxidx;
	m_curhlboxidx = -1;
	// the first selection box that accepts the mouse event is regarded
	// as the currently focused
	size_t szBoxes = m_boxes.size();
	for (size_t idx = 0; idx < szBoxes; idx++) {
		if ( 0 == m_boxes[idx].onMousePassiveMove(x, y) ) {
			m_curhlboxidx = idx;
		}
	}

	if ( orghlidx != m_curhlboxidx ) {
		glutPostRedisplay();
	}
}

void CGLIBoxApp::onReshape( GLsizei w, GLsizei h )
{
	// boxes should also be reset 
	if (m_bFirstReshape) {
		if ( m_bIboxEnabled ) {
			_updateDefaultLayout();
		}
	}
	CGLApp::onReshape(w, h);
}

void CGLIBoxApp::display ( void )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	draw();
	if ( m_bIboxEnabled ) {
		drawBoxes();
	}

	if ( m_bGadgetEnabled ) {
		drawGadgets();
	}
	trigger();
}

int CGLIBoxApp::_updateDefaultLayout()
{
	GLdouble w = ( m_maxx - m_minx );
	//int h = ( m_maxy - m_miny );
	int szBox = static_cast<int> ( m_boxes.size() );
	GLdouble dx = w/szBox;
    //GLfloat	dy = (h-8*szBox)/szBox;

	if ( 0 >= szBox ) {
		return -1;
	}

	GLdouble tx = - (m_minx + m_maxx)/2,
			 ty = -(m_miny + m_maxy)/2,
			 tz = -(m_minz + m_maxz)/2;

	for (int idx = 0; idx < szBox; ++ idx) {
		m_boxes[idx].setMinMax( 
		 m_minx + idx*dx + tx, m_miny + ty, m_minz + tz,
		 m_minx + (idx+1)*dx + tx, m_maxy + ty, m_maxz + tz);
		//m_boxes[idx].setFrameColor(0.1*((idx+1)%10), 0.2*((1+idx)%5), 1.0/((idx+1)%10));
		m_boxes[idx].setFrameColor(0.1, 0.2, 1.0);
	}

	return 0;
}

void CGLIBoxApp::_genBoxColors()
{
	glrand::genFloatvector<GLfloat> (3, m_boxcolors, 
			ARRAY_SIZE(m_boxcolors), 0, 1);
	if ( m_cout.isswitchon() ) {
		cout << "box colors: ";
		copy (m_boxcolors, m_boxcolors + ARRAY_SIZE(m_boxcolors), 
				ostream_iterator<GLfloat> (cout, " "));
		cout << "\n";
	}
	int szBox = static_cast<int> ( m_boxes.size() );
	for (int idx = 0; idx < szBox; ++ idx) {
		m_boxes[idx].setFrameColor(m_boxcolors[0], m_boxcolors[1], m_boxcolors[2]);
	}
}

/* set ts=4 sts=4 tw=80 sw=4 */

