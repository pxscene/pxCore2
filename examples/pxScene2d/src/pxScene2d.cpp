// pxCore Copyright 2007-2015 John Robinson
// pxScene2d.cpp

#include "pxScene2d.h"

#include <math.h>
#include <assert.h>

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
#include "pxFont.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxImage.h"
#include "pxImage9.h"

#include "pxContext.h"
#include "pxFileDownloader.h"
#include "rtMutex.h"

#include "pxIView.h"

rtThreadQueue gUIThreadQueue;

// TODO move to rt*
// Taken from
// http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

#include <stdint.h>
#include <stdlib.h>


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};
static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table() {

  decoding_table = (char*)malloc(256);

    for (int i = 0; i < 64; i++)
        decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup() {
    free(decoding_table);
}

char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length) {

    *output_length = 4 * ((input_length + 2) / 3);

    char *encoded_data = (char *)malloc(*output_length);
    if (encoded_data == NULL) return NULL;

    for (uint32_t i = 0, j = 0; i < input_length;) 
    {

        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
    }

    for (int i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[*output_length - 1 - i] = '=';

    return encoded_data;
}


unsigned char *base64_decode(const unsigned char *data,
                             size_t input_length,
                             size_t *output_length) {

    if (decoding_table == NULL) build_decoding_table();

    if (input_length % 4 != 0) return NULL;

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char *decoded_data = (unsigned char*)malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (uint32_t i = 0, j = 0; i < input_length;) {

        uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
        uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

        uint32_t triple = (sextet_a << 3 * 6)
        + (sextet_b << 2 * 6)
        + (sextet_c << 1 * 6)
        + (sextet_d << 0 * 6);

        if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
    }

    return decoded_data;
}

// TODO get rid of globals
pxContext context;
rtFunctionRef gOnScene;

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

#ifdef PX_DIRTY_RECTANGLES
pxRect pxScene2d::mDirtyRect;
#endif //PX_DIRTY_RECTANGLES

double pxInterpLinear(double i)
{
  return pxClamp<double>(i, 0, 1);
}

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


// pxObject methods
void pxObject::sendPromise()
{ 
  if(mInitialized && !((rtPromise*)mReady.getPtr())->status())
  {
    mReady.send("resolve",this); 
  }
}

void pxObject::createNewPromise()
{ 
  // Only create a new promise if the existing one has been 
  // resolved or rejected already.
  if(((rtPromise*)mReady.getPtr())->status()) 
  {
    rtLogDebug("CREATING NEW PROMISE\n");
    mReady = new rtPromise; 
  }
}


rtError pxObject::Set(const char* name, const rtValue* value)
{
  #ifdef PX_DIRTY_RECTANGLES
  mIsDirty = true;
  #endif //PX_DIRTY_RECTANGLES
  if (strcmp(name, "x") != 0 && strcmp(name, "y") != 0 &&  strcmp(name, "a") != 0)
  {
    repaint();
  }
  pxObject* parent = mParent;
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
  mScene->invalidateRect(NULL);
  mScene->mDirty = true;
  return rtObject::Set(name, value);
}

rtError pxObject::animateToP2(rtObjectRef props, double duration, 
                              uint32_t interp, uint32_t animationType, 
                              rtObjectRef& promise)
{

  if (!props) return RT_FAIL;

  promise = new rtPromise;

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateTo(key, props.get<float>(key), duration, interp, animationType,(i==0)?promise:rtObjectRef());
    }
  }
//  promise.send("resolve","hello");

  return RT_OK;
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

