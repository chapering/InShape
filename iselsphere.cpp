// ----------------------------------------------------------------------------
// iselsphere.cpp : an independent class encapsulating functions needed for a
//				selection box in the interactive graphics application.
//
// Creation : Sept. 20th 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "iselsphere.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// class implementation of CIselsphere
//
CIselsphere::CIselsphere() :
	m_minx(LOCAL_MAXDOUBLE),	
	m_miny(LOCAL_MAXDOUBLE),	
	m_minz(LOCAL_MAXDOUBLE),	
	m_maxx(-LOCAL_MAXDOUBLE),	
	m_maxy(-LOCAL_MAXDOUBLE),	
	m_maxz(-LOCAL_MAXDOUBLE),
	m_fcr(1.0f), m_fcg(1.0f), m_fcb(1.0f),	
	m_pressedbtn(-1),
	m_nearestZ(0),
	m_mx(0), m_my(0), m_mz(0),
	m_pvertices(NULL),
	m_pedgeflags(NULL),
	m_dx(0),m_dy(0),m_dz(0),
	m_bmvlimit(true),
	m_bhint(true),
	m_bMoveAlongZ(false),
	m_bEPscaling(true),
	m_bRemovalbased(false),
	m_longs(100),
	m_lats(100),
	m_radius(1.0)
{
}

CIselsphere::CIselsphere(const CIselsphere& other)
{
	m_pressedbtn = -1;
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;
	m_bmvlimit = true;
	m_bhint = true;
	m_bMoveAlongZ = false;
	m_bEPscaling = true;
	// a box will inherit the selection mode from its producer
	m_bRemovalbased = other.m_bRemovalbased;
	m_pvertices = NULL;
	m_pedgeflags = NULL;

	m_longs = other.m_longs;
	m_lats = other.m_lats;
	m_radius = other.m_radius;
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

CIselsphere::~CIselsphere()
{
}

CIselsphere& CIselsphere::operator = (const CIselsphere& other)
{
	this->m_minx = other.m_minx;
	this->m_miny = other.m_miny;
	this->m_minz = other.m_minz;
	this->m_maxx = other.m_maxx;
	this->m_maxy = other.m_maxy;
	this->m_maxz = other.m_maxz;

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
	this->m_bMoveAlongZ = other.m_bMoveAlongZ;
	this->m_bEPscaling = other.m_bEPscaling;
	this->m_bRemovalbased = other.m_bRemovalbased;

	this->m_lats = other.m_lats;
	this->m_longs = other.m_longs;
	this->m_radius = other.m_radius;

	m_pressedbtn = -1;
	m_nearestZ = .0;
	m_mx = 0, m_my = 0, m_mz = 0;

	/* since already armed with new corners, faces and box vertices all need
	 * updating
	 */
	m_boxvertices.clear();
	m_winboxvertices.clear();
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_onCenterUpdated();
	_initFaces();

	return *this;
}

void CIselsphere::setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
				GLdouble maxx, GLdouble maxy, GLdouble maxz)
{
	m_minx = minx, m_miny = miny, m_minz = minz;
	m_maxx = maxx, m_maxy = maxy, m_maxz = maxz;


	m_radius = fabs( min( min(m_maxx-m_minx, m_maxy-m_miny), m_maxz-m_minz ) );

	// since we desire a sphere not elliptical curlinear shape; the virtual box is
	// also desirable to be a cube not cuboid
	m_maxx = m_minx + m_radius;
	m_maxy = m_miny + m_radius;
	m_maxz = m_minz + m_radius;

	m_objcenter.update( 
			(m_minx + m_maxx)/2.0, 
			(m_miny + m_maxy)/2.0, 
			(m_minz + m_maxz)/2.0 );

	m_radius /= 2.0;

	refreshGeometry();
}

void CIselsphere::refreshGeometry()
{
	m_boxvertices.resize(8);
	m_winboxvertices.resize(8);
	_onCenterUpdated();
	_initFaces();
}

void CIselsphere::setFrameColor(GLfloat r, GLfloat g, GLfloat b)
{
	m_fcr = r, m_fcg = g, m_fcb = b;
}

