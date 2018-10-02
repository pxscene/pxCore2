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
#include "pxWindowUtil.h"

#include "pxRectangle.h"
#include "pxFont.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxImage.h"

#ifdef BUILD_WITH_PXPATH
#include "pxPath.h"
#endif

#ifdef PX_SERVICE_MANAGER
#include "pxServiceManager.h"
#endif //PX_SERVICE_MANAGER
#include "pxImage9.h"
#include "pxImageA.h"
#include "pxImage9Border.h"

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

#ifdef ENABLE_RT_NODE
#include "rtScript.h"
#endif //ENABLE_RT_NODE

using namespace rapidjson;

using namespace std;


#define xstr(s) str(s)
#define str(s) #s

#ifndef PX_SCENE_VERSION
#define PX_SCENE_VERSION dev_ver
#endif

// #define DEBUG_SKIP_DRAW       // Skip DRAW   code - for testing.
// #define DEBUG_SKIP_UPDATE     // Skip UPDATE code - for testing.

extern rtThreadQueue* gUIThreadQueue;
extern pxContext      context;

static int fpsWarningThreshold = 25;

rtEmitRef pxScriptView::mEmit = new rtEmit();

// Debug Statistics
#ifdef USE_RENDER_STATS

uint32_t gDrawCalls;
uint32_t gTexBindCalls;
uint32_t gFboBindCalls;

#endif //USE_RENDER_STATS

// TODO move to rt*
// Taken from
// http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

#include <stdint.h>
#include <stdlib.h>

#ifdef ENABLE_RT_NODE
extern void rtWrapperSceneUpdateEnter();
extern void rtWrapperSceneUpdateExit();
#ifdef RUNINMAIN
rtScript script;
#else
class AsyncScriptInfo;
extern vector<AsyncScriptInfo*> scriptsInfo;
extern uv_mutex_t moreScriptsMutex;
extern uv_async_t asyncNewScript;
extern uv_async_t gcTrigger;
#endif // RUNINMAIN
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
int pxObjectCount = 0;
bool gApplicationIsClosing = false;

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


// Small helper class that vends the children of a pxObject as a collection
class pxObjectChildren: public rtObject {
public:

  rtDeclareObject(pxObjectChildren, rtObject);

  pxObjectChildren(pxObject* o)
  {
    mObject = o;
  }

  virtual rtError Get(const char* name, rtValue* value) const
  {
    if (!value) return RT_FAIL;
    if (!strcmp(name, "length"))
    {
      value->setUInt32( (uint32_t) mObject->numChildren());
      return RT_OK;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

  virtual rtError Get(uint32_t i, rtValue* value) const
  {
    if (!value) return RT_FAIL;
    if (i < mObject->numChildren())
    {
      rtObjectRef o;
      rtError e = mObject->getChild(i, o);
      *value = o;
      return e;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(const char* name, const rtValue* value)
  {
    (void)name;
    (void)value;
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(uint32_t i, const rtValue* value)
  {
    (void)i;
    (void)value;
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

private:
  rtRef<pxObject> mObject;
};

rtDefineObject(pxObjectChildren, rtObject);


// pxObject methods
pxObject::pxObject(pxScene2d* scene): rtObject(), mParent(NULL), mpx(0), mpy(0), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0), 
#ifdef ANIMATION_ROTATE_XYZ
    mrx(0), mry(0), mrz(1.0),
#endif //ANIMATION_ROTATE_XYZ
    msx(1), msy(1), mw(0), mh(0),
    mInteractive(true),
    mSnapshotRef(), mPainting(true), mClip(false), mMask(false), mDraw(true), mHitTest(true), mReady(),
    mFocus(false),mClipSnapshotRef(),mCancelInSet(true),mUseMatrix(false), mRepaint(true)
#ifdef PX_DIRTY_RECTANGLES
    , mIsDirty(true), mRenderMatrix(), mScreenCoordinates(), mDirtyRect()
#endif //PX_DIRTY_RECTANGLES
    ,mDrawableSnapshotForMask(), mMaskSnapshot(), mIsDisposed(false), mSceneSuspended(false)
  {
    pxObjectCount++;
    mScene = scene;
    mReady = new rtPromise;
    mEmit = new rtEmit;
  }

pxObject::~pxObject()
{
//    rtString d;
    // TODO... why is this bad
//    sendReturns<rtString>("description",d);
    //rtLogDebug("**************** pxObject destroyed: %s\n",getMap()->className);
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      (*it)->mParent = NULL;  // setParent mutates the mChildren collection
    }
    mChildren.clear();
    pxObjectCount--;
    clearSnapshot(mSnapshotRef);
    clearSnapshot(mClipSnapshotRef);
    clearSnapshot(mDrawableSnapshotForMask);
    clearSnapshot(mMaskSnapshot);
    mSnapshotRef = NULL;
    mClipSnapshotRef = NULL;
    mDrawableSnapshotForMask = NULL;
    mMaskSnapshot = NULL;
}

void pxObject::sendPromise()
{
  if(mInitialized && !((rtPromise*)mReady.getPtr())->status())
  {
    mReady.send("resolve",this);
  }
}

void pxObject::createNewPromise()
{
  // Only create a new promise if the existing one has been
  // resolved or rejected already.
  if(((rtPromise*)mReady.getPtr())->status())
  {
    rtLogDebug("CREATING NEW PROMISE\n");
    mReady = new rtPromise();
  }
}

void pxObject::dispose(bool pumpJavascript)
{
  if (!mIsDisposed)
  {
    //rtLogInfo(__FUNCTION__);
    mIsDisposed = true;
    rtValue nullValue;
    vector<animation>::iterator it = mAnimations.begin();
    for(;it != mAnimations.end();it++)
    {
      if ((*it).promise)
      {
	  (*it).promise.send("reject",nullValue);
      }
    }

    mReady.send("reject",nullValue);

    mAnimations.clear();
    mEmit->clearListeners();
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      (*it)->mParent = NULL;  // setParent mutates the mChildren collection
      (*it)->dispose(false);
    }
    mChildren.clear();
    clearSnapshot(mSnapshotRef);
    clearSnapshot(mClipSnapshotRef);
    clearSnapshot(mDrawableSnapshotForMask);
    clearSnapshot(mMaskSnapshot);
    mSnapshotRef = NULL;
    mClipSnapshotRef = NULL;
    mDrawableSnapshotForMask = NULL;
    mMaskSnapshot = NULL;
    if (mScene)
    {
      mScene->innerpxObjectDisposed(this);
    }
#ifdef ENABLE_RT_NODE
    if (pumpJavascript)
    {
      script.pump();
    }
#else
    (void)pumpJavascript;
#endif
 }
}

/** since this is a boolean, we have to handle if someone sets it to
 * false - for now, it will mean "set focus to my parent scene" */
rtError pxObject::setFocus(bool v)
{
  rtLogDebug("pxObject::setFocus v=%d\n",v);
  if(v) {
    return mScene->setFocus(this);
  }
  else {
    return mScene->setFocus(NULL);
  }

}

rtError pxObject::Set(uint32_t i, const rtValue* value)
{
  (void)i;
  (void)value;
  rtLogError("pxObject::Set(uint32_t, const rtValue*) - not implemented");
  return RT_ERROR_NOT_IMPLEMENTED;
}

rtError pxObject::Set(const char* name, const rtValue* value)
{
  #ifdef PX_DIRTY_RECTANGLES
  mIsDirty = true;
  //mScreenCoordinates = getBoundingRectInScreenCoordinates();

  #endif //PX_DIRTY_RECTANGLES
  if (strcmp(name, "x") != 0 && strcmp(name, "y") != 0 &&  strcmp(name, "a") != 0)
  {
    repaint();
  }
  repaintParents();
  mScene->mDirty = true;
  return rtObject::Set(name, value);
}

// TODO Cleanup animateTo methods... animateTo animateToP2 etc...
rtError pxObject::animateToP2(rtObjectRef props, double duration,
                              uint32_t interp, uint32_t options,
                              int32_t count, rtObjectRef& promise)
{
  if (mIsDisposed)
  {
    rtLogWarn("animation is performed on disposed object !!!!");
    promise = new rtPromise();
    rtValue nullValue;
    promise.send("reject",nullValue);
    return RT_OK;
  }

  if (!props) return RT_FAIL;

  // TODO JR... not sure that we should do an early out here... thinking
  // we should still return a resolved promise given time...
  // just going to get exceptions if you try to do a .then on the return result
  //if (!props) return RT_OK;
  // Default to Linear, Loop and count==1
  if (!interp)  { interp = pxConstantsAnimation::TWEEN_LINEAR;}
  if (!options) {options = pxConstantsAnimation::OPTION_LOOP;}
  if (!count)   {  count = 1;}

  promise = new rtPromise();

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateTo(key, props.get<float>(key), duration, interp, options, count,(i==0)?promise:rtObjectRef());
    }
  }

  return RT_OK;
}

rtError pxObject::animateToObj(rtObjectRef props, double duration,
                              uint32_t interp, uint32_t options,
                              int32_t count, rtObjectRef& animateObj)
{

  if (!props) return RT_FAIL;
  // TODO JR... not sure that we should do an early out here... thinking
  // we should still return a resolved promise given time...
  // just going to get exceptions if you try to do a .then on the return result
  //if (!props) return RT_OK;
  // Default to Linear, Loop and count==1

  if (!interp)  {  interp = pxConstantsAnimation::TWEEN_LINEAR;}
  if (!options) { options = pxConstantsAnimation::OPTION_LOOP; }
  if (!count)   {   count = 1;}

  rtObjectRef promise = new rtPromise();
  animateObj = new pxAnimate(props, interp, (pxConstantsAnimation::animationOptions)options, duration, count, promise, this);
  if (mIsDisposed)
  {
    rtLogWarn("animation is performed on disposed object !!!!");
    rtValue nullValue;
    promise.send("reject",nullValue);
    return RT_OK;
  }

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateToInternal(key, props.get<float>(key), duration, ((pxConstantsAnimation*)CONSTANTS.animationConstants.getPtr())->getInterpFunc(interp), (pxConstantsAnimation::animationOptions)options, count,(i==0)?promise:rtObjectRef(),animateObj);
    }
  }
  if (NULL != animateObj.getPtr())
    ((pxAnimate*)animateObj.getPtr())->setStatus(pxConstantsAnimation::STATUS_INPROGRESS);
  return RT_OK;
}

