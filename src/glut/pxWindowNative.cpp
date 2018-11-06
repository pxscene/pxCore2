/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

// pxWindowNativeGlut.cpp

#include "../pxCore.h"
#include "../pxWindow.h"
#include "pxWindowNative.h"
#include "../pxTimer.h"
#include "../pxWindowUtil.h"
#include "pxKeycodes.h"

#include "pxConfigNative.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#define GLUT_PX_CORE_FPS 30

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/gl.h>
//#include <GL/glext.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

using namespace std;

#ifndef rtLogWarn
#define rtLogWarn(...) (void)0
#endif

#ifndef rtLogError
#define rtLogError(...) (void)0
#endif

#if 1

unsigned int frame_ms = 16;

GLuint textureId1;
GLuint mTextureName;
static GLint u_resolution = -1;
GLint u_texture = -1;
GLint attr_pos = 0, attr_uv = 2;

static const char *fShaderText =
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
  "precision mediump float;\n"
#endif
  "uniform sampler2D s_texture;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  "  vec4 textureColor = texture2D(s_texture, v_uv);\n"
  "  gl_FragColor = vec4(textureColor.rgb,1);;\n"
  "}\n";

static const char *vShaderText =
  "uniform vec2 u_resolution;\n"
  "attribute vec2 pos;\n"
  "attribute vec2 uv;\n"
  "varying vec2 v_uv;\n"
  "void main() {\n"
  // map from "pixel coordinates"
  " vec4 zeroToOne = vec4(pos,0,1) / vec4(u_resolution, u_resolution.x, 1);\n"
  " vec4 zeroToTwo = zeroToOne * vec4(2.0, 2.0, 1, 1);\n"
  " vec4 clipSpace = zeroToTwo - vec4(1.0, 1.0, 0, 0);\n"
  " gl_Position =  clipSpace * vec4(1, -1, 1, 1);\n"
  "v_uv = uv;\n"
  "}\n"; 

GLuint createShaderProgram(const char* vShaderTxt, const char* fShaderTxt)
{
  GLuint fragShader, vertShader, program = 0;
  GLint stat;
  
  fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragShader, 1, (const char **) &fShaderTxt, NULL);
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    rtLogError("Error: fragment shader did not compile: %d", glGetError());
    
    GLint maxLength = 0;
    glGetShaderiv(fragShader, GL_INFO_LOG_LENGTH, &maxLength);
    
    //The maxLength includes the NULL character
    std::vector<char> errorLog(maxLength);
    glGetShaderInfoLog(fragShader, maxLength, &maxLength, &errorLog[0]);
    
    //Provide the infolog in whatever manor you deem best.
    rtLogWarn("%s", &errorLog[0]);
    //Exit with failure.
    glDeleteShader(fragShader); //Don't leak the shader.

    exit(1);
  }
  
  vertShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertShader, 1, (const char **) &vShaderTxt, NULL);
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);

  if (!stat)
  {
    rtLogError("vertex shader did not compile: %d", glGetError());
    exit(1);
  }
  
  program = glCreateProgram();
  glAttachShader(program, fragShader);
  glAttachShader(program, vertShader);
  glLinkProgram(program);
  
  glGetProgramiv(program, GL_LINK_STATUS, &stat);
  if (!stat)
  {
    char log[1000];
    GLsizei len;
    glGetProgramInfoLog(program, 1000, &len, log);
    rtLogError("faild to link:%s", log);
    exit(1);
  }
  
  /* test setting attrib locations */
  glBindAttribLocation(program, attr_pos, "pos");
  glBindAttribLocation(program, attr_uv, "uv");
  glLinkProgram(program);  /* needed to put attribs into effect */
  
  return program;
}

#if 0
static void drawImageTexture(float x, float y, float w, float h/*, pxBuffer& b*/)
{
  const float verts[4][2] = 
  {

    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };

#if 0
  const float uv[4][2] = {
    { 0, 0 },
    { b.width(), 0 },
    { 0,  b.height() },
    { b.width(), b.height() }
  };
#endif
  
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    //glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    //glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    //glDisableVertexAttribArray(attr_uv);
  }
}
#endif

void doinit()
{
#if 1
#if !defined(__APPLE__) && !defined(PX_PLATFORM_WAYLAND_EGL) && !defined(PX_PLATFORM_GENERIC_EGL)

  GLenum err = glewInit();

  if (err != GLEW_OK)
  {
    rtLogError("failed to initialize glew");
    exit(1); // or handle the error in a nicer way
  }

  if (!GLEW_VERSION_2_1)  // check that the machine supports the 2.1 API.
  {
    rtLogError("invalid glew version");
    exit(1); // or handle the error in a nicer way
  }
#endif
#endif

  glClearColor(0, 0, 0, 1);

  GLuint program = createShaderProgram(vShaderText, fShaderText);

  u_resolution   = glGetUniformLocation(program, "u_resolution");
  u_texture      = glGetUniformLocation(program, "s_texture");

#if 0
#if 1
  // Using for RGBA texture
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId1);
  glBindTexture(GL_TEXTURE_2D, textureId1);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
