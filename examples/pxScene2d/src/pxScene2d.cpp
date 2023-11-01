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

// pxScene2d.cpp

#include "pxScene2d.h"

#include <math.h>
#include <assert.h>

#include "rtLog.h"
#include "rtRef.h"
#include "rtString.h"

#include "rtPathUtils.h"
#include "rtUrlUtils.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxTimer.h"

#include "pxRectangle.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxTextCanvas.h"
#include "pxImage.h"

#ifdef ENABLE_SPARK_WEBGL
#include "pxWebGL.h"
#endif //ENABLE_SPARK_WEBGL

#ifdef ENABLE_SPARK_VIDEO
#include "pxVideo.h"
#endif //ENABLE_SPARK_VIDEO

#ifdef PX_SERVICE_MANAGER
#include "pxServiceManager.h"
#endif //PX_SERVICE_MANAGER
#include "pxImage9.h"
#include "pxImageA.h"
#include "pxImage9Border.h"

#include "pxShaderResource.h"


#if !defined(ENABLE_DFB) && !defined(DISABLE_WAYLAND)
#include "pxWaylandContainer.h"
#endif //ENABLE_DFB

#include "pxContext.h"
#include "rtFileDownloader.h"
#include "rtMutex.h"

#include "pxIView.h"

#include "pxClipboard.h"
#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <algorithm>

#ifdef ENABLE_RT_NODE
#include "rtScript.h"
#endif //ENABLE_RT_NODE

#include "rtJsonUtils.h"
#include "rtHttpRequest.h"
#include "rtUrlUtils.h"

#ifdef ENABLE_SPARK_THUNDER
#include <securityagent/securityagent.h>
#endif //ENABLE_SPARK_THUNDER
#define MAX_TOKEN_BUFFER_LENGTH 2048

using namespace rapidjson;

using namespace std;


#define xstr(s) str(s)
#define str(s) #s

#ifndef PX_SCENE_VERSION
#define PX_SCENE_VERSION dev_ver
#endif

// #define DEBUG_SKIP_DRAW       // Skip DRAW   code - for testing.
// #define DEBUG_SKIP_UPDATE     // Skip UPDATE code - for testing.

#ifdef DEBUG_SKIP_DRAW
#warning  DEBUG_SKIP_DRAW is ON !
#endif

#ifdef DEBUG_SKIP_UPDATE
#warning  DEBUG_SKIP_UPDATE is ON !
#endif

#ifdef PX_DIRTY_RECTANGLES
bool gDirtyRectsEnabled = true;
#else
bool gDirtyRectsEnabled = false;
#endif //PX_DIRTY_RECTANGLES

extern rtThreadQueue* gUIThreadQueue;
extern pxContext      context;

static int fpsWarningThreshold = 25;
bool topSparkView = true;

rtEmitRef pxScriptView::mEmit = new rtEmit();

rtRef<rtFunctionCallback> pxScriptView::mSparkHttp = NULL;
rtString pxScriptView::mSparkGlInitApp;
rtString pxScriptView::mSparkInitApp;


#ifdef PXSCENE_SUPPORT_STORAGE
#define DEFAULT_LOCALSTORAGE_DIR ".spark/storage/"
#define DEFAULT_LOCALSTORAGE_DIR_ENV_NAME "SPARK_STORAGE"
#endif


// Debug Statistics
#ifdef USE_RENDER_STATS

uint32_t gDrawCalls;
uint32_t gTexBindCalls;
uint32_t gFboBindCalls;

#endif //USE_RENDER_STATS

#include <stdint.h>
#include <stdlib.h>

#ifdef ENABLE_RT_NODE
extern void rtWrapperSceneUpdateEnter();
extern void rtWrapperSceneUpdateExit();

extern rtScript script;

#endif //ENABLE_RT_NODE

#ifdef ENABLE_VALGRIND
#include <valgrind/callgrind.h>
void startProfiling()
{
  CALLGRIND_START_INSTRUMENTATION;
}

void stopProfiling()
{
  CALLGRIND_STOP_INSTRUMENTATION;
}
#endif //ENABLE_VALGRIND

extern int pxObjectCount;
bool gApplicationIsClosing = false;

bool enableOptimizedUpdateOnStartup()
{
#ifdef ENABLE_SPARK_OPTIMIZED_UPDATE
  bool enableSparkOptimizedUpdate = true;
#else
  bool enableSparkOptimizedUpdate = false;
#endif //ENABLE_SPARK_OPTIMIZED_UPDATE
  char const *s = getenv("SPARK_OPTIMIZED_UPDATE");
  if (s)
  {
    if (strlen(s) > 0)
    {
      int value = atoi(s);
      if (value > 0)
      {
        enableSparkOptimizedUpdate = true;
      }
    }
  }
  if (enableSparkOptimizedUpdate)
  {
    printf("enabling optimized update on startup\n");
  }
  return enableSparkOptimizedUpdate;
}

bool pxScene2d::mOptimizedUpdateEnabled = enableOptimizedUpdateOnStartup();

#include <rapidjson/document.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/error/en.h>

// store the mapping between wayland app names and binary paths
map<string, string> gWaylandAppsMap;
map<string, string> gWaylandRegistryAppsMap;
map<string, string> gPxsceneWaylandAppsMap;
#if !(defined(ENABLE_DFB) || defined(DISABLE_WAYLAND))
static bool gWaylandAppsConfigLoaded = false;
#endif
#define DEFAULT_WAYLAND_APP_CONFIG_FILE "./waylandregistry.conf"
#define DEFAULT_ALL_APPS_CONFIG_FILE "./pxsceneappregistry.conf"

// ubuntu is mapped with glut
#if defined(PX_PLATFORM_WIN)
const rtString gPlatformOS = "Windows";
#elif defined(PX_PLATFORM_MAC)
const rtString gPlatformOS = "macOS";
#else
const rtString gPlatformOS = "Linux";
#endif

void populateWaylandAppsConfig()
{
  //populate from the wayland registry file
  FILE* fp = NULL;
  char const* s = getenv("WAYLAND_APPS_CONFIG");
  if (s)
  {
    fp = fopen(s, "rb");
  }
  if (NULL == fp)
  {
    fp = fopen(DEFAULT_WAYLAND_APP_CONFIG_FILE, "rb");
    if (NULL == fp)
    {
      rtLogInfo("Wayland config read error : [unable to read waylandregistry.conf]\n");
      return;
    }
  }
  char readBuffer[65536];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  rapidjson::Document doc;
  rapidjson::ParseResult result = doc.ParseStream(is);
  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogInfo("Wayland config read error : [JSON parse error while reading waylandregistry.conf: %s (%zu)]\n",rapidjson::GetParseError_En(e), result.Offset());
    fclose(fp);
    return;
  }
  fclose(fp);

  if (! doc.HasMember("waylandapps"))
  {
    rtLogInfo("Wayland config read error : [waylandapps element not found]\n");
    return;
  }

  const rapidjson::Value& appList = doc["waylandapps"];
  for (rapidjson::SizeType i = 0; i < appList.Size(); i++)
  {
    if (appList[i].IsObject())
    {
      if ((appList[i].HasMember("name")) && (appList[i]["name"].IsString()) && (appList[i].HasMember("binary")) &&
          (appList[i]["binary"].IsString()))
      {
        string appName = appList[i]["name"].GetString();
        string binary = appList[i]["binary"].GetString();
        if ((appName.length() != 0) && (binary.length() != 0))
        {
          gWaylandRegistryAppsMap[appName] = binary;
          rtLogInfo("Mapped wayland app [%s] to path [%s] \n", appName.c_str(), binary.c_str());
        }
        else
        {
          rtLogInfo("Wayland config read error : [one of the entry not added due to name/binary is empty]\n");
        }
      }
      else
      {
        rtLogInfo("Wayland config read error : [one of the entry not added due to name/binary not present]\n");
      }
    }
  }
}

void populateAllAppsConfig()
{
  //populate from the apps registry file
  FILE* fp = NULL;
  char const* s = getenv("PXSCENE_APPS_CONFIG");
  if (s)
  {
    fp = fopen(s, "rb");
  }
  if (NULL == fp)
  {
    fp = fopen(DEFAULT_ALL_APPS_CONFIG_FILE, "rb");
    if (NULL == fp)
    {
      rtLogInfo("pxscene app config read error : [unable to read all apps config file]\n");
      return;
    }
  }
  char readBuffer[65536];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  rapidjson::Document doc;
  rapidjson::ParseResult result = doc.ParseStream(is);
  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogInfo("pxscene app config read error : [JSON parse error while reading all apps conf file: %s (%zu)]\n",rapidjson::GetParseError_En(e), result.Offset());
    fclose(fp);
    return;
  }
  fclose(fp);

  if (! doc.HasMember("applications"))
  {
    rtLogInfo("pxscene apps config read error : [applications element not found]\n");
    return;
  }

  const rapidjson::Value& appList = doc["applications"];
  for (rapidjson::SizeType i = 0; i < appList.Size(); i++)
  {
    if (appList[i].IsObject())
    {
      if ((appList[i].HasMember("cmdName")) && (appList[i]["cmdName"].IsString()) && (appList[i].HasMember("uri")) &&
          (appList[i]["uri"].IsString()) && (appList[i].HasMember("applicationType")) && (appList[i]["applicationType"].IsString()))
      {
        string appName = appList[i]["cmdName"].GetString();
        string binary = appList[i]["uri"].GetString();
        string type = appList[i]["applicationType"].GetString();
        if ((appName.length() != 0) && (binary.length() != 0) && (type == "native"))
        {
          gPxsceneWaylandAppsMap[appName] = binary;
          rtLogInfo("Mapped wayland app [%s] to path [%s] \n", appName.c_str(), binary.c_str());
        }
        else
        {
          rtLogInfo("pxscene app config read error : [one of the entry not added due to name/uri is empty].  type=%s\n", type.c_str());
        }
      }
      else
      {
        rtLogInfo("pxscene config read error : [one of the entry not added due to name/uri not present or type is not native]\n");
      }
    }
  }
}

void populateAllAppDetails(rtString& appDetails)
{
  appDetails = "[ ";
  int appCount = 0;
  for (std::map<string, string>::iterator it=gWaylandRegistryAppsMap.begin(); it!=gWaylandRegistryAppsMap.end(); ++it)
  {
    if (appCount > 0)
    {
      appDetails.append(", ");
    }
    rtString app("{\"displayName\":\"");
    app.append((it->first).c_str());
    app.append("\", \"cmdName\":\"");
    app.append((it->first).c_str());
    app.append("\",");
    app.append("\"uri\":\"");
    app.append((it->second).c_str());
    app.append("\",");
    app.append("\"applicationType\" : \"native\"}");
    appDetails.append(app);
    appCount++;
  }
  //populate from the apps registry file
  FILE* fp = NULL;
  char const* s = getenv("PXSCENE_APPS_CONFIG");
  if (s)
  {
    fp = fopen(s, "rb");
  }
  if (NULL == fp)
  {
    fp = fopen(DEFAULT_ALL_APPS_CONFIG_FILE, "rb");
    if (NULL == fp)
    {
      rtLogInfo("pxscene app config read error : [unable to read all apps config file]\n");
      appDetails.append("]");
      return;
    }
  }
  char readBuffer[65536];
  memset(readBuffer, 0, sizeof(readBuffer));
  rapidjson::FileReadStream is(fp, readBuffer, sizeof(readBuffer));
  rapidjson::Document doc;
  rapidjson::ParseResult result = doc.ParseStream(is);
  if (!result)
  {
    rapidjson::ParseErrorCode e = doc.GetParseError();
    rtLogInfo("pxscene app config read error : [JSON parse error while reading all apps conf file: %s (%zu)]\n",rapidjson::GetParseError_En(e), result.Offset());
    fclose(fp);
    appDetails.append("]");
    return;
  }
  fclose(fp);

  if (! doc.HasMember("applications"))
  {
    rtLogInfo("pxscene apps config read error : [applications element not found]\n");
    appDetails.append("]");
    return;
  }

  const rapidjson::Value& appList = doc["applications"];
  for (rapidjson::SizeType i = 0; i < appList.Size(); i++)
  {
    if (appList[i].IsObject())
    {
      if (appCount > 0)
      {
        appDetails.append(", ");
      }
      rapidjson::StringBuffer sb;
      Writer<rapidjson::StringBuffer> writer(sb);
      appList[i].Accept(writer);
      appDetails.append(sb.GetString());
      appCount++;
    }
  }
  appDetails.append("]");
}


void pxRoot::sendPromise()
{
  if(!((rtPromise*)mReady.getPtr())->status())
  {
    mReady.send("resolve",this);
  }
}


rtDefineObject(pxRoot,pxObject);

int gTag = 0;

pxScene2d::pxScene2d(bool top, pxScriptView* scriptView)
  : mRoot(), mInfo(), mCapabilityVersions(), start(0), sigma_draw(0), sigma_update(0), end2(0), frameCount(0), mWidth(0), mHeight(0), mStopPropagation(false), mContainer(NULL), mReportFps(false), mShowDirtyRectangle(false),
    mEnableDirtyRectangles(gDirtyRectsEnabled),
    mInnerpxObjects(), mSuspended(false),
#ifdef PX_DIRTY_RECTANGLES
    mArchive(),mDirtyRect(), mLastFrameDirtyRect(),
#endif //PX_DIRTY_RECTANGLES
    mDirty(true), mDragging(false), mDragType(pxConstantsDragType::NONE), mDragTarget(NULL), mTestView(NULL), mDisposed(false), mArchiveSet(false)