void pxObject::setParent(rtRef<pxObject>& parent)
{
  if (mParent != parent)
  {
    remove();
    mParent = parent;
    if (parent)
      parent->mChildren.push_back(this);
#ifdef PX_DIRTY_RECTANGLES
    mIsDirty = true;
    //mScreenCoordinates = getBoundingRectInScreenCoordinates();
#endif //PX_DIRTY_RECTANGLES
  }
}

rtError pxObject::children(rtObjectRef& v) const
{
  v = new pxObjectChildren(const_cast<pxObject*>(this));
  return RT_OK;
}

rtError pxObject::remove()
{
  if (mParent)
  {
    for(vector<rtRef<pxObject> >::iterator it = mParent->mChildren.begin();
        it != mParent->mChildren.end(); ++it)
    {
      if ((it)->getPtr() == this)
      {
        pxObject* parent = mParent;
        mParent->mChildren.erase(it);
        mParent = NULL;
        parent->repaint();
        parent->repaintParents();
        mScene->mDirty = true;
        return RT_OK;
      }
    }
  }
  return RT_OK;
}

rtError pxObject::removeAll()
{
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->mParent = NULL;
  }
  mChildren.clear();
  repaint();
  repaintParents();
  mScene->mDirty = true;
  return RT_OK;
}

