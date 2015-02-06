// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNativeGlut.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNativeGlut.h"
#include "../pxTimer.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define GLUT_PX_CORE_FPS 30

glutDisplay* displayRef::mDisplay  = NULL;
int          displayRef::mRefCount = 0;

vector<pxWindowNative*> pxWindowNative::mWindowVector;

bool    pxWindowNative::mEventLoopTimerStarted = false;
float   pxWindowNative::mEventLoopInterval = 1000.0 / (float)GLUT_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;

//start glut callbacks

static void reshape(int width, int height)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onSize(width, height);
  }
}

void display()
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }
  glutSwapBuffers();
}

void onTimer(int v)
{
  display();
  // schedule next timer event
  glutTimerFunc(16, onTimer, 0);
}

void onMouse(int button, int state, int x, int y)
{
  unsigned long flags;
  switch(button)
  {
    case GLUT_MIDDLE_BUTTON: flags = PX_MIDDLEBUTTON;
      break;
    case GLUT_RIGHT_BUTTON:  flags = PX_RIGHTBUTTON;
      break;
    default: flags = PX_LEFTBUTTON;
      break;
  }

  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    if (state == GLUT_DOWN)
    {
      w->onMouseDown(x, y, flags);
    }
    else
    {
      w->onMouseUp(x, y, flags);
    }
  }
}

void onMouseMotion(int x, int y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onMousePassiveMotion(int x, int y)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->onMouseMove(x, y);
  }
}

void onKeyboard(unsigned char key, int x, int y) 
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    // JR Did I mention Glut keyboard support is not very good
    w->onKeyDown(key, 0);
    w->onKeyUp(key, 0);
  }
}

//end glut callbacks

displayRef::displayRef()
{
  if (mRefCount == 0)
  {
    mRefCount++;
    createGlutDisplay();
  }
  else
  {
    mRefCount++;
  }
}

displayRef::~displayRef()
{
  mRefCount--;

  if (mRefCount == 0)
  {
    cleanupGlutDisplay();
  }
}

glutDisplay* displayRef::getDisplay() const
{
  return mDisplay;
}

pxError displayRef::createGlutDisplay()
{
  if (mDisplay == NULL)
  {
    mDisplay = new glutDisplay();
  }

  int argc = 0;
  char **argv = NULL;

  glutInit(&argc, argv);

  //callbacks
  // XXX: These have no affect glutGetWindow() == 0
#if 0
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(onMouse);
  glutMotionFunc(onMouseMotion);
  glutPassiveMotionFunc(onMousePassiveMotion);
  glutKeyboardFunc(onKeyboard);
#endif    
  return PX_OK;
}

void displayRef::cleanupGlutDisplay()
{

  if (mDisplay != NULL)
  {
    delete mDisplay;
  }
  mDisplay = NULL;
}

bool exitFlag = false;

pxWindowNative::~pxWindowNative()
{
  cleanupGlutWindow();
}

void pxWindowNative::createGlutWindow(int left, int top, int width, int height)
{
  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA  /*| GLUT_DEPTH | GLUT_ALPHA*/);
  glutInitWindowPosition (left, top);
  glutInitWindowSize (width, height);

  mGlutWindowId = glutCreateWindow ("pxWindow");
}

void pxWindowNative::cleanupGlutWindow()
{
  glutDestroyWindow(mGlutWindowId);
}

pxError pxWindow::init(int left, int top, int width, int height)
{
  glutDisplay* display = mDisplayRef.getDisplay();

  if (display == NULL)
  {
    cout << "Error initializing display\n" << endl;
    return PX_FAIL;
  }
  else
  {
    mLastWidth  = width;
    mLastHeight = height;
    mResizeFlag = true;

    createGlutWindow(left,top,width,height);

    // XXX: Need to register callbacks after window is created^M
    glutReshapeFunc(reshape);
    //    glutDisplayFunc(display);
    //    glutReshapeFunc(reshape);

    glutMouseFunc(onMouse);
    glutMotionFunc(onMouseMotion);
    glutPassiveMotionFunc(onMousePassiveMotion);
    glutKeyboardFunc(onKeyboard);

    registerWindow(this);
    this->onCreate();

    static bool glewInitialized = false;

    if (!glewInitialized)
    {
      GLenum err = glewInit();

      if (err != GLEW_OK)
      {
        cout << "error with glewInit()\n";
        exit(1); // or handle the error in a nicer way
      }
      glewInitialized = true;
    }
  }
  return PX_OK;
}

pxError pxWindow::term()
{
  return PX_OK;
}

void pxWindow::invalidateRect(pxRect *r)
{
  invalidateRectInternal(r);
}

// This can be improved by collecting the dirty regions and painting
// when the event loop goes idle
void pxWindowNative::invalidateRectInternal(pxRect *r)
{
  drawFrame();
}

bool pxWindow::visibility()
{
  return mVisible;
}

void pxWindow::setVisibility(bool visible)
{
  mVisible = visible;

  if (mVisible)
  {
    glutShowWindow();
  }
  else
  {
    glutHideWindow();
  }
}

pxError pxWindow::setAnimationFPS(long fps)
{
  mTimerFPS = fps;
  mLastAnimationTime = pxMilliseconds();

  return PX_OK;
}

void pxWindow::setTitle(char* title)
{
  glutSetWindowTitle(title);
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
  //TODO

  return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
  //TODO

  return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
  if (mTimerFPS)
  {
    onAnimationTimer();
  }
}

void pxWindowNative::runEventLoop()
{
  exitFlag = false;

  glutTimerFunc(32, onTimer, 0);
  glutMainLoop();
}

void pxWindowNative::exitEventLoop()
{
  exitFlag = true;
}

void pxWindowNative::animateAndRender()
{
  static double lastAnimationTime = pxMilliseconds();
  double currentAnimationTime = pxMilliseconds();
  drawFrame();

  double animationDelta = currentAnimationTime - lastAnimationTime;

  if (mResizeFlag)
  {
    mResizeFlag = false;
    onSize(mLastWidth, mLastHeight);
    invalidateRectInternal(NULL);
  }

  if (mTimerFPS)
  {
    animationDelta = currentAnimationTime - getLastAnimationTime();

    if (animationDelta > (1000/mTimerFPS))
    {
      onAnimationTimerInternal();
      setLastAnimationTime(currentAnimationTime);
    }
  }
}

void pxWindowNative::setLastAnimationTime(double time)
{
  mLastAnimationTime = time;
}

double pxWindowNative::getLastAnimationTime()
{
  return mLastAnimationTime;
}

void pxWindowNative::drawFrame()
{
  pxSurfaceNativeDesc d;
  d.windowWidth  = mLastWidth;
  d.windowHeight = mLastHeight;

  onDraw(&d);
}

void pxWindowNative::registerWindow(pxWindowNative* p)
{
  mWindowVector.push_back(p);
}

void pxWindowNative::unregisterWindow(pxWindowNative* p)
{
  vector<pxWindowNative*>::iterator i;

  for (i = mWindowVector.begin(); i < mWindowVector.end(); i++)
  {
    if ((*i) == p)
    {
      mWindowVector.erase(i);
      return;
    }
  }
}