#ifdef PXSCENE_SUPPORT_STORAGE
, mStorage(NULL)
#endif
{
  mRoot = new pxRoot(this);
  #ifdef ENABLE_PXOBJECT_TRACKING
  rtLogInfo("pxObjectTracking CREATION pxScene2d::pxScene2d  [%p]", mRoot.getPtr());
  #endif
  mFocusObj = mRoot;
  mEmit = new rtEmit();
  mTop = top;
  mScriptView = scriptView;
  mTag = gTag++;

  rtString origin = scriptView != NULL ? rtUrlGetOrigin(scriptView->getUrl().cString()) : rtString();
#ifdef ENABLE_PERMISSIONS_CHECK
  // rtPermissions accounts parent scene permissions too
  mPermissions = new rtPermissions(origin.cString());
#endif
#ifdef ENABLE_ACCESS_CONTROL_CHECK
  mCORS = new rtCORS(origin.cString());
#endif

  // make sure that initial onFocus is sent
  rtObjectRef e = new rtMapObject;
  mRoot->setFocusInternal(true);
  e.set("target",mFocusObj);
  rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
  t->mEmit.send("onFocus",e);

  if (mTop)
  {
    static bool checkForFpsMinOverride = true;
    if (checkForFpsMinOverride)
    {
      char const* s = getenv("PXSCENE_FPS_WARNING");
      if (s)
      {
        int fpsWarnOverride = atoi(s);
        if (fpsWarnOverride > 0)
        {
          fpsWarningThreshold = fpsWarnOverride;
        }
      }
    }
    checkForFpsMinOverride = false;
  }

  mPointerHidden= false;
  mPointerX= 0;
  mPointerY= 0;
  mPointerLastUpdated= 0;
  #ifdef USE_SCENE_POINTER
  mPointerW= 0;
  mPointerH= 0;
  mPointerHotSpotX= 40;
  mPointerHotSpotY= 16;
  mPointerResource= pxImageManager::getImage("cursor.png");
  #endif

  mInfo = new rtMapObject;
  mInfo.set("version", xstr(PX_SCENE_VERSION));

#ifdef ENABLE_RT_NODE
  mInfo.set("engine", script.engine());
#endif

  rtObjectRef build = new rtMapObject;
  build.set("date", xstr(__DATE__));
  build.set("time", xstr(__TIME__));
  build.set("revision", xstr(SPARK_BUILD_GIT_REVISION));
  build.set("os", gPlatformOS);

  mInfo.set("build", build);
  mInfo.set("gfxmemory", context.currentTextureMemoryUsageInBytes());

  //////////////////////////////////////////////////////
  //
  //                 CAPABILITY VERSIONS
  //
  //////////////////////////////////////////////////////
  //
  // capabilities.graphics.svg          = 2
  // capabilities.graphics.cursor       = 1
  // capabilities.graphics.colors       = 1
  // capabilities.graphics.screenshots  = 2
  // capabilities.graphics.shaders      = 1
  // capabilities.graphics.text         = 3
  // capabilities.graphics.text_fallback = 1
  // capabilities.graphics.imageAResource  = 2
  //
  // capabilities.font.fallback = 1
  //
  // capabilities.scene.external = 1
  //
  // capabilities.network.cors          = 1
  // capabilities.network.corsResources = 1
  // capabilities.network.http2         = 2
  //
  // capabilities.metrics.textureMemory = 1
  //
  // capabilities.animations.durations = 2
  //
  // capabilities.events.drag_n_drop    = 2   // additional Drag'n'Drop events
  //
  // capabilities.video.player         = 1
  // capabilities.sparkgl.nativedrawing    = 1
  // capabilities.sparkgl.supports1080    = 1
  // capabilities.sparkgl.animatedImages    = 1
  // capabilities.sparkgl.optimus = 2
  // capabilities.sparkgl.bootstrap = 2

  mCapabilityVersions = new rtMapObject;

  rtObjectRef graphicsCapabilities = new rtMapObject;

  graphicsCapabilities.set("svg", 2);
  graphicsCapabilities.set("colors", 1);

#ifdef SUPPORT_GIF
    graphicsCapabilities.set("gif", 2);
#endif //SUPPORT_GIF
  graphicsCapabilities.set("imageAResource", 2);

  graphicsCapabilities.set("screenshots", 2);
  graphicsCapabilities.set("shaders", 1);
  graphicsCapabilities.set("text", 3);


  rtObjectRef fontCapabilities = new rtMapObject;
  fontCapabilities.set("fallback", 1);

  mCapabilityVersions.set("font", fontCapabilities);

#ifdef SPARK_CURSOR_SUPPORT
  graphicsCapabilities.set("cursor", 1);

#else
  rtValue enableCursor;
  if (RT_OK == rtSettings::instance()->value("enableCursor", enableCursor))
  {
    if (enableCursor.toString().compare("true") == 0)
    {
      graphicsCapabilities.set("cursor", 1);
    }
  }
#endif // SPARK_CURSOR_SUPPORT

  mCapabilityVersions.set("graphics", graphicsCapabilities);

  rtObjectRef sceneCapabilities = new rtMapObject;
#if defined(DISABLE_WAYLAND)
  sceneCapabilities.set("external", 0);
#else
  sceneCapabilities.set("external", 1);
#endif //DISABLE_WAYLAND
  mCapabilityVersions.set("scene", sceneCapabilities);

  rtObjectRef networkCapabilities = new rtMapObject;

#ifdef ENABLE_ACCESS_CONTROL_CHECK
  networkCapabilities.set("cors", 1);

#ifdef ENABLE_CORS_FOR_RESOURCES
  networkCapabilities.set("corsResources", 1);
#endif // ENABLE_CORS_FOR_RESOURCES

#endif // ENABLE_ACCESS_CONTROL_CHECK

  networkCapabilities.set("http2", 2);

  mCapabilityVersions.set("network", networkCapabilities);
#ifdef PXSCENE_SUPPORT_STORAGE
  mCapabilityVersions.set("storage", 2);
#endif

  rtObjectRef metricsCapabilities = new rtMapObject;

  metricsCapabilities.set("textureMemory", 2);
  metricsCapabilities.set("resources", 1);
  mCapabilityVersions.set("metrics", metricsCapabilities);

  rtObjectRef animationCapabilities = new rtMapObject;

  animationCapabilities.set("durations", 2);
  mCapabilityVersions.set("animations", animationCapabilities);
  //////////////////////////////////////////////////////

  rtObjectRef userCapabilities = new rtMapObject;

  mCapabilityVersions.set("events", userCapabilities);
  userCapabilities.set("drag_n_drop", (gPlatformOS == "macOS") ? 2 : 1);

#ifdef ENABLE_SPARK_VIDEO
  rtObjectRef videoCapabilities = new rtMapObject;
  videoCapabilities.set("player", 2);
  rtValue enableVideo;
  if (RT_OK == rtSettings::instance()->value("enableVideo", enableVideo))
  {
    if (enableVideo.toString().compare("false") == 0)
    {
      videoCapabilities.set("player", 0);
    }
  }
  mCapabilityVersions.set("video", videoCapabilities);
#endif //ENABLE_SPARK_VIDEO

  rtObjectRef sparkGlCapabilities = new rtMapObject;
  sparkGlCapabilities.set("nativedrawing", 1);
  rtValue enableSparkGlNativeDrawing;
  char const* sparkGlNativeDrawingEnv = getenv("SPARK_ENABLE_SPARKGL_NATIVE_DRAWING");
  if (sparkGlNativeDrawingEnv && (strcmp(sparkGlNativeDrawingEnv,"0") == 0))
  {
    rtLogWarn("disabling SparkGL native rendering capability");
    sparkGlCapabilities.set("nativedrawing", 0);
  }
  else if (RT_OK == rtSettings::instance()->value("enableSparkGlNativeDrawing", enableSparkGlNativeDrawing))
  {
    if (enableSparkGlNativeDrawing.toString().compare("false") == 0)
    {
      //disable SparkGL native drawing support if setting disables it
      rtLogWarn("disabling SparkGL native rendering");
      sparkGlCapabilities.set("nativedrawing", 0);
    }
  }
  sparkGlCapabilities.set("supports1080", 1);
  sparkGlCapabilities.set("animatedImages", 1);
  sparkGlCapabilities.set("optimus", 2);
  sparkGlCapabilities.set("bootstrap", 2);
  sparkGlCapabilities.set("api", 1);
  mCapabilityVersions.set("sparkgl", sparkGlCapabilities);

  //////////////////////////////////////////////////////
}

rtError pxScene2d::dispose()
{
    mDisposed = true;
    mMouseEntered = NULL;
    rtObjectRef e = new rtMapObject;
    // pass false to make onClose asynchronous
    mEmit.send("onClose", e);
    for (unsigned int i=0; i<mInnerpxObjects.size(); i++)
    {
      pxObject* temp = (pxObject *) (mInnerpxObjects[i].getPtr());
      if ((NULL != temp) && (NULL == temp->parent()))
      {
        temp->dispose(false);
      }
    }
    mInnerpxObjects.clear();

    if (mRoot)
      mRoot->dispose(false);
    // send scene terminate after dispose to make sure, no cleanup can happen further on app side
    // after clearing the sandbox
    // pass false to make onSceneTerminate asynchronous
    mEmit.send("onSceneTerminate", e);
    mEmit->clearListeners();

    mRoot     = NULL;
    mInfo     = NULL;
    mCapabilityVersions = NULL;
    mFocusObj = NULL;

#ifdef PXSCENE_SUPPORT_STORAGE
    if (mStorage)
      mStorage->term(); // Close database file now
    mStorage = NULL;
#endif

    return RT_OK;
}

// JRJR TODO... Try to get rid of this...  but watch for leaks mentioned by Madan
void pxScene2d::onCloseRequest()
{
  rtLogInfo(__FUNCTION__);
  dispose();
}

#if 0
void pxScene2d::init()
{
  rtLogInfo("Object Sizes");
  rtLogInfo("============");
  rtLogInfo("pxObject     : %zu", sizeof(pxObject));
  rtLogInfo("pxImage      : %zu", sizeof(pxImage));
  rtLogInfo("pxImage9     : %zu", sizeof(pxImage9));
  rtLogInfo("pxRectangle  : %zu", sizeof(pxRectangle));
  rtLogInfo("pxText       : %zu", sizeof(pxText));

  // TODO move this to the window
  context.init();
}
#endif

rtError pxScene2d::create(rtObjectRef p, rtObjectRef& o)
{
  if (mDisposed)
  {
    rtLogInfo("Scene is disposed, not creating any pxobjects");
    return RT_FAIL;
  }

  rtError e = RT_OK;
  rtString t = p.get<rtString>("t");
  bool needpxObjectTracking = true;

  if (!strcmp("rect",t.cString()))
    e = createRectangle(p,o);
  else if (!strcmp("text",t.cString()))
    e = createText(p,o);
  else if (!strcmp("textBox",t.cString()))
    e = createTextBox(p,o);
  else if (!strcmp("textCanvas",t.cString()))
    e = createTextCanvas(p,o);
  else if (!strcmp("image",t.cString()))
    e = createImage(p,o);
  else if (!strcmp("image9",t.cString()))
    e = createImage9(p,o);
  else if (!strcmp("imageA",t.cString()))
    e = createImageA(p,o);
  else if (!strcmp("image9Border",t.cString()))
    e = createImage9Border(p,o);
  else if (!strcmp("imageResource",t.cString()))
  {
    e = createImageResource(p,o);
    needpxObjectTracking = false;
  }
  else if (!strcmp("imageAResource",t.cString()))
  {
    e = createImageAResource(p,o);
    needpxObjectTracking = false;
  }
  else if (!strcmp("fontResource",t.cString()))
  {
    e = createFontResource(p,o);
    needpxObjectTracking = false;
  }
  else if (!strcmp("shaderResource",t.cString()))
  {
    e = createShaderResource(p,o);
    needpxObjectTracking = false;
  }
  else if (!strcmp("scene",t.cString()))
    e = createScene(p,o);
  else if (!strcmp("external",t.cString()))
    e = createExternal(p,o);
  else if (!strcmp("wayland",t.cString()))
    e = createWayland(p,o);
  else if (!strcmp("webgl",t.cString()))
    e = createWebGL(p,o);
  else if (!strcmp("video",t.cString()))
    e = createVideo(p,o);
  else if (!strcmp("object",t.cString()))
    e = createObject(p,o);
  else
  {
    rtLogError("Unknown object type, %s in scene.create.", t.cString());
    return RT_FAIL;
  }

  // Handle psuedo property here for children.  Probably should make this
  rtObjectRef c = p.get<rtObjectRef>("c");
  if (c)
  {
    uint32_t l = c.get<uint32_t>("length");
    for (uint32_t i = 0; i < l; i++)
    {
      rtObjectRef n;
      if ((e = create(c.get<rtObjectRef>(i),n)) == RT_OK)
        n.set("parent", o);
      else
        break;
    }
  }

  if (needpxObjectTracking)
  {
    #ifdef ENABLE_PXOBJECT_TRACKING
    rtLogInfo("pxObjectTracking CREATION pxScene2d::create [%p] [%s] [%s]", o.getPtr(), t.cString(), mScriptView->getUrl().cString());
    #endif
    mInnerpxObjects.push_back((pxObject*)o.getPtr());
  }
  return e;
}

