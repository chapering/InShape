// ----------------------------------------------------------------------------
// glface.h : class encapsulation for describing and manipulating a face, shortly,
//				a GL polygon and mostly a quadruple
//
//				this is just simply picked up from original iselbox.h for a clearer
//				class design
//
// Creation : Sept. 22nd 2011
// revision : 
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _GLFACE_H_
#define _GLFACE_H_

#include <iostream>
#include <vector>

#include <GL/glut.h>
#include "glrand.h"
#include "point.h"

using std::vector;
using std::ostream;
using std::cout;
using std::string;

// distance threshold for corner touch judgement
const GLdouble CORNER_DIST_THRESHOLD = 3.50;
const GLdouble EDGE_DIST_THRESHOLD = 4.50;
typedef _point_t<int>	edge_idx_t;

// struct describing a polygon, as is also holding a 2D face
typedef struct  _face_t {
	vector<GLubyte> vidxs; // index to the vertex sequence
	vector<point_t>* pvs; // pointer to a sequence of vertices
	vector<point_t>* wps; // pointer to 2D points of window coordinate 
						 // corresponding to obj vertices pointed by pvs
	bool	isSelected;  // if this face is currently focused

	_face_t() {
		pvs = wps = NULL;
		isSelected = false;
	}

	_face_t(const _face_t& other);
	_face_t& operator = (const _face_t& other);

	void setSelected(bool bSelected = false);
	point_t getNormal() const;
	point_t getNormalInWincoord() const;
	bool isUserOriented();
	bool isFrontFacing();
	bool isInFace(GLdouble x, GLdouble y);
	void drawFrame();
	int  updateVertices(GLdouble dx = 0, GLdouble dy = 0, GLdouble dz = 0);
	GLdouble getNearestZ(GLdouble x, GLdouble y);
	int getNearestVidx(GLdouble x, GLdouble y);

	// the edge is represented by the indices of its two end vertices
	int getNearestEidx(GLdouble x, GLdouble y, edge_idx_t& eIdx);
	bool atCenter(GLdouble x, GLdouble y, int proportion=5);

	/*
	 * @brief retrieve the center in terms of vertex geometry
	 * @param bwinc a boolean value giving if calc center by window 
	 *  rather than object coordinate
	 * @return the center geometry
	 */
	point_t getCenter(bool bwinc = false) const;

}face_t, *pface_t;

#endif // _IGLFACE_H_

/* set ts=4 sts=4 tw=80 sw=4 */

