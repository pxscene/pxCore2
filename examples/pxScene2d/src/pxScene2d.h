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

#include "rtDefs.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"

#include "pxMatrix4T.h"

#include "pxCore.h"
#include "pxInterpolators.h"

#include "rtCore.h"


typedef double (*pxInterp)(double i);
typedef void (*pxAnimationEnded)(void* ctx);

double pxInterpLinear(double i);

enum pxAnimationType {PX_STOP = 0, PX_SEESAW, PX_LOOP};

struct pxPoint2f {
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
#if 0
  pxAnimationEnded ended;
  void* ctx;
#else
  rtFunctionRef ended;
#endif
};

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
  rtProperty(painting, painting, setPainting, bool);
  // would be nice to expose as collection to js
  rtReadOnlyProperty(numChildren, numChildren, int32_t);
  rtMethod1ArgAndReturn("getChild", getChild, int32_t, rtObjectRef);

  rtMethodNoArgAndNoReturn("remove", remove);
  rtMethod5ArgAndNoReturn("animateTo", animateTo, rtString, double, double, 
			  uint32_t, uint32_t);

  // Until we can sort out how to do optional/default args
  rtMethod6ArgAndNoReturn("animateTo2", animateTo2, rtString, double, double, 
                          uint32_t, uint32_t, rtFunctionRef);

 pxObject(): mRef(0), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0), 
    mrx(0), mry(0), mrz(1.0), msx(1), msy(1), mw(0), mh(0),
    mContextSurfaceSnapshot(NULL), mPainting(true) {}

  virtual ~pxObject() { /*printf("pxObject destroyed\n");*/ deleteSnapshot(); }
  virtual unsigned long AddRef() { return ++mRef; }
  virtual unsigned long Release() { if (--mRef == 0) delete this; return mRef; }
  
  // TODO missing conversions in rtValue between uint32_t and int32_t
  rtError numChildren(int32_t& v) const {
    v = mChildren.size();
    return RT_OK;
  }

  rtError getChild(int32_t i, rtObjectRef& r) const {
    r = mChildren[i];
    return RT_OK;
  }

  // clean this up
  void setParent(rtRefT<pxObject>& parent);

  rtError parent(rtObjectRef& v) const {
    v = mParent.getPtr();
    return RT_OK;
  }

  rtError setParent(rtObjectRef parent) {
    void* p = parent.get<voidPtr>("_pxObject");
    if (p) {
      rtRefT<pxObject> p2 = (pxObject*)p;
      setParent(p2);
    }
    return RT_OK;
  }

  rtError remove();
  
  float x()             const { return mx; }
  rtError x(float& v)   const { v = mx; return RT_OK;   }
  rtError setX(float v)       { mx = v; return RT_OK;   }
  float y()             const { return my; }
  rtError y(float& v)   const { v = my; return RT_OK;   }
  rtError setY(float v)       { my = v; return RT_OK;   }
  float w()             const { return mw; }
  rtError w(float& v)   const { v = mw; return RT_OK;   }
  rtError setW(float v)       { mw = v; return RT_OK;   }
  float h()             const { return mh; }
  rtError h(float& v)   const { v = mh; return RT_OK;   }
  rtError setH(float v)       { mh = v; return RT_OK;   }
  float cx()            const { return mcx;}
  rtError cx(float& v)  const { v = mcx; return RT_OK;  }
  rtError setCX(float v)      { mcx = v; return RT_OK;  }
  float cy()            const { return mcy;}
  rtError cy(float& v)  const { v = mcy; return RT_OK;  }
  rtError setCY(float v)      { mcy = v; return RT_OK;  }
  float sx()            const { return msx;}
  rtError sx(float& v)  const { v = msx; return RT_OK;  }
  rtError setSX(float v)      { msx = v; return RT_OK;  }
  float sy()            const { return msy;}
  rtError sy(float& v)  const { v = msx; return RT_OK;  } 
  rtError setSY(float v)      { msy = v; return RT_OK;  }
  float a()             const { return ma; }
  rtError a(float& v)   const { v = ma; return RT_OK;   }
  rtError setA(float v)       { ma = v; return RT_OK;   }
  float r()             const { return mr; }
  rtError r(float& v)   const { v = mr; return RT_OK;   }
  rtError setR(float v)       { mr = v; return RT_OK;   }
  float rx()            const { return mrx;}
  rtError rx(float& v)  const { v = mrx; return RT_OK;  }
  rtError setRX(float v)      { mrx = v; return RT_OK;  }
  float ry()            const { return mry;}
  rtError ry(float& v)  const { v = mry; return RT_OK;  }
  rtError setRY(float v)      { mry = v; return RT_OK;  }
  float rz()            const { return mrz;}
  rtError rz(float& v)  const { v = mrz; return RT_OK;  }
  rtError setRZ(float v)      { mrz = v; return RT_OK;  }
  bool painting()            const { return mPainting;}
  rtError painting(bool& v)  const { v = mPainting; return RT_OK;  }
  rtError setPainting(bool v)
  { 
      mPainting = v; 
      if (!mPainting)
      {
          createSnapshot();
      }
      else
      {
          deleteSnapshot();
      }
      return RT_OK;
  }

  void moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();

