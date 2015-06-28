// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.h

#ifndef PX_SCENE2D_H
#define PX_SCENE2D_H

#include <stdio.h>

#include <vector>
#include <list>
using namespace std;

#ifndef finline
#define finline __attribute__((always_inline))
#endif

#include "rtRefT.h"
#include "rtString.h"

// TODO rtDefs vs rtCore.h
#include "rtDefs.h"
#include "rtCore.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"

#include "pxCore.h"
#include "pxIView.h"

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
#include "pxTextureCacheObject.h"

//#include "pxTransform.h"


// Interpolator constants
#define PX_LINEAR_         0
#define PX_EXP1_           1 
#define PX_EXP2_           2
#define PX_EXP3_           3
#define PX_STOP_           4
#define PX_INQUAD_         5
#define PX_INCUBIC_        6
#define PX_INBACK_         7
#define PX_EASEINELASTIC_  8
#define PX_EASEOUTELASTIC_ 9
#define PX_EASEOUTBOUNCE_  10

#define PX_NONE_           0
#define PX_STRETCH_        1
#define PX_REPEAT_         2


#if 0
typedef rtError (*objectFactory)(void* context, const char* t, rtObjectRef& o);
void registerObjectFactory(objectFactory f, void* context);
rtError createObject2(const char* t, rtObjectRef& o);
#endif

typedef double (*pxInterp)(double i);
typedef void (*pxAnimationEnded)(void* ctx);

double pxInterpLinear(double i);

enum pxAnimationType {PX_END = 0, PX_SEESAW, PX_LOOP};

struct pxPoint2f {
  pxPoint2f() {}
  pxPoint2f(float _x, float _y) { x = _x; y = _y; } 
  float x, y;
};

struct pxAnimationTarget {
  char* prop;
  float to;
};

typedef double (*pxInterp)(double i);
typedef void (*pxAnimationEnded)(void* ctx);

double pxInterpLinear(double i);

struct animation {
  bool cancelled;
  rtString prop;
  float from;
  float to;
  bool flip;
  double start;
  double duration;
  pxAnimationType at;
  pxInterp interp;
  rtFunctionRef ended;
  rtObjectRef promise;
};

class pxFileDownloadRequest;


enum rtPromiseState {PENDING,FULFILLED,REJECTED};

struct thenData
{
  rtFunctionRef mResolve;
  rtFunctionRef mReject;
  rtObjectRef mNextPromise;
};

class rtPromise: public rtObject
{
public:
  rtDeclareObject(rtPromise,rtObject);

  rtMethod2ArgAndReturn("then",then,rtFunctionRef,rtFunctionRef,rtObjectRef);

  rtMethod1ArgAndNoReturn("resolve",resolve,rtValue);
  rtMethod1ArgAndNoReturn("reject",reject,rtValue);

  rtPromise():mState(PENDING) {}

  rtError then(rtFunctionRef resolve, rtFunctionRef reject, 
               rtObjectRef& newPromise)
  {
    if (mState == PENDING)
    {
      thenData d;
      d.mResolve = resolve;
      d.mReject = reject;
      d.mNextPromise = new rtPromise;
      mThenData.push_back(d);

      newPromise = d.mNextPromise;
    }
    else if (mState == FULFILLED)
    {
      if (resolve)
      {
        resolve.send(mValue);
        newPromise = new rtPromise;
        newPromise.send("resolve",mValue);
      }
    }
    else if (mState == REJECTED)
    {
      if (reject)
      {
        reject.send(mValue);
        newPromise = new rtPromise;
        newPromise.send("reject",mValue);
      }
    }
    else
      rtLogError("Invalid rtPromise state");
    return RT_OK;
  }

  rtError resolve(const rtValue& v)
  {
    mState = FULFILLED;
    mValue = v;
    for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
    {
      if (it->mResolve)
      {
        it->mResolve.send(mValue);
        it->mNextPromise.send("resolve",mValue);
      }
    }
    mThenData.clear();
    return RT_OK;
  }

