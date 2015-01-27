// pxCore CopyRight 2007-2015 John Robinson
// main.cpp
// GLUT driver

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <fstream>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif

#include "rtLog.h"
#include "rtRefT.h"
#include "rtPathUtils.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxText.h"

#include "testScene.h"

extern rtRefT<pxScene2d> gScene;

static int win = 0;

/* new window size or exposure */
static void reshape(int width, int height)
{
  gScene->onSize(width, height);
}

void display() {
  gScene->onDraw();
  glutSwapBuffers();
}

void onTimer(int v) {
  display();
  // schedule next timer event
  glutTimerFunc(16, onTimer, 0);
}

void onMouse(int button, int state, int x, int y) {
  printf("mouse, ");
  switch(button) {
  case GLUT_LEFT_BUTTON:
    printf("left, ");
    break;
  case GLUT_MIDDLE_BUTTON:
    printf("middle, ");
    break;
  case GLUT_RIGHT_BUTTON:
    printf("right, ");
    break;
  default:
    printf("unknown button, ");
  }
  switch(state) {
  case GLUT_UP:
    printf("up, ");
    gScene->onMouseUp(x, y, 0);
    break;
  case GLUT_DOWN:
    printf("down, ");
    gScene->onMouseDown(x, y, 0);
    break;
  default:
    printf("unknown state, ");
    break;
  }
  printf("x: %d, ", x);
  printf("y: %d, ", y);
  printf("\n");
}

void onMouseMotion(int x, int y) {
  //  printf("onMouseMotion x: %d y: %d\n", x, y);
  gScene->onMouseMove(x,y);
}

void onMousePassiveMotion(int x, int y) {
  //printf("onMousePassiveMotion x: %d y: %d\n", x, y);
  gScene->onMouseMove(x, y);
}

void createGlutWindow() {
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA  /*| GLUT_DEPTH | GLUT_ALPHA*/);
  glutInitWindowPosition (10, 10);
  glutInitWindowSize (1280, 720);
  win = glutCreateWindow ("GLUT");
}

void createGlutCallbacks() {
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(onMouse);
  glutMotionFunc(onMouseMotion);
  glutPassiveMotionFunc(onMousePassiveMotion);
}


void exitGlut() {
  glutDestroyWindow(win);
}

int main (int argc, char **argv) {

  glutInit(&argc, argv);
  createGlutWindow();
  createGlutCallbacks();
  
  testScene();
  
  glutTimerFunc(32, onTimer, 0);  
  glutMainLoop();
  
  exitGlut();
  return 0;
}