rtError pxObject::moveToFront()
{
  pxObject* parent = this->parent();

  if(!parent) return RT_OK;

  // If this pxObject is already at the front (last child),
  // make this a no-op
  uint32_t size = parent->mChildren.size();
  rtRef<pxObject> lastChild = parent->mChildren[size-1];
  if( lastChild.getPtr() == this) {
    return RT_OK;
  }

  remove();
  setParent(parent);

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

rtError pxObject::moveToBack()
{
  pxObject* parent = this->parent();

  if(!parent) return RT_OK;

  // If this pxObject is already at the back (first child),
  // make this a no-op
  rtRef<pxObject> firstChild = parent->mChildren[0];
  if( firstChild.getPtr() == this) {
    return RT_OK;
  }

  remove();
  mParent = parent;
  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin();
  parent->mChildren.insert(it, this);

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

/**
 * moveForward: Move this child in front of its next closest sibling in z-order, which means
 *              moving it toward end of array because last item is at top of z-order
 **/
rtError pxObject::moveForward()
{
  pxObject* parent = this->parent();

  if(!parent)
      return RT_OK;

  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin(), it_prev;
  while( it != parent->mChildren.end() )
  {
      if( it->getPtr() == this )
      {
        it_prev = it++;
        break;
      }
      it++;
  }

  if( it == parent->mChildren.end() )
      return RT_OK;

  std::iter_swap(it_prev, it);

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

/**
 * moveBackward: Move this child behind its next closest sibling in z-order, which means
 *               moving it toward beginning of array because first item is at bottom of z-order
 **/
rtError pxObject::moveBackward()
{
  pxObject* parent = this->parent();

  if(!parent)
      return RT_OK;

  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin(), it_prev;
  while( it != parent->mChildren.end() )
  {
      if( it->getPtr() == this )
      {
          break;
      }
      it_prev = it++;
  }
  if( it == parent->mChildren.begin() )
      return RT_OK;

  std::iter_swap(it_prev, it);

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

rtError pxObject::animateTo(const char* prop, double to, double duration,
                             uint32_t interp, uint32_t options,
                            int32_t count, rtObjectRef promise)
{
  if (mIsDisposed)
  {
    return RT_OK;
  }
  animateToInternal(prop, to, duration, ((pxConstantsAnimation*)CONSTANTS.animationConstants.getPtr())->getInterpFunc(interp),
            (pxConstantsAnimation::animationOptions)options, count, promise, rtObjectRef());
  return RT_OK;
}

// Dont fastforward when calling from set* methods since that will
// recurse indefinitely and crash and we're going to change the value in
// the set* method anyway.
void pxObject::cancelAnimation(const char* prop, bool fastforward, bool rewind)
{
  if (!mCancelInSet)
    return;
  bool f = mCancelInSet;
  // Do not reenter
  mCancelInSet = false;

  // If an animation for this property is in progress we cancel it here
  vector<animation>::iterator it = mAnimations.begin();
  while (it != mAnimations.end())
  {
    animation& a = (*it);
    if (!a.cancelled && a.prop == prop)
    {
      pxAnimate* pAnimateObj = (pxAnimate*) a.animateObj.getPtr();

      // Fastforward or rewind, if specified
      if( fastforward)
        set(prop, a.to);
      else if( rewind)
        set(prop, a.from);

      // If animation was never-ending, promise was already resolved.
      // If not, send it now.
      if( a.count != pxConstantsAnimation::COUNT_FOREVER)
      {
        if (a.ended)
          a.ended.send(this);
        if (a.promise && a.promise.getPtr() != NULL)
        {
          a.promise.send("resolve", this);

          if (NULL != pAnimateObj)
          {
            pAnimateObj->setStatus(pxConstantsAnimation::STATUS_CANCELLED);
          }
        }
      }
#if 0
      else
      {
        // TODO experiment if we cancel non ending animations set back
        // to beginning
        if (fastforward)
          set(prop, a.to);
      }
#endif
      a.cancelled = true;

      if (NULL != pAnimateObj)
      {
        pAnimateObj->update(prop, &a, pxConstantsAnimation::STATUS_CANCELLED);
      }
    }
    ++it;
  }
  mCancelInSet = f;
}

void pxObject::animateToInternal(const char* prop, double to, double duration,
                         pxInterp interp, pxConstantsAnimation::animationOptions options,
                         int32_t count, rtObjectRef promise, rtObjectRef animateObj)
{
  cancelAnimation(prop,(options & pxConstantsAnimation::OPTION_FASTFORWARD),
                       (options & pxConstantsAnimation::OPTION_REWIND));

  // schedule animation
  animation a;

  a.cancelled = false;
  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = static_cast<float>(to);
  a.start    = -1;
  a.duration = duration;
  a.interpFunc  = interp ? interp : pxInterpLinear;
  a.options     = options;
  a.count    = count;
  a.actualCount = 0;
  a.reversing = false;
//  a.ended = onEnd;
  a.promise = promise;
  a.animateObj = animateObj;

  mAnimations.push_back(a);

  pxAnimate *animObj = (pxAnimate *)a.animateObj.getPtr();

  if (NULL != animObj)
  {
    animObj->update(prop, &a, pxConstantsAnimation::STATUS_INPROGRESS);
  }

  // resolve promise immediately if this is COUNT_FOREVER
  if( count == pxConstantsAnimation::COUNT_FOREVER)
  {
    if (a.ended)
      a.ended.send(this);
    if (a.promise)
      a.promise.send("resolve",this);
  }
}

void pxObject::update(double t)
{
#ifdef DEBUG_SKIP_UPDATE
#warning " 'DEBUG_SKIP_UPDATE' is Enabled"
  return;
#endif

  // Update animations
  vector<animation>::iterator it = mAnimations.begin();

  while (it != mAnimations.end())
  {
    animation& a = (*it);

    pxAnimate *animObj = (pxAnimate *)a.animateObj.getPtr();

    if (a.start < 0) a.start = t;
    double end = a.start + a.duration;

    // if duration has elapsed, increment the count for this animation
    if( t >=end && a.count != pxConstantsAnimation::COUNT_FOREVER
        && !(a.options & pxConstantsAnimation::OPTION_OSCILLATE))
    {
        a.actualCount++;
        a.start  = -1;
    }
    // if duration has elapsed and count is met, end the animation
    if (t >= end && a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count)
    {
      // TODO this sort of blows since this triggers another
      // animation traversal to cancel animations
#if 0
      cancelAnimation(a.prop, true, false);
#else
      assert(mCancelInSet);
      mCancelInSet = false;
      set(a.prop, a.to);
      mCancelInSet = true;

      if (a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count )
      {
        if (a.ended)
          a.ended.send(this);
        if (a.promise)
        {
          a.promise.send("resolve",this);
          if (NULL != animObj)
          {
            animObj->setStatus(pxConstantsAnimation::STATUS_ENDED);
          }
        }
        // Erase making sure to push the iterator forward before
        a.cancelled = true;
        if (NULL != animObj)
        {
          animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_ENDED);
        }
        it = mAnimations.erase(it);
        continue;
      }
#endif

    }

    if (a.cancelled)
    {
      if (NULL != animObj)
      {
        animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_CANCELLED);
      }

      it = mAnimations.erase(it);  // returns next element
      continue;
    }

    double t1 = (t-a.start)/a.duration; // Some of this could be pushed into the end handling
    double t2 = floor(t1);
    t1 = t1-t2; // 0-1

    double d = a.interpFunc(t1);
    float from = a.from;
    float   to = a.to;

    if (a.options & pxConstantsAnimation::OPTION_OSCILLATE)
    {
      bool justReverseChange = false;
      double toVal = a.to;
      if( (fmod(t2,2) != 0))  // TODO perf chk ?
      {
        if(!a.reversing)
        {
          a.reversing = true;
          justReverseChange = true;
          a.actualCount++;
        }
        from = a.to;
        to   = a.from;
      }
      else if( a.reversing && (fmod(t2,2) == 0))
      {
        toVal = a.from;
        justReverseChange = true;
        a.reversing = false;
        a.actualCount++;
        a.start = -1;
      }
      // Prevent one more loop through oscillate
      if(a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count )
      {
          // if(a.actualCount == a.count)
          // {
          //   justReverseChange = false;
          // }

          if (true == justReverseChange)
          {
            mCancelInSet = false;
            set(a.prop, toVal);
            mCancelInSet = true;
          }

        if (NULL != animObj)
        {
          animObj->setStatus(pxConstantsAnimation::STATUS_ENDED);
        }
        cancelAnimation(a.prop, false, false);

        if (NULL != animObj)
        {
          animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_ENDED);
        }

        it = mAnimations.erase(it);
        continue;
      }

    }

    float v = static_cast<float> (from + (to - from) * d);
    assert(mCancelInSet);
    mCancelInSet = false;
    set(a.prop, v);
    mCancelInSet = true;
    if (NULL != animObj)
    {
      animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_INPROGRESS);
    }
    ++it;
  }

#ifdef PX_DIRTY_RECTANGLES
    pxMatrix4f m;
    applyMatrix(m);
    context.setMatrix(m);
    mRenderMatrix = m;
    if (mIsDirty)
    {
        mScene->invalidateRect(&mScreenCoordinates);
        
        pxRect dirtyRect = getBoundingRectInScreenCoordinates();
        if (!dirtyRect.isEqual(mScreenCoordinates))
        {
            mScene->invalidateRect(&dirtyRect);
            dirtyRect.unionRect(mScreenCoordinates);
            setDirtyRect(&dirtyRect);
        }
        else
            setDirtyRect(&mScreenCoordinates);
        
        mIsDirty = false;
    }
#endif

  // Recursively update children
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
#ifdef PX_DIRTY_RECTANGLES
    context.pushState();
#endif //PX_DIRTY_RECTANGLES
// JR TODO  this lock looks suspicious... why do we need it?
ENTERSCENELOCK()
    (*it)->update(t);
EXITSCENELOCK()
#ifdef PX_DIRTY_RECTANGLES
    context.popState();
#endif //PX_DIRTY_RECTANGLES
  }
    
#ifdef PX_DIRTY_RECTANGLES
    context.setMatrix(m);
    mRenderMatrix = m;
