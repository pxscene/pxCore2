// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.h

#ifndef PX_SCENE2D_H
#define PX_SCENE2D_H

#include <stdio.h>

#include <vector>
#include <list>
using namespace std;

#ifndef finline
#ifdef WIN32
#define finline __forceinline
#else
#define finline __attribute__((always_inline))
#endif
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

#include "rtPromise.h"

#include "pxCore.h"
#include "pxIView.h"

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
#include "pxTextureCacheObject.h"
#include "pxContextFramebuffer.h"

#include "testView.h"

// TODO Finish
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

enum pxAnimationType {PX_END = 0, PX_OSCILLATE, PX_LOOP};

struct pxAnimationTarget {
  char* prop;
  float to;
};

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

struct pxPoint2f {
  pxPoint2f() {}
  pxPoint2f(float _x, float _y) { x = _x; y = _y; } 
  float x, y;
};

typedef double (*pxInterp)(double i);
typedef void (*pxAnimationEnded)(void* ctx);

double pxInterpLinear(double i);

class pxFileDownloadRequest;

class pxScene2d;

class pxObjectClone;

typedef rtRefT<pxObjectClone> pxObjectCloneRef;

class pxObjectImpl
{
protected:
  vector<animation> mAnimations;  
};

struct pxObjectCloneProperty {
    rtString propertyName;
    rtValue value;
};

class pxObjectClone
{
public:
    pxObjectClone();
    virtual ~pxObjectClone();

    virtual unsigned long AddRef()
    {
      return rtAtomicInc(&mRef);
    }

    virtual unsigned long Release()
    {
      unsigned long l = rtAtomicDec(&mRef);
      if (l == 0)
        delete this;
      return l;
    }

    rtError getProperty(rtString propertyName, rtValue &value);
    const vector<pxObjectCloneProperty>& getProperties();
    rtError setProperty(rtString propertyName, rtValue value);
    const vector<rtRefT<pxObject> >& getChildren();
    rtError setChildren(vector<rtRefT<pxObject> > children);
    rtError addChild(rtRefT<pxObject> child);
    rtError removeChild(rtRefT<pxObject> child);
    rtError removeAllChildren();
    rtRefT<pxObject> getParent();
    rtError setParent(rtRefT<pxObject> parent);
    rtError clearProperties();
    bool childrenAreModified();
    rtError setMatrixValue(int index, float value);
    const pxMatrix4f& getMatrix();
    bool matrixIsModified();

    rtError reset();

private:
    rtAtomic mRef;
    vector<pxObjectCloneProperty> mProperties;
    vector<rtRefT<pxObject> > mChildren;
    rtRefT<pxObject> mParent;
    bool mChildrenAreModified;
    pxMatrix4f mMatrix;
    bool mMatrixIsModified;
};

class pxObject: public rtObject, private pxObjectImpl
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

  pxObject(pxScene2d* scene): rtObject(), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0),
    mrx(0), mry(0), mrz(1.0), msx(1), msy(1), mw(0), mh(0),
    mInteractive(true),
    mSnapshotRef(), mPainting(true), mClip(false), mMaskUrl(), mDrawAsMask(false), mDraw(true), mDrawAsHitTest(true), mReady(), mMaskTextureRef(),
    mMaskTextureCacheObject(),mClipSnapshotRef(),mCancelInSet(true),mUseMatrix(false), mRepaint(true)
      , mRepaintCount(0), mClone() //TODO - remove mRepaintCount as it's only needed on certain platforms
