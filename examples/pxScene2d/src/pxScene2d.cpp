// pxCore CopyRight 2007-2015 John Robinson
// pxScene2d.cpp

#include "pxScene2d.h"


#include <math.h>

#include "rtLog.h"
#include "rtRefT.h"
#include "rtString.h"
#include "rtPathUtils.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxTimer.h"

#include "pxText.h"
#include "pxImage.h"

#include "pxContext.h"

pxContext context;

double pxInterpLinear(double i) {
  return pxClamp<double>(i, 0, 1);
}

void pxObject::setParent(rtRefT<pxObject>& parent) {
  mParent = parent;
  parent->mChildren.push_back(this);
}

void pxObject::animateTo(const char* prop, double to, double duration, 
			 pxInterp interp, pxAnimationType at, 
			 pxAnimationEnded e, void* c) 
{
  animation a;
  a.prop = prop;
  a.from = get<float>(prop);
  a.to = to;
  a.start = -1;
  a.duration = duration;
  a.interp = interp?interp:pxInterpLinear;
  a.at = at;
  a.ended = e;
  a.ctx = c;
  
  animation b;
  b = a;
  mAnimations.push_back(a);
}

void pxObject::update(double t) {
  // Update animations
  vector<animation>::iterator it = mAnimations.begin();
  while (it != mAnimations.end()) {
    animation& a = (*it);
    if (a.start < 0) a.start = t;
    double end = a.start + a.duration;
    
    // if duration has elapsed
    if (t >= end) {
      set(a.prop, a.to);
      if (a.at == stop) {
	if (a.ended)
	  a.ended(a.ctx);
	it = mAnimations.erase(it);
	continue;
      }
#if 0
      else if (a.at == seesaw) {
	// flip
	double t;
	t = a.from;
	a.from = a.to;
	a.to = t;
      }
#endif
    }
    
    double t1 = (t-a.start)/a.duration; // Some of this could be pushed into the end handling
    double t2 = floor(t1);
    t1 = t1-t2; // 0-1
    double d = a.interp(t1);
    float from, to;
    from = a.from;
    to = a.to;
    if (a.at == seesaw) {
      if (fmod(t2,2) != 0) {  // perf chk ?
	from = a.to;
	to = a.from;
      }
    }
    
    float v = from + (to - from) * d;
      set(a.prop, v);
      ++it;
  }
  
  // Recursively update children
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    (*it)->update(t);
  }
}

void pxObject::drawInternal(pxMatrix4f m) {

  m.translate(mx+mcx, my+mcy);

  m.rotateInDegrees(mr, mrx, mry, mrz);
  m.scale(msx, msy);
  m.translate(-mcx, -mcy);
  
  context.setMatrix(m);
  context.setAlpha(ma);

  draw();
  
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it) {
    (*it)->drawInternal(m);
  }
}

rtDefineObject(pxObject, rtObject);
rtDefineProperty(pxObject, _pxObject);
rtDefineProperty(pxObject, parent);
rtDefineProperty(pxObject, x);
rtDefineProperty(pxObject, y);
rtDefineProperty(pxObject, w);
rtDefineProperty(pxObject, h);
rtDefineProperty(pxObject, cx);
rtDefineProperty(pxObject, cy);
rtDefineProperty(pxObject, sx);
rtDefineProperty(pxObject, sy);
rtDefineProperty(pxObject, a);
rtDefineProperty(pxObject, r);
rtDefineProperty(pxObject, rx);
rtDefineProperty(pxObject, ry);
rtDefineProperty(pxObject, rz);
rtDefineMethod(pxObject, animateTo);

rtDefineObject(rectangle, pxObject);
rtDefineProperty(rectangle, fillColor);
rtDefineProperty(rectangle, lineColor);
rtDefineProperty(rectangle, lineWidth);

void rectangle::draw() {
  context.drawRect(mw, mh, mLineWidth, mFillColor, mLineColor);
}

void rectangle9::draw() {
  context.drawRect9(mw, mh, mLineWidth, mFillColor, mLineColor);  
}
  
