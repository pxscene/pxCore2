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
#include "pxFileDownloader.h"
#include "rtMutex.h"

// TODO get rid of globals
pxContext context;
rtFunctionRef gOnScene;

int gFileDownloadsPending = 0; //must only be set in the main thread

rtMutex fileDownloadMutex;
bool fileDownloadsAvailable = false;
vector<pxFileDownloadRequest*> completedFileDownloads;

void pxFileDownloadComplete(pxFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL)
  {
    fileDownloadMutex.lock();
    completedFileDownloads.push_back(fileDownloadRequest);
    fileDownloadsAvailable = true;
    fileDownloadMutex.unlock();
  }
}


#if 0
pxInterp interps[] = 
{
  pxInterpLinear,
  easeOutElastic,
  easeOutBounce,
  pxExp,
  pxStop,
};
int numInterps = sizeof(interps)/sizeof(interps[0]);
#else

struct _pxInterpEntry
{
  const char* n;
  pxInterp i;
};
_pxInterpEntry interps[] = 
{
  {"PX_LINEAR", pxInterpLinear},
  {"PX_EXP1", pxExp1},
  {"PX_EXP2", pxExp2},
  {"PX_EXP3", pxExp3},
  {"PX_STOP", pxStop},
  {"PX_INQUAD", pxInQuad},
  {"PX_INCUBIC", pxInCubic},
  {"PX_INBACK", pxInBack},
  {"PX_EASEINELASTIC", pxEaseInElastic},
  {"PX_EASEOUTELASTIC", pxEaseOutElastic},
  {"PX_EASEOUTBOUNCE", pxEaseOutBounce},
};
int numInterps = sizeof(interps)/sizeof(interps[0]);

#endif

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
  animateTo(prop, to, duration, interps[interp].i, 
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
  animateTo(prop, to, duration, interps[interp].i, 
            (pxAnimationType)animationType, onEnd);
  return RT_OK;
}
#endif

// Dont fastforward when calling from set* methods since that will
// recurse indefinitely and crash and we're going to change the value in
// the set* method anyway.
void pxObject::cancelAnimation(const char* prop, bool fastforward)
{
  // TODO we need to fix the threading issues so that we can call this from set*...
  // a different thread
  if (!fastforward)
    return;

  // If an animation for this property is in progress we cancel it here
  // we also fastforward the animation if it is of type PX_END
  vector<animation>::iterator it = mAnimations.begin();
  while (it != mAnimations.end())
  {
    animation& a = (*it);
    if (a.prop == prop)
    {
      if (a.at == PX_END)
      {
        // fastforward
        if (fastforward)
	  set(prop, a.to);
        if (a.ended)
        {
          a.ended.send(this);
        }
      }
      it = mAnimations.erase(it);
    }
    else
      ++it;
  }  

}

void pxObject::animateTo(const char* prop, double to, double duration, 
                         pxInterp interp, pxAnimationType at,
                         rtFunctionRef onEnd)
{
  cancelAnimation(prop, true);
  
  // schedule animation
  animation a;

  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = to;
  a.start    = -1;
  a.duration = duration;
  a.interp   = interp?interp:pxInterpLinear;
  a.at       = at;
  a.ended = onEnd;

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
      // TODO this sort of blows since this triggers another
      // animation traversal to cancel animations
      set(a.prop, a.to);

      if (a.at == PX_END)
      {
        if (a.ended)
        {
          a.ended.send(this);
        }

        // Erase making sure to push the iterator forward before
        it = mAnimations.erase(it);
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

const float alphaEpsilon = (1.0f/255.0f);

void pxObject::drawInternal(pxMatrix4f m, float parentAlpha)
{
  // TODO what to do about multiple vanishing points in a given scene
  // TODO consistent behavior between clipping and no clipping when z is in use

  parentAlpha = parentAlpha * ma;
  if (parentAlpha < alphaEpsilon)
    return;  // trivial reject for objects that are transparent

#if 1
#if 1
  // translate based on xy rotate/scale based on cx, cy
  m.translate(mx+mcx, my+mcy);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  //  m.rotateInDegrees(mr, mrx, mry, mrz);
  if (mr) m.rotateInDegrees(mr, 0, 0, 1);
  if (msx != 1.0f || msy != 1.0f) m.scale(msx, msy);
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


  context.setMatrix(m);
  context.setAlpha(parentAlpha);
  
  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, mw, mh, c);

  if (mPainting)
  {
    if (mClip || mMaskUrl.length() > 0)
    {
      mClipTextureRef = createSnapshot(mClipTextureRef);
      context.setMatrix(m);
      context.setAlpha(parentAlpha);
      context.drawImage(0,0,mw,mh, mClipTextureRef, mMaskTextureRef, PX_NONE, PX_NONE);
    }
    else
    {

      if (mw>0.0f && mh>0.0f)
        draw();

      for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        (*it)->drawInternal(m, parentAlpha);
      }
    }
  }
  else
  {
    context.drawImage(0,0,mw,mh, mTextureRef, mMaskTextureRef, PX_NONE, PX_NONE);
  }
}


