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

// pxScene2d.h

#ifndef PX_SCENE2D_H
#define PX_SCENE2D_H

#include <stdio.h>

#include <vector>
#include <list>

#ifndef finline
#ifdef WIN32
#define finline __forceinline
#else
#define finline __attribute__((always_inline))
#endif
#endif

#include "rtRef.h"
#include "rtString.h"

#include "rtCore.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"
#include "rtThreadQueue.h"

#include "pxResource.h"

#include "pxCore.h"
#include "pxIView.h"

#include "pxObject.h"

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"

#include "pxArchive.h"
#include "pxAnimate.h"
#include "testView.h"

class pxConstantsDragType;

#ifdef ENABLE_RT_NODE
#include "rtScript.h"
#endif //ENABLE_RT_NODE

#ifdef ENABLE_PERMISSIONS_CHECK
#include "rtPermissions.h"
#endif
#include "rtCORS.h"

#include "rtServiceProvider.h"
#include "rtSettings.h"

#ifdef PXSCENE_SUPPORT_STORAGE
#include "rtStorage.h"
#endif


#ifdef RUNINMAIN
#define ENTERSCENELOCK()
#define EXITSCENELOCK() 
#else
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK() rtWrapperSceneUpdateExit(); 
class pxScriptView;
class AsyncScriptInfo {
  public:
    pxScriptView * m_pView;
    //pxIViewContainer * m_pWindow;
};
#endif

#define MAX_URL_SIZE 8000

//Uncomment to enable display of pointer by pxScene
//#define USE_SCENE_POINTER

// TODO Move this to pxEventLoop
extern rtThreadQueue* gUIThreadQueue;

// TODO Finish
//#include "pxTransform.h"
#include "pxConstants.h"

// Constants
static pxConstants CONSTANTS;

#if 0
typedef rtError (*objectFactory)(void* context, const char* t, rtObjectRef& o);
void registerObjectFactory(objectFactory f, void* context);
rtError createObject2(const char* t, rtObjectRef& o);
#endif

typedef void (*pxAnimationEnded)(void* ctx);

struct pxAnimationTarget 
{
  char* prop;
  float to;
};

struct animation
{
  bool cancelled;
  bool flip;
  bool reversing;

  rtString prop;

  float from;
  float to;

  double start;
  double duration;

  pxConstantsAnimation::animationOptions  options;
  
  pxInterp interpFunc;

  int32_t count;
  float actualCount;

  rtFunctionRef ended;
  rtObjectRef promise;
  rtObjectRef animateObj;
};

struct pxPoint2f 
{
  pxPoint2f():x(0),y(0) {}
  pxPoint2f(float _x, float _y) { x = _x; y = _y; } 
  float x, y;
};


class rtFileDownloadRequest;

class pxScene2d;
class pxScriptView;
class pxFontManager;

class pxRoot: public pxObject
{
  rtDeclareObject(pxRoot, pxObject);
public:
  pxRoot(pxScene2d* scene): pxObject(scene) {}
  virtual void sendPromise();
};

class pxViewContainer: public pxObject, public pxIViewContainer
{
public:
  rtDeclareObject(pxViewContainer, pxObject);
//  rtProperty(uri, uri, setURI, rtString);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtMethod1ArgAndNoReturn("onMouseDown", onMouseDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseUp",   onMouseUp,   rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseMove", onMouseMove, rtObjectRef);

  rtMethod1ArgAndNoReturn("onDragMove",  onDragMove,  rtObjectRef);
  rtMethod1ArgAndNoReturn("onDragEnter", onDragEnter, rtObjectRef);
  rtMethod1ArgAndNoReturn("onDragLeave", onDragLeave, rtObjectRef);
  rtMethod1ArgAndNoReturn("onDragDrop",  onDragDrop,  rtObjectRef);

  rtMethod1ArgAndNoReturn("onScrollWheel", onScrollWheel, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseEnter",  onMouseEnter,  rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseLeave",  onMouseLeave,  rtObjectRef);

  rtMethod1ArgAndNoReturn("onFocus",   onFocus,   rtObjectRef);
  rtMethod1ArgAndNoReturn("onBlur",    onBlur,    rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyDown", onKeyDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyUp",   onKeyUp,   rtObjectRef);
  rtMethod1ArgAndNoReturn("onChar",    onChar,    rtObjectRef);

