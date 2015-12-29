/*
 * stereo rendering exemplifying the "correct method" implementation from 
 * hcai $Exp 06/08/2012
 *
 * Acknowledgement:
 *	this method is coded on the basis of that written by Paul Bourke in an example program
 *	for stereo rendering named "pulsar.c"
 */
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <GL/glut.h>

#define PI 3.141592653589793238462643
#define DTOR            0.0174532925
#define RTOD            57.2957795
#define CROSSPROD(p1,p2,p3) \
   p3.x = p1.y*p2.z - p1.z*p2.y; \
   p3.y = p1.z*p2.x - p1.x*p2.z; \
   p3.z = p1.x*p2.y - p1.y*p2.x


static GLboolean fullscreen = GL_FALSE;	/* Create a single fullscreen window */
static GLboolean stereo = GL_FALSE;	/* Enable stereo.  */

typedef struct {
    double x, y, z;
} XYZ;
typedef struct {
    double r, g, b;
} COLOUR;
typedef struct {
    unsigned char r, g, b, a;
} PIXELA;
typedef struct {
    XYZ vp;			/* View position           */
    XYZ vd;			/* View direction vector   */
    XYZ vu;			/* View up direction       */
    XYZ pr;			/* Point to rotate about   */
    double focallength;		/* Focal Length along vd   */
    double aperture;		/* Camera aperture         */
    double eyesep;		/* Eye separation          */
    int screenwidth, screenheight;
} CAMERA;
double dtheta = 1;
CAMERA camera;
XYZ origin = { 0.0, 0.0, 0.0 };

static void 
Normalise(XYZ * p)
{
    double length;

    length = sqrt(p->x * p->x + p->y * p->y + p->z * p->z);
    if (length != 0) {
		p->x /= length;
		p->y /= length;
		p->z /= length;
    } else {
		p->x = 0;
		p->y = 0;
		p->z = 0;
    }
}

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
initCamera()
{
    camera.aperture = 20;

	// increasing this parameter will make the 
	// stereo object pump out of the sreen, and decreasing will make it get into the screen
    camera.focallength = 50; 

    camera.eyesep = camera.focallength / 20;
    camera.pr = origin;

	/*
    camera.vp.x = 39;
    camera.vp.y = 53;
	*/
    camera.vp.x = 0;
    camera.vp.y = 0;
    camera.vp.z = 22;
    camera.vd.x = -camera.vp.x;
    camera.vd.y = -camera.vp.y;
    camera.vd.z = -camera.vp.z;

    camera.vu.x = 0;
    camera.vu.y = 1;
    camera.vu.z = 0;

    camera.screenwidth = 600;
    camera.screenheight = 480;
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
			return;
	}
}

static void
do_draw(void)
{
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glColor3f(0.0, 1.0, 1.0);

   glPushMatrix();
   glTranslatef(0.0, 0.0, -65.0);
   glRotatef(30, 0,1,0);
   glRotatef(30, 1,0,0);
   glutWireCube(10.0);
   glPopMatrix();

   glutSwapBuffers();
}

static void
draw(void)
{
    XYZ r;
    double dist, ratio, radians, scale, wd2, ndfl;
    double left, right, top, bottom, near = 0.1, far = 10000;

    /* Clip to avoid extreme stereo */
    if (stereo) {
		near = camera.focallength / 5;
	}

    /* Misc stuff */
    ratio = camera.screenwidth / (double) camera.screenheight;
    radians = DTOR * camera.aperture / 2;
    wd2 = near * tan(radians);
    ndfl = near / camera.focallength;

    /* Clear the buffers */
    glDrawBuffer(GL_BACK_LEFT);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (stereo) {
		glDrawBuffer(GL_BACK_RIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    if (stereo) {
		/* Derive the two eye positions */
		CROSSPROD(camera.vd, camera.vu, r);
		Normalise(&r);
		r.x *= camera.eyesep / 2.0;
		r.y *= camera.eyesep / 2.0;
		r.z *= camera.eyesep / 2.0;

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2 - 0.5 * camera.eyesep * ndfl;
		right = ratio * wd2 - 0.5 * camera.eyesep * ndfl;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near, far);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_RIGHT);
		glLoadIdentity();
		gluLookAt(camera.vp.x + r.x, camera.vp.y + r.y, camera.vp.z + r.z,
				camera.vp.x + r.x + camera.vd.x,
				camera.vp.y + r.y + camera.vd.y,
				camera.vp.z + r.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2 + 0.5 * camera.eyesep * ndfl;
		right = ratio * wd2 + 0.5 * camera.eyesep * ndfl;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near, far);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_LEFT);
		glLoadIdentity();
		gluLookAt(camera.vp.x - r.x, camera.vp.y - r.y, camera.vp.z - r.z,
				camera.vp.x - r.x + camera.vd.x,
				camera.vp.y - r.y + camera.vd.y,
				camera.vp.z - r.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();

    } else {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		left = -ratio * wd2;
		right = ratio * wd2;
		top = wd2;
		bottom = -wd2;
		glFrustum(left, right, bottom, top, near, far);

		glMatrixMode(GL_MODELVIEW);
		glDrawBuffer(GL_BACK_LEFT);
		glLoadIdentity();
		gluLookAt(camera.vp.x, camera.vp.y, camera.vp.z,
				camera.vp.x + camera.vd.x,
				camera.vp.y + camera.vd.y,
				camera.vp.z + camera.vd.z,
				camera.vu.x, camera.vu.y, camera.vu.z);
		do_draw();
    }

    //glutSwapBuffers();
}

/* new window size or exposure */
static void
reshape(int width, int height)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, (GLsizei) width, (GLsizei) height);
    camera.screenwidth = width;
    camera.screenheight = height;
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
   glutCreateWindow ( "stereo rendering - P. Bourke's correct method" );
   if ( fullscreen ) {
	   glutFullScreen();
   }
   initCamera();
   glutReshapeFunc(reshape);
   glutDisplayFunc(draw);
   glutKeyboardFunc(keyFunc);
   glutMainLoop() ;

   return 0;
}

