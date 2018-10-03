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

#define ANIMATION_ROTATE_XYZ

#include "pxResource.h"

#include "pxCore.h"
#include "pxIView.h"

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"

#include "pxArchive.h"
#include "pxAnimate.h"
#include "testView.h"

#ifdef ENABLE_RT_NODE
#include "rtScript.h"
#endif //ENABLE_RT_NODE

#ifdef ENABLE_PERMISSIONS_CHECK
#include "rtPermissions.h"
#endif
#include "rtCORS.h"

#include "rtServiceProvider.h"

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
class pxObject: public rtObject
{
public:
  rtDeclareObject(pxObject, rtObject);
  rtReadOnlyProperty(_pxObject, _pxObject, voidPtr);
  rtProperty(parent, parent, setParent, rtObjectRef);
  rtProperty(x, x, setX, float); 
  rtProperty(y, y, setY, float);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtProperty(px, px, setPX, float);
  rtProperty(py, py, setPY, float);
  rtProperty(cx, cx, setCX, float);
  rtProperty(cy, cy, setCY, float);
  rtProperty(sx, sx, setSX, float);
  rtProperty(sy, sy, setSY, float);
  rtProperty(a, a, setA, float);
  rtProperty(r, r, setR, float);
#ifdef ANIMATION_ROTATE_XYZ
  rtProperty(rx, rx, setRX, float);
  rtProperty(ry, ry, setRY, float);
  rtProperty(rz, rz, setRZ, float);
#endif // ANIMATION_ROTATE_XYZ
  rtProperty(id, id, setId, rtString);
  rtProperty(interactive, interactive, setInteractive, bool);
  rtProperty(painting, painting, setPainting, bool);
  rtProperty(clip, clip, setClip, bool);
  rtProperty(mask, mask, setMask, bool);
  rtProperty(draw, drawEnabled, setDrawEnabled, bool);
  rtProperty(hitTest, hitTest, setHitTest, bool);
  rtProperty(focus, focus, setFocus, bool); 
  rtReadOnlyProperty(ready, ready, rtObjectRef);

  rtReadOnlyProperty(numChildren, numChildren, int32_t);
  rtMethod1ArgAndReturn("getChild", getChild, uint32_t, rtObjectRef);
  rtReadOnlyProperty(children, children, rtObjectRef);

  rtMethodNoArgAndNoReturn("remove", remove);
  rtMethodNoArgAndNoReturn("removeAll", removeAll);
  rtMethodNoArgAndNoReturn("moveToFront", moveToFront);
  rtMethodNoArgAndNoReturn("moveToBack", moveToBack);
  rtMethodNoArgAndNoReturn("moveForward", moveForward);
  rtMethodNoArgAndNoReturn("moveBackward", moveBackward);

  rtMethod5ArgAndReturn("animateTo", animateToP2, rtObjectRef, double,
                        uint32_t, uint32_t, int32_t, rtObjectRef);