  pxViewContainer(pxScene2d* scene):pxObject(scene)
  {
    addListener("onMouseDown", get<rtFunctionRef>("onMouseDown"));
    addListener("onMouseUp", get<rtFunctionRef>("onMouseUp"));
    addListener("onMouseMove", get<rtFunctionRef>("onMouseMove"));

    addListener("onDragMove",  get<rtFunctionRef>("onDragMove"));
    addListener("onDragEnter", get<rtFunctionRef>("onDragEnter"));
    addListener("onDragLeave", get<rtFunctionRef>("onDragLeave"));
    addListener("onDragDrop",  get<rtFunctionRef>("onDragDrop"));

    addListener("onScrollWheel", get<rtFunctionRef>("onScrollWheel"));
    addListener("onMouseEnter",  get<rtFunctionRef>("onMouseEnter"));
    addListener("onMouseLeave",  get<rtFunctionRef>("onMouseLeave"));

    addListener("onFocus",   get<rtFunctionRef>("onFocus"));
    addListener("onBlur",    get<rtFunctionRef>("onBlur"));
    addListener("onKeyDown", get<rtFunctionRef>("onKeyDown"));
    addListener("onKeyUp",   get<rtFunctionRef>("onKeyUp"));
    addListener("onChar",    get<rtFunctionRef>("onChar"));
  }

  virtual ~pxViewContainer() { /*rtLogDebug("#################~pxViewContainer\n");*/}

  rtError setView(pxIView* v)
  {
    if (mView)
      mView->setViewContainer(NULL);

    mView = v;

    if (mView)
    {
      mView->setViewContainer(this);
      mView->onSize(static_cast<int32_t>(mw),static_cast<int32_t>(mh));
    }
    return RT_OK;
  }

  void invalidateRect(pxRect* r);

  virtual void* getInterface(const char* /*name*/)
  {
    return NULL;
  }  

#if 0
  rtError url(rtString& v) const { v = mUri; return RT_OK; }
  rtError setUrl(rtString v) { mUri = v; return RT_OK; }
#endif

  rtError w(float& v) const { v = mw; return RT_OK; }
  rtError setW(float v) 
  { 
    mw = v; 
    if (mView)
      mView->onSize(static_cast<int32_t>(mw),static_cast<int32_t>(mh)); 
    return RT_OK; 
  }
  
  rtError h(float& v) const { v = mh; return RT_OK; }
  rtError setH(float v) 
  { 
    mh = v; 
    if (mView)
      mView->onSize(static_cast<int32_t>(mw),static_cast<int32_t>(mh)); 
    return RT_OK; 
  }

  rtError onMouseDown(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseDown");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onMouseDown(static_cast<int32_t>(x),static_cast<int32_t>(y),flags);
    }
    return RT_OK;
  }

  rtError onMouseUp(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseUp");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onMouseUp(static_cast<int32_t>(x),static_cast<int32_t>(y),flags);
    }
    return RT_OK;
  }

  rtError onMouseMove(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onMouseMove");
    if (mView)
    {
      float x = o.get<float>("x");
      float y = o.get<float>("y");
      mView->onMouseMove(static_cast<int32_t>(x),static_cast<int32_t>(y));
    }
    return RT_OK;
  }

  rtError onDragMove(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onDragMove");
    if (mView)
    {
      float          x = o.get<float>("x");
      float          y = o.get<float>("y");
      int32_t     type = o.get<int32_t>("type");

      mView->onDragMove(static_cast<int32_t>(x), static_cast<int32_t>(y), type );
    }
    return RT_OK;
  }

  rtError onDragEnter(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onDragEnter");
    if (mView)
    {
      float          x = o.get<float>("x");
      float          y = o.get<float>("y");
      int32_t     type = o.get<int32_t>("type");

      mView->onDragEnter(static_cast<int32_t>(x), static_cast<int32_t>(y), type );
    }
    return RT_OK;
  }

  rtError onDragLeave(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onDragLeave");
    if (mView)
    {
      float          x = o.get<float>("x");
      float          y = o.get<float>("y");
      int32_t     type = o.get<int32_t>("type");

      mView->onDragLeave(static_cast<int32_t>(x), static_cast<int32_t>(y), type );
    }
    return RT_OK;
  }

  rtError onDragDrop(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onDragDropped");
    if (mView)
    {
      float          x = o.get<float>("x");
      float          y = o.get<float>("y");
      rtString dropped = o.get<rtString>("dropped");
      int32_t     type = o.get<int32_t>("type");

      mView->onDragDrop(static_cast<int32_t>(x), static_cast<int32_t>(y), type, dropped.cString() );
    }
    return RT_OK;
  }

  rtError onScrollWheel(rtObjectRef o)
  {
    rtLogDebug("pxViewContainer::onScrollWheel");
    if (mView)
    {
      float dx = o.get<float>("dx");
      float dy = o.get<float>("dy");
      bool consumed = mView->onScrollWheel( dx, dy );
      if (consumed)
      {
        rtFunctionRef stopPropagation = o.get<rtFunctionRef>("stopPropagation");
        if (stopPropagation)
        {
          stopPropagation.send();
        }
      }
    }
    return RT_OK;
  }

  rtError onMouseEnter(rtObjectRef /*o*/)
  {
    if (mView)
      mView->onMouseEnter();
    return RT_OK;
  }

  rtError onMouseLeave(rtObjectRef /*o*/)
  {
    if (mView)
      mView->onMouseLeave();
    return RT_OK;
  }

  rtError onFocus(rtObjectRef /*o*/)
  {
    if (mView) {
      mView->onFocus();
    }
    return RT_OK;
  }
  rtError onBlur(rtObjectRef /*o*/)
  {
    if (mView) {
      mView->onBlur();
    }
    return RT_OK;
  }
  rtError onKeyDown(rtObjectRef e)
  {
    if (mView)
    {
      rtLogDebug("pxViewContainer::onKeyDown enter\n");
      uint32_t keyCode = e.get<uint32_t>("keyCode");
      uint32_t flags = e.get<uint32_t>("flags");
      bool consumed = mView->onKeyDown(keyCode, flags);
      if (consumed)
      {
        rtFunctionRef stopPropagation = e.get<rtFunctionRef>("stopPropagation");
        if (stopPropagation)
        {
          stopPropagation.send();
        }
      }
    }
    return RT_OK;
  }

  rtError onKeyUp(rtObjectRef e)
  {
    if (mView)
    {
      uint32_t keyCode = e.get<uint32_t>("keyCode");
      uint32_t flags = e.get<uint32_t>("flags");
      bool consumed = mView->onKeyUp(keyCode, flags);
      if (consumed)
      {
        rtFunctionRef stopPropagation = e.get<rtFunctionRef>("stopPropagation");
        if (stopPropagation)
        {
          stopPropagation.send();
        }
      }
    }
    return RT_OK;
  }

  rtError onChar(rtObjectRef e)
  {
    if (mView)
    {
      uint32_t codePoint = e.get<uint32_t>("charCode");
      bool consumed = mView->onChar(codePoint);
      if (consumed)
      {
        rtFunctionRef stopPropagation = e.get<rtFunctionRef>("stopPropagation");
        if (stopPropagation)
        {
          stopPropagation.send();
        }
      }
    }
    return RT_OK;
  }

  virtual void update(double t, bool updateChildren=true)
  {
    if (mView)
      mView->onUpdate(t);
    pxObject::update(t,updateChildren);
  }

  virtual void draw() 
  {
    if (mView)
      mView->onDraw();
  }

  

