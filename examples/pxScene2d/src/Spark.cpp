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

// main.cpp

#include "rtPathUtils.h"

#include "pxCore.h"
#include "pxTimer.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#define ANIMATION_ROTATE_XYZ
#include "pxContext.h"
#include "pxScene2d.h"
#include "rtUrlUtils.h"
#include "rtScript.h"

#include "pxUtil.h"
#include "rtSettings.h"

#ifdef RUNINMAIN
extern rtScript script;
#else
using namespace std;
#include "rtNodeThread.h"
#endif

//#include "jsbindings/rtWrapperUtils.h"
#include <signal.h>
#ifndef WIN32
#include <unistd.h>
#endif

#ifdef WIN32
#include <winsparkle.h>
//todo: is this resource file needed?  if so, uncomment
//#include "../../../pxCore.vsbuild/pxScene2d/resource.h"
#endif

#include <stdint.h>    // for PRId64
#include <inttypes.h>  // for PRId64

#ifndef RUNINMAIN
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK()  rtWrapperSceneUpdateExit();
#else
#define ENTERSCENELOCK()
#define EXITSCENELOCK()
#endif

#ifndef PX_SCENE_VERSION
#define PX_SCENE_VERSION dev
#endif

#ifdef HAS_LINUX_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#elif HAS_WINDOWS_BREAKPAD
#include <windows.h>
#include <wchar.h>
#include <client/windows/handler/exception_handler.h>
#endif

#ifdef PX_SERVICE_MANAGER_LINKED
#include "rtservicemanager.h"
#endif //PX_SERVICE_MANAGER_LINKED

#ifndef RUNINMAIN
class AsyncScriptInfo;
vector<AsyncScriptInfo*> scriptsInfo;
static uv_work_t nodeLoopReq;
#endif

#include "rtThreadPool.h"

#include <stdlib.h>
#include <fstream>

pxEventLoop  eventLoop;
pxEventLoop* gLoop = &eventLoop;

pxContext context;
#ifdef ENABLE_DEBUG_MODE
extern int g_argc;
extern char** g_argv;
char *nodeInput = NULL;
char** g_origArgv = NULL;
#endif
bool gDumpMemUsage = false;
extern bool gApplicationIsClosing;
extern int pxObjectCount;

#include "pxFont.h"

#ifdef PXSCENE_FONT_ATLAS
extern pxFontAtlas gFontAtlas;
#endif

#ifdef HAS_LINUX_BREAKPAD
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
void* context, bool succeeded) {
  UNUSED_PARAM(descriptor);
  UNUSED_PARAM(context);
  return succeeded;
}
#elif HAS_WINDOWS_BREAKPAD
bool dumpCallback(const wchar_t* dump_path,
                     const wchar_t* minidump_id,
                     void* context,
                     EXCEPTION_POINTERS* exinfo,
                     MDRawAssertionInfo* assertion,
                     bool succeeded) {
  return succeeded;
}
#endif

#ifdef ENABLE_CODE_COVERAGE
extern "C" void __gcov_flush();
#endif

#ifdef ENABLE_OPTIMUS_SUPPORT
#include "optimus_client.h"
#endif //ENABLE_OPTIMUS_SUPPORT

class sceneWindow : public pxWindow, public pxIViewContainer
{
public:
  sceneWindow(): mWidth(-1),mHeight(-1),mClosed(false) {}
  virtual ~sceneWindow()
  {
    mView = NULL;
  }