  rtMethod5ArgAndReturn("animate", animateToObj, rtObjectRef, double,
                        uint32_t, uint32_t, int32_t, rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
  rtMethodNoArgAndNoReturn("dispose",releaseResources);
 // rtProperty(onReady, onReady, setOnReady, rtFunctionRef);

//  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  rtMethod1ArgAndReturn("getObjectById",getObjectById,rtString,rtObjectRef);

  rtProperty(m11,m11,setM11,float);
  rtProperty(m12,m12,setM12,float);
  rtProperty(m13,m13,setM13,float);
  rtProperty(m14,m14,setM14,float);
  rtProperty(m21,m21,setM21,float);
  rtProperty(m22,m22,setM22,float);
  rtProperty(m23,m23,setM23,float);
  rtProperty(m24,m24,setM24,float);
  rtProperty(m31,m31,setM31,float);
  rtProperty(m32,m32,setM32,float);
  rtProperty(m33,m33,setM33,float);
  rtProperty(m34,m34,setM34,float);
  rtProperty(m41,m41,setM41,float);
  rtProperty(m42,m42,setM42,float);
  rtProperty(m43,m43,setM43,float);
  rtProperty(m44,m44,setM44,float);
  rtProperty(useMatrix,useMatrix,setUseMatrix,bool);

  pxObject(pxScene2d* scene);
  virtual unsigned long Release()
  {
    rtString d;
    // Why is this bad
    //sendReturns<rtString>("description",d);
    //rtString d2 = getMap()->className;
    unsigned long c =  rtObject::Release();
    #if 0
    if (c == 0)
      rtLogDebug("pxObject %s  %ld: %s\n", (c==0)?" *********** Destroyed":"Released",c, d2.cString());
    #endif
    return c;
  }

  virtual ~pxObject() ;

  

  // TODO missing conversions in rtValue between uint32_t and int32_t
  size_t numChildren() const { return mChildren.size(); }
  rtError numChildren(int32_t& v) const 
  {
    v = (int32_t) mChildren.size();
    return RT_OK;
  }

  virtual rtError Set(uint32_t i, const rtValue* value) override;
  virtual rtError Set(const char* name, const rtValue* value) override;

  rtError getChild(uint32_t i, rtObjectRef& r) const
  {
    if(i >= mChildren.size())
    {
      return RT_ERROR_INVALID_ARG;
    }
    
    r = mChildren[i];
    return RT_OK;
  }

  rtError children(rtObjectRef& v) const;

  // TODO clean this up
  void setParent(rtRef<pxObject>& parent);
  pxObject* parent() const
  {
    return mParent;
  }

  rtError parent(rtObjectRef& v) const 
  {
    v = mParent;
    return RT_OK;
  }

  rtError setParent(rtObjectRef parent) 
  {
    rtRef<pxObject> p;

    if (parent)
      p = (pxObject*)parent.get<voidPtr>("_pxObject");
    
    setParent(p);

    return RT_OK;
  }

  rtError remove();
  rtError removeAll();
  
  rtString id() { return mId; }
  rtError id(rtString& v) const { v = mId; return RT_OK; }
  rtError setId(const rtString& v) { mId = v; return RT_OK; }

  rtError interactive(bool& v) const { v = mInteractive; return RT_OK; }
  rtError setInteractive(bool v) { mInteractive = v; return RT_OK; }

  float x()             const { return mx; }
  rtError x(float& v)   const { v = mx; return RT_OK;   }
  rtError setX(float v)       { cancelAnimation("x"); mx = v; return RT_OK;   }
  float y()             const { return my; }
  rtError y(float& v)   const { v = my; return RT_OK;   }
  rtError setY(float v)       { cancelAnimation("y"); my = v; return RT_OK;   }
  float w()             const { return mw; }
  rtError w(float& v)   const { v = mw; return RT_OK;   }
  virtual rtError setW(float v)       { cancelAnimation("w"); createNewPromise();mw = v; return RT_OK;   }
  float h()             const { return mh; }
  rtError h(float& v)   const { v = mh; return RT_OK;   }
  virtual rtError setH(float v)       { cancelAnimation("h"); createNewPromise();mh = v; return RT_OK;   }
  float px()            const { return mpx;}
  rtError px(float& v)  const { v = mpx; return RT_OK;  }
  rtError setPX(float v)      { cancelAnimation("px"); createNewPromise();mpx = (v > 1) ? 1 : (v < 0) ? 0 : v; return RT_OK;  }
  float py()            const { return mpy;}
  rtError py(float& v)  const { v = mpy; return RT_OK;  }
  rtError setPY(float v)      { cancelAnimation("py"); createNewPromise();mpy = (v > 1) ? 1 : (v < 0) ? 0 : v; return RT_OK;  }
  float cx()            const { return mcx;}
  rtError cx(float& v)  const { v = mcx; return RT_OK;  }
  rtError setCX(float v)      { cancelAnimation("cx"); createNewPromise();mcx = v; return RT_OK;  }
  float cy()            const { return mcy;}
  rtError cy(float& v)  const { v = mcy; return RT_OK;  }
  rtError setCY(float v)      { cancelAnimation("cy"); createNewPromise();mcy = v; return RT_OK;  }
  float sx()            const { return msx;}
  rtError sx(float& v)  const { v = msx; return RT_OK;  }
  rtError setSX(float v)      { cancelAnimation("sx"); createNewPromise();msx = v; return RT_OK;  }
  float sy()            const { return msy;}
  rtError sy(float& v)  const { v = msx; return RT_OK;  } 
  rtError setSY(float v)      { cancelAnimation("sy");createNewPromise(); msy = v; return RT_OK;  }
  float a()             const { return ma; }
  rtError a(float& v)   const { v = ma; return RT_OK;   }
  rtError setA(float v)       { cancelAnimation("a"); ma = v; return RT_OK;   }
  float r()             const { return mr; }
  rtError r(float& v)   const { v = mr; return RT_OK;   }
  rtError setR(float v)       { cancelAnimation("r"); createNewPromise();mr = v; return RT_OK;   }
#ifdef ANIMATION_ROTATE_XYZ
  float rx()            const { return mrx;}
  rtError rx(float& v)  const { v = mrx; return RT_OK;  }
  rtError setRX(float v)      { cancelAnimation("rx"); createNewPromise(); mrx = v; return RT_OK;  }
  float ry()            const { return mry;}
  rtError ry(float& v)  const { v = mry; return RT_OK;  }
  rtError setRY(float v)      { cancelAnimation("ry"); createNewPromise();mry = v; return RT_OK;  }
  float rz()            const { return mrz;}
  rtError rz(float& v)  const { v = mrz; return RT_OK;  }
  rtError setRZ(float v)      { cancelAnimation("rz"); createNewPromise();mrz = v; return RT_OK;  }
#endif // ANIMATION_ROTATE_XYZ
  bool painting()            const { return mPainting;}
  rtError painting(bool& v)  const { v = mPainting; return RT_OK;  }
  rtError setPainting(bool v);

  bool clip()            const { return mClip;}
  rtError clip(bool& v)  const { v = mClip; return RT_OK;  }
  virtual rtError setClip(bool v) { mClip = v; return RT_OK; }

  bool mask()            const { return mMask;}
  rtError mask(bool& v)  const { v = mMask; return RT_OK;  }
  rtError setMask(bool v) { mMask = v; return RT_OK; }

  bool drawEnabled()            const { return mDraw;}
  rtError drawEnabled(bool& v)  const { v = mDraw; return RT_OK;  }
  rtError setDrawEnabled(bool v) { mDraw = v; return RT_OK; }

  bool hitTest()            const { return mHitTest;}
  rtError hitTest(bool& v)  const { v = mHitTest; return RT_OK;  }
  rtError setHitTest(bool v) { mHitTest = v; return RT_OK; }

  bool focus()            const { return mFocus;}
  rtError focus(bool& v)  const { v = mFocus; return RT_OK;  }
  rtError setFocus(bool v);
  
  rtError ready(rtObjectRef& v) const
  {
    v = mReady;
    return RT_OK;
  }

  rtError moveForward();
  rtError moveBackward();
  rtError moveToFront();
  rtError moveToBack();

  virtual void dispose(bool pumpJavascript);

  void drawInternal(bool maskPass=false);
  virtual void draw() {}
  virtual void sendPromise();
  virtual void createNewPromise();

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRef<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);