#endif
    
  // Send promise
  sendPromise();
}

void pxObject::releaseData(bool sceneSuspended)
{
  clearSnapshot(mClipSnapshotRef);
  clearSnapshot(mDrawableSnapshotForMask);
  clearSnapshot(mMaskSnapshot);
  mSceneSuspended = sceneSuspended;
  // Recursively suspend the children
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->releaseData(sceneSuspended);
  }
}

void pxObject::reloadData(bool sceneSuspended)
{
  mSceneSuspended = sceneSuspended;
  mRepaint = true;
  // Recursively resume the children
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->reloadData(sceneSuspended);
  }
}

uint64_t pxObject::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (mClipSnapshotRef.getPtr() != NULL)
  {
    textureMemory += (mClipSnapshotRef->width() * mClipSnapshotRef->height() * 4);
  }
  if (mDrawableSnapshotForMask.getPtr() != NULL)
  {
    textureMemory += (mDrawableSnapshotForMask->width() * mDrawableSnapshotForMask->height() * 4);
  }
  if (mSnapshotRef.getPtr() != NULL)
  {
    textureMemory += (mSnapshotRef->width() * mSnapshotRef->height() * 4);
  }
  if (mMaskSnapshot.getPtr() != NULL)
  {
    textureMemory += (mMaskSnapshot->width() * mMaskSnapshot->height() * 4);
  }

  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    textureMemory += (*it)->textureMemoryUsage();
  }
  return textureMemory;
}

#ifdef PX_DIRTY_RECTANGLES
void pxObject::setDirtyRect(pxRect *r)
{
  if (r != NULL)
  {
    mDirtyRect.unionRect(*r);
    if (mParent != NULL)
    {
      mParent->setDirtyRect(&mDirtyRect);
    }
  }
}

pxRect pxObject::getBoundingRectInScreenCoordinates()
{
  int w = getOnscreenWidth();
  int h = getOnscreenHeight();
  int x[4], y[4];
  mRenderMatrix = context.getMatrix();
  context.mapToScreenCoordinates(mRenderMatrix, 0,0,x[0],y[0]);
  context.mapToScreenCoordinates(mRenderMatrix, w, h, x[1], y[1]);
  context.mapToScreenCoordinates(mRenderMatrix, 0, h, x[2], y[2]);
  context.mapToScreenCoordinates(mRenderMatrix, w, 0, x[3], y[3]);
  int left, right, top, bottom;

  left = x[0];
  right = x[0];
  top = y[0];
  bottom = y[0];
  for (int i = 0; i < 4; i ++)
  {
    if (x[i] < left)
    {
      left = x[i];
    }
    else if (x[i] > right)
    {
      right = x[i];
    }

    if (y[i] < top)
    {
      top = y[i];
    }
    else if (y[i] > bottom)
    {
      bottom = y[i];
    }
  }
  return pxRect(left, top, right, bottom);
}

pxRect pxObject::convertToScreenCoordinates(pxRect* r)
{
  if (r == NULL)
  {
     return pxRect();
  }
  int rectLeft = r->left();
  int rectRight = r->right();
  int rectTop = r->top();
  int rectBottom = r->bottom();
  int x[4], y[4];
  context.mapToScreenCoordinates(mRenderMatrix, rectLeft,rectTop,x[0],y[0]);
  context.mapToScreenCoordinates(mRenderMatrix, rectRight, rectBottom, x[1], y[1]);
  context.mapToScreenCoordinates(mRenderMatrix, rectLeft, rectBottom, x[2], y[2]);
  context.mapToScreenCoordinates(mRenderMatrix, rectRight, rectTop, x[3], y[3]);
  int left, right, top, bottom;

  left = x[0];
  right = x[0];
  top = y[0];
  bottom = y[0];
  for (int i = 0; i < 4; i ++)
  {
    if (x[i] < left)
    {
      left = x[i];
    }
    else if (x[i] > right)
    {
      right = x[i];
    }

    if (y[i] < top)
    {
      top = y[i];
    }
    else if (y[i] > bottom)
    {
      bottom = y[i];
    }
  }
  return pxRect(left, top, right, bottom);
}
#endif //PX_DIRTY_RECTANGLES

const float alphaEpsilon = (1.0f/255.0f);

void pxObject::drawInternal(bool maskPass)
{
  //rtLogInfo("pxObject::drawInternal mw=%f mh=%f\n", mw, mh);

  if (!drawEnabled() && !maskPass)
  {
    return;
  }
  // TODO what to do about multiple vanishing points in a given scene
  // TODO consistent behavior between clipping and no clipping when z is in use

  if (context.getAlpha() < alphaEpsilon)
  {
    return;  // trivial reject for objects that are transparent
  }

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

  pxMatrix4f m;

#if 1
#if 1
#if 0
  // translate based on xy rotate/scale based on cx, cy
  m.translate(mx+mcx, my+mcy);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  if (mr) {
    m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
    , mrx, mry, mrz
#endif //ANIMATION_ROTATE_XYZ
    );
  }
  //if (mr) m.rotateInDegrees(mr, 0, 0, 1);
  if (msx != 1.0f || msy != 1.0f) m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#else

#ifdef PX_DIRTY_RECTANGLES
    m = mRenderMatrix;
#else
    applyMatrix(m); // ANIMATE !!!
#endif
#endif
#else
  // translate/rotate/scale based on cx, cy
  m.translate(mx, my);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  //  m.rotateInDegrees(mr, mrx, mry, mrz);
  m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
  , 0, 0, 1
#endif // ANIMATION_ROTATE_XYZ
  );
  m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#endif
#endif

#if 0

  rtLogDebug("drawInternal: %s\n", mId.cString());
  m.dump();

  pxVector4f v1(mx+w, my, 0, 1);
  rtLogDebug("Print vector top\n");
  v1.dump();

  pxVector4f result1 = m.multiply(v1);
  rtLogDebug("Print vector top after\n");
  result1.dump();

  pxVector4f v2(mx+w, my+mh, 0, 1);
  rtLogDebug("Print vector bottom\n");
  v2.dump();

  pxVector4f result2 = m.multiply(v2);
  rtLogDebug("Print vector bottom after\n");
  result2.dump();