//  void tick(double t);
  virtual void drawInternal(pxMatrix4f m);
  virtual void draw() {}
  bool hitTest(const pxPoint2f& pt);
  
  rtError animateTo(const char* prop, double to, double duration, 
                    uint32_t interp, uint32_t animationType);

  rtError animateTo2(const char* prop, double to, double duration, 
                     uint32_t interp, uint32_t animationType, 
                     rtFunctionRef onEnd);
#if 0
  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp=0, pxAnimationType at=stop, 
		 pxAnimationEnded e = 0, void* c = 0);  
#else
  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp, pxAnimationType at, 
		 rtFunctionRef onEnd);  

  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp=0, pxAnimationType at=PX_STOP)
  {
    animateTo(prop, to, duration, interp, at, rtFunctionRef());
  }  
#endif

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

protected:
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
  vector<animation> mAnimations;
  unsigned long mRef;
  float mcx, mcy, mx, my, ma, mr, mrx, mry, mrz, msx, msy, mw, mh;
  pxContextSurfaceNativeDesc* mContextSurfaceSnapshot;
  bool mPainting;
  
  void createSnapshot();
  void deleteSnapshot();
 private:
  rtError _pxObject(voidPtr& v) const {
    v = (void*)this;
    return RT_OK;
  }
};

typedef rtRefT<pxObject> pxObjectRef;


class pxInnerScene: public pxObject {
public:
  rtDeclareObject(pxInnerScene, pxObject);
  rtReadOnlyProperty(root, root, rtObjectRef);
  rtMethodNoArgAndReturn("createRectangle", createRectangle, rtObjectRef);
  rtMethodNoArgAndReturn("createImage", createImage, rtObjectRef);
  rtMethodNoArgAndReturn("createImage9", createImage9, rtObjectRef);
  rtMethodNoArgAndReturn("createText", createText, rtObjectRef);
  rtMethodNoArgAndReturn("createScene", createScene, rtObjectRef);
  
  pxInnerScene()
  {
    mRoot = new pxObject;
  }

  rtError root(rtObjectRef& v) const
  {
    v = mRoot;
    return RT_OK;
  }

  rtError createRectangle(rtObjectRef& o);
  rtError createText(rtObjectRef& o);
  rtError createImage(rtObjectRef& o);
  rtError createImage9(rtObjectRef& o);
  rtError createScene(rtObjectRef& o);

  virtual void update(double t)
  {
    mRoot->update(t);
  }
  
  virtual void drawInternal(pxMatrix4f m) 
  {
    mRoot->drawInternal(m);
  }

private:
  rtRefT<pxObject> mRoot;
};

