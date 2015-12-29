// ----------------------------------------------------------------------------
// iselbox.h : an independent class encapsulating functions needed for a
//				selection box in the interactive graphics application.
//
// Creation : Feb. 11th 2011
// revision : 
//  @Feb. 12th 2011 
//			adjust into an independent class from the
//			original CGLApp inheritance for the purpose of the support of
//			multiple-box selection
//	@Feb. 19th 2011
//			evolution into intuitive user-oriented box selection, with
//			resistance to modeling and viewing transformations
//	@Feb. 21th 2011
//			1.upgrade the front-facing judgement algorithm to what is suggested
//			in the redBook of openGL, and make the decision more correct and
//			reliable
//			2.interaction for the face movement on the selection box perfected
//			3.whole selection box moving interaction added, following the same
//			idea for the same interaction applied towards the whole object:
//			 - mouse dragging with right button for translation along X/Y axis
//			 - combined with CTRL+Right_Mouse_Button for translating along Z-axis,
//	@Feb. 22th 2011
//			1. add hint for the currently front facing faces of the selection
//			box
//			2. when mouse moving over a selectable (front facing) face, further
//			cueing the user by filling the polygon translucently.
//			3. fix the glitch in the selection stretching : while face 1,3
//			rotated to be upwards or downwards, or face 2,4 to be leftwards or
//			rightwards,the movement of mouse does not match the desirable
//			stretching of the faces intuitively. Fixing by following the same
//			idea for face 0 and 5.
//			4. add limit on the selection stretching : forbidding a movement
//			intending to push or pull a face over its opposite one, which is
//			meaningless in most cases and absurd in the box selection
//			interaction especially.
//	@Feb. 23th 2011
//			1. Make an uniform and smoother movement in the selection box
//			stretching by calculating the difference of object coordinates
//			"Unprojected" from the mouse window coordinate rather than the
//			original mouse vector length as used before
//			2. the very initial selection stretching is changed to be driven by
//			the movement along the Y axis rathen than X axis as was before, this
//			should perfect the interaction by making it further intuitive
//			3. add limit on the selection stretching : forbidding a movement
//			that will lead the focused face to go beyond the window
//			4. add a switch for the selection box sketching/moving limit
//			control, when it is off, the abrupt, maybe faster in other users'
//			view, movement will be resumed by using the mouse vector length
//			again.
//			5. add a switch for the front facing hint. When users prefer to
//			better performance, they might want to turn off this gaudy feature
// @Feb. 25th 2011
//			1. add a default axis orientation gadget
//			2. change the front facing hint filling color to grey
// @Feb. 26th 2011
//			1. place a more rigid limit control on the box stretching by
//			detecting if the focused face has any of its vertices in window
//			coordinate got outside the viewport, rather than simply checking if
//			the mouse click goes beyond the viewport as previously adopted
// @Mar. 8th 2011
//			1.in order to make the behaviour of m_pcout controllable as m_cout
//			in its hosting container CGLIBoxApp, change this pointer to a plain
//			MyCout instance; the properties of it will be force to conformed
//			to that of CGLIBoxApp::m_cout when a CIselbox object is added to its
//			container, a CGLIBoxApp object
// @Mar. 10th 2011
//			1.Extra member CIselbox::clone() is added to support run-time
//			addition or removal of selection box; cloning a selection box is to
//			duplicate a new box with the same size but with a deviation in terms
//			of position to help user discern it from the source; for this
//			purpose, copy constructor of CIselbox ensues
// @Mar. 11th 2011
//			1.For correct cloning of CIselbox, default trivial operator = is not
//			sufficiently secure so is also needed, thus added as well; for this
//			purpose, copy constructor and operator = are both added to _face_t
//			as well in the consideration of pointer safety
// @Mar. 12th 2011
//			1.Fix a seemingly odd, really inveterate, bug: when wanting to
//			delete an non-tail box, the operation actually causes all boxes
//			after it removed;  Long reckoning leads to the bug found in
//			CIselbox::operator=. this operator will be called indirectly by
//			CGLIBoxApp::removeBox.
// @Mar. 15th 2011
//			.getDimension() is added to offer the caller the size of this box
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _ISELBOX_H_
#define _ISELBOX_H_

#include <iostream>
#include <vector>

#include <GL/glut.h>
#include "glrand.h"
#include "point.h"
#include "cppmoth.h"

class CGLIBoxApp;

using std::vector;
using std::ostream;
using std::cout;

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
}face_t;

class CIselbox {
public:
	friend class CGLIBoxApp;

	CIselbox();
	CIselbox(const CIselbox& other);
	~CIselbox();

	CIselbox& operator = (const CIselbox& other);

	// tell the position and size of the selection box, as is simply by
	// indicating the two opposite corners, i.e. (minx, miny, minz) and 
	// (maxx, maxy, maxz), the box is always rectangular
	void setMinMax(GLdouble minx, GLdouble miny, GLdouble minz,
					GLdouble maxx, GLdouble maxy, GLdouble maxz);

	// designate preferrable color to use while drawing the box frame
	void setFrameColor(GLfloat r=0.2, GLfloat g=0.5, GLfloat b=0.4);

	// trigger of frame drawing
	void drawFrame();

	/*
	 * @brief associate vertices of object with the instance of CIselbox
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
	ostream& reportself(ostream& os);

	// judege if the single vertex, not moust point, (x,y,z) is inside this
	// box or not using the simplest collision detection 
	bool isInside(GLdouble x, GLdouble y, GLdouble z);

	// transform all the eight box vertices according to the current matrices at
	// the top of each of the matrix stacks of  modelview and projection
	// transformation
	int transformBoxVertices();

	// examine which face is currently hit with mouse coordinate (x,y)
	int getSelectedFace(int x, int y);

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
	CIselbox clone(GLdouble dx=0, GLdouble dy=0, GLdouble dz=0);

	// dimension, i.e. the distance from minor corner to the major corner
	GLdouble getDimension() const;
	
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

	// color of the box wire frame
	GLdouble m_fcr, m_fcg, m_fcb;

	/* record which mouse button is being pressed, -1 indicates none */
	int m_pressedbtn;

	/* record recently selected face index */
	int m_selectedFid;

	/* whenever a mouse cursor passing over a face, the face will be highlighted
	 * as a sort of hint
	 */
	int m_highlightedFid;

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

	// update selection box vertices ON THE TWO OPPOSITE CORNERS 
	int _updateMinMax();

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
	
	// internal initialization for all boundary faces
	int _initFaces();

	// convert box vertices of object coordinates to that of window coordinates
	int _objcoord2wincoord();

	// convert a window coordinate, the point of mouse click, say, to object
	// coordinate
	int _wincoord2objcoord( int winx, int winy, int winz,
		GLdouble *objx, GLdouble *objy, GLdouble *objz);
};

#endif // _ISELBOX_H_

/* set ts=4 sts=4 tw=80 sw=4 */

