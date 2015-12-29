/*
 * stereo rendering exemplifying the current implementation for the display study
 * hcai $Exp 06/08/2012
 *
 *
 * Acknowledgement:
 *	this method is coded on the basis of that written by Brian Paul for "glxgears" 
 *	and copyright info is:
 *		"Copyright (C) 1999-2001  Brian Paul   All Rights Reserved."
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>

#ifndef M_PI
#define M_PI 3.14159265
#endif

static GLboolean fullscreen = GL_FALSE;	/* Create a single fullscreen window */
static GLboolean stereo = GL_FALSE;	/* Enable stereo.  */
static GLfloat eyesep = 1.0;		/* Eye separation. */
static GLfloat fix_point = 100.0;	/* Fixation point distance.  */
static GLfloat left, right, asp;	/* Stereo frustum params.  */

/*
   Set up the lighing environment
*/
static void 
MakeLighting(void)
{
    GLfloat fullambient[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat position[4] = { 0.0, 0.0, 0.0, 0.0 };
    GLfloat ambient[4] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[4] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat specular[4] = { 0.0, 0.0, 0.0, 1.0 };

    /* Turn off all the lights */
    glDisable(GL_LIGHT0);
    glDisable(GL_LIGHT1);
    glDisable(GL_LIGHT2);
    glDisable(GL_LIGHT3);
    glDisable(GL_LIGHT4);
    glDisable(GL_LIGHT5);
    glDisable(GL_LIGHT6);
    glDisable(GL_LIGHT7);
    glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_FALSE);

    /* Turn on the appropriate lights */
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, fullambient);
    glLightfv(GL_LIGHT0, GL_POSITION, position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
    glEnable(GL_LIGHT0);

    /* Sort out the shading algorithm */
    glShadeModel(GL_SMOOTH);

    /* Turn lighting on */
    glEnable(GL_LIGHTING);
}

static void 
keyFunc(unsigned char key, int x, int y)
{
    switch (key) {
		case 27:			/* Quit */
		case 'Q':
		case 'q':
			exit(0);
		defaut:
			break;
	}
}

static void
do_draw(void)
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glColor3f(0.0, 1.0, 1.0);
   glutWireCube(1.0);
   glutSwapBuffers();
}

static void
draw(void)
{
   if (stereo) {
      /* First left eye.  */
      glDrawBuffer(GL_BACK_LEFT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glFrustum(left, right, -asp, asp, 5.0, 60.0);

      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glTranslated(+0.5 * eyesep, 0.0, 0.0);
      do_draw();
      glPopMatrix();

      /* Then right eye.  */
      glDrawBuffer(GL_BACK_RIGHT);

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glFrustum(-right, -left, -asp, asp, 5.0, 60.0);

      glMatrixMode(GL_MODELVIEW);

      glPushMatrix();
      glTranslated(-0.5 * eyesep, 0.0, 0.0);
      do_draw();
      glPopMatrix();
   } else
      do_draw();
}

/* new window size or exposure */
static void
reshape(int width, int height)
{
   glViewport(0, 0, (GLint) width, (GLint) height);

   if (stereo) {
      GLfloat w;
	  GLfloat wf = 50.0;
	  w = fix_point * (1.0 / wf);

      asp = (GLfloat) height / (GLfloat) width;

      left = -wf * ((w - 0.5 * eyesep) / fix_point);
      right = wf * ((w + 0.5 * eyesep) / fix_point);
   } else {
      GLfloat h = (GLfloat) height / (GLfloat) width;

      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glFrustum(-1.0, 1.0, -h, h, 5.0, 60.0);
   }
   
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0.0, 0.0, -10.0);
   glRotatef(30, 0,1,0);
   glRotatef(30, 1,0,0);
}

int
main(int argc, char *argv[])
{
   int i;

   for (i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-stereo") == 0) {
         stereo = GL_TRUE;
      }
      else if (strcmp(argv[i], "-fullscreen") == 0) {
         fullscreen = GL_TRUE;
      }
      else {
		 printf("Warrning: unknown parameter: %s\n", argv[i]);
	  }
   }

   glutInit (&argc, argv);
   glutInitWindowPosition ( 0, 0 );
   glutInitWindowSize ( 600, 480 );
   if ( stereo ) {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STEREO );
   }
   else {
	   glutInitDisplayMode	( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );
   }
   glutCreateWindow ( "stereo rendering - following Brain Paul's implementation in glxgears" );
   if ( fullscreen ) {
	   glutFullScreen();
   }
   glutReshapeFunc(reshape);
   glutDisplayFunc(draw);
   glutKeyboardFunc(keyFunc);
   glutMainLoop() ;

   return 0;
}

