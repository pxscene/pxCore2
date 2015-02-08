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
#include "pxWindowUtil.h"

#include "pxRectangle.h"
#include "pxText.h"
#include "pxImage.h"
#include "pxImage9.h"

#include "pxContext.h"

// TODO get rid of globals
pxContext context;
rtFunctionRef gOnScene;

pxInterp interps[] = 
{
  pxInterpLinear,
  easeOutElastic,
  easeOutBounce,
  pxExp,
  pxStop,
};
int numInterps = sizeof(interps)/sizeof(interps[0]);

double pxInterpLinear(double i)
{
  return pxClamp<double>(i, 0, 1);
}

void pxObject::setParent(rtRefT<pxObject>& parent)
{
  mParent = parent;
  parent->mChildren.push_back(this);
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
    for(vector<rtRefT<pxObject> >::iterator it = mParent->mChildren.begin(); 
        it != mParent->mChildren.end(); ++it)
    {
      if ((it)->getPtr() == this)
      {
        mParent->mChildren.erase(it);
        return RT_OK;
      }
    }
  }
  return RT_OK;
}

rtError pxObject::removeAll()
{
  mChildren.clear();
  return RT_OK;
}

#if 0
rtError pxObject::animateTo(const char* prop, double to, double duration, 
                            uint32_t interp, uint32_t animationType) 
{
  interp = pxClamp<uint32_t>(interp, 0, numInterps-1);
  animateTo(prop, to, duration, interps[interp], 
            (pxAnimationType)animationType);
  return RT_OK;
  }
#endif

#if 1
rtError pxObject::animateTo(const char* prop, double to, double duration, 
                             uint32_t interp, uint32_t animationType, 
                             rtFunctionRef onEnd) 
{
  interp = pxClamp<uint32_t>(interp, 0, numInterps-1);
  animateTo(prop, to, duration, interps[interp], 
            (pxAnimationType)animationType, onEnd);
  return RT_OK;
}
#endif

#if 0
void pxObject::animateTo(const char* prop, double to, double duration, 
                         pxInterp interp, pxAnimationType at,
                         pxAnimationEnded e, void* c)
#else
void pxObject::animateTo(const char* prop, double to, double duration, 
                         pxInterp interp, pxAnimationType at,
                         rtFunctionRef onEnd)
#endif
{
  animation a;

  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = to;
  a.start    = -1;
  a.duration = duration;
  a.interp   = interp?interp:pxInterpLinear;
  a.at       = at;
#if 0
  a.ended    = e;
  a.ctx      = c;
#else
  a.ended = onEnd;
#endif
  
  animation b;
  b = a;
  mAnimations.push_back(a);
}

void pxObject::update(double t)
{
  // Update animations
  vector<animation>::iterator it = mAnimations.begin();

  while (it != mAnimations.end())
  {
    animation& a = (*it);

    if (a.start < 0) a.start = t;
    double end = a.start + a.duration;
    
    // if duration has elapsed
    if (t >= end)
    {

      set(a.prop, a.to);

      if (a.at == PX_STOP)
      {
        if (a.ended)
        {
#if 0
          a.ended(a.ctx);
#else
          a.ended.send(this);
#endif
        }

        // Erase making sure to push the iterator forward before
        it = mAnimations.erase(it++);
        continue;
      }
#if 0
      else if (a.at == PX_SEESAW)
      {
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
    if (a.at == PX_SEESAW)
    {
      if (fmod(t2,2) != 0)   // perf chk ?
      {
        from = a.to;
        to   = a.from;
      }
    }
    
    float v = from + (to - from) * d;
    set(a.prop, v);
    ++it;
  }
  
  // Recursively update children
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->update(t);
  }
}

void pxObject::drawInternal(pxMatrix4f m, float parentAlpha)
{
  // TODO not propogating parent alpha
  // TODO cull when alpha is near zero
  // TODO what to do about multiple vanishing points in a given scene
  // TODO consistent behavior between clipping and no clipping when z is in use

#if 1
  // translate based on xy rotate/scale based on cx, cy
  m.translate(mx+mcx, my+mcy);

//  Only allow z rotation until we can reconcile multiple vanishing point thoughts
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m.rotateInDegrees(mr, 0, 0, 1);
  m.scale(msx, msy);
  
  m.translate(-mcx, -mcy);

#else
  // translate/rotate/scale based on cx, cy
  m.translate(mx, my);

//  Only allow z rotation until we can reconcile multiple vanishing point thoughts
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m.rotateInDegrees(mr, 0, 0, 1);
  m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#endif

#if 0

  printf("drawInternal: %s\n", mId.cString());
  m.dump();

  pxVector4f v1(mx+mw, my, 0, 1);
  printf("Print vector top\n");
  v1.dump();

  pxVector4f result1 = m.multiply(v1);
  printf("Print vector top after\n");
  result1.dump();


  pxVector4f v2(mx+mw, my+mh, 0, 1);
  printf("Print vector bottom\n");
  v2.dump();

  pxVector4f result2 = m.multiply(v2);
  printf("Print vector bottom after\n");
  result2.dump();
  
#endif

  parentAlpha = parentAlpha * ma;

  context.setMatrix(m);
  context.setAlpha(parentAlpha);
  
  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, mw, mh, c);

  if (mPainting)
  {
    if (mClip || mMaskUrl.length() > 0)
    {
      pxTextureRef snapshot = createSnapshot();
      context.setMatrix(m);
      context.setAlpha(ma);
      context.drawImage(mw,mh, snapshot, mMaskTextureRef, PX_NONE, PX_NONE);
      deleteSnapshot(snapshot);
    }
    else
    {
      draw();

      for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        (*it)->drawInternal(m, parentAlpha);
      }
    }
  }
  else
  {
    context.drawImage(mw,mh, mTextureRef, mMaskTextureRef, PX_NONE, PX_NONE);
  }
}