protected:
  pxViewRef mView;
  rtString mUrl;
};

static int pxSceneContainerCount = 0;
class pxSceneContainer: public pxViewContainer
{
public:
  rtDeclareObject(pxSceneContainer, pxViewContainer);
  rtProperty(url, url, setUrl, rtString);
#ifdef ENABLE_PERMISSIONS_CHECK
  // permissions can be set to either scene or to its container
  rtProperty(permissions, permissions, setPermissions, rtObjectRef);
#endif
  rtReadOnlyProperty(cors, cors, rtObjectRef);
  rtReadOnlyProperty(api, api, rtValue);
  rtReadOnlyProperty(ready, ready, rtObjectRef);
  rtProperty(serviceContext, serviceContext, setServiceContext, rtObjectRef);
  rtMethod1ArgAndReturn("suspend", suspend, rtValue, bool);
  rtMethod1ArgAndReturn("resume", resume, rtValue, bool);
  rtMethod1ArgAndReturn("screenshot", screenshot, rtString, rtValue);

//  rtMethod1ArgAndNoReturn("makeReady", makeReady, bool);  // DEPRECATED ?
  
  pxSceneContainer(pxScene2d* scene):pxViewContainer(scene){  pxSceneContainerCount++;}
  virtual ~pxSceneContainer() {rtLogDebug("###############~pxSceneContainer\n");pxSceneContainerCount--;}

  virtual unsigned long Release()
  {
    unsigned long c = pxViewContainer::Release();
//    rtLogDebug("pxSceneContainer::Release(): %ld\n", c);
    return c;
  }

  virtual void dispose(bool pumpJavascript);
  rtError url(rtString& v) const { v = mUrl; return RT_OK; }
  rtError setUrl(rtString v);

  rtError setScriptView(pxScriptView* scriptView);

  rtError api(rtValue& v) const;
  rtError ready(rtObjectRef& o) const;
  
  rtError serviceContext(rtObjectRef& o) const { o = mServiceContext; return RT_OK;}
  rtError setServiceContext(rtObjectRef o);

  rtError suspend(const rtValue& v, bool& b);
  rtError resume(const rtValue& v, bool& b);
  rtError screenshot(rtString type, rtValue& returnValue);

#ifdef ENABLE_PERMISSIONS_CHECK
  rtError permissions(rtObjectRef& v) const;
  rtError setPermissions(const rtObjectRef& v);
#endif
  rtError cors(rtObjectRef& v) const;

//  rtError makeReady(bool ready);  // DEPRECATED ?

  // in the case of pxSceneContainer, the makeReady should be the  
  // catalyst for ready to fire, so override sendPromise  
  // to prevent firing from update() 
  virtual void sendPromise() { /*rtLogDebug("pxSceneContainer ignoring sendPromise\n");*/ }