  rtError reject(const rtValue& v)
  {
    mState = REJECTED;
    mValue = v;
    for (vector<thenData>::iterator it = mThenData.begin();
         it != mThenData.end(); ++it)
    {
      if (it->mReject)
      {
        it->mReject.send(mValue);
        it->mNextPromise.send("reject",mValue);
      }
    }
    mThenData.clear();
    return RT_OK;
  }

private:
  rtPromiseState mState;
  vector<thenData> mThenData;
  rtValue mValue;
};

class pxObject;

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
  rtProperty(cx, cx, setCX, float);
  rtProperty(cy, cy, setCY, float);
  rtProperty(sx, sx, setSX, float);
  rtProperty(sy, sy, setSY, float);
  rtProperty(a, a, setA, float);
  rtProperty(r, r, setR, float);
  rtProperty(rx, rx, setRX, float);
  rtProperty(ry, ry, setRY, float);
  rtProperty(rz, rz, setRZ, float);
  rtProperty(id, id, setId, rtString);
  rtProperty(interactive, interactive, setInteractive, bool);
  rtProperty(painting, painting, setPainting, bool);
  rtProperty(clip, clip, setClip, bool);
  rtProperty(mask, mask, setMask, rtString);
  rtProperty(drawAsMask, drawAsMask, setDrawAsMask, bool);
  rtProperty(draw, drawEnabled, setDrawEnabled, bool);
  rtProperty(drawAsHitTest, drawAsHitTest, setDrawAsHitTest, bool);
  rtReadOnlyProperty(ready, ready, rtObjectRef);

  rtReadOnlyProperty(numChildren, numChildren, int32_t);
  rtMethod1ArgAndReturn("getChild", getChild, int32_t, rtObjectRef);
  rtReadOnlyProperty(children, children, rtObjectRef);

  rtMethodNoArgAndNoReturn("remove", remove);
  rtMethodNoArgAndNoReturn("removeAll", removeAll);
  rtMethodNoArgAndNoReturn("moveToFront", moveToFront);

  #if 0
  //TODO - remove
  rtMethod5ArgAndNoReturn("animateToF", animateToF2, rtObjectRef, double,
                          uint32_t, uint32_t, rtFunctionRef);
  #endif

  rtMethod4ArgAndReturn("animateTo", animateToP2, rtObjectRef, double,
                        uint32_t, uint32_t, rtObjectRef);

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
  rtProperty(onReady, onReady, setOnReady, rtFunctionRef);

  rtReadOnlyProperty(emit, emit, rtFunctionRef);
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

  pxObject(): rtObject(), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0),
    mrx(0), mry(0), mrz(1.0), msx(1), msy(1), mw(0), mh(0),
    mInteractive(true),
    mTextureRef(), mPainting(true), mClip(false), mMaskUrl(), mDrawAsMask(false), mDraw(true), mDrawAsHitTest(true), mReady(), mMaskTextureRef(),
    mMaskTextureCacheObject(),mClipTextureRef(),mCancelInSet(true),mUseMatrix(false)
  {
    mReady = new rtPromise;
    mEmit = new rtEmit;
  }

  virtual ~pxObject() { /*printf("pxObject destroyed\n");*/ deleteSnapshot(mTextureRef); deleteSnapshot(mClipTextureRef);}

  // TODO missing conversions in rtValue between uint32_t and int32_t
  uint32_t numChildren() const { return mChildren.size(); }
  rtError numChildren(int32_t& v) const 
  {
    v = mChildren.size();
    return RT_OK;
  }

  rtError getChild(int32_t i, rtObjectRef& r) const 
  {
    r = mChildren[i];
    return RT_OK;
  }

  rtError children(rtObjectRef& v) const;

  // TODO clean this up
  void setParent(rtRefT<pxObject>& parent);
  pxObject* parent() const
  {
    return mParent;
  }

  rtError parent(rtObjectRef& v) const 
  {
    v = mParent.getPtr();
    return RT_OK;
  }

  rtError setParent(rtObjectRef parent) 
  {
    void* p = parent.get<voidPtr>("_pxObject");
    if (p) {
      rtRefT<pxObject> p2 = (pxObject*)p;
      setParent(p2);
    }
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
  rtError setW(float v)       { cancelAnimation("w"); mw = v; return RT_OK;   }
  float h()             const { return mh; }
  rtError h(float& v)   const { v = mh; return RT_OK;   }
  rtError setH(float v)       { cancelAnimation("h"); mh = v; return RT_OK;   }
  float cx()            const { return mcx;}
  rtError cx(float& v)  const { v = mcx; return RT_OK;  }
  rtError setCX(float v)      { cancelAnimation("cx"); mcx = v; return RT_OK;  }
  float cy()            const { return mcy;}
  rtError cy(float& v)  const { v = mcy; return RT_OK;  }
  rtError setCY(float v)      { cancelAnimation("cy"); mcy = v; return RT_OK;  }
  float sx()            const { return msx;}
  rtError sx(float& v)  const { v = msx; return RT_OK;  }
  rtError setSX(float v)      { cancelAnimation("sx"); msx = v; return RT_OK;  }
  float sy()            const { return msy;}
  rtError sy(float& v)  const { v = msx; return RT_OK;  } 
  rtError setSY(float v)      { cancelAnimation("sy"); msy = v; return RT_OK;  }
  float a()             const { return ma; }
  rtError a(float& v)   const { v = ma; return RT_OK;   }
  rtError setA(float v)       { cancelAnimation("a"); ma = v; return RT_OK;   }
  float r()             const { return mr; }
  rtError r(float& v)   const { v = mr; return RT_OK;   }
  rtError setR(float v)       { cancelAnimation("r"); mr = v; return RT_OK;   }
  float rx()            const { return mrx;}
  rtError rx(float& v)  const { v = mrx; return RT_OK;  }
  rtError setRX(float v)      { cancelAnimation("rx"); mrx = v; return RT_OK;  }
  float ry()            const { return mry;}
  rtError ry(float& v)  const { v = mry; return RT_OK;  }
  rtError setRY(float v)      { cancelAnimation("ry"); mry = v; return RT_OK;  }
  float rz()            const { return mrz;}
  rtError rz(float& v)  const { v = mrz; return RT_OK;  }
  rtError setRZ(float v)      { cancelAnimation("rz"); mrz = v; return RT_OK;  }
  bool painting()            const { return mPainting;}
  rtError painting(bool& v)  const { v = mPainting; return RT_OK;  }
  rtError setPainting(bool v)
  { 
      mPainting = v; 
      if (!mPainting)
      {
          mTextureRef = createSnapshot(mTextureRef);
          mTextureRef->enablePremultipliedAlpha(true);
      }
      else
      {
          deleteSnapshot(mTextureRef);
      }
      return RT_OK;
  }

  bool clip()            const { return mClip;}
  rtError clip(bool& v)  const { v = mClip; return RT_OK;  }
  rtError setClip(bool v) { mClip = v; return RT_OK; }
  
  rtString mask()            const { return mMaskUrl;}
  rtError mask(rtString& v)  const { v = mMaskUrl; return RT_OK;  }
  rtError setMask(rtString v) { mMaskUrl = v; createMask(); return RT_OK; }

  bool drawAsMask()            const { return mDrawAsMask;}
  rtError drawAsMask(bool& v)  const { v = mDrawAsMask; return RT_OK;  }
  rtError setDrawAsMask(bool v) { mDrawAsMask = v; return RT_OK; }

  bool drawEnabled()            const { return mDraw;}
  rtError drawEnabled(bool& v)  const { v = mDraw; return RT_OK;  }
  rtError setDrawEnabled(bool v) { mDraw = v; return RT_OK; }

  bool drawAsHitTest()            const { return mDrawAsHitTest;}
  rtError drawAsHitTest(bool& v)  const { v = mDrawAsHitTest; return RT_OK;  }
  rtError setDrawAsHitTest(bool v) { mDrawAsHitTest = v; return RT_OK; }

  rtError ready(rtObjectRef& v) const
  {
    v = mReady;
    return RT_OK;
  }

  rtError moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();

  void drawInternal(pxMatrix4f m, float parentAlpha, bool maskPass=false);
  virtual void draw() {}

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);

  #if 0
  //TODO - remove
  rtError animateToF(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t animationType, 
                     rtFunctionRef onEnd);
  #endif

  rtError animateTo(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t animationType, 
                     rtObjectRef promise);

  #if 0
  //TODO - REMOVE
  rtError animateToF2(rtObjectRef props, double duration,
                     uint32_t interp, uint32_t animationType,
                     rtFunctionRef onEnd)
  {
    if (!props) return RT_FAIL;
    rtObjectRef keys = props.get<rtObjectRef>("allKeys");
    if (keys)
    {
      uint32_t len = keys.get<uint32_t>("length");
      for (uint32_t i = 0; i < len; i++)
      {
        rtString key = keys.get<rtString>(i);
        animateToF(key, props.get<float>(key), duration, interp, animationType,
                  (i==0)?onEnd:rtFunctionRef());
      }
    }
    return RT_OK;
  }
  #endif

  rtError animateToP2(rtObjectRef props, double duration, 
                      uint32_t interp, uint32_t animationType, 
                      rtObjectRef& promise);

  #if 0
  //TODO - remove
  void animateToF(const char* prop, double to, double duration,
		 pxInterp interp, pxAnimationType at, 
                 rtFunctionRef onEnd);
   #endif

  void animateTo(const char* prop, double to, double duration,
		 pxInterp interp, pxAnimationType at, 
                 rtObjectRef promise);

  #if 0
  //TODO - remove
  void animateToF(const char* prop, double to, double duration,
		 pxInterp interp=0, pxAnimationType at=PX_END)
  {
    animateToF(prop, to, duration, interp, at, rtFunctionRef());
  }
  #endif

  void cancelAnimation(const char* prop, bool fastforward = false);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError onReady(rtFunctionRef& /*f*/) const
  {
    rtLogError("onReady get not implemented\n");
    return RT_OK;
  }

  // TODO why does this have to be const
  rtError setOnReady(const rtFunctionRef& f)
  {
    mEmit->setListener("onReady", f);
    return RT_OK;
  }

  virtual void update(double t);

  // non-destructive applies transform on top of of provided matrix
  virtual void applyMatrix(pxMatrix4f& m)
  {
#if 0
    rtRefT<pxTransform> t = new pxTransform;
    rtObjectRef i = new rtMapObject();
    i.set("x",0);
    i.set("y",0);
    i.set("cx",0);
    i.set("cy",0);
    i.set("sx",1);
    i.set("sy",1);
    i.set("r",0);
    i.set("rx",0);
    i.set("ry",0);
    i.set("rz",1);

    printf("before initTransform\n");
    t->initTransform(i, 
      "x cx + y cy + translateXY "
      "r rx ry rz rotateInDegreesXYZ "
      "sx sy scaleXY "
      "cx -1 * cy -1 * translateXY "
      );
    printf("after initTransform\n");
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
      
      printf("Before applyMatrix\n");    
      d->applyMatrix(m);
      printf("After applyMatrix\n");    
     
#endif 
      t->deleteData(d);
      printf("After deleteData\n");
    }
    else
      rtLogError("Could not allocate pxTransformData");