pxTextureRef pxObject::createSnapshot()
{
  pxMatrix4f m;

  float parentAlpha = ma;

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  pxTextureRef texture = context.createContextSurface(mw, mh);
  pxTextureRef previousRenderSurface = context.getCurrentRenderSurface();
  if (context.setRenderSurface(texture) == PX_OK)
  {
    context.clear(mw, mh);
    draw();

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      (*it)->drawInternal(m, parentAlpha);
    }
  }
  context.setRenderSurface(previousRenderSurface);
  
  return texture;
}

void pxObject::deleteSnapshot(pxTextureRef texture)
{
  if (texture.getPtr() != NULL)
  {
    texture->deleteTexture();
  }
}

void pxObject::createMask()
{
  deleteMask();
  
  if (mMaskUrl.length() > 0)
  {
    pxImage image;
    image.setURL(mMaskUrl.cString());
    mMaskTextureRef = context.createMask(image.getTexture());
  }
}

void pxObject::deleteMask()
{
  if (mMaskTextureRef.getPtr() != NULL)
  {
    mMaskTextureRef->deleteTexture();
  }
}

rtDefineObject(pxObject, rtObject);
rtDefineProperty(pxObject, _pxObject);
rtDefineProperty(pxObject, parent);
rtDefineProperty(pxObject, children);
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
rtDefineProperty(pxObject, id);
rtDefineProperty(pxObject, painting);
rtDefineProperty(pxObject, clip);
rtDefineProperty(pxObject, mask);
rtDefineProperty(pxObject, numChildren);
rtDefineMethod(pxObject, getChild);
rtDefineMethod(pxObject, remove);
//rtDefineMethod(pxObject, animateTo);
rtDefineMethod(pxObject, animateTo2);

pxScene2d::pxScene2d()
 :start(0),frameCount(0) 
{ 
  mRoot = new pxObject(); 
  mEmit = new rtEmit();
}

void pxScene2d::init()
{
  // TODO move this to the window
  context.init();
}

rtError pxScene2d::createRectangle(rtObjectRef& o)
{
  o = new pxRectangle;
  return RT_OK;
}

rtError pxScene2d::createText(rtObjectRef& o)
{
  o = new pxText;
  return RT_OK;
}

rtError pxScene2d::createImage(rtObjectRef& o)
{
  o = new pxImage;
  return RT_OK;
}

rtError pxScene2d::createImage9(rtObjectRef& o)
{
  o = new pxImage9;
  return RT_OK;
}

rtError pxScene2d::createScene(rtObjectRef& o)
{
  o = new pxScene();
  return RT_OK;
}

