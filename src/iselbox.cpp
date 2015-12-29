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
// implementation of struct face_t

_face_t::_face_t(const _face_t& other)
{
	pvs = NULL;
	wps = NULL;
	isSelected = false;
}

_face_t& _face_t::operator = (const _face_t& other)
{
	// vertex index store is also to be reconstructed
	vidxs.clear();
	// face must be associated with an object who will give the pointer to the
	// real storage of vertices as follows, just copying the pointers is
	// detrimental
	pvs = NULL;
	wps = NULL;
	isSelected = false;
	return *this;
}

void _face_t::setSelected(bool bSelected )
{
	isSelected = bSelected;
}

point_t _face_t::getNormal() const
{
	assert (pvs != NULL);
	vector_t edge1( (*pvs)[ vidxs[1] ].x - (*pvs)[ vidxs[0] ].x,
					(*pvs)[ vidxs[1] ].y - (*pvs)[ vidxs[0] ].y,
					(*pvs)[ vidxs[1] ].z - (*pvs)[ vidxs[0] ].z );

	vector_t edge2( (*pvs)[ vidxs[2] ].x - (*pvs)[ vidxs[1] ].x,
					(*pvs)[ vidxs[2] ].y - (*pvs)[ vidxs[1] ].y,
					(*pvs)[ vidxs[2] ].z - (*pvs)[ vidxs[1] ].z );

	return edge1.crossproduct(edge2);
}

point_t _face_t::getNormalInWincoord() const
{
	assert ( wps != NULL );
	return ((*wps)[ vidxs[1] ] - (*wps)[ vidxs[0] ]).crossproduct(
			(*wps)[ vidxs[2] ] - (*wps)[ vidxs[1] ]).normalize();
}

bool _face_t::isUserOriented()
{
	assert (pvs != NULL);
	// for simplicity, we presume that the vector connecting eye to the
	// centeroid of the viewport is always perpendicular to the field of view 
	GLdouble mvmat[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	
	GLdouble w = 0;
	GLdouble a = 0, b = 0, c = 1;
	transpoint(mvmat, a,b,c,w);

	GLdouble ang = vector_t(a,b,c).angleTo( getNormal() );

	return ang > M_PI/2;
}

bool _face_t::isFrontFacing()
{
	assert ( wps != NULL );
	// decide if this face is front facing by computing its area and then see
	// the sign of the area
	int i,j;
	GLdouble a = .0;
	for (size_t ei = 0; ei < vidxs.size(); ++ei) {
		i = vidxs[ei], j = vidxs[ (ei + 1) % vidxs.size() ];
		a += (*wps)[i].x * (*wps)[j].y - (*wps)[j].x * (*wps)[i].y;
	}
	a /= 2.0;

	GLint ff;
	glGetIntegerv(GL_FRONT_FACE, &ff);
	if ( ff == GL_CCW ) {
		return a < 0;
	}
	return a > 0;
}

bool _face_t::isInFace(GLdouble x, GLdouble y)
{
	assert ( wps != NULL );
	// check (x,y) against each edge of the polygon, boundary of the face
	int i,j,k;
	GLdouble sign1, sign2, slope;
	for (size_t ei = 0; ei < vidxs.size(); ++ei) {
		// current edge : 
		// (*pvs)[ vidxs[ei] ] -> (*pvs)[ vidxs[ (ei + 1) % vidxs.size() ] ]
		i = vidxs[ei], j = vidxs[ (ei + 1) % vidxs.size() ];
		k = vidxs[ (ei+2) % vidxs.size() ];

		// if (x,y) is inside the polygon, it should lead the normal line equation
		// established from the two ends, (*pvs)[i] and (*pvs)[j], to a same
		// sign as does any of other vertices than these two on the polygon
		slope = ( (*wps)[j].y - (*wps)[i].y ) / ( (*wps)[j].x - (*wps)[i].x );
		sign1 =   (*wps)[k].y - (*wps)[i].y - slope* ( (*wps)[k].x - (*wps)[i].x );
		sign2 =   y - (*wps)[i].y - slope* ( x - (*wps)[i].x );

		if (sign1 * sign2 < 0) {
			return false;
		}
	}

	return true;
}

void _face_t::drawFrame()
{
	GLenum mode = GL_LINE_STRIP;
	if ( isSelected ) {
		mode = GL_POLYGON;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPolygonMode(GL_FRONT, GL_FILL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f( 0.2f, 0.2f, 0.2f, 0.6f );
	}
	//glBegin (GL_LINE_LOOP);
	//glBegin (GL_POLYGON);
	glBegin (mode);
		for (size_t i = 0; i < vidxs.size(); ++i) {
			glVertex3f( (*pvs)[ vidxs[i] ].x,
						(*pvs)[ vidxs[i] ].y,
						(*pvs)[ vidxs[i] ].z );
		}
	glEnd();
	if ( isSelected ) {
		glPopAttrib();
	}
}

int _face_t::updateVertices(GLdouble dx, GLdouble dy, GLdouble dz) 
{

	for (size_t i = 0; i < vidxs.size(); ++i) {
		(*pvs)[ vidxs[i] ].x += dx;
		(*pvs)[ vidxs[i] ].y += dy;
		(*pvs)[ vidxs[i] ].z += dz;
	}

	return 0;
}

GLdouble _face_t::getNearestZ(GLdouble x, GLdouble y) 
{
	GLdouble minDist = 0xffffffff, curDist;
	point_t p(x,y);
	size_t ti = 0;
	for (size_t i = 0; i < vidxs.size(); ++i) {
		curDist = p.distanceTo( (*pvs)[ vidxs[i] ] );
		if (curDist < minDist) {
			minDist = curDist;
			ti = i;
		}
	}

	return (*wps) [ vidxs[ti] ].z;
}

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
	m_fcr(1.0f), m_fcg(0.1f), m_fcb(0.2f),	
	m_pressedbtn(-1),
	m_selectedFid(-1),
	m_highlightedFid(-1),
	m_nearestZ(0),
	m_mx(0), m_my(0), m_mz(0),
	m_pvertices(NULL),
	m_pedgeflags(NULL),
	m_dx(0),m_dy(0),m_dz(0),
	m_bmvlimit(true),
	m_bhint(true) 
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
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;
	m_bmvlimit = true;
	m_bhint = true;
	m_pvertices = NULL;
	m_pedgeflags = NULL;
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
		other.m_maxx, other.m_maxy, other.m_maxz);
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
	this->m_dx = other.m_dx;
	this->m_dy = other.m_dy;
	this->m_dz = other.m_dz;

	this->m_cout = other.m_cout;
	this->m_bmvlimit = other.m_bmvlimit;
	this->m_bhint = other.m_bhint;

	/* since already armed with new corners, faces and box vertices all need
	 * updating
	 */
	m_boxvertices.clear();
	m_winboxvertices.clear();
	m_faces.clear();
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_initFaces();
	_onMinMaxUpdated();

	m_pressedbtn = -1;
	m_selectedFid = -1;
	m_highlightedFid = -1;
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;

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

	m_boxvertices.clear();
	m_winboxvertices.clear();
	m_faces.clear();
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_onMinMaxUpdated();
	_initFaces();
}

