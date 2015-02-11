#include "pxCore.h"
#include "pxWindowNative.h"
#include "rtLog.h"
#include "../pxCore.h"
#include "../pxWindow.h"
#include "../pxTimer.h"

#include <dlfcn.h>
#include <vector>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <assert.h>

#define EGL_PX_CORE_FPS 30

vector<pxWindowNative*> pxWindowNative::mWindowVector;
bool pxWindowNative::mEventLoopTimerStarted = false;
float pxWindowNative::mEventLoopInterval = 1000.0 / (float)EGL_PX_CORE_FPS;
timer_t pxWindowNative::mRenderTimerId;
pxEGLProvider* pxWindowNative::mEGLProvider = NULL;

bool exitFlag = false;

/*static const char* kEglProviderName = "RT_EGL_PROVIDER";
static const char* kEGLProviderCreate = "pxCreateEGLProvider";
static const char* kEGLProviderDestroy = "pxDestroyEGLProvider";


typedef pxEGLProvider* (*EGLProviderFunction)();
typedef void (*EGLProviderDestroyFunction)(pxEGLProvider *);

static EGLProviderFunction pxCreateEGLProvider = NULL;
static EGLProviderDestroyFunction pxDestroyEGLProvider = NULL;

static void* findSymbol(const char* libname, const char* function)
{
  // TODO: spiff up error handling. Maybe try to open file, etc to provider more
  // accurate error like file_not_found, permission, etc. This will definitely
  // be something people fight with
  void* lib = dlopen(libname, RTLD_NOW);
  if (!lib)
    printf("failed to find %s", libname);

  void* func = dlsym(lib, function);
  if (!func)
    printf("failed to function %s from %s", function, libname);

  dlclose(lib);
  return func;
}

static pxEGLProvider* createPlatformEGLProvider()
{
  if (!pxCreateEGLProvider)
  {
    const char* name = getenv(kEglProviderName);
    if (!name)
      printf("%s unset. Please set like %s=libprovider.so", kEglProviderName,
          kEglProviderName);

    pxCreateEGLProvider = (EGLProviderFunction) findSymbol(name, kEGLProviderCreate);
  }

  return pxCreateEGLProvider();
}

static void destroyPlatformEGLProvider(pxEGLProvider* provider)
{
  if (!pxDestroyEGLProvider)
  {
    const char* name = getenv(kEglProviderName);
    if (!name)
      printf("%s unset. Please set like %s=libprovider.so", kEglProviderName,
        kEglProviderName);

    pxDestroyEGLProvider = (EGLProviderDestroyFunction) findSymbol(name, kEGLProviderDestroy);
  }

  return pxDestroyEGLProvider(provider);
}

static EGLConfig chooseEGLConfig(EGLDisplay display)
{
  EGLint configCount = 0;
  if (!eglGetConfigs(display, 0, 0, &configCount) == EGL_TRUE)
    printf("failed to get EGL configuration count");

  typedef std::vector<EGLConfig> egl_config_list_t;

  egl_config_list_t conf;
  conf.resize(configCount);

  if (!eglGetConfigs(display, &conf[0], configCount, &configCount))
    printf("failed to get EGL configuration list");

  int chosenConfig = 0;
  for (int i = 0; i < static_cast<int>(conf.size()); ++i)
  {
    EGLint depthRed;
    EGLint depthBlue;
    EGLint depthGreen;
    EGLint depthAlpha;

    const EGLConfig& c = conf[i];
    
    if (!eglGetConfigAttrib(display, c, EGL_RED_SIZE, &depthRed))
      printf("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_GREEN_SIZE, &depthGreen))
      printf("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthBlue))
      printf("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthAlpha))
      printf("failed to get depth of red");

    printf("egl config[%d]: rgba(%d, %d, %d, %d)", i, depthRed, depthGreen, depthBlue,
        depthAlpha);

    if (depthRed == 8 && depthGreen == 8 && depthBlue == 8 && depthAlpha == 8)
    {
      printf("choosing %d of %d EGL configurations", i, static_cast<int>(conf.size()));
      chosenConfig = i;
      break;
    }
  }

  return conf[chosenConfig];
}*/

static void onWindowTimerFired( int sig, siginfo_t *si, void *uc )
{
  (void)sig;
  (void)si;
  (void)uc;
  vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
  vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }
}

pxWindowNative::pxWindowNative() : mTimerFPS (0), mLastWidth(0), mLastHeight(0),
        mResizeFlag(false), mLastAnimationTime(0),
        mVisible(false)
{
}

pxWindowNative::~pxWindowNative()
{
  stopAndDeleteEventLoopTimer();
  unregisterWindow(this);
  pxDestroyEGLProvider(mEGLProvider);
}

pxError pxWindow::init(int left, int top, int width, int height)
{
  (void)left;
  (void)top;
  mLastWidth = width;
  mLastHeight = height;
  if (mEGLProvider == NULL)
  {
    mEGLProvider = pxCreateEGLProvider();
    if (mEGLProvider->initWithDefaults(width,height) != PX_OK)
    {
      printf("error creating EGL window\n");
      return PX_FAIL;
    }
  }
  mResizeFlag = true;
  
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
  //drawFrame();
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

pxError pxWindow::setAnimationFPS(long fps)
{
  mTimerFPS = fps;
  mLastAnimationTime = pxMilliseconds();
  return PX_OK;
}

void pxWindow::setTitle(char* title)
{
  (void)title;
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

  //Set up signal handler
  sa.sa_flags = SA_SIGINFO;
  sa.sa_sigaction = onWindowTimerFired;
  sigemptyset(&sa.sa_mask);
  if (sigaction(sigNo, &sa, NULL) == -1)
  {
      fprintf(stderr, "Unable to setup signal handling for timer.\n");
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

void pxWindowNative::runEventLoop()
{
  exitFlag = false;

  //createAndStartEventLoopTimer((int)mEventLoopInterval);

  while(!exitFlag)
  {
    vector<pxWindowNative*> windowVector = pxWindowNative::getNativeWindows();
  vector<pxWindowNative*>::iterator i;
  for (i = windowVector.begin(); i < windowVector.end(); i++)
  {
    pxWindowNative* w = (*i);
    w->animateAndRender();
  }
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
  static double lastAnimationTime = pxMilliseconds();
  double currentAnimationTime = pxMilliseconds();
  drawFrame(); 

  double animationDelta = currentAnimationTime-lastAnimationTime;
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
