// pxCore CopyRight 2005-2006 John Robinson
// Portable Framebuffer and Windowing Library
// pxWindowNativeGlut.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNativeGlut.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"

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

int getRawNativeKeycodeFromGlut(int key, int modifiers);

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
  glutTimerFunc(10, onTimer, 0);
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
  
  char unmodifiedKey = (char)key;
  
  key = getRawNativeKeycodeFromGlut((int)key, glutGetModifiers());
  
  unsigned long flags = 0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_SHIFT)?PX_MOD_SHIFT:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_CTRL)?PX_MOD_CONTROL:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_ALT)?PX_MOD_ALT:0;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    // JR Did I mention Glut keyboard support is not very good
    w->onChar(unmodifiedKey);
    w->onKeyDown(keycodeFromNative((int)key), flags);
    w->onKeyUp(keycodeFromNative((int)key), flags);
  }
}

void onKeyboardSpecial(int key, int x, int y)
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
      printf("unknown special key: %d\n",key);
  }
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    // JR Did I mention Glut keyboard support is not very good
    w->onKeyDown(keycodeFromNative((int)keycode), 0);
    w->onKeyUp(keycodeFromNative((int)keycode), 0);
  }
}


void onEntry(int state)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    if (state == GLUT_LEFT)
      w->onMouseLeave();
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
  glutSpecialFunc(onKeyboardSpecial);
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
    glutSpecialFunc(onKeyboardSpecial);
    glutEntryFunc(onEntry);

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

int getRawNativeKeycodeFromGlut(int key, int modifiers)
{
  //glut doesn't support keycodes. we do some mappings here
  key = tolower(key);
  
  //bool shiftPressed = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);
  bool ctrlPressed = (glutGetModifiers() & GLUT_ACTIVE_CTRL);
  //bool altPressed = (glutGetModifiers() & GLUT_ACTIVE_ALT);
  
  int rawKeycode = key;
  
  if (ctrlPressed)
  {
    switch (rawKeycode)
    {
      case 1:
        rawKeycode = 'a';
        break;
      case 2:
        rawKeycode = 'b';
        break;
      case 3:
        rawKeycode = 'c';
        break;
      case 4:
        rawKeycode = 'd';
        break;
      case 5:
        rawKeycode = 'e';
        break;
      case 6:
        rawKeycode = 'f';
        break;
      case 7:
        rawKeycode = 'g';
        break;
      case 8:
        rawKeycode = 'h';
        break;
      case 9:
        rawKeycode = 'i';
        break;
      case 10:
        rawKeycode = 'j';
        break;
      case 11:
        rawKeycode = 'k';
        break;
      case 12:
        rawKeycode = 'l';
        break;
      case 13:
        rawKeycode = 'm';
        break;
      case 14:
        rawKeycode = 'n';
        break;
      case 15:
        rawKeycode = 'o';
        break;
      case 16:
        rawKeycode = 'p';
        break;
      case 17:
        rawKeycode = 'q';
        break;
      case 18:
        rawKeycode = 'r';
        break;
      case 19:
        rawKeycode = 's';
        break;
      case 20:
        rawKeycode = 't';
        break;
      case 21:
        rawKeycode = 'u';
        break;
      case 22:
        rawKeycode = 'v';
        break;
      case 23:
        rawKeycode = 'w';
        break;
      case 24:
        rawKeycode = 'x';
        break;
      case 25:
        rawKeycode = 'y';
        break;
      case 26:
        rawKeycode = 'z';
        break;
    }
  }
  
  return rawKeycode;
}