  void init(int x, int y, int w, int h, const char* url = NULL)
  {
    pxWindow::init(x,y,w,h);

    // escape url begin
    std::string escapedUrl;
    std::string origUrl = url;
    for (std::string::iterator it=origUrl.begin(); it!=origUrl.end(); ++it)
    {
      char currChar = *it;
      if ((currChar == '"') || (currChar == '\\'))
      {
        escapedUrl.append(1, '\\');
      }
      escapedUrl.append(1, currChar);
    }
    if (escapedUrl.length() > MAX_URL_SIZE)
    {
      rtLogWarn("url size greater than 8000 bytes, so restting url to browser.js");
      escapedUrl = "browser.js";
    }
    // escape url end
    char buffer[MAX_URL_SIZE + 50];
    memset (buffer, 0, sizeof(buffer));

    if (std::string::npos != escapedUrl.find("http")) {
      snprintf(buffer,sizeof(buffer),"shell.js?url=%s",rtUrlEncodeParameters(escapedUrl.c_str()).cString());
    }
    else {
      snprintf(buffer,sizeof(buffer),"shell.js?url=%s",escapedUrl.c_str());
    }

#ifdef RUNINMAIN
    setView( new pxScriptView(buffer,"javascript/node/v8"));
#else
    pxScriptView * scriptView = new pxScriptView(buffer, "javascript/node/v8");
    rtLogInfo("new scriptView is %x\n",scriptView);
    AsyncScriptInfo * info = new AsyncScriptInfo();
    info->m_pView = scriptView;
    uv_mutex_lock(&moreScriptsMutex);
    scriptsInfo.push_back(info);
    uv_mutex_unlock(&moreScriptsMutex);
    rtLogDebug("sceneWindow::script is pushed on vector\n");
    uv_async_send(&asyncNewScript);
    setView(scriptView);
#endif
  }

  void* getInterface(const char* /*name*/)
  {
     return NULL;
  }
  
  rtError setView(pxIView* v)
  {
    mView = v;

    if (v)
    {
      ENTERSCENELOCK()
      v->setViewContainer(this);
      v->onSize(mWidth, mHeight);
      EXITSCENELOCK()
    }

    return RT_OK;
  }

  virtual void invalidateRect(pxRect* r)
  {
    pxWindow::invalidateRect(r);
  }

  void close()
  {
    onCloseRequest();
  }
protected:

  virtual void onSize(int32_t w, int32_t h)
  {
    if (mWidth != w || mHeight != h)
    {
      mWidth  = w;
      mHeight = h;
      ENTERSCENELOCK()
      if (mView)
        mView->onSize(w, h);
      EXITSCENELOCK()
    }
  }

  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onMouseDown(x, y, flags);
    EXITSCENELOCK()
  }

  virtual void onCloseRequest()
  {
    if (mClosed)
      return;
    mClosed = true;
    if(gDumpMemUsage)
      gApplicationIsClosing = true;
    
    rtLogInfo(__FUNCTION__);
    fflush(stdout);
    ENTERSCENELOCK();
    if (mView)
    {
      rtLogInfo("onClose request started");
      fflush(stdout);
      mView->onCloseRequest();
      rtLogInfo("onClose request completed");
      fflush(stdout);
    }
    EXITSCENELOCK()
    // delete mView;

#ifndef RUNINMAIN
    uv_close((uv_handle_t*) &asyncNewScript, NULL);
    uv_close((uv_handle_t*) &gcTrigger, NULL);
#endif
   // pxScene.cpp:104:12: warning: deleting object of abstract class type ‘pxIView’ which has non-virtual destructor will cause undefined behaviour [-Wdelete-non-virtual-dtor]


  ENTERSCENELOCK()
    mView = NULL;
  EXITSCENELOCK()
  #ifndef RUNINMAIN
   script.setNeedsToEnd(true);
  #endif
  #ifdef ENABLE_DEBUG_MODE
    free(g_origArgv);
  #endif

    rtLogInfo("about to clear all the fonts during close");
    fflush(stdout);
    pxFontManager::clearAllFonts();
    rtLogInfo("cleared all the fonts during close");
    fflush(stdout);
    context.term();
#ifdef RUNINMAIN
    script.pump();
#endif
    rtLogInfo("about to call garbage collect during close");
    fflush(stdout);
    script.collectGarbage();
    rtLogInfo("called garbage collect during close");
    fflush(stdout);

    if (gDumpMemUsage)
    {
      #ifdef RUNINMAIN
          script.pump();
      #endif
      script.collectGarbage();
      rtLogInfo("pxobjectcount is [%d]",pxObjectCount);
#ifndef PX_PLATFORM_DFB_NON_X11
      rtLogInfo("texture memory usage is [%" PRId64 "]",context.currentTextureMemoryUsageInBytes());
#endif
      fflush(stdout);
// #ifdef PX_PLATFORM_MAC
//       rtLogInfo("texture memory usage is [%lld]",context.currentTextureMemoryUsageInBytes());
// #else
//       rtLogInfo("texture memory usage is [%ld]",context.currentTextureMemoryUsageInBytes());
// #endif
    }
    #ifdef ENABLE_CODE_COVERAGE
    __gcov_flush();
    #endif
  ENTERSCENELOCK()
      eventLoop.exit();
  EXITSCENELOCK()
  }

  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onMouseUp(x, y, flags);
    EXITSCENELOCK()
  }

  virtual void onMouseLeave()
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onMouseLeave();
    EXITSCENELOCK()
  }

  virtual void onMouseMove(int32_t x, int32_t y)
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onMouseMove(x, y);
    EXITSCENELOCK()
  }

  virtual void onFocus()
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onFocus();
    EXITSCENELOCK()
  }
  virtual void onBlur()
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onBlur();
    EXITSCENELOCK()
  }

  virtual void onKeyDown(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK()
    if (mView)
    {
      mView->onKeyDown(keycode, flags);
    }
    EXITSCENELOCK()
  }

  virtual void onKeyUp(uint32_t keycode, uint32_t flags)
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onKeyUp(keycode, flags);
    EXITSCENELOCK()
  }

  virtual void onChar(uint32_t c)
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onChar(c);
    EXITSCENELOCK()
  }

  virtual void onDraw(pxSurfaceNative )
  {
    ENTERSCENELOCK()
    if (mView)
      mView->onDraw();
    EXITSCENELOCK()
  }

  virtual void onAnimationTimer()
  {
    ENTERSCENELOCK()
    if (mView && !mClosed)
      mView->onUpdate(pxSeconds());
    EXITSCENELOCK()
#ifdef ENABLE_OPTIMUS_SUPPORT
    OptimusClient::pumpRemoteObjectQueue();
#endif //ENABLE_OPTIMUS_SUPPORT
#ifdef RUNINMAIN
    script.pump();
#endif
  }

  int mWidth;
  int mHeight;
  rtRef<pxIView> mView;
  bool mClosed;
};
sceneWindow win;
#define xstr(s) str(s)
#define str(s) #s

void handleTerm(int)
{
  rtLogInfo("Signal TERM received. closing the window");
  win.close();
}

void handleSegv(int)
{
  signal(SIGSEGV, SIG_DFL);
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal SEGV received. sleeping to collect data");
#ifndef WIN32
  sleep(1800);
#endif //WIN32
}

void handleAbrt(int)
{
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal ABRT received. sleeping to collect data");
#ifndef WIN32
  sleep(1800);
#endif //WIN32
}

int pxMain(int argc, char* argv[])
{
#ifdef HAS_LINUX_BREAKPAD
  google_breakpad::MinidumpDescriptor descriptor("/tmp");
  google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL, true, -1);
#elif HAS_WINDOWS_BREAKPAD
  //register exception handler for breakpad
  google_breakpad::ExceptionHandler* handler = NULL;
  handler = new google_breakpad::ExceptionHandler(L"C:\\dumps\\",
                                   NULL,
                                   dumpCallback,
                                   NULL,
                                   google_breakpad::ExceptionHandler::HANDLER_ALL,
                                   MiniDumpNormal,
                                   L"",
                                   NULL);