  virtual void* getInterface(const char* name);
  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage(std::vector<rtObject*> &objectsCounted);
  
private:
  rtRef<pxScriptView> mScriptView;
  rtString mUrl;
  rtObjectRef mServiceContext;
};
typedef rtRef<pxSceneContainer> pxSceneContainerRef;

typedef rtRef<pxObject> pxObjectRef;

// Important that this have a separate lifetime from scene object
// and to not hold direct references to this objects from the script context
// Don't make this into an rtObject
class pxScriptView: public pxIView
{
public:
  pxScriptView(const char* url, const char* /*lang*/, pxIViewContainer* container=NULL);
#ifndef RUNINMAIN
  void runScript(); // Run the script
#endif
  virtual ~pxScriptView()
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
      mGetSetting->clearContext();

      // TODO Given that the context is being cleared we likely don't need to zero these out
      mCtx->add("getScene", 0);
      mCtx->add("makeReady", 0);
      mCtx->add("getContextID", 0);
      mCtx->add("getSetting", 0);
    }
#endif //ENABLE_RT_NODE

    if (mView)
      mView->setViewContainer(NULL);

    // TODO JRJR Do we have GC tests yet
    // Hack to try and reduce leaks until garbage collection can
    // be cleaned up
    if(mScene)
      mEmit.send("onSceneRemoved", mScene);

    if (mScene)
      mScene.send("dispose");

    mView = NULL;
    mScene = NULL;
  }

  virtual unsigned long AddRef() 
  {
    //rtLogInfo(__FUNCTION__);
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() 
  {
    //rtLogInfo(__FUNCTION__);
    long l = rtAtomicDec(&mRefCount);
    //  rtLogDebug("pxScene2d release %ld\n",l);
    if (l == 0)
      delete this;
    return l;
  }

  rtError api(rtValue& v)
  {
    if (!mApi)
      return RT_FAIL;

    v = mApi;
    return RT_OK;
  }

  rtError ready(rtObjectRef& o)
  {
    if (!mReady)
      return RT_FAIL;
    
    o = mReady;
    return RT_OK;
  }

  rtString getUrl() const { return mUrl; }

#ifdef ENABLE_PERMISSIONS_CHECK
  rtError permissions(rtObjectRef& v) const { return mScene.get("permissions", v); }
  rtError setPermissions(const rtObjectRef& v) { return mScene.set("permissions", v); }
#endif
  rtError cors(rtObjectRef& v) const { return mScene.get("cors", v); }

  static rtError addListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  static rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError suspend(const rtValue& v, bool& b);
  rtError resume(const rtValue& v, bool& b);
  rtError textureMemoryUsage(rtValue& v);

  rtError screenshot(rtString type, rtValue& returnValue);
  
protected:


  static rtError getScene(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx);
  static rtError makeReady(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx);

  static rtError getContextID(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* /*ctx*/);
  static rtError getSetting(int numArgs, const rtValue* args, rtValue* result, void* /*ctx*/);

  virtual void onSize(int32_t w, int32_t h)
  {
    mWidth = w;
    mHeight = h;
    if (mView)
      mView->onSize(w,h);
  }

  virtual void onCloseRequest()
  {
    rtLogDebug("pxScriptView::onCloseRequest()\n");
    if (mScene)
      mScene.send("dispose");
    mScene = NULL;
    mView = NULL;
  }

  virtual bool onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    if (mView)
      return mView->onMouseDown(x,y,flags);
    return false;
  }

  virtual bool onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    if (mView)
      return mView->onMouseUp(x,y,flags);
    return false;
  }

  virtual bool onMouseMove(int32_t x, int32_t y)
  {
    if (mView)
      return mView->onMouseMove(x,y);
    return false;
  }

  virtual bool onDragMove(int32_t x, int32_t y, int32_t type)
  {
    if (mView)
    return mView->onDragMove(x,y, type);
    return false;
  }

  virtual bool onDragEnter(int32_t x, int32_t y, int32_t type)
  {
    if (mView)
    return mView->onDragEnter(x,y, type);
    return false;
  }

  virtual bool onDragLeave(int32_t x, int32_t y, int32_t type)
  {
    if (mView)
    return mView->onDragLeave(x,y, type);
    return false;
  }

  virtual bool onDragDrop(int32_t x, int32_t y, int32_t type, const char *dropped)
  {
    if (mView)
    return mView->onDragDrop(x,y, type, dropped);
    return false;
  }

  virtual bool onScrollWheel(float dx, float dy)
  {
    if (mView)
      return mView->onScrollWheel(dx,dy);
    return false;
  }
  
  virtual bool onMouseEnter()
  {
    if (mView)
      return mView->onMouseEnter();
    return false;
  }

  virtual bool onMouseLeave()
  {
    if (mView)
      return mView->onMouseLeave();
    return false;
  }

  virtual bool onFocus()
  {
    if (mView)
      return mView->onFocus();
    return false;
  }

  virtual bool onBlur()
  {
    if (mView)
      return mView->onBlur();
    return false;
  }

  virtual bool onKeyDown(uint32_t keycode, uint32_t flags)
  {
    if (mView)
      return mView->onKeyDown(keycode, flags);
    return false;
  }

  virtual bool onKeyUp(uint32_t keycode, uint32_t flags)
  {
    if (mView)
      return mView->onKeyUp(keycode,flags);
    return false;
  }

  virtual bool onChar(uint32_t codepoint)
  {
    if (mView)
      return mView->onChar(codepoint);
    return false;
  }

  virtual void onUpdate(double t)
  {
    if (mView)
      mView->onUpdate(t);
  }

  virtual void onDraw(/*pxBuffer& b, pxRect* r*/)
  {
    if (mView)
      mView->onDraw();
  }

  virtual void setViewContainer(pxIViewContainer* l)
  {
    if (mView)
      mView->setViewContainer(l);
    mViewContainer = l;
  }

  int mWidth;
  int mHeight;
  rtObjectRef mApi;
  rtObjectRef mReady;
  rtObjectRef mScene;
  rtRef<pxIView> mView;
  rtRef<rtFunctionCallback> mGetScene;
  rtRef<rtFunctionCallback> mMakeReady;
  rtRef<rtFunctionCallback> mGetContextID;
  rtRef<rtFunctionCallback> mGetSetting;