bool pxObject::hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, 
			       pxPoint2f& hitPt)
{

  // setup matrix
  pxMatrix4f m2;
  m2.translate(mx+mcx, my+mcy);
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m2.rotateInDegrees(mr, 0, 0, 1);
  m2.scale(msx, msy);  
  m2.translate(-mcx, -mcy);
  m2.invert();
  m2.multiply(m);

  {
    for(vector<rtRefT<pxObject> >::reverse_iterator it = mChildren.rbegin(); it != mChildren.rend(); ++it)
    {
      if ((*it)->hitTestInternal(m2, pt, hit, hitPt))
        return true;
    }
  }

  {
    // map pt to object coordinate space
    pxVector4f v(pt.x, pt.y, 0, 1);
    v = m2.multiply(v);
    pxPoint2f newPt;
    newPt.x = v.mX;
    newPt.y = v.mY;
    if (hitTest(newPt))
    {
      hit = this;
      hitPt = newPt;
      return true;
    }
    else
      return false;
  }
}

// TODO should we bother with pxPoint2f or just use pxVector4f
// pt is in object coordinates
bool pxObject::hitTest(pxPoint2f& pt)
{
  // default hitTest checks against object bounds (0, 0, w, h)
  // Can override for more interesting hit tests like alpha
//  printf("hitTest pt(x:%f,y%f) object(w:%f,h:%f)\n", v.mX, v.mY, mw, mh);
  return (pt.x >= 0 && pt.y >= 0 && pt.x <= mw && pt.y <= mh);
}



