// pxCore CopyRight 2007-2015 John Robinson
// main.cpp
// GLUT driver

#ifdef WIN32
#include <windows.h>
#endif

#include <stdlib.h>
#include <iostream>
#include <fstream>

#if 1
#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
//#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif
#endif

#include "rtLog.h"
#include "rtRefT.h"
#include "rtPathUtils.h"
#include "pxScene2d.h"
#include "pxImage.h"
#include "pxText.h"
#include "pxText2.h"
#include "pxWindowUtil.h"

#include "testScene.h"

extern pxScene2dRef scene;

static int win = 0;

/* new window size or exposure */
static void reshape(int width, int height)
{
  scene->onSize(width, height);
}

void display() {
  scene->onDraw();
  glutSwapBuffers();
}

void onTimer(int /*v*/) {
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
    scene->onMouseUp(x, y, 0);
    break;
  case GLUT_DOWN:
    printf("down, ");
    scene->onMouseDown(x, y, 0);
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
  scene->onMouseMove(x,y);
}

void onMousePassiveMotion(int x, int y) {
  //printf("onMousePassiveMotion x: %d y: %d\n", x, y);
  scene->onMouseMove(x, y);
}

void onKeyboard(unsigned char key, int /*x*/, int /*y*/)
{
  // Ugh GLUT key support not so hot.. 
  char unmodifiedKey = (char)key;
  key = tolower(key);
  unsigned long flags = 0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_SHIFT)?8:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_CTRL)?16:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_ALT)?32:0;
  scene->onKeyDown(keycodeFromNative((int)key), flags);
  scene->onKeyUp(keycodeFromNative((int)key), flags);
  scene->onChar(unmodifiedKey);
}

void onSpecial(int key, int /*x*/, int /*y*/)
{
  int keycode = key;
  switch (key)
  {
    case GLUT_KEY_F1:
      keycode = PX_KEY_NATIVE_F1;
      break;
    case GLUT_KEY_F2:
      keycode = PX_KEY_NATIVE_F2;
      break;
    case GLUT_KEY_F3:
      keycode = PX_KEY_NATIVE_F3;
      break;
    case GLUT_KEY_F4:
      keycode = PX_KEY_NATIVE_F4;
      break;
    case GLUT_KEY_F5:
      keycode = PX_KEY_NATIVE_F5;
      break;
    case GLUT_KEY_F6:
      keycode = PX_KEY_NATIVE_F6;
      break;
    case GLUT_KEY_F7:
      keycode = PX_KEY_NATIVE_F7;
      break;
    case GLUT_KEY_F8:
      keycode = PX_KEY_NATIVE_F8;
      break;
    case GLUT_KEY_F9:
      keycode = PX_KEY_NATIVE_F9;
      break;
    case GLUT_KEY_F10:
      keycode = PX_KEY_NATIVE_F10;
      break;
    case GLUT_KEY_F11:
      keycode = PX_KEY_NATIVE_F11;
      break;
    case GLUT_KEY_F12:
      keycode = PX_KEY_NATIVE_F12;
      break;
    case GLUT_KEY_LEFT:
      keycode = PX_KEY_NATIVE_LEFT;
      break;
    case GLUT_KEY_UP:
      keycode = PX_KEY_NATIVE_UP;
      break;
    case GLUT_KEY_RIGHT:
      keycode = PX_KEY_NATIVE_RIGHT;
      break;
    case GLUT_KEY_DOWN:
      keycode = PX_KEY_NATIVE_DOWN;
      break;
    case GLUT_KEY_PAGE_UP:
      keycode = PX_KEY_NATIVE_PAGEUP;
      break;
    case GLUT_KEY_PAGE_DOWN:
      keycode = PX_KEY_NATIVE_PAGEDOWN;
      break;
    case GLUT_KEY_HOME:
      keycode = PX_KEY_NATIVE_HOME;
      break;
    case GLUT_KEY_END:
      keycode = PX_KEY_NATIVE_END;
      break;
    case GLUT_KEY_INSERT:
      keycode = PX_KEY_NATIVE_INSERT;
      break;
    default:
      printf("unknown special key (in main): %d\n",key);
  }
  scene->onKeyDown(keycodeFromNative((int)keycode), 0);
  scene->onKeyUp(keycodeFromNative((int)keycode), 0);
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
  glutKeyboardFunc(onKeyboard);
  glutSpecialFunc(onSpecial);
  glutPassiveMotionFunc(onMousePassiveMotion);
}


void exitGlut() {
  glutDestroyWindow(win);
}

int main (int argc, char **argv)
{

  glutInit(&argc, argv);
  createGlutWindow();
  createGlutCallbacks();
  
  testScene();
  
  glutTimerFunc(32, onTimer, 0);  
  glutMainLoop();
  
  exitGlut();
  return 0;
}