void CIselbox::setFrameColor(GLfloat r, GLfloat g, GLfloat b)
{
	m_fcr = r, m_fcg = g, m_fcb = b;
}

void CIselbox::drawFrame()
{
	glColor3f(m_fcr, m_fcg, m_fcb);
	if ( ! m_bhint ) {
		for (size_t i = 0; i < m_faces.size(); ++i) {
			m_faces[i].drawFrame();
		}
		return;
	}

	_objcoord2wincoord();
	glLineWidth(1);

	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) {
			m_faces[i].drawFrame();
		}
	}

	glColor3f(m_fcb, m_fcr, m_fcg);
	glLineWidth(3);
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( m_faces[i].isFrontFacing() ) {
			m_faces[i].drawFrame();
		}
	}
}

int CIselbox::associateObj(vector<GLdouble>* pvertices,
						vector<GLboolean>* pedgeflags,
						GLdouble dx, GLdouble dy, GLdouble dz)
{
	m_dx = dx, m_dy = dy, m_dz = dz;

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

ostream& CIselbox::reportself(ostream& os)
{
	os << "min: (" << m_minx << "," << m_miny << "," << m_minz << ")\n";
	os << "max: (" << m_maxx << "," << m_maxy << "," << m_maxz << ")\n";

	// color of the box wire frame
	os << "frame color: (" << m_fcr << "," << m_fcg << "," << m_fcb << ")\n";
	return os;
}

bool CIselbox::isInside(GLdouble x, GLdouble y, GLdouble z)
{
	return x <= m_maxx && x >= m_minx &&
			y <= m_maxy && y >= m_miny &&
			z <= m_maxz && z >= m_minz;
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

GLdouble CIselbox::getDimension() const
{
	return point_t(m_minx, m_miny, m_minz).distanceTo(
			point_t(m_maxx, m_maxy, m_maxz) );
}

int CIselbox::onMouseClicked(int button, int state, 
								int x, int y)
{
	if ( GLUT_DOWN == state ) {
		// transform box vertices in case certain transformation has taken place
		// however, the transformation will be taken into account during the
		// following _objcoord2wincoord, so here it is superfluous
		//transformBoxVertices();

		// update windows coordinates of box vertices
		_objcoord2wincoord();

		m_selectedFid  = getSelectedFace(x,y);
		if ( -1 == m_selectedFid ) {
			m_cout << "no face selected.\n";
			return 1;
		}
		m_cout << "(" << x << "," << y << ") falls in face " << m_selectedFid << "\n";
		if ( m_bmvlimit ) {
			GLdouble ox, oy, oz;
			m_nearestZ = m_faces[m_selectedFid].getNearestZ(x,y);

			_wincoord2objcoord(x, y, m_nearestZ, &ox, &oy, &oz);
			//m_cout << "nearestZ = " << m_nearestZ << "\n";
			//m_cout << "ox=" << ox << ",oy=" << oy << ", oz=" << oz << "\n";

			m_mx = ox, m_my = oy, m_mz = oz;
		}
		else {
			m_mx = x, m_my = y;
		}
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
				 (GLUT_ACTIVE_CTRL == glutGetModifiers()?"zooming":"moving")
				 << " by mouse.\n";
         }
		 else {
			 m_cout << "selbox " <<
				 (GLUT_ACTIVE_CTRL == glutGetModifiers()?"zooming":"moving")
				 << " finished.\n";
		 }
         break;
      default:
		 break;
	} 
	return 0;
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
	if ( -1 == m_selectedFid ||
		 -1 == m_pressedbtn ) {
		return 1;
	}

	GLdouble dx, dy, dz;
	GLdouble ox, oy, oz;
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

	int ret = 0;
	switch (m_pressedbtn) {
		case GLUT_LEFT_BUTTON: // sel box stretching
			{
				GLdouble sign = 1;
				point_t selectedFaceNormal = 
					m_faces[m_selectedFid].getNormalInWincoord();
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

				switch (m_selectedFid) {
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
				ret = _onMinMaxUpdated(1);
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			{
			} 
			return 1;
		case GLUT_RIGHT_BUTTON: // sel box moving
			{
				GLdouble mvmat[16];
				glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
				GLdouble a = dx, b = -dy, c = dz, d = 0;

				if ( GLUT_ACTIVE_CTRL == glutGetModifiers() ) {
					a = b = 0;
					c = 1;
					c *= (dx > 0 || dy > 0)?1:-1;
				}
				transpoint(mvmat, a, b, c, d);
				m_minx += a, m_maxx += a;
				m_miny += b, m_maxy += b;
				m_minz += c, m_maxz += c;
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
	
	//refreshEdgeFlags();
	if ( 0 != ret ) {
		m_minx = mins[0], m_miny = mins[1], m_minz = mins[2];
		m_maxx = maxs[0], m_maxy = maxs[1], m_maxz = maxs[2];
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

	int nhFid = getSelectedFace(x,y);
	if ( nhFid == m_highlightedFid ) {
		return m_highlightedFid != -1?0:1;
	}

	if (m_highlightedFid != -1) {
		m_faces[m_highlightedFid].setSelected();
	}
	m_highlightedFid = nhFid;

	if ( m_highlightedFid != -1 ) {
		m_faces[m_highlightedFid].setSelected(true);
		m_cout << "face " << nhFid << " hightlighted.\n";
		return 0;
	}

	return 1;
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
	m_boxvertices[0] = (point_t(m_maxx, m_maxy, m_minz)); // 1
	m_boxvertices[1] = (point_t(m_maxx, m_miny, m_minz)); // 2
	m_boxvertices[2] = (point_t(m_minx, m_miny, m_minz)); // 3
	m_boxvertices[3] = (point_t(m_minx, m_maxy, m_minz)); // 4

	m_boxvertices[4] = (point_t(m_minx, m_maxy, m_maxz)); // 5
	m_boxvertices[5] = (point_t(m_minx, m_miny, m_maxz)); // 6
	m_boxvertices[6] = (point_t(m_maxx, m_miny, m_maxz)); // 7
	m_boxvertices[7] = (point_t(m_maxx, m_maxy, m_maxz)); // 8

	if (!m_bmvlimit || 0 == bCheckRange) {
		return 0;
	}

	_objcoord2wincoord();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	if ( 1 == bCheckRange ) {
		assert ( m_selectedFid >= 0 );
		const vector<GLubyte>& vidxs = m_faces[m_selectedFid].vidxs;

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
	}

	if ( 2 == bCheckRange ) {
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

/* set ts=4 sts=4 tw=80 sw=4 */

