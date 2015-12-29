// ----------------------------------------------------------------------------
// iselbox.cpp : an independent class encapsulating functions needed for a
//				selection box in the interactive graphics application.
//
//
// Creation : Feb. 11th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "iselbox.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// class implementation of CIselbox
//
CIselbox::CIselbox() :
	m_minx(0xffffffff),	
	m_miny(0xffffffff),	
	m_minz(0xffffffff),	
	m_maxx(-0xffffffff),	
	m_maxy(-0xffffffff),	
	m_maxz(-0xffffffff),
	m_fcr(1.0f), m_fcg(1.0f), m_fcb(1.0f),	
	m_pressedbtn(-1),
	m_selectedFid(-1),
	m_highlightedFid(-1),
	m_selectedVid(-1),
	m_highlightedVid(-1),
	m_selectedEid(-1,-1),
	m_highlightedEid(-1,-1),
	m_nearestZ(0),
	m_mx(0), m_my(0), m_mz(0),
	m_pvertices(NULL),
	m_pedgeflags(NULL),
	m_pbuttons(NULL),
	m_dx(0),m_dy(0),m_dz(0),
	m_bmvlimit(true),
	m_bhint(true),
	m_bMoveAlongZ(false),
	m_bEPscaling(false),
	m_bRemovalbased(false)
{
}

CIselbox::CIselbox(const CIselbox& other)
{
	/* it is never reasonable to coarsely copy following member data 
	 * ,which accounts for the initialization as follows just like it was
	 * treated in the default constructor
	 */ 
	m_pressedbtn = -1;
	m_selectedFid = -1;
	m_highlightedFid = -1;
	m_selectedVid = -1;
	m_highlightedVid = -1;
	m_selectedEid.update(-1,-1);
	m_highlightedEid.update(-1,-1);
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;
	m_bmvlimit = true;
	m_bhint = true;
	m_bMoveAlongZ = false;
	m_bEPscaling = false;
	// a box will inherit the selection mode from its producer
	m_bRemovalbased = other.m_bRemovalbased;
	m_pvertices = NULL;
	m_pedgeflags = NULL;
	m_pbuttons = other.m_pbuttons;
	/* particularly for this specific class, following duplication should be
	 * rational to make
	 */
	m_fcr = other.m_fcr;
	m_fcg = other.m_fcg;
	m_fcb = other.m_fcb;
	m_dx = other.m_dx;
	m_dy = other.m_dy;
	m_dz = other.m_dz;
	m_cout = other.m_cout;
	/* it is a crux to reconstruct faces and box vertices after being assigned a
	 * set of new corners
	 */
	setMinMax( other.m_minx, other.m_miny, other.m_minz, 
		other.m_maxx, other.m_maxy, other.m_maxz );
}

CIselbox::~CIselbox()
{
}

CIselbox& CIselbox::operator = (const CIselbox& other)
{
	this->m_minx = other.m_minx;
	this->m_miny = other.m_miny;
	this->m_minz = other.m_minz;
	this->m_maxx = other.m_maxx;
	this->m_maxy = other.m_maxy;
	this->m_maxz = other.m_maxz;

	// for CIselbox, m_boxvertices, m_winboxvertices and m_faces are all
	// reconstructed instantly from the coordinates fo the two opposite
	// corners, it is meaningless or even pernicious to simply copy from other
	// instance
	/*
	this->m_boxvertices.resize( other.m_boxvertices.size() );
	std::copy(other.m_boxvertices.begin(), other.m_boxvertices.end(),
			this->m_boxvertices.begin());

	this->m_winboxvertices.resize( other.m_winboxvertices.size() );
	std::copy(other.m_winboxvertices.begin(), other.m_winboxvertices.end(),
			this->m_winboxvertices.begin());

	this->m_faces.resize( other.m_faces.size() );
	std::copy(other.m_faces.begin(), other.m_faces.end(),
			this->m_faces.begin());
	*/
	this->m_fcr = other.m_fcr;
	this->m_fcg = other.m_fcg;
	this->m_fcb = other.m_fcb;

	this->m_pvertices = other.m_pvertices;
	this->m_pedgeflags = other.m_pedgeflags;
	this->m_pbuttons = other.m_pbuttons;
	this->m_dx = other.m_dx;
	this->m_dy = other.m_dy;
	this->m_dz = other.m_dz;

	this->m_cout = other.m_cout;
	this->m_bmvlimit = other.m_bmvlimit;
	this->m_bhint = other.m_bhint;
	this->m_bMoveAlongZ = other.m_bMoveAlongZ;
	this->m_bEPscaling = other.m_bEPscaling;
	this->m_bRemovalbased = other.m_bRemovalbased;

	m_pressedbtn = -1;
	m_selectedFid = -1;
	m_highlightedFid = -1;
	m_selectedVid = -1;
	m_highlightedVid = -1;
	m_selectedEid.update(-1,-1);
	m_highlightedEid.update(-1,-1);
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;

	/* since already armed with new corners, faces and box vertices all need
	 * updating
	 */
	m_boxvertices.clear();
	m_winboxvertices.clear();
	m_faces.clear();
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_onMinMaxUpdated();
	_initFaces();

	return *this;
}

/* ---------------------------------------
 * original order of vertices on the box
 *
		|Y
		|
	   4|_____1
	   /|	 /|
	 5/_|__8/ |
	  |	|___|_|______X	
	  |	/3	| /2	
	 6|/____|/
	  /     7
	 /
	Z

 * CCW for front faces adopted, so none 
 * of the faces will have inwards normal
 *
----------------------------------------*/

