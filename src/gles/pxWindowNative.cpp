#include "pxCore.h"
#include "pxWindowNative.h"
#include "../pxCore.h"
#include "../pxWindow.h"
#include "../pxWindowUtil.h"
#include "../pxTimer.h"
#include "../rtMutex.h"

#include <dlfcn.h>
#include <errno.h>
#include <algorithm>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <pthread.h>

using namespace std;

#ifndef NB_ENABLE
#include <termios.h>
#define NON_BLOCKING_ENABLED (0)
#define NON_BLOCKING_DISABLED (1)
#define NB_ENABLE NON_BLOCKING_ENABLED
#define NB_DISABLE NON_BLOCKING_DISABLED
#define nonblock setBlockingMode
#endif

// TODO figure out what to do with rtLog
#if 0
#include "rtLog.h"
#else
#define rtLogWarn printf
#define rtLogError printf
#define rtLogFatal printf
#define rtLogInfo printf
#endif

#define EGL_PX_CORE_FPS 30

bool pxWindowNative::mEventLoopTimerStarted = false;
float pxWindowNative::mEventLoopInterval = 1000.0 / (float)EGL_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;

typedef std::vector<pxWindowNative *> window_vector_t;
static window_vector_t sWindowVector;

rtMutex keyAndMouseMutex;
vector <pxKeyEvent> keyEvents;
vector <pxMouseEvent> mouseEvents;

static void registerWindow(pxWindowNative* win)
{
  sWindowVector.push_back(win);
}

static void unregisterWindow(pxWindowNative* win)
{
  window_vector_t::iterator i = std::find(sWindowVector.begin(), sWindowVector.end(), win);
  if (i != sWindowVector.end())
    sWindowVector.erase(i);
}

pxEGLProvider* pxCreateEGLProvider();
void pxDestroyEGLProvider(pxEGLProvider* provider);

bool exitFlag = false;

static const char* kEGLProviderName = "RT_EGL_PROVIDER";
static const char* kEGLProviderCreate = "pxCreateEGLProvider";
static const char* kEGLProviderDestroy = "pxDestroyEGLProvider";

typedef pxEGLProvider* (*EGLProviderFunction)();
typedef void (*EGLProviderDestroyFunction)(pxEGLProvider *);

static EGLProviderFunction createEGLProvider = NULL;
static EGLProviderDestroyFunction destroyEGLProvider = NULL;

pxEGLProvider* pxCreateEGLProvider();
void pxDestroyEGLProvider(pxEGLProvider* provider);

static void* findSymbol(const char* libname, const char* function)
{
  // TODO: spiff up error handling. Maybe try to open file, etc to provider more
  // accurate error like file_not_found, permission, etc. This will definitely
  // be something people fight with
  void* lib = dlopen(libname, RTLD_NOW);
  if (!lib)
    rtLogError("failed to find %s", libname);

  void* func = dlsym(lib, function);

  dlclose(lib);
  return func;
}

static pxEGLProvider* createPlatformEGLProvider()
{
  if (!createEGLProvider)
  {
    const char* name = getenv(kEGLProviderName);
    if (!name)
      rtLogError("%s unset. Please set like %s=libprovider.so", kEGLProviderName,
          kEGLProviderName);

    createEGLProvider = (EGLProviderFunction) findSymbol(name, kEGLProviderCreate);

    if (!createEGLProvider)
      createEGLProvider = &pxCreateEGLProvider;

    if (!createEGLProvider)
      rtLogFatal("failed to find symbol: %s in: %s", kEGLProviderCreate, name);
  }

  return createEGLProvider();
}

static void destroyPlatformEGLProvider(pxEGLProvider* provider)
{
  if (!destroyEGLProvider)
  {
    const char* name = getenv(kEGLProviderName);
    if (!name)
      rtLogError("%s unset. Please set like %s=libprovider.so", kEGLProviderName,
        kEGLProviderName);

    destroyEGLProvider = (EGLProviderDestroyFunction) findSymbol(name, kEGLProviderDestroy);

    if (!destroyEGLProvider)
      destroyEGLProvider = &pxDestroyEGLProvider;

    if (!destroyEGLProvider)
      rtLogWarn("failed to find symbol: %s in %s", kEGLProviderDestroy, name);
  }

  return destroyEGLProvider(provider);
}