  void setFocusInternal(bool focus) { mFocus = focus; }
  
  rtError animateTo(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t options,
                     int32_t count, rtObjectRef promise);

  rtError animateToP2(rtObjectRef props, double duration, 
                      uint32_t interp, uint32_t options,
                      int32_t count, rtObjectRef& promise);

  rtError animateToObj(rtObjectRef props, double duration,
                      uint32_t interp, uint32_t options,
                      int32_t count, rtObjectRef& promise);

  void animateToInternal(const char* prop, double to, double duration,
                 pxInterp interp, pxConstantsAnimation::animationOptions options,
                 int32_t count, rtObjectRef promise, rtObjectRef animateObj);

  void cancelAnimation(const char* prop, bool fastforward = false, bool rewind = false);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  //rtError onReady(rtFunctionRef& /*f*/) const
  //{
    //rtLogError("onReady get not implemented\n");
    //return RT_OK;
  //}

  // TODO why does this have to be const
  //rtError setOnReady(const rtFunctionRef& f)
  //{
    //mEmit->setListener("onReady", f);
    //return RT_OK;
  //}

  virtual void update(double t);
  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage();

  // non-destructive applies transform on top of of provided matrix
  virtual void applyMatrix(pxMatrix4f& m)
  {
#if 0
    rtRef<pxTransform> t = new pxTransform;
    rtObjectRef i = new rtMapObject();
    i.set("x",0);
    i.set("y",0);
    i.set("cx",0);
    i.set("cy",0);
    i.set("sx",1);
    i.set("sy",1);
    i.set("r",0);
#ifdef ANIMATION_ROTATE_XYZ    
    i.set("rx",0);
    i.set("ry",0);
    i.set("rz",1);
#endif //ANIMATION_ROTATE_XYZ
    rtLogDebug("before initTransform\n");
    t->initTransform(i, 
      "x cx + y cy + translateXY "
#ifdef ANIMATION_ROTATE_XYZ
      "r rx ry rz rotateInDegreesXYZ "
#else
      "r rotateInDegrees rotateInDegreesXYZ "
#endif //ANIMATION_ROTATE_XYZ
      "sx sy scaleXY "
      "cx -1 * cy -1 * translateXY "
      );
    rtLogDebug("after initTransform\n");
    pxTransformData* d = t->newData();
    if (d)
    {
#if 1
      pxMatrix4f m;
      
      d->set("x",100);
      d->set("y",100);
      
      float v;
      d->get("x", v);
      d->get("cx", v);
      
      rtLogDebug("Before applyMatrix\n");    
      d->applyMatrix(m);
      rtLogDebug("After applyMatrix\n");    
     
#endif 
      t->deleteData(d);
      rtLogDebug("After deleteData\n");
    }
    else
      rtLogError("Could not allocate pxTransformData");
#endif
    if (!mUseMatrix)
    {
#if 1
      float dx = -(mpx * mw);
      float dy = -(mpy * mh);
      
      // translate based on xy rotate/scale based on cx, cy
      bool doTransToOrig = msx != 1.0 || msy != 1.0 || mr;
      if (doTransToOrig)
          m.translate(mx + mcx + dx, my + mcy + dy);
      else
          m.translate(mx + dx, my + dy);
      if (mr)
      {
        m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
        , mrx, mry, mrz
#endif // ANIMATION_ROTATE_XYZ        
        );
      }
      if (msx != 1.0 || msy != 1.0) m.scale(msx, msy);
      if (doTransToOrig)
          m.translate(-mcx, -mcy);
#else
      // translate/rotate/scale based on cx, cy
      m.translate(mx, my);
      if (mr) {
        m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
        , mrx, mry, mrz
#endif // ANIMATION_ROTATE_XYZ        
        );
      }
      if (msx != 1.0 || msy != 1.0) m.scale(msx, msy);
      m.translate(-mcx, -mcy);
#endif
    }
    else
    {
      m.multiply(mMatrix);
    }
  }

  static void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m) {
#if 1
    m.identity();
    
    while(o)
    {
      pxMatrix4f m2;
#if 0
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) {
        m2.rotateInDegrees(j->mr
#ifdef ANIMATION_ROTATE_XYZ        
        , j->mrx, j->mry, j->mrz
#endif //ANIMATION_ROTATE_XYZ        
        );
      }
      if (j->msx != 1.0 || j->msy != 1.0) m2.scale(j->msx, j->msy);  
      m2.translate(-j->mcx, -j->mcy);
#else
      o->applyMatrix(m2);
#endif
      // TODO adding a different operator here would eliminate the copy
      m2.multiply(m);
      m = m2;
      o = o->mParent;
    }