#endif
  signal(SIGTERM, handleTerm);
  char const* handle_signals = getenv("HANDLE_SIGNALS");
  if (handle_signals && (strcmp(handle_signals,"1") == 0))
  {
    signal(SIGSEGV, handleSegv);
    signal(SIGABRT, handleAbrt);
  }
#ifndef RUNINMAIN
  rtLogWarn("Setting  __rt_main_thread__ to be %x\n",pthread_self());
   __rt_main_thread__ = pthread_self(); //  NB
  rtLogWarn("Now  __rt_main_thread__ is %x\n",__rt_main_thread__);
  //rtLogWarn("rtIsMainThread() returns %d\n",rtIsMainThread());

    #if PX_PLATFORM_X11
    XInitThreads();
    #endif

  uv_mutex_init(&moreScriptsMutex);
  uv_mutex_init(&threadMutex);

  // Start script thread
  uv_queue_work(nodeLoop, &nodeLoopReq, nodeThread, nodeIsEndingCallback);
  // init asynch that will get notifications about new scripts
  uv_async_init(nodeLoop, &asyncNewScript, processNewScript);
  uv_async_init(nodeLoop, &gcTrigger,collectGarbage);

#endif

  rtModuleDirs::instance();

  rtString settingsPath;
  if (RT_OK == rtGetHomeDirectory(settingsPath))
  {
    settingsPath.append(".sparkSettings.json");
    if (rtFileExists(settingsPath))
      rtSettings::instance()->loadFromFile(settingsPath);
  }

  // overwrite file settings with settings from the command line
  rtSettings::instance()->loadFromArgs(argc, argv);

char const* s = getenv("PX_DUMP_MEMUSAGE");
if (s && (strcmp(s,"1") == 0))
{
  gDumpMemUsage = true;
}

  const char* url = "browser.js";
  for (int i=1;i<argc;i++)
  {
    const char* arg = argv[i];
    if (arg && arg[0] != '-' && arg[0] != 'Y') // Xcode Debugger adds args of "-NSDocumentRevisionsDebugMode YES"
    {
      url = arg;
      break;
    }
  }

#ifdef ENABLE_DEBUG_MODE
#ifdef RTSCRIPT_SUPPORT_NODE
  bool isDebugging = false;

  g_argv = (char**)malloc((argc+2) * sizeof(char*));
  g_origArgv = g_argv;
  int size  = 0;
  for (int i=1;i<argc;i++)
  {
    if (strstr(argv[i],"--"))
    {
      if (strstr(argv[i],"--debug"))
      {
        isDebugging = true;
      }
      size += strlen(argv[i])+1;
    }
  }
  if (isDebugging == true)
  {
    nodeInput = (char *)malloc(size+8);
    memset(nodeInput,0,size+8);
  }
  else
  {
    nodeInput = (char *)malloc(size+46);
    memset(nodeInput,0,size+46);
  }
  int curpos = 0;
  strcpy(nodeInput,"pxscene\0");
  g_argc  = 0;
  g_argv[g_argc++] = &nodeInput[0];
  curpos += 8;

  for (int i=1;i<argc;i++)
  {
    if (strstr(argv[i],"--"))
    {
        strcpy(nodeInput+curpos,argv[i]);
        *(nodeInput+curpos+strlen(argv[i])) = '\0';
        g_argv[g_argc++] = &nodeInput[curpos];
        curpos = curpos + (int) strlen(argv[i]) + 1;
    }
  }
  if (false == isDebugging)
  {
      strcpy(nodeInput+curpos,"-e\0");
      g_argv[g_argc++] = &nodeInput[curpos];
      curpos = curpos + 3;
      strcpy(nodeInput+curpos,"console.log(\"rtNode Initialized\");\0");
      g_argv[g_argc++] = &nodeInput[curpos];
      curpos = curpos + 35;
  }
  #endif
#endif

#ifdef RUNINMAIN
  script.init();
