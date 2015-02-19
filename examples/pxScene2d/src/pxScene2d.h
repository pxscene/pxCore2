// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.h

#ifndef PX_SCENE2D_H
#define PX_SCENE2D_H

#include <stdio.h>

using namespace std;
#include <vector>
#include <list>

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
#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"

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
  rtString prop;
  float from;
  float to;
  bool flip;
  double start;
  double duration;
  pxAnimationType at;
  pxInterp interp;
  rtFunctionRef ended;
};

class pxObject;

class pxObject: public rtObject {
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
  rtProperty(painting, painting, setPainting, bool);
  rtProperty(clip, clip, setClip, bool);
  rtProperty(mask, mask, setMask, rtString);

  rtReadOnlyProperty(numChildren, numChildren, int32_t);
  rtMethod1ArgAndReturn("getChild", getChild, int32_t, rtObjectRef);
  rtReadOnlyProperty(children, children, rtObjectRef);

  rtMethodNoArgAndNoReturn("remove", remove);

  rtMethod5ArgAndNoReturn("animateTo", animateTo2, rtObjectRef, double, 
                          uint32_t, uint32_t, rtFunctionRef);

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
  rtProperty(onReady, onReady, setOnReady, rtFunctionRef);

  rtReadOnlyProperty(emit, emit, rtFunctionRef);