rtError pxObject::moveToFront()
{
  pxObject* parent = this->parent();
 
  remove();
  setParent(parent);

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

#if 0
//TODO - remove
rtError pxObject::animateToF(const char* prop, double to, double duration,
                             uint32_t interp, uint32_t animationType, 
                            rtFunctionRef onEnd) 
{
  interp = pxClamp<uint32_t>(interp, 0, numInterps-1);
  animateToF(prop, to, duration, interps[interp].i,
            (pxAnimationType)animationType, onEnd);
  return RT_OK;
}
#endif

rtError pxObject::animateTo(const char* prop, double to, double duration,
                             uint32_t interp, uint32_t animationType, 
                            rtObjectRef promise) 
{
  interp = pxClamp<uint32_t>(interp, 0, numInterps-1);
  animateTo(prop, to, duration, interps[interp].i,
            (pxAnimationType)animationType, promise);
  return RT_OK;
}

// Dont fastforward when calling from set* methods since that will
// recurse indefinitely and crash and we're going to change the value in
// the set* method anyway.
void pxObject::cancelAnimation(const char* prop, bool fastforward)
{
  if (!mCancelInSet)
    return;

  bool f = mCancelInSet;
  // Do not reenter
  mCancelInSet = false;

  // If an animation for this property is in progress we cancel it here
  // we also fastforward the animation if it is of type PX_END
  vector<animation>::iterator it = mAnimations.begin();
  while (it != mAnimations.end())
  {
    animation& a = (*it);
    if (!a.cancelled && a.prop == prop)
    {
      if (a.at == PX_END)
      {
        // fastforward
#if 1
        if (fastforward)
          set(prop, a.to);
#endif

        if (a.ended)
          a.ended.send(this);

        if (a.promise)
          a.promise.send("resolve",this);
      }
      else
      {
        // TODO experiment if we cancel non ending animations set back
        // to beginning
        if (fastforward)
          set(prop, a.from);
      }
      a.cancelled = true;
    }
    ++it;
  }  
  mCancelInSet = f;
}

#if 0
//TODO - remove
void pxObject::animateToF(const char* prop, double to, double duration,
                         pxInterp interp, pxAnimationType at,
                         rtFunctionRef onEnd)
{
  cancelAnimation(prop, true);
  
  // schedule animation
  animation a;

  a.cancelled = false;
  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = to;
  a.start    = -1;
  a.duration = duration;
  a.interp   = interp?interp:pxInterpLinear;
  a.at       = at;
  a.ended = onEnd;
//  a.promise = promise;

  mAnimations.push_back(a);
}
#endif

void pxObject::animateTo(const char* prop, double to, double duration,
                         pxInterp interp, pxAnimationType at,
                         rtObjectRef promise)
{
  cancelAnimation(prop, true);
  
  // schedule animation
  animation a;

  a.cancelled = false;
  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = to;
  a.start    = -1;
  a.duration = duration;
  a.interp   = interp?interp:pxInterpLinear;
  a.at       = at;
//  a.ended = onEnd;
  a.promise = promise;

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
    if (t >= end && a.at == PX_END)
    {
      // TODO this sort of blows since this triggers another
      // animation traversal to cancel animations
#if 1
      cancelAnimation(a.prop, true);
#else
      set(a.prop, a.to);

      if (a.at == PX_END)
      {
        if (a.ended)
          a.ended.send(this);
        if (a.promise)
          a.promise.send("resolved",this);

        // Erase making sure to push the iterator forward before
        it = mAnimations.erase(it);
        continue;
      }
#endif
#if 0
      else if (a.at == PX_OSCILLATE)
      {
        // flip
        double t;
        t = a.from;
        a.from = a.to;
        a.to = t;
      }
#endif
    }
    if (a.cancelled)
    {
      it = mAnimations.erase(it);
      continue;
    }
    
    double t1 = (t-a.start)/a.duration; // Some of this could be pushed into the end handling
    double t2 = floor(t1);
    t1 = t1-t2; // 0-1
    double d = a.interp(t1);
    float from, to;
    from = a.from;
    to = a.to;
    if (a.at == PX_OSCILLATE)
    {
      if (fmod(t2,2) != 0)   // TODO perf chk ?
      {
        from = a.to;
        to   = a.from;
      }
    }
    
    float v = from + (to - from) * d;
    assert(mCancelInSet);
    mCancelInSet = false;
    set(a.prop, v);
    mCancelInSet = true;
    ++it;
  }


  #ifdef PX_DIRTY_RECTANGLES
  if (mIsDirty)
  {
    mScene->mDirtyRect.unionRect(mScreenCoordinates);
    pxRect dirtyRect = getBoundingRectInScreenCoordinates();
    mScene->mDirtyRect.unionRect(dirtyRect);
    mIsDirty = false;
  }
  #endif //PX_DIRTY_RECTANGLES
  
  // Recursively update children
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->update(t);
  }
  // Send promise
  sendPromise();
}

#ifdef PX_DIRTY_RECTANGLES
pxRect pxObject::getBoundingRectInScreenCoordinates()
{
  int x[4], y[4];
  context.mapToScreenCoordinates(mLastRenderMatrix, 0,0,x[0],y[0]);
  context.mapToScreenCoordinates(mLastRenderMatrix, mw, mh, x[1], y[1]);
  context.mapToScreenCoordinates(mLastRenderMatrix, 0, mh, x[2], y[2]);
  context.mapToScreenCoordinates(mLastRenderMatrix, mw, 0, x[3], y[3]);
  int left, right, top, bottom;

  left = x[0];
  right = x[0];
  top = y[0];
  bottom = y[0];
  for (int i = 0; i < 4; i ++)
  {
    if (x[i] < left)
    {
      left = x[i];
    }
    else if (x[i] > right)
    {
      right = x[i];
    }

    if (y[i] < top)
    {
      top = y[i];
    }
    else if (y[i] > bottom)
    {
      bottom = y[i];
    }
  }
  return pxRect(left, top, right, bottom);
}
#endif //PX_DIRTY_RECTANGLES

const float alphaEpsilon = (1.0f/255.0f);