#if 0
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#endif
#endif
#endif

  glEnable(GL_BLEND);

  // assume non-premultiplied... 
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  

  glUseProgram(program);
}


#endif

glutDisplay* displayRef::mDisplay  = NULL;
int          displayRef::mRefCount = 0;

vector<pxWindowNative*> pxWindowNative::mWindowVector;

//bool    pxWindowNative::mEventLoopTimerStarted = false;
//float   pxWindowNative::mEventLoopInterval = 1000.0 / (float)GLUT_PX_CORE_FPS;
//timer_t pxWindowNative::mRenderTimerId;

uint32_t getRawNativeKeycodeFromGlut(uint32_t key, uint32_t modifiers);

//start glut callbacks


pxWindowNative* pxWindowNative::getWindowFromGlutID(int id)
{
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;

  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    if (w->mGlutWindowId == id)
      return w;
  }
  return NULL;
}

void pxWindowNative::onGlutReshape(int width, int height)
{
  glViewport(0, 0, (GLint)width, (GLint)height);
  glUniform2f(u_resolution, width, height);

  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
    w->onSize(width, height);
}

void pxWindowNative::onGlutDisplay()
{
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
    w->drawFrame();
}

void pxWindowNative::onGlutClose()
{
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    unregisterWindow(w);
    w->onCloseRequest();
  }
}

void pxWindowNative::onGlutTimer(int v)
{

#if 0
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    w->onAnimationTimer();
    glutTimerFunc(/*16*/ frame_ms, onGlutTimer, 0);
  }
#else
  vector<pxWindowNative*> windowVector = pxWindow::getNativeWindows();
  vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);

    glutSetWindow(w->mGlutWindowId);
    w->onAnimationTimer();
  }
  glutTimerFunc(/*16*/ frame_ms, onGlutTimer, 0);
#endif
}

void pxWindowNative::onGlutMouse(int button, int state, int x, int y)
{
  unsigned long flags;

  if (button == GLUT_LEFT_BUTTON || button == GLUT_RIGHT_BUTTON)
  {
    if (button == GLUT_LEFT_BUTTON)
      flags = GLUT_LEFT_BUTTON;
    if (button == GLUT_RIGHT_BUTTON)
      flags = GLUT_RIGHT_BUTTON;

    pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
    if (w)
    {
      w->mMouseDown = (state == GLUT_DOWN);
      if (state == GLUT_DOWN)
        w->onMouseDown(x, y, flags);
      else
      {
        w->onMouseUp(x, y, flags);
        if (!w->mMouseEntered)
        {
          w->onMouseLeave();
        }
      }
    }
  }
  else
  // some GLUT impls uses button 3,4 etc for mouse wheel
  if ((button == 3) || (button == 4)) // It's a wheel event
  {
    int dir = (button == 3) ? 1 : -1;

    pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
    if (w)
      w->onScrollWheel( dir * ((x > 0) ? 1 : -1),
                        dir * ((y > 0) ? 1 : -1) );
  }
}


// The second parameter gives the direction of the scroll. Values of +1 is forward, -1 is backward.

void pxWindowNative::onGlutScrollWheel(int button, int dir, int x, int y)
{
  if ((button == 3) || (button == 4)) // It's a wheel event
  {
    pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
    if (w)
      w->onScrollWheel(dir * x, dir * y);
  }
}

void pxWindowNative::onGlutMouseMotion(int x, int y)
{
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
    w->onMouseMove(x,y);
}

void pxWindowNative::onGlutMousePassiveMotion(int x, int y)
{
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    if (w->mMouseEntered)
      w->onMouseMove(x,y);
  }
}

void pxWindowNative::onGlutKeyboard(unsigned char key, int x, int y) 
{
  uint32_t unmodifiedKey = (uint32_t)key;  
  uint32_t mappedKey = getRawNativeKeycodeFromGlut((uint32_t)key, glutGetModifiers());
  
  uint32_t flags = 0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_SHIFT)?PX_MOD_SHIFT:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_CTRL)?PX_MOD_CONTROL:0;
  flags |= (glutGetModifiers() & GLUT_ACTIVE_ALT)?PX_MOD_ALT:0;

  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    // JR Did I mention Glut keyboard support is not very good
    w->onKeyDown(keycodeFromNative(mappedKey), flags);
    if (!iscntrl(unmodifiedKey))
      w->onChar(unmodifiedKey);
    w->onKeyUp(keycodeFromNative(mappedKey), flags);
  }
}