pxTextureRef pxObject::createSnapshot(pxTextureRef texture)
{
  pxMatrix4f m;

  float parentAlpha = ma;

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  if (texture.getPtr() == NULL || texture->width() != mw || texture->height() != mh)
  {
    texture = context.createContextSurface(mw, mh);
  }
  else
  {
    context.updateContextSurface(texture, mw, mh);
  }
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
    // TODO add a pxTexture object and use that instead of pxImage
    // TODO This only works if the image load is synchronous
    rtRefT<pxImage> i = new pxImage;
    i->send("init");
    i->setURL(mMaskUrl.cString());
    mMaskTextureRef = i->getTexture();
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
rtDefineMethod(pxObject, addListener);
rtDefineMethod(pxObject, delListener);
rtDefineProperty(pxObject, emit);
rtDefineProperty(pxObject, onReady);

pxScene2d::pxScene2d()
 :start(0),frameCount(0) 
{ 
  pxFileDownloader::getInstance()->setDefaultCallbackFunction(pxFileDownloadComplete);
  mRoot = new pxObject(); 
  mEmit = new rtEmit();
}

void pxScene2d::init()
{
  rtLogInfo("Object Sizes");
  rtLogInfo("============");
  rtLogInfo("pxObject     : %zu", sizeof(pxObject));
  rtLogInfo("pxImage      : %zu", sizeof(pxImage));
  rtLogInfo("pxImage9     : %zu", sizeof(pxImage9));
  rtLogInfo("pxRectangle  : %zu", sizeof(pxRectangle));
  rtLogInfo("pxText       : %zu", sizeof(pxText));

  // TODO move this to the window
  context.init();
}

rtError pxScene2d::createRectangle(rtObjectRef& o)
{
  o = new pxRectangle;
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createText(rtObjectRef& o)
{
  o = new pxText;
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage(rtObjectRef& o)
{
  o = new pxImage;
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage9(rtObjectRef& o)
{
  o = new pxImage9;
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createScene(rtObjectRef& o)
{
  o = new pxScene();
  o.send("init");
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

void pxScene2d::checkForCompletedFileDownloads()
{
  if (gFileDownloadsPending > 0)
  {
    fileDownloadMutex.lock();
    if (fileDownloadsAvailable)
    {
      for(vector<pxFileDownloadRequest*>::iterator it = completedFileDownloads.begin(); it != completedFileDownloads.end(); ++it)
      {
        pxFileDownloadRequest* fileDownloadRequest = (*it);

        if (fileDownloadRequest != NULL && 
            fileDownloadRequest->getDownloadStatusCode() == 0 && 
            fileDownloadRequest->getHttpStatusCode() == 200 &&
            fileDownloadRequest->getDownloadedData() != NULL)
        {
          pxOffscreen imageOffscreen;
          if (pxLoadImage(fileDownloadRequest->getDownloadedData(),
                          fileDownloadRequest->getDownloadedDataSize(), 
                          imageOffscreen) != RT_OK)
          {
            rtLogError("Image Decode Failed: %s", fileDownloadRequest->getFileURL().cString());
          }
          else
          {
            if (fileDownloadRequest->getCallbackData() != NULL)
            {
                pxImage* image = (pxImage*)fileDownloadRequest->getCallbackData();
                pxTextureRef imageTexture =  context.createTexture(imageOffscreen);
                image->setTexture(imageTexture);
            }

          }
        }
        else
          rtLogWarn("Image Download Failed: %s Error: %s HTTP Status Code: %ld", 
                    fileDownloadRequest->getFileURL().cString(),
                    fileDownloadRequest->getErrorString().cString(),
                    fileDownloadRequest->getHttpStatusCode());

        delete fileDownloadRequest;
        fileDownloadsAvailable = false;
        gFileDownloadsPending--;
      }
      completedFileDownloads.clear();
      if (gFileDownloadsPending < 0)
      {
        //this is a safety check (hopefully never used)
        //to ensure downloads are still processed in the event of a gFileDownloadsPending bug in the future
        gFileDownloadsPending = 0;
      }
    }
    fileDownloadMutex.unlock();
  }
}

void pxScene2d::onDraw()
{
  if (start == 0)
  {
    start = pxSeconds();
  }
  
#if 1
  checkForCompletedFileDownloads();
  update(pxSeconds());
  draw();
#endif

  if (frameCount >= 60)
  {
    end2 = pxSeconds();

    double fps = (double)frameCount/(end2-start);
    printf("%f fps\n", fps);
    // TODO FUTURES... might be nice to have "struct" style object's that get copied
    // at the interop layer so we don't get remoted calls back to the render thread
    // for accessing the values (events would be the primary usecase)
    rtObjectRef e = new rtMapObject;
    e.set("fps", fps);
    mEmit.send("onFPS", e);
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
  context.setSize(w, h);

  mWidth  = w;
  mHeight = h;

  rtObjectRef e = new rtMapObject;
  e.set("name", "onResize");
  e.set("w", w);
  e.set("h", h);
  mEmit.send("onResize", e);
}

void pxScene2d::onMouseDown(int x, int y, unsigned long flags)
{
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDown");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    mEmit.send("onMouseDown", e);
  }
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(x,y), hitPt;
    //    pt.x = x; pt.y = y;
    rtRefT<pxObject> hit;
    
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      rtString id = hit->get<rtString>("id");
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseDown");
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
      hit->mEmit.send("onMouseDown", e);
    }
  }
}

void pxScene2d::onMouseUp(int x, int y, unsigned long flags)
{
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseUp");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", static_cast<uint32_t>(flags));
    mEmit.send("onMouseUp", e);
  }
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(x,y), hitPt;
    pt.x = x; pt.y = y;
    rtRefT<pxObject> hit;
    
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseUp");
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
      hit->mEmit.send("onMouseUp", e);
    }
  }
}

void pxScene2d::onMouseLeave()
{
  mEmit.send("onMouseLeave");
}