#ifdef ENABLE_RT_NODE
  rtScriptContextRef mCtx;
#endif //ENABLE_RT_NODE
  pxIViewContainer* mViewContainer;
  unsigned long mRefCount;
  rtString mUrl;
#ifndef RUNINMAIN
  rtString mLang;
#endif
  static rtEmitRef mEmit;
};

class pxScene2d: public rtObject, public pxIView, public rtIServiceProvider
{
public:
  rtDeclareObject(pxScene2d, rtObject);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtProperty(showDirtyRect, showDirtyRect, setShowDirtyRect, bool);
  rtProperty(reportFps, reportFps, setReportFps, bool);
  rtReadOnlyProperty(dirtyRectangle, dirtyRectangle, rtObjectRef);
  rtReadOnlyProperty(dirtyRectanglesEnabled, dirtyRectanglesEnabled, bool);
  rtProperty(enableDirtyRect, enableDirtyRect, setEnableDirtyRect, bool);
  rtProperty(customAnimator, customAnimator, setCustomAnimator, rtFunctionRef);
  rtMethod1ArgAndReturn("loadArchive",loadArchive,rtString,rtObjectRef); 
  rtMethod1ArgAndReturn("create", create, rtObjectRef, rtObjectRef);
  rtMethodNoArgAndReturn("clock", clock, double);
  rtMethodNoArgAndNoReturn("logDebugMetrics", logDebugMetrics);
  rtMethodNoArgAndNoReturn("collectGarbage", collectGarbage);
  rtReadOnlyProperty(info, info, rtObjectRef);
  rtReadOnlyProperty(capabilities, capabilities, rtObjectRef);
  rtMethod1ArgAndReturn("suspend", suspend, rtValue, bool);
  rtMethod1ArgAndReturn("resume", resume, rtValue, bool);
  rtMethodNoArgAndReturn("suspended", suspended, bool);
  rtMethodNoArgAndReturn("textureMemoryUsage", textureMemoryUsage, rtValue);
/*
  rtMethod1ArgAndReturn("createExternal", createExternal, rtObjectRef,
                        rtObjectRef);
  rtMethod1ArgAndReturn("createWayland", createWayland, rtObjectRef,
                        rtObjectRef);
*/
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  // TODO make this a property
  // focus is now a bool property on pxObject
  //rtMethod1ArgAndNoReturn("setFocus", setFocus, rtObjectRef);
  rtMethodNoArgAndReturn("getFocus", getFocus, rtObjectRef);
  
  
//  rtMethodNoArgAndNoReturn("stopPropagation",stopPropagation);
  
  rtMethod1ArgAndReturn("screenshot", screenshot, rtString, rtValue);

  rtMethod1ArgAndReturn("clipboardGet", clipboardGet, rtString, rtString);
  rtMethod2ArgAndNoReturn("clipboardSet", clipboardSet, rtString, rtString);

  rtMethod1ArgAndReturn("getService", getService, rtString, rtObjectRef);

  rtMethodNoArgAndReturn("getAvailableApplications", getAvailableApplications, rtString);
    
    
  rtProperty(ctx, ctx, setCtx, rtValue);
  rtProperty(api, api, setAPI, rtValue);
//  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  // Properties for returning various CONSTANTS
  rtReadOnlyProperty(animation,animation,rtObjectRef);
  rtReadOnlyProperty(stretch,stretch,rtObjectRef);
  rtReadOnlyProperty(maskOp,maskOp,rtObjectRef);
  rtReadOnlyProperty(dragType,dragType,rtObjectRef);
  rtReadOnlyProperty(alignVertical,alignVertical,rtObjectRef);
  rtReadOnlyProperty(alignHorizontal,alignHorizontal,rtObjectRef);
  rtReadOnlyProperty(truncation,truncation,rtObjectRef);