#endif

  context.setMatrix(m);
  context.setAlpha(ma);

  if ((mClip && !context.isObjectOnScreen(0,0,w,h)) || mSceneSuspended)
  {
    //rtLogInfo("pxObject::drawInternal returning because object is not on screen mw=%f mh=%f\n", mw, mh);
    return;
  }

  #ifdef PX_DIRTY_RECTANGLES
  //mLastRenderMatrix = context.getMatrix();
  mScreenCoordinates = getBoundingRectInScreenCoordinates();
  #endif //PX_DIRTY_RECTANGLES

  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, w, h, c);

  //rtLogInfo("pxObject::drawInternal mPainting=%d mw=%f mh=%f\n", mPainting, mw, mh);
  if (mPainting)
  {
    pxConstantsMaskOperation::constants maskOp = pxConstantsMaskOperation::NORMAL; // default
    
    // MASKING ? ---------------------------------------------------------------------------------------------------
    bool maskFound = false;
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->mask())
      {
        //rtLogInfo("pxObject::drawInternal mask is true mw=%f mh=%f\n", mw, mh);
        maskFound = true;
        
        pxImage *img = dynamic_cast<pxImage *>( &*it->getPtr() ) ;
        if(img)
        {
          int32_t val;
          img->maskOp(val); // get mask operation
          
          maskOp = (pxConstantsMaskOperation::constants) val;
        }
        
        break;
      }
    }

    // MASKING ? ---------------------------------------------------------------------------------------------------
    if (maskFound)
    {
      if (w>alphaEpsilon && h>alphaEpsilon)
      {
        draw();
      }
      createSnapshotOfChildren();
      context.setMatrix(m);
      //rtLogInfo("context.drawImage\n");
      
      context.drawImageMasked(0, 0, w, h, maskOp, mDrawableSnapshotForMask->getTexture(), mMaskSnapshot->getTexture());
    }
    // CLIPPING ? ---------------------------------------------------------------------------------------------------
    else if (mClip)
    {
      //rtLogInfo("calling createSnapshot for mw=%f mh=%f\n", mw, mh);
      if (mRepaint)
      {
        createSnapshot(mClipSnapshotRef);
        context.setMatrix(m);
        context.setAlpha(ma);
      }

      if (mClipSnapshotRef.getPtr() != NULL)
      {
        //rtLogInfo("context.drawImage\n");
        static pxTextureRef nullMaskRef;
        context.drawImage(0, 0, w, h, mClipSnapshotRef->getTexture(), nullMaskRef);
      }
    }
    // DRAWING ---------------------------------------------------------------------------------------------------
    else
    {
      // trivially reject things too small to be seen
      if ( !mClip || (w>alphaEpsilon && h>alphaEpsilon && context.isObjectOnScreen(0, 0, w, h)))
      {
        //rtLogInfo("calling draw() mw=%f mh=%f\n", mw, mh);
        draw();
      }

      // CHILDREN -------------------------------------------------------------------------------------
      for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        if((*it)->drawEnabled() == false)
        {
          continue;
        }
        context.pushState();
        //rtLogInfo("calling drawInternal() mw=%f mh=%f\n", (*it)->mw, (*it)->mh);
        (*it)->drawInternal();
#ifdef PX_DIRTY_RECTANGLES
        int left = (*it)->mScreenCoordinates.left();
        int right = (*it)->mScreenCoordinates.right();
        int top = (*it)->mScreenCoordinates.top();
        int bottom = (*it)->mScreenCoordinates.bottom();
        if (right > mScreenCoordinates.right())
        {
          mScreenCoordinates.setRight(right);
        }
        if (left < mScreenCoordinates.left())
        {
          mScreenCoordinates.setLeft(left);
        }
        if (top < mScreenCoordinates.top())
        {
          mScreenCoordinates.setTop(top);
        }
        if (bottom > mScreenCoordinates.bottom())
        {
          mScreenCoordinates.setBottom(bottom);
        }
#endif //PX_DIRTY_RECTANGLES
        context.popState();
      }
      // ---------------------------------------------------------------------------------------------------
    }
  }
  else
  {
    //rtLogInfo("context.drawImage mw=%f mh=%f\n", mw, mh);
    static pxTextureRef nullMaskRef;
    context.drawImage(0,0,w,h, mSnapshotRef->getTexture(), nullMaskRef);
  }

  // ---------------------------------------------------------------------------------------------------
  if (!maskPass)
  {
    mRepaint = false;
  }
  // ---------------------------------------------------------------------------------------------------
#ifdef PX_DIRTY_RECTANGLES
  mDirtyRect.setEmpty();
#endif //PX_DIRTY_RECTANGLES
}


bool pxObject::hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRef<pxObject>& hit,
                   pxPoint2f& hitPt)
{

  // setup matrix
  pxMatrix4f m2;
#if 0
  m2.translate(mx+mcx, my+mcy);
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m2.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
  , 0, 0, 1
#endif // ANIMATION_ROTATE_XYZ
  );
  m2.scale(msx, msy);
  m2.translate(-mcx, -mcy);
#else
  applyMatrix(m2);
#endif
  m2.invert();
  m2.multiply(m);

  {
    for(vector<rtRef<pxObject> >::reverse_iterator it = mChildren.rbegin(); it != mChildren.rend(); ++it)
    {
      if ((*it)->hitTestInternal(m2, pt, hit, hitPt))
        return true;
    }
  }

  {
    // map pt to object coordinate space
    pxVector4f v(pt.x, pt.y, 0, 1);
    v = m2.multiply(v);
    pxPoint2f newPt;
    newPt.x = v.x();
    newPt.y = v.y();
    if (mInteractive && hitTest(newPt))
    {
      hit = this;
      hitPt = newPt;
      return true;
    }
    else
      return false;
  }
}

// TODO should we bother with pxPoint2f or just use pxVector4f
// pt is in object coordinates
bool pxObject::hitTest(pxPoint2f& pt)
{
  // default hitTest checks against object bounds (0, 0, w, h)
  // Can override for more interesting hit tests like alpha
  return (pt.x >= 0 && pt.y >= 0 && pt.x <= mw && pt.y <= mh);
}

rtError pxObject::setPainting(bool v)
{
  mPainting = v;
  if (!mPainting)
  {
    //rtLogInfo("in setPainting and calling createSnapshot mw=%f mh=%f\n", mw, mh);
#ifdef RUNINMAIN
    createSnapshot(mSnapshotRef, false, true);
#else
    createSnapshot(mSnapshotRef, true, true);
#endif //RUNINMAIN
  }
  else
  {
    clearSnapshot(mSnapshotRef);
  }
  return RT_OK;
}


void pxObject::createSnapshot(pxContextFramebufferRef& fbo, bool separateContext,
                              bool antiAliasing)
{
  pxMatrix4f m;

//  float parentAlpha = ma;

  float parentAlpha = 1.0;
  if (separateContext)
  {
    context.enableInternalContext(true);
  }

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

#ifdef PX_DIRTY_RECTANGLES
  bool fullFboRepaint = false;
#endif //PX_DIRTY_RECTANGLES

  //rtLogInfo("createSnapshot  w=%f h=%f\n", w, h);
  if (fbo.getPtr() == NULL || fbo->width() != floor(w) || fbo->height() != floor(h))
  {
    clearSnapshot(fbo);
    //rtLogInfo("createFramebuffer  mw=%f mh=%f\n", w, h);
    fbo = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)), antiAliasing);
#ifdef PX_DIRTY_RECTANGLES
    fullFboRepaint = true;
#endif //PX_DIRTY_RECTANGLES
  }
  else
  {
    //rtLogInfo("updateFramebuffer  mw=%f mh=%f\n", w, h);
    context.updateFramebuffer(fbo, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }
  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (mRepaint && context.setFramebuffer(fbo) == PX_OK)
  {
    //context.clear(static_cast<int>(w), static_cast<int>(h));
#ifdef PX_DIRTY_RECTANGLES
    int clearX = mDirtyRect.left();
    int clearY = mDirtyRect.top();
    int clearWidth = mDirtyRect.right() - clearX+1;
    int clearHeight = mDirtyRect.bottom() - clearY+1;

    if (!mIsDirty)
        context.clear(static_cast<int>(w), static_cast<int>(h));
      
    if (fullFboRepaint)
    {
        clearX = 0;
        clearY = 0;
        clearWidth = w;
        clearHeight = h;
        context.clear(clearX, clearY, clearWidth, clearHeight);
    }
#else
    context.clear(static_cast<int>(w), static_cast<int>(h));
#endif //PX_DIRTY_RECTANGLES
    draw();

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      context.pushState();
      (*it)->drawInternal();
      context.popState();
    }
  }
  context.setFramebuffer(previousRenderSurface);
  if (separateContext)
  {
    context.enableInternalContext(false);
  }
}