void pxScene2d::onMouseMove(int x, int y)
{
  {
  // Send to root scene in global window coordinates
  rtObjectRef e = new rtMapObject;
  e.set("name", "onMouseMove");
  e.set("x", x);
  e.set("y", y);
  mEmit.send("onMouseMove", e);
  }
#if 0
  // This probably won't stay ... we can probably send onMouseMove to the child scene level
  // rather than the object... we can send objects enter/leave events
  // and we can send drag events to objects that are being drug... 
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt;
  pt.x = x; pt.y = y;
  rtRefT<pxObject> hit;

  if (mRoot->hitTestInternal(m, pt, hit))
  {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("data", "hello");
    hit->mEmit.send("onMouseMove",e);
  }
#endif
#if 0
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt;
  pt.x = x; pt.y = y;
  rtRefT<pxObject> hit;

  if (mRoot->hitTestInternal(m, pt, hit))
  {
    rtString id = hit->get<rtString>("id");
    printf("found object id: %s\n", id.isEmpty()?"none":id.cString());
  }
#endif
}

void pxScene2d::onKeyDown(int keyCode, unsigned long flags) 
{
  rtObjectRef e = new rtMapObject;
  e.set("name", "onKeyDown");
  e.set("keyCode", keyCode);
  e.set("flags", (uint32_t)flags);
  mEmit.send("onKeyDown",e);
}

void pxScene2d::onKeyUp(int keyCode, unsigned long flags)
{
  rtObjectRef e = new rtMapObject;
  e.set("name", "onKeyUp");
  e.set("keyCode", keyCode);
  e.set("flags",(uint32_t)flags);
  mEmit.send("onKeyUp",e);
}

//TODO not utf8 friendly
void pxScene2d::onChar(char c)
{
  // char buffer[32];
  //sprintf(buffer, "%c", c);
  rtObjectRef e = new rtMapObject;
  e.set("name", "onChar");
  //e.set("char", buffer);
  e.set("charCode", (uint32_t)c);
  mEmit.send("onChar",e);
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
    rtLogInfo("pxScene::setURL(), Unloading scene %s", mURL.cString());
  mURL = v; 
  if (!mURL.isEmpty())
  {
    rtLogInfo("pxScene::setURL(), Loading scene %s", mURL.cString());
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
    rtLogDebug("pxScene::setURL, null url");
  return RT_OK; 
}

rtDefineObject(pxScene, pxObject);
//rtDefineProperty(pxScene, innerScene);
rtDefineProperty(pxScene, url);
rtDefineProperty(pxScene, w);
rtDefineProperty(pxScene, h);
rtDefineProperty(pxScene, emit);

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
rtDefineProperty(pxInnerScene, emit);
rtDefineProperty(pxInnerScene, allInterpolators);
rtDefineProperty(pxInnerScene, PX_LINEAR);
rtDefineProperty(pxInnerScene, PX_EXP1);
rtDefineProperty(pxInnerScene, PX_EXP2);
rtDefineProperty(pxInnerScene, PX_EXP3);
rtDefineProperty(pxInnerScene, PX_STOP);
rtDefineProperty(pxInnerScene, PX_INQUAD);
rtDefineProperty(pxInnerScene, PX_INCUBIC);
rtDefineProperty(pxInnerScene, PX_INBACK);
rtDefineProperty(pxInnerScene, PX_EASEINELASTIC);
rtDefineProperty(pxInnerScene, PX_EASEOUTELASTIC);
rtDefineProperty(pxInnerScene, PX_EASEOUTBOUNCE);
rtDefineProperty(pxInnerScene, PX_END);
rtDefineProperty(pxInnerScene, PX_SEESAW);
rtDefineProperty(pxInnerScene, PX_LOOP);
rtDefineProperty(pxInnerScene, PX_NONE);
rtDefineProperty(pxInnerScene, PX_STRETCH);
rtDefineProperty(pxInnerScene, PX_REPEAT);


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
  o.send("init");
  return RT_OK;
}

rtError pxInnerScene::createText(rtObjectRef p, rtObjectRef& o)
{
  o = new pxText;
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxInnerScene::createImage(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage;
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxInnerScene::createImage9(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage9;
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxInnerScene::createScene(rtObjectRef p, rtObjectRef& o)
{
  o = new pxScene();
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxInnerScene::allInterpolators(rtObjectRef& v) const
{
  rtRefT<rtArrayObject> keys = new rtArrayObject;

  for (int i = 0; i < numInterps; i++)
  {
    keys->pushBack(interps[i].n);
  }
  v = keys;
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