pxScene2d::pxScene2d():start(0),frameCount(0) { 
  mRoot = new pxObject(); 
}

void pxScene2d::init() {
  context.init();
}


rtError pxScene2d::createRectangle(rtObjectRef& o) {
  o = new rectangle;
  return RT_OK;
}

rtError pxScene2d::createText(rtObjectRef& o) {
  o = new pxText;
  return RT_OK;
}

rtError pxScene2d::createImage(rtObjectRef& o) {
  o = new pxImage;
  return RT_OK;
}

void pxScene2d::draw() {
  context.clear(mWidth, mHeight);
  
  if (mRoot) {
    pxMatrix4f m;
    mRoot->drawInternal(m);
  }
}
  
void pxScene2d::getMatrixFromObjectToScene(pxObject* /*o*/, pxMatrix4f& /*m*/) {
    
}
  
void pxScene2d::getMatrixFromSceneToObject(pxObject* /*o*/, pxMatrix4f& /*m*/) {
    
}
  
void pxScene2d::getMatrixFromObjectToObject(pxObject* /*from*/, pxObject* /*to*/, pxMatrix4f& /*m*/) {
    
}
  
void pxScene2d::transformPointFromObjectToScene(pxObject* /*o*/, const pxPoint2f& /*from*/, pxPoint2f& /*to*/) 
{
    
}
  
void pxScene2d::transformPointFromObjectToObject(pxObject* /*fromObject*/, pxObject* /*toObject*/, 
						 pxPoint2f& /*from*/, pxPoint2f& /*to*/) {
  
}
  
void pxScene2d::hitTest(pxPoint2f /*p*/, vector<rtRefT<pxObject> > /*hitList*/) {
    
}

void pxScene2d::onDraw() {
  if (start == 0)
    start = pxSeconds();
  
  update(pxSeconds());
  draw();
  if (frameCount >= 60) {
    end2 = pxSeconds();
    rtLog("elapsed no clip %g\n", (double)frameCount/(end2-start));
    start = end2;
    frameCount = 0;
  }
  frameCount++;
}

// Does not draw updates scene to time t
// t is assumed to be monotonically increasing
void pxScene2d::update(double t) {
  if (mRoot)
    mRoot->update(t);
}

pxObject* pxScene2d::getRoot() const { 
  return mRoot; 
}

int pxScene2d::width() {
  return mWidth;
}

int pxScene2d::height() {
  return mHeight;
}

void pxScene2d::onSize(int w, int h) {
  //  glViewport(0, 0, (GLint)w, (GLint)h);
  context.setSize(w, h);
  mWidth = w;
  mHeight = h;
}

void pxScene2d::onMouseDown(int /*x*/, int /*y*/, unsigned long /*flags*/) {
}

void pxScene2d::onMouseUp(int /*x*/, int /*y*/, unsigned long /*flags*/) {
}

void pxScene2d::onMouseLeave() {
}

void pxScene2d::onMouseMove(int /*x*/, int /*y*/) {
#if 0
  rtLog("onMousePassiveMotion x: %d y: %d\n", x, y);
  
  //  pxMatrix4f m1, m2;
  pxVector4f from(x, y);
  pxVector4f to;
  
  pxObject::transformPointFromSceneToObject(target, from, to);
  
  rtLog("in target coords x: %f y: %f\n", to.mX, to.mY);
  
  pxObject::transformPointFromObjectToScene(target, to, from);
  
  rtLog("in sceme coords x: %f y: %f\n", from.mX, from.mY);
#endif
}

void pxScene2d::onKeyDown(int /*keycode*/, unsigned long /*flags*/) {
}

void pxScene2d::onKeyUp(int /*keycode*/, unsigned long /*flags*/) {
}

rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineMethod(pxScene2d, createRectangle);
rtDefineMethod(pxScene2d, createText);
rtDefineMethod(pxScene2d, createImage);

rtError pxScene2dRef::Get(const char* name, rtValue* value) {
  return (*this)->Get(name, value);
}
 
rtError pxScene2dRef::Set(const char* name, const rtValue* value) {
  return (*this)->Set(name, value);
}
