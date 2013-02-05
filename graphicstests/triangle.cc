// Shamelessly stolen from
// http://stackoverflow.com/questions/2350167
// /basic-skeleton-for-a-c-opengl-program-on-osx.
// Not my work!
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>

// Called to initialize state, before main loop.
void init () {
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glEnable (GL_DEPTH_TEST);
  glShadeModel (GL_SMOOTH);
}

// Called at start of program, after glutPostDisplay(), and during idle to
// display a frame
void display () {
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity ();

  glColor3f (1.0f, 0.85f, 0.35f);
  glBegin (GL_TRIANGLES);
  glVertex3f (0.0, 0.6, -3.0);
  glVertex3f (-0.2, -0.3, -5.0);
  glVertex3f (0.2, -0.3, -5.0);
  glEnd ();

  glutSwapBuffers ();
}

// Called upon resizing of the window
void reshape (int w, int h) {
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity ();
  glFrustum(-0.1, 0.1, -float(h)/(10.0*float(w)), float(h)/(10.0*float(w)), 0.2,
            9999999.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

int main(int argc, char **argv)
{
  glutInit(&argc, argv); // Initializes glut

  // Sets up a double buffer with RGBA components and a depth component
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA);

  // Sets the window size to 512*512 square pixels
  glutInitWindowSize(512, 512);

  // Sets the window position to the upper left
  glutInitWindowPosition(0, 0);

  // Creates a window using internal glut functionality
  glutCreateWindow("Hello!");

  // passes reshape and display functions to the OpenGL machine for callback
  glutReshapeFunc(reshape);
  glutDisplayFunc(display);
  glutIdleFunc(display);

  init();

  // Starts the program.
  glutMainLoop();
  return 0;
}
