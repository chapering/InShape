// ----------------------------------------------------------------------------
// glbutton.cpp : a button class description simply shaped in a GL quadruple
//
// Creation : Sept. 22nd 2011
// revision : 
//
// Copyright(C) 2011-2012 Haipeng Cai
//
// ----------------------------------------------------------------------------
#include "glbutton.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////////
// implementation of struct button_t
//

_button_t::_button_t(const _button_t& other)
{
	leftbottom = other.leftbottom;
	righttop = other.righttop;
	winVertices.resize(4);
	winVertices = other.winVertices;
	text =  other.text;
	textpos = other.textpos;
	isSelected = false;
	isShown = false;
	isPressed = false;

	pIconImgData = other.pIconImgData;
	iconW = other.iconW;
	iconH = other.iconH;

	isPrimary = other.isPrimary;
	text2 = other.text2;
	pIconImgData2 = other.pIconImgData2;
	iconW2 = other.iconW2;
	iconH2 = other.iconH2;
}

_button_t& _button_t::operator = (const _button_t& other)
{
	leftbottom = other.leftbottom;
	righttop = other.righttop;
	winVertices.resize(4);
	winVertices = other.winVertices;
	text =  other.text;
	textpos = other.textpos;
	isSelected = other.isSelected;
	isShown = other.isShown;
	isPressed = other.isPressed;

	pIconImgData = other.pIconImgData;
	iconW = other.iconW;
	iconH = other.iconH;

	isPrimary = other.isPrimary;
	text2 = other.text2;
	pIconImgData2 = other.pIconImgData2;
	iconW2 = other.iconW2;
	iconH2 = other.iconH2;
	return *this;
}

void _button_t::setSelected(bool bSelected)
{
	isSelected = bSelected;
}

void _button_t::setPressed(bool bPressed)
{
	isPressed = bPressed;
}

void _button_t::switchStatus()
{
	isPrimary = !isPrimary;
}

bool _button_t::isInButton(GLdouble x, GLdouble y)
{
	if ( !isShown ) {
		return false;
	}
	/*
	// we presume the buttons are always decorated/bound by regular rectangles,
	// so the inclusion detection can be much simplified as follows
	return ( x >= wlb.x && x <= wrt.x &&
			y >= wlb.y && y <= wrt.y);
	*/
	// check (x,y) against each edge of the polygon, boundary of the face
	// similar way as to check if a mouse point is in a box face above, see
	// the annotation in _face_t::isInFace.
	int i,j,k;
	GLdouble sign1, sign2, slope;
	for (size_t ei = 0; ei < winVertices.size(); ++ei) {
		i = ei, j = (ei + 1) % winVertices.size();
		k = (ei+2) % winVertices.size();

		slope = ( (winVertices)[j].y - (winVertices)[i].y ) / ( (winVertices)[j].x - (winVertices)[i].x );
		sign1 =   (winVertices)[k].y - (winVertices)[i].y - slope* ( (winVertices)[k].x - (winVertices)[i].x );
		sign2 =   y - (winVertices)[i].y - slope* ( x - (winVertices)[i].x );

		if (sign1 * sign2 < 0) {
			return false;
		}
	}

	return true;
}