#endif
    if (!mUseMatrix)
    {
#if 1
      // translate based on xy rotate/scale based on cx, cy
      m.translate(mx+mcx, my+mcy);
      if (mr) m.rotateInDegrees(mr, mrx, mry, mrz);
      if (msx != 1.0 || msy != 1.0) m.scale(msx, msy);  
      m.translate(-mcx, -mcy);    
#else
      // translate/rotate/scale based on cx, cy
      m.translate(mx, my);
      if (mr) m.rotateInDegrees(mr, mrx, mry, mrz);
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
//      rtRefT<pxObject> j = o;
      pxMatrix4f m2;
#if 0
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) m2.rotateInDegrees(j->mr, j->mrx, j->mry, j->mrz);
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
    
    vector<rtRefT<pxObject> > v;
    rtRefT<pxObject> t = o;
    
    while(t) {
      v.push_back(t);
      t = t->mParent;
    }
    
    for(vector<rtRefT<pxObject> >::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) 
    {
      rtRefT<pxObject>& j = *it;;
      pxMatrix4f m2;
      m2.translate(j->mx+j->mcx, j->my+j->mcy);
      if (j->mr) m2.rotateInDegrees(j->mr, j->mrx, j->mry, j->mrz);
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
    // TODO fix rtString empty check
    if (from->mId.cString() && !strcmp(id, from->mId.cString()))
      return from;
    
    for(vector<rtRefT<pxObject> >::iterator it = from->mChildren.begin(); it != from->mChildren.end(); ++it)
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

  virtual bool onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status);

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

  rtError setM11(const float& v) { mMatrix.data()[0] = v; return RT_OK; }
  rtError setM12(const float& v) { mMatrix.data()[1] = v; return RT_OK; }
  rtError setM13(const float& v) { mMatrix.data()[2] = v; return RT_OK; }
  rtError setM14(const float& v) { mMatrix.data()[3] = v; return RT_OK; }
  rtError setM21(const float& v) { mMatrix.data()[4] = v; return RT_OK; }
  rtError setM22(const float& v) { mMatrix.data()[5] = v; return RT_OK; }
  rtError setM23(const float& v) { mMatrix.data()[6] = v; return RT_OK; }
  rtError setM24(const float& v) { mMatrix.data()[7] = v; return RT_OK; }
  rtError setM31(const float& v) { mMatrix.data()[8] = v; return RT_OK; }
  rtError setM32(const float& v) { mMatrix.data()[9] = v; return RT_OK; }
  rtError setM33(const float& v) { mMatrix.data()[10] = v; return RT_OK; }
  rtError setM34(const float& v) { mMatrix.data()[11] = v; return RT_OK; }
  rtError setM41(const float& v) { mMatrix.data()[12] = v; return RT_OK; }
  rtError setM42(const float& v) { mMatrix.data()[13] = v; return RT_OK; }
  rtError setM43(const float& v) { mMatrix.data()[14] = v; return RT_OK; }
  rtError setM44(const float& v) { mMatrix.data()[15] = v; return RT_OK; }

  rtError useMatrix(bool& v) const { v = mUseMatrix; return RT_OK; }
  rtError setUseMatrix(const bool& v) { mUseMatrix = v; return RT_OK; }