void CIselbox::setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
				GLdouble maxx, GLdouble maxy, GLdouble maxz)
{
	m_minx = minx, m_miny = miny, m_minz = minz;
	m_maxx = maxx, m_maxy = maxy, m_maxz = maxz;

	refreshGeometry();
}

void CIselbox::refreshGeometry()
{
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_onMinMaxUpdated();
	_initFaces();
}

void CIselbox::setFrameColor(GLfloat r, GLfloat g, GLfloat b)
{
	m_fcr = r, m_fcg = g, m_fcb = b;
}

void CIselbox::drawFrame(bool bBtnEnabled)
{
	glLineWidth(1.5);
	glColor3f(m_fcr, m_fcg, m_fcb);
	if ( ! m_bhint ) {
		for (size_t i = 0; i < m_faces.size(); ++i) {
			m_faces[i].drawFrame();
		}
		return;
	}

	_objcoord2wincoord();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0x0001);
	glColor4f(m_fcr, m_fcg, m_fcb, 0.8);

	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) 
		{
			m_faces[i].drawFrame();
		}
	}
	glPopAttrib();

	// draw in-situ buttons if enabled & ready
	if (bBtnEnabled && m_pbuttons && -1 != m_highlightedFid) {
		// initially all buttons are forced to disappear
		size_t szBtn = m_pbuttons->size();
		for (size_t idx = 0; idx < szBtn; idx++) {
			(*m_pbuttons)[idx].isShown = false;
		}

		if (0 == _calcBtnLayout()) {
			for (size_t i = 0; i < szBtn; i++) {
				// the selection mode is now a box-affiliated property
				if ("a" == (*m_pbuttons)[i].text ||
					"e" == (*m_pbuttons)[i].text ) {
					(*m_pbuttons)[i].text = (m_bRemovalbased?"a":"e");
				}
				(*m_pbuttons)[i].draw();
			}
		}
	}

	glColor4f(m_fcr, m_fcg, m_fcb, 1.0);
	//glLineWidth(1);
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( m_faces[i].isFrontFacing() ) {
			m_faces[i].drawFrame();
		}
	}

	if ( m_highlightedVid != -1 ) {
		// draw a sphere at the highlighted vertex as a hint/feedback
		//
		GLdouble mvmat[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor3f(1.0, 1.0, 0.0);
		glPushMatrix();
		glTranslatef(m_boxvertices[m_highlightedVid].x, 
			m_boxvertices[m_highlightedVid].y, 
			m_boxvertices[m_highlightedVid].z);
		glutSolidSphere(0.5 * fabs(mvmat[14]/(m_dz*3)) , 50, 50);
		glPopMatrix();
		glPopAttrib();
		return;
	}

	if ( m_highlightedEid != edge_idx_t(-1,-1) ) {
		// further increase thickness as the highlighted edge as a hint/feedback
		//
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glColor3f(1.0, 1.0, 0.0);
		glLineWidth(5.0);
		glPushMatrix();
		glBegin(GL_LINES);
			glVertex3f( 
				m_boxvertices[m_highlightedEid[0]].x,
				m_boxvertices[m_highlightedEid[0]].y,
				m_boxvertices[m_highlightedEid[0]].z);
			glVertex3f( 
				m_boxvertices[m_highlightedEid[1]].x,
				m_boxvertices[m_highlightedEid[1]].y,
				m_boxvertices[m_highlightedEid[1]].z);
		glEnd();
		glPopMatrix();
		glPopAttrib();
		return;
	}

	if ( m_highlightedFid != -1 && m_bEPscaling ) {
		// draw a sphere at the center of the highlighted face as a hint/feedback
		// to tell that congruent scaling is ready to trigger
		point_t fc = m_faces[m_highlightedFid].getCenter();
		/*
		GLdouble mvmat[16];
		glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		GLint maxLW[2];
		glGetIntegerv( GL_ALIASED_LINE_WIDTH_RANGE, maxLW);
		glLineWidth(maxLW[1]);
		glPushMatrix();
		glColor3f(1.0, 1.0, 0.0);
		glTranslated(fc.x, fc.y, fc.z);
		//glutSolidSphere(1 * fabs(mvmat[14]/(m_dz*3)) , 50, 50);
		glutWireCube(1 * fabs(mvmat[14]/(m_dz*3)));
		glPopMatrix();
		glPopAttrib();
		*/

		printText("Congruent Scaling", fc.x, fc.y, fc.z, 
				1.0, 0.0, 0.0, 1.0, GLUT_BITMAP_HELVETICA_18);

		/*
		GLdouble mvmat[16], prjmat[16];
		GLint	viewmat[4];

		glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
		glGetDoublev(GL_PROJECTION_MATRIX, prjmat);
		glGetIntegerv(GL_VIEWPORT, viewmat);
		//fc.y = (GLdouble)viewmat[3] - (GLdouble)fc.y;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0, (GLdouble)viewmat[2], (GLdouble)viewmat[3], 0.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor3f(1.0, 0.0, 0.0);
		glRasterPos2d( fc.x, fc.y );
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, 'X');

		glMatrixMode(GL_PROJECTION);
		glLoadMatrixd(prjmat);
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixd(mvmat);
		*/
	}
}

int CIselbox::associateObj(vector<GLdouble>* pvertices,
						vector<GLboolean>* pedgeflags,
						vector<button_t>* pbuttons,
						GLdouble dx, GLdouble dy, GLdouble dz)
{
	m_dx = dx, m_dy = dy, m_dz = dz;
	m_pbuttons	= pbuttons;

	if ( !pvertices || !pedgeflags ||
			pvertices->size()/3 != pedgeflags->size() ) {
		return -1;
	}

	m_pvertices = pvertices;
	m_pedgeflags = pedgeflags;

	return 0;
}