void _button_t::draw()
{
	// drawing a button is simply consisted of drawing a rectangular frame and
	// printing the button text; specifically, then highlighting by different
	// coloring and enlarging combined
	vector<point_t> frameVertices(4);
	if ( leftbottom.x == righttop.x ) {
		// on the yz plane
		frameVertices[0].update(leftbottom.x, righttop.y, leftbottom.z);
		frameVertices[1].update(leftbottom.x, leftbottom.y, leftbottom.z);
		frameVertices[2].update(leftbottom.x, leftbottom.y, righttop.z);
		frameVertices[3].update(leftbottom.x, righttop.y, righttop.z);
	}
	else if ( leftbottom.y == righttop.y ) {
		// on the xz plane
		frameVertices[0].update(leftbottom.x, leftbottom.y, righttop.z);
		frameVertices[1].update(leftbottom.x, leftbottom.y, leftbottom.z);
		frameVertices[2].update(righttop.x, leftbottom.y, leftbottom.z);
		frameVertices[3].update(righttop.x, leftbottom.y, righttop.z);
	}
	else if ( leftbottom.z == righttop.z ) {
		// on the xy plane
		frameVertices[0].update(leftbottom.x, righttop.y, righttop.z);
		frameVertices[1].update(leftbottom.x, leftbottom.y, righttop.z);
		frameVertices[2].update(righttop.x, leftbottom.y, righttop.z);
		frameVertices[3].update(righttop.x, righttop.y, righttop.z);
	}

	_updateBtnWinCoords( frameVertices );

	unsigned char* curpIconImg = isPrimary?pIconImgData:pIconImgData2;
	string curText = isPrimary?text:text2;
	unsigned int curiconH = isPrimary?iconH:iconH2;
	unsigned int curiconW = isPrimary?iconW:iconW2;

	if ( !curpIconImg ) { // no icon assigned, use text instead
		// draw the rectangular border
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_BLEND);
		glDisable(GL_LIGHTING);
		glLineWidth(2);
		glColor3f(0, 1, 0.5);
		if ( isSelected ) {
			glLineWidth(4);
			glColor3f(1, 0, 0.5);
		}
		if ( isPressed ) {
			glLineWidth(6);
			glColor3f(0, 0, 0.5);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_CULL_FACE);

		glBegin(GL_POLYGON);
			for (int i = 0; i < 4; ++i) {
				glVertex3f( frameVertices[i].x, frameVertices[i].y, frameVertices[i].z);
			}
		glEnd();

		// button text are never been printed as lucent or translucent since we just
		// want to see it clearly
		if (isPressed) {
			printText(curText.c_str(), 
					textpos.x, textpos.y, textpos.z,
					1, 0, 0, 1.0, GLUT_BITMAP_HELVETICA_12);
		}
		else if ( isSelected ) {
			printText(curText.c_str(), 
					textpos.x, textpos.y, textpos.z,
					1.0, 0.0, 0.0, 1.0, GLUT_BITMAP_TIMES_ROMAN_24);
		}
		else {
			printText(curText.c_str(), 
					textpos.x, textpos.y, textpos.z,
					1, 1, 0, 1.0, GLUT_BITMAP_HELVETICA_18);
		}

		glPopAttrib();
		isShown = true;
		return;
	}

	// draw icon image using 2D texture mapping if resource,i.e. the image, is available
	//
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	/* CREATE TEXTURE FROM Icon image for this button */
	GLuint texture_id;
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_2D, texture_id);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	*/
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//cout << "load icon image : " << iconW  << " x " << iconH << "\n";
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 
				(curiconW-2)/2*2+2, (curiconH-2)/2*2+2,
				isSelected?1:0, GL_RGB, 
				GL_UNSIGNED_BYTE, pIconImgData);

	/*
	for (int i = 0; i < iconW*iconH; i++) {
		printf(" 0x%02x ", pIconImgData[i]);
	}
	*/

	// ----------------------------------------------------
	// --------- To map the texture as follows successfully, 
	// --------- setting the polygon mode to GL_FILL is a must, must!!!
	// --------- having fiddled with this nasty problem for more than 5 hours at
	// --------- least. What a shame!
	//
	glDisable( GL_CULL_FACE );
	glDisable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glColor3f(1,1,1);
	if ( isSelected ) {
		glColor3f(1,1,0);
	}
	if ( isPressed ) {
		glColor3f(1,0,0);
	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); 
		glVertex3f( frameVertices[0].x, frameVertices[0].y, frameVertices[0].z);
		
		glTexCoord2f(1.0, 0.0); 
		glVertex3f( frameVertices[1].x, frameVertices[1].y, frameVertices[1].z);
		
		glTexCoord2f(1.0, 1.0); 
		glVertex3f( frameVertices[2].x, frameVertices[2].y, frameVertices[2].z);

		glTexCoord2f(0.0, 1.0); 
		glVertex3f( frameVertices[3].x, frameVertices[3].y, frameVertices[3].z);
	glEnd(); 

	/*
	glRasterPos3f( frameVertices[0].x, frameVertices[0].y, frameVertices[0].z);
	glDrawPixels( (iconW-2)/2*2+2, (iconH-2)/2*2+2,
			GL_RGB, GL_UNSIGNED_BYTE, pIconImgData);
	*/

	glPopAttrib();
	isShown = true;
}

int _button_t::_updateBtnWinCoords(const vector<point_t>& btnFrameVertices)
{
	GLint viewport[4];
	GLdouble mvmat[16], prjmat[16];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmat);
	glGetDoublev(GL_PROJECTION_MATRIX, prjmat);

	GLboolean ret;
	for (size_t i = 0; i < btnFrameVertices.size(); ++i) {
		ret = gluProject( 
				btnFrameVertices[i].x, btnFrameVertices[i].y, btnFrameVertices[i].z,
				mvmat, prjmat, viewport,
				&winVertices[i].x, &winVertices[i].y, &winVertices[i].z
				);
		winVertices[i].y = viewport[3] - winVertices[i].y;
		if (GL_TRUE != ret) {
			return -1;
		}
	}

	return 0;
}

/* set ts=4 sts=4 tw=80 sw=4 */