rtError pxScene2d::createObject(rtObjectRef p, rtObjectRef& o)
{
  o = new pxObject(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createRectangle(rtObjectRef p, rtObjectRef& o)
{
  o = new pxRectangle(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createText(rtObjectRef p, rtObjectRef& o)
{
  o = new pxText(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createTextBox(rtObjectRef p, rtObjectRef& o)
{
  o = new pxTextBox(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createTextCanvas(rtObjectRef p, rtObjectRef& o)
{
  o = new pxTextCanvas(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage9(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage9(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImageA(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImageA(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage9Border(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage9Border(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImageResource(rtObjectRef p, rtObjectRef& o)
{
  rtString url     = p.get<rtString>("url");
  rtString proxy   = p.get<rtString>("proxy");

  rtString param_w = p.get<rtString>("w");
  rtString param_h = p.get<rtString>("h");

  rtString param_sx = p.get<rtString>("sx");
  rtString param_sy = p.get<rtString>("sy");

  int32_t iw = 0;
  int32_t ih = 0;
  float   sx = 1.0f;
  float   sy = 1.0f;

  // W x H dimensions
  if(param_w.isEmpty() == false && param_w.length() > 0)
  {
    iw = rtValue(param_w).toInt32();
  }

  if(param_h.isEmpty() == false && param_h.length() > 0)
  {
    ih = rtValue(param_h).toInt32();
  }

  // X Y scaling
  if(param_sx.isEmpty() == false && param_sx.length() > 0)
  {
    sx = rtValue(param_sx).toFloat();
  }

  if(param_sy.isEmpty() == false && param_sy.length() > 0)
  {
    sy = rtValue(param_sy).toFloat();
  }

#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  o = pxImageManager::getImage(url, proxy, mCORS, iw, ih, sx, sy, mArchive);

  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImageAResource(rtObjectRef p, rtObjectRef& o)
{
  rtString url   = p.get<rtString>("url");
  rtString proxy = p.get<rtString>("proxy");

#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  o = pxImageManager::getImageA(url, proxy, mCORS, mArchive);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createFontResource(rtObjectRef p, rtObjectRef& o)
{
  rtString url = p.get<rtString>("url");
  rtString proxy = p.get<rtString>("proxy");
  rtString fontStyle = p.get<rtString>("fontStyle");

  if (!fontStyle.isEmpty()) fontStyle.toLowerAscii();

#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  o = pxFontManager::getFont(url, proxy, mCORS, mArchive, fontStyle);
  return RT_OK;
}

rtError pxScene2d::createShaderResource(rtObjectRef p, rtObjectRef& o)
{
  rtString fragmentUrl = p.get<rtString>("fragment");
  rtString vertexUrl   = p.get<rtString>("vertex");

  if(fragmentUrl.isEmpty() && vertexUrl.isEmpty())
  {
     rtLogError("Failed to create [shaderResource] ... no Fragment/Vertex shader found.");
     return RT_FAIL;
  }

  o = pxShaderManager::getShader(fragmentUrl, vertexUrl, mCORS, mArchive);
  o.set(p);
  o.send("init");

  return RT_OK;
}

rtError pxScene2d::createScene(rtObjectRef p, rtObjectRef& o)
{
  pxSceneContainer* sceneContainer = new pxSceneContainer(this);
  o = sceneContainer;
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::logDebugMetrics()
{
#ifdef ENABLE_DEBUG_METRICS
    script.collectGarbage();
    rtLogInfo("pxobjectcount is [%d]",pxObjectCount);
#ifdef PX_PLATFORM_MAC
      rtLogInfo("texture memory usage is [%lld]",context.currentTextureMemoryUsageInBytes());
#else
      rtLogInfo("texture memory usage is [%ld]",context.currentTextureMemoryUsageInBytes());
#endif
#else
    rtLogWarn("logDebugMetrics is disabled");
#endif
  return RT_OK;
}

rtError pxScene2d::collectGarbage()
{
  rtLogDebug("calling collectGarbage");
  static bool collectGarbageEnabled = false;
  static bool checkEnv = true;
  if (checkEnv)
  {
    char const* s = getenv("SPARK_ENABLE_COLLECT_GARBAGE");
    if (s && (strcmp(s,"1") == 0))
    {
      collectGarbageEnabled = true;
    }
    checkEnv = false;
  }
  if (collectGarbageEnabled)
  {
    rtLogWarn("performing a garbage collection");
    script.collectGarbage();
  }
  else
  {
    rtLogWarn("forced garbage collection is disabled");
  }
  return RT_OK;
}

rtError pxScene2d::suspend(const rtValue &/*v*/, bool& b)
{
  //rtLogDebug("before suspend: %" PRId64 ".", context.currentTextureMemoryUsageInBytes());
  mSuspended = true;
  b = true;
  ENTERSCENELOCK()
  mRoot->releaseData(true);
  EXITSCENELOCK()
  mDirty = true;
  //rtLogDebug("after suspend complete: %" PRId64 ".", context.currentTextureMemoryUsageInBytes());
  return RT_OK;
}

rtError pxScene2d::resume(const rtValue& /*v*/, bool& b)
{
  mSuspended = false;
  b = true;
  ENTERSCENELOCK()
  mRoot->reloadData(false);
  EXITSCENELOCK()
  mDirty = true;
  return RT_OK;
}

rtError pxScene2d::suspended(bool &b)
{
  b = mSuspended;
  return RT_OK;
}

rtError pxScene2d::textureMemoryUsage(rtValue &v)
{
  uint64_t textureMemory = 0;
  std::vector<rtObject*> objectsCounted;
  textureMemory += mRoot->textureMemoryUsage(objectsCounted);
  v.setUInt64(textureMemory);
  return RT_OK;
}

rtError pxScene2d::thunderToken(rtValue &v)
{
#ifdef ENABLE_SPARK_THUNDER
  v.setString("");
  unsigned char tokenBuffer[MAX_TOKEN_BUFFER_LENGTH];
  memset(tokenBuffer, 0, MAX_TOKEN_BUFFER_LENGTH);
  rtString appUrl = mScriptView != NULL ? mScriptView->getUrl() : "";
  if (!appUrl.isEmpty())
  {
    appUrl = rtUrlGetOrigin(appUrl.cString());
  }
  rtString params = "{\"url\":\"" + appUrl;
  params += "\"}";
  size_t paramsLength = (size_t)params.length();
  if(!memcpy(tokenBuffer,params.cString(),paramsLength))
  {
    rtLogError("unable to copy url buffer for token");
    return RT_FAIL;
  }
  rtLogInfo("thunder request: %s length: %d", (char*)tokenBuffer, (int)paramsLength);
  int result = GetToken(MAX_TOKEN_BUFFER_LENGTH, paramsLength, tokenBuffer);
  if (result < 0)
  {
    rtLogError("unable to get token for app");
    return RT_FAIL;
  }
  rtString tokenString = tokenBuffer;
  v.setString(tokenString);
  return RT_OK;
#else
  rtLogWarn("thunder support is not available");
  return RT_FAIL;
#endif //ENABLE_SPARK_THUNDER
}

rtError pxScene2d::clock(double & time)
{
  time = pxMilliseconds();

  return RT_OK;
}
rtError pxScene2d::createExternal(rtObjectRef p, rtObjectRef& o)
{
#if defined(ENABLE_DFB) || defined(DISABLE_WAYLAND)
  rtRef<pxViewContainer> c = new pxViewContainer(this);
  mTestView = new testView;
  c->setView(mTestView);
  o = c.getPtr();
  o.set(p);
  // Add a text label of the app type which would be displayed if extnerals was enabled
  rtObjectRef text = new pxText(this);
  rtObjectRef textProps = new rtMapObject;
  rtString externalLabel = "App: ";
  rtValue appName;
  p->Get("cmd", &appName);
  if (appName.toString().isEmpty())
  {
    p->Get("server", &appName);
    if (!appName.toString().isEmpty())
    {
      externalLabel.append("Web App");
    }
    else
    {
      externalLabel.append("Unknown Type");
    }
  }
  else
  {
    externalLabel.append(appName.toString());
  }
  textProps.set("text", externalLabel);
  textProps.set("pixelSize", 22);
  textProps.set("y", 10);
  textProps.set("x", 10);
  textProps.set("textColor", 0x000000ff);
  text.set(textProps);
  rtValue value(c);
  text->Set("parent", &value);
  o.send("init");
  return RT_OK;
#else
  if (false == gWaylandAppsConfigLoaded)
  {
    populateWaylandAppsConfig();
#ifndef PXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG
    gWaylandAppsMap.insert(gWaylandRegistryAppsMap.begin(), gWaylandRegistryAppsMap.end());
#endif // !defined PXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG
    gWaylandAppsConfigLoaded = true;
  }
#ifdef PXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG
  gWaylandAppsMap.clear();
  gWaylandAppsMap.insert(gWaylandRegistryAppsMap.begin(), gWaylandRegistryAppsMap.end());
  populateAllAppsConfig();
  gWaylandAppsMap.insert(gPxsceneWaylandAppsMap.begin(), gPxsceneWaylandAppsMap.end());
  /*for(std::map<string,string>::iterator it = gWaylandAppsMap.begin(); it != gWaylandAppsMap.end(); ++it) {
   rtLogDebug("key: %s !!!!!", it->first.c_str());
  }*/
#endif
  rtRef<pxWaylandContainer> c = new pxWaylandContainer(this);
  c->setView(new pxWayland(true, this));
  o = c.getPtr();
  o.set(p);
  o.send("init");
  return RT_OK;
#endif //ENABLE_DFB
}

rtError pxScene2d::createWayland(rtObjectRef p, rtObjectRef& o)
{
  rtLogWarn("Type 'wayland' is deprecated; use 'external' instead.\n");
  UNUSED_PARAM(p);
  return this->createExternal(p, o);
}

rtError pxScene2d::createWebGL(rtObjectRef p, rtObjectRef& o)
{
#ifdef ENABLE_SPARK_WEBGL
  o = new pxWebgl(this);
  o.set(p);
  o.send("init");
  return RT_OK;
#else
  rtLogError("Type 'webgl' is not supported");
  return RT_FAIL;
#endif //ENABLE_SPARK_WEBGL
}

rtError pxScene2d::createVideo(rtObjectRef p, rtObjectRef& o)
{
#ifdef ENABLE_SPARK_VIDEO
  o = new pxVideo(this);
  o.set(p);
  o.send("init");
  return RT_OK;
#else
  UNUSED_PARAM(p);
  UNUSED_PARAM(o);

  rtLogError("Type 'video' is not supported");
  return RT_FAIL;
#endif //ENABLE_SPARK_VIDEO
}

void pxScene2d::draw()
{
#ifdef DEBUG_SKIP_DRAW
#warning " 'DEBUG_SKIP_DRAW' is Enabled"
  return;
#endif

  double __frameStart = pxMilliseconds();

  //rtLogInfo("pxScene2d::draw()\n");
  if (gDirtyRectsEnabled) {
      pxRect dirtyRectangle = mDirtyRect;
      dirtyRectangle.unionRect(mLastFrameDirtyRect);
      int x = dirtyRectangle.left();
      int y = dirtyRectangle.top();
      int w = dirtyRectangle.right() - x+1;
      int h = dirtyRectangle.bottom() - y+1;

      static bool previousShowDirtyRect = false;

      if (mShowDirtyRectangle || previousShowDirtyRect || !mEnableDirtyRectangles)
      {
        context.enableDirtyRectangles(false);
      }

      if (mTop)
      {
        if (mShowDirtyRectangle || !mEnableDirtyRectangles)
        {
          context.enableClipping(false);
          context.clear(mWidth, mHeight);
        }
        else
        {
          context.clear(x, y, w, h);
        }
      }

      if (mRoot)
      {
        context.pushState();

    ENTERSCENELOCK()
        mRoot->drawInternal(true);
    EXITSCENELOCK()
        context.popState();
        mLastFrameDirtyRect.setLTRB(mDirtyRect.left(), mDirtyRect.top(), mDirtyRect.right(), mDirtyRect.bottom());
        mDirtyRect.setEmpty();
      }

      if (mTop && mShowDirtyRectangle)
      {
        /*pxMatrix4f identity;
          identity.identity();
          pxMatrix4f currentMatrix = context.getMatrix();
          context.setMatrix(identity);*/
          float red[]= {1,0,0,1};
          bool showOutlines = context.showOutlines();
          context.setShowOutlines(true);
          context.drawDiagRect(x, y, w, h, red);
          context.setShowOutlines(showOutlines);
          //context.setMatrix(currentMatrix);
          context.enableClipping(true);
      }
      previousShowDirtyRect = mShowDirtyRectangle;

  } else {

      if (mTop)
      {
        context.clear(mWidth, mHeight);
      }

      if (mRoot)
      {
        pxMatrix4f m;
        context.pushState();
    ENTERSCENELOCK()
        mRoot->drawInternal(true); // mask it !
    EXITSCENELOCK()
        context.popState();
      }
  }

  #ifdef USE_SCENE_POINTER
  if (mPointerTexture.getPtr() == NULL)
  {
    mPointerTexture= ((rtImageResource*)mPointerResource.getPtr())->getTexture();
    if (mPointerTexture.getPtr() != NULL)
    {
      mPointerW = mPointerTexture->width();
      mPointerH = mPointerTexture->height();
    }
  }
  if ( (mPointerTexture.getPtr() != NULL) &&
       !mPointerHidden )
  {
     context.drawImage( mPointerX-mPointerHotSpotX, mPointerY-mPointerHotSpotY,
                        mPointerW, mPointerH,
                        mPointerTexture, mNullTexture);
  }
#endif //USE_SCENE_POINTER

double __frameEnd = pxMilliseconds();

static double __frameTotal = 0;

__frameTotal = __frameTotal + (__frameEnd-__frameStart);

static int __frameCount = 0;
__frameCount++;
if (__frameCount > 60*5)
{
  rtLogDebug("avg frame draw duration(ms): %f\n", __frameTotal/__frameCount);
  __frameTotal = 0;
  __frameCount = 0;
}


}

std::map<pxObject*, pxObject*> gUpdateObjects;

void pxScene2d::updateObject(pxObject* o, bool update)
{
  if (!mOptimizedUpdateEnabled)
  {
    return;
  }
  if (update)
  {
    gUpdateObjects[o] = o;
  }
  else
  {
    gUpdateObjects.erase(o);
  }
}

void pxScene2d::updateObjects(double t)
{
  std::map<pxObject*, pxObject*>::const_iterator it;
  for (it=gUpdateObjects.begin(); it!=gUpdateObjects.end();)
  {
    pxObject* obj = (*it).second;
    obj->update(t, false);
    if (!obj->needsUpdate())
    {
      it = gUpdateObjects.erase(it);
    }
    else
    {
      it++;
    }

  }
}

void pxScene2d::enableOptimizedUpdate(bool enable)
{
  if (!enable)
  {
    gUpdateObjects.clear();
  }
  mOptimizedUpdateEnabled = enable;
  rtLogInfo("Optimized update enabled: %s", enable ? "true":"false");
}

void pxScene2d::onUpdate(double t)
{
  if (mDisposed)
  {
    return;
  }
  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateEnter();
  }
  #endif //ENABLE_RT_NODE
  // TODO if (mTop) check??
 // pxTextureCacheObject::checkForCompletedDownloads();
  //pxFont::checkForCompletedDownloads();

  // Dispatch various tasks on the main UI thread
  if (gUIThreadQueue)
  {
    gUIThreadQueue->process(0.01);
  }

  if (start == 0)
  {
    start = pxSeconds();
  }

  double start_frame = pxSeconds(); //##
  if (mOptimizedUpdateEnabled)
  {
    static double lastTime = 0;
    if (mTop || lastTime != t)
    {
      lastTime = t;
      updateObjects(t);
    }
  }
  else
  {
    update(t);
  }

  sigma_update += (pxSeconds() - start_frame); //##

  if (mDirty)
  {
    mDirty = false;
    if (mContainer)
      mContainer->invalidateRect(NULL);
  }
  // TODO get rid of mTop somehow
  if (mTop)
  {
    unsigned int target_frame_ms = 60;
    int targetFPS = static_cast<int> ((1.0 / ((double) target_frame_ms)) * 1000);

    if (frameCount >= targetFPS)
    {
      end2 = pxSeconds();

    int fps = (int)rint((double)frameCount/(end2-start));

#ifdef USE_RENDER_STATS
      double   dpf = rint( (double) gDrawCalls    / (double) frameCount ); // e.g.   glDraw*()           - calls per frame
      double   bpf = rint( (double) gTexBindCalls / (double) frameCount ); // e.g.   glBindTexture()     - calls per frame
      double   fpf = rint( (double) gFboBindCalls / (double) frameCount ); // e.g.   glBindFramebuffer() - calls per frame

      // TODO:  update / render times need some work...

      // double draw_ms   = ( (double) sigma_draw     / (double) frameCount ) * 1000.0f; // Average frame  time
      // double update_ms = ( (double) sigma_update   / (double) frameCount ) * 1000.0f; // Average update time

      // rtLogDebug("%g fps   pxObjects: %d   Draw: %g   Tex: %g   Fbo: %g     draw_ms: %0.04g   update_ms: %0.04g\n",
      //     fps, pxObjectCount, dpf, bpf, fpf, draw_ms, update_ms );

      rtLogDebug("%g fps   pxObjects: %d   Draw: %g   Tex: %g   Fbo: %g \n", fps, pxObjectCount, dpf, bpf, fpf);

      gDrawCalls    = 0;
      gTexBindCalls = 0;
      gFboBindCalls = 0;

      sigma_draw   = 0;
      sigma_update = 0;
#else
    static int previousFps = 60;
    //only log fps if there is a change to avoid log flooding
    if (previousFps != fps)
    {
      if (fps < fpsWarningThreshold && previousFps >= fpsWarningThreshold )
      {
        rtLogWarn("pxScene fps: %d  (below warn threshold of %d)", fps, fpsWarningThreshold);
      }
      else if (fps < fpsWarningThreshold)
      {
        rtLogDebug("pxScene fps: %d", fps);
      }
      else if (previousFps < fpsWarningThreshold)
      {
        rtLogWarn("pxScene fps: %d (above warn threshold of %d)", fps, fpsWarningThreshold);
      }
    }
    previousFps = fps;
    rtLogDebug("%d fps   pxObjects: %d\n", fps, pxObjectCount);
#endif //USE_RENDER_STATS
    if (mReportFps)
    {
#ifdef ENABLE_RT_NODE
      rtWrapperSceneUnlocker unlocker;
#endif //ENABLE_RT_NODE

      rtObjectRef e = new rtMapObject;
      e.set("fps", fps);
      mEmit.send("onFPS", e);
    }

    start = end2; // start of frame
    frameCount = 0;
  }

  frameCount++;
  }

  // Periodically let's poke the onMouseMove handler with the current pointer position
  // to better handle objects that animate in or out from under the mouse cursor
  // eg. scrolling
  if (t-mPointerLastUpdated > 0.2) // every 0.2 seconds
  {
    updateMouseEntered();
    mPointerLastUpdated = t;
  }

  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateExit();
  }
  #endif //ENABLE_RT_NODE
}

void pxScene2d::onDraw()
{
//  rtLogDebug("**** drawing \n");

  if (mTop)
  {
    context.updateRenderTick();
    #ifdef ENABLE_RT_NODE
    rtWrapperSceneUpdateEnter();
    #endif //ENABLE_RT_NODE
    context.setSize(mWidth, mHeight);
  }
#if 1

#ifdef USE_RENDER_STATS
  double start_draw = pxSeconds(); //##
#endif //USE_RENDER_STATS

  draw();

#ifdef USE_RENDER_STATS
  sigma_draw += (pxSeconds() - start_draw); //##
#endif //USE_RENDER_STATS

#endif
  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateExit();
  }
  #endif //ENABLE_RT_NODE
}

// Does not draw updates scene to time t
// t is assumed to be monotonically increasing
void pxScene2d::update(double t)
{
  if (mRoot)
  {
    if (gDirtyRectsEnabled) {
      context.pushState();
      }

      if( mCustomAnimator != NULL ) {
          mCustomAnimator->Send( 0, NULL, NULL );
      }

#ifndef DEBUG_SKIP_UPDATE
      mRoot->update(t);
#else
      UNUSED_PARAM(t);
#endif

    if (gDirtyRectsEnabled) {
      context.popState();
    }
  }
}

pxObject* pxScene2d::getRoot() const
{
  return mRoot;
}

rtObjectRef pxScene2d::getInfo() const
{
  return mInfo;
}

rtObjectRef pxScene2d::getCapabilities() const
{
  return mCapabilityVersions;
}

void pxScene2d::onComplete()
{
  rtObjectRef e = new rtMapObject;
  e.set("name", "onComplete");
  mEmit.send("onComplete", e);
}

void pxScene2d::onSize(int32_t w, int32_t h)
{
#if 0
  if (mTop)
    context.setSize(w, h);
#endif

  mWidth  = w;
  mHeight = h;

  mRoot->set("w", w);
  mRoot->set("h", h);

  rtObjectRef e = new rtMapObject;
  e.set("name", "onResize");
  e.set("w", w);
  e.set("h", h);
  mEmit.send("onResize", e);

#if 0 // JRJR... this shouldn't crash
  if (mContainer)
    mContainer->invalidateRect(NULL);
#endif
}

bool pxScene2d::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDown");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    mEmit.send("onMouseDown", e);
  }
#endif
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(static_cast<float>(x),static_cast<float>(y)), hitPt;
    //    pt.x = x; pt.y = y;
    rtRef<pxObject> hit;

    if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      mMouseDown = hit;
      // scene coordinates
      mMouseDownPt.x = static_cast<float>(x);
      mMouseDownPt.y = static_cast<float>(y);

      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseDown");
      e.set("target", hit.getPtr());
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
      e.set("flags", flags);
      #if 0
      hit->mEmit.send("onMouseDown", e);
      #else
      bubbleEvent(e,hit,"onPreMouseDown","onMouseDown");
      #endif

      setMouseEntered(hit);
    }
    else
      setMouseEntered(NULL);
  }
  return false;
}


bool pxScene2d::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseUp");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    mEmit.send("onMouseUp", e);
  }
#endif
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(static_cast<float>(x),static_cast<float>(y)), hitPt;
    rtRef<pxObject> hit;
    //rtRef<pxObject> tMouseDown = mMouseDown;

    if (mMouseDown)
    {
      // Since onClick is a proper subset of onMouseUp fire first so
      // that appropriate action can be taken in this case.

      // TODO optimization... we really only need to check mMouseDown
      if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
      {
        // Send onClick if this object got an onMouseDown
        if (mMouseDown == hit)
        {
          rtObjectRef e = new rtMapObject;
          e.set("name", "onClick");
          e.set("target",hit.getPtr());
          e.set("x", hitPt.x); // In object local coordinates
          e.set("y", hitPt.y);
          e.set("flags", flags);
          bubbleEvent(e,hit,"onPreClick","onClick");
        }
        setMouseEntered(hit);
      }

      pxVector4f from(static_cast<float>(x),static_cast<float>(y),0,1);
      pxVector4f to;
      pxObject::transformPointFromSceneToObject(mMouseDown, from, to);

      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseUp");
      e.set("target",mMouseDown.getPtr());
      e.set("x", to.x()); // In object local coordinates
      e.set("y", to.y());
      e.set("flags", flags);
      bubbleEvent(e,mMouseDown,"onPreMouseUp","onMouseUp");

      mMouseDown = NULL;
    }
    else
      setMouseEntered(NULL);
  }
  return false;
}

#if 0
bool pxScene2d::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseUp");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    mEmit.send("onMouseUp", e);
  }
#endif
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(static_cast<float>(x),static_cast<float>(y)), hitPt;
    rtRef<pxObject> hit;
    rtRef<pxObject> tMouseDown = mMouseDown;

    mMouseDown = NULL;

    // TODO optimization... we really only need to check mMouseDown
    if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      // Only send onMouseUp if this object got an onMouseDown -- WHY???
//      if (tMouseDown == hit)
      {
        rtObjectRef e = new rtMapObject;
        e.set("name", "onMouseUp");
        e.set("target",hit.getPtr());
        e.set("x", hitPt.x);
        e.set("y", hitPt.y);
        e.set("flags", flags);
        #if 0
        hit->mEmit.send("onMouseUp", e);
        #else
        bubbleEvent(e,hit,"onPreMouseUp","onMouseUp");
        #endif
      }

      setMouseEntered(hit);
    }
    else
      setMouseEntered(NULL);
  }
  return false;
}
#endif

// TODO rtRef doesn't like non-const !=
// JRJR what are the x and y for?
void pxScene2d::setMouseEntered(rtRef<pxObject> o, int32_t x /* = 0*/, int32_t y /* = 0*/)
{
  if (mMouseEntered != o)
  {
    // Tell old object we've left
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseLeave");
      e.set("target", mMouseEntered.getPtr());
      e.set("x", x);
      e.set("y", y);

      bubbleEvent(e,mMouseEntered,"onPreMouseLeave","onMouseLeave");
    }
    mMouseEntered = o;

    // Tell new object we've entered
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseEnter");
      e.set("target", mMouseEntered.getPtr());
      bubbleEvent(e,mMouseEntered, "onPreMouseEnter", "onMouseEnter");
    }
  }
}

void pxScene2d::clearMouseObject(rtRef<pxObject> obj)
{
  if (mMouseEntered == obj)
  {
    mMouseEntered = NULL;
  }
}

/** This function is not exposed to javascript; it is called when
 * mFocus = true is set for a pxObject whose parent scene is this scene
 **/
rtError pxScene2d::setFocus(rtObjectRef o)
{
  rtLogDebug("pxScene2d::setFocus");
  rtObjectRef focusObj;
  if (o)
  {
    focusObj = o;
  }
  else
  {
    focusObj = getRoot();
  }

  if(mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    ((pxObject*)mFocusObj.get<voidPtr>("_pxObject"))->setFocusInternal(false);
    e.set("target",mFocusObj);
    rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    //t->mEmit.send("onBlur",e);
    rtRef<pxObject> u = (pxObject*)focusObj.get<voidPtr>("_pxObject");
    bubbleEventOnBlur(e,t,u);
  }

  mFocusObj = focusObj;

  rtObjectRef e = new rtMapObject;
  ((pxObject*)mFocusObj.get<voidPtr>("_pxObject"))->setFocusInternal(true);
  e.set("target",mFocusObj);
  rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
  //t->mEmit.send("onFocus",e);
  bubbleEvent(e,t,"onPreFocus","onFocus");

  return RT_OK;
}

bool pxScene2d::onMouseEnter()
{
  return false;
}

bool pxScene2d::onMouseLeave()
{
  // top level scene event
  #if 0
  rtObjectRef e = new rtMapObject;
  e.set("name", "onMouseLeave");
  mEmit.send("onMouseLeave", e);
  #endif

  // Don't change here if dragging
  if (!mMouseDown.getPtr())
  {
    mMouseDown = NULL;
  }
  setMouseEntered(NULL);

  return false;
}

bool pxScene2d::onFocus()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onFocus");
  mEmit.send("onFocus", e);
  return false;
}

bool pxScene2d::onBlur()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onBlur");
  mEmit.send("onBlur", e);
  return false;
}

bool gStopPropagation;
rtError stopPropagation2(int /*numArgs*/, const rtValue* /*args*/, rtValue* /*result*/, void* ctx)
{
  bool& stopProp = *(bool*)ctx;
  stopProp = true;
  return RT_OK;
}

bool pxScene2d::bubbleEvent(rtObjectRef e, rtRef<pxObject> t,
                            const char* preEvent, const char* event)
{
  bool consumed = false;
  mStopPropagation = false;
  rtValue stop;
  if (e && t)
  {
    AddRef();  // TODO refactor? make sure scene stays alive while we bubble since we're using the address of mStopPropagation
//    e.set("stopPropagation", get<rtFunctionRef>("stopPropagation"));
    e.set("stopPropagation", new rtFunctionCallback(stopPropagation2, (void*)&mStopPropagation));

    vector<rtRef<pxObject> > l;
    while(t)
    {
      l.push_back(t);
      t = t->parent();
    }

//    rtLogDebug("before %s bubble\n", preEvent);
    e.set("name", preEvent);
    vector<rtRef<pxObject> >::reverse_iterator itReverseEnd = l.rend();
    for (vector<rtRef<pxObject> >::reverse_iterator it = l.rbegin();!mStopPropagation && it != itReverseEnd;++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      if (emit)
        emit.sendReturns(preEvent,e,stop);
      if (mStopPropagation)
        break;
    }
//    rtLogDebug("after %s bubble\n", preEvent);

//    rtLogDebug("before %s bubble\n", event);
    e.set("name", event);
    vector<rtRef<pxObject> >::iterator itEnd = l.end();
    for (vector<rtRef<pxObject> >::iterator it = l.begin();!mStopPropagation && it != itEnd;++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      // TODO: As we bubble onMouseMove we need to keep adjusting the coordinates into the
      // coordinate space of the successive parents object ??
      // JRJR... not convinced on this comment please discus with me first.
      if (emit)
        emit.sendReturns(event,e,stop);
//      rtLogDebug("mStopPropagation %d\n", mStopPropagation);
      if (mStopPropagation)
      {
        rtLogDebug("Event bubble aborted\n");
        break;
      }
    }
//    rtLogDebug("after %s bubble\n", event);
    consumed = mStopPropagation;
    Release();
  }
  return consumed;
}

bool pxScene2d::bubbleEventOnBlur(rtObjectRef e, rtRef<pxObject> t, rtRef<pxObject> o)
{
  bool consumed = false;
  mStopPropagation = false;
  rtValue stop;
  if (e && t)
  {
    AddRef();
    e.set("stopPropagation", new rtFunctionCallback(stopPropagation2, (void*)&mStopPropagation));

    vector<rtRef<pxObject> > l;
    while(t)
    {
      l.push_back(t);
      t = t->parent();
    }

    vector<rtRef<pxObject> > m;
    while(o)
    {
      m.push_back(o);
      o = o->parent();
    }

    // Walk through object hierarchy starting from root for t (object losing focus) and o (object getting focus) to
    // find index (loseFocusChainIdx) of first common parent.
    unsigned long loseFocusChainIdx = l.size();
    vector<rtRef<pxObject> >::reverse_iterator it_l = l.rbegin();
    vector<rtRef<pxObject> >::reverse_iterator it_lEnd = l.rend();
    vector<rtRef<pxObject> >::reverse_iterator it_m = m.rbegin(); // traverse the hierarchy of object getting focus in REVERSE starting with the top most parent
    vector<rtRef<pxObject> >::reverse_iterator it_mEnd  = m.rend();
    while((it_l != it_lEnd) && (it_m != it_mEnd) && (*it_l == *it_m))
    {
      loseFocusChainIdx--;
      it_l++;
      it_m++;
    }

    //    rtLogDebug("before %s bubble\n", preEvent);
    e.set("name", "onPreBlur");
    vector<rtRef<pxObject> >::reverse_iterator it_reverseEnd = l.rend();
    for (vector<rtRef<pxObject> >::reverse_iterator it = l.rbegin();!mStopPropagation && it != it_reverseEnd;++it)
    {
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      if (emit)
        emit.sendReturns("onPreBlur",e,stop);
    }
    //    rtLogDebug("after %s bubble\n", preEvent);

    //    rtLogDebug("before %s bubble\n", event);
    e.set("name", "onBlur");
    for (unsigned long i = 0;!mStopPropagation && i < l.size();i++)
    {
      rtFunctionRef emit = l[i]->mEmit.getPtr();
      if (emit)
      {
        // For range [0,loseFocusChainIdx),loseFocusChain is true
        // For range [loseFocusChainIdx,l.size()),loseFocusChain is false

        //if(!l[i]->id().isEmpty())
        //  rtLogDebug("\nSetting loseFocusChain for %s",l[i]->id().cString());

        if(i < loseFocusChainIdx)
          e.set("loseFocusChain",rtValue(true));
        else
          e.set("loseFocusChain",rtValue(false));

        emit.sendReturns("onBlur",e,stop);
      }
    }
    //    rtLogDebug("after %s bubble\n", event);
    consumed = mStopPropagation;
    Release();
  }
  return consumed;

}


bool pxScene2d::onMouseMove(int32_t x, int32_t y)
{
  mPointerX= x;
  mPointerY= y;
  #ifdef USE_SCENE_POINTER
  // JRJR this should be passing mouse cursor bounds in rather than dirty entire scene
  invalidateRect(NULL);
  mDirty= true;
  #endif
#if 1
  {
    // Send to root scene in global window coordinates
    // Used to send to child scenes for event propogation
    // Always non translated events
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("x", x);  // Sent in global scene
    e.set("y", y);
    mEmit.send("onMouseMove", e);
  }
#endif

#if 1
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt(static_cast<float>(x),static_cast<float>(y)), hitPt;
  rtRef<pxObject> hit;

  if (mMouseDown)
  {
    {
      pxVector4f from(static_cast<float>(x),static_cast<float>(y),0,1);
      pxVector4f to;
      pxObject::transformPointFromSceneToObject(mMouseDown, from, to);

//      to.dump();
      #if 0
      {
        pxVector4f validate;
        pxObject::transformPointFromObjectToScene(mMouseDown, to, validate);
        if (fabs(validate.x()-(float)x)> 0.01 ||
            fabs(validate.y()-(float)y) > 0.01)
        {
          rtLogInfo("Error in point transformation (%d,%d) != (%f,%f); (%f, %f)",
                 x,y,validate.x(),validate.y(),to.x(),to.y());
        }
      }

      {
        pxVector4f validate;
        pxObject::transformPointFromObjectToObject(mMouseDown, mMouseDown, to, validate);
        if (fabs(validate.x()-(float)to.x())> 0.01 ||
            fabs(validate.y()-(float)to.y()) > 0.01)
        {
          rtLogInfo("Error in point transformation (o2o) (%f,%f) != (%f,%f)",
                 to.x(),to.y(),validate.x(),validate.y());
        }
      }
      #endif

#if 0
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("target", mMouseDown.getPtr());
    e.set("x", to.mX);
    e.set("y", to.mY);
    mMouseDown->mEmit.send("onMouseMove", e);
#else
    rtObjectRef e = new rtMapObject;
    e.set("target", mMouseDown.getPtr());
    e.set("x", to.x());
    e.set("y", to.y());
    bubbleEvent(e, mMouseDown, "onPreMouseMove", "onMouseMove");
#endif
    }
    {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDrag");
    e.set("target", mMouseDown.getPtr());
    e.set("x", x);
    e.set("y", y);
    e.set("startX", mMouseDownPt.x);
    e.set("startY", mMouseDownPt.y);
#if 0
    mMouseDown->mEmit.send("onMouseDrag", e);
#else
    bubbleEvent(e,mMouseDown,"onPreMouseDrag","onMouseDrag");
#endif
    }
  }
  else // Only send mouse leave/enter events if we're not dragging
  {
    if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      // This probably won't stay ... we can probably send onMouseMove to the child scene level
      // rather than the object... we can send objects enter/leave events
      // and we can send drag events to objects that are being drug...
#if 1
      rtObjectRef e = new rtMapObject;
//      e.set("name", "onMouseMove");
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
#if 0
      hit->mEmit.send("onMouseMove",e);
#else
      bubbleEvent(e, hit, "onPreMouseMove", "onMouseMove");
#endif
#endif

      setMouseEntered(hit);
    }
    else
    {
      setMouseEntered(NULL);
    }
  }
#endif
#if 0
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt;
  pt.x = x; pt.y = y;
  rtRef<pxObject> hit;

  if (mRoot && (mRoot->hitTestInternal(m, pt, hit)))
  {
    rtString id = hit->get<rtString>("id");
    rtLogDebug("found object id: %s\n", id.isEmpty()?"none":id.cString());
  }
#endif
  return false;
}

