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

#include "pxMatrix4T.h"

typedef double (*pxInterp)(double i);
typedef void (*pxAnimationEnded)(void* ctx);

double pxInterpLinear(double i);

enum pxAnimationType {seesaw, loop, stop};

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
  pxAnimationEnded ended;
  void* ctx;
};

class pxObject{
public:
  pxObject(): cx(0), cy(0), x(0), y(0), a(1.0), r(0), sx(1), sy(1), mRef(0) {}
  virtual ~pxObject() { /*printf("pxObject destroyed\n");*/ }
  virtual unsigned long AddRef() { return ++mRef; }
  virtual unsigned long Release() { if (--mRef == 0) delete this; return mRef; }
  
  void setParent(rtRefT<pxObject>& parent);
  void remove();
  
  void set(const char* prop, float v);
  float get(const char* prop) const;
  
  void moveToFront();
  void moveToBack();
  void moveForward();
  void moveBackward();
  void animateTo(char* prop, pxInterp i, double duration, pxAnimationType t, 
		 pxAnimationEnded e = 0, void* c = 0);
  void tick(double t);
  virtual void drawInternal(pxMatrix4f m);
  virtual void draw() {}
  bool hitTest(const pxPoint2f& pt);
  
  void animateTo(const char* prop, double to, double duration, 
		 pxInterp interp=0, pxAnimationType at=stop, 
		 pxAnimationEnded e = 0, void* c = 0);  
  void update(double t);

  static void getMatrixFromObjectToScene(pxObject* o, pxMatrix4f& m) {
    m.identity();
    rtRefT<pxObject> t = o;
    while(t) {
      m.translate(t->x, t->y);
      m.scale(t->sx, t->sy);
      m.rotateZInDegrees(t->r);
      m.translate(-t->cx, -t->cy);
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

  float cx, cy, x, y, a, r, sx, sy, w, h;

private:
  rtRefT<pxObject> mParent;
  vector<rtRefT<pxObject> > mChildren;
  vector<animation> mAnimations;
  unsigned long mRef;
};

typedef rtRefT<pxObject> pxObjectRef;


class rectangle: public pxObject {
public:
  rectangle() {
    mLineWidth = 0;
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
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
  
  void setLineWidth(float w) {
    mLineWidth = w;
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};

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

class pxScene2d {
public:
  pxScene2d();
  
  int width();
  int height();
  
  // The following methods are delegated to the view
  virtual void onSize(int w, int h);
  virtual void onMouseDown(int x, int y, unsigned long flags);
  virtual void onMouseUp(int x, int y, unsigned long flags);
  virtual void onMouseLeave();
  virtual void onMouseMove(int x, int y);
  
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
  
  pxObject* getRoot();
  
private:
  void tick(double t);
  void draw();
  // Does not draw updates scene to time t
  // t is assumed to be monotonically increasing
  void update(double t);
  

  rtRefT<pxObject> mRoot;
  double start, end2;
  int frameCount;
  int mWidth;
  int mHeight;
};

#endif