void pxScene2d::draw()
{
  context.clear(mWidth, mHeight);
  
  if (mRoot)
  {
    pxMatrix4f m;
    mRoot->drawInternal(m, 1.0);
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

void pxScene2d::onDraw()
{
  if (start == 0)
  {
    start = pxSeconds();
  }
  
#if 1
  update(pxSeconds());
  draw();
#endif

  if (frameCount >= 60)
  {
    end2 = pxSeconds();

    // I want this always on for now
    printf("%f fps\n", (double)frameCount/(end2-start));

    start = end2;
    frameCount = 0;
  }

  frameCount++;
}

// Does not draw updates scene to time t
// t is assumed to be monotonically increasing
void pxScene2d::update(double t)
{
  if (mRoot)
  {
    mRoot->update(t);
  }
}

pxObject* pxScene2d::getRoot() const
{
  return mRoot;
}

void pxScene2d::onSize(int w, int h)
{
  //  glViewport(0, 0, (GLint)w, (GLint)h);
  context.setSize(w, h);

  mWidth  = w;
  mHeight = h;

  mEmit.send("resize", w, h);
}

void pxScene2d::onMouseDown(int x, int y, unsigned long flags)
{
  mEmit.send("mousedown", x, y, (uint64_t)flags);
}

void pxScene2d::onMouseUp(int x, int y, unsigned long flags)
{
  mEmit.send("mouseup", x, y, (uint64_t)flags);
}

void pxScene2d::onMouseLeave()
{
  mEmit.send("mouseleave");
}

void pxScene2d::onMouseMove(int x, int y)
{
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
  mEmit.send("mousemove", x, y);
}

void pxScene2d::onKeyDown(int keycode, unsigned long flags) 
{
  printf("pxScene2d::onKeyDown %d\n", keycode);
  mEmit.send("keydown", keycodeFromNative(keycode), (uint64_t)flags);
}

void pxScene2d::onKeyUp(int keycode, unsigned long flags)
{
  mEmit.send("keyup", keycodeFromNative(keycode), (uint64_t)flags);
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

rtError pxScene2d::onScene(rtFunctionRef& v) const 
{ 
  v = gOnScene; 
  return RT_OK; 
}

rtError pxScene2d::setOnScene(rtFunctionRef v) 
{ 
  gOnScene = v; 
  return RT_OK; 
}

rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineProperty(pxScene2d, onScene);
rtDefineProperty(pxScene2d, w);
rtDefineProperty(pxScene2d, h);
rtDefineProperty(pxScene2d, showOutlines);
rtDefineMethod(pxScene2d, createRectangle);
rtDefineMethod(pxScene2d, createText);
rtDefineMethod(pxScene2d, createImage);
rtDefineMethod(pxScene2d, createImage9);
rtDefineMethod(pxScene2d, createScene);
rtDefineMethod(pxScene2d, addListener);
rtDefineMethod(pxScene2d, delListener);

rtError pxScene::setURL(rtString v) 
{ 
  // Release old root from parent scene perspective
  removeAll();
  mInnerScene = NULL;
  
  // TODO.. should we be able to do if (mURL)?
  if (!mURL.isEmpty())
    rtLogInfo("pxScene::setURL(), Unloading scene %s\n", mURL.cString());
  mURL = v; 
  if (!mURL.isEmpty())
  {
    rtLogInfo("pxScene::setURL(), Loading scene %s\n", mURL.cString());
    // Add new root into scene
    rtRefT<pxObject> newRoot  = new pxObject;
    mChildren.push_back(newRoot);
    mInnerScene = new pxInnerScene(newRoot); 
    mInnerScene->setW(mw);
    mInnerScene->setH(mh);
    if (gOnScene)
    {
      // TODO experiment always set painting to false initially with the expectation that
      // load.js will set painting = true after initial js has executed to reduce flicker and
      // transition
      setPainting(false);
      //TODO double check can this hang on the js side
      gOnScene.send((rtObject*)this, mInnerScene.getPtr(), mURL);
    }
  }
  else
    rtLogDebug("pxScene::setURL, null url\n");
  return RT_OK; 
}

rtDefineObject(pxScene, pxObject);
//rtDefineProperty(pxScene, innerScene);
rtDefineProperty(pxScene, url);
rtDefineProperty(pxScene, w);
rtDefineProperty(pxScene, h);

rtDefineObject(pxInnerScene, rtObject);
rtDefineProperty(pxInnerScene, root);
rtDefineProperty(pxInnerScene, w);
rtDefineProperty(pxInnerScene, h);
rtDefineProperty(pxInnerScene, showOutlines);
rtDefineMethod(pxInnerScene, createRectangle);
rtDefineMethod(pxInnerScene, createText);
rtDefineMethod(pxInnerScene, createImage);
rtDefineMethod(pxInnerScene, createImage9);
rtDefineMethod(pxInnerScene, createScene);
rtDefineMethod(pxInnerScene, addListener);
rtDefineMethod(pxInnerScene, delListener);
rtDefineProperty(pxInnerScene, ctx);

rtError pxInnerScene::showOutlines(bool& v) const 
{ 
  v=context.showOutlines(); 
  return RT_OK;
}

rtError pxInnerScene::setShowOutlines(bool v) 
{ 
  context.setShowOutlines(v); 
  return RT_OK; 
}

rtError pxInnerScene::createRectangle(rtObjectRef p, rtObjectRef& o)
{
  o = new pxRectangle;
  o.set(p);
  return RT_OK;
}

rtError pxInnerScene::createText(rtObjectRef p, rtObjectRef& o)
{
  o = new pxText;
  o.set(p);
  return RT_OK;
}

rtError pxInnerScene::createImage(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage;
  o.set(p);
  return RT_OK;
}

rtError pxInnerScene::createImage9(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage9;
  o.set(p);
  return RT_OK;
}

rtError pxInnerScene::createScene(rtObjectRef p, rtObjectRef& o)
{
  o = new pxScene();
  o.set(p);
  return RT_OK;
}

rtError pxScene2dRef::Get(const char* name, rtValue* value)
{
  return (*this)->Get(name, value);
}

rtError pxScene2dRef::Get(uint32_t i, rtValue* value)
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