#if 0
static EGLConfig chooseEGLConfig(EGLDisplay display)
{
  EGLint configCount = 0;
  if (!eglGetConfigs(display, 0, 0, &configCount) == EGL_TRUE)
    rtLogError("failed to get EGL configuration count");

  typedef std::vector<EGLConfig> egl_config_list_t;

  egl_config_list_t conf;
  conf.resize(configCount);

  if (!eglGetConfigs(display, &conf[0], configCount, &configCount))
    rtLogError("failed to get EGL configuration list");

  int chosenConfig = 0;
  for (int i = 0; i < static_cast<int>(conf.size()); ++i)
  {
    EGLint depthRed;
    EGLint depthBlue;
    EGLint depthGreen;
    EGLint depthAlpha;

    const EGLConfig& c = conf[i];
    
    if (!eglGetConfigAttrib(display, c, EGL_RED_SIZE, &depthRed))
      rtLogError("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_GREEN_SIZE, &depthGreen))
      rtLogError("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthBlue))
      rtLogError("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthAlpha))
      rtLogInfo("failed to get depth of red");

    rtLogInfo("egl config[%d]: rgba(%d, %d, %d, %d)", i, depthRed, depthGreen, depthBlue,
        depthAlpha);

    if (depthRed == 8 && depthGreen == 8 && depthBlue == 8 && depthAlpha == 8)
    {
      rtLogError("choosing %d of %d EGL configurations", i, static_cast<int>(conf.size()));
      chosenConfig = i;
      break;
    }
  }

  return conf[chosenConfig];
}
#endif

static void onWindowTimerFired(int /*sig*/, siginfo_t* /*si*/, void* /*uc*/)
{
  for (window_vector_t::iterator i = sWindowVector.begin(); i != sWindowVector.end(); ++i)
  {
    pxWindowNative* win = (*i);
    win->animateAndRender();
  }
}

static void setBlockingMode(int blockingState )  
{  
   struct termios ttystate;
   int mask, bits;  
 
   mask= (blockingState == NON_BLOCKING_ENABLED) ? ~(ICANON|ECHO) : -1;
   bits= (blockingState == NON_BLOCKING_ENABLED) ? 0 : (ICANON|ECHO);

   // Obtain the current terminal state and alter the attributes to achieve 
   // the requested blocking behaviour
   tcgetattr(STDIN_FILENO, &ttystate);  

   ttystate.c_lflag= ((ttystate.c_lflag & mask) | bits);  
 
   if (blockingState == NON_BLOCKING_ENABLED)  
   {  
       ttystate.c_cc[VMIN]= 1;  
   }  

   tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);   
}

int kbhit()
{
  struct timeval tv;
  fd_set fds;
  tv.tv_sec = 0;
  tv.tv_usec = 0;
  FD_ZERO(&fds);
  FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
  select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
  return FD_ISSET(STDIN_FILENO, &fds);
}


pxWindowNative::pxWindowNative()
  : mTimerFPS(0)
  , mLastWidth(0)
  , mLastHeight(0)
  , mResizeFlag(false)
  , mLastAnimationTime(0)
  , mVisible(false)
{
  mEGLProvider = createPlatformEGLProvider();
  if (!mEGLProvider)
    rtLogFatal("failed to find EGL provider");

  mInputProvider = pxInputDeviceEventProvider::createDefaultProvider();
  mInputProvider->addMouseListener(&mouseEventListener, this);
  mInputProvider->addKeyListener(&keyEventListener, this);
  mInputProvider->init();

  nonblock(NB_ENABLE);

  // TODO: rtThreadCreate
  pthread_create(&mInputEventThread, NULL, &pxWindowNative::dispatchInput, this);
}

pxWindowNative::~pxWindowNative()
{
  stopAndDeleteEventLoopTimer();
  unregisterWindow(this);

  destroyPlatformEGLProvider(mEGLProvider);
  nonblock(NB_DISABLE);
}

pxError pxWindow::init(int /*left*/, int /*top*/, int width, int height)
{
  mLastWidth = width;
  mLastHeight = height;
  mResizeFlag = true;

  mEGLProvider->initWithDefaults(width, height);
  
  registerWindow(this);
  this->onCreate();
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
  (void)r;
  //rendering for egl is now handled inside of onWindowTimerFired()
  drawFrame();
}

bool pxWindow::visibility()
{
  return mVisible;
}

void pxWindow::setVisibility(bool visible)
{
  //todo - hide the window
  mVisible = visible;
}

pxError pxWindow::setAnimationFPS(uint32_t fps)
{
  mTimerFPS = fps;
  mLastAnimationTime = pxMilliseconds();
  return PX_OK;
}

void pxWindow::setTitle(const char* /*title*/)
{
  //todo
}

pxError pxWindow::beginNativeDrawing(pxSurfaceNative& s)
{
  (void)s;
  //todo

  return PX_OK;
}

pxError pxWindow::endNativeDrawing(pxSurfaceNative& s)
{
  (void)s;
  //todo

  return PX_OK;
}

// pxWindowNative

void pxWindowNative::onAnimationTimerInternal()
{
  if (mTimerFPS) onAnimationTimer();
  // TODO HACK
  /*if (kbhit())
  {
    char c = fgetc(stdin);
    if (!iscntrl(c))
      onChar(c);
  }*/
}

int pxWindowNative::createAndStartEventLoopTimer(int timeoutInMilliseconds )
{
  struct sigevent         te;
  struct itimerspec       its;
  struct sigaction        sa;
  int                     sigNo = SIGRTMIN;

  if (mEventLoopTimerStarted)
  {
      stopAndDeleteEventLoopTimer();
  }

  rtLogInfo("starting event loop timer with delay: %d", timeoutInMilliseconds);

  //Set up signal handler
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = onWindowTimerFired;
  sigemptyset(&sa.sa_mask);
  if (sigaction(sigNo, &sa, NULL) == -1)
  {
    rtLogError("Unable to setup signal handling for timer: %d", errno);
    return(-1);
  }

  //Set and enable alarm
  te.sigev_notify = SIGEV_SIGNAL;
  te.sigev_signo = sigNo;
  te.sigev_value.sival_ptr = NULL;
  timer_create(CLOCK_REALTIME, &te, &mRenderTimerId);

  its.it_value.tv_sec = 0;
  its.it_value.tv_nsec = timeoutInMilliseconds * 1000000;
  its.it_interval = its.it_value;
  timer_settime(mRenderTimerId, 0, &its, NULL);

  mEventLoopTimerStarted = true;

  return(0);
}

int pxWindowNative::stopAndDeleteEventLoopTimer()
{
  int returnValue = 0;
  if (mEventLoopTimerStarted)
  {
    returnValue = timer_delete(mRenderTimerId);
  }
  mEventLoopTimerStarted = false;
  return returnValue;
}

void pxWindowNative::runEventLoopOnce()
{
  for (window_vector_t::iterator i = sWindowVector.begin(); i != sWindowVector.end(); ++i)
  {
    pxWindowNative* win = (*i);
    win->animateAndRender();
  }

  // TODO: Why are we sleeping? 
  usleep(1000); //TODO - find out why pxSleepMS causes a crash on xi3
}

void pxWindowNative::runEventLoop()
{
  exitFlag = false;

  //createAndStartEventLoopTimer((int)mEventLoopInterval);

  while(!exitFlag)
  {
    for (window_vector_t::iterator i = sWindowVector.begin(); i != sWindowVector.end(); ++i)
      (*i)->animateAndRender();

    // TODO: Why are we sleeping here? 
    usleep(1000); //TODO - find out why pxSleepMS causes a crash on xi3
    //pxSleepMS(32); // Breath
  }
}

void pxWindowNative::exitEventLoop()
{
  exitFlag = true;
}

void pxWindowNative::animateAndRender()
{
  keyAndMouseMutex.lock();
  for (size_t i = 0; i < keyEvents.size(); i++)
  {
    pxKeyEvent evt = keyEvents.at(i);
    uint32_t keycode = keycodeFromNative(evt.code);
    if (evt.state == pxKeyStatePressed)
      onKeyDown(keycode, evt.modifiers);
  
    if (evt.state == pxKeyStatePressed || evt.state == pxKeyStateRepeat)
    {
      uint32_t ascii = keycodeToAscii(keycode, evt.modifiers);
      if (!iscntrl(ascii))
        onChar(ascii);
    }

    if (evt.state == pxKeyStateRelease)
      onKeyUp(keycode, evt.modifiers);
  }

  for (size_t i = 0; i < mouseEvents.size(); i++)
  {
    pxMouseEvent evt = mouseEvents.at(i);
    if (evt.type == pxMouseEventTypeMove)
    {
      onMouseMove(evt.move.x, evt.move.y);
    }
    else if (evt.type == pxMouseEventTypeButton)
    {
      if (evt.button.state == pxKeyStatePressed)
        onMouseDown(evt.button.x, evt.button.y, evt.modifiers);
      else if (evt.button.state == pxKeyStateRelease)
        onMouseUp(evt.button.x, evt.button.y, evt.modifiers);
    }
  }
  keyEvents.clear();
  mouseEvents.clear();

  keyAndMouseMutex.unlock();
  static double lastAnimationTime = pxMilliseconds();
  double currentAnimationTime = pxMilliseconds();
  //drawFrame(); 

  double animationDelta = currentAnimationTime-lastAnimationTime;
  if (mResizeFlag)
  {
    mResizeFlag = false;
    mInputProvider->setMouseBounds(rtPoint<int>(0, 0), rtPoint<int>(mLastWidth, mLastHeight));
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
  else printf("mTimerFPS is 0\n");
}

void pxWindowNative::drawFrame()
{
  pxSurfaceNativeDesc d;
  d.windowWidth = mLastWidth;
  d.windowHeight = mLastHeight;

  onDraw(&d);

  eglSwapBuffers(eglGetCurrentDisplay(), eglGetCurrentSurface(EGL_READ));
}

void pxWindowNative::setLastAnimationTime(double time)
{
  mLastAnimationTime = time;
}

double pxWindowNative::getLastAnimationTime()
{
  return mLastAnimationTime;
}

void pxWindowNative::keyEventListener(const pxKeyEvent& evt, void* /* argp */)
{
  keyAndMouseMutex.lock();
  keyEvents.push_back(evt);
  keyAndMouseMutex.unlock();
}

void pxWindowNative::mouseEventListener(const pxMouseEvent& evt, void* /* argp */)
{
  keyAndMouseMutex.lock();
  mouseEvents.push_back(evt);
  keyAndMouseMutex.unlock();
}

void* pxWindowNative::dispatchInput(void* argp)
{
  pxWindowNative* p = reinterpret_cast<pxWindowNative *>(argp);
  p->dispatchInputEvents();
  return 0;
}

void pxWindowNative::dispatchInputEvents()
{
  // TODO: proper shutdown
  while (true)
  {
    mInputProvider->next(1000);
  }
}