void pxObject::drawInternal(bool maskPass)
{
  if (!drawEnabled() && !maskPass)
  {
    return;
  }
  // TODO what to do about multiple vanishing points in a given scene
  // TODO consistent behavior between clipping and no clipping when z is in use

  if (context.getAlpha() < alphaEpsilon)
    return;  // trivial reject for objects that are transparent

  pxMatrix4f m;

#if 1
#if 1
#if 0
  // translate based on xy rotate/scale based on cx, cy
  m.translate(mx+mcx, my+mcy);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  if (mr) m.rotateInDegrees(mr, mrx, mry, mrz);
  //if (mr) m.rotateInDegrees(mr, 0, 0, 1);
  if (msx != 1.0f || msy != 1.0f) m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#else
  applyMatrix(m);
#endif
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
  context.setAlpha(ma);

  if (mClip && !context.isObjectOnScreen(0,0,mw,mh))
  {
    return;
  }

  #ifdef PX_DIRTY_RECTANGLES
  mLastRenderMatrix = context.getMatrix();
  mScreenCoordinates = getBoundingRectInScreenCoordinates();
  #endif //PX_DIRTY_RECTANGLES
  
  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, mw, mh, c);

  if (mPainting)
  {
    bool maskFound = false;
    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->drawAsMask())
      {
        maskFound = true;
        break;
      }
    }
    if (maskFound)
    {
      if (mw>alphaEpsilon && mh>alphaEpsilon)
        draw();

      pxContextFramebufferRef drawableSnapshot = context.createFramebuffer(mw, mh);
      pxContextFramebufferRef maskSnapshot = context.createFramebuffer(mw, mh);
      createSnapshotOfChildren(drawableSnapshot, maskSnapshot);
      context.setMatrix(m);
      context.drawImage(0, 0, mw, mh, drawableSnapshot->getTexture(), maskSnapshot->getTexture(), PX_NONE, PX_NONE);
    }
    else if (mClip || mMaskUrl.length() > 0)
    {
      mClipSnapshotRef = createSnapshot(mClipSnapshotRef);
      context.setMatrix(m);
      context.setAlpha(ma);
      if (mClipSnapshotRef.getPtr() != NULL)
      {
        context.drawImage(0, 0, mw, mh, mClipSnapshotRef->getTexture(), mMaskTextureRef, PX_NONE, PX_NONE);
      }
    }
    else
    {
      // trivially reject things too small to be seen
      if (mw>alphaEpsilon && mh>alphaEpsilon && context.isObjectOnScreen(0, 0, mw, mh))
      {
        draw();
      }


      for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        context.pushState();
        (*it)->drawInternal();
        context.popState();
      }
    }
  }
  else
  {
    context.drawImage(0,0,mw,mh, mSnapshotRef->getTexture(), mMaskTextureRef, PX_NONE, PX_NONE);
  }

  if (!maskPass)
  {
    //TODO - remove need for mRepaintCount
    if (mRepaintCount > 0)
    {
      mRepaint = false;
    }
    else if (mRepaint)
    {
      mRepaintCount++;
    }
  }
}


bool pxObject::hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit, 
                   pxPoint2f& hitPt)
{

  // setup matrix
  pxMatrix4f m2;
#if 0
  m2.translate(mx+mcx, my+mcy);
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m2.rotateInDegrees(mr, 0, 0, 1);
  m2.scale(msx, msy);  
  m2.translate(-mcx, -mcy);
#else
  applyMatrix(m2);
#endif
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
    if (mInteractive && hitTest(newPt))
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
  return (pt.x >= 0 && pt.y >= 0 && pt.x <= mw && pt.y <= mh);
}


pxContextFramebufferRef pxObject::createSnapshot(pxContextFramebufferRef fbo)
{
  pxMatrix4f m;

//  float parentAlpha = ma;
  
  float parentAlpha = 1.0;

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  if (fbo.getPtr() == NULL || fbo->width() != floor(mw) || fbo->height() != floor(mh))
  {
    fbo = context.createFramebuffer(floor(mw), floor(mh));
  }
  else
  {
    context.updateFramebuffer(fbo, floor(mw), floor(mh));
  }
  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (mRepaint && context.setFramebuffer(fbo) == PX_OK)
  {
    context.clear(mw, mh);
    draw();

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      context.pushState();
      (*it)->drawInternal();
      context.popState();
    }
  }
  context.setFramebuffer(previousRenderSurface);

  return fbo;
}

void pxObject::createSnapshotOfChildren(pxContextFramebufferRef drawableFbo, pxContextFramebufferRef maskFbo)
{

  pxMatrix4f m;
  float parentAlpha = ma;

  context.setMatrix(m);

  context.setAlpha(parentAlpha);

  if (drawableFbo.getPtr() == NULL || drawableFbo->width() != floor(mw) || drawableFbo->height() != floor(mh))
  {
    drawableFbo = context.createFramebuffer(floor(mw), floor(mh));
  }
  else
  {
    context.updateFramebuffer(drawableFbo, floor(mw), floor(mh));
  }

  if (maskFbo.getPtr() == NULL || maskFbo->width() != floor(mw) || maskFbo->height() != floor(mh))
  {
    maskFbo = context.createFramebuffer(floor(mw), floor(mh));
  }
  else
  {
    context.updateFramebuffer(maskFbo, floor(mw), floor(mh));
  }

  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (context.setFramebuffer(maskFbo) == PX_OK)
  {
    context.clear(mw, mh);

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->drawAsMask())
      {
        context.pushState();
        (*it)->drawInternal(true);
        context.popState();
      }
    }
  }

  if (context.setFramebuffer(drawableFbo) == PX_OK)
  {
    context.clear(mw, mh);

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->drawEnabled())
      {
        context.pushState();
        (*it)->drawInternal();
        context.popState();
      }
    }
  }

  context.setFramebuffer(previousRenderSurface);
}