void pxScene2d::updateMouseEntered()
{
  if (mDisposed)
  {
    return;
  }
  #if 1
    pxMatrix4f m;
    pxPoint2f pt(static_cast<float>(mPointerX),static_cast<float>(mPointerY)), hitPt;
    rtRef<pxObject> hit;
    if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      setMouseEntered(hit);
    }
    else
      setMouseEntered(NULL);
  #endif
}

bool pxScene2d::onDragMove(int32_t x, int32_t y, int32_t type)
{
  pxMatrix4f m;
  rtRef<pxObject> hit;
  pxPoint2f pt(static_cast<float>(x),static_cast<float>(y)), hitPt;

  if (mRoot && mRoot->hitTestInternal(m, pt, hit, hitPt))
  {
    mDragType = (pxConstantsDragType::constants) type;

    {
      rtObjectRef e = new rtMapObject;

      e.set("name", "onDragMove");
      e.set("target", hit.getPtr());

      e.set("x",       hitPt.x); // TODO - should really be the local coordinates of the point of "leave"-ing drop target
      e.set("y",       hitPt.y); // TODO - should really be the local coordinates of the point of "leave"-ing drop target

      e.set("screenX", x);
      e.set("screenY", y);

      e.set("type",    mDragType);  // TODO:  Change to "dataTransfer" object + MIME types

      bubbleEvent(e, hit, "onPreDragMove", "onDragMove");
    }

    if(mDragTarget != hit) // a new Drag Target...
    {
      // LEAVE old target
      if(mDragTarget)
      {
        rtObjectRef e = new rtMapObject;

        e.set("name", "onDragLeave");
        e.set("target", hit.getPtr());

        e.set("x",       x );
        e.set("y",       y );

        e.set("screenX", x);
        e.set("screenY", y);

        bubbleEvent(e, mDragTarget, "onPreDragLeave", "onDragLeave");
      }
      {
        mDragTarget = hit;  // ENTER new target

        rtObjectRef e = new rtMapObject;

        e.set("name", "onDragEnter");
        e.set("target", hit.getPtr());

        e.set("x",       hitPt.x);
        e.set("y",       hitPt.y);

        e.set("screenX", x);
        e.set("screenY", y);

        bubbleEvent(e, hit, "onPreDragEnter", "onDragEnter");
      }
    }
  }
  else
  {
    mDragTarget = NULL;
  }

  return false;
}

