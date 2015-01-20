// pxCore CopyRight 2007-2015 John Robinson
// main.cpp

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

pxScene2d scene;

static int win = 0;

/* new window size or exposure */
static void reshape(int width, int height)
{
  scene.onSize(width, height);
}

void display() {
  scene.onDraw();
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
    scene.onMouseUp(x, y, 0);
    break;
  case GLUT_DOWN:
    printf("down, ");
    scene.onMouseDown(x, y, 0);
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
  scene.onMouseMove(x,y);
}

void onMousePassiveMotion(int x, int y) {
  //printf("onMousePassiveMotion x: %d y: %d\n", x, y);
  scene.onMouseMove(x, y);
}

void createGlutWindow() {
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA  /*| GLUT_DEPTH | GLUT_ALPHA*/);
  glutInitWindowPosition (10, 10);
  glutInitWindowSize (1280, 720);
  win = glutCreateWindow ("TestGL");
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

void initGL();


#if 0
void setupScene() {

  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/grapes.png");

  pxObjectRef root = scene.getRoot();
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    pxObjectRef p;

    if (i < 2) {
      pxText* t = new pxText();
      p = t;
      char buffer[256];
      sprintf(buffer, "Hello %d", i);
      t->setText(buffer);
    }
    else {
      pxImage* i = new pxImage();
      p = i;
      i->setURL(d.cString());
      p->cx = i->width()/2;
      p->cy = i->height()/2;
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }
    p->x = nx;
    p->y = ny;

    p->setParent(root);
    
    p->r = 45*i;
    p->animateTo("r", 360+(90*i), 2.0, pxInterpLinear, seesaw);
    p->animateTo("x", 800, 2.0, pxInterpLinear, seesaw);
  }
}

#else
void setupScene() {
  rtString d;
  rtGetCurrentDirectory(d);
  d.append("/../images/banana.png");

  pxObjectRef root = scene.getRoot();
  
  int n = 3;
  int nx = 100;
  int ny = 200;
  for (int i = 0; i < n; i++) {
    pxObjectRef p;

    if (i == 0) {
      rtRefT<rectangle9> r = new rectangle9();
      float c1[4] = {1,0,0,1};
      r->setFillColor(c1);
      float c2[4] = {1,1,1,0.5};
      r->setLineColor(c2);
      r->setLineWidth(10);
      r->w = 300;
      r->h = 30;
      p = r;
      p->animateTo("h", 600, 0.2, pxInterpLinear, seesaw);
    }
    else if (i == 1) {
      rtRefT<pxText> t = new pxText();
      t->setText("Hello");
      p = t;
    }
    else {
      pxImage* i = new pxImage();
      p = i;
      i->setURL(d.cString());
      p->cx = i->width()/2;
      p->cy = i->height()/2;
    }

    nx += 10;
    if (nx > 1000) {
      nx = 0;
      ny += 10;
    }
    p->x = nx;
    p->y = ny;

    p->setParent(root);
    
    p->animateTo("r", 360, 2.0, pxInterpLinear, loop);
    p->animateTo("x", 800, 2.0, pxInterpLinear, seesaw);
    p->animateTo("a", 0.5, 2.0, pxInterpLinear, seesaw);
  }
}

#endif

int main (int argc, char **argv) {

  glutInit(&argc, argv);
  createGlutWindow();
  createGlutCallbacks();
  
  setupScene();
  
  initGL();
  
  glutTimerFunc(32, onTimer, 0);  
  glutMainLoop();
  
  exitGlut();
  return 0;
}