void pxObject::deleteSnapshot(pxContextFramebufferRef fbo)
{
  if (fbo.getPtr() != NULL)
  {
    fbo->resetFbo();
  }
}

void pxObject::createMask()
{
  deleteMask();
  
  if (mMaskUrl.length() > 0)
  {
    const char* s = mMaskUrl.cString();
    mMaskTextureCacheObject.setParent(this);
    mMaskTextureCacheObject.setUrl(s);
  }
}

void pxObject::deleteMask()
{
  if (mMaskTextureRef.getPtr() != NULL)
  {
    mMaskTextureRef->deleteTexture();
  }
}

bool pxObject::onTextureReady(pxTextureCacheObject* textureCacheObject, rtError status)
{
  if (textureCacheObject != NULL && status == RT_OK)
  {
    if (textureCacheObject == &mMaskTextureCacheObject)
    {
      mMaskTextureRef = textureCacheObject->getTexture();
      return true;
    }
  }
  repaint();
  pxObject* parent = mParent;
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
  #ifdef PX_DIRTY_RECTANGLES
  mIsDirty = true;
  #endif //PX_DIRTY_RECTANGLES
  return false;
}

rtDefineObject(rtPromise, rtObject);
rtDefineMethod(rtPromise, then);
rtDefineMethod(rtPromise, resolve);
rtDefineMethod(rtPromise, reject);

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
rtDefineProperty(pxObject, interactive);
rtDefineProperty(pxObject, painting);
rtDefineProperty(pxObject, clip);
rtDefineProperty(pxObject, mask);
rtDefineProperty(pxObject, drawAsMask);
rtDefineProperty(pxObject, draw);
rtDefineProperty(pxObject, drawAsHitTest);
rtDefineProperty(pxObject,ready);
rtDefineProperty(pxObject, numChildren);
rtDefineMethod(pxObject, getChild);
rtDefineMethod(pxObject, remove);
rtDefineMethod(pxObject, removeAll);
rtDefineMethod(pxObject, moveToFront);
//rtDefineMethod(pxObject, animateTo);
#if 0
//TODO - remove
rtDefineMethod(pxObject, animateToF2);
#endif
rtDefineMethod(pxObject, animateToP2);
rtDefineMethod(pxObject, addListener);
rtDefineMethod(pxObject, delListener);
rtDefineProperty(pxObject, emit);
rtDefineProperty(pxObject, onReady);
rtDefineMethod(pxObject, getObjectById);
rtDefineProperty(pxObject,m11);
rtDefineProperty(pxObject,m12);
rtDefineProperty(pxObject,m13);
rtDefineProperty(pxObject,m14);
rtDefineProperty(pxObject,m21);
rtDefineProperty(pxObject,m22);
rtDefineProperty(pxObject,m23);
rtDefineProperty(pxObject,m24);
rtDefineProperty(pxObject,m31);
rtDefineProperty(pxObject,m32);
rtDefineProperty(pxObject,m33);
rtDefineProperty(pxObject,m34);
rtDefineProperty(pxObject,m41);
rtDefineProperty(pxObject,m42);
rtDefineProperty(pxObject,m43);
rtDefineProperty(pxObject,m44);
rtDefineProperty(pxObject,useMatrix);


int gTag = 0;

