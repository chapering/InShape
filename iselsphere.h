// ----------------------------------------------------------------------------
// iselsphere.h : an independent class encapsulating functions needed for a
//				selection sphere in the interactive graphics application.
//
// Creation : Sept. 20th 2011
// revision : 
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _ISELSPHERE_H_
#define _ISELSPHERE_H_

#include <iostream>
#include <vector>

#include <GL/glut.h>
#include "glrand.h"
#include "point.h"
#include "cppmoth.h"

#include "glface.h"
#include "iselbox.h"

class CGLIBoxApp;

using std::vector;
using std::ostream;
using std::cout;
using std::string;

class CIselsphere : public CIselbox {
public:
	friend class CGLIBoxApp;

	CIselsphere();
	CIselsphere(const CIselsphere& other);
	~CIselsphere();

	CIselsphere& operator = (const CIselsphere& other);

	// tell the position and size of the selection box, as is simply by
	// indicating the two opposite corners, i.e. (minx, miny, minz) and 
	// (maxx, maxy, maxz), the box is always rectangular
	void setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
					GLdouble maxx, GLdouble maxy, GLdouble maxz);

	// trigger of frame drawing
	void drawFrame(bool bBtnEnabled = true);

	// mostly used for debugging
	ostream& reportself(ostream& os, bool bthin=false);

	// judege if the single vertex, not moust point, (x,y,z) is inside this
	// box or not using the simplest collision detection 
	bool isInside(GLdouble x, GLdouble y, GLdouble z);

	// examine which face is currently hit with mouse coordinate (x,y)
	int getSelectedFace(int x, int y);

	// examine which of the eight vertices is currently hit
	int getSelectedVertex(int x, int y);

	// examine which of the twelve edges is currently hit
	edge_idx_t getSelectedEdge(int x, int y);

	// obtain current centric point of the box
	const point_t getCenter() const;

	// duplicate a selection box, possibly with shifts
	// (0==dx && 0==dy && 0==dz) indicates using the largest dimension of the
	// source box as the displacement
	CIselbox* clone(GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

protected:
	/* 
	 * since the interaction handling is the major charge of this class, it
	 * might need hand over the user input received by the client, as is mostly
	 * the GL application
	 *
	 * returns:
	 *	0 indicates that things has been tackled and the event flow terminates
	 *	  therein
	 *	1 indicates a need for relaying event back tot the caller
	 *	-1 for errors
	 */
	int onMouseClicked(int button, int state, int x, int y);
	int onMouseMove(int x, int y);
	int onMousePassiveMove(int x, int y);

private:
	// sphere center
	point_t		m_objcenter;

	// the window coordinate counterpart to m_objcenter
	point_t		m_wincenter;

	/* LOD parameters : lattitudinal and longitudinal magnitude */
	GLint m_longs;
	GLint m_lats;

	/* sphere radius */
	GLdouble m_radius;
private:

	/* 
	 * @brief update box vertices while the two corners change and check 
	 * if the box will be stretched out of the viewport
	 * @param bCheckRange enumeration controlling the checking purview
	 *	- 0 no check at all
	 *	- 1 only check vertices on the selected face
	 *	- 2 check all vertices on the selected selection box
	 * @return 0 symbolizing pass and -1 for failure 
	 */
	int _onMinMaxUpdated(int bCheckRange=0);

	// convert box vertices of object coordinates to that of window coordinates
	int _objcoord2wincoord();
};

#endif // _ISELSPHERE_H_

/* set ts=4 sts=4 tw=80 sw=4 */