void pxWindowNative::onGlutEntry(int state)
{
  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    w->mMouseEntered = (state != GLUT_LEFT);
    if (state == GLUT_LEFT)
    {
      if (!w->mMouseDown) // Eat here if dragging
        w->onMouseLeave();
    }
    else
      w->onMouseEnter();
  }
}

void pxWindowNative::onGlutKeyboardSpecial(int key, int x, int y)
{  
  int      keycode = 0;
  uint32_t   flags = 0;

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
    case 112: // LEFT SHIFT  (not defined in freeglut.h) 
      flags |= PX_MOD_SHIFT;
      break;      
    case 113: // RIGHT SHIFT  (not defined in freeglut.h) 
      flags |= PX_MOD_SHIFT;
      break;
    case 114: // CTRL  (not defined in freeglut.h) 
      flags |= PX_MOD_CONTROL;
      break;        
    case 116: // ALT  (not defined in freeglut.h) 
      keycode = PX_KEY_NATIVE_ALT;
      break;        
    default:
      printf("unknown special key: %d\n",key);
  }

  if(flags == 0)
  {
    flags |= (glutGetModifiers() & GLUT_ACTIVE_SHIFT)?PX_MOD_SHIFT:0;
    flags |= (glutGetModifiers() & GLUT_ACTIVE_CTRL)?PX_MOD_CONTROL:0;
    flags |= (glutGetModifiers() & GLUT_ACTIVE_ALT)?PX_MOD_ALT:0;
  }
    
//  printf("\n onGlutKeyboardSpecial() - key = %d (0x%0X)    keycode = % d     flags = %d  PX_KEY_NATIVE_LEFT = %d\n",
//      key, key, keycode, flags, PX_KEY_NATIVE_LEFT);

  pxWindowNative* w = getWindowFromGlutID(glutGetWindow());
  if (w)
  {
    // JR Did I mention Glut keyboard support is not very good
    w->onKeyDown(keycodeFromNative(keycode), flags);
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
}

void pxWindowNative::createGlutWindow(int left, int top, int width, int height)
{

  glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGBA  /*| GLUT_DEPTH | GLUT_ALPHA*/);
  glutInitWindowPosition (left, top);
  glutInitWindowSize (width, height);

  mGlutWindowId = glutCreateWindow ("pxWindow");

  #ifndef __APPLE__
  glutSetOption(GLUT_RENDERING_CONTEXT ,GLUT_USE_CURRENT_CONTEXT );
  #endif

  //doinit();

  glClearColor(0, 0, 0, 1);
}

void pxWindowNative::cleanupGlutWindow()
{
  // NOOP
}