#ifdef PX_DIRTY_RECTANGLES
    , mIsDirty(false), mLastRenderMatrix(), mScreenCoordinates()
    #endif //PX_DIRTY_RECTANGLES
  {
    mScene = scene;
    mReady = new rtPromise;
    mEmit = new rtEmit;
    createClone();
  }

  virtual ~pxObject() { /*printf("pxObject destroyed\n");*/ rtValue nullValue; mReady.send("reject",nullValue); deleteSnapshot(mSnapshotRef); deleteSnapshot(mClipSnapshotRef);}

  // TODO missing conversions in rtValue between uint32_t and int32_t
  uint32_t numChildren() const { return getChildren().size(); }
  rtError numChildren(int32_t& v) const 
  {
    v = getChildren().size();
    return RT_OK;
  }

  virtual rtError Set(const char* name, const rtValue* value);

  rtError getChild(int32_t i, rtObjectRef& r) const 
  {
    r = getChildren()[i];
    return RT_OK;
  }

  rtError children(rtObjectRef& v) const;

  // TODO clean this up
  void setParent(rtRefT<pxObject>& parent);
  pxObject* parent() const
  {
    if (mClone->getParent().getPtr() != NULL)
    {
      return mClone->getParent();
    }
    else
    {
      return mParent;
    }
  }

  rtError parent(rtObjectRef& v) const
  {
    if (mClone->getParent().getPtr() != NULL)
    {
      v = mClone->getParent();
    }
    else
    {
      v = mParent.getPtr();
    }
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

  float x()             const {
    rtValue value;
    if (getCloneProperty("x", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mx;
  }
  rtError x(float& v)   const {
    rtValue value;
    if (getCloneProperty("x", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mx;
    return RT_OK;
  }
  rtError setX(float v)       { cancelAnimation("x"); setCloneProperty("x",v); /*mx = v; mfnote*/ return RT_OK; }

  float y()             const {
    rtValue value;
    if (getCloneProperty("y", value) == RT_OK)
    {
      return value.toFloat();
    }
    return my;
  }
  rtError y(float& v)   const {
    rtValue value;
    if (getCloneProperty("y", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = my;
    return RT_OK;
  }
  rtError setY(float v)       { cancelAnimation("y"); setCloneProperty("y",v); /*my = v;*/ return RT_OK;   }

  float w()             const {
    rtValue value;
    if (getCloneProperty("w", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mw;
  }
  rtError w(float& v)   const {
    rtValue value;
    if (getCloneProperty("w", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mw;
    return RT_OK;
  }
  virtual rtError setW(float v)       { cancelAnimation("w"); setCloneProperty("w",v); /*mw = v;*/ return RT_OK;   }

  float h()             const {
    rtValue value;
    if (getCloneProperty("h", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mh;
  }
  rtError h(float& v)   const {
    rtValue value;
    if (getCloneProperty("h", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mh;
    return RT_OK;
  }
  virtual rtError setH(float v)       { cancelAnimation("h"); setCloneProperty("h",v); /*mh = v;*/ return RT_OK;   }

  float cx()            const {
    rtValue value;
    if (getCloneProperty("cx", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mcx;
  }
  rtError cx(float& v)  const {
    rtValue value;
    if (getCloneProperty("cx", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mcx;
    return RT_OK;
  }
  rtError setCX(float v)      { cancelAnimation("cx"); setCloneProperty("cx",v); /*mcx = v;*/ return RT_OK;  }

  float cy()            const {
    rtValue value;
    if (getCloneProperty("cy", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mcy;
  }
  rtError cy(float& v)  const {
    rtValue value;
    if (getCloneProperty("cy", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mcy;
    return RT_OK;
  }
  rtError setCY(float v)      { cancelAnimation("cy"); setCloneProperty("cy",v); /*mcy = v;*/ return RT_OK;  }

  float sx()            const {
    rtValue value;
    if (getCloneProperty("sx", value) == RT_OK)
    {
      return value.toFloat();
    }
    return msx;
  }
  rtError sx(float& v)  const {
    rtValue value;
    if (getCloneProperty("sx", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = msx;
    return RT_OK;
  }
  rtError setSX(float v)      { cancelAnimation("sx"); setCloneProperty("sx",v); /*msx = v;*/ return RT_OK;  }

  float sy()            const {
    rtValue value;
    if (getCloneProperty("sy", value) == RT_OK)
    {
      return value.toFloat();
    }
    return msy;
  }
  rtError sy(float& v)  const {
    rtValue value;
    if (getCloneProperty("sy", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = msx;
    return RT_OK;
  }
  rtError setSY(float v)      { cancelAnimation("sy"); setCloneProperty("sy",v); /*msy = v;*/ return RT_OK;  }

  float a()             const {
    rtValue value;
    if (getCloneProperty("a", value) == RT_OK)
    {
      return value.toFloat();
    }
    return ma;
  }
  rtError a(float& v)   const {
    rtValue value;
    if (getCloneProperty("a", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = ma;
    return RT_OK;
  }
  rtError setA(float v)       { cancelAnimation("a"); setCloneProperty("a",v); /*ma = v;*/ return RT_OK;   }

  float r()             const {
    rtValue value;
    if (getCloneProperty("r", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mr;
  }
  rtError r(float& v)   const {
    rtValue value;
    if (getCloneProperty("r", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mr;
    return RT_OK;
  }
  rtError setR(float v)       { cancelAnimation("r"); setCloneProperty("r",v); /*mr = v;*/ return RT_OK;   }

  float rx()            const {
    rtValue value;
    if (getCloneProperty("rx", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mrx;
  }
  rtError rx(float& v)  const {
    rtValue value;
    if (getCloneProperty("rx", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mrx;
    return RT_OK;
  }
  rtError setRX(float v)      { cancelAnimation("rx"); setCloneProperty("rx",v); /*mrx = v;*/ return RT_OK;  }

  float ry()            const {
    rtValue value;
    if (getCloneProperty("ry", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mry;
  }
  rtError ry(float& v)  const {
    rtValue value;
    if (getCloneProperty("ry", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mry;
    return RT_OK;
  }
  rtError setRY(float v)      { cancelAnimation("ry"); setCloneProperty("ry",v); /*mry = v;*/ return RT_OK;  }

  float rz()            const {
    rtValue value;
    if (getCloneProperty("rz", value) == RT_OK)
    {
      return value.toFloat();
    }
    return mrz;
  }
  rtError rz(float& v)  const {
    rtValue value;
    if (getCloneProperty("rz", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mrz;
    return RT_OK;
  }
  rtError setRZ(float v)      { cancelAnimation("rz"); setCloneProperty("rz",v); /*mrz = v;*/ return RT_OK;  }

  bool painting()            const {
    rtValue value;
    if (getCloneProperty("painting", value) == RT_OK)
    {
      return value.toBool();
    }
    return mPainting;
  }
  rtError painting(bool& v)  const {
    rtValue value;
    if (getCloneProperty("painting", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mPainting;
    return RT_OK;
  }
  rtError setPainting(bool /*v*/)
  {
    setCloneProperty("painting",true/*v*/);
     /* mPainting = v;
      if (!mPainting)
      {
        mSnapshotRef = createSnapshot(mSnapshotRef);
      }
      else
      {
          deleteSnapshot(mSnapshotRef);
      }*/
      return RT_OK;
  }

  bool clip()            const {
    rtValue value;
    if (getCloneProperty("clip", value) == RT_OK)
    {
      return value.toBool();
    }
    return mClip;
  }
  rtError clip(bool& v)  const {
    rtValue value;
    if (getCloneProperty("clip", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mClip;
    return RT_OK;
  }
  virtual rtError setClip(bool v) { setCloneProperty("clip",v); /*mClip = v;*/ return RT_OK; }
  
  rtString mask()            const {
    rtValue value;
    if (getCloneProperty("mask", value) == RT_OK)
    {
      return value.toString();
    }
    return mMaskUrl;
  }
  rtError mask(rtString& v)  const {
    rtValue value;
    if (getCloneProperty("mask", value) == RT_OK)
    {
      v = value.toString();
      return RT_OK;
    }
    v = mMaskUrl;
    return RT_OK;
  }
  rtError setMask(rtString v) { setCloneProperty("mask",v); /*mMaskUrl = v; createMask();*/ return RT_OK; }

  bool drawAsMask()            const {
    rtValue value;
    if (getCloneProperty("drawAsMask", value) == RT_OK)
    {
      return value.toBool();
    }
    return mDrawAsMask;
  }
  rtError drawAsMask(bool& v)  const {
    rtValue value;
    if (getCloneProperty("drawAsMask", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mDrawAsMask;
    return RT_OK;
  }
  rtError setDrawAsMask(bool v) { setCloneProperty("drawAsMask",v); /*mDrawAsMask = v;*/ return RT_OK; }

  bool drawEnabled()            const {
    rtValue value;
    if (getCloneProperty("draw", value) == RT_OK)
    {
      return value.toBool();
    }
    return mDraw;
  }
  rtError drawEnabled(bool& v)  const {
    rtValue value;
    if (getCloneProperty("draw", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mDraw;
    return RT_OK;
  }
  rtError setDrawEnabled(bool v) { setCloneProperty("draw",v); /*mDraw = v;*/ return RT_OK; }

  bool drawAsHitTest()            const {
    rtValue value;
    if (getCloneProperty("drawAsHitTest", value) == RT_OK)
    {
      return value.toBool();
    }
    return mDrawAsHitTest;
  }
  rtError drawAsHitTest(bool& v)  const {
    rtValue value;
    if (getCloneProperty("drawAsHitTest", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mDrawAsHitTest;
    return RT_OK;
  }
  rtError setDrawAsHitTest(bool v) { setCloneProperty("drawAsHitTest",v); /*mDrawAsHitTest = v;*/ return RT_OK; }

  rtError ready(rtObjectRef& v) const
  {
    v = mReady;
    return RT_OK;
  }

  rtError moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();

  virtual void commit();
  void drawInternal(bool maskPass=false);
  virtual void draw() {}

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);

  rtError animateTo(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t animationType, 
                     rtObjectRef promise);

  rtError animateToP2(rtObjectRef props, double duration, 
                      uint32_t interp, uint32_t animationType, 
                      rtObjectRef& promise);

  void animateTo(const char* prop, double to, double duration,
		 pxInterp interp, pxAnimationType at, 
                 rtObjectRef promise);

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

    for (vector<rtRefT<pxObject> >::const_iterator it = from->getChildren().begin(); it != from->getChildren().end(); ++it)
    {
      pxObject *o = getObjectById(id, (*it).getPtr());
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

  rtError m11(float& v) const {v = getMatrix().constData(0);return RT_OK; }
  rtError m12(float& v) const {v = getMatrix().constData(1); return RT_OK; }
  rtError m13(float& v) const {v = getMatrix().constData(2); return RT_OK; }
  rtError m14(float& v) const {v = getMatrix().constData(3); return RT_OK; }
  rtError m21(float& v) const {v = getMatrix().constData(4); return RT_OK; }
  rtError m22(float& v) const {v = getMatrix().constData(5); return RT_OK; }
  rtError m23(float& v) const {v = getMatrix().constData(6); return RT_OK; }
  rtError m24(float& v) const {v = getMatrix().constData(7); return RT_OK; }
  rtError m31(float& v) const {v = getMatrix().constData(8); return RT_OK; }
  rtError m32(float& v) const {v = getMatrix().constData(9); return RT_OK; }
  rtError m33(float& v) const {v = getMatrix().constData(10); return RT_OK; }
  rtError m34(float& v) const {v = getMatrix().constData(11); return RT_OK; }
  rtError m41(float& v) const {v = getMatrix().constData(12); return RT_OK; }
  rtError m42(float& v) const {v = getMatrix().constData(13); return RT_OK; }
  rtError m43(float& v) const {v = getMatrix().constData(14); return RT_OK; }
  rtError m44(float& v) const {v = getMatrix().constData(15); return RT_OK; }

  rtError setM11(const float& v) { cancelAnimation("m11",true); mClone->setMatrixValue(0,v); /*mMatrix.data()[0] = v;*/ return RT_OK; }
  rtError setM12(const float& v) { cancelAnimation("m12",true); mClone->setMatrixValue(1,v); /*mMatrix.data()[1] = v;*/ return RT_OK; }
  rtError setM13(const float& v) { cancelAnimation("m13",true); mClone->setMatrixValue(2,v); /*mMatrix.data()[2] = v;*/ return RT_OK; }
  rtError setM14(const float& v) { cancelAnimation("m14",true); mClone->setMatrixValue(3,v); /*mMatrix.data()[3] = v;*/ return RT_OK; }
  rtError setM21(const float& v) { cancelAnimation("m21",true); mClone->setMatrixValue(4,v); /*mMatrix.data()[4] = v;*/ return RT_OK; }
  rtError setM22(const float& v) { cancelAnimation("m22",true); mClone->setMatrixValue(5,v); /*mMatrix.data()[5] = v;*/ return RT_OK; }
  rtError setM23(const float& v) { cancelAnimation("m23",true); mClone->setMatrixValue(6,v); /*mMatrix.data()[6] = v;*/ return RT_OK; }
  rtError setM24(const float& v) { cancelAnimation("m24",true); mClone->setMatrixValue(7,v); /*mMatrix.data()[7] = v;*/ return RT_OK; }
  rtError setM31(const float& v) { cancelAnimation("m31",true); mClone->setMatrixValue(8,v); /* mMatrix.data()[8] = v;*/ return RT_OK; }
  rtError setM32(const float& v) { cancelAnimation("m32",true); mClone->setMatrixValue(9,v); /*mMatrix.data()[9] = v;*/ return RT_OK; }
  rtError setM33(const float& v) { cancelAnimation("m33",true); mClone->setMatrixValue(10,v); /*mMatrix.data()[10] = v;*/ return RT_OK; }
  rtError setM34(const float& v) { cancelAnimation("m34",true); mClone->setMatrixValue(11,v); /*mMatrix.data()[11] = v;*/ return RT_OK; }
  rtError setM41(const float& v) { cancelAnimation("m41",true); mClone->setMatrixValue(12,v); /*mMatrix.data()[12] = v;*/ return RT_OK; }
  rtError setM42(const float& v) { cancelAnimation("m42",true); mClone->setMatrixValue(13,v); /*mMatrix.data()[13] = v;*/ return RT_OK; }
  rtError setM43(const float& v) { cancelAnimation("m43",true); mClone->setMatrixValue(14,v); /*mMatrix.data()[14] = v;*/ return RT_OK; }
  rtError setM44(const float& v) { cancelAnimation("m44",true); mClone->setMatrixValue(15,v); /*mMatrix.data()[15] = v;*/ return RT_OK; }

  rtError useMatrix(bool& v) const {
    rtValue value;
    if (getCloneProperty("useMatrix", value) == RT_OK)
    {
      v = value.toBool();
      return RT_OK;
    }
    v = mUseMatrix;
    return RT_OK;
  }
  rtError setUseMatrix(const bool& v) { mUseMatrix = v; return RT_OK; }

  void repaint() { mRepaint = true; mRepaintCount = 0; }

  pxObjectCloneRef getClone() { return mClone; }
  const vector<rtRefT<pxObject> >& getChildren() const
  {
    if (mClone->childrenAreModified())
    {
      return mClone->getChildren();
    }
    return mChildren;
  }

  const pxMatrix4f& getMatrix() const
  {
    if (mClone->matrixIsModified())
    {
      return mClone->getMatrix();
    }
    return mMatrix;
  }

public:
  rtEmitRef mEmit;

protected:
  // TODO getting freaking huge... 
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
//  vector<animation> mAnimations;
  float mcx, mcy, mx, my, ma, mr, mrx, mry, mrz, msx, msy, mw, mh;
  bool mInteractive;
  pxContextFramebufferRef mSnapshotRef;
  bool mPainting;
  bool mClip;
  rtString mMaskUrl;
  bool mDrawAsMask;
  bool mDraw;
  bool mDrawAsHitTest;
  rtObjectRef mReady;
  pxTextureRef mMaskTextureRef;
  pxTextureCacheObject mMaskTextureCacheObject;
  pxContextFramebufferRef mClipSnapshotRef;
  bool mCancelInSet;
  rtString mId;
  pxMatrix4f mMatrix;
  bool mUseMatrix;
  bool mRepaint;
  int mRepaintCount;
  pxObjectCloneRef mClone;
  #ifdef PX_DIRTY_RECTANGLES
  bool mIsDirty;
  pxMatrix4f mLastRenderMatrix;
  pxRect mScreenCoordinates;
  #endif //PX_DIRTY_RECTANGLES

  pxContextFramebufferRef createSnapshot(pxContextFramebufferRef fbo);
  void createSnapshotOfChildren(pxContextFramebufferRef drawableFbo, pxContextFramebufferRef maskFbo);
  void deleteSnapshot(pxContextFramebufferRef fbo);
  void createMask();
  void deleteMask();
  virtual void commitClone();
  void createClone();
  void deleteClone();
  rtError getCloneProperty(rtString propertyName, rtValue& value) const;
  rtError setCloneProperty(rtString propertyName, rtValue value);
  bool hasClone() const;
  #ifdef PX_DIRTY_RECTANGLES
  pxRect getBoundingRectInScreenCoordinates();
  #endif //PX_DIRTY_RECTANGLES

  pxScene2d* mScene;

 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
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

  virtual ~pxViewContainer() {}

  rtError setView(pxIView* v)
  {
    mView = v;
    if (mView)
    {
      float width = 0;
      float height = 0;
      w(width);
      h(height);
      mView->onSize(width,height);
      mView->setViewContainer(this);
    }
    return RT_OK;
  }

  void invalidateRect(pxRect* r);

#if 0
  rtError uri(rtString& v) const { v = mURI; return RT_OK; }
  rtError setURI(rtString v) { mURI = v; return RT_OK; }
#endif

  rtError w(float& v) const {
    rtValue value;
    if (getCloneProperty("w", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mw;
    return RT_OK;
  }
  rtError setW(float v) 
  {
    setCloneProperty("w",v);
    //mw = v;
    if (mView)
    {
      float width = 0;
      float height = 0;
      w(width);
      h(height);
      mView->onSize(width, height);
    }

    return RT_OK; 
  }
  
  rtError h(float& v) const {
    rtValue value;
    if (getCloneProperty("h", value) == RT_OK)
    {
      v = value.toFloat();
      return RT_OK;
    }
    v = mh;
    return RT_OK;
  }
  rtError setH(float v) 
  {
    setCloneProperty("h",v);
    //mh = v;
    if (mView)
    {
      float width = 0;
      float height = 0;
      w(width);
      h(height);
      mView->onSize(width, height);
    }
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

  virtual void update(double t)
  {
    if (mView)
      mView->onUpdate(t);
    pxObject::update(t);
  }

  virtual void commit()
  {
    if (mView)
      mView->onCommit();
    pxObject::commit();
  }

  virtual void draw() 
  {
    if (mView)
      mView->onDraw();
  }

  

protected:
  virtual void commitClone();
  pxViewRef mView;
  rtString mURI;
};


class pxSceneContainer: public pxViewContainer
{
public:
  rtDeclareObject(pxSceneContainer, pxViewContainer);
  rtProperty(url, uri, setURI, rtString);
  rtReadOnlyProperty(api, api, rtValue);
  
pxSceneContainer(pxScene2d* scene):pxViewContainer(scene){}

  rtError uri(rtString& v) const {
    //mfnote: todo
    /*rtValue value;
    if (getCloneProperty("uri", value) == RT_OK)
    {
      v = value.toString();
      return RT_OK;
    }*/
    v = mURI;
    return RT_OK;
  }
  rtError setURI(rtString v);

  rtError api(rtValue& v) const;
private:
  rtRefT<pxScene2d> mScene;
  rtString mURI;
};


typedef rtRefT<pxObject> pxObjectRef;

class pxScene2d: public rtObject, public pxIView {
public:
  rtDeclareObject(pxScene2d, rtObject);
  rtProperty(onScene, onScene, setOnScene, rtFunctionRef);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtProperty(showDirtyRect, showDirtyRect, setShowDirtyRect, bool);
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
  
  rtMethod1ArgAndReturn("screenshot", screenshot, rtString, rtString);

  rtProperty(ctx, ctx, setCtx, rtValue);
  rtProperty(api, api, setAPI, rtValue);
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
  // TODO deprecated remove from js samples
  rtConstantProperty(PX_SEESAW, PX_OSCILLATE, uint32_t);
  rtConstantProperty(PX_OSCILLATE, PX_OSCILLATE, uint32_t);
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

  rtError showDirtyRect(bool& v) const;
  rtError setShowDirtyRect(bool v);

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

  rtError api(rtValue& v) const { v = mAPI; return RT_OK; }
  rtError setAPI(const rtValue& v) { mAPI = v; return RT_OK; }

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
  
  virtual void onUpdate(double t);
  virtual void onDraw();
  virtual void onCommit();

  virtual void setViewContainer(pxIViewContainer* l) 
  {
    mContainer = l;
  }

  void invalidateRect(pxRect* r);
  
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

  // Note: Only type currently supported is "image/png;base64"
  rtError screenshot(rtString type, rtString& pngData);
  
  rtRefT<pxObject> mRoot;
  rtObjectRef mFocus;
  double start, end2;
  int frameCount;
  int mWidth;
  int mHeight;
  rtEmitRef mEmit;

// TODO Top level scene only
  rtRefT<pxObject> mMouseEntered;
  rtRefT<pxObject> mMouseDown;
  pxPoint2f mMouseDownPt;
  rtValue mContext;
  rtValue mAPI;
  bool mTop;
  bool mStopPropagation;
  int mTag;
  pxIViewContainer *mContainer;
  bool mShowDirtyRect;
public:
  bool mDirty;
  #ifdef PX_DIRTY_RECTANGLES
  static pxRect mDirtyRect;
  #endif //PX_DIRTY_RECTANGLES
};

// TODO do we need this anymore?
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