bool pxScene2d::onDragEnter(int32_t x, int32_t y, int32_t type)
{
  UNUSED_PARAM(x); UNUSED_PARAM(y);

  mDragType = (pxConstantsDragType::constants) type;
  mDragging = true;
  return false;
}

bool pxScene2d::onDragLeave(int32_t x, int32_t y, int32_t type)
{
  UNUSED_PARAM(x); UNUSED_PARAM(y); UNUSED_PARAM(type);

  mDragging = false;
  return false;
}

bool pxScene2d::onDragDrop(int32_t x, int32_t y, int32_t type, const char *dropped)
{
  pxConstantsDragType::constants dragType = (pxConstantsDragType::constants) type;

  if (mDragTarget)
  {
    mDragging = false;

    pxVector4f from(static_cast<float>(x),static_cast<float>(y),0,1);
    pxVector4f to;
    pxObject::transformPointFromSceneToObject(mDragTarget, from, to);

    rtObjectRef e = new rtMapObject;
    e.set("name",    "onDragDrop");
    e.set("target",  mDragTarget.getPtr());
    e.set("x",       to.x());
    e.set("y",       to.y());
    e.set("screenX", x);
    e.set("screenY", y);
    e.set("type",    dragType);  // TODO:  Change to "dataTransfer" object + MIME types
    e.set("dropped", dropped);

    return bubbleEvent(e, mDragTarget, "onPreDragDrop", "onDragDrop");
  }

  return false;
}

bool pxScene2d::onScrollWheel(float dx, float dy)
{
  if (mMouseEntered)
  {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onScrollWheel");
    e.set("target", mMouseEntered.getPtr());
    e.set("dx", dx);
    e.set("dy", dy);

    return bubbleEvent(e, mMouseEntered, "onPreScrollWheel", "onScrollWheel");
  }
  return false;
}


bool pxScene2d::onKeyDown(uint32_t keyCode, uint32_t flags)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreKeyDown", "onKeyDown");
  }
  return false;
}

bool pxScene2d::onKeyUp(uint32_t keyCode, uint32_t flags)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreKeyUp", "onKeyUp");
  }
  return false;
}

bool pxScene2d::onChar(uint32_t c)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("charCode", c);
    rtRef<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreChar", "onChar");
  }
  return false;
}

rtError pxScene2d::showOutlines(bool& v) const
{
  v=context.showOutlines();
  return RT_OK;
}

rtError pxScene2d::setShowOutlines(bool v)
{
  context.setShowOutlines(v);

  onSize(mRoot->getOnscreenWidth(), mRoot->getOnscreenHeight());

  return RT_OK;
}

rtError pxScene2d::showDirtyRect(bool& v) const
{
  v=mShowDirtyRectangle;
  return RT_OK;
}
rtError pxScene2d::setShowDirtyRect(bool v)
{
  mShowDirtyRectangle = v;
  return RT_OK;
}

rtError pxScene2d::reportFps(bool& v) const
{
  v=mReportFps;
  return RT_OK;
}

rtError pxScene2d::setReportFps(bool v)
{
  mReportFps = v;
  return RT_OK;
}

rtError pxScene2d::dirtyRectanglesEnabled(bool& v) const {
    v = gDirtyRectsEnabled;
    return RT_OK;
}

rtError pxScene2d::dirtyRectangle(rtObjectRef& v) const {
  v = new rtMapObject();
  if (gDirtyRectsEnabled) {
    v.set("x1", mDirtyRect.left());
    v.set("y1", mDirtyRect.top());
    v.set("x2", mDirtyRect.right());
    v.set("y2", mDirtyRect.bottom());
  }
  return RT_OK;
}

rtError pxScene2d::enableDirtyRect(bool& v) const
{
    v=mEnableDirtyRectangles;
    return RT_OK;
}

rtError pxScene2d::setEnableDirtyRect(bool v)
{
    mEnableDirtyRectangles = v;
    rtLogInfo("enable dirty rectangles: %s", mEnableDirtyRectangles ? "true":"false");
    return RT_OK;
}

rtError pxScene2d::customAnimator(rtFunctionRef& v) const
{
  v = mCustomAnimator;
  return RT_OK;
}

rtError pxScene2d::setCustomAnimator(const rtFunctionRef& v)
{
  static bool customAnimatorSupportEnabled = false;

  //check for custom animator enabled support only once
  static bool checkForCustomAnimatorSupport = true;
  if (checkForCustomAnimatorSupport)
  {
    char const *s = getenv("PXSCENE_ENABLE_CUSTOM_ANIMATOR");
    if (s)
    {
      int animatorSetting = atoi(s);
      if (animatorSetting > 0)
      {
        customAnimatorSupportEnabled = true;
      }
    }
    checkForCustomAnimatorSupport = false;
  }

  if (customAnimatorSupportEnabled)
  {
    mCustomAnimator = v;
    return RT_OK;
  }
  else
  {
    rtLogError("custom animator support is not available");
    return RT_FAIL;
  }
}

rtError pxScene2d::screenshot(rtString type, rtValue& returnValue)
{
  returnValue = "";
#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows("screenshot", rtPermissions::FEATURE))
    return RT_ERROR_NOT_ALLOWED;
#endif

  // Is this a type we support?
  if (type != "image/png;base64" && type != "image/image")
  {
    return RT_FAIL;
  }

  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  pxContextFramebufferRef newFBO;
  // w/o multisampling
  // if needed, render texture of a multisample FBO to a non-multisample FBO and then read from it
  mRoot->createSnapshot(newFBO, false, false);
  context.setFramebuffer(newFBO);
  pxOffscreen o;
  context.snapshot(o);
  context.setFramebuffer(previousRenderSurface);

//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK
#if 0
  FILE *myFile = fopen("/mnt/nfs/env/snap.png", "wb");
  if( myFile != NULL)
  {
    fwrite( pngData2.data(), sizeof(char), pngData2.length(),myFile);
    fclose(myFile);
  }
#endif
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK

  if (type == "image/png;base64")
  {
    rtData pngData2;
    if (pxStorePNGImage(o, pngData2) != RT_OK)
    {
      return RT_FAIL;
    }

    rtString base64coded;

    if (base64_encode(pngData2, base64coded) == RT_OK)
    {
      // We return a data Url string containing the image data
      rtString pngData = "data:image/png;base64,";

      pngData += base64coded;

//        FILE *saveFile  = fopen("/var/tmp/snap.txt", "wt"); // base64
//        fwrite( base64coded.cString(), base64coded.length(), sizeof(char), saveFile);
//        fclose(saveFile);
//
//        FILE *inFile  = fopen("/var/tmp/snap.txt", "rt"); // base64
//        if( inFile != NULL)
//        {
//          fseek(inFile, 0L, SEEK_END);
//          size_t sz = ftell(inFile);
//          fseek(inFile, 0L, SEEK_SET);
//
//          rtData base64in; base64in.init(sz);
//          fread(base64in.data(), base64in.length(), 1, inFile);
//          fclose(inFile);
//
//          rtString my64string( (const char* ) base64in.data(), base64in.length());
//
//          rtData pngData2;
//
//          rtError res = base64_decode(my64string, pngData2);
//
//          if(res == RT_OK)
//          {
//            FILE *outFile = fopen("/var/tmp/snap.png", "wb"); // PNG
//
//            if(outFile)
//            {
//              fwrite( pngData2.data(), pngData2.length(), sizeof(char), outFile);
//              fclose(outFile);
//            }
//          }
//        }
      returnValue = pngData;

      return RT_OK;

    }//ENDIF
  }
  else if (type == "image/image")
  {
    pxImage* image = new pxImage(this);
    image->createWithOffscreen(o);
    returnValue = image;
    return RT_OK;
  }

  return RT_FAIL;
}