pxScene2d::pxScene2d(bool top)
  :start(0),frameCount(0), mContainer(NULL), mShowDirtyRect(false)
{ 
  mRoot = new pxObject(this);
  mFocus = mRoot;
  mEmit = new rtEmit();
  mTop = top;
  mTag = gTag++;
  // make sure that initial onFocus is sent
  rtObjectRef e = new rtMapObject;
  e.set("target",mFocus);
  rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
  t->mEmit.send("onFocus",e);
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

rtError pxScene2d::create(rtObjectRef p, rtObjectRef& o)
{
  rtError e = RT_OK;
  rtString t = p.get<rtString>("t");

  if (!strcmp("rect",t.cString()))
    e = createRectangle(p,o);
  else if (!strcmp("text",t.cString()))
    e = createText(p,o);
  else if (!strcmp("textBox",t.cString()))
    e = createTextBox(p,o);    
  else if (!strcmp("image",t.cString()))
    e = createImage(p,o);
  else if (!strcmp("image9",t.cString()))
    e = createImage9(p,o);
  else if (!strcmp("scene",t.cString()))
    e = createScene(p,o);
  else if (!strcmp("external",t.cString()))
    e = createExternal(p,o);
  else if (!strcmp("object",t.cString()))
    e = createObject(p,o);
  else
  {
    rtLogError("Unknown object type, %s in scene.create.", t.cString());
    return RT_FAIL;
  }

  rtObjectRef c = p.get<rtObjectRef>("c");
  if (c)
  {
    uint32_t l = c.get<uint32_t>("length");
    for (uint32_t i = 0; i < l; i++)
    {
      rtObjectRef n;
      if ((e = create(c.get<rtObjectRef>(i),n)) == RT_OK)
        n.set("parent", o);
      else
        break;
    }
  }

  return e;
}

rtError pxScene2d::createObject(rtObjectRef p, rtObjectRef& o)
{
  o = new pxObject(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createRectangle(rtObjectRef p, rtObjectRef& o)
{
  o = new pxRectangle(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createText(rtObjectRef p, rtObjectRef& o)
{
  o = new pxText(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createTextBox(rtObjectRef p, rtObjectRef& o)
{
  o = new pxTextBox(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createImage9(rtObjectRef p, rtObjectRef& o)
{
  o = new pxImage9(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createScene(rtObjectRef p, rtObjectRef& o)
{
  o = new pxSceneContainer(this);
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::getFont(rtString p, rtObjectRef& o)
{
  //printf("pxScene2d::getFont\n");

  o = pxFontManager::getFont(this, p);

  return RT_OK;
}

rtError pxScene2d::clock(uint64_t & time)
{
  time = (uint64_t)pxMilliseconds();
  
  return RT_OK;
}
rtError pxScene2d::createExternal(rtObjectRef p, rtObjectRef& o)
{
  rtRefT<pxViewContainer> c = new pxViewContainer(this);
  c->setView(new testView);
  o = c.getPtr();
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::allInterpolators(rtObjectRef& v) const
{
  rtRefT<rtArrayObject> keys = new rtArrayObject;

  for (int i = 0; i < numInterps; i++)
  {
    keys->pushBack(interps[i].n);
  }
  v = keys;
  return RT_OK;
}

void pxScene2d::draw()
{
  #ifdef PX_DIRTY_RECTANGLES
  int x = mDirtyRect.left();
  int y = mDirtyRect.top();
  int w = mDirtyRect.right() - x;
  int h = mDirtyRect.bottom() - y;
  if (mTop)
  {
    if (mShowDirtyRect)
    {
      glDisable(GL_SCISSOR_TEST);
      context.clear(mWidth, mHeight);
    }
    else
    {
      context.clear(x, y, w, h);
    }
  }

  if (mRoot)
  {
    pxMatrix4f m;
    mRoot->drawInternal(m, 1.0);
    mDirtyRect.setEmpty();
  }

  if (mTop && mShowDirtyRect)
  {
    pxMatrix4f identity;
    identity.identity();
    pxMatrix4f currentMatrix = context.getMatrix();
    context.setMatrix(identity);
    float red[]= {1,0,0,1};
    bool showOutlines = context.showOutlines();
    context.setShowOutlines(true);
    context.drawDiagRect(x, y, w, h, red);
    context.setShowOutlines(showOutlines);
    context.setMatrix(currentMatrix);
    glEnable(GL_SCISSOR_TEST);
  }
  #else
  if (mTop)
    context.clear(mWidth, mHeight);

  if (mRoot)
  {
    pxMatrix4f m;
    context.pushState();
    mRoot->drawInternal(1.0);
    context.popState();
  }
  #endif //PX_DIRTY_RECTANGLES
}


void pxScene2d::onUpdate(double t)
{
  // TODO if (mTop) check??
  pxTextureCacheObject::checkForCompletedDownloads();
  pxFont::checkForCompletedDownloads();

  // Dispatch various tasks on the main UI thread
  gUIThreadQueue.process(0.01);

  if (start == 0)
    start = pxSeconds();

  update(t);
  if (mDirty)
  {
    mDirty = false;
    if (mContainer)
      mContainer->invalidateRect(NULL);
  }
  // TODO get rid of mTop somehow
  if (mTop)
  {
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
}

void pxScene2d::onDraw()
{
//  printf("**** drawing \n");

  if (mTop)
    context.setSize(mWidth, mHeight);  
#if 1
    draw();
#endif

}

// Does not draw updates scene to time t
// t is assumed to be monotonically increasing
void pxScene2d::update(double t)
{
  if (mRoot)
    mRoot->update(t);
}

pxObject* pxScene2d::getRoot() const
{
  return mRoot;
}

void pxScene2d::onSize(int32_t w, int32_t h)
{
#if 0
  if (mTop)
    context.setSize(w, h);
#endif

  mWidth  = w;
  mHeight = h;

  mRoot->set("w", w);
  mRoot->set("h", h);

  rtObjectRef e = new rtMapObject;
  e.set("name", "onResize");
  e.set("w", w);
  e.set("h", h);
  mEmit.send("onResize", e);
}

void pxScene2d::onMouseDown(int32_t x, int32_t y, uint32_t flags)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDown");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", (uint32_t)flags);
    mEmit.send("onMouseDown", e);
  }
#endif
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(x,y), hitPt;
    //    pt.x = x; pt.y = y;
    rtRefT<pxObject> hit;
    
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      mMouseDown = hit;
      // scene coordinates
      mMouseDownPt.x = x;
      mMouseDownPt.y = y;

      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseDown");
      e.set("target", (rtObject*)hit.getPtr());
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
      e.set("flags", flags);
      #if 0
      hit->mEmit.send("onMouseDown", e);
      #else
      bubbleEvent(e,hit,"onPreMouseDown","onMouseDown");
      #endif
    }
  }
}

void pxScene2d::onMouseUp(int32_t x, int32_t y, uint32_t flags)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseUp");
    e.set("x", x);
    e.set("y", y);
    e.set("flags", static_cast<uint32_t>(flags));
    mEmit.send("onMouseUp", e);
  }
#endif
  {
    //Looking for an object
    pxMatrix4f m;
    pxPoint2f pt(x,y), hitPt;
    rtRefT<pxObject> hit;
    rtRefT<pxObject> tMouseDown = mMouseDown;
    
    mMouseDown = NULL;

    // TODO optimization... we really only need to check mMouseDown
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {


      // Only send onMouseUp if this object got an onMouseDown
      if (tMouseDown == hit)
      {
        rtObjectRef e = new rtMapObject;
        e.set("name", "onMouseUp");
        e.set("target",hit.getPtr());
        e.set("x", hitPt.x);
        e.set("y", hitPt.y);
        e.set("flags", flags);
        #if 0
        hit->mEmit.send("onMouseUp", e);
        #else
        bubbleEvent(e,hit,"onPreMouseUp","onMouseUp");
        #endif
      }

      setMouseEntered(hit);
    }
    else
      setMouseEntered(NULL);
  }
}

// TODO rtRefT doesn't like non-const !=
void pxScene2d::setMouseEntered(pxObject* o)
{
  if (mMouseEntered != o)
  {
    // Tell old object we've left
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseLeave");
      e.set("target", mMouseEntered.getPtr());
      #if 0
      mMouseEntered->mEmit.send("onMouseLeave", e);
      #else
      bubbleEvent(e,mMouseEntered,"onPreMouseLeave","onMouseLeave");
      #endif
    }
    mMouseEntered = o;

    // Tell new object we've entered
    if (mMouseEntered)
    {
      rtObjectRef e = new rtMapObject;
      e.set("name", "onMouseEnter");
      e.set("target", mMouseEntered.getPtr());
      #if 0
      mMouseEntered->mEmit.send("onMouseEnter", e);
      #else
      bubbleEvent(e,mMouseEntered,"onPreMouseEnter","onMouseEnter");
      #endif
    }
  }
}
rtError pxScene2d::setFocus(rtObjectRef o)
{

  if(mFocus) 
  {
	    rtObjectRef e = new rtMapObject;
	    e.set("target",mFocus);
	    rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
	    t->mEmit.send("onBlur",e);
  }

  if (o) 
  {
	  mFocus = o;
  }
  else 
  {
	  mFocus = getRoot();
  }
  rtObjectRef e = new rtMapObject;
  e.set("target",mFocus);
  rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
  t->mEmit.send("onFocus",e);

  return RT_OK;
}
void pxScene2d::onMouseEnter()
{
}

void pxScene2d::onMouseLeave()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onMouseLeave");
  mEmit.send("onMouseLeave", e);
  
  mMouseDown = NULL;
  setMouseEntered(NULL);
}

void pxScene2d::onFocus()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onFocus");
  mEmit.send("onFocus", e);

}
void pxScene2d::onBlur()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onBlur");
  mEmit.send("onBlur", e);

}

void pxScene2d::bubbleEvent(rtObjectRef e, rtRefT<pxObject> t, 
                            const char* preEvent, const char* event) 
{
  if (e && t)
  {
    mStopPropagation = false;
    e.set("stopPropagation", get<rtFunctionRef>("stopPropagation"));
    
    vector<rtRefT<pxObject> > l;
    while(t)
    {
      l.push_back(t);
      t = t->parent();
    }

    e.set("name", preEvent);
    for (vector<rtRefT<pxObject> >::reverse_iterator it = l.rbegin();!mStopPropagation && it != l.rend();++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      if (emit)
        emit.send(preEvent,e);
    }

    e.set("name", event);
    for (vector<rtRefT<pxObject> >::iterator it = l.begin();!mStopPropagation && it != l.end();++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      if (emit)
        emit.send(event,e);
    }
  }
}

void pxScene2d::onMouseMove(int32_t x, int32_t y)
{
#if 1
  {
    // Send to root scene in global window coordinates
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("x", x);
    e.set("y", y);
    mEmit.send("onMouseMove", e);
  }
#endif

#if 1
  //Looking for an object
  pxMatrix4f m;
  pxPoint2f pt(x,y), hitPt;
  rtRefT<pxObject> hit;

  if (mMouseDown)
  {
    {
      pxVector4f from(x,y,0,1);
      pxVector4f to;
      pxObject::transformPointFromSceneToObject(mMouseDown, from, to);

//      to.dump();
      {
        pxVector4f validate;
        pxObject::transformPointFromObjectToScene(mMouseDown, to, validate);
        if (fabs(validate.mX -(float)x)> 0.01 || 
            fabs(validate.mY -(float)y) > 0.01) 
        {
          printf("Error in point transformation (%d,%d) != (%f,%f); (%f, %f)",
                 x,y,validate.mX,validate.mY,to.mX,to.mY);
        }
      }

      {
        pxVector4f validate;
        pxObject::transformPointFromObjectToObject(mMouseDown, mMouseDown, to, validate);
        if (fabs(validate.mX -(float)to.mX)> 0.01 || 
            fabs(validate.mY -(float)to.mY) > 0.01) 
        {
          printf("Error in point transformation (o2o) (%f,%f) != (%f,%f)",
                 to.mX,to.mY,validate.mX,validate.mY);
        }
      }


#if 0
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseMove");
    e.set("target", mMouseDown.getPtr());
    e.set("x", to.mX);
    e.set("y", to.mY);
    mMouseDown->mEmit.send("onMouseMove", e);
#else
    rtObjectRef e = new rtMapObject;
    e.set("target", mMouseDown.getPtr());
    e.set("x", to.mX);
    e.set("y", to.mY);
    bubbleEvent(e, mMouseDown, "onPreMouseMove", "onMouseMove");
#endif
    }
    {
    rtObjectRef e = new rtMapObject;
    e.set("name", "onMouseDrag");
    e.set("target", mMouseDown.getPtr());
    e.set("x", x);
    e.set("y", y);
    e.set("startX", mMouseDownPt.x);
    e.set("startY", mMouseDownPt.y);
#if 0
    mMouseDown->mEmit.send("onMouseDrag", e);
#else
    bubbleEvent(e,mMouseDown,"onPreMouseDrag","onMouseDrag");
#endif
    }
  }
  else // Only send mouse leave/enter events if we're not dragging
  {
    if (mRoot->hitTestInternal(m, pt, hit, hitPt))
    {
      // This probably won't stay ... we can probably send onMouseMove to the child scene level
      // rather than the object... we can send objects enter/leave events
      // and we can send drag events to objects that are being drug... 
#if 1
      rtObjectRef e = new rtMapObject;
//      e.set("name", "onMouseMove");
      e.set("x", hitPt.x);
      e.set("y", hitPt.y);
#if 0
      hit->mEmit.send("onMouseMove",e);
#else
      bubbleEvent(e, hit, "onPreMouseMove", "onMouseMove");
#endif
#endif
      
      setMouseEntered(hit);
    }
    else
      setMouseEntered(NULL);
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

void pxScene2d::onKeyDown(uint32_t keyCode, uint32_t flags) 
{
  if (mFocus)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocus);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
    bubbleEvent(e, t, "onPreKeyDown", "onKeyDown");
  }
}

void pxScene2d::onKeyUp(uint32_t keyCode, uint32_t flags)
{
  if (mFocus)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocus);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
    bubbleEvent(e, t, "onPreKeyUp", "onKeyUp");
  }
}

void pxScene2d::onChar(uint32_t c)
{
  if (mFocus)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocus);
    e.set("charCode", c);
    rtRefT<pxObject> t = (pxObject*)mFocus.get<voidPtr>("_pxObject");
    bubbleEvent(e, t, "onPreChar", "onChar");
  }
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