public:
  rtEmitRef mEmit;

protected:
  // TODO getting freaking huge... 
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
  vector<animation> mAnimations;
  float mcx, mcy, mx, my, ma, mr, mrx, mry, mrz, msx, msy, mw, mh;
  bool mInteractive;
  pxTextureRef mTextureRef;
  bool mPainting;
  bool mClip;
  rtString mMaskUrl;
  bool mDrawAsMask;
  bool mDraw;
  bool mDrawAsHitTest;
  rtObjectRef mReady;
  pxTextureRef mMaskTextureRef;
  pxTextureCacheObject mMaskTextureCacheObject;
  pxTextureRef mClipTextureRef;
  bool mCancelInSet;
  rtString mId;
  pxMatrix4f mMatrix;
  bool mUseMatrix;

  pxTextureRef createSnapshot(pxTextureRef texture);
  void createSnapshotOfChildren(pxTextureRef drawableTexture, pxTextureRef maskTexture);
  void deleteSnapshot(pxTextureRef texture);
  void createMask();
  void deleteMask();

 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
};


class testView: public pxIView
{
public:
  
testView(): mContainer(NULL),mw(0),mh(0),mEntered(false),mMouseX(0), mMouseY(0) {}
  virtual ~testView() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  virtual void RT_STDCALL onSize(int32_t w, int32_t h)
  {
    rtLogInfo("testView::onSize(%d, %d)", w, h);
    mw = w;
    mh = h;
  }

