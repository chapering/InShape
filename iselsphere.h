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

class CGLIBoxApp;

using std::vector;
using std::ostream;
using std::cout;
using std::string;

class CIselsphere {
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
	void refreshGeometry();

	// designate preferrable color to use while drawing the box frame
	void setFrameColor(GLfloat r=1.0, GLfloat g=1.0, GLfloat b=1.0);

	// trigger of frame drawing
	void drawSphere(bool bBtnEnabled = true);

	/*
	 * @brief associate vertices of object with the instance of CIselsphere
	 * @param pvertices pointer to the client vertex store
	 * @param pedgeflags pointer to the client edge flag store
	 * @return 0 for success and -1 otherwise
	 */
	int associateObj(vector<GLdouble>* pvertices, 
					vector<GLboolean>* pedgeflags,
					GLdouble dx = 0, GLdouble dy = 0, GLdouble dz = 0);

	/*
	 * @brief the core function of this class - update the edge flags of the
	 * client according to current box scale, the simple working is this:
	 *	EdgeFlag[ i ] = ( Vertex[ i ] is inside this box )
	 *	currently we use the brute-force algorithm,i.e. traverse throughout the
	 *	whole vertex store...
	 * @return -1 if error occurred, number of vertices that has been culled out
	 *		otherwise
	 */
	int refreshEdgeFlags();

	// mostly used for debugging
	ostream& reportself(ostream& os, bool bthin=false);

	// judege if the single vertex, not moust point, (x,y,z) is inside this
	// box or not using the simplest collision detection 
	bool isInside(GLdouble x, GLdouble y, GLdouble z);

	// transform all the eight box vertices according to the current matrices at
	// the top of each of the matrix stacks of  modelview and projection
	// transformation
	int transformBoxVertices();

	// examine which face is currently hit with mouse coordinate (x,y)
	int getSelectedFace(int x, int y);

	// obtain current centric point of the box
	const point_t getCenter() const;

	// alternate the control of limit on the selection box
	// stretching/moving
	// return the update status
	bool switchLimit();

	// alternate the selection hint for the front facing faces
	// return the update status
	bool switchHint();

	// duplicate a selection box, possibly with shifts
	// (0==dx && 0==dy && 0==dz) indicates using the largest dimension of the
	// source box as the displacement
	CIselsphere clone(GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

	// query the selection mode of this box alone, may also act as setting entry
	GLboolean& isRemovalBased();

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
	int onKeyPressed(unsigned char key);
	int onSpecialKeyPressed(int key);
	int onMouseMove(int x, int y);
	int onMousePassiveMove(int x, int y);

private:
	// two opposite corners of the box
	GLdouble m_minx, m_miny, m_minz;
	GLdouble m_maxx, m_maxy, m_maxz;

	// storage of the box vertices
	vector<point_t>		m_boxvertices;

	// the window coordinate counterparts to m_boxvertices
	vector<point_t>		m_winboxvertices;

	// all faces of the selection box
	vector<face_t>		m_faces;

	// sphere center
	point_t		m_objcenter;

	// the window coordinate counterpart to m_objcenter
	point_t		m_wincenter;

	// color of the box wire frame
	GLdouble m_fcr, m_fcg, m_fcb;

	/* record which mouse button is being pressed, -1 indicates none */
	int m_pressedbtn;

	/* z in the window coordinate of the box vertice that is the nearest to
	 * the point of mouse click
	 */
	GLdouble m_nearestZ;

	/* record object coordinate of mouse point when 
	 * any of the mouse buttons is pressed */
	GLdouble m_mx, m_my, m_mz;

	// reference to the geometry of the encompassed object
	vector<GLdouble>* m_pvertices;

	// reference to the flags used to control the visibility of vertices
	vector<GLboolean>* m_pedgeflags;

	// coordinate offsets, possibly needed for judging if vertices of the
	// encompassed object is inside the box
	GLdouble m_dx, m_dy, m_dz;

	// custom output stream sharing with that of the client
	MyCout m_cout;

	// limit on the selection box stretching/moving
	GLboolean m_bmvlimit;

	// hint for front facing faces
	GLboolean m_bhint;

	// ready for moving box along Z axis
	GLboolean m_bMoveAlongZ;

	// ready for equal-proportion scaling by right dragging
	GLboolean m_bEPscaling;

	/* switch between selection and removal mode */
	GLboolean m_bRemovalbased;

	/* LOD parameters : lattitudinal and longitudinal magnitude */
	GLint m_longs;
	GLint m_lats;

	/* sphere radius */
	GLdouble m_radius;
private:

	// update selection box vertices ON THE TWO OPPOSITE CORNERS 
	int _updateCenter();

	/* 
	 * @brief update box vertices while the two corners change and check 
	 * if the box will be stretched out of the viewport
	 * @param bCheckRange enumeration controlling the checking purview
	 *	- 0 no check at all
	 *	- 1 only check vertices on the selected face
	 *	- 2 check all vertices on the selected selection box
	 * @return 0 symbolizing pass and -1 for failure 
	 */
	int _onCenterUpdated(int bCheckRange=0);

	// internal initialization for all boundary faces
	int _initFaces();
	
	// convert box vertices of object coordinates to that of window coordinates
	int _objcoord2wincoord();

	// convert a window coordinate, the point of mouse click, say, to object
	// coordinate
	int _wincoord2objcoord( int winx, int winy, int winz,
		GLdouble *objx, GLdouble *objy, GLdouble *objz);

	// stretching a face according to given magnitude of movement
	int _strechFace(int fidx, GLdouble dx, GLdouble dy, GLdouble dz);
};

#endif // _ISELSPHERE_H_

/* set ts=4 sts=4 tw=80 sw=4 */