  rtMethodNoArgAndNoReturn("dispose",dispose);

  rtMethod1ArgAndReturn("sparkSetting", sparkSetting, rtString, rtValue);
  rtMethod1ArgAndNoReturn("addServiceProvider", addServiceProvider, rtFunctionRef);
  rtMethod1ArgAndNoReturn("removeServiceProvider", removeServiceProvider, rtFunctionRef);

  rtReadOnlyProperty(storage,storage,rtObjectRef);

#ifdef ENABLE_PERMISSIONS_CHECK
  // permissions can be set to either scene or to its container
  rtProperty(permissions, permissions, setPermissions, rtObjectRef);
#endif
  rtReadOnlyProperty(cors, cors, rtObjectRef);

  pxScene2d(bool top = true, pxScriptView* scriptView = NULL);
  virtual ~pxScene2d()
  {
     rtLogDebug("***** deleting pxScene2d\n");
    if (mTestView != NULL)
    {
       //delete mTestView; // HACK: Only used in testing... 'delete' causes unknown crash.
       mTestView = NULL;
    }
    if (mArchive != NULL)
    {
       mArchive = NULL;
    }
    mArchiveSet = false;
  }

  virtual unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  virtual unsigned long Release()
  {
    long l = rtAtomicDec(&mRefCount);
    //  rtLogDebug("pxScene2d release %ld\n",l);
    if (l == 0)
      delete this;
    return l;
  }

  rtError addServiceProvider(const rtFunctionRef& p)
  {
    if (p)
      mServiceProviders.push_back(p);
    return RT_OK;
  }

  rtError removeServiceProvider(const rtFunctionRef& p)
  {
    if (p)
    {
      for(std::vector<rtFunctionRef>::iterator i = mServiceProviders.begin(); i != mServiceProviders.end(); i++)
      {
        if (*i == p)
        {
          mServiceProviders.erase(i);
          break;
        }
      }
    }
    return RT_OK;
  }

//  void init();
  rtError dispose();
  void onCloseRequest();
  int32_t w() const { return mWidth;  }
  rtError w(int32_t& v) const { v = mWidth;  return RT_OK; }
  int32_t h() const { return mHeight; }
  rtError h(int32_t& v) const { v = mHeight; return RT_OK; }

  rtError showOutlines(bool& v) const;
  rtError setShowOutlines(bool v);

  rtError reportFps(bool& v) const;
  rtError setReportFps(bool v);

  rtError showDirtyRect(bool& v) const;
  rtError setShowDirtyRect(bool v);

  rtError dirtyRectangle(rtObjectRef& v) const;
  rtError dirtyRectanglesEnabled(bool& v) const;

  rtError enableDirtyRect(bool& v) const;
  rtError setEnableDirtyRect(bool v);

  rtError customAnimator(rtFunctionRef& f) const;
  rtError setCustomAnimator(const rtFunctionRef& f);

  rtError create(rtObjectRef p, rtObjectRef& o);

  rtError createObject(rtObjectRef p, rtObjectRef& o);
  rtError createRectangle(rtObjectRef p, rtObjectRef& o);
  rtError createText(rtObjectRef p, rtObjectRef& o);
  rtError createTextBox(rtObjectRef p, rtObjectRef& o);
  rtError createImage(rtObjectRef p, rtObjectRef& o);
  rtError createPath(rtObjectRef p, rtObjectRef& o);
  rtError createImage9(rtObjectRef p, rtObjectRef& o);
  rtError createImageA(rtObjectRef p, rtObjectRef& o);
  rtError createImage9Border(rtObjectRef p, rtObjectRef& o);
  rtError createImageResource(rtObjectRef p, rtObjectRef& o);
  rtError createImageAResource(rtObjectRef p, rtObjectRef& o);
  rtError createFontResource(rtObjectRef p, rtObjectRef& o);
  rtError createShaderResource(rtObjectRef p, rtObjectRef& o);
  rtError createScene(rtObjectRef p,rtObjectRef& o);
  rtError createExternal(rtObjectRef p, rtObjectRef& o);
  rtError createWayland(rtObjectRef p, rtObjectRef& o);
  rtError createVideo(rtObjectRef p, rtObjectRef& o);

  rtError clock(double & time);
  rtError logDebugMetrics();
  rtError collectGarbage();
  rtError suspend(const rtValue& v, bool& b);
  rtError resume(const rtValue& v, bool& b);
  rtError suspended(bool &b);
  rtError textureMemoryUsage(rtValue &v);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError getFocus(rtObjectRef& o)
  {
    o = mFocusObj;
    return RT_OK;
  }