  virtual void RT_STDCALL onMouseDown(int32_t x, int32_t y, uint32_t flags)
  {
    rtLogInfo("testView::onMouseDown(%d, %d, %u)", x, y, flags);
  }

  virtual void RT_STDCALL onMouseUp(int32_t x, int32_t y, uint32_t flags)
  {
    rtLogInfo("testView::onMouseUp(%d, %d, %u)", x, y, flags);
  }

  virtual void RT_STDCALL onMouseMove(int32_t x, int32_t y)
  {
    rtLogInfo("testView::onMouseMove(%d, %d)", x, y);
    mMouseX = x;
    mMouseY = y;
  }

  virtual void RT_STDCALL onMouseEnter()
  {
    rtLogInfo("testView::onMouseEnter()");
    mEntered = true;
  }

  virtual void RT_STDCALL onMouseLeave()
  {
    rtLogInfo("testView::onMouseLeave()");
    mEntered = false;
  }

  virtual void RT_STDCALL onFocus()
  {
    rtLogInfo("testView::onFocus()");
  }
  virtual void RT_STDCALL onBlur()
  {
    rtLogInfo("testView::onBlur()");

  }
  virtual void RT_STDCALL onKeyDown(uint32_t keycode, uint32_t flags)
  {
    rtLogInfo("testView::onKeyDown(%u, %u)", keycode, flags);
  }