#else
    getMatrixFromSceneToObject(o, m);
    m.invert();
#endif
  }
  
  static void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m) {
#if 0
    m.identity();
    
    vector<rtRef<pxObject> > v;
    rtRef<pxObject> t = o;
    
    while(t) {
      v.push_back(t);
      t = t->mParent;
    }
    
    for(vector<rtRef<pxObject> >::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) 
    {
      rtRef<pxObject>& j = *it;;
      pxMatrix4f m2;
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) {
        m2.rotateInDegrees(j->mr
#ifdef ANIMATION_ROTATE_XYZ        
        , j->mrx, j->mry, j->mrz
#endif //ANIMATION_ROTATE_XYZ      
        );
      }
      
      if (j->msx != 1.0 || j->msy != 1.0) m2.scale(j->msx, j->msy);  
      m2.translate(-j->mcx, -j->mcy);
      m2.invert();
      m2.multiply(m);
      m = m2;
    }
#else
    getMatrixFromObjectToScene(o, m);
    m.invert();
#endif
  }
  
  static void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m) {
    pxMatrix4f t;
    getMatrixFromObjectToScene(from, t);
    getMatrixFromSceneToObject(to, m);
    
    m.multiply(t);
  }
  
  static void transformPointFromObjectToScene(pxObject* o, const pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromObjectToScene(o, m);
    to = m.multiply(from);
  }
  
  static void transformPointFromSceneToObject(pxObject* o, const pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromSceneToObject(o, m);
    to = m.multiply(from);
  }
  
  static void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject, pxVector4f& from, pxVector4f& to) {
    pxMatrix4f m;
    getMatrixFromObjectToObject(fromObject, toObject, m);
    to = m.multiply(from);
  }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }
  
  static pxObject* getObjectById(const char* id, pxObject* from)
  {
    if(id == NULL || from == NULL)
    {
      return NULL; // bad args
    }
    
    // TODO fix rtString empty check
    if (from->mId.cString() && !strcmp(id, from->mId.cString()))
      return from;
    
    for(std::vector<rtRef<pxObject> >::iterator it = from->mChildren.begin(); it != from->mChildren.end(); ++it)
    {
      pxObject* o = getObjectById(id, (*it).getPtr());
      if (o)
        return o;
    }
    
    return NULL;
  }

  rtError getObjectById(const char* id, rtObjectRef& o)
  {
    o = getObjectById(id, this);
    return RT_OK;
  }

  virtual bool onTextureReady();
  // !CLF: To Do: These names are terrible... find better ones!
  // These to functions are not exposed to javascript; they are for internal
  // determination of w/h for the pxObject. For instance, pxImage could be
  // from the texture or the pxImage values themselves.
  virtual float getOnscreenWidth() {  return mw; }
  virtual float getOnscreenHeight() { return mh;  }

  rtError m11(float& v) const { v = mMatrix.constData(0); return RT_OK; }
  rtError m12(float& v) const { v = mMatrix.constData(1); return RT_OK; }
  rtError m13(float& v) const { v = mMatrix.constData(2); return RT_OK; }
  rtError m14(float& v) const { v = mMatrix.constData(3); return RT_OK; }
  rtError m21(float& v) const { v = mMatrix.constData(4); return RT_OK; }
  rtError m22(float& v) const { v = mMatrix.constData(5); return RT_OK; }
  rtError m23(float& v) const { v = mMatrix.constData(6); return RT_OK; }
  rtError m24(float& v) const { v = mMatrix.constData(7); return RT_OK; }
  rtError m31(float& v) const { v = mMatrix.constData(8); return RT_OK; }
  rtError m32(float& v) const { v = mMatrix.constData(9); return RT_OK; }
  rtError m33(float& v) const { v = mMatrix.constData(10); return RT_OK; }
  rtError m34(float& v) const { v = mMatrix.constData(11); return RT_OK; }
  rtError m41(float& v) const { v = mMatrix.constData(12); return RT_OK; }
  rtError m42(float& v) const { v = mMatrix.constData(13); return RT_OK; }
  rtError m43(float& v) const { v = mMatrix.constData(14); return RT_OK; }
  rtError m44(float& v) const { v = mMatrix.constData(15); return RT_OK; }

  rtError setM11(const float& v) { cancelAnimation("m11",true); mMatrix.data()[0] = v; return RT_OK; }
  rtError setM12(const float& v) { cancelAnimation("m12",true); mMatrix.data()[1] = v; return RT_OK; }
  rtError setM13(const float& v) { cancelAnimation("m13",true); mMatrix.data()[2] = v; return RT_OK; }
  rtError setM14(const float& v) { cancelAnimation("m14",true); mMatrix.data()[3] = v; return RT_OK; }
  rtError setM21(const float& v) { cancelAnimation("m21",true); mMatrix.data()[4] = v; return RT_OK; }
  rtError setM22(const float& v) { cancelAnimation("m22",true); mMatrix.data()[5] = v; return RT_OK; }
  rtError setM23(const float& v) { cancelAnimation("m23",true); mMatrix.data()[6] = v; return RT_OK; }
  rtError setM24(const float& v) { cancelAnimation("m24",true); mMatrix.data()[7] = v; return RT_OK; }
  rtError setM31(const float& v) { cancelAnimation("m31",true); mMatrix.data()[8] = v; return RT_OK; }
  rtError setM32(const float& v) { cancelAnimation("m32",true); mMatrix.data()[9] = v; return RT_OK; }
  rtError setM33(const float& v) { cancelAnimation("m33",true); mMatrix.data()[10] = v; return RT_OK; }
  rtError setM34(const float& v) { cancelAnimation("m34",true); mMatrix.data()[11] = v; return RT_OK; }
  rtError setM41(const float& v) { cancelAnimation("m41",true); mMatrix.data()[12] = v; return RT_OK; }
  rtError setM42(const float& v) { cancelAnimation("m42",true); mMatrix.data()[13] = v; return RT_OK; }
  rtError setM43(const float& v) { cancelAnimation("m43",true); mMatrix.data()[14] = v; return RT_OK; }
  rtError setM44(const float& v) { cancelAnimation("m44",true); mMatrix.data()[15] = v; return RT_OK; }

  rtError useMatrix(bool& v) const { v = mUseMatrix; return RT_OK; }
  rtError setUseMatrix(const bool& v) { mUseMatrix = v; return RT_OK; }

  void repaint() { mRepaint = true; }

  rtError releaseResources()
  {
     dispose(true);
     return RT_OK;
  }

  pxScene2d* getScene() { return mScene; }
  void createSnapshot(pxContextFramebufferRef& fbo, bool separateContext=false, bool antiAliasing=false);

