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

// pxObject.h

#define ANIMATION_ROTATE_XYZ

#ifndef PX_OBJECT_H
#define PX_OBJECT_H

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

#include "pxMatrix4T.h"
#include "pxInterpolators.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"


#define ANIMATION_ROTATE_XYZ

#include "pxCore.h"
#include "pxAnimate.h"

struct pxPoint2f; //fwd
class pxScene2d;  //fwd

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
  rtProperty(a,  a,  setA,  float);
  rtProperty(r,  r,  setR,  float);

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

  rtMethod5ArgAndReturn("animateTo", animateToP2, rtObjectRef, rtValue,
                        uint32_t, uint32_t, int32_t, rtObjectRef);

  rtMethod5ArgAndReturn("animate", animateToObj, rtObjectRef, rtValue,
                        uint32_t, uint32_t, int32_t, rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);
  rtMethodNoArgAndNoReturn("dispose",releaseResources);
 // rtProperty(onReady, onReady, setOnReady, rtFunctionRef);

//  rtReadOnlyProperty(emit, emit, rtFunctionRef);
  rtMethod1ArgAndReturn("getObjectById",getObjectById,rtString,rtObjectRef);

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

  virtual void onInit();


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
  virtual rtError setW(float v)       { cancelAnimation("w"); mw = v; return RT_OK;   }
  float h()             const { return mh; }
  rtError h(float& v)   const { v = mh; return RT_OK;   }
  virtual rtError setH(float v)       { cancelAnimation("h"); mh = v; return RT_OK;   }

  float px()            const { return mpx;}
  rtError px(float& v)  const { v = mpx; return RT_OK;  }
  rtError setPX(float v)      { cancelAnimation("px"); mpx = (v > 1) ? 1 : (v < 0) ? 0 : v; return RT_OK;  }
  float py()            const { return mpy;}
  rtError py(float& v)  const { v = mpy; return RT_OK;  }
  rtError setPY(float v)      { cancelAnimation("py"); mpy = (v > 1) ? 1 : (v < 0) ? 0 : v; return RT_OK;  }
  float cx()            const { return mcx;}
  rtError cx(float& v)  const { v = mcx; return RT_OK;  }
  rtError setCX(float v)      { cancelAnimation("cx"); mcx = v; return RT_OK;  }
  float cy()            const { return mcy;}
  rtError cy(float& v)  const { v = mcy; return RT_OK;  }
  rtError setCY(float v)      { cancelAnimation("cy"); mcy = v; return RT_OK;  }
  float sx()            const { return msx;}
  rtError sx(float& v)  const { v = msx; return RT_OK;  }
  virtual rtError setSX(float v) { cancelAnimation("sx"); msx = v; return RT_OK;  }
  float sy()            const { return msy;}
  rtError sy(float& v)  const { v = msy; return RT_OK;  }
  virtual rtError setSY(float v) { cancelAnimation("sy"); msy = v; return RT_OK;  }
  float a()             const { return ma; }
  rtError a(float& v)   const { v = ma; return RT_OK;   }
  rtError setA(float v)       { cancelAnimation("a"); ma = v; return RT_OK;   }
  float r()             const { return mr; }
  rtError r(float& v)   const { v = mr; return RT_OK;   }
  rtError setR(float v)       { cancelAnimation("r"); mr = v; return RT_OK;   }
#ifdef ANIMATION_ROTATE_XYZ
  float rx()            const { return mrx;}
  rtError rx(float& v)  const { v = mrx; return RT_OK;  }
  rtError setRX(float v)      { cancelAnimation("rx");  mrx = v; return RT_OK;  }
  float ry()            const { return mry;}
  rtError ry(float& v)  const { v = mry; return RT_OK;  }
  rtError setRY(float v)      { cancelAnimation("ry"); mry = v; return RT_OK;  }
  float rz()            const { return mrz;}
  rtError rz(float& v)  const { v = mrz; return RT_OK;  }
  rtError setRZ(float v)      { cancelAnimation("rz"); mrz = v; return RT_OK;  }
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

  bool hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRef<pxObject>& hit, pxPoint2f& hitPt);
  virtual bool hitTest(pxPoint2f& pt);

  void setFocusInternal(bool focus) { mFocus = focus; }

  rtError animateTo(const char* prop, double to, double duration,
                     uint32_t interp, uint32_t options,
                     int32_t count, rtObjectRef promise);

  rtError animateToP2(rtObjectRef props, rtValue duration,
                      uint32_t interp, uint32_t options,
                      int32_t count, rtObjectRef& promise);

  rtError animateToObj(rtObjectRef props, rtValue duration,
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

  virtual void update(double t, bool updateChildren=true);
  virtual void releaseData(bool sceneSuspended);
  virtual void reloadData(bool sceneSuspended);
  virtual uint64_t textureMemoryUsage(std::vector<rtObject*> &objectsCounted);

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

  void repaint() { mRepaint = true; }

  rtError releaseResources()
  {
     dispose(true);
     return RT_OK;
  }

  virtual bool needsUpdate();

  pxScene2d* getScene() { return mScene; }
  void createSnapshot(pxContextFramebufferRef& fbo, bool separateContext=false, bool antiAliasing=false);

public:
  rtEmitRef mEmit;

protected:
  void triggerUpdate();
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
  bool mRepaint;
  //#ifdef PX_DIRTY_RECTANGLES
  bool mIsDirty;
  pxMatrix4f mRenderMatrix;
  pxMatrix4f mLastRenderMatrix;
  pxRect mScreenCoordinates;
  pxRect mDirtyRect;
  //#endif //PX_DIRTY_RECTANGLES

  void createSnapshotOfChildren();
  void clearSnapshot(pxContextFramebufferRef fbo);
  // #ifdef PX_DIRTY_RECTANGLES
  void setDirtyRect(pxRect* r);
  void markDirty();
  pxRect getBoundingRectInScreenCoordinates();
  pxRect convertToScreenCoordinates(pxRect* r);
  // #endif //PX_DIRTY_RECTANGLES

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


#endif // PX_OBJECT_H
