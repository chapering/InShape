// ----------------------------------------------------------------------------
// glface.cpp : class encapsulation for describing and manipulating a face, shortly,
//				a GL polygon and mostly a quadruple
//
// Creation : Sept. 22nd 2011
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "glface.h"

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
		return a <= 0;
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
	/*
	if ( isSelected ) {
		mode = GL_POLYGON;
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glColor4f( 0.3f, 0.3f, 0.3f, 0.2f );
	}
	//glBegin (GL_LINE_LOOP);
	//glBegin (GL_POLYGON);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	if ( !isFrontFacing() ) {
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0001);
	}
	*/
	glBegin (mode);
		for (size_t i = 0; i < vidxs.size(); ++i) {
			glVertex3f( (*pvs)[ vidxs[i] ].x,
						(*pvs)[ vidxs[i] ].y,
						(*pvs)[ vidxs[i] ].z );
		}
	glEnd();
	/*
	if ( isSelected ) {
		glPopAttrib();
	}
	glPopAttrib();
	*/
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
		curDist = p.distanceTo( (*wps)[ vidxs[i] ] );
		if (curDist < minDist) {
			minDist = curDist;
			ti = i;
		}
	}

	return (*wps) [ vidxs[ti] ].z;
}

int _face_t::getNearestVidx(GLdouble x, GLdouble y)
{
	GLdouble minDist = 0xffffffff, curDist;
	point_t p(x,y);
	int ret = -1;
	int ti = (int)vidxs.size();
	for (int i = 0; i < (int)vidxs.size(); ++i) {
		// retrieve the vertex nearest to (x,y) that is closer than the
		// predefined threshold to this mouse point
		curDist = p.distanceTo( point_t((*wps)[ vidxs[i] ].x, (*wps)[ vidxs[i] ].y) );
		if (curDist > CORNER_DIST_THRESHOLD) {
			continue;
		}
		if (curDist < minDist) {
			minDist = curDist;
			ti = i;
		}
	}

	if ( ti < (int)vidxs.size() ) {
		ret = (int)vidxs[ti];
	}

	return ret;
}

int _face_t::getNearestEidx(GLdouble x, GLdouble y, edge_idx_t& eIdx)
{
	GLdouble minDist = 0xffffffff, curDist;
	point_t p(x,y);
	int ret = -1;
	int ti = (int)vidxs.size(), tj = 0;
	for (int i = 0,j = 0; i < (int)vidxs.size(); ++i) {
		// retrieve the edge nearest to (x,y) that is closer than the
		// predefined threshold to this mouse point
		j = i+1;
		if ( i == (int)vidxs.size() - 1) {
			j = 0;
		}

		// we are actually deciding if (x,y) is close to a straight line segment 
		// rather than a mathematical straight line
		if ( x < min( (*wps)[ vidxs[i] ].x, (*wps)[ vidxs[j] ].x ) || 
			x > max( (*wps)[ vidxs[i] ].x, (*wps)[ vidxs[j] ].x ) || 
			y < min( (*wps)[ vidxs[i] ].y, (*wps)[ vidxs[j] ].y ) || 
			y > max( (*wps)[ vidxs[i] ].y, (*wps)[ vidxs[j] ].y ) ) {
			continue;
		}
		curDist = p.dist2line( point_t((*wps)[ vidxs[i] ].x, (*wps)[ vidxs[i] ].y),
				point_t( (*wps)[ vidxs[j] ].x, (*wps)[ vidxs[j] ].y ) );

		if (curDist > EDGE_DIST_THRESHOLD) {
			continue;
		}
		if (curDist < minDist) {
			minDist = curDist;
			ti = i;
			tj = j;
		}
	}

	if ( ti < (int)vidxs.size() ) {
		eIdx.update(vidxs[ti], vidxs[tj]);
		ret = 0;
	}

	return ret;
}

bool _face_t::atCenter(GLdouble x, GLdouble y, int proportion)
{
	/*
	GLdouble cx = ( ( (*wps)[ vidxs[0] ].x + (*wps)[ vidxs[1] ].x )*1.0/2
				+ ( (*wps)[ vidxs[2] ].x + (*wps)[ vidxs[3] ].x )*1.0/2 )/2;
	GLdouble cy = ( ( (*wps)[ vidxs[0] ].y + (*wps)[ vidxs[1] ].y )*1.0/2
				+ ( (*wps)[ vidxs[2] ].y + (*wps)[ vidxs[3] ].y )*1.0/2 )/2;
	*/

	GLdouble maxx = (*wps)[ vidxs[0] ].x, maxy = (*wps)[ vidxs[0] ].y,
			 minx = maxx, miny = maxy;
	for (int i = 1; i < (int)vidxs.size(); ++i) {
		if ( maxx < (*wps)[ vidxs[i] ].x ) {
			maxx = (*wps)[ vidxs[i] ].x;
		}
		if ( minx > (*wps)[ vidxs[i] ].x ) {
			minx = (*wps)[ vidxs[i] ].x;
		}
		if ( maxy < (*wps)[ vidxs[i] ].y ) {
			maxy = (*wps)[ vidxs[i] ].y;
		}
		if ( miny > (*wps)[ vidxs[i] ].y ) {
			miny = (*wps)[ vidxs[i] ].y;
		}
	}

	if (proportion < 2) {
		proportion = 2;
	}

	int regW = (maxx - minx)/proportion;
	int regH = (maxy - miny)/proportion;
	if ( x >= minx + (maxx-minx -regW)/2 && 
		 x <= minx + maxx-minx - (maxx-minx - regW)/2 &&
		 y >= miny + (maxy-miny -regH)/2 && 
		 y <= miny + maxy-miny - (maxy-miny - regH)/2 ) {
		return true;
	}

	return false;
}

point_t _face_t::getCenter(bool bwinc) const
{
	vector<point_t>* pp = (bwinc?wps:pvs);

	GLdouble x = 0, y = 0, z = 0;
	for (int i = 0; i < (int)vidxs.size(); ++i) {
		x += (*pp)[ vidxs[i] ].x;
		y += (*pp)[ vidxs[i] ].y;
		z += (*pp)[ vidxs[i] ].z;
	}
	x /= (int)vidxs.size();
	y /= (int)vidxs.size();
	z /= (int)vidxs.size();
	return point_t(x,y,z);
}

/* set ts=4 sts=4 tw=80 sw=4 */