public:
  rtEmitRef mEmit;

protected:
  // TODO getting freaking huge... 
//  rtRef<pxObject> mParent;
  pxObject* mParent;
  std::vector<rtRef<pxObject> > mChildren;
//  vector<animation> mAnimations;
  float mpx, mpy, mcx, mcy, mx, my, ma, mr;
#ifdef ANIMATION_ROTATE_XYZ
  float mrx, mry, mrz;
#endif // ANIMATION_ROTATE_XYZ
  float msx, msy, mw, mh;
  bool mInteractive;
  pxContextFramebufferRef mSnapshotRef;
  bool mPainting;
  bool mClip;
  bool mMask;
  bool mDraw;
  bool mHitTest;
  rtObjectRef mReady;
  bool mFocus;
  pxContextFramebufferRef mClipSnapshotRef;
  bool mCancelInSet;
  rtString mId;
  pxMatrix4f mMatrix;
  bool mUseMatrix;
  bool mRepaint;
  #ifdef PX_DIRTY_RECTANGLES
  bool mIsDirty;
  pxMatrix4f mRenderMatrix;
  pxRect mScreenCoordinates;
  pxRect mDirtyRect;
  #endif //PX_DIRTY_RECTANGLES

  void createSnapshotOfChildren();
  void clearSnapshot(pxContextFramebufferRef fbo);
  #ifdef PX_DIRTY_RECTANGLES
  void setDirtyRect(pxRect* r);
  pxRect getBoundingRectInScreenCoordinates();
  pxRect convertToScreenCoordinates(pxRect* r);
  #endif //PX_DIRTY_RECTANGLES

  pxScene2d* mScene;

  std::vector<animation> mAnimations;  
  pxContextFramebufferRef mDrawableSnapshotForMask;
  pxContextFramebufferRef mMaskSnapshot;
  bool mIsDisposed;
  bool mSceneSuspended;

 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
  void repaintParents();
};