  virtual void RT_STDCALL onKeyUp(uint32_t keycode, uint32_t flags)
  {
    rtLogInfo("testView::onKeyUp(%u, %u)", keycode, flags);
  }

  virtual void RT_STDCALL onChar(uint32_t codepoint)
  {
    rtLogInfo("testView::onChar(%u)", codepoint);
  }

  virtual void RT_STDCALL setViewContainer(pxIViewContainer* l)
  {
    rtLogInfo("testView::setViewContainer(%p)", l);
  }

  virtual void RT_STDCALL onDraw();


#if 0
  virtual rtError RT_STDCALL setURI(const char* s) = 0;
#endif

private:
  pxIViewContainer *mContainer;
  rtAtomic mRefCount;
  float mw, mh;
  bool mEntered;
  int32_t mMouseX, mMouseY;
};

class pxViewContainer: public pxObject
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

  pxViewContainer()
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

  virtual ~pxViewContainer() {}

  rtError setView(pxIView* v)
  {
    mView = v;
    if (mView)
      mView->onSize(mw,mh);
    return RT_OK;
  }

#if 0
  rtError uri(rtString& v) const { v = mURI; return RT_OK; }
  rtError setURI(rtString v) { mURI = v; return RT_OK; }
#endif

  rtError w(float& v) const { v = mw; return RT_OK; }
  rtError setW(float v) 
  { 
    mw = v; 
    if (mView)
      mView->onSize(mw,mh); 
    return RT_OK; 
  }
  
  rtError h(float& v) const { v = mh; return RT_OK; }
  rtError setH(float v) 
  { 
    mh = v; 
    if (mView)
      mView->onSize(mw,mh); 
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
      mView->onMouseDown(x,y,flags);
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
      mView->onMouseUp(x,y,flags);
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
      mView->onMouseMove(x,y);
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
  rtError onKeyDown(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t keyCode = o.get<uint32_t>("keyCode");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onKeyDown(keyCode, flags);
    }
    return RT_OK;
  }

  rtError onKeyUp(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t keyCode = o.get<uint32_t>("keyCode");
      uint32_t flags = o.get<uint32_t>("flags");
      mView->onKeyUp(keyCode, flags);
    }
    return RT_OK;
  }

  rtError onChar(rtObjectRef o)
  {
    if (mView)
    {
      uint32_t codePoint = o.get<uint32_t>("charCode");
      mView->onChar(codePoint);
    }
    return RT_OK;
  }

  virtual void draw() 
  {
    if (mView)
      mView->onDraw();
  }

  