void pxObject::createSnapshotOfChildren()
{
  //rtLogInfo("pxObject::createSnapshotOfChildren\n");
  pxMatrix4f m;
  float parentAlpha = ma;

  context.setMatrix(m);

  context.setAlpha(parentAlpha);

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

  //rtLogInfo("createSnapshotOfChildren  w=%f h=%f\n", w, h);

  if (mDrawableSnapshotForMask.getPtr() == NULL || mDrawableSnapshotForMask->width() != floor(w) || mDrawableSnapshotForMask->height() != floor(h))
  {
    mDrawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }
  else
  {
    context.updateFramebuffer(mDrawableSnapshotForMask, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }

  if (mMaskSnapshot.getPtr() == NULL || mMaskSnapshot->width() != floor(w) || mMaskSnapshot->height() != floor(h))
  {
    mMaskSnapshot = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)), false, true);
  }
  else
  {
    context.updateFramebuffer(mMaskSnapshot, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }

  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (context.setFramebuffer(mMaskSnapshot) == PX_OK)
  {
    context.clear(static_cast<int>(w), static_cast<int>(h));

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->mask())
      {
        context.pushState();
        (*it)->drawInternal(true);
        context.popState();
      }
    }
  }

  if (context.setFramebuffer(mDrawableSnapshotForMask) == PX_OK)
  {
    context.clear(static_cast<int>(w), static_cast<int>(h));

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->drawEnabled())
      {
        context.pushState();
        (*it)->drawInternal();
        context.popState();
      }
    }
  }

  context.setFramebuffer(previousRenderSurface);
}

void pxObject::clearSnapshot(pxContextFramebufferRef fbo)
{
  if (fbo.getPtr() != NULL)
  {
    fbo->resetFbo();
  }
}



bool pxObject::onTextureReady()
{
  repaint();
  repaintParents();
  if (mScene != NULL)
  {
    mScene->invalidateRect(NULL);
  }
  #ifdef PX_DIRTY_RECTANGLES
  mIsDirty = true;
  #endif //PX_DIRTY_RECTANGLES
  return false;
}

void pxObject::repaintParents()
{
  pxObject* parent = mParent;
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
}

#if 0
rtDefineObject(rtPromise, rtObject);
rtDefineMethod(rtPromise, then);
rtDefineMethod(rtPromise, resolve);
rtDefineMethod(rtPromise, reject);
#endif

rtDefineObject(pxObject, rtObject);
rtDefineProperty(pxObject, _pxObject);
rtDefineProperty(pxObject, parent);
rtDefineProperty(pxObject, children);
rtDefineProperty(pxObject, x);
rtDefineProperty(pxObject, y);
rtDefineProperty(pxObject, w);
rtDefineProperty(pxObject, h);
rtDefineProperty(pxObject, px);
rtDefineProperty(pxObject, py);
rtDefineProperty(pxObject, cx);
rtDefineProperty(pxObject, cy);
rtDefineProperty(pxObject, sx);
rtDefineProperty(pxObject, sy);
rtDefineProperty(pxObject, a);
rtDefineProperty(pxObject, r);
#ifdef ANIMATION_ROTATE_XYZ
rtDefineProperty(pxObject, rx);
rtDefineProperty(pxObject, ry);
rtDefineProperty(pxObject, rz);
#endif //ANIMATION_ROTATE_XYZ
rtDefineProperty(pxObject, id);
rtDefineProperty(pxObject, interactive);
rtDefineProperty(pxObject, painting);
rtDefineProperty(pxObject, clip);
rtDefineProperty(pxObject, mask);
rtDefineProperty(pxObject, draw);
rtDefineProperty(pxObject, hitTest);
rtDefineProperty(pxObject,focus);
rtDefineProperty(pxObject,ready);
rtDefineProperty(pxObject, numChildren);
rtDefineMethod(pxObject, getChild);
rtDefineMethod(pxObject, remove);
rtDefineMethod(pxObject, removeAll);
rtDefineMethod(pxObject, moveToFront);
rtDefineMethod(pxObject, moveToBack);
rtDefineMethod(pxObject, moveForward);
rtDefineMethod(pxObject, moveBackward);
rtDefineMethod(pxObject, releaseResources);
//rtDefineMethod(pxObject, animateTo);
#if 0
//TODO - remove
rtDefineMethod(pxObject, animateToF2);
#endif
rtDefineMethod(pxObject, animateToP2);
rtDefineMethod(pxObject, animateToObj);
rtDefineMethod(pxObject, addListener);
rtDefineMethod(pxObject, delListener);
//rtDefineProperty(pxObject, emit);
//rtDefineProperty(pxObject, onReady);
rtDefineMethod(pxObject, getObjectById);
rtDefineProperty(pxObject,m11);
rtDefineProperty(pxObject,m12);
rtDefineProperty(pxObject,m13);
rtDefineProperty(pxObject,m14);
rtDefineProperty(pxObject,m21);
rtDefineProperty(pxObject,m22);
rtDefineProperty(pxObject,m23);
rtDefineProperty(pxObject,m24);
rtDefineProperty(pxObject,m31);
rtDefineProperty(pxObject,m32);
rtDefineProperty(pxObject,m33);
rtDefineProperty(pxObject,m34);
rtDefineProperty(pxObject,m41);
rtDefineProperty(pxObject,m42);
rtDefineProperty(pxObject,m43);
rtDefineProperty(pxObject,m44);
rtDefineProperty(pxObject,useMatrix);


rtDefineObject(pxRoot,pxObject);

int gTag = 0;

pxScene2d::pxScene2d(bool top, pxScriptView* scriptView)
  : mRoot(), mInfo(), mCapabilityVersions(), start(0), sigma_draw(0), sigma_update(0), end2(0), frameCount(0), mWidth(0), mHeight(0), mStopPropagation(false), mContainer(NULL), mShowDirtyRectangle(false),
    mInnerpxObjects(), mSuspended(false),
#ifdef PX_DIRTY_RECTANGLES
    mArchive(),mDirtyRect(), mLastFrameDirtyRect(),