int CIselbox::refreshEdgeFlags()
{
	// check against the mandatory precondition
	if ( !m_pvertices || !m_pedgeflags ||
			m_pvertices->size()/3 != m_pedgeflags->size() ) {
		return -1;
	}

	// examine every vertex to see if it is inside
	size_t szVertex = m_pvertices->size()/3;
	int ret = 0;
	
	for (size_t idx = 0; idx < szVertex; idx++) {
		(*m_pedgeflags)[idx] = 
			( isInside( (*m_pvertices)[idx*3 + 0] + m_dx,
						 (*m_pvertices)[idx*3 + 1] + m_dy,
						 (*m_pvertices)[idx*3 + 2] + m_dz)
			  ? GL_TRUE:GL_FALSE
			);
		if ( GL_FALSE == (*m_pedgeflags)[idx] ) {
			ret ++;
		}
	}
	m_cout << ret << " points culled due to changes in edge flags\n";
	return ret;
}

ostream& CIselbox::reportself(ostream& os, bool bthin)
{
	os << setprecision(6);
	if ( bthin ) {
		os << m_minx << " " << m_miny << " " << m_minz << "\n";
		os << m_maxx << " " << m_maxy << " " << m_maxz << "\n";
		return os;
	}

	os << "min: (" << m_minx << "," << m_miny << "," << m_minz << ")\n";
	os << "max: (" << m_maxx << "," << m_maxy << "," << m_maxz << ")\n";

	// color of the box wire frame
	os << "frame color: (" << m_fcr << "," << m_fcg << "," << m_fcb << ")\n";
	return os;
}

bool CIselbox::isInside(GLdouble x, GLdouble y, GLdouble z)
{
	return ( m_bRemovalbased !=
			(x <= m_maxx && x >= m_minx &&
			y <= m_maxy && y >= m_miny &&
			z <= m_maxz && z >= m_minz) );
}

int CIselbox::transformBoxVertices()
{
	GLint viewport[4];
	GLdouble mvmat[16];
	GLdouble prjmat[16];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	glGetDoublev(GL_PROJECTION_MATRIX, prjmat);
	
	// mime the vertex processing pipeline of openGL
	for (size_t i = 0; i < m_boxvertices.size(); ++i) {
		// viewing and model transformation
		transpoint(mvmat, 
				m_boxvertices[i].x, 
				m_boxvertices[i].y, 
				m_boxvertices[i].z,
				m_boxvertices[i].w
				);
		
		// projection transformation
		transpoint(prjmat, 
				m_boxvertices[i].x, 
				m_boxvertices[i].y, 
				m_boxvertices[i].z,
				m_boxvertices[i].w
				);

		// viewport transformation
		m_boxvertices[i].x = 0.5*(m_boxvertices[i].x + 1)*viewport[2];
		m_boxvertices[i].y = 0.5*(m_boxvertices[i].y + 1)*viewport[3];
		m_boxvertices[i].z = 0.5*(m_boxvertices[i].z + 1);
	}

	return 0;
}

int CIselbox::getSelectedFace(int x, int y)
{
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) {
			// we just check faces that are facing the user currently
			continue;
		}

		if ( m_faces[i].isInFace( (GLdouble)x, (GLdouble)y ) ) {
			return i;
		}
	}

	// no user-oriented face selected now
	return -1;
}

int CIselbox::getSelectedVertex(int x, int y)
{
	int ret = -1;
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) {
			// only vertices of front facing sides are to be checked
			continue;
		}

		ret = m_faces[i].getNearestVidx( (GLdouble)x, (GLdouble)y );
		if (-1 != ret) {
			return ret;
		}
	}

	return -1;
}

edge_idx_t CIselbox::getSelectedEdge(int x, int y)
{
	edge_idx_t ret(-1,-1);
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) {
			// only vertices of front facing sides are to be checked
			continue;
		}

		if ( 0 == m_faces[i].getNearestEidx( (GLdouble)x, (GLdouble)y, ret ) ) {
			return ret;
		}
	}

	return ret;
}

const point_t CIselbox::getCenter() const
{
	return point_t( (m_minx + m_maxx)/2.0, 
			(m_miny + m_maxy)/2.0, 
			(m_minz + m_maxz)/2.0 );
}

bool CIselbox::switchLimit()
{
	m_bmvlimit = !m_bmvlimit;
	return m_bmvlimit;
}

bool CIselbox::switchHint()
{
	m_bhint = !m_bhint;
	if ( !m_bhint ) {
		for (size_t i = 0; i < m_faces.size(); ++i) {
			m_faces[i].setSelected(false);
		}
	}
	return m_bhint;
}

CIselbox CIselbox::clone(GLdouble dx, GLdouble dy, GLdouble dz)
{
	CIselbox other(*this);

	if ( 0 == dx && 0 == dy && 0 == dz ) {
		dx = dy = dz = max( (m_maxx - m_minx), 
				max( (m_maxy - m_miny), (m_maxz - m_minz) ) );
	}

	other.m_minx += dx;
	other.m_miny += dy;
	other.m_minz += dz;
	other.m_maxx += dx;
	other.m_maxy += dy;
	other.m_maxz += dz;
	return other;
}

GLboolean& CIselbox::isRemovalBased() 
{
	return m_bRemovalbased;
}

