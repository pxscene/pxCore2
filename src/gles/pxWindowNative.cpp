#include "pxCore.h"
#include "pxWindowNative.h"
#include "rtLog.h"

#include <dlfcn.h>
#include <vector>

static const char* kEglProviderName = "RT_EGL_PROVIDER";
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
    rtLogFatal("failed to find %s", libname);

  void* func = dlsym(lib, function);
  if (!func)
    rtLogFatal("failed to function %s from %s", function, libname);

  dlclose(lib);
  return func;
}

static pxEGLProvider* createPlatformEGLProvider()
{
  if (!pxCreateEGLProvider)
  {
    const char* name = getenv(kEglProviderName);
    if (!name)
      rtLogFatal("%s unset. Please set like %s=libprovider.so", kEglProviderName,
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
      rtLogFatal("%s unset. Please set like %s=libprovider.so", kEglProviderName,
        kEglProviderName);

    pxDestroyEGLProvider = (EGLProviderDestroyFunction) findSymbol(name, kEGLProviderDestroy);
  }

  return pxDestroyEGLProvider(provider);
}

static EGLConfig chooseEGLConfig(EGLDisplay display)
{
  EGLint configCount = 0;
  if (!eglGetConfigs(display, 0, 0, &configCount) == EGL_TRUE)
    rtLogFatal("failed to get EGL configuration count");

  typedef std::vector<EGLConfig> egl_config_list_t;

  egl_config_list_t conf;
  conf.resize(configCount);

  if (!eglGetConfigs(display, &conf[0], configCount, &configCount))
    rtLogFatal("failed to get EGL configuration list");

  int chosenConfig = 0;
  for (int i = 0; i < static_cast<int>(conf.size()); ++i)
  {
    EGLint depthRed;
    EGLint depthBlue;
    EGLint depthGreen;
    EGLint depthAlpha;

    const EGLConfig& c = conf[i];
    
    if (!eglGetConfigAttrib(display, c, EGL_RED_SIZE, &depthRed))
      rtLogFatal("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_GREEN_SIZE, &depthGreen))
      rtLogFatal("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthBlue))
      rtLogFatal("failed to get depth of red");

    if (!eglGetConfigAttrib(display, c, EGL_BLUE_SIZE, &depthAlpha))
      rtLogFatal("failed to get depth of red");

    rtLogInfo("egl config[%d]: rgba(%d, %d, %d, %d)", i, depthRed, depthGreen, depthBlue,
        depthAlpha);

    if (depthRed == 8 && depthGreen == 8 && depthBlue == 8 && depthAlpha == 8)
    {
      rtLogInfo("choosing %d of %d EGL configurations", i, static_cast<int>(conf.size()));
      chosenConfig = i;
      break;
    }
  }

  return conf[chosenConfig];
}

pxWindowNative::pxWindowNative()
{
  mEGLProvider = createPlatformEGLProvider();
  if (!mEGLProvider)
    rtLogFatal("failed to get EGL provider");

  mEGLProvider->init();

  rtError err = RT_OK;
  if ((err = mEGLProvider->createDisplay(&mEGLDisplay)) != RT_OK)
    rtLogFatal("failed to get default EGL display");

  EGLConfig config = chooseEGLConfig(mEGLDisplay);

  if ((err = mEGLProvider->createSurface(mEGLDisplay, config, &mEGLSurface)) != RT_OK)
    rtLogFatal("failed to create EGL surface: %s", rtStrError(err));

  if ((err = mEGLProvider->createContext(mEGLDisplay, config, &mEGLContext)) != RT_OK)
    rtLogFatal("failed to create EGL context: %s", rtStrError(err));

  // TODO: This should be called in "render" thread
  eglMakeCurrent(mEGLDisplay, mEGLSurface, mEGLSurface, mEGLContext);
  eglSwapInterval(mEGLDisplay, 1);
}

pxWindowNative::~pxWindowNative()
{
  destroyPlatformEGLProvider(mEGLProvider);
}
