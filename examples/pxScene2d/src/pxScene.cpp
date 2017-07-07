/*

 pxCore Copyright 2005-2017 John Robinson

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

#include "pxCore.h"
#include "pxTimer.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#define ANIMATION_ROTATE_XYZ
#include "pxContext.h"
#include "pxScene2d.h"
#include "rtUrlUtils.h"

#include "rtNode.h"
#include "pxUtil.h"

#ifdef RUNINMAIN
extern rtNode script;
#else
using namespace std;
#include "rtNodeThread.h"
#endif

#include "jsbindings/rtWrapperUtils.h"
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
#endif

#ifndef RUNINMAIN
class AsyncScriptInfo;
vector<AsyncScriptInfo*> scriptsInfo;
static uv_work_t nodeLoopReq;
#endif

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
extern int pxObjectCount;
#ifdef HAS_LINUX_BREAKPAD
static bool dumpCallback(const google_breakpad::MinidumpDescriptor& descriptor,
void* context, bool succeeded) {
  UNUSED_PARAM(descriptor);
  UNUSED_PARAM(context);
  return succeeded;
}
#endif

#ifdef ENABLE_CODE_COVERAGE
extern "C" void __gcov_flush();
#endif
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

    char buffer[1024];
		std::string urlStr(url);
		if (urlStr.find("http")) {
    sprintf(buffer,"shell.js?url=%s",rtUrlEncodeParameters(url).cString());
		}
		else {
			sprintf(buffer, "shell.js?url=%s",url);
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
    rtLogInfo(__FUNCTION__);
    ENTERSCENELOCK();
    if (mView)
      mView->onCloseRequest();
    EXITSCENELOCK()
    // delete mView;

#ifndef RUNINMAIN
    uv_close((uv_handle_t*) &asyncNewScript, NULL);
    uv_close((uv_handle_t*) &gcTrigger, NULL);
#endif
   // pxScene.cpp:104:12: warning: deleting object of abstract class type ‘pxIView’ which has non-virtual destructor will cause undefined behaviour [-Wdelete-non-virtual-dtor]

#ifdef RUNINMAIN
   script.garbageCollect();
#endif
ENTERSCENELOCK()
    mView = NULL;
EXITSCENELOCK()
#ifndef RUNINMAIN
   script.setNeedsToEnd(true);
#endif
  #ifdef ENABLE_DEBUG_MODE
    free(g_origArgv);
  #endif
    script.garbageCollect();
    if (gDumpMemUsage)
    {
      rtLogInfo("pxobjectcount is [%d]",pxObjectCount);
      rtLogInfo("texture memory usage is [%" PRId64 "]",context.currentTextureMemoryUsageInBytes());

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
    if (mView)
      mView->onUpdate(pxSeconds());
    EXITSCENELOCK()
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
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal SEGV received. sleeping to collect data");
  sleep(1800);
}

void handleAbrt(int)
{
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal ABRT received. sleeping to collect data");
  sleep(1800);
}

int pxMain(int argc, char* argv[])
{
#ifdef HAS_LINUX_BREAKPAD
  google_breakpad::MinidumpDescriptor descriptor("/tmp");
  google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL, true, -1);
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
  rtLogWarn("rtIsMainThread() returns %d\n",rtIsMainThread());

    #if PX_PLATFORM_X11
    XInitThreads();
    #endif

  uv_mutex_init(&moreScriptsMutex);
  uv_mutex_init(&threadMutex);

  // Start script thread
  uv_queue_work(nodeLoop, &nodeLoopReq, nodeThread, nodeIsEndingCallback);
  // init asynch that will get notifications about new scripts
  uv_async_init(nodeLoop, &asyncNewScript, processNewScript);
  uv_async_init(nodeLoop, &gcTrigger,garbageCollect);

#endif
char const* s = getenv("PX_DUMP_MEMUSAGE");
if (s && (strcmp(s,"1") == 0))
{
  gDumpMemUsage = true;
}
#ifdef ENABLE_DEBUG_MODE
  int urlIndex  = -1;
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
    else
    {
      if (strstr(argv[i],".js"))
      {
        urlIndex = i;
      }
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
        curpos = curpos + strlen(argv[i]) + 1;
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
#ifdef RUNINMAIN
  script.initializeNode();
#endif
#endif
  char buffer[256];
  sprintf(buffer, "pxscene: %s", xstr(PX_SCENE_VERSION));
  // OSX likes to pass us some weird parameter on first launch after internet install
#ifdef ENABLE_DEBUG_MODE
  win.init(10, 10, 1280, 720, (urlIndex != -1)?argv[urlIndex]:"browser.js");
#else
  win.init(10, 10, 1280, 720, (argc >= 2 && argv[1][0] != '-')?argv[1]:"browser.js");
#endif
  win.setTitle(buffer);
  // JRJR TODO Why aren't these necessary for glut... pxCore bug
  win.setVisibility(true);
  win.setAnimationFPS(60);

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
  win_sparkle_set_appcast_url("http://jmgasper.gitlab.io/appcast.xml");
  win_sparkle_init(); 

#endif

  eventLoop.run();
#ifdef ENABLE_DEBUG_MODE
  free(g_origArgv);
#endif
  script.garbageCollect();
  if (gDumpMemUsage)
  {
    rtLogInfo("pxobjectcount is [%d]",pxObjectCount);
    rtLogInfo("texture memory usage is [%ld]",context.currentTextureMemoryUsageInBytes());
  }


#ifdef WIN32
  win_sparkle_cleanup();
#endif

  return 0;
}