#endif //PX_DIRTY_RECTANGLES
    mDirty(true), mTestView(NULL), mDisposed(false)
{
  mRoot = new pxRoot(this);
  mFocusObj = mRoot;
  mEmit = new rtEmit();
  mTop = top;
  mScriptView = scriptView;
  mTag = gTag++;

  if (scriptView != NULL)
  {
    mOrigin = rtUrlGetOrigin(scriptView->getUrl().cString());
  }

#ifdef ENABLE_PERMISSIONS_CHECK
  // rtPermissions accounts parent scene permissions too
  mPermissions = new rtPermissions(mOrigin.cString());
#endif
#ifdef ENABLE_ACCESS_CONTROL_CHECK
  mCORS = new rtCORS(mOrigin.cString());
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
  #ifdef USE_SCENE_POINTER
  mPointerX= 0;
  mPointerY= 0;
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

  mInfo.set("build", build);
  mInfo.set("gfxmemory", context.currentTextureMemoryUsageInBytes());


  //capability versions
  mCapabilityVersions = new rtMapObject;
  rtObjectRef graphicsCapabilities = new rtMapObject;
  graphicsCapabilities.set("svg", 1);
  mCapabilityVersions.set("graphics", graphicsCapabilities);

  rtObjectRef networkCapabilities = new rtMapObject;
#ifdef ENABLE_ACCESS_CONTROL_CHECK
  networkCapabilities.set("cors", 1);
#ifdef ENABLE_CORS_FOR_RESOURCES
  networkCapabilities.set("corsResources", 1);
#endif
#endif
  mCapabilityVersions.set("network", networkCapabilities);

  rtObjectRef metricsCapabilities = new rtMapObject;
  metricsCapabilities.set("textureMemory", 1);
  mCapabilityVersions.set("metrics", metricsCapabilities);
}

rtError pxScene2d::dispose()
{
    mDisposed = true;
    rtObjectRef e = new rtMapObject;
    // pass false to make onClose asynchronous
    mEmit.send("onClose", false, e);
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
    mEmit.send("onSceneTerminate", false, e);
    mEmit->clearListeners();

    mRoot     = NULL;
    mInfo     = NULL;
    mCapabilityVersions = NULL;
    mFocusObj = NULL;

    return RT_OK;
}

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
  else if (!strcmp("image",t.cString()))
    e = createImage(p,o);
#ifdef BUILD_WITH_PXPATH
  else if (!strcmp("path",t.cString()))
    e = createPath(p,o);
#endif // BUILD_WITH_PXPATH
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
  else if (!strcmp("scene",t.cString()))
    e = createScene(p,o);
  else if (!strcmp("external",t.cString()))
    e = createExternal(p,o);
  else if (!strcmp("wayland",t.cString()))
    e = createWayland(p,o);
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
    mInnerpxObjects.push_back((pxObject*)o.getPtr());
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

rtError pxScene2d::createImage(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}
#ifdef BUILD_WITH_PXPATH
rtError pxScene2d::createPath(rtObjectRef p, rtObjectRef& o)
{
  o = new pxPath(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}
#endif // BUILD_WITH_PXPATH

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

#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif
  
  o = pxFontManager::getFont(url, proxy, mCORS, mArchive);
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
  textureMemory += mRoot->textureMemoryUsage();
  v.setUInt64(textureMemory);
  return RT_OK;
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

void pxScene2d::draw()
{
#ifdef DEBUG_SKIP_DRAW
#warning " 'DEBUG_SKIP_DRAW' is Enabled"
  return;
#endif

  double __frameStart = pxMilliseconds();

  //rtLogInfo("pxScene2d::draw()\n");
  #ifdef PX_DIRTY_RECTANGLES
  pxRect dirtyRectangle = mDirtyRect;
  dirtyRectangle.unionRect(mLastFrameDirtyRect);
  int x = dirtyRectangle.left();
  int y = dirtyRectangle.top();
  int w = dirtyRectangle.right() - x+1;
  int h = dirtyRectangle.bottom() - y+1;

  static bool previousShowDirtyRect = false;

  if (mShowDirtyRectangle || previousShowDirtyRect)
  {
    context.enableDirtyRectangles(false);
  }

  if (mTop)
  {
    if (mShowDirtyRectangle)
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
    pxMatrix4f identity;
    identity.identity();
    pxMatrix4f currentMatrix = context.getMatrix();
    context.setMatrix(identity);
    float red[]= {1,0,0,1};
    bool showOutlines = context.showOutlines();
    context.setShowOutlines(true);
    context.drawDiagRect(x, y, w, h, red);
    context.setShowOutlines(showOutlines);
    context.setMatrix(currentMatrix);
    context.enableClipping(true);
  }
  previousShowDirtyRect = mShowDirtyRectangle;

#else // Not ... PX_DIRTY_RECTANGLES

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
  #endif //PX_DIRTY_RECTANGLES

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

void pxScene2d::onUpdate(double t)
{
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

  update(t);

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
#ifdef PX_DIRTY_RECTANGLES
      context.pushState();
#endif //PX_DIRTY_RECTANGLES

      if( mCustomAnimator != NULL ) {
          mCustomAnimator->Send( 0, NULL, NULL );
      }

#ifndef DEBUG_SKIP_UPDATE
      mRoot->update(t);
#else
      UNUSED_PARAM(t);
#endif

#ifdef PX_DIRTY_RECTANGLES
      context.popState();
#endif //PX_DIRTY_RECTANGLES
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

    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      mMouseDown = hit;
      // scene coordinates
      mMouseDownPt.x = static_cast<float>(x);
      mMouseDownPt.y = static_cast<float>(y);

      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseDown");
      e.set("target", (rtObject*)hit.getPtr());
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
      e.set("flags", flags);
      #if 0
      hit->mEmit.send("onMouseDown", e);
      #else
      bubbleEvent(e,hit,"onPreMouseDown","onMouseDown");
      #endif
    }
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
    e.set("flags", static_cast<uint32_t>(flags));
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
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {


      // Only send onMouseUp if this object got an onMouseDown
      if (tMouseDown == hit)
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

// TODO rtRef doesn't like non-const !=
void pxScene2d::setMouseEntered(rtRef<pxObject> o)//pxObject* o)
{
  if (mMouseEntered != o)
  {
    // Tell old object we've left
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseLeave");
      e.set("target", mMouseEntered.getPtr());
      #if 0
      mMouseEntered->mEmit.send("onMouseLeave", e);
      #else
      bubbleEvent(e,mMouseEntered,"onPreMouseLeave","onMouseLeave");
      #endif
    }
    mMouseEntered = o;

    // Tell new object we've entered
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseEnter");
      e.set("target", mMouseEntered.getPtr());
      #if 0
      mMouseEntered->mEmit.send("onMouseEnter", e);
      #else
      bubbleEvent(e,mMouseEntered,"onPreMouseEnter","onMouseEnter");
      #endif
    }
  }
}
/** This function is not exposed to javascript; it is called when
 * mFocus = true is set for a pxObject whose parent scene is this scene
 **/
rtError pxScene2d::setFocus(rtObjectRef o)
{
  rtLogInfo("pxScene2d::setFocus");
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
  rtObjectRef e = new rtMapObject;
  e.set("name", "onMouseLeave");
  mEmit.send("onMouseLeave", e);

  mMouseDown = NULL;
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
  #ifdef USE_SCENE_POINTER
  mPointerX= x;
  mPointerY= y;
  invalidateRect(NULL);
  mDirty= true;
  #endif
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("x", x);
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
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
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
      setMouseEntered(NULL);
  }
#endif
#if 0
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt;
  pt.x = x; pt.y = y;
  rtRef<pxObject> hit;

  if (mRoot->hitTestInternal(m, pt, hit))
  {
    rtString id = hit->get<rtString>("id");
    rtLogDebug("found object id: %s\n", id.isEmpty()?"none":id.cString());
  }
#endif
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

rtError pxScene2d::screenshot(rtString type, rtString& pngData)
{
#ifdef ENABLE_PERMISSIONS_CHECK
  if (RT_OK != mPermissions->allows("screenshot", rtPermissions::FEATURE))
    return RT_ERROR_NOT_ALLOWED;
#endif

  // Is this a type we support?
  if (type != "image/png;base64")
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

  rtData pngData2;
  if (pxStorePNGImage(o, pngData2) != RT_OK)
  {
    return RT_FAIL;
  }

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

  rtString base64coded;
  
  if( base64_encode(pngData2, base64coded) == RT_OK )
  {
    // We return a data Url string containing the image data
    pngData = "data:image/png;base64,";
    
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
    
        return RT_OK;
      
  }//ENDIF

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
      return RT_ERROR_NOT_ALLOWED;
    #endif //ENABLE_PERMISSIONS_CHECK
    rtObjectRef serviceManager;
    rtError result = pxServiceManager::findServiceManager(serviceManager);
    if (result != RT_OK)
    {
      rtLogWarn("service manager not found");
      return result;
    }
    result = serviceManager.sendReturns<rtObjectRef>("createService", mScriptView != NULL ? mScriptView->getUrl() : "", name, service);
    rtLogInfo("create %s service result: %d", name, result);
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

rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineProperty(pxScene2d, info);
rtDefineProperty(pxScene2d, capabilities);
rtDefineProperty(pxScene2d, w);
rtDefineProperty(pxScene2d, h);
rtDefineProperty(pxScene2d, showOutlines);
rtDefineProperty(pxScene2d, showDirtyRect);
rtDefineProperty(pxScene2d, customAnimator);
rtDefineMethod(pxScene2d, create);
rtDefineMethod(pxScene2d, clock);
rtDefineMethod(pxScene2d, logDebugMetrics);
rtDefineMethod(pxScene2d, collectGarbage);
rtDefineMethod(pxScene2d, suspend);
rtDefineMethod(pxScene2d, resume);
rtDefineMethod(pxScene2d, suspended);
rtDefineMethod(pxScene2d, textureMemoryUsage);
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
rtDefineProperty(pxScene2d,alignVertical);
rtDefineProperty(pxScene2d,alignHorizontal);
rtDefineProperty(pxScene2d,truncation);
rtDefineMethod(pxScene2d, dispose);

rtDefineProperty(pxScene2d, origin);
#ifdef ENABLE_PERMISSIONS_CHECK
rtDefineProperty(pxScene2d, permissions);
#endif
rtDefineProperty(pxScene2d, cors);
rtDefineMethod(pxScene2d, addServiceProvider);
rtDefineMethod(pxScene2d, removeServiceProvider);

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
#ifdef PX_DIRTY_RECTANGLES
    pxRect screenRect = convertToScreenCoordinates(r);
    mScene->invalidateRect(&screenRect);
    setDirtyRect(r);
#else
    mScene->invalidateRect(NULL);
    UNUSED_PARAM(r);
#endif //PX_DIRTY_RECTANGLES
  }
}