protected:
  pxViewRef mView;
  rtString mURI;
};


class pxSceneContainer: public pxViewContainer
{
public:
  rtDeclareObject(pxSceneContainer, pxViewContainer);
  rtProperty(url, uri, setURI, rtString);
  
  rtError uri(rtString& v) const { v = mURI; return RT_OK; }
  rtError setURI(rtString v);
private:
  rtString mURI;
};


typedef rtRefT<pxObject> pxObjectRef;

// Small helper class that vends the children of a pxObject as a collection
class pxObjectChildren: public rtObject {
public:
  pxObjectChildren(pxObject* o)
  {
    mObject = o;
  }

  virtual rtError Get(const char* name, rtValue* value)
  {
    if (!value) return RT_FAIL;
    if (!strcmp(name, "length"))
    {
      value->setUInt32(mObject->numChildren());
      return RT_OK;
    }
    else
      return RT_PROP_NOT_FOUND;
  }

  virtual rtError Get(uint32_t i, rtValue* value)
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

  virtual rtError Set(const char* /*name*/, const rtValue* /*value*/)
  {
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(uint32_t /*i*/, const rtValue* /*value*/)
  {
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

private:
  rtRefT<pxObject> mObject;
};

class pxScene2d: public rtObject, public pxIView {
public:
  rtDeclareObject(pxScene2d, rtObject);
  rtProperty(onScene, onScene, setOnScene, rtFunctionRef);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtMethod1ArgAndReturn("create", create, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createRectangle", createRectangle, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createImage", createImage, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createImage9", createImage9, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createText", createText, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createText2", createText2, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createScene", createScene, rtObjectRef, rtObjectRef);
  rtMethod1ArgAndReturn("createExternal", createExternal, rtObjectRef,
                        rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  // TODO make this a property
  rtMethod1ArgAndNoReturn("setFocus", setFocus, rtObjectRef);
  
  rtMethodNoArgAndNoReturn("stopPropagation",stopPropagation);

  rtProperty(ctx, ctx, setCtx, rtValue);
  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  rtConstantProperty(PX_LINEAR, PX_LINEAR_, uint32_t);
  rtConstantProperty(PX_EXP1, PX_EXP1_, uint32_t);
  rtConstantProperty(PX_EXP2, PX_EXP2_, uint32_t);
  rtConstantProperty(PX_EXP3, PX_EXP3_, uint32_t);
  rtConstantProperty(PX_STOP, PX_STOP_, uint32_t);
  rtConstantProperty(PX_INQUAD, PX_INQUAD_, uint32_t);
  rtConstantProperty(PX_INCUBIC, PX_INCUBIC_, uint32_t);
  rtConstantProperty(PX_INBACK, PX_INBACK_, uint32_t);
  rtConstantProperty(PX_EASEINELASTIC, PX_EASEINELASTIC_, uint32_t);
  rtConstantProperty(PX_EASEOUTELASTIC, PX_EASEOUTELASTIC_, uint32_t);
  rtConstantProperty(PX_EASEOUTBOUNCE, PX_EASEOUTBOUNCE_, uint32_t);
  rtConstantProperty(PX_END, PX_END, uint32_t);
  rtConstantProperty(PX_SEESAW, PX_SEESAW, uint32_t);
  rtConstantProperty(PX_LOOP, PX_LOOP, uint32_t);
  rtConstantProperty(PX_NONE, PX_NONE_, uint32_t);
  rtConstantProperty(PX_STRETCH, PX_STRETCH_, uint32_t);
  rtConstantProperty(PX_REPEAT, PX_REPEAT_, uint32_t);
 
  rtReadOnlyProperty(allInterpolators, allInterpolators, rtObjectRef);

  pxScene2d(bool top = true);
  
  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  void init();

  rtError onScene(rtFunctionRef& v) const;
  rtError setOnScene(rtFunctionRef v);

  int32_t w() const { return mWidth;  }
  rtError w(int32_t& v) const { v = mWidth;  return RT_OK; }
  int32_t h() const { return mHeight; }
  rtError h(int32_t& v) const { v = mHeight; return RT_OK; }

  rtError showOutlines(bool& v) const;
  rtError setShowOutlines(bool v);

  rtError create(rtObjectRef p, rtObjectRef& o);
  rtError createObject(rtObjectRef p, rtObjectRef& o);
  rtError createRectangle(rtObjectRef p, rtObjectRef& o);
  rtError createText(rtObjectRef p, rtObjectRef& o);
  rtError createText2(rtObjectRef p, rtObjectRef& o);
  rtError createImage(rtObjectRef p, rtObjectRef& o);
  rtError createImage9(rtObjectRef p, rtObjectRef& o);
  rtError createScene(rtObjectRef p,rtObjectRef& o);
  rtError createExternal(rtObjectRef p, rtObjectRef& o);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError focus(rtObjectRef& o)
  {
    o = mFocus;
    return RT_OK;
  }

  rtError setFocus(rtObjectRef o);
 
  rtError stopPropagation()
  {
    printf("stopPropagation()\n");
    mStopPropagation = true;
    return RT_OK;
  }

  rtError ctx(rtValue& v) const { v = mContext; return RT_OK; }
  rtError setCtx(const rtValue& v) { mContext = v; return RT_OK; }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }
  
  rtError allInterpolators(rtObjectRef& v) const;

  void setMouseEntered(pxObject* o);

  // The following methods are delegated to the view
  virtual void onSize(int32_t w, int32_t h);



  virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags);
  virtual void onMouseEnter();
  virtual void onMouseLeave();
  virtual void onMouseMove(int32_t x, int32_t y);

  virtual void onFocus();
  virtual void onBlur();

  virtual void onKeyDown(uint32_t keycode, uint32_t flags);
  virtual void onKeyUp(uint32_t keycode, uint32_t flags);
  virtual void onChar(uint32_t codepoint);
  
  virtual void onDraw();

  virtual void setViewContainer(pxIViewContainer* /*l*/) {}
  
  void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m);
  void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m);
  void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m);
  void transformPointFromObjectToScene(pxObject* o, const pxPoint2f& from, 
				       pxPoint2f& to);
  void transformPointFromSceneToObject(pxObject* o, const pxPoint2f& from, pxPoint2f& to);
  void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject,
					pxPoint2f& from, pxPoint2f& to);
  
  void hitTest(pxPoint2f p, vector<rtRefT<pxObject> > hitList);
  
  pxObject* getRoot() const;
  rtError root(rtObjectRef& v) const 
  {
    v = getRoot();
    return RT_OK;
  }
  
private:
  void bubbleEvent(rtObjectRef e, rtRefT<pxObject> t, 
                   const char* preEvent, const char* event) ;

  void draw();
  // Does not draw updates scene to time t
  // t is assumed to be monotonically increasing
  void update(double t);
  
  rtRefT<pxObject> mRoot;
  rtObjectRef mFocus;
  double start, end2;
  int frameCount;
  int mWidth;
  int mHeight;
  rtEmitRef mEmit;

// Top level scene only
  rtRefT<pxObject> mMouseEntered;
  rtRefT<pxObject> mMouseDown;
  pxPoint2f mMouseDownPt;
  rtValue mContext;
  bool mTop;
  bool mStopPropagation;
};

class pxScene2dRef: public rtRefT<pxScene2d>, public rtObjectBase
{
 public:
  pxScene2dRef() {}
  pxScene2dRef(pxScene2d* s) { asn(s); }

  // operator= is not inherited
  pxScene2dRef& operator=(pxScene2d* s) { asn(s); return *this; }
  
 private:
  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Get(uint32_t i, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);
  virtual rtError Set(uint32_t i, const rtValue* value);
};


#endif
