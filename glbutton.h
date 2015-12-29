// ----------------------------------------------------------------------------
// glbutton.h : a button class description simply shaped in a GL quadruple
//
// Creation : Sept. 22nd 2011
// revision : 
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#ifndef _GLBUTTON_H_
#define _GLBUTTON_H_

#include <iostream>
#include <vector>

#include <GL/glut.h>
#include "glrand.h"
#include "point.h"

using std::vector;
using std::ostream;
using std::cout;
using std::string;

// struct describing a button
typedef struct _button_t {
	// two points defining the bounding rectangle : in object coordinates
	// used to draw button frame and print button text
	point_t leftbottom, righttop;
	// two points defining the bounding rectangle : in window coordinates
	// used to detect mouse hit
	vector<point_t> winVertices;

	// where the button text will start to display, not pondering on the length
	// of the text string
	point_t textpos;
	string text;

	bool isSelected;	// if this button is currently focused
	bool	isShown;	// if this button is currently shown
	bool isPressed;		// if the button is currently pressed down

	unsigned char *pIconImgData;	// graphic image as alternative to the button text
	unsigned int iconW, iconH;		// width and height of the icon image

	bool isPrimary;
	string text2;		// for binary-status button, the text for another status
	unsigned char *pIconImgData2;	// graphic image for another status of the button
	unsigned int iconW2, iconH2;		// width and height of the secondary icon image

	_button_t(const string& btntext=" ", const string& btntext2="") {
		winVertices.resize(4);
		text = btntext;
		isSelected = false;
		isShown	 = false;
		isPressed = false;
		pIconImgData = NULL;
		iconW = iconH = 0;

		isPrimary = true;

		text2 = btntext2;
		pIconImgData2 = NULL;
		iconW2 = iconH2 = 0;
	}

	_button_t(const _button_t& other);
	_button_t& operator = (const _button_t& other);

	void setSelected(bool bSelected = false);
	void setPressed(bool bPressed = false);
	void switchStatus();
	bool isInButton(GLdouble x, GLdouble y);
	void draw();
	// convert button location from object coordinates to window coordinates
	int  _updateBtnWinCoords(const vector<point_t>& btnFrameVertices);
}button_t, *pbutton_t;

#endif // _GLBUTTON_H_

/* set ts=4 sts=4 tw=80 sw=4 */