rtError pxScene2d::clipboardSet(rtString type, rtString clipString)
{
//    rtLogDebug("\n ##########   clipboardSet()  >> %s ", type.cString() ); fflush(stdout);

    pxClipboard::instance()->setString(type.cString(), clipString.cString());

    return RT_OK;
}

rtError pxScene2d::clipboardGet(rtString type, rtString &retString)
{
//    rtLogDebug("\n ##########   clipboardGet()  >> %s ", type.cString() ); fflush(stdout);
    std::string retVal = pxClipboard::instance()->getString(type.cString());

    retString = rtString(retVal.c_str());

    return RT_OK;
}

rtError pxScene2d::getService(rtString name, rtObjectRef& returnObject)
{
  rtLogDebug("inside getService");
  returnObject = NULL;

  // Create context from requesting scene
  rtObjectRef ctx = new rtMapObject();
  rtObjectRef o;
  ctx.set("url", mScriptView != NULL ? mScriptView->getUrl() : "");
  pxSceneContainer * container = dynamic_cast<pxSceneContainer*>(mContainer);
  if( container != NULL)  {
    container->serviceContext(o);
  }
  ctx.set("serviceContext", o);

#ifdef ENABLE_PERMISSIONS_CHECK
  rtValue permissionsValue = mPermissions.getPtr();
  ctx.set("permissions", permissionsValue);
#endif //ENABLE_PERMISSIONS_CHECK

  returnObject = NULL;
  getService(name, ctx, returnObject);
  return RT_OK;
}

// todo change rtString to const char*
rtError pxScene2d::getService(const char* name, const rtObjectRef& ctx, rtObjectRef& service)
{
  rtLogDebug("inside getService internal");
  static pxScene2d* reentered = NULL;

  // Only query this scene  if we're not already in the middle of querying this scene
  if (reentered != this)
  {
    for (std::vector<rtFunctionRef>::iterator i = mServiceProviders.begin(); i != mServiceProviders.end(); i++)
    {
      rtValue result;
      rtError e;

      reentered = this;
      e = (*i).sendReturns<rtValue>(name, ctx, result);
      reentered = NULL;

      if (e == RT_OK)
      {
        if (result.getType() == RT_stringType)
        {
          rtString access = result.toString();
          // denied stop searching for service
          if ((access == "deny") || (access == "DENY"))
          {
            rtLogDebug("service denied");
            return RT_FAIL;
            break;
          }
          // if not explicitly allowed then break
          if (!((access == "allow") || (access == "ALLOW")))
          {
            rtLogDebug("unknown access string - denied");
            return RT_FAIL;
            break;
          }
          // otherwise keep on looking
        }
        else if (result.getType() == RT_objectType)
        {
          rtObjectRef o = result.toObject();
          if (o)
          {
              service = o;
              return RT_OK;
          }
          else
          {
            // if object reference is null don't keep looking. service provider must explicitly allow.
            break;
          }
        }
        else
        {
          // unexpected result from service provider stop searching for service.
          break;
        }
      }
    }
  }

  // See if the view's container can provide the service
  rtRef<rtIServiceProvider> serviceProvider;
  if (mContainer)
  {
    serviceProvider = (rtIServiceProvider*)(mContainer->getInterface("serviceProvider"));
  }
  if (serviceProvider)
  {
    if (serviceProvider->getService(name, ctx, service) == RT_OK)
    {
      return RT_OK;
    }
    else
      return RT_FAIL;
  }
  else
  {
    // TODO JRJR should move this to top level container only...

    rtLogInfo("trying to get service for name: %s", name);
  #ifdef PX_SERVICE_MANAGER
    rtError result = RT_OK;
    #ifdef ENABLE_PERMISSIONS_CHECK
    rtPermissionsRef serviceCheckPermissions = mPermissions;
    rtValue permissionsValue;
    if (ctx.get("permissions", permissionsValue) == RT_OK)
    {
      rtObjectRef permissionsRef;
      if (permissionsValue.getObject(permissionsRef) == RT_OK)
      {
        serviceCheckPermissions = permissionsRef;
      }
    }
    if (serviceCheckPermissions != NULL && RT_OK != serviceCheckPermissions->allows(name, rtPermissions::SERVICE))
    {
      rtLogWarn("does not have permissions to check the service manager for %s", name);
    }
    else
    #endif //ENABLE_PERMISSIONS_CHECK
    {
      rtObjectRef serviceManager;
      result = pxServiceManager::findServiceManager(serviceManager);
      if (result != RT_OK)
      {
        rtLogWarn("service manager not found");
      }
      else
      {
        result = serviceManager.sendReturns<rtObjectRef>("createService", mScriptView != NULL ? mScriptView->getUrl() : "", name, service);
        rtLogInfo("create %s service result: %d", name, result);
      }
    }

    if (result != RT_OK || service.getPtr() == NULL)
    {
      //if not found, search for a rtRemote object with the given name
      rtLogInfo("searching rtRemote for %s", name);
      #ifdef ENABLE_PERMISSIONS_CHECK
      rtPermissionsRef rtRemoteCheckPermissions = serviceCheckPermissions;
      if (rtRemoteCheckPermissions != NULL && RT_OK != rtRemoteCheckPermissions->allows(name, rtPermissions::RTREMOTE))
      {
        rtLogInfo("permission to access rtRemote for %s not allowed", name);
        return RT_ERROR_NOT_ALLOWED;
      }
      else
      #endif //ENABLE_PERMISSIONS_CHECK
      {
        rtObjectRef rtRemoteObject;
        result = pxServiceManager::findRtRemoteObject(name, rtRemoteObject);
        if (result != RT_OK)
        {
          rtLogWarn("rtRemote object %s not found", name);
        }
        else
        {
          rtLogInfo("rtRemote object %s found", name);
          service = rtRemoteObject;
        }
      }
    }
    return result;
  #else
    rtLogInfo("service manager not supported");
    return RT_FAIL;
  #endif //PX_SERVICE_MANAGER
  }
}

rtError pxScene2d::getAvailableApplications(rtString& availableApplications)
{
  availableApplications = "";
#if defined(ENABLE_DFB) || defined(DISABLE_WAYLAND)
  rtLogWarn("wayland apps are not supported");
#else
  if (false == gWaylandAppsConfigLoaded)
  {
    populateWaylandAppsConfig();
#ifndef PXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG
    gWaylandAppsMap.insert(gWaylandRegistryAppsMap.begin(), gWaylandRegistryAppsMap.end());
#endif // !defined PXSCENE_ENABLE_ALL_APPS_WAYLAND_CONFIG
    gWaylandAppsConfigLoaded = true;
  }
  populateAllAppDetails(availableApplications);
#endif
  return RT_OK;
}

rtError pxScene2d::storage(rtObjectRef& v) const
{
#ifdef PXSCENE_SUPPORT_STORAGE
  if (!mStorage)
  {
    rtString origin(mScriptView != NULL ? rtUrlGetOrigin(mScriptView->getUrl().cString()) : NULL);
    if (origin.isEmpty())
      origin = "file://";

    uint32_t storageQuota = 0;
#ifdef ENABLE_PERMISSIONS_CHECK
    mPermissions->getStorageQuota(storageQuota);
#endif
    if( storageQuota == 0)
    {
      rtLogWarn("Origin %s has no local storage quota", origin.cString());
      return RT_OK;
    }

    rtString storagePath;
    rtValue storagePathVal;
    if (RT_OK == rtSettings::instance()->value("defaultStoragePath", storagePathVal))
    {
      storagePath = storagePathVal.toString();
    }
    else
    {
      // runtime location, if available
      const char* env = getenv(DEFAULT_LOCALSTORAGE_DIR_ENV_NAME);
      if (env)
        storagePath = env;
    }
    if (storagePath.isEmpty())
    {
      // default location
      if (RT_OK == rtGetHomeDirectory(storagePath))
        storagePath.append(DEFAULT_LOCALSTORAGE_DIR);
    }

    rtEnsureTrailingPathSeparator(storagePath);

    // Create the path if it doesn't yet exist
    if (!rtMakeDirectory(storagePath))
    {
      rtLogWarn("creation of storage directory %s failed", storagePath.cString());
      return RT_OK;
    }

    rtString storageName = rtUrlEscape(origin);
    storagePath.append(storageName);
    rtLogInfo("storage path: %s", storagePath.cString());

    mStorage = new rtStorage(storagePath, storageQuota, origin);
  }

  v = mStorage;
  return RT_OK;
#else
  UNUSED_PARAM(v);
  rtLogInfo("storage not supported");
  return RT_FAIL;
#endif
}

rtDefineObject(scriptViewShadow, rtObject);
rtDefineMethod(scriptViewShadow, addListener);
rtDefineMethod(scriptViewShadow, delListener);

rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineProperty(pxScene2d, info);
rtDefineProperty(pxScene2d, capabilities);
rtDefineProperty(pxScene2d, w);
rtDefineProperty(pxScene2d, h);
rtDefineProperty(pxScene2d, showOutlines);
rtDefineProperty(pxScene2d, showDirtyRect);
rtDefineProperty(pxScene2d, reportFps);
rtDefineProperty(pxScene2d, dirtyRectangle);
rtDefineProperty(pxScene2d, dirtyRectanglesEnabled);
rtDefineProperty(pxScene2d, enableDirtyRect);
rtDefineProperty(pxScene2d, customAnimator);
rtDefineMethod(pxScene2d, create);
rtDefineMethod(pxScene2d, clock);
rtDefineMethod(pxScene2d, logDebugMetrics);
rtDefineMethod(pxScene2d, collectGarbage);
rtDefineMethod(pxScene2d, suspend);
rtDefineMethod(pxScene2d, resume);
rtDefineMethod(pxScene2d, suspended);
rtDefineMethod(pxScene2d, textureMemoryUsage);
rtDefineMethod(pxScene2d, thunderToken);
//rtDefineMethod(pxScene2d, createWayland);
rtDefineMethod(pxScene2d, addListener);
rtDefineMethod(pxScene2d, delListener);
rtDefineMethod(pxScene2d, getFocus);
//rtDefineMethod(pxScene2d, stopPropagation);
rtDefineMethod(pxScene2d, screenshot);

rtDefineMethod(pxScene2d, clipboardGet);
rtDefineMethod(pxScene2d, clipboardSet);
rtDefineMethod(pxScene2d, getService);
rtDefineMethod(pxScene2d, getAvailableApplications);

rtDefineMethod(pxScene2d, loadArchive);
rtDefineProperty(pxScene2d, ctx);
rtDefineProperty(pxScene2d, api);
//rtDefineProperty(pxScene2d, emit);
// Properties for access to Constants
rtDefineProperty(pxScene2d,animation);
rtDefineProperty(pxScene2d,stretch);
rtDefineProperty(pxScene2d,maskOp);
rtDefineProperty(pxScene2d,dragType);
rtDefineProperty(pxScene2d,alignVertical);
rtDefineProperty(pxScene2d,alignHorizontal);
rtDefineProperty(pxScene2d,truncation);
rtDefineMethod(pxScene2d, dispose);

#ifdef ENABLE_PERMISSIONS_CHECK
rtDefineProperty(pxScene2d, permissions);
#endif
rtDefineMethod(pxScene2d, sparkSetting);
rtDefineProperty(pxScene2d, cors);
rtDefineMethod(pxScene2d, addServiceProvider);
rtDefineMethod(pxScene2d, removeServiceProvider);
rtDefineProperty(pxScene2d, storage);

rtError pxScene2dRef::Get(const char* name, rtValue* value) const
{
  return (*this)->Get(name, value);
}

rtError pxScene2dRef::Get(uint32_t i, rtValue* value) const
{
  return (*this)->Get(i, value);
}

rtError pxScene2dRef::Set(const char* name, const rtValue* value)
{
  return (*this)->Set(name, value);
}

rtError pxScene2dRef::Set(uint32_t i, const rtValue* value)
{
  return (*this)->Set(i, value);
}

void RT_STDCALL testView::onUpdate(double /*t*/)
{
  if (mContainer)
    mContainer->invalidateRect(NULL);
}

void RT_STDCALL testView::onDraw()
{
//  rtLogInfo("testView::onDraw()");
  float white[] = {1,1,1,1};
  float black[] = {0,0,0,1};
  float red[]= {1,0,0,1};
  float green[] = {0,1,0,1};
  context.drawRect(mw, mh, 1, mEntered?green:red, white);
  context.drawDiagLine(0,static_cast<float>(mMouseY),mw,static_cast<float>(mMouseY),black);
  context.drawDiagLine(static_cast<float>(mMouseX),0,static_cast<float>(mMouseX),mh,black);
}

void pxViewContainer::invalidateRect(pxRect* r)
{
  if (mScene)
  {
    mScene->mDirty = true;
  }
  repaint();
  pxObject* parent = this->parent();
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
  if (mScene)
  {
    if (gDirtyRectsEnabled) {
        pxRect screenRect = convertToScreenCoordinates(r);
        mScene->invalidateRect(&screenRect);
        setDirtyRect(r);
    } else {
        mScene->invalidateRect(NULL);
        UNUSED_PARAM(r);
    }
  }
}