pxError pxWindow::init(int left, int top, int width, int height)
{
  glutDisplay* d = mDisplayRef.getDisplay();

  if (d == NULL)
  {
    cout << "Error initializing display\n" << endl;
    return PX_FAIL;
  }
  else
  {
//    mLastWidth  = width;
//    mLastHeight = height;
//    mResizeFlag = true;

    createGlutWindow(left,top,width,height);

    // XXX: Need to register callbacks after window is created
    glutDisplayFunc(onGlutDisplay);
    glutReshapeFunc(onGlutReshape);
    glutMouseFunc(onGlutMouse);
    glutMotionFunc(onGlutMouseMotion);
    glutMouseWheelFunc(onGlutScrollWheel);
    glutPassiveMotionFunc(onGlutMousePassiveMotion);
    glutKeyboardFunc(onGlutKeyboard);
//#ifdef PX_USE_GLUT_ON_CLOSE
    glutWMCloseFunc(onGlutClose);
//#endif

    glutSpecialFunc(onGlutKeyboardSpecial);
    glutEntryFunc(onGlutEntry);

    registerWindow(this);
    this->onCreate();

    static bool glewInitialized = false;

    if (!glewInitialized)
    {
#ifdef PX_USE_GLEW
      GLenum err = glewInit();

      if (err != GLEW_OK)
      {
        cout << "error with glewInit()\n";
        exit(1); // or handle the error in a nicer way
      }
#endif
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

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
  mTimerFPS = fps;
      
  frame_ms = ((1.0 / (double) mTimerFPS) * 1000.0);

  //mLastAnimationTime = pxMilliseconds();

  return PX_OK;
}

void pxWindow::setTitle(const char* title)
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

#if 0
void pxWindowNative::onAnimationTimerInternal()
{
  if (mTimerFPS)
  {
    onAnimationTimer();
  }
}
#endif

// TODO this should be done better...
static bool gTimerSet = false;

void pxWindowNative::runEventLoop()
{
//glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
  glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

  exitFlag = false;

  if (!gTimerSet)
  {
    glutTimerFunc(32, onGlutTimer, 0);
    gTimerSet = true;
  }
  
  glutMainLoop();
}

void pxWindowNative::runEventLoopOnce()
{
  if (!gTimerSet)
  {
    glutTimerFunc(32, onGlutTimer, 0);
    gTimerSet = true;
  }

  // Can't do this in plain old glut
#ifdef __APPLE__
  glutCheckLoop(); // OSX glut extension
#else
  glutMainLoopEvent(); // freeglut variant
#endif
}

void pxWindowNative::exitEventLoop()
{

  exitFlag = true;
//#ifdef glutLeaveMainLoop
  glutLeaveMainLoop();
//#else
    // exit(0);
//#endif
}

#if 0
void pxWindowNative::animateAndRender()
{
#if 0
  static double lastAnimationTime = pxMilliseconds();
  double currentAnimationTime = pxMilliseconds();
#endif
  drawFrame();

  //  double animationDelta = currentAnimationTime - lastAnimationTime;

#if 0
  if (mResizeFlag)
  {
    mResizeFlag = false;
    onSize(mLastWidth, mLastHeight);
    invalidateRectInternal(NULL);
  }
#endif

#if 0
  if (mTimerFPS)
  {
    animationDelta = currentAnimationTime - getLastAnimationTime();

    if (animationDelta > (1000/mTimerFPS))
    {
      onAnimationTimerInternal();
      setLastAnimationTime(currentAnimationTime);
    }
  }
#endif
}
#endif

#if 0
void pxWindowNative::setLastAnimationTime(double time)
{
  mLastAnimationTime = time;
}

double pxWindowNative::getLastAnimationTime()
{
  return mLastAnimationTime;
}
#endif

void pxWindowNative::drawFrame()
{
#if 1
#if 0
  pxSurfaceNativeDesc d;
  d.windowWidth  = mLastWidth;
  d.windowHeight = mLastHeight;
#endif
  onDraw(this);
  glutSwapBuffers();
#endif
}

void pxWindowNative::blit(pxBuffer& b, int32_t dstLeft, int32_t dstTop, 
                          int32_t dstWidth, int32_t dstHeight,
                          int32_t srcLeft, int32_t srcTop)
{
#if 0
  float x = dstLeft;
  float y = dstTop;
  float w = dstWidth;
  float h = dstHeight;

  const float verts[4][2] = 
  {
    { x,y },
    {  x+w, y },
    {  x,  y+h },
    {  x+w, y+h }
  };

  const float uv[4][2] = {
    { 0, 0 },
    { 1.0, 0 },
    { 0,  1.0 },
    { 1.0, 1.0 }
  };
  
  {
    // leaky

    GLuint mTextureName;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &mTextureName);
    glBindTexture(GL_TEXTURE_2D, mTextureName);    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
                 b.width(), b.height(), 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, b.base());
    glUniform1i(u_texture, 0);
    glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
    glVertexAttribPointer(attr_uv, 2, GL_FLOAT, GL_FALSE, 0, uv);
    glEnableVertexAttribArray(attr_pos);
    glEnableVertexAttribArray(attr_uv);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisableVertexAttribArray(attr_pos);
    glDisableVertexAttribArray(attr_uv);
  }
#endif
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

uint32_t getRawNativeKeycodeFromGlut(uint32_t key, uint32_t modifiers)
{
  //glut doesn't support keycodes. we do some mappings here
  key = tolower(key);
  
  bool shiftPressed = (glutGetModifiers() & GLUT_ACTIVE_SHIFT);
  bool ctrlPressed = (glutGetModifiers() & GLUT_ACTIVE_CTRL);
  //bool altPressed = (glutGetModifiers() & GLUT_ACTIVE_ALT);
  
//  printf("\n###  getRawNativeKeycodeFromGlut() - key = %d    shiftPressed = %d ",key, shiftPressed);

  uint32_t rawKeycode = key;
  
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
  if (shiftPressed)
  {
    switch (rawKeycode)
    {
      case 33:
        rawKeycode = '1';
        break;
      case 64:
        rawKeycode = '2';
        break;
      case 35:
        rawKeycode = '3';
        break;
      case 36:
        rawKeycode = '4';
        break;
      case 37:
        rawKeycode = '5';
        break;
      case 94:
        rawKeycode = '6';
        break;
      case 38:
        rawKeycode = '7';
        break;
      case 42:
        rawKeycode = '8';
        break;
      case 40:
        rawKeycode = '9';
        break;
      case 41:
        rawKeycode = '0';
        break;
      case 95:
        rawKeycode = '-';
        break;
      case 43:
        rawKeycode = '=';
        break;
    }
  }
  
  return rawKeycode;
}