// For now just child scene objects
class pxScene: public pxObject {
public:
  rtDeclareObject(pxScene, pxObject);
  // we'd want to remove external access to this... 
//  rtReadOnlyProperty(innerScene, innerScene, rtObjectRef);
  rtProperty(url, url, setURL, rtString);

  pxScene() { mInnerScene = new pxInnerScene; }

#if 0
  rtError innerScene(rtObjectRef& v) const
  {
    v = mInnerScene;
    return RT_OK;
  }
#endif

  rtError url(rtString& v) const { v = mURL; return RT_OK; }
  rtError setURL(rtString v);

  virtual void update(double t)
  {
    mInnerScene->update(t);
  }

  virtual void drawInternal(pxMatrix4f m)
  {
    mInnerScene->drawInternal(m);
  }


private:
  rtRefT<pxInnerScene> mInnerScene;
  rtString mURL;
};

class rectangle: public pxObject {
public:
  rtDeclareObject(rectangle, pxObject);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(lineColor, lineColor, setLineColor, uint32_t);
  rtProperty(lineWidth, lineWidth, setLineWidth, float);

  rectangle() {
    mLineWidth = 0;
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
  }
  
  rtError fillColor(uint32_t& /*c*/) const {
    rtLog("fillColor not implemented");
    return RT_OK;
  }

  rtError setFillColor(uint32_t c) {
    mFillColor[0] = (float)((c>>24)&0xff)/255.0f;
    mFillColor[1] = (float)((c>>16)&0xff)/255.0f;
    mFillColor[2] = (float)((c>>8)&0xff)/255.0f;
    mFillColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineColor(uint32_t& /*c*/) const {
    rtLog("lineColor not implemented");
    return RT_OK;
  }

  rtError setLineColor(uint32_t c) {
    mLineColor[0] = (float)((c>>24)&0xff)/255.0f;
    mLineColor[1] = (float)((c>>16)&0xff)/255.0f;
    mLineColor[2] = (float)((c>>8)&0xff)/255.0f;
    mLineColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineWidth(float& w) const {
    w = mLineWidth;
    return RT_OK;
  }
  
  rtError setLineWidth(float w) {
    mLineWidth = w;
    return RT_OK;
  }

  // c is assumed to not be premultiplied
  void setFillColor(float* c) {
    mFillColor[0] = c[0];
    mFillColor[1] = c[1];
    mFillColor[2] = c[2];
    mFillColor[3] = c[3];
  }
  
  // c is assumed to not be premultiplied
  void setLineColor(float* c) {
    mLineColor[0] = c[0];
    mLineColor[1] = c[1];
    mLineColor[2] = c[2];
    mLineColor[3] = c[3];
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};

#if 0
class rectangle9: public pxObject {
public:
  rectangle9() {
    mLineWidth = 0;
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
  }
  
  void setFillColor(float* c) {
    mFillColor[0] = c[0];
    mFillColor[1] = c[1];
    mFillColor[2] = c[2];
    mFillColor[3] = c[3];
  }
  
  void setLineColor(float* c) {
    mLineColor[0] = c[0];
    mLineColor[1] = c[1];
    mLineColor[2] = c[2];
    mLineColor[3] = c[3];
  }
  
  void setLineWidth(float w) {
    mLineWidth = w;
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};
#endif

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

  rtError root(rtObjectRef& v) const {
    v = getRoot();
    return RT_OK;
  }
  
private:
//  void tick(double t);
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
  pxScene2dRef(pxScene2d* s) {
    asn(s);
  }

  // operator= is not inherited
  pxScene2dRef& operator=(pxScene2d* s) {
    asn(s);
    return *this;
  }
  
 private:
  virtual rtError Get(const char* name, rtValue* value);
  virtual rtError Set(const char* name, const rtValue* value);

};

#endif