void pxScene2d::invalidateRect(pxRect* r)
{
  if (gDirtyRectsEnabled) {
      if (r != NULL)
      {
        mDirtyRect.unionRect(*r);
        mDirty = true;
      }
  } else {
    UNUSED_PARAM(r);
  }
  if (mContainer && !mTop)
  {
    if (gDirtyRectsEnabled) {
        mContainer->invalidateRect(mDirty ? &mDirtyRect : NULL);
    } else {
        mContainer->invalidateRect(NULL);
    }
  }
}

bool pxScene2d::isObjectTracked(rtObjectRef ref)
{
    bool isTracked = false;
    unsigned int pos = 0;
    for (; pos<mInnerpxObjects.size(); pos++)
    {
      if (mInnerpxObjects[pos] == ref)
      {
        isTracked = true;
        break;
      }
    }
    return isTracked;
}

void pxScene2d::innerpxObjectDisposed(rtObjectRef ref)
{
  // this is to make sure, we are not clearing the rtobject references, while it is under process from scene dispose
  if (!mDisposed)
  {
    unsigned int pos = 0;
    for (; pos<mInnerpxObjects.size(); pos++)
    {
      if (mInnerpxObjects[pos] == ref)
        break;
    }
    if (pos != mInnerpxObjects.size())
    {
      mInnerpxObjects.erase(mInnerpxObjects.begin()+pos);
    }
  }
}

rtError pxScene2d::sparkSetting(const rtString& setting, rtValue& value) const
{
  rtValue val;
  if (RT_OK != rtSettings::instance()->value(setting, val))
  {
    value = rtValue();
    return RT_OK;
  }
  value = val;
  return RT_OK;
}

void pxScene2d::setViewContainer(pxIViewContainer* l)
{
  mContainer = l;
#ifdef ENABLE_PERMISSIONS_CHECK
  pxObject* obj = dynamic_cast<pxObject*>(l);
  if (obj != NULL && obj->getScene())
  {
    // rtPermissions accounts parent scene permissions too
    mPermissions->setParent(obj->getScene()->mPermissions);
  }
#endif
}

pxIViewContainer* pxScene2d::viewContainer()
{
  return mContainer;
}

rtDefineObject(pxViewContainer, pxObject);
rtDefineProperty(pxViewContainer, w);
rtDefineProperty(pxViewContainer, h);
rtDefineMethod(pxViewContainer, onMouseDown);
rtDefineMethod(pxViewContainer, onMouseUp);
rtDefineMethod(pxViewContainer, onMouseMove);

rtDefineMethod(pxViewContainer, onDragMove);
rtDefineMethod(pxViewContainer, onDragEnter);
rtDefineMethod(pxViewContainer, onDragLeave);
rtDefineMethod(pxViewContainer, onDragDrop);

rtDefineMethod(pxViewContainer, onScrollWheel);
rtDefineMethod(pxViewContainer, onMouseEnter);
rtDefineMethod(pxViewContainer, onMouseLeave);
rtDefineMethod(pxViewContainer, onFocus);
rtDefineMethod(pxViewContainer, onBlur);
rtDefineMethod(pxViewContainer, onKeyDown);
rtDefineMethod(pxViewContainer, onKeyUp);
rtDefineMethod(pxViewContainer, onChar);

rtDefineObject(pxSceneContainer, pxViewContainer);
rtDefineProperty(pxSceneContainer, url);
#ifdef ENABLE_PERMISSIONS_CHECK
rtDefineProperty(pxSceneContainer, permissions);
#endif
rtDefineProperty(pxSceneContainer, cors);
rtDefineProperty(pxSceneContainer, api);
rtDefineProperty(pxSceneContainer, ready);
rtDefineProperty(pxSceneContainer, serviceContext);
rtDefineMethod(pxSceneContainer, suspend);
rtDefineMethod(pxSceneContainer, resume);
rtDefineMethod(pxSceneContainer, screenshot);
//rtDefineMethod(pxSceneContainer, makeReady);   // DEPRECATED ?


rtError pxSceneContainer::setUrl(rtString url)
{
  rtLogDebug("pxSceneContainer::setUrl(%s)",url.cString());

#ifdef ENABLE_PERMISSIONS_CHECK
  if (mScene != NULL && RT_OK != mScene->permissions()->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  // If old promise is still unfulfilled resolve it
  // and create a new promise for the context of this Url
  mReady.send("resolve", this);
  mReady = new rtPromise();
  triggerUpdate();

  mUrl = url;
#ifdef RUNINMAIN
    setScriptView(new pxScriptView(url.cString(), "", this));
#else
    pxScriptView * scriptView = new pxScriptView(url.cString(),"", this);
    AsyncScriptInfo * info = new AsyncScriptInfo();
    info->m_pView = scriptView;
    //info->m_pWindow = this;
    uv_mutex_lock(&moreScriptsMutex);
    scriptsInfo.push_back(info);
    uv_mutex_unlock(&moreScriptsMutex);
    uv_async_send(&asyncNewScript);
    setScriptView(scriptView);
#endif

  return RT_OK;
}

rtError pxSceneContainer::api(rtValue& v) const
{
//  return mScene->api(v);
  if (mScriptView)
    return mScriptView->api(v);
  else
    return RT_FAIL;
}

rtError pxSceneContainer::ready(rtObjectRef& o) const
{
  rtLogDebug("pxSceneContainer::ready\n");
  if (mScriptView) {
    rtLogDebug("mScriptView is set!\n");
    return mScriptView->ready(o);
  }
  rtLogInfo("mScriptView is NOT set!\n");
  return RT_FAIL;
}

rtError pxSceneContainer::setServiceContext(rtObjectRef o)
{
  // Only allow serviceContext to be set at construction time
  if( !mInitialized)
    mServiceContext = o;

  return RT_OK;
}

rtError pxSceneContainer::suspend(const rtValue& v, bool& b)
{
  b = false;
  pxScriptView* scriptView = dynamic_cast<pxScriptView*>(mView.getPtr());
  if (scriptView != NULL)
  {
    return scriptView->suspend(v, b);
  }
  return RT_OK;
}

rtError pxSceneContainer::resume(const rtValue& v, bool& b)
{
  b = false;
  pxScriptView* scriptView = dynamic_cast<pxScriptView*>(mView.getPtr());
  if (scriptView != NULL)
  {
    return scriptView->resume(v, b);
  }
  return RT_OK;
}

rtError pxSceneContainer::screenshot(rtString type, rtValue& returnValue)
{
  pxScriptView* scriptView = dynamic_cast<pxScriptView*>(mView.getPtr());
  if (scriptView != NULL)
  {
    return scriptView->screenshot(type, returnValue);
  }
  return RT_FAIL;
}

rtError pxSceneContainer::setScriptView(pxScriptView* scriptView)
{
  mScriptView = scriptView;
  setView(scriptView);
  return RT_OK;
}

void pxSceneContainer::dispose(bool pumpJavascript)
{
  if (!mIsDisposed)
  {
    rtLogInfo(__FUNCTION__);
    //Adding ref to make sure, object not destroyed from event listeners
    AddRef();
    setScriptView(NULL);
    pxObject::dispose(pumpJavascript);
    Release();
  }
}

  void* pxSceneContainer::getInterface(const char* name)
  {
    if (strcmp(name, "serviceProvider") == 0)
    {
      return (rtIServiceProvider*)mScene;
    }
    return NULL;
  }

void pxSceneContainer::releaseData(bool sceneSuspended)
{
  if (mScriptView.getPtr())
  {
    rtValue v;
    bool result;
    mScriptView->suspend(v, result);
  }
  pxObject::releaseData(sceneSuspended);
}

void pxSceneContainer::reloadData(bool sceneSuspended)
{
  if (mScriptView.getPtr())
  {
    rtValue v;
    bool result;
    mScriptView->resume(v, result);
  }
  pxObject::reloadData(sceneSuspended);
}

uint64_t pxSceneContainer::textureMemoryUsage(std::vector<rtObject*> &objectsCounted)
{
  uint64_t textureMemory = 0;
  if (std::find(objectsCounted.begin(), objectsCounted.end(), this) == objectsCounted.end() )
  {
    if (mScriptView.getPtr())
    {
      rtValue v;
      mScriptView->textureMemoryUsage(v);
      textureMemory += v.toUInt64();
    }
    textureMemory += pxObject::textureMemoryUsage(objectsCounted);
  }
  return textureMemory;
}

#ifdef ENABLE_PERMISSIONS_CHECK
rtError pxSceneContainer::permissions(rtObjectRef& v) const
{
  if (mScriptView.getPtr())
  {
    return mScriptView->permissions(v);
  }
  v = NULL;
  return RT_OK;
}

rtError pxSceneContainer::setPermissions(const rtObjectRef& v)
{
  if (mScriptView.getPtr())
  {
    return mScriptView->setPermissions(v);
  }
  return RT_OK;
}
#endif

rtError pxSceneContainer::cors(rtObjectRef& v) const
{
  if (mScriptView.getPtr())
  {
    return mScriptView->cors(v);
  }
  v = NULL;
  return RT_OK;
}

#if 0
void* gObjectFactoryContext = NULL;
objectFactory gObjectFactory = NULL;
void registerObjectFactory(objectFactory f, void* context)
{
  gObjectFactory = f;
  gObjectFactoryContext = context;
}

rtError createObject2(const char* t, rtObjectRef& o)
{
  return gObjectFactory(gObjectFactoryContext, t, o);
}
#endif

int contextId = 1;

pxScriptView::pxScriptView(const char* url, const char* /*lang*/, pxIViewContainer* container)
     : mWidth(-1), mHeight(-1), mDrawing(false), mSharedContext(), mViewContainer(container), mRefCount(0)
{
  rtLogDebug("pxScriptView::pxScriptView()entering\n");

// escape url begin
  string escapedUrl;
  string origUrl = url;
  for (string::iterator it=origUrl.begin(); it!=origUrl.end(); ++it)
  {
    char currChar = *it;
    if ((currChar == '"') || (currChar == '\\'))
    {
      escapedUrl.append(1, '\\');
    }
    escapedUrl.append(1, currChar);
  }
  mUrl = escapedUrl.c_str();
  if (mUrl.length() > MAX_URL_SIZE)
  {
    rtLogWarn("url size greater than 8000 bytes, so restting url to empty");
    mUrl = "";
  }
// escape url end

  shadow = new scriptViewShadow;

  mReady = new rtPromise();

  #ifndef RUNINMAIN // NOTE this ifndef ends after runScript decl, below
  triggerUpdate();
 // mLang = lang;
  rtLogDebug("pxScriptView::pxScriptView() exiting\n");
#else
  runScript();
#endif // ifndef RUNINMAIN
}

void pxScriptView::runScript()
{
  rtLogDebug(__FUNCTION__);

  #ifdef ENABLE_RT_NODE

  if (rtUrlGetExtension(mUrl).compare(".spark") == 0)
  {
    if (!mBootstrap)
    {
      if (!mBootstrapResolve && !mBootstrapReject)
      {
        mBootstrapResolve = new rtFunctionCallback(bootstrapResolve, this);
        mBootstrapReject = new rtFunctionCallback(bootstrapReject, this);

        rtRef<pxArchive> a = new pxArchive;

        // get rid of the query part
        rtString archiveUrl = mUrl;
        int32_t pos = archiveUrl.find(0, '#');
        if (pos != -1) {
          archiveUrl = archiveUrl.substring(0, pos);
        }
        pos = archiveUrl.find(0, '?');
        if (pos != -1) {
          archiveUrl = archiveUrl.substring(0, pos);
        }

        a->initFromUrl(archiveUrl);
        rtObjectRef ready; // rtPromise
        a->ready(ready);
        rtObjectRef newPromise;
        ready.sendReturns<rtObjectRef>("then", mBootstrapResolve.getPtr(), mBootstrapReject.getPtr(), newPromise);
      }
      return;
    }
  }

  rtLogDebug("pxScriptView::pxScriptView is just now creating a context for mUrl=%s\n",mUrl.cString());
  //mCtx = script.createContext("javascript");
  script.createContext("javascript", mCtx);

  if (mCtx)
  {
    mGetScene = new rtFunctionCallback(getScene,  this);
    mMakeReady = new rtFunctionCallback(makeReady, this);
    mGetContextID = new rtFunctionCallback(getContextID, this);
    mGetSetting = new rtFunctionCallback(getSetting, this);

    mCtx->add("getScene", mGetScene.getPtr());
    mCtx->add("makeReady", mMakeReady.getPtr());
    mCtx->add("getContextID", mGetContextID.getPtr());
    mCtx->add("getSetting", mGetSetting.getPtr());

    // JRJR Temporary webgl integration
    if (isGLUrl())
    {
      mSharedContext = context.createSharedContext(true);
      mBeginDrawing = new rtFunctionCallback(beginDrawing2, this);
      mEndDrawing = new rtFunctionCallback(endDrawing2, this);
      if (mSparkHttp.getPtr() == NULL)
      {
        mSparkHttp = new rtFunctionCallback(sparkHttp, NULL);
      }
      //mCtx->add("view", this);

      // JRJR TODO initially with zero mWidth/mHeight until onSize event
      // defer to onSize once events have been ironed out
      int width = 1280;
      int height = 720;
      if (mUrl.find(0, "enableSparkGL1080") >= 0)
      {
        rtLogInfo("enabling 1080 SparkGL app");
        width = 1920;
        height = 1080;
      }
      mSharedContext->makeCurrent(true);
      cached = context.createFramebuffer(width,height,false,false,true);
      mSharedContext->makeCurrent(false);

      beginDrawing();
      glClearColor(0, 0, 0, 0);
      glClear(GL_COLOR_BUFFER_BIT);
      // compile initGL.js
      if (mSparkGlInitApp.isEmpty())
      {
        rtString s = getenv("SPARK_PATH");
        s.append("initApp.js");
        rtData initData;
        rtError e = rtLoadFile(s.cString(), initData);
        if(e != RT_OK)
        {
          rtLogError("Failed to load - 'initApp.js' ");
        }
        mSparkGlInitApp = rtString((char*)initData.data(), (uint32_t) initData.length());
      }
      mCtx->runScript(mSparkGlInitApp.cString());
      rtValue foo = mCtx->get("loadAppUrl");
      rtFunctionRef f = foo.toFunction();
      bool b = true;

      // JRJR Adding an AddRef to this... causes bad things to happen when reloading gl scenes
      // investigate...
      // JRJR WARNING! must use sendReturns since wrappers will invoke asyncronously otherwise.
      f.sendReturns<bool>(mUrl,mBeginDrawing.getPtr(),mEndDrawing.getPtr(), shadow.getPtr(), mBootstrap, mSparkHttp.getPtr(), b);
      endDrawing();

    }
    else
    {
      // compile init.js
      if (mSparkInitApp.isEmpty())
      {
        rtString s = getenv("SPARK_PATH");
        s.append("init.js");
        rtData initData;
        rtError e = rtLoadFile(s.cString(), initData);

        if (e != RT_OK)
          rtLogError("Failed to load file: %s", s.cString());

        mSparkInitApp = rtString((char*)initData.data(), (uint32_t)initData.length());
      }
      mCtx->runScript(mSparkInitApp.cString());

      rtString url = mUrl;
      if (mBootstrap)
      {
        url = mBootstrap.get<rtString>("applicationURL");
      }

      char buffer[MAX_URL_SIZE + 50];
      memset(buffer, 0, sizeof(buffer));
      snprintf(buffer, sizeof(buffer), "loadUrl(\"%s\");", url.cString());
      rtLogDebug("pxScriptView::runScript calling runScript with %s\n",url.cString());
  #ifdef WIN32 // process \\ to /
      unsigned int bufferLen = strlen(buffer);
      char * newBuffer = (char*)malloc(sizeof(char)*(bufferLen + 1));
      unsigned int newBufferLen = 0;
      for (size_t i = 0; i < bufferLen - 1; i++) {
        if (buffer[i] == '\\') {
          newBuffer[newBufferLen++] = '/';
          if (buffer[i + 1] == '\\') {
            i = i + 1;
          }
        }
        else {
          newBuffer[newBufferLen++] = buffer[i];
        }
      }
      newBuffer[newBufferLen++] = '\0';
      strcpy(buffer, newBuffer);
      free(newBuffer);
  #endif
      mCtx->runScript(buffer);
      rtLogDebug("pxScriptView::runScript() ending\n");
    }
//#endif
  }
  #endif //ENABLE_RT_NODE
}

pxScriptView::~pxScriptView()
{
  rtLogDebug("~pxScriptView for mUrl=%s\n",mUrl.cString());
  // Clear out these references since the script context
  // can outlive this view
#ifdef ENABLE_RT_NODE
  if(mCtx)
  {
    mGetScene->clearContext();
    mMakeReady->clearContext();
    mGetContextID->clearContext();

    if (mBootstrapResolve)
      mBootstrapResolve->clearContext();
    if (mBootstrapReject)
      mBootstrapReject->clearContext();

    // TODO Given that the context is being cleared we likely don't need to zero these out
    mCtx->add("getScene", 0);
    mCtx->add("makeReady", 0);
    mCtx->add("getContextID", 0);
  }

  if (mDrawing) {
    context.setFramebuffer(previousSurface);
    mSharedContext->makeCurrent(false);
  }
  mDrawing = false;

  if (NULL != mBeginDrawing.getPtr())
    mBeginDrawing->clearContext();
  if (NULL != mEndDrawing.getPtr())
    mEndDrawing->clearContext();

#endif //ENABLE_RT_NODE

  if (mView)
    mView->setViewContainer(NULL);

  // TODO JRJR Do we have GC tests yet
  // Hack to try and reduce leaks until garbage collection can
  // be cleaned up

  shadow->emit()->clearListeners();

  // JRJR TODO Not Releasing GL Context

  if(mScene)
    mEmit.send("onSceneRemoved", mScene);

  if (mScene)
    mScene.send("dispose");

  mView = NULL;
  mScene = NULL;
}

void pxScriptView::onSize(int32_t w, int32_t h)
{
  mWidth = w;
  mHeight = h;

  {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onResize");
    e.set("w", w);
    e.set("h", h);
    shadow->emit().send("onResize", e);
  }

  #if 0  // JRJR TODO Leave out until we get the resizing events ironed out for gl content
  if (mUrl == "triangle")
  {
    cached = context.createFramebuffer(mWidth,mHeight);
  }
  #endif

  if (mView)
    mView->onSize(w,h);
}

bool pxScriptView::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDown");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    shadow->emit().send("onMouseDown", e);
  }

  if (mView)
    return mView->onMouseDown(x,y,flags);

  return false;
}