void CIselsphere::drawSphere(bool bBtnEnabled)
{
	glLineWidth(1.0);
	glColor3f(m_fcr, m_fcg, m_fcb);
	if ( ! m_bhint ) {
	}

	/*
	for (size_t i = 0; i < m_faces.size(); ++i) {
		m_faces[i].drawFrame();
	}
	*/

	_objcoord2wincoord();

	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glPushMatrix();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(m_fcr, m_fcg, m_fcb, 0.5);
	glShadeModel( GL_SMOOTH );
	glPolygonMode(GL_FRONT, GL_FILL);
	glEnable(GL_LIGHTING);

	GLdouble thi1 = .0, thi2 = .0;
	GLdouble theta = .0;

	point_t p1, p2;

	/* 
	 * we are not gonna use vertex buffer or display list because during 
	 * the interactivity, once the sphere gets resized, the geometry of the 
	 * sphere, i.e. the constructing vertices, needs be refreshed. 
	 * In the end therefore, we will not be able to benefit the vertex caching
	 * means in terms of run-time computational performance
	 */
	glTranslated( m_objcenter.x, m_objcenter.y, m_objcenter.z );
	for (int i = 0; i <= m_lats; i++) {
		thi1 = M_PI * (.5 + (GLdouble) (i-1) / m_lats);
		thi2 = M_PI * (.5 + (GLdouble) i / m_lats);

		glBegin( GL_QUAD_STRIP );
		for (int j = 0; j <= m_longs; j++) {
			theta = 2*M_PI* (j-1) / m_longs;

			p1.update ( m_radius*cos(theta)*cos(thi1),
						m_radius*sin(theta)*cos(thi1),
						m_radius*sin(thi1) );

			p2.update ( m_radius*cos(theta)*cos(thi2),
						m_radius*sin(theta)*cos(thi2),
						m_radius*sin(thi2) );

			//p1 = p1 - m_objcenter;
			//p2 = p2 - m_objcenter;

			glNormal3d( p1.x, p1.y, p1.z );
			glVertex3d( p1.x, p1.y, p1.z );

			glNormal3d( p2.x, p2.y, p2.z );
			glVertex3d( p2.x, p2.y, p2.z );
		}
		glEnd();
	}

	glPopMatrix();
	glPopAttrib();
}