class pxRoot: public pxObject
{
  rtDeclareObject(pxRoot, pxObject);
public:
  pxRoot(pxScene2d* scene): pxObject(scene) {}
};

class pxViewContainer: public pxObject, public pxIViewContainer
{
public:
  rtDeclareObject(pxViewContainer, pxObject);
//  rtProperty(uri, uri, setURI, rtString);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtMethod1ArgAndNoReturn("onMouseDown", onMouseDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseUp", onMouseUp, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseMove", onMouseMove, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseEnter", onMouseEnter, rtObjectRef);
  rtMethod1ArgAndNoReturn("onMouseLeave", onMouseLeave, rtObjectRef);
  rtMethod1ArgAndNoReturn("onFocus", onFocus, rtObjectRef);
  rtMethod1ArgAndNoReturn("onBlur", onBlur, rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyDown", onKeyDown, rtObjectRef);
  rtMethod1ArgAndNoReturn("onKeyUp", onKeyUp, rtObjectRef);
  rtMethod1ArgAndNoReturn("onChar", onChar, rtObjectRef);

  pxViewContainer(pxScene2d* scene):pxObject(scene)
  {
    addListener("onMouseDown", get<rtFunctionRef>("onMouseDown"));
    addListener("onMouseUp", get<rtFunctionRef>("onMouseUp"));
    addListener("onMouseMove", get<rtFunctionRef>("onMouseMove"));
    addListener("onMouseEnter", get<rtFunctionRef>("onMouseEnter"));
    addListener("onMouseLeave", get<rtFunctionRef>("onMouseLeave"));
    addListener("onFocus", get<rtFunctionRef>("onFocus"));
    addListener("onBlur", get<rtFunctionRef>("onBlur"));
    addListener("onKeyDown", get<rtFunctionRef>("onKeyDown"));
    addListener("onKeyUp", get<rtFunctionRef>("onKeyUp"));
    addListener("onChar", get<rtFunctionRef>("onChar"));
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

  virtual void update(double t)
  {
    if (mView)
      mView->onUpdate(t);
    pxObject::update(t);
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

#ifdef ENABLE_PERMISSIONS_CHECK
  rtError permissions(rtObjectRef& v) const;
  rtError setPermissions(const rtObjectRef& v);
#endif
  rtError cors(rtObjectRef& v) const;

//  rtError makeReady(bool ready);  // DEPRECATED ?

  // in the case of pxSceneContainer, the makeReady should be the  
  // catalyst for ready to fire, so override sendPromise and 
  // createNewPromise to prevent firing from update() 
  virtual void sendPromise() { /*rtLogDebug("pxSceneContainer ignoring sendPromise\n");*/ }
  virtual void createNewPromise(){ rtLogDebug("pxSceneContainer ignoring createNewPromise\n"); }

  virtual void* getInterface(const char* name);
  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage();
  
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
    rtLogInfo(__FUNCTION__);
    rtLogDebug("~pxScriptView for mUrl=%s\n",mUrl.cString());
    // Clear out these references since the script context
    // can outlive this view
#ifdef ENABLE_RT_NODE
    if(mCtx)
    {
      mGetScene->clearContext();
      mMakeReady->clearContext();
      mGetContextID->clearContext();

      // TODO Given that the context is being cleared we likely don't need to zero these out
      mCtx->add("getScene", 0);
      mCtx->add("makeReady", 0);
      mCtx->add("getContextID", 0);
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
  
protected:

  static rtError printFunc(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx);

  static rtError getScene(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx);
  static rtError makeReady(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* ctx);

  static rtError getContextID(int /*numArgs*/, const rtValue* /*args*/, rtValue* result, void* /*ctx*/);

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
  rtRef<rtFunctionCallback> mPrintFunc;
  rtRef<rtFunctionCallback> mGetScene;
  rtRef<rtFunctionCallback> mMakeReady;
  rtRef<rtFunctionCallback> mGetContextID;

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
  
  rtMethod1ArgAndReturn("screenshot", screenshot, rtString, rtString);

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
  rtReadOnlyProperty(alignVertical,alignVertical,rtObjectRef);
  rtReadOnlyProperty(alignHorizontal,alignHorizontal,rtObjectRef);
  rtReadOnlyProperty(truncation,truncation,rtObjectRef);

  rtReadOnlyProperty(origin, origin, rtString);

  rtMethodNoArgAndNoReturn("dispose",dispose);

  rtMethod1ArgAndNoReturn("addServiceProvider", addServiceProvider, rtFunctionRef);
  rtMethod1ArgAndNoReturn("removeServiceProvider", removeServiceProvider, rtFunctionRef);

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

  rtError showDirtyRect(bool& v) const;
  rtError setShowDirtyRect(bool v);

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
  rtError createScene(rtObjectRef p,rtObjectRef& o);
  rtError createExternal(rtObjectRef p, rtObjectRef& o);
  rtError createWayland(rtObjectRef p, rtObjectRef& o);

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
  
  rtError animation(rtObjectRef& v) const {v = CONSTANTS.animationConstants; return RT_OK;}
  rtError stretch(rtObjectRef& v)   const {v = CONSTANTS.stretchConstants;   return RT_OK;}
  rtError maskOp(rtObjectRef& v)    const {v = CONSTANTS.maskOpConstants;    return RT_OK;}  
  
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
  rtError origin(rtString& v) const { v = mOrigin; return RT_OK; }

  void setMouseEntered(rtRef<pxObject> o);//setMouseEntered(pxObject* o);

  // The following methods are delegated to the view
  virtual void onSize(int32_t w, int32_t h);

  virtual bool onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseUp(int32_t x, int32_t y, uint32_t flags);
  virtual bool onMouseEnter();
  virtual bool onMouseLeave();
  virtual bool onMouseMove(int32_t x, int32_t y);

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
  void transformPointFromObjectToScene(pxObject* o, const pxPoint2f& from, 
				       pxPoint2f& to);
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
      mArchive = a;
    return e;
  }

  void innerpxObjectDisposed(rtObjectRef ref);

  // Note: Only type currently supported is "image/png;base64"
  rtError screenshot(rtString type, rtString& pngData);
  rtError clipboardGet(rtString type, rtString& retString);
  rtError clipboardSet(rtString type, rtString clipString);
  rtError getService(rtString name, rtObjectRef& returnObject);
  rtError getService(const char* name, const rtObjectRef& ctx, rtObjectRef& service);
  rtError getAvailableApplications(rtString& availableApplications);
  rtObjectRef getArchive()
  {
    return mArchive;
  }

private:
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
  bool mShowDirtyRectangle;
  #ifdef USE_SCENE_POINTER
  pxTextureRef mNullTexture;
  rtObjectRef mPointerResource;
  pxTextureRef mPointerTexture;
  int32_t mPointerX;
  int32_t mPointerY;
  int32_t mPointerW;
  int32_t mPointerH;
  int32_t mPointerHotSpotX;
  int32_t mPointerHotSpotY;
  #endif
  bool mPointerHidden;
  std::vector<rtObjectRef> mInnerpxObjects;
  rtFunctionRef mCustomAnimator;
  rtString mOrigin;
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
  #ifdef PX_DIRTY_RECTANGLES
  pxRect mDirtyRect;
  pxRect mLastFrameDirtyRect;
  #endif //PX_DIRTY_RECTANGLES
  bool mDirty;
  testView* mTestView;
  bool mDisposed;
  std::vector<rtFunctionRef> mServiceProviders;
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