#endif
  char buffer[256];
  sprintf(buffer, "Spark: %s", xstr(PX_SCENE_VERSION));

  int32_t windowWidth = rtGetEnvAsValue("PXSCENE_WINDOW_WIDTH","1280").toInt32();
  int32_t windowHeight = rtGetEnvAsValue("PXSCENE_WINDOW_HEIGHT","720").toInt32();

  rtValue screenWidth, screenHeight;
  if (RT_OK == rtSettings::instance()->value("screenWidth", screenWidth))
    windowWidth = screenWidth.toInt32();
  if (RT_OK == rtSettings::instance()->value("screenHeight", screenHeight))
    windowHeight = screenHeight.toInt32();

  // OSX likes to pass us some weird parameter on first launch after internet install
  rtLogInfo("window width = %d height = %d", windowWidth, windowHeight);
  win.init(10, 10, windowWidth, windowHeight, url);
  win.setTitle(buffer);
  // JRJR TODO Why aren't these necessary for glut... pxCore bug
  win.setVisibility(true);

  uint32_t animationFPS = 60;
  rtString f;
  if (RT_OK == rtGetHomeDirectory(f))
  {
    f.append(".sparkFps");
    if (rtFileExists(f))
    {
      std::fstream fs(f.cString(), std::fstream::in);
      uint32_t val = 0;
      fs >> val;
      if (val > 0)
        animationFPS = val;
    }
  }
  rtLogInfo("Animation FPS: %lu", (unsigned long) animationFPS);
  win.setAnimationFPS(animationFPS);

#ifdef WIN32

  HDC hdc = ::GetDC(win.mWindow);
  HGLRC hrc;

	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW |
		PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER |
		PFD_SWAP_EXCHANGE,
		PFD_TYPE_RGBA,
		24,
		0, 0, 0, 0, 0, 0,
		8,
		0,
		0,
		0, 0, 0, 0,
		24,
		8,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pixelFormat = ChoosePixelFormat(hdc, &pfd);
  if (::SetPixelFormat(hdc, pixelFormat, &pfd)) {
	  hrc = wglCreateContext(hdc);
	  if (::wglMakeCurrent(hdc, hrc)) {
			glewExperimental = GL_TRUE;
			if (glewInit() != GLEW_OK)
				throw std::runtime_error("glewInit failed");

			char *GL_version = (char *)glGetString(GL_VERSION);
			char *GL_vendor = (char *)glGetString(GL_VENDOR);
			char *GL_renderer = (char *)glGetString(GL_RENDERER);


			rtLogInfo("GL_version = %s", GL_version);
			rtLogInfo("GL_vendor = %s", GL_vendor);
			rtLogInfo("GL_renderer = %s", GL_renderer);
	  }
  }


#endif
  #if 0
  sceneWindow win2;
  win2.init(50, 50, 1280, 720);
  #endif

// JRJR TODO this needs happen after GL initialization which right now only happens after a pxWindow has been created.
// Likely will move this to pxWindow...  as an option... a "context" type
// would like to decouple it from pxScene2d specifically
  context.init();

#ifdef WIN32

  // Initialize WinSparkle as soon as the app itself is initialized, right
  // before entering the event loop:
  win_sparkle_set_appcast_url("https://github.com/pxscene/pxscene/tree/gh-pages/dist/windows/appcast.xml");
  win_sparkle_init(); 

#endif

#ifdef ENABLE_OPTIMUS_SUPPORT
  rtObjectRef tempObject;
  OptimusClient::registerApi(tempObject);
#endif //ENABLE_OPTIMUS_SUPPORT

#ifdef PX_SERVICE_MANAGER_LINKED
  RtServiceManager::start();

#endif //PX_SERVICE_MANAGER_LINKED

  eventLoop.run();

#ifdef WIN32
  win_sparkle_cleanup();
#endif
  
  base64_cleanup();

  return 0;
}