  rtError setFocus(rtObjectRef o);

#if 0
  rtError stopPropagation()
  {
    rtLogDebug("stopPropagation()\n");
    mStopPropagation = true;
    return RT_OK;
  }
#endif

  rtError ctx(rtValue& v) const { v = mContext; return RT_OK; }
  rtError setCtx(const rtValue& v) { mContext = v; return RT_OK; }

  rtError api(rtValue& v) const { v = mAPI; return RT_OK; }
  rtError setAPI(const rtValue& v) { mAPI = v; return RT_OK; }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }

  rtError animation(rtObjectRef& v)       const {v = CONSTANTS.animationConstants;       return RT_OK;}
  rtError stretch(rtObjectRef& v)         const {v = CONSTANTS.stretchConstants;         return RT_OK;}
  rtError maskOp(rtObjectRef& v)          const {v = CONSTANTS.maskOpConstants;          return RT_OK;}
  rtError dragType(rtObjectRef& v)        const {v = CONSTANTS.dragTypeConstants;        return RT_OK;}
  rtError alignVertical(rtObjectRef& v)   const {v = CONSTANTS.alignVerticalConstants;   return RT_OK;}
  rtError alignHorizontal(rtObjectRef& v) const {v = CONSTANTS.alignHorizontalConstants; return RT_OK;}
  rtError truncation(rtObjectRef& v)      const {v = CONSTANTS.truncationConstants;      return RT_OK;}

#ifdef ENABLE_PERMISSIONS_CHECK
  rtPermissionsRef permissions() const { return mPermissions; }
  rtError permissions(rtObjectRef& v) const { v = mPermissions; return RT_OK; }
  rtError setPermissions(const rtObjectRef& v) { return mPermissions->set(v); }
#endif
  rtCORSRef cors() const { return mCORS; }
  rtError cors(rtObjectRef& v) const { v = mCORS; return RT_OK; }
  rtError sparkSetting(const rtString& setting, rtValue& value) const;

   void setMouseEntered(rtRef<pxObject> o, int32_t x = 0, int32_t y = 0);
   void clearMouseObject(rtRef<pxObject>);

  // The following methods are delegated to the view
  virtual void onSize(int32_t w, int32_t h);

  virtual bool onMouseEnter();
  virtual bool onMouseLeave();

  virtual bool onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseUp(  int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseMove(int32_t x, int32_t y);

  virtual bool onScrollWheel(float dx, float dy);

  virtual bool onDragMove( int32_t x, int32_t y, int32_t type);
  virtual bool onDragEnter(int32_t x, int32_t y, int32_t type);
  virtual bool onDragLeave(int32_t x, int32_t y, int32_t type);
  virtual bool onDragDrop( int32_t x, int32_t y, int32_t type, const char* dropped);

  void updateMouseEntered();

  virtual bool onFocus();
  virtual bool onBlur();

  virtual bool onKeyDown(uint32_t keycode, uint32_t flags);
  virtual bool onKeyUp(uint32_t keycode, uint32_t flags);
  virtual bool onChar(uint32_t codepoint);

  virtual void onUpdate(double t);
  virtual void onDraw();
  virtual void onComplete();

  virtual void setViewContainer(pxIViewContainer* l);
  pxIViewContainer* viewContainer();
  void invalidateRect(pxRect* r);

  void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m);
  void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m);
  void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m);

  void transformPointFromObjectToScene(pxObject* o, const pxPoint2f& from, pxPoint2f& to);
  void transformPointFromSceneToObject(pxObject* o, const pxPoint2f& from, pxPoint2f& to);
  void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject,
					pxPoint2f& from, pxPoint2f& to);

  void hitTest(pxPoint2f p, std::vector<rtRef<pxObject> > hitList);

  pxObject* getRoot() const;
  rtError root(rtObjectRef& v) const
  {
    v = getRoot();
    return RT_OK;
  }

  rtObjectRef  getInfo() const;
  rtError info(rtObjectRef& v) const
  {
    v = getInfo();
    return RT_OK;
  }

  rtObjectRef  getCapabilities() const;
  rtError capabilities(rtObjectRef& v) const
  {
    v = getCapabilities();
    return RT_OK;
  }

  rtError loadArchive(const rtString& url, rtObjectRef& archive)
  {
#ifdef ENABLE_PERMISSIONS_CHECK
    if (RT_OK != mPermissions->allows(url, rtPermissions::DEFAULT))
      return RT_ERROR_NOT_ALLOWED;
#endif

    rtError e = RT_FAIL;
    rtRef<pxArchive> a = new pxArchive;
    pxIViewContainer* view = viewContainer();
    pxSceneContainer* sceneContainer = (pxSceneContainer*)view;
    rtObjectRef parentArchive = NULL;
    if (NULL != sceneContainer)
    {
      pxScene2d* scene = sceneContainer->getScene();
      if (NULL != scene)
      {
        parentArchive = scene->getArchive();
      }
    }
    if (a->initFromUrl(url, mCORS, parentArchive) == RT_OK)
    {
      archive = a;
      e = RT_OK;
    }

    if (false == mArchiveSet)
    {
      pxArchive* myArchive = (pxArchive*) a.getPtr();
      /* decide whether further file access from this scene need to to taken from,
      parent -> if this scene is created from archive
      itself -> if this file itself is archive
      */
      if ((parentArchive != NULL ) && (((pxArchive*)parentArchive.getPtr())->isFile() == false))
      {
        if ((myArchive != NULL ) && (myArchive->isFile() == false))
        {
          mArchive = a;
        }
        else
        {
          mArchive = parentArchive;
        }
      }
      else
      {
        mArchive = a;
      }
      mArchiveSet = true;
    }
    return e;
  }

  void innerpxObjectDisposed(rtObjectRef ref);
  bool isObjectTracked(rtObjectRef ref);
  // Note: Only type currently supported is "image/png;base64"
  rtError screenshot(rtString type, rtValue& returnValue);
  rtError clipboardGet(rtString type, rtString& retString);
  rtError clipboardSet(rtString type, rtString clipString);
  rtError getService(rtString name, rtObjectRef& returnObject);
  rtError getService(const char* name, const rtObjectRef& ctx, rtObjectRef& service);
  rtError getAvailableApplications(rtString& availableApplications);
  rtObjectRef getArchive()
  {
    return mArchive;
  }

  rtError storage(rtObjectRef& v) const;

  static void enableOptimizedUpdate(bool enable);
  static void updateObject(pxObject* o, bool update);