int CIselbox::onMouseClicked(int button, int state, 
								int x, int y)
{
	m_bMoveAlongZ = false;
	m_bEPscaling = false;
	int ret = 1;
	if ( GLUT_DOWN == state ) {
		// transform box vertices in case certain transformation has taken place
		// however, the transformation will be taken into account during the
		// following _objcoord2wincoord, so here it is superfluous
		//transformBoxVertices();

		// update windows coordinates of box vertices
		_objcoord2wincoord();

		m_selectedVid = getSelectedVertex(x,y);
		if ( -1 == m_selectedVid ) {
			// we always set a higher priority for vertex selection, then edge
			// selection and finally for face selection
			m_selectedEid = getSelectedEdge(x,y);
			if (edge_idx_t(-1,-1) == m_selectedEid) {

				m_selectedFid  = getSelectedFace(x,y);
				if ( -1 == m_selectedFid ) {
					m_cout << "no face selected.\n";
					return 1;
				}
				if ( GLUT_RIGHT_BUTTON == button ) {
					m_bEPscaling = m_faces[m_selectedFid].atCenter(x,y);
				}
				m_cout << "(" << x << "," << y << ") falls in face " << m_selectedFid << "\n";
				ret = 0;
			}
			else {
				// ready to start moving box along Z axis
				if ( GLUT_RIGHT_BUTTON == button ) {
					m_bMoveAlongZ = true;
				}
				m_cout << "(" << x << "," << y << ") closes to edge " << m_selectedEid;
				ret = 2;
			}
		}
		else {
			m_cout << "(" << x << "," << y << ") snaps vertex " << m_selectedVid << "\n";
			ret = 4;
		}

		if ( m_bmvlimit ) {
			GLdouble ox, oy, oz;
			if ( m_selectedVid != -1 ) {
				m_nearestZ = m_winboxvertices[m_selectedVid].z;
			}
			else if ( m_selectedEid != edge_idx_t(-1,-1) ){
				m_nearestZ = m_winboxvertices[ m_selectedEid[0] ].z;
				if ( edge_idx_t(m_winboxvertices[ m_selectedEid[1] ].x,
								m_winboxvertices[ m_selectedEid[1] ].y).distanceTo( edge_idx_t(x,y) )
					 < edge_idx_t(m_winboxvertices[ m_selectedEid[0] ].x,
								m_winboxvertices[ m_selectedEid[1] ].y).distanceTo( edge_idx_t(x,y) ) ) {
					m_nearestZ = m_winboxvertices[ m_selectedEid[1] ].z;
				}
			}
			else {
				m_nearestZ = m_faces[m_selectedFid].getNearestZ(x,y);
			}

			_wincoord2objcoord(x, y, m_nearestZ, &ox, &oy, &oz);
			//m_cout << "nearestZ = " << m_nearestZ << "\n";
			//m_cout << "ox=" << ox << ",oy=" << oy << ", oz=" << oz << "\n";

			m_mx = ox, m_my = oy, m_mz = oz;
		}
		else {
			m_mx = x, m_my = y;
		}
		m_mx = x, m_my = y;

		m_pressedbtn = button;
	}
	else { // when button has been bounced up again
		m_pressedbtn = -1;
		m_selectedFid = -1;
	}

	switch (button) {
      case GLUT_LEFT_BUTTON:
         if (state == GLUT_DOWN) {
			m_cout << "selbox stretching by mouse.\n";
         }
		 else {
			m_cout << "selbox stretching finished.\n";
		 }
         break;
      case GLUT_MIDDLE_BUTTON:
		 return 1;
      case GLUT_RIGHT_BUTTON:
		 // moving/zooming selection box
         if (state == GLUT_DOWN) {
			 m_cout << "selbox " <<
				 (m_bMoveAlongZ || GLUT_ACTIVE_CTRL == glutGetModifiers()?"zooming":
				  m_bEPscaling || GLUT_ACTIVE_SHIFT == glutGetModifiers()?"equal-proportion scaling":"moving")
				 << " by mouse.\n";
         }
		 else {
			 m_cout << "selbox " <<
				 (m_bMoveAlongZ || GLUT_ACTIVE_CTRL == glutGetModifiers()?"zooming":
				  m_bEPscaling || GLUT_ACTIVE_SHIFT == glutGetModifiers()?"equal-proportion scaling":"moving")
				 << " finished.\n";
		 }
         break;
      default:
		 break;
	} 
	// when more than one box are hit by current cursor, we take the box
	// expected to accept "corner stretching" as prior one, so the caller need
	// to know which action, corner or face stretching, is attempted
	//return (m_selectedVid != -1)?2:0;
	return ret;
}

int CIselbox::onKeyPressed(unsigned char key)
{
	return 1;
}

int CIselbox::onSpecialKeyPressed(int key)
{
	return 1;
}