int CIselsphere::associateObj(vector<GLdouble>* pvertices,
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

int CIselsphere::refreshEdgeFlags()
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

ostream& CIselsphere::reportself(ostream& os, bool bthin)
{
	os << setprecision(6);

	if ( bthin ) {
		os << m_minx << " " << m_miny << " " << m_minz << "\n";
		os << m_maxx << " " << m_maxy << " " << m_maxz << "\n";
		os << m_objcenter.x << " " << m_objcenter.y << " " << m_objcenter.z << "\n";
		os << m_radius << "\n";
		return os;
	}

	os << "min: (" << m_minx << "," << m_miny << "," << m_minz << ")\n";
	os << "max: (" << m_maxx << "," << m_maxy << "," << m_maxz << ")\n";
	os << "center: " << m_objcenter << "\n";
	os << "radius: " << m_radius << "\n";

	// color of the box wire frame
	os << "frame color: (" << m_fcr << "," << m_fcg << "," << m_fcb << ")\n";
	return os;
}

bool CIselsphere::isInside(GLdouble x, GLdouble y, GLdouble z)
{
	return m_objcenter.distanceTo( point_t(x,y,z) ) <= m_radius;
}

int CIselsphere::transformBoxVertices()
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

int CIselsphere::getSelectedFace(int x, int y)
{
	int ret = -1;
	/*
	for (size_t i = 0; i < m_faces.size(); ++i) {
		if ( !m_faces[i].isFrontFacing() ) {
			// we just check faces that are facing the user currently
			continue;
		}

		if ( !m_faces[i].isInFace( (GLdouble)x, (GLdouble)y ) ) {
			ret = i;
			break;
		}
	}

	if ( -1 == ret ) {
		return ret;
	}
	*/

	//point_t fw(x,y), fo;

	ret = -1;
	m_wincenter.z = .0;
	GLdouble dist = m_wincenter.distanceTo( point_t((GLdouble)x, (GLdouble)y, .0) );
	if ( dist  <= m_radius*4.0 ) {
		ret = 0;
	}

	// no user-oriented face selected now
	return ret;
}

const point_t CIselsphere::getCenter() const
{
	return m_objcenter;
}

bool CIselsphere::switchLimit()
{
	m_bmvlimit = !m_bmvlimit;
	return m_bmvlimit;
}

bool CIselsphere::switchHint()
{
	m_bhint = !m_bhint;
	if ( !m_bhint ) {
	}
	return m_bhint;
}

CIselsphere CIselsphere::clone(GLdouble dx, GLdouble dy, GLdouble dz)
{
	CIselsphere other(*this);

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

GLboolean& CIselsphere::isRemovalBased() 
{
	return m_bRemovalbased;
}

int CIselsphere::onMouseClicked(int button, int state, 
								int x, int y)
{
	m_bMoveAlongZ = false;
	m_bEPscaling = true;
	int ret = 1;
	if ( GLUT_DOWN == state ) {
		// transform box vertices in case certain transformation has taken place
		// however, the transformation will be taken into account during the
		// following _objcoord2wincoord, so here it is superfluous
		//transformBoxVertices();

		// update windows coordinates of box vertices
		_objcoord2wincoord();

		m_cout << "test if inside the sphere" << "\n";
		/*
		cout << m_winboxvertices[5];
		cout << m_winboxvertices[7];
		cout << "(" << x << "," << y << ")" << "\n";
		if ( x >= m_winboxvertices[5].x && x <= m_winboxvertices[7].x &&
			 y <= m_winboxvertices[5].y && y >= m_winboxvertices[7].y ) {
			m_cout << "(" << x << "," << y << ") falls in the sphere" << "\n";
			ret = 0;
		}
		*/

		if ( -1 != getSelectedFace(x,y) ) {
			m_cout << "(" << x << "," << y << ") falls in the sphere" << "\n";
			ret = 0;
		}

		if ( m_bmvlimit ) {
			GLdouble ox, oy, oz;
			m_nearestZ = m_wincenter.z;

			_wincoord2objcoord(x, y, m_nearestZ, &ox, &oy, &oz);

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
	}

	switch (button) {
      case GLUT_LEFT_BUTTON:
         if (state == GLUT_DOWN) {
			m_cout << "selsphere stretching by mouse.\n";
         }
		 else {
			m_cout << "selsphere stretching finished.\n";
		 }
         break;
      case GLUT_MIDDLE_BUTTON:
		 return 1;
      case GLUT_RIGHT_BUTTON:
		 // moving/zooming selection box
         if (state == GLUT_DOWN) {
			 m_cout << "selsphere " <<
				 (m_bMoveAlongZ || GLUT_ACTIVE_CTRL == glutGetModifiers()?"zooming":
				  m_bEPscaling || GLUT_ACTIVE_SHIFT == glutGetModifiers()?"equal-proportion scaling":"moving")
				 << " by mouse.\n";
         }
		 else {
			 m_cout << "selsphere " <<
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

int CIselsphere::onKeyPressed(unsigned char key)
{
	return 1;
}

int CIselsphere::onSpecialKeyPressed(int key)
{
	return 1;
}

int CIselsphere::onMouseMove(int x, int y)
{
	if ( -1 == m_pressedbtn ) {
		return 1;
	}

	GLdouble dx, dy, dz;
	GLdouble ox = x, oy = y, oz = 0;
	GLdouble mins [] = { m_minx, m_miny, m_minz };
	GLdouble maxs [] = { m_maxx, m_maxy, m_maxz };

	if (m_bmvlimit) {
		_wincoord2objcoord(x, y, m_nearestZ, &ox, &oy, &oz);

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
				GLdouble a = dx, b = -dy, c = dz, d = 0;
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
				ret = _onCenterUpdated(2);
			}
			break;
		case GLUT_MIDDLE_BUTTON:
			{
			} 
			return 1;
		case GLUT_RIGHT_BUTTON: // sel box moving
			{
				GLdouble d = fabs(dx) > fabs(dy)?dx:dy;
				// keep proportion among the three dimensions
				GLdouble xd = -d, yd = xd * ((m_maxy - m_miny)/(m_maxx - m_minx)),
						 zd = xd * ((m_maxz - m_minz)/(m_maxx - m_minx));
				m_minx += xd, m_maxx -= xd;
				m_miny += yd, m_maxy -= yd;
				m_minz += zd, m_maxz -= zd;

				ret = _onCenterUpdated(2);
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

int CIselsphere::onMousePassiveMove(int x, int y)
{
	if ( !m_bhint ) {
		return 1;
	}

	// update windows coordinates of box vertices
	//_objcoord2wincoord();
	return 0;
}

int CIselsphere::_updateCenter()
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
	m_objcenter.update( 
			(m_minx + m_maxx)/2.0, 
			(m_miny + m_maxy)/2.0, 
			(m_minz + m_maxz)/2.0 );
	return 0;
}

int CIselsphere::_onCenterUpdated(int bCheckRange)
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

	m_objcenter.update( 
			(m_minx + m_maxx)/2.0, 
			(m_miny + m_maxy)/2.0, 
			(m_minz + m_maxz)/2.0 );

	m_radius = fabs( min( min(m_maxx-m_minx, m_maxy-m_miny), m_maxz-m_minz ) );
	m_radius /= 2.0;

	if (!m_bmvlimit || 0 == bCheckRange) {
		return 0;
	}

	_objcoord2wincoord();

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	// check if the whole box will go out the viewport
	if ( 2 == bCheckRange ) {
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

int CIselsphere::_initFaces()
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

int CIselsphere::_objcoord2wincoord()
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
			m_cout << "FATAL: failed in gluProject for CIselsphere::_objcoord2wincoord\n";
			return -1;
		}
	}

	ret = gluProject(m_objcenter.x, m_objcenter.y, m_objcenter.z,
				mvmat, prjmat, viewport,
				&m_wincenter.x, &m_wincenter.y, &m_wincenter.z);
	m_wincenter.y = viewport[3] - (GLint)m_wincenter.y;

	if (GL_TRUE != ret) {
		m_cout << "FATAL: failed in gluProject for _objcoord2wincoord\n";
		return -1;
	}

	return 0;
}

int CIselsphere::_wincoord2objcoord(
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
		m_cout << "FATAL: failed in gluUnProject for CIselsphere::_wincoord2objcoord\n";
		return -1; 
	}

	*objx *= fabs(m_dx*2);
	*objy *= fabs(m_dy*2);
	*objz *= fabs(m_dz*2);

	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