bool pxScriptView::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseUp");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", static_cast<uint32_t>(flags));
    shadow->emit().send("onMouseUp", e);
  }
  if (mView)
    return mView->onMouseUp(x,y,flags);
  return false;
}

bool pxScriptView::onMouseMove(int32_t x, int32_t y)
{
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("x", x);
    e.set("y", y);
    shadow->emit().send("onMouseMove", e);
  }

  if (mView)
    return mView->onMouseMove(x,y);
  return false;
}

bool pxScriptView::onScrollWheel(float dx, float dy)
{
  {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onScrollWheel");
    e.set("dx", dx);
    e.set("dy", dy);
    shadow->emit().send("onScrollWheel");
  }

  if (mView)
    return mView->onScrollWheel(dx,dy);
  return false;
}

bool pxScriptView::onMouseEnter()
{
  if (mView)
    return mView->onMouseEnter();
  return false;
}

bool pxScriptView::onMouseLeave()
{
  if (mView)
    return mView->onMouseLeave();
  return false;
}

bool pxScriptView::onDragMove(int32_t x, int32_t y, int32_t type)
{
  if (mView)
    return mView->onDragMove(x, y, type);
  return false;
}

bool pxScriptView::onDragEnter(int32_t x, int32_t y, int32_t type)
{
  if (mView)
    return mView->onDragEnter(x, y, type);
  return false;
}

bool pxScriptView::onDragLeave(int32_t x, int32_t y, int32_t type)
{
  if (mView)
    return mView->onDragLeave(x, y, type);
  return false;
}

bool pxScriptView::onDragDrop(int32_t x, int32_t y, int32_t type, const char *dropped)
{
  if (mView)
    return mView->onDragDrop(x, y, type, dropped);
  return false;
}

bool pxScriptView::onFocus()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onFocus");
  shadow->emit().send("onFocus", e);

  if (mView)
    return mView->onFocus();
  return false;
}

bool pxScriptView::onBlur()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onBlur");
  shadow->emit().send("onBlur", e);

  if (mView)
    return mView->onBlur();
  return false;
}

bool pxScriptView::onKeyDown(uint32_t keycode, uint32_t flags)
{
  {
    rtObjectRef e = new rtMapObject;
    e.set("keyCode", keycode);
    e.set("flags", (uint32_t)flags);
    shadow->emit().send("onKeyDown", e);
  }

  if (mView)
    return mView->onKeyDown(keycode, flags);
  return false;
}

bool pxScriptView::onKeyUp(uint32_t keycode, uint32_t flags)
{
  {
    rtObjectRef e = new rtMapObject;
    e.set("keyCode", keycode);
    e.set("flags", (uint32_t)flags);
    shadow->emit().send("onKeyUp", e);
  }

  if (mView)
    return mView->onKeyUp(keycode,flags);
  return false;
}

bool pxScriptView::onChar(uint32_t codepoint)
{
  {
    rtObjectRef e = new rtMapObject;
    e.set("charCode", codepoint);
    shadow->emit().send("onChar", e);
  }
  if (mView)
    return mView->onChar(codepoint);
  return false;
}

void pxScriptView::onDraw(/*pxBuffer& b, pxRect* r*/)
{
  static pxTextureRef nullMaskRef;
  if (isGLUrl())
  {
    /* code */
    if (cached.getPtr() && cached->getTexture().getPtr())
    {
      context.drawImage(0, 0, cached->width(), cached->height(), cached->getTexture(), nullMaskRef);
    }
  }
  else
  {
    if (mView)
      mView->onDraw();
  }

}

rtError pxScriptView::suspend(const rtValue& v, bool& b)
{
  b = false;
  if (mScene)
  {
    mScene.sendReturns("suspend", v, b);
  }
  return RT_OK;
}

rtError pxScriptView::resume(const rtValue& v, bool& b)
{
  b = false;
  if (mScene)
  {
    mScene.sendReturns("resume", v, b);
  }
  return RT_OK;
}

rtError pxScriptView::textureMemoryUsage(rtValue& v)
{
  v = 0;
  if (mScene)
  {
    mScene.sendReturns("textureMemoryUsage",v);
  }
  return RT_OK;
}

rtError pxScriptView::screenshot(rtString type, rtValue& returnValue)
{
  if (mScene)
  {
    return mScene.sendReturns("screenshot",type, returnValue);
  }
  return RT_FAIL;
}

rtError pxScriptView::getScene(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  rtLogDebug(__FUNCTION__);
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    if (numArgs == 1)
    {
      rtString sceneType = args[0].toString();
      // JR Todo can specify what scene version/type to create in args
      if (!v->mScene)
      {
        pxScene2dRef scene = new pxScene2d(topSparkView, v);
        topSparkView = false;
        v->mView = scene;
        v->mScene = scene;

        v->mView->setViewContainer(v->mViewContainer);
        v->mView->onSize(v->mWidth,v->mHeight);
      }
      rtLogDebug("pxScriptView::getScene() Almost done \n");

      if (result)
      {
        *result = v->mScene;
        return RT_OK;
      }
    }
  }
  return RT_FAIL;
}


#if 1
rtError pxScriptView::getContextID(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* /*ctx*/)
{
  #if 0
  //rtLogInfo(__FUNCTION__);
  UNUSED_PARAM(numArgs);
  UNUSED_PARAM(args);

#ifdef ENABLE_RT_NODE
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    Locker                locker(v->mCtx->getIsolate());
    Isolate::Scope isolate_scope(v->mCtx->getIsolate());
    HandleScope     handle_scope(v->mCtx->getIsolate());

    Local<Context> ctx = v->mCtx->getLocalContext();
    uint32_t ctx_id = GetContextId( ctx );

    if (result)
    {
      *result = rtValue(ctx_id);
      return RT_OK;
    }
  }
#endif //ENABLE_RT_NODE

  return RT_FAIL;
  #else
  *result = 0;
  return RT_OK;
  #endif
}
#endif

void pxScriptView::beginDrawing()
{
  if (!mDrawing)
  {
    mDrawing = true;
    mSharedContext->makeCurrent(true);
    previousSurface = context.getCurrentFramebuffer();
    context.setFramebuffer(cached);
    return;
  }
  rtLogWarn("pxScriptView::beginDrawing() already entered");
}

void pxScriptView::endDrawing()
{
  if (mDrawing)
  {
    glViewport(0,0,1,1);
    glFlush();
    context.setFramebuffer(previousSurface);
    mSharedContext->makeCurrent(false);
    mViewContainer->invalidateRect(NULL);
    mDrawing = false;
    return;
  }
  rtLogWarn("pxScriptView::endDrawing() not currently drawing");
}

rtError pxScriptView::beginDrawing2(int /*numArgs*/, const rtValue* /*args*/, rtValue* /*result*/, void* ctx)
{
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;
    v->beginDrawing();
  }
  return RT_OK;
}

rtError pxScriptView::endDrawing2(int /*numArgs*/, const rtValue* /*args*/, rtValue* /*result*/, void* ctx)
{
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;
    v->endDrawing();
  }
  return RT_OK;
}

// JRJR could be made much simpler...
rtError pxScriptView::getSetting(int numArgs, const rtValue* args, rtValue* result, void* /*ctx*/)
{
  if (numArgs >= 1)
  {
    rtValue val;
    if (RT_OK != rtSettings::instance()->value(args[0].toString(), val))
    {
      *result = rtValue();
      return RT_OK;
    }
    *result = val;
    return RT_OK;
  }
  else
    return RT_ERROR_NOT_ENOUGH_ARGS;
}

rtError pxScriptView::makeReady(int numArgs, const rtValue* args, rtValue* /*result*/, void* ctx)
{
  rtLogDebug(__FUNCTION__);
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    if (numArgs >= 1)
    {
      bool success = false;
      if (args[0].toBool())
      {
        if (numArgs >= 2)
        {
          v->mApi = args[1].toObject();
        }
        success = true;
        v->mReady.send("resolve", v->mScene);
      }
      else
      {
        success = false;
        v->mReady.send("reject", new rtObject); // TODO JRJR  Why does this fail if I leave the argment as null...
      }

      rtValue urlValue(v->mUrl);
      mEmit.send("onSceneReady", v->mScene, urlValue, success);

      return RT_OK;
    }
  }
  return RT_FAIL;
}

rtError pxScriptView::bootstrapResolve(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  rtLogDebug("%s", __FUNCTION__);

  UNUSED_PARAM(result);

  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;
    if (numArgs < 1)
      return RT_FAIL;

    pxArchive* a = (pxArchive*)args[0].toObject().getPtr();

    rtString s;
    if (a->getFileAsString(NULL, s) != RT_OK
      || json2rtObject(s.cString(), v->mBootstrap) != RT_OK)
    {
      rtLogError("%s: can't get bootstrap", __FUNCTION__);
      return RT_FAIL;
    }

    v->runScript();
  }
  return RT_OK;
}

rtError pxScriptView::bootstrapReject(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  rtLogError("%s", __FUNCTION__);

  UNUSED_PARAM(numArgs);
  UNUSED_PARAM(args);
  UNUSED_PARAM(result);
  UNUSED_PARAM(ctx);

  return RT_OK;
}

bool pxScriptView::isGLUrl() const
{
  return mUrl.beginsWith("gl:")
    || (mBootstrap && mBootstrap.get<rtString>("frameworkType").compare("sparkGL") == 0);
}

rtError pxScriptView::sparkHttp(int numArgs, const rtValue* args, rtValue* result, void* /*ctx*/)
{
  if (numArgs < 1)
  {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rtHttpRequest* req;
  if (args[0].getType() == RT_stringType)
    req = new rtHttpRequest(args[0].toString());
  else if (args[0].getType() == RT_objectType)
    req = new rtHttpRequest(args[0].toObject());
  else
  {
    rtLogError("%s: invalid arg type", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (numArgs > 1 && args[1].getType() == RT_functionType)
    req->addListener("response", args[1].toFunction());

  rtObjectRef ref = req;
  *result = ref;

  return RT_OK;
}