int CIselbox::onMouseMove(int x, int y)
{
	if ( (-1 == m_selectedFid && -1 == m_selectedVid && edge_idx_t(-1,-1) == m_selectedEid)
		   	|| -1 == m_pressedbtn ) {
		return 1;
	}

	GLdouble dx, dy, dz;
	GLdouble ox = x, oy = y, oz = 0;
	GLdouble mins [] = { m_minx, m_miny, m_minz };
	GLdouble maxs [] = { m_maxx, m_maxy, m_maxz };

	if (m_bmvlimit) {
		_wincoord2objcoord(x, y, m_nearestZ, &ox, &oy, &oz);

		/* since we have stricter check against the moving face's vertices in
		 * window coordinate, against the viewport as well, this check will 
		 * never fail, thus become nonsense
		GLint viewport[4];
		glGetIntegerv(GL_VIEWPORT, viewport);

		if ( x <= viewport[0] || x >= viewport[2] ||
			 y <= viewport[1] || y >= viewport[3] ) {
			m_mx = ox, m_my = oy, m_mz = oz;
			m_cout << "selection box stretching out of viewport.\n";
			return 0;
		}
		*/

		dx = ox - m_mx, dy = - oy + m_my, dz = oz - m_mz;
		//cout << "dx=" << dx << ",dy=" << dy << ",dz=" << dz << "\n";
	}
	else {
		dx = x - m_mx, dy = y - m_my, dz = 0;
	}

	dx = x - m_mx, dy = y - m_my, dz = 0;

	/* enlightened by the solution to precise global translation for all objects in the scene,
	 * ameliorate the inveterate problem of imprecise movement imposed upon selection boxes
	 * IT WORKS, benefitting from the basic math of openGL
	 */
	GLint viewmat[4];
	GLdouble mvmat[16];
	GLdouble prjmat[16];
	glGetIntegerv(GL_VIEWPORT, viewmat);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	glGetDoublev(GL_PROJECTION_MATRIX, prjmat);

	GLdouble eyeZ = mvmat[14], fvoy = atan(1.0/prjmat[5])*2 * 180.0/M_PI;
	GLdouble fh = 1/
		((abs(eyeZ)>1e-6?abs(eyeZ):1)*tan(fvoy*M_PI/180.0/2.0)/viewmat[3]);
	GLdouble fw = fh / (viewmat[2]*1.0/viewmat[3]);

	dx = dx/fw, dy = dy*2/fh, dz = dz/fh;

	int ret = 0;
	switch (m_pressedbtn) {
		case GLUT_LEFT_BUTTON: // sel box stretching
			{
				// resizing by left dragging a box vertex
				if ( m_selectedVid != -1 ) {
					//m_boxvertices[m_selectedVid].update(dx, dy, min(dx,dy));
					//  each box vertex pertains to three adjacent faces
					static GLubyte findices[][3] = { 
						{0,1,2},
						{0,1,4},
						{0,3,4},
						{0,2,3},
						{2,3,5},
						{3,4,5},
						{1,4,5},
						{1,2,5}
					};
					int oldsFid = m_selectedFid;
					for (int k = 0; k < 3; ++k) {
						ret = _strechFace( 
								m_selectedFid = findices[m_selectedVid][k], 
								dx/1.3, dy/1.3, dz);
						if ( 0 != ret ) {
							break;
						}
					}

					m_selectedFid = oldsFid;
					ret = _onMinMaxUpdated(2);
				}
				// resizing by left dragging a box edge
				else if ( edge_idx_t(-1,-1) != m_selectedEid ) {
					int oldsFid = m_selectedFid;
					for ( int k = 0; k < (int)m_faces.size(); k++) {
						if ( std::find(m_faces[k].vidxs.begin(), m_faces[k].vidxs.end(),
									m_selectedEid[0]) == m_faces[k].vidxs.end() ||
							std::find(m_faces[k].vidxs.begin(), m_faces[k].vidxs.end(),
									m_selectedEid[1]) == m_faces[k].vidxs.end() ) {
							continue;
						}

						ret = _strechFace( m_selectedFid = k, 
								dx/1.3, dy/1.3, dz);
						if ( 0 != ret ) {
							break;
						}
					}
					m_selectedFid = oldsFid;
				}
				// resizing by left dragging a box face
				else { // m_selectedFid != -1 
					ret = _strechFace(m_selectedFid, dx, dy, dz);
				}
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			{
			} 
			return 1;
		case GLUT_RIGHT_BUTTON: // sel box moving
			{
				GLdouble a = dx, b = -dy, c = dz, d = 0;

				if ( m_bEPscaling || GLUT_ACTIVE_SHIFT == glutGetModifiers() ) {
					GLdouble d = fabs(dx) > fabs(dy)?dx:dy;
					// keep proportion among the three dimensions
					GLdouble xd = -d, yd = xd * ((m_maxy - m_miny)/(m_maxx - m_minx)),
							 zd = xd * ((m_maxz - m_minz)/(m_maxx - m_minx));
					m_minx += xd, m_maxx -= xd;
					m_miny += yd, m_maxy -= yd;
					m_minz += zd, m_maxz -= zd;
				}
				else {
					if ( m_bMoveAlongZ || GLUT_ACTIVE_CTRL == glutGetModifiers() ) {
						// moving along the Z axis
						a = b = 0;
						/*
						c = 1;
						c *= (dx > 0 || dy > 0)?1:-1;
						*/
						c = fabs(dx) > fabs(dy)?-dx:dy;
						c*=3.5;
					}
					transpoint(mvmat, a, b, c, d);
					m_minx += a, m_maxx += a;
					m_miny += b, m_maxy += b;
					m_minz += c, m_maxz += c;
				}
				ret = _onMinMaxUpdated(2);
			}
			break;
		default:
			return 1;
	}
	if ( m_bmvlimit ) {
		m_mx = ox, m_my = oy, m_mz = oz;
	}
	else {
		m_mx = x, m_my = y;
	}
	m_mx = x, m_my = y;
	
	//refreshEdgeFlags();
	if ( 0 != ret ) {
		m_minx = mins[0], m_miny = mins[1], m_minz = mins[2];
		m_maxx = maxs[0], m_maxy = maxs[1], m_maxz = maxs[2];
		ret = 0;
	}
	return ret;
}

int CIselbox::onMousePassiveMove(int x, int y)
{
	if ( !m_bhint ) {
		return 1;
	}

	// update windows coordinates of box vertices
	_objcoord2wincoord();

	// judge if any vertex highligthed in the first place
	int nhVid = getSelectedVertex(x,y);
	if ( nhVid == m_highlightedVid ) {
		if ( m_highlightedVid != -1 ) {
			return 2;
		}
		else {
			goto L_edgehighlight;
		}
	}

	if ( m_highlightedVid != -1 ) {
		SETCURSOR(GLUT_CURSOR_LEFT_ARROW);
	}
	m_highlightedVid = nhVid;
	if ( m_highlightedVid != -1 ) {
		SETCURSOR(GLUT_CURSOR_CROSSHAIR);
		glutPostRedisplay();
		m_cout << "box vertex " << nhVid << " hightlighted.\n";
		return 2;
	}

L_edgehighlight:
	// if no vertex hightlighted, then see if we should highlight any edge
	edge_idx_t nhEid = getSelectedEdge(x,y);
	if ( nhEid == m_highlightedEid ) {
		if ( m_highlightedEid != edge_idx_t(-1,-1) ) {
			return 4;
		}
		else {
			goto L_facehighlight;
		}
	}

	if ( m_highlightedEid != edge_idx_t(-1,-1) ) {
		SETCURSOR(GLUT_CURSOR_LEFT_ARROW);
	}
	m_highlightedEid = nhEid;
	if ( m_highlightedEid != edge_idx_t(-1,-1) ) {
		SETCURSOR(GLUT_CURSOR_TOP_RIGHT_CORNER);
		glutPostRedisplay();
		m_cout << "box edge highlighted :" << nhEid;
		return 4;
	}

L_facehighlight:
	// if no edge hightlighted, then see if we should highlight any face
	int nhFid = getSelectedFace(x,y);
	int ret = 1;

	// check if it is ready for "right dragging face for equal-proportion
	// scaling"
	GLboolean orgEPscaling = m_bEPscaling;
	m_bEPscaling = false;
	if ( nhFid != -1 ) {
		if ( m_faces[nhFid].atCenter( (GLdouble)x, (GLdouble)y ) ) {
			SETCURSOR(GLUT_CURSOR_CYCLE);
			m_bEPscaling = true;
			ret = 6;
		}
		if ( orgEPscaling != m_bEPscaling ) {
			glutPostRedisplay();
		}
	}

	if ( nhFid == m_highlightedFid ) {
		if ( m_highlightedFid != -1 && 6 != ret ) {
			ret = 0;
		}
		return ret;
		//return m_highlightedFid != -1?0:1;
	}
	else {
		glutPostRedisplay();
	}

	if (m_highlightedFid != -1) {
		m_faces[m_highlightedFid].setSelected(false);
	}
	m_highlightedFid = nhFid;

	if ( m_highlightedFid != -1 ) {
		m_faces[m_highlightedFid].setSelected(true);
		m_cout << "face " << nhFid << " hightlighted.\n";
		if ( 6 != ret ) {
			ret = 0;
		}
	}

	return ret;
}

int CIselbox::_updateMinMax()
{
	// start over the min/max competetion among the box vertices
	for (size_t i = 0; i < m_boxvertices.size(); ++i) {
		if ( m_boxvertices[i].x < m_minx ) {
			m_minx = m_boxvertices[i].x;
		}
		if ( m_boxvertices[i].y < m_miny ) {
			m_miny = m_boxvertices[i].y;
		}
		if ( m_boxvertices[i].z < m_minz ) {
			m_minz = m_boxvertices[i].z;
		}
		if ( m_boxvertices[i].x > m_maxx ) {
			m_maxx = m_boxvertices[i].x;
		}
		if ( m_boxvertices[i].y > m_maxy ) {
			m_maxy = m_boxvertices[i].y;
		}
		if ( m_boxvertices[i].z > m_maxz ) {
			m_maxz = m_boxvertices[i].z;
		}
	}
	return 0;
}

int CIselbox::_onMinMaxUpdated(int bCheckRange)
{
	if ( m_bmvlimit && 0 != bCheckRange ) {
		if (m_maxx <= m_minx || m_maxy <= m_miny || m_maxz <= m_minz) {
			m_cout << "Forbidden: face to impinge over its opposite.\n";
			return -1;
		}
	}

	vector<point_t> orgBoxVertices;
	if ( m_bmvlimit && 0 != bCheckRange ) {
		orgBoxVertices = m_boxvertices;
	}

	// all eight vertices are determined by the two opposite corners
	m_boxvertices[0].update(m_maxx, m_maxy, m_minz); // 1
	m_boxvertices[1].update(m_maxx, m_miny, m_minz); // 2
	m_boxvertices[2].update(m_minx, m_miny, m_minz); // 3
	m_boxvertices[3].update(m_minx, m_maxy, m_minz); // 4

	m_boxvertices[4].update(m_minx, m_maxy, m_maxz); // 5
	m_boxvertices[5].update(m_minx, m_miny, m_maxz); // 6
	m_boxvertices[6].update(m_maxx, m_miny, m_maxz); // 7
	m_boxvertices[7].update(m_maxx, m_maxy, m_maxz); // 8

	if (!m_bmvlimit || 0 == bCheckRange) {
		return 0;
	}

	_objcoord2wincoord();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// check if currently face will go out the viewport
	if ( 1 == bCheckRange ) {
		assert ( m_selectedFid >= 0 );
		const vector<GLubyte>& vidxs = m_faces[m_selectedFid].vidxs;

		/*
		// all vertices out of viewport is forbidden
		for (size_t i=0; i<vidxs.size(); ++i) {
			if ( !(m_winboxvertices[ vidxs[i] ].x <= viewport[0] || 
				 m_winboxvertices[ vidxs[i] ].x >= viewport[2] ||
				 m_winboxvertices[ vidxs[i] ].y <= viewport[1] ||
				 m_winboxvertices[ vidxs[i] ].y >= viewport[3]) ) {
				return 0;
			}
		}
		m_cout << "selection box stretching out of viewport.\n";
		m_boxvertices = orgBoxVertices;
		return -1;
		*/

		// no vertex can be out of viewport
		for (size_t i=0; i<vidxs.size(); ++i) {
			if ( m_winboxvertices[ vidxs[i] ].x <= viewport[0] || 
				 m_winboxvertices[ vidxs[i] ].x >= viewport[2] ||
				 m_winboxvertices[ vidxs[i] ].y <= viewport[1] ||
				 m_winboxvertices[ vidxs[i] ].y >= viewport[3] ) {
				 m_cout << "current face out of viewport.\n";
				 m_boxvertices = orgBoxVertices;
				 return -1;
			}
		}
		return 0;
	}

	// check if the whole box will go out the viewport
	if ( 2 == bCheckRange ) {
		/*
		// all vertices out of viewport is forbidden
		for (size_t i=0; i<m_winboxvertices.size(); ++i) {
			if ( !(m_winboxvertices[i].x <= viewport[0] || 
				 m_winboxvertices[i].x >= viewport[2] ||
				 m_winboxvertices[i].y <= viewport[0] ||
				 m_winboxvertices[i].y >= viewport[3]) ) {
				return 0;
			}
		}
		m_cout << "selection box getting out of viewport.\n";
		m_boxvertices = orgBoxVertices;
		return -1;
		*/

		// no vertex can be out of viewport
		for (size_t i=0; i<m_winboxvertices.size(); ++i) {
			if ( m_winboxvertices[i].x <= viewport[0] || 
				 m_winboxvertices[i].x >= viewport[2] ||
				 m_winboxvertices[i].y <= viewport[1] ||
				 m_winboxvertices[i].y >= viewport[3] ) {
				m_cout << "selection box getting out of viewport.\n";
				m_boxvertices = orgBoxVertices;
				return -1;
			}
		}
		return 0;
	}

	return 0;
}

int CIselbox::_initFaces()
{
	// all six faces
	GLubyte indices[][4] = { 
		{1,2,3,4},
		{1,8,7,2},
		{1,4,5,8},
		{4,3,6,5},
		{2,7,6,3},
		{5,6,7,8}
	};
	m_faces.resize(6);
	for (int i = 0; i < 6; i++) {
		for (int j = 0; j < 4; j++) {
			m_faces[i].vidxs.push_back( indices[i][j] - 1 );
		}
		m_faces[i].pvs = &m_boxvertices;
		m_faces[i].wps = &m_winboxvertices;
	}

	return 0;
}

int CIselbox::_objcoord2wincoord()
{
	GLint viewport[4];
	GLdouble mvmat[16], prjmat[16];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	glGetDoublev(GL_PROJECTION_MATRIX, prjmat);

	GLboolean ret;
	for (size_t i = 0; i < m_boxvertices.size(); ++i) {
		ret = gluProject(m_boxvertices[i].x, m_boxvertices[i].y, m_boxvertices[i].z,
				mvmat, prjmat, viewport,
				&m_winboxvertices[i].x, &m_winboxvertices[i].y, &m_winboxvertices[i].z);
		m_winboxvertices[i].y = viewport[3] - (GLint)m_winboxvertices[i].y;
		if (GL_TRUE != ret) {
			m_cout << "FATAL: failed in gluProject for CIselbox::_objcoord2wincoord\n";
			return -1;
		}
	}

	return 0;
}

int CIselbox::_wincoord2objcoord(
		int winx, int winy, int winz,
		GLdouble *objx, GLdouble *objy, GLdouble *objz)
{
	GLint viewport[4];
	GLdouble mvmat[16], prjmat[16];

	glGetIntegerv(GL_VIEWPORT, viewport);
	for (int i=0;i<4;++i) {
		for (int j=0;j<4;++j) {
			mvmat[ i*4+j ] = (i==j?1:0);
			prjmat[ i*4+j ] = (i==j?1:0);
		}
	}

	winy = viewport[3] - (GLint)winy;

	if ( GL_TRUE != gluUnProject((GLdouble)winx, (GLdouble)winy, (GLdouble)winz, 
								mvmat, prjmat, viewport,
								objx, objy, objz)) {
		m_cout << "FATAL: failed in gluUnProject for CIselbox::_wincoord2objcoord\n";
		return -1; 
	}

	*objx *= fabs(m_dx*2);
	*objy *= fabs(m_dy*2);
	*objz *= fabs(m_dz*2);

	return 0;
}

int CIselbox::_strechFace(int fidx, GLdouble dx, GLdouble dy, GLdouble dz)
{
	if ( fidx < 0 || fidx >= (int)m_faces.size() ) {
		return -1;
	}
	GLdouble sign = 1;
	point_t selectedFaceNormal = 
		m_faces[fidx].getNormalInWincoord();
	// find the axes having the smallest angle to the
	// face normal
	point_t axes[4];
	axes[0].update(0,1,0);
	axes[1].update(0,-1,0);
	axes[2].update(1,0,0);
	axes[3].update(-1,0,0);
	GLdouble angle, minAngle = M_PI*3;
	int minIdx = 0;
	for (int i=0;i<4;++i) {
		angle = selectedFaceNormal.angleTo( axes[i] );
		if (angle < minAngle) {
			minAngle = angle;
			minIdx = i;
		}
	}
	dz = ( minIdx > 1 )?dx:dy;

	switch (fidx) {
		// since mouse movement did not derive delta in Z axis,
		// substitution from that in X or Y is needed therefore.
		case 0:
			sign *= ( minIdx % 2 == 0 )?-1:1;
			m_minz += sign*dz;
			break;
		case 5:
			sign *= ( minIdx % 2 == 0 )?1:-1;
			m_maxz += sign*dz;
			break;
		case 1:
			sign *= ( minIdx % 2 == 0 )?1:-1;
			m_maxx += sign*dz;
			break;
		case 3:
			sign *= ( minIdx % 2 == 0 )?-1:1;
			m_minx += sign*dz;
			break;
		case 2:
			sign *= ( minIdx % 2 == 0 )?1:-1;
			m_maxy += sign*dz;
			break;
		case 4:
			sign *= ( minIdx % 2 == 0 )?-1:1;
			m_miny += sign*dz;
			break;
		default:
			break;
	}
	return _onMinMaxUpdated(1);
}

int CIselbox::_calcBtnLayout(int fidx)
{
	if ( fidx < 0 ) {
		if ( m_highlightedFid < 0 ) {
			return -1;
		}
		fidx = m_highlightedFid;
	}

	int signx = 1, signy = 1, offset = 1;
	GLdouble startx, starty, twidth, theight, __z;
	_point_t<int> pointidxs(0,0,0,0);
	switch ( fidx ) {
		case 0:
			startx = m_maxx - offset;
			starty = m_maxy - offset;
			__z = m_minz;
			signx = -1;
			signy = 1;
			twidth = m_maxx - m_minx - offset;
			theight = m_maxy - m_miny - offset;
			pointidxs.update(0,1,2);
			break;
		case 1:
			startx = m_maxz - offset;
			starty = m_maxy - offset;
			__z = m_maxx;
			signx = -1;
			signy = 1;
			twidth = m_maxz - m_minz - offset;
			theight = m_maxy - m_miny - offset;
			pointidxs.update(2,1,0);
			break;
		case 2:
			startx = m_minx + offset;
			starty = m_minz + offset;
			__z = m_maxy;
			signx = 1;
			signy = -1;
			twidth = m_maxx - m_minx - offset;
			theight = m_maxz - m_minz - offset;
			pointidxs.update(0,2,1);
			break;
		case 3:
			startx = m_minz + offset;
			starty = m_maxy - offset;
			__z = m_minx;
			signx = 1;
			signy = 1;
			twidth = m_maxz - m_minz - offset;
			theight = m_maxy - m_miny - offset;
			pointidxs.update(2,1,0);
			break;
		case 4:
			startx = m_minx + offset;
			starty = m_maxz - offset;
			__z = m_miny;
			signx = 1;
			signy = 1;
			twidth = m_maxx - m_minx - offset;
			theight = m_maxz - m_minz - offset;
			pointidxs.update(0,2,1);
			break;
		case 5:
			startx = m_minx + offset;
			starty = m_maxy - offset;
			__z = m_maxz;
			signx = 1;
			signy = 1;
			twidth = m_maxx - m_minx - offset;
			theight = m_maxy - m_miny - offset;
			pointidxs.update(0,1,2);
			break;
		default: // quaint face, refuse to handle!
			return -1;
	}

	int szbtn = int(m_pbuttons->size());
	GLdouble mvmat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	GLdouble sf = fabs(mvmat[14]/(m_dz*3))/2.5;
	int btnW = 3+2, btnH = 4+2;
	btnW *= sf;
	btnH *= sf;
	GLdouble spacing = 1 * sf;
	int numPerRow = fabs(twidth) / (spacing + btnW);
	if (numPerRow < 1) {
		numPerRow = 1;
	}
	GLdouble curlbx, curlby, currtx, currty;
	int numRow = int(szbtn*1.0 / numPerRow + .999999), ibtn = 0;
	if ( numRow < 1 ) {
		numRow = 1;
	}

	// if there is no enough space on the face to hold all buttons, it will be
	// improper to show all of these buttons forcibly since that will cause
	// conflict between the box button interaction and the interaction for the
	// ordinary face stretching
	if ( numRow*btnH + (numRow-1)*spacing > fabs(theight) ) {
		return -1;
	}

	curlby = starty - btnH*signy;
	point_t _coordlb, _coordrt;
	//int textwidth = glutBitmapLength(pfont, (const unsigned char*)text.c_str());
	for (int i = 0; i <= numRow; ++i) {
		curlbx = startx + spacing*signx;
		for (int j = 0; j < numPerRow; ++j) {
			currtx = curlbx + btnW*signx;
			currty = curlby + btnH*signy;

			_coordlb.update( curlbx, curlby, __z );
			_coordrt.update( currtx, currty, __z );
			for ( int k = 0; k < 3; ++k ) {
				(*m_pbuttons)[ ibtn ].leftbottom[ pointidxs[k] ] = _coordlb[k]; 
				(*m_pbuttons)[ ibtn ].righttop[ pointidxs[k] ] = _coordrt[k];
				(*m_pbuttons)[ ibtn ].textpos [ pointidxs[k] ] = (_coordlb[k] + _coordrt[k] )/2;
			}

			/*
			textwidth = glutBitmapLength(pfont, 
					(const unsigned char*)(*m_pbuttons)[ibtn].text.c_str());
			(*m_pbuttons)[ ibtn ].textpos.update(
						(*m_pbuttons)[ ibtn ].leftbottom.x,
						(*m_pbuttons)[ ibtn ].leftbottom.y,
						(*m_pbuttons)[ ibtn ].leftbottom.z
					);
			*/

			curlbx += (btnW + spacing)*signx;
			ibtn ++;
			if ( ibtn >= szbtn ) {
				return 0;
			}
		}
		curlby -= (btnH + spacing)*signy;
	}

	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