  pxObject(): rtObject(), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0),
    mrx(0), mry(0), mrz(1.0), msx(1), msy(1), mw(0), mh(0),
    mTextureRef(), mPainting(true), mClip(false), mMaskUrl(), mMaskTextureRef(),
    mClipTextureRef()
  {
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

  void moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();


  void drawInternal(pxMatrix4f m, float parentAlpha);
  virtual void draw() {}

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);
  
  rtError animateTo(const char* prop, double to, double duration, 
                     uint32_t interp, uint32_t animationType, 
                     rtFunctionRef onEnd);

  rtError animateTo2(rtObjectRef props, double duration, 
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
        animateTo(key, props.get<float>(key), duration, interp, animationType, 
                  (i==0)?onEnd:rtFunctionRef());
      }
    }
    return RT_OK;
  }

  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp, pxAnimationType at, 
		 rtFunctionRef onEnd);  

  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp=0, pxAnimationType at=PX_END)
  {
    animateTo(prop, to, duration, interp, at, rtFunctionRef());
  }  

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

  static void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m) {
    m.identity();
    rtRefT<pxObject> t = o;
    while(t) {
      m.translate(t->mx, t->my);
      m.scale(t->msx, t->msy);
      m.rotateZInDegrees(t->mr);
      m.translate(-t->mcx, -t->mcy);
      t = t->mParent;
    }
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
    
    for(vector<rtRefT<pxObject> >::reverse_iterator it = v.rbegin(); it != v.rend(); ++it) {
      rtRefT<pxObject>& j = *it;;
      m.translate(-j->cx, -j->cy);
      m.rotateZInDegrees(-j->r);
      m.scale(1/ j->sx, 1/ j->sy);
      m.translate(-j->x, -j->y);
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

public:
  rtEmitRef mEmit;

protected:
  // TODO getting freaking huge... 
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
  vector<animation> mAnimations;
  float mcx, mcy, mx, my, ma, mr, mrx, mry, mrz, msx, msy, mw, mh;
  pxTextureRef mTextureRef;
  bool mPainting;
  bool mClip;
  rtString mMaskUrl;
  pxTextureRef mMaskTextureRef;
  pxTextureRef mClipTextureRef;
  rtString mId;
  
  
  pxTextureRef createSnapshot(pxTextureRef texture);
  void deleteSnapshot(pxTextureRef texture);
  void createMask();
  void deleteMask();

 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
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

class pxInnerScene: public rtObject {
public:
  rtDeclareObject(pxInnerScene, rtObject);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, float);
  rtReadOnlyProperty(h, h, float);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtMethod1ArgAndReturn("createRectangle", createRectangle, rtObjectRef,
                        rtObjectRef);
  rtMethod1ArgAndReturn("createImage", createImage, rtObjectRef,
                        rtObjectRef);
  rtMethod1ArgAndReturn("createImage9", createImage9, rtObjectRef, 
                        rtObjectRef);
  rtMethod1ArgAndReturn("createText", createText, rtObjectRef, 
                        rtObjectRef);
  rtMethod1ArgAndReturn("createScene", createScene, rtObjectRef, 
                        rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
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

  pxInnerScene(rtRefT<pxObject> root)
  {
    mRoot = root;
    mEmit = new rtEmit;
  }

  rtRefT<pxObject> root()
  {
    return mRoot;
  }

  rtError root(rtObjectRef& v) const
  {
    v = mRoot;
    return RT_OK;
  }

  float w() const { return mWidth;  }
  rtError w(float& v) const { v = mWidth;  return RT_OK; }
  float h() const { return mHeight; }
  rtError h(float& v) const { v = mHeight; return RT_OK; }
  rtError setW(float v) 
  {
    mWidth = v; 
    rtObjectRef e = new rtMapObject;
    e.set("name", "onResize");
    e.set("w", mWidth);
    e.set("h", mHeight);
    mEmit.send("onResize", e);
    return RT_OK; 
  }

  rtError setH(float v) 
  { 
    mHeight = v; 
    rtObjectRef e = new rtMapObject;
    e.set("name", "onResize");
    e.set("w", mWidth);
    e.set("h", mHeight);
    mEmit.send("onResize", e);
    return RT_OK; 
  }

  rtError showOutlines(bool& v) const;
  rtError setShowOutlines(bool v);

  rtError createRectangle(rtObjectRef p, rtObjectRef& o);
  rtError createText(rtObjectRef p, rtObjectRef& o);
  rtError createImage(rtObjectRef p, rtObjectRef& o);
  rtError createImage9(rtObjectRef p, rtObjectRef& o);
  rtError createScene(rtObjectRef p,rtObjectRef& o);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  rtError ctx(rtValue& v) const { v = mContext; return RT_OK; }
  rtError setCtx(const rtValue& v) { mContext = v; return RT_OK; }

  rtError emit(rtFunctionRef& v) const { v = mEmit; return RT_OK; }
  
  rtError allInterpolators(rtObjectRef& v) const;

private:
  rtRefT<pxObject> mRoot;
  rtEmitRef mEmit;
  float mWidth;
  float mHeight;
  rtValue mContext;
};

// For now just child scene objects
class pxScene: public pxObject {
public:
  rtDeclareObject(pxScene, pxObject);
  rtProperty(url, url, setURL, rtString);
  rtProperty(w, w, setW, float);
  rtProperty(h, h, setH, float);
  rtReadOnlyProperty(emit, emit, rtFunctionRef);

  pxScene() : pxObject()
  { 
  }

  rtError url(rtString& v) const { v = mURL; return RT_OK; }
  rtError setURL(rtString v);

  // TODO probably just keep w, h in scene and have innerscene use that
  rtError w(float& v) const { v = mInnerScene->w(); return RT_OK; }
  rtError setW(float v) { mw = v; mInnerScene->setW(v); return RT_OK; }
  rtError h(float& v) const { v = mInnerScene->h(); return RT_OK; }
  rtError setH(float v) { mh = v; mInnerScene->setH(v); return RT_OK; }

  rtError emit(rtFunctionRef& v) const { return mInnerScene->get<rtFunctionRef>("emit", v); }

private:
  rtRefT<pxInnerScene> mInnerScene;
  rtString mURL;
};

class pxScene2d: public rtObject {
public:
  rtDeclareObject(pxScene2d, rtObject);
  rtProperty(onScene, onScene, setOnScene, rtFunctionRef);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);
  rtProperty(showOutlines, showOutlines, setShowOutlines, bool);
  rtMethodNoArgAndReturn("createRectangle", createRectangle, rtObjectRef);
  rtMethodNoArgAndReturn("createImage", createImage, rtObjectRef);
  rtMethodNoArgAndReturn("createImage9", createImage9, rtObjectRef);
  rtMethodNoArgAndReturn("createText", createText, rtObjectRef);
  rtMethodNoArgAndReturn("createScene", createScene, rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
  pxScene2d();
  
  void init();

  rtError onScene(rtFunctionRef& v) const;
  rtError setOnScene(rtFunctionRef v);

  int32_t w() const { return mWidth;  }
  rtError w(int32_t& v) const { v = mWidth;  return RT_OK; }
  int32_t h() const { return mHeight; }
  rtError h(int32_t& v) const { v = mHeight; return RT_OK; }

  rtError showOutlines(bool& v) const;
  rtError setShowOutlines(bool v);

  rtError createRectangle(rtObjectRef& o);
  rtError createText(rtObjectRef& o);
  rtError createImage(rtObjectRef& o);
  rtError createImage9(rtObjectRef& o);
  rtError createScene(rtObjectRef& o);

  rtError addListener(rtString eventName, const rtFunctionRef& f)
  {
    return mEmit->addListener(eventName, f);
  }

  rtError delListener(rtString  eventName, const rtFunctionRef& f)
  {
    return mEmit->delListener(eventName, f);
  }

  // The following methods are delegated to the view
  virtual void onSize(int w, int h);
  virtual void onMouseDown(int x, int y, unsigned long flags);
  virtual void onMouseUp(int x, int y, unsigned long flags);
  virtual void onMouseLeave();
  virtual void onMouseMove(int x, int y);
  
  // JR expect keycodes etc to change quite a bit
  // not well supported in glut
  // glut doesn't seem to support notion of key up vs down so both of these
  // get fired on physical key down right now
  // I want to normalize on "browser" key codes
  virtual void onKeyDown(int keycode, unsigned long flags);
  virtual void onKeyUp(int keycode, unsigned long flags);
  virtual void onChar(char c);
  
  virtual void onDraw();
  
  void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m);
  void getMatrixFromSceneToObject(pxObject* o, pxMatrix4f& m);
  void getMatrixFromObjectToObject(pxObject* from, pxObject* to, pxMatrix4f& m);
  void transformPointFromObjectToScene(pxObject* o, const pxPoint2f& from, 
				       pxPoint2f& to);
  void transformPointFromObjectToObject(pxObject* fromObject, pxObject* toObject,
					pxPoint2f& from, pxPoint2f& to);
  
  void hitTest(pxPoint2f p, vector<rtRefT<pxObject> > hitList);
  
  pxObject* getRoot() const;
  rtError root(rtObjectRef& v) const 
  {
    v = getRoot();
    return RT_OK;
  }
  
  void checkForCompletedImageDownloads();
  
private:
  void draw();
  // Does not draw updates scene to time t
  // t is assumed to be monotonically increasing
  void update(double t);
  
  rtRefT<pxObject> mRoot;
  double start, end2;
  int frameCount;
  int mWidth;
  int mHeight;

  rtEmitRef mEmit;
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