rtError pxScene2d::showDirtyRect(bool& v) const
{
  v=mShowDirtyRect;
  return RT_OK;
}

rtError pxScene2d::setShowDirtyRect(bool v)
{
  mShowDirtyRect = v;
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

rtError pxScene2d::screenshot(rtString type, rtString& pngData)
{
  // Is this a type we support?
  if (type == "image/png;base64")
  {
    pxOffscreen o;
    context.snapshot(o);
    o.setUpsideDown(true);
    rtData pngData2;
    if (pxStorePNGImage(o, pngData2) == RT_OK)
    {
      size_t l;
      char* d = base64_encode(pngData2.data(), pngData2.length(), &l);
      if (d)
      {
        // We return a data Url string containing the image data
        pngData = "data:image/png;base64,";
        pngData.append(d);
        free(d);
        return RT_OK;
      }
      else 
        return RT_FAIL;
    }
    else
      return RT_FAIL;
  }
  else
    return RT_FAIL;
}


rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineProperty(pxScene2d, onScene);
rtDefineProperty(pxScene2d, w);
rtDefineProperty(pxScene2d, h);
rtDefineProperty(pxScene2d, showOutlines);
rtDefineProperty(pxScene2d, showDirtyRect);
rtDefineMethod(pxScene2d, create);
rtDefineMethod(pxScene2d, createRectangle);
rtDefineMethod(pxScene2d, createText);
rtDefineMethod(pxScene2d, createTextBox);
rtDefineMethod(pxScene2d, getFont);
rtDefineMethod(pxScene2d, clock);
rtDefineMethod(pxScene2d, createImage);
rtDefineMethod(pxScene2d, createImage9);
rtDefineMethod(pxScene2d, createScene);
rtDefineMethod(pxScene2d, createExternal);
rtDefineMethod(pxScene2d, addListener);
rtDefineMethod(pxScene2d, delListener);
rtDefineMethod(pxScene2d, setFocus);
rtDefineMethod(pxScene2d, stopPropagation);
rtDefineMethod(pxScene2d, screenshot);
rtDefineMethod(pxScene2d, loadArchive);
rtDefineProperty(pxScene2d, ctx);
rtDefineProperty(pxScene2d, api);
rtDefineProperty(pxScene2d, emit);
rtDefineProperty(pxScene2d, allInterpolators);
rtDefineProperty(pxScene2d, PX_LINEAR);
rtDefineProperty(pxScene2d, PX_EXP1);
rtDefineProperty(pxScene2d, PX_EXP2);
rtDefineProperty(pxScene2d, PX_EXP3);
rtDefineProperty(pxScene2d, PX_STOP);
rtDefineProperty(pxScene2d, PX_INQUAD);
rtDefineProperty(pxScene2d, PX_INCUBIC);
rtDefineProperty(pxScene2d, PX_INBACK);
rtDefineProperty(pxScene2d, PX_EASEINELASTIC);
rtDefineProperty(pxScene2d, PX_EASEOUTELASTIC);
rtDefineProperty(pxScene2d, PX_EASEOUTBOUNCE);
rtDefineProperty(pxScene2d, PX_END);
rtDefineProperty(pxScene2d, PX_SEESAW);
rtDefineProperty(pxScene2d, PX_OSCILLATE);
rtDefineProperty(pxScene2d, PX_LOOP);
rtDefineProperty(pxScene2d, PX_NONE);
rtDefineProperty(pxScene2d, PX_STRETCH);
rtDefineProperty(pxScene2d, PX_REPEAT);

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

void RT_STDCALL testView::onUpdate(double /*t*/)
{
  if (mContainer)
    mContainer->invalidateRect(NULL);
}

void RT_STDCALL testView::onDraw()
{
//  rtLogInfo("testView::onDraw()");
  float white[] = {1,1,1,1};
  float black[] = {0,0,0,1};
  float red[]= {1,0,0,1};
  float green[] = {0,1,0,1};
  context.drawRect(mw, mh, 1, mEntered?green:red, white); 
  context.drawDiagLine(0,mMouseY,mw,mMouseY,black);
  context.drawDiagLine(mMouseX,0,mMouseX,mh,black);
}

void pxViewContainer::invalidateRect(pxRect* /*r*/)
{
  mScene->mDirty = true;
  repaint();
  pxObject* parent = this->parent();
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
  if (mScene)
  {
    mScene->invalidateRect(NULL);
  }
}

void pxScene2d::invalidateRect(pxRect* /*r*/)
{
  if (mContainer && !mTop)
  {
    mContainer->invalidateRect(NULL);
  }
}

rtDefineObject(pxViewContainer, pxObject);
rtDefineProperty(pxViewContainer, w);
rtDefineProperty(pxViewContainer, h);
rtDefineMethod(pxViewContainer, onMouseDown);
rtDefineMethod(pxViewContainer, onMouseUp);
rtDefineMethod(pxViewContainer, onMouseMove);
rtDefineMethod(pxViewContainer, onMouseEnter);
rtDefineMethod(pxViewContainer, onMouseLeave);
rtDefineMethod(pxViewContainer, onFocus);
rtDefineMethod(pxViewContainer, onBlur);
rtDefineMethod(pxViewContainer, onKeyDown);
rtDefineMethod(pxViewContainer, onKeyUp);
rtDefineMethod(pxViewContainer, onChar);

rtDefineObject(pxSceneContainer, pxViewContainer);
rtDefineProperty(pxSceneContainer, url);
rtDefineProperty(pxSceneContainer, api);
rtDefineMethod(pxSceneContainer, makeReady);

rtError pxSceneContainer::setUrl(rtString v)
{ 
  // If old promise is still unfulfilled reject it
  // and create a new promise for the context of this Url
  mReady.send("reject", this); 
  mReady = new rtPromise;  
  rtRefT<pxScene2d> newScene = new pxScene2d(false);
  setView(newScene);
  mScene = newScene;
  mUrl = v; 
  if (gOnScene)
  {
    // TODO experiment to improve interstitial rendering at scene load time
    // assuming that the script loading code restores painting at a "good" time
    setPainting(false);
    gOnScene.send((rtObject*)this, newScene.getPtr(), mUrl);
    //mReady.send("resolve",this);
  }
  return RT_OK; 
}

rtError pxSceneContainer::api(rtValue& v) const 
{ 
  return mScene->api(v); 
}

rtError pxSceneContainer::makeReady(bool ready)
{
  printf("make ready: %d\n", ready);
  mReady.send(ready?"resolve":"reject", this);
  return RT_OK;
}

#if 0
void* gObjectFactoryContext = NULL;
objectFactory gObjectFactory = NULL;
void registerObjectFactory(objectFactory f, void* context)
{
  gObjectFactory = f;
  gObjectFactoryContext = context;
}

rtError createObject2(const char* t, rtObjectRef& o)
{
  return gObjectFactory(gObjectFactoryContext, t, o);
}
#endif