void pxScene2d::invalidateRect(pxRect* r)
{
#ifdef PX_DIRTY_RECTANGLES
  if (r != NULL)
  {
    mDirtyRect.unionRect(*r);
    mDirty = true;
  }
#else
  UNUSED_PARAM(r);
#endif //PX_DIRTY_RECTANGLES
  if (mContainer && !mTop)
  {
#ifdef PX_DIRTY_RECTANGLES
    mContainer->invalidateRect(mDirty ? &mDirtyRect : NULL);
#else
    mContainer->invalidateRect(NULL);
#endif //PX_DIRTY_RECTANGLES
  }
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
//rtDefineMethod(pxSceneContainer, makeReady);   // DEPRECATED ?


rtError pxSceneContainer::setUrl(rtString url)
{
  rtLogInfo("pxSceneContainer::setUrl(%s)",url.cString());

#ifdef ENABLE_PERMISSIONS_CHECK
  if (mScene != NULL && RT_OK != mScene->permissions()->allows(url, rtPermissions::DEFAULT))
    return RT_ERROR_NOT_ALLOWED;
#endif

  // If old promise is still unfulfilled resolve it
  // and create a new promise for the context of this Url
  mReady.send("resolve", this);
  mReady = new rtPromise();

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
  rtLogInfo("pxSceneContainer::ready\n");
  if (mScriptView) {
    rtLogInfo("mScriptView is set!\n");
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

uint64_t pxSceneContainer::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (mScriptView.getPtr())
  {
    rtValue v;
    mScriptView->textureMemoryUsage(v);
    textureMemory += v.toUInt64();
  }
  textureMemory += pxObject::textureMemoryUsage();
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

pxScriptView::pxScriptView(const char* url, const char* /*lang*/, pxIViewContainer* container)
     : mWidth(-1), mHeight(-1), mViewContainer(container), mRefCount(0)
{
  rtLogInfo(__FUNCTION__);
  rtLogDebug("pxScriptView::pxScriptView()entering\n");
  mUrl = url;
#ifndef RUNINMAIN // NOTE this ifndef ends after runScript decl, below
  mReady = new rtPromise();
 // mLang = lang;
  rtLogDebug("pxScriptView::pxScriptView() exiting\n");
}

void pxScriptView::runScript()
{
  rtLogInfo(__FUNCTION__);
#endif // ifndef RUNINMAIN

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

  #ifdef ENABLE_RT_NODE
  rtLogDebug("pxScriptView::pxScriptView is just now creating a context for mUrl=%s\n",mUrl.cString());
  //mCtx = script.createContext("javascript");
  script.createContext("javascript", mCtx);

  if (mCtx)
  {
    mPrintFunc = new rtFunctionCallback(printFunc, this);
    mGetScene = new rtFunctionCallback(getScene,  this);
    mMakeReady = new rtFunctionCallback(makeReady, this);
    mGetContextID = new rtFunctionCallback(getContextID, this);

    mCtx->add("print", mPrintFunc.getPtr());
    mCtx->add("getScene", mGetScene.getPtr());
    mCtx->add("makeReady", mMakeReady.getPtr());
    mCtx->add("getContextID", mGetContextID.getPtr());

#ifdef RUNINMAIN
    mReady = new rtPromise();
#endif

    mCtx->runFile("init.js");

    char buffer[MAX_URL_SIZE + 50];
    memset(buffer, 0, sizeof(buffer));
    snprintf(buffer, sizeof(buffer), "loadUrl(\"%s\");", mUrl.cString());
    rtLogDebug("pxScriptView::runScript calling runScript with %s\n",mUrl.cString());
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
    rtLogInfo("pxScriptView::runScript() ending\n");
//#endif
  }
  #endif //ENABLE_RT_NODE
}

rtError pxScriptView::printFunc(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  UNUSED_PARAM(result);
  //rtLogInfo(__FUNCTION__);

  if (ctx)
  {
    if (numArgs > 0 && !args[0].isEmpty())
    {
      rtString toPrint = args[0].toString();
      rtLogWarn("%s", toPrint.cString());
    }
  }
  return RT_OK;
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

rtError pxScriptView::getScene(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  rtLogInfo(__FUNCTION__);
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    if (numArgs == 1)
    {
      rtString sceneType = args[0].toString();
      // JR Todo can specify what scene version/type to create in args
      if (!v->mScene)
      {
        static bool top = true;
        pxScene2dRef scene = new pxScene2d(top, v);
        top = false;
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

rtError pxScriptView::makeReady(int numArgs, const rtValue* args, rtValue* /*result*/, void* ctx)
{
  rtLogInfo(__FUNCTION__);
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