private:
  static void updateObjects(double t);
  bool bubbleEvent(rtObjectRef e, rtRef<pxObject> t, 
                   const char* preEvent, const char* event) ;
  
  bool bubbleEventOnBlur(rtObjectRef e, rtRef<pxObject> t, rtRef<pxObject> o);

  void draw();
  // Does not draw updates scene to time t
  // t is assumed to be monotonically increasing
  void update(double t);


  rtRef<pxObject> mRoot;
  rtObjectRef mInfo;
  rtObjectRef mCapabilityVersions;
  rtObjectRef mFocusObj;
  double start, sigma_draw, sigma_update, end2;

  int frameCount;
  int mWidth;
  int mHeight;

  rtEmitRef mEmit;

// TODO Top level scene only
  rtRef<pxObject> mMouseEntered;
  rtRef<pxObject> mMouseDown;
  pxPoint2f mMouseDownPt;
  rtValue mContext;
  rtValue mAPI;
  bool mTop;
  bool mStopPropagation;
  int mTag;
  pxIViewContainer *mContainer;
  pxScriptView *mScriptView;
  bool mReportFps;
  bool mShowDirtyRectangle;
  bool mEnableDirtyRectangles;
  int32_t mPointerX;
  int32_t mPointerY;
  double mPointerLastUpdated;

  #ifdef USE_SCENE_POINTER
  pxTextureRef mNullTexture;
  rtObjectRef mPointerResource;
  pxTextureRef mPointerTexture;
  int32_t mPointerW;
  int32_t mPointerH;
  int32_t mPointerHotSpotX;
  int32_t mPointerHotSpotY;
  #endif
  bool mPointerHidden;
  std::vector<rtObjectRef> mInnerpxObjects;
  rtFunctionRef mCustomAnimator;
#ifdef ENABLE_PERMISSIONS_CHECK
  rtPermissionsRef mPermissions;
#endif
  bool mSuspended;
  rtCORSRef mCORS;
  rtObjectRef mArchive;
public:
  void hidePointer( bool hide )
  {
     mPointerHidden= hide;
  }
  //#ifdef PX_DIRTY_RECTANGLES
  pxRect mDirtyRect;
  pxRect mLastFrameDirtyRect;
  //#endif //PX_DIRTY_RECTANGLES
  bool mDirty;

  bool mDragging;
  pxConstantsDragType::constants mDragType;
  rtRef<pxObject> mDragTarget;

  testView* mTestView;
  bool mDisposed;
  std::vector<rtFunctionRef> mServiceProviders;
  bool mArchiveSet;
#ifdef PXSCENE_SUPPORT_STORAGE
  mutable rtStorageRef mStorage;
#endif
  static bool mOptimizedUpdateEnabled;
};

// TODO do we need this anymore?
class pxScene2dRef: public rtRef<pxScene2d>, public rtObjectBase
{
 public:
  pxScene2dRef() {}
  pxScene2dRef(pxScene2d* s) { asn(s); }

  // operator= is not inherited
  pxScene2dRef& operator=(pxScene2d* s) { asn(s); return *this; }
  
 private:
  virtual rtError Get(const char* name, rtValue* value) const override;
  virtual rtError Get(uint32_t i, rtValue* value) const override;
  virtual rtError Set(const char* name, const rtValue* value) override;
  virtual rtError Set(uint32_t i, const rtValue* value) override;
};


#endif
