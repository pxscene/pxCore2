// pxCore Copyright 2007-2015 John Robinson
// pxScene2d.cpp

#include "pxScene2d.h"

#include <math.h>
#include <assert.h>

#include "rtLog.h"
#include "rtRefT.h"
#include "rtString.h"
#include "rtNode.h"
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

#if !defined(ENABLE_DFB) && !defined(DISABLE_WAYLAND)
#include "pxWaylandContainer.h"
#endif //ENABLE_DFB

#include "pxContext.h"
#include "pxFileDownloader.h"
#include "rtMutex.h"

#include "pxIView.h"

#include "pxClipboard.h"


// #define DEBUG_SKIP_DRAW       // Skip DRAW   code - for testing.
// #define DEBUG_SKIP_UPDATE     // Skip UPDATE code - for testing.

extern rtThreadQueue gUIThreadQueue;
uint32_t rtPromise::promiseID = 200;

// Debug Statistics
#ifdef USE_RENDER_STATS

uint32_t gDrawCalls;
uint32_t gTexBindCalls;
uint32_t gFboBindCalls;

#endif //USE_RENDER_STATS

// TODO move to rt*
// Taken from
// http://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c

#include <stdint.h>
#include <stdlib.h>

#ifdef ENABLE_RT_NODE
extern void rtWrapperSceneUpdateEnter();
extern void rtWrapperSceneUpdateExit();
rtNode script;
#endif //ENABLE_RT_NODE

#ifdef ENABLE_VALGRIND
#include <valgrind/callgrind.h>
void startProfiling()
{
  CALLGRIND_START_INSTRUMENTATION;
}

void stopProfiling()
{
  CALLGRIND_STOP_INSTRUMENTATION;
}
#endif //ENABLE_VALGRIND

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
extern pxContext context;
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


#endif

// Small helper class that vends the children of a pxObject as a collection
class pxObjectChildren: public rtObject {
public:
  pxObjectChildren(pxObject* o)
  {
    mObject = o;
  }

  virtual rtError Get(const char* name, rtValue* value) const
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

  virtual rtError Get(uint32_t i, rtValue* value) const
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
    mReady = new rtPromise();
  }
}

void pxObject::dispose()
{
  vector<animation>::iterator it = mAnimations.begin();
  for(;it != mAnimations.end();it++)
  {
    if ((*it).promise)
      (*it).promise.send("reject",this);
  }

  rtValue nullValue;
  mReady.send("reject",nullValue);

  mAnimations.clear();
  mEmit->clearListeners();
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->dispose();
    (*it)->mParent = NULL;  // setParent mutates the mChildren collection
  } 
  mChildren.clear();
  deleteSnapshot(mSnapshotRef); 
  deleteSnapshot(mClipSnapshotRef);
  deleteSnapshot(mDrawableSnapshotForMask);
  deleteSnapshot(mMaskSnapshot);
  mSnapshotRef = NULL;
  mClipSnapshotRef = NULL;
  mDrawableSnapshotForMask = NULL;
  mMaskSnapshot = NULL;
#ifdef ENABLE_RT_NODE
  script.pump();
#endif
}

/** since this is a boolean, we have to handle if someone sets it to
 * false - for now, it will mean "set focus to my parent scene" */
rtError pxObject::setFocus(bool v)
{
  printf("pxObject::setFocus v=%d\n",v);
  if(v) {
    return mScene->setFocus(this);
  }
  else {
    return mScene->setFocus(NULL);
  }

}

rtError pxObject::Set(const char* name, const rtValue* value)
{
  #ifdef PX_DIRTY_RECTANGLES
  mIsDirty = true;
  //mScreenCoordinates = getBoundingRectInScreenCoordinates();
  
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
  mScene->mDirty = true;
  return rtObject::Set(name, value);
}

rtError pxObject::animateToP2(rtObjectRef props, double duration,
                              uint32_t interp, uint32_t animationType,
                              int32_t count, rtObjectRef& promise)
{

  if (!props) return RT_FAIL;
  // Default to Linear, Loop and count==1
  if (!interp) {interp = pxConstantsAnimation::TWEEN_LINEAR;}
  if (!animationType) {animationType = pxConstantsAnimation::OPTION_LOOP;}
  if (!count) {count = 1;}

  promise = new rtPromise();

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateTo(key, props.get<float>(key), duration, interp, animationType, count,(i==0)?promise:rtObjectRef());
    }
  }

  return RT_OK;
}

void pxObject::setParent(rtRefT<pxObject>& parent)
{
  if (mParent != parent)
  {
    remove();
    mParent = parent;
    if (parent)
      parent->mChildren.push_back(this);
  }
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
        mParent = NULL;
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

rtError pxObject::animateTo(const char* prop, double to, double duration,
                             uint32_t interp, uint32_t animationType,
                            int32_t count, rtObjectRef promise)
{
  animateToInternal(prop, to, duration, ((pxConstantsAnimation*)CONSTANTS.animationConstants.getPtr())->getInterpFunc(interp),
            (pxConstantsAnimation::animationOptions)animationType, count, promise);
  return RT_OK;
}

// Dont fastforward when calling from set* methods since that will
// recurse indefinitely and crash and we're going to change the value in
// the set* method anyway.
void pxObject::cancelAnimation(const char* prop, bool fastforward, bool rewind, bool resolve)
{
  if (!mCancelInSet)
    return;
  bool f = mCancelInSet;
  // Do not reenter
  mCancelInSet = false;

  // If an animation for this property is in progress we cancel it here
  vector<animation>::iterator it = mAnimations.begin();
  while (it != mAnimations.end())
  {
    animation& a = (*it);
    if (!a.cancelled && a.prop == prop)
    {
      // Fastforward or rewind, if specified
      if( fastforward)
        set(prop, a.to);
      else if( rewind)
        set(prop, a.from);

      // If animation was never-ending, promise was already resolved.
      // If not, send it now.
      if( a.count != pxConstantsAnimation::COUNT_FOREVER)
      {
        if (a.ended)
          a.ended.send(this);
        if (a.promise)
        {
          if( resolve)
            a.promise.send("resolve",this);
          else
            a.promise.send("reject",this);
        }
      }
#if 0
      else
      {
        // TODO experiment if we cancel non ending animations set back
        // to beginning
        if (fastforward)
          set(prop, a.to);
      }
#endif
      a.cancelled = true;
    }
    ++it;
  }
  mCancelInSet = f;
}

void pxObject::animateToInternal(const char* prop, double to, double duration,
                         pxInterp interp, pxConstantsAnimation::animationOptions at,
                         int32_t count, rtObjectRef promise)
{
  cancelAnimation(prop,(at & pxConstantsAnimation::OPTION_FASTFORWARD), (at & pxConstantsAnimation::OPTION_REWIND));

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
  a.count    = count;
  a.actualCount = 0;
  a.reversing = false;
//  a.ended = onEnd;
  a.promise = promise;

  mAnimations.push_back(a);

  // resolve promise immediately if this is COUNT_FOREVER
  if( count == pxConstantsAnimation::COUNT_FOREVER)
  {
    if (a.ended)
      a.ended.send(this);
    if (a.promise)
      a.promise.send("resolve",this);
  }
}

void pxObject::update(double t)
{
#ifdef DEBUG_SKIP_UPDATE
#warning " 'DEBUG_SKIP_UPDATE' is Enabled"
  return;
#endif

  // Update animations
  vector<animation>::iterator it = mAnimations.begin();

  while (it != mAnimations.end())
  {
    animation& a = (*it);
    if (a.start < 0) a.start = t;
    double end = a.start + a.duration;

    // if duration has elapsed, increment the count for this animation
    if( t >=end && a.count != pxConstantsAnimation::COUNT_FOREVER
        && !(a.at & pxConstantsAnimation::OPTION_OSCILLATE))
    {
        a.actualCount++;
        a.start  = -1;
    }
    // if duration has elapsed and count is met, end the animation
    if (t >= end && a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count)
    {
      // TODO this sort of blows since this triggers another
      // animation traversal to cancel animations
#if 0
      cancelAnimation(a.prop, true, false, true);
#else
      assert(mCancelInSet);
      mCancelInSet = false;
      set(a.prop, a.to);
      mCancelInSet = true;

      if (a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count )
      {
        if (a.ended)
          a.ended.send(this);
        if (a.promise)
          a.promise.send("resolve",this);

        // Erase making sure to push the iterator forward before
        a.cancelled = true;
        it = mAnimations.erase(it);
        continue;
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
    if (a.at & pxConstantsAnimation::OPTION_OSCILLATE)
    {
      if( (fmod(t2,2) != 0))  // TODO perf chk ?
      {
        if(!a.reversing)
        {
          a.reversing = true;
          a.actualCount++;
        }
        from = a.to;
        to   = a.from;

      }
      else if( a.reversing && (fmod(t2,2) == 0))
      {
        a.reversing = false;
        a.actualCount++;
        a.start = -1;
      }
      // Prevent one more loop through oscillate
      if(a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count )
      {
        cancelAnimation(a.prop, false, false, true);
        it = mAnimations.erase(it);
        continue;
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
  pxMatrix4f m;
  applyMatrix(m);
  context.setMatrix(m);
  if (mIsDirty)
  {
    mScene->invalidateRect(&mScreenCoordinates);
    mLastRenderMatrix = context.getMatrix();
    pxRect dirtyRect = getBoundingRectInScreenCoordinates();
    mScene->invalidateRect(&dirtyRect);
    mIsDirty = false;
  }
  #endif //PX_DIRTY_RECTANGLES

  // Recursively update children
  for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
#ifdef PX_DIRTY_RECTANGLES
    context.pushState();
#endif //PX_DIRTY_RECTANGLES
    (*it)->update(t);
#ifdef PX_DIRTY_RECTANGLES
    context.popState();
#endif //PX_DIRTY_RECTANGLES
  }

  // Send promise
  sendPromise();
}

#ifdef PX_DIRTY_RECTANGLES
pxRect pxObject::getBoundingRectInScreenCoordinates()
{
  int w = getOnscreenWidth();
  int h = getOnscreenHeight();
  int x[4], y[4];
  context.mapToScreenCoordinates(mLastRenderMatrix, 0,0,x[0],y[0]);
  context.mapToScreenCoordinates(mLastRenderMatrix, w, h, x[1], y[1]);
  context.mapToScreenCoordinates(mLastRenderMatrix, 0, h, x[2], y[2]);
  context.mapToScreenCoordinates(mLastRenderMatrix, w, 0, x[3], y[3]);
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

pxRect pxObject::convertToScreenCoordinates(pxRect* r)
{
  if (r == NULL)
  {
     return pxRect();
  }
  int rectLeft = r->left();
  int rectRight = r->right();
  int rectTop = r->top();
  int rectBottom = r->bottom();
  int x[4], y[4];
  context.mapToScreenCoordinates(mLastRenderMatrix, rectLeft,rectTop,x[0],y[0]);
  context.mapToScreenCoordinates(mLastRenderMatrix, rectRight, rectBottom, x[1], y[1]);
  context.mapToScreenCoordinates(mLastRenderMatrix, rectLeft, rectBottom, x[2], y[2]);
  context.mapToScreenCoordinates(mLastRenderMatrix, rectRight, rectTop, x[3], y[3]);
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
  //rtLogInfo("pxObject::drawInternal mw=%f mh=%f\n", mw, mh);

  if (!drawEnabled() && !maskPass)
  {
    return;
  }
  // TODO what to do about multiple vanishing points in a given scene
  // TODO consistent behavior between clipping and no clipping when z is in use

  if (context.getAlpha() < alphaEpsilon)
  {
    return;  // trivial reject for objects that are transparent
  }

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

  pxMatrix4f m;

#if 1
#if 1
#if 0
  // translate based on xy rotate/scale based on cx, cy
  m.translate(mx+mcx, my+mcy);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  if (mr) {
    m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
    , mrx, mry, mrz
#endif //ANIMATION_ROTATE_XYZ
    );
  }
  //if (mr) m.rotateInDegrees(mr, 0, 0, 1);
  if (msx != 1.0f || msy != 1.0f) m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#else

    applyMatrix(m);  // ANIMATE !!!

#endif
#else
  // translate/rotate/scale based on cx, cy
  m.translate(mx, my);
  //  Only allow z rotation until we can reconcile multiple vanishing point thoughts
  //  m.rotateInDegrees(mr, mrx, mry, mrz);
  m.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
  , 0, 0, 1
#endif // ANIMATION_ROTATE_XYZ
  );
  m.scale(msx, msy);
  m.translate(-mcx, -mcy);
#endif
#endif

#if 0

  printf("drawInternal: %s\n", mId.cString());
  m.dump();

  pxVector4f v1(mx+w, my, 0, 1);
  printf("Print vector top\n");
  v1.dump();

  pxVector4f result1 = m.multiply(v1);
  printf("Print vector top after\n");
  result1.dump();

  pxVector4f v2(mx+w, my+mh, 0, 1);
  printf("Print vector bottom\n");
  v2.dump();

  pxVector4f result2 = m.multiply(v2);
  printf("Print vector bottom after\n");
  result2.dump();

#endif


  context.setMatrix(m);
  context.setAlpha(ma);

  if (mClip && !context.isObjectOnScreen(0,0,w,h))
  {
    //rtLogInfo("pxObject::drawInternal returning because object is not on screen mw=%f mh=%f\n", mw, mh);
    return;
  }

  #ifdef PX_DIRTY_RECTANGLES
  mLastRenderMatrix = context.getMatrix();
  mScreenCoordinates = getBoundingRectInScreenCoordinates();
  #endif //PX_DIRTY_RECTANGLES

  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, w, h, c);

  //rtLogInfo("pxObject::drawInternal mPainting=%d mw=%f mh=%f\n", mPainting, mw, mh);
  if (mPainting)
  {
    // MASKING ? ---------------------------------------------------------------------------------------------------
    bool maskFound = false;
    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->mask())
      {
        //rtLogInfo("pxObject::drawInternal mask is true mw=%f mh=%f\n", mw, mh);
        maskFound = true;
        break;
      }
    }

    // MASKING ? ---------------------------------------------------------------------------------------------------
    if (maskFound)
    {
      if (w>alphaEpsilon && h>alphaEpsilon)
      {
        draw();
      }
      createSnapshotOfChildren();
      context.setMatrix(m);
      //rtLogInfo("context.drawImage\n");
      context.drawImage(0, 0, w, h, mDrawableSnapshotForMask->getTexture(), mMaskSnapshot->getTexture());
    }
    // CLIPPING ? ---------------------------------------------------------------------------------------------------
    else if (mClip )
    {
      //rtLogInfo("calling createSnapshot for mw=%f mh=%f\n", mw, mh);
      createSnapshot(mClipSnapshotRef);

      context.setMatrix(m); // TODO: Move within if() below ?
      context.setAlpha(ma); // TODO: Move within if() below ?

      if (mClipSnapshotRef.getPtr() != NULL)
      {
        //rtLogInfo("context.drawImage\n");
        static pxTextureRef nullMaskRef;
        context.drawImage(0, 0, w, h, mClipSnapshotRef->getTexture(), nullMaskRef);
      }
    }
    // DRAWING ---------------------------------------------------------------------------------------------------
    else
    {
      // trivially reject things too small to be seen
      if ( !mClip || (w>alphaEpsilon && h>alphaEpsilon && context.isObjectOnScreen(0, 0, w, h)))
      {
        //rtLogInfo("calling draw() mw=%f mh=%f\n", mw, mh);
        draw();
      }

      // CHILDREN -------------------------------------------------------------------------------------
      for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        if((*it)->drawEnabled() == false)
        {
          continue;
        }

        context.pushState();
        //rtLogInfo("calling drawInternal() mw=%f mh=%f\n", (*it)->mw, (*it)->mh);
        (*it)->drawInternal();
#ifdef PX_DIRTY_RECTANGLES
        int left = (*it)->mScreenCoordinates.left();
        int right = (*it)->mScreenCoordinates.right();
        int top = (*it)->mScreenCoordinates.top();
        int bottom = (*it)->mScreenCoordinates.bottom();
        if (right > mScreenCoordinates.right())
        {
          mScreenCoordinates.setRight(right);
        }
        if (left < mScreenCoordinates.left())
        {
          mScreenCoordinates.setLeft(left);
        }
        if (top < mScreenCoordinates.top())
        {
          mScreenCoordinates.setTop(top);
        }
        if (bottom > mScreenCoordinates.bottom())
        {
          mScreenCoordinates.setBottom(bottom);
        }
#endif //PX_DIRTY_RECTANGLES
        context.popState();
      }
      // ---------------------------------------------------------------------------------------------------
    }
  }
  else
  {
    //rtLogInfo("context.drawImage mw=%f mh=%f\n", mw, mh);
    static pxTextureRef nullMaskRef;
    context.drawImage(0,0,w,h, mSnapshotRef->getTexture(), nullMaskRef);
  }

  // ---------------------------------------------------------------------------------------------------
  if (!maskPass)
  {
    mRepaint = false;
  }
  // ---------------------------------------------------------------------------------------------------
}


bool pxObject::hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRefT<pxObject>& hit,
                   pxPoint2f& hitPt)
{

  // setup matrix
  pxMatrix4f m2;
#if 0
  m2.translate(mx+mcx, my+mcy);
//  m.rotateInDegrees(mr, mrx, mry, mrz);
  m2.rotateInDegrees(mr
#ifdef ANIMATION_ROTATE_XYZ
  , 0, 0, 1
#endif // ANIMATION_ROTATE_XYZ
  );
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
    newPt.x = v.x();
    newPt.y = v.y();
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


void pxObject::createSnapshot(pxContextFramebufferRef& fbo)
{
  pxMatrix4f m;

//  float parentAlpha = ma;

  float parentAlpha = 1.0;

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

  //rtLogInfo("createSnapshot  w=%f h=%f\n", w, h);
  if (fbo.getPtr() == NULL || fbo->width() != floor(w) || fbo->height() != floor(h))
  {
    deleteSnapshot(fbo);
    //rtLogInfo("createFramebuffer  mw=%f mh=%f\n", w, h);
    fbo = context.createFramebuffer(floor(w), floor(h));
  }
  else
  {
    //rtLogInfo("updateFramebuffer  mw=%f mh=%f\n", w, h);
    context.updateFramebuffer(fbo, floor(w), floor(h));
  }
  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (mRepaint && context.setFramebuffer(fbo) == PX_OK)
  {
    context.clear(w, h);
    draw();

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      context.pushState();
      (*it)->drawInternal();
      context.popState();
    }
  }
  context.setFramebuffer(previousRenderSurface);
}

void pxObject::createSnapshotOfChildren()
{
  //rtLogInfo("pxObject::createSnapshotOfChildren\n");
  pxMatrix4f m;
  float parentAlpha = ma;

  context.setMatrix(m);

  context.setAlpha(parentAlpha);

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

  //rtLogInfo("createSnapshotOfChildren  w=%f h=%f\n", w, h);

  if (mDrawableSnapshotForMask.getPtr() == NULL || mDrawableSnapshotForMask->width() != floor(w) || mDrawableSnapshotForMask->height() != floor(h))
  {
    mDrawableSnapshotForMask = context.createFramebuffer(floor(w), floor(h));
  }
  else
  {
    context.updateFramebuffer(mDrawableSnapshotForMask, floor(w), floor(h));
  }

  if (mMaskSnapshot.getPtr() == NULL || mMaskSnapshot->width() != floor(w) || mMaskSnapshot->height() != floor(h))
  {
    mMaskSnapshot = context.createFramebuffer(floor(w), floor(h));
  }
  else
  {
    context.updateFramebuffer(mMaskSnapshot, floor(w), floor(h));
  }

  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (context.setFramebuffer(mMaskSnapshot) == PX_OK)
  {
    context.clear(w, h);

    for(vector<rtRefT<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->mask())
      {
        context.pushState();
        (*it)->drawInternal(true);
        context.popState();
      }
    }
  }

  if (context.setFramebuffer(mDrawableSnapshotForMask) == PX_OK)
  {
    context.clear(w, h);

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



bool pxObject::onTextureReady()
{
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
#ifdef ANIMATION_ROTATE_XYZ
rtDefineProperty(pxObject, rx);
rtDefineProperty(pxObject, ry);
rtDefineProperty(pxObject, rz);
#endif //ANIMATION_ROTATE_XYZ
rtDefineProperty(pxObject, id);
rtDefineProperty(pxObject, interactive);
rtDefineProperty(pxObject, painting);
rtDefineProperty(pxObject, clip);
rtDefineProperty(pxObject, mask);
rtDefineProperty(pxObject, draw);
rtDefineProperty(pxObject, hitTest);
rtDefineProperty(pxObject,focus);
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
//rtDefineProperty(pxObject, emit);
//rtDefineProperty(pxObject, onReady);
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


rtDefineObject(pxRoot,pxObject);

int gTag = 0;

pxScene2d::pxScene2d(bool top)
  : start(0), sigma_draw(0), sigma_update(0), frameCount(0), mContainer(NULL), mShowDirtyRectangle(false), mTestView(NULL)
{
  mRoot = new pxRoot(this);
  mFocusObj = mRoot;
  mEmit = new rtEmit();
  mTop = top;
  mTag = gTag++;

  // make sure that initial onFocus is sent
  rtObjectRef e = new rtMapObject;
  mRoot->setFocusInternal(true);
  e.set("target",mFocusObj);
  rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
  t->mEmit.send("onFocus",e);

  #ifdef USE_SCENE_POINTER
  mPointerX= 0;
  mPointerY= 0;
  mPointerW= 0;
  mPointerH= 0;
  mPointerHotSpotX= 40;
  mPointerHotSpotY= 16;
  mPointerHidden= false;
  mPointerResource= pxImageManager::getImage("cursor.png");
  #endif
}

rtError pxScene2d::dispose()
{
    rtObjectRef e = new rtMapObject;
    mEmit.send("onClose", e);
    if (mRoot)
      mRoot->dispose();
    mEmit->clearListeners();
    mRoot = NULL;
    mFocusObj = NULL;
    pxFontManager::clearAllFonts();
    return RT_OK;
}

void pxScene2d::onCloseRequest()
{
  dispose();
}

#if 0
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
#endif

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
  else if (!strcmp("imageResource",t.cString()))
    e = createImageResource(p,o);
  else if (!strcmp("fontResource",t.cString()))
    e = createFontResource(p,o);
  else if (!strcmp("scene",t.cString()))
    e = createScene(p,o);
  else if (!strcmp("external",t.cString()))
    e = createExternal(p,o);
  else if (!strcmp("wayland",t.cString()))
    e = createWayland(p,o);
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

rtError pxScene2d::createImageResource(rtObjectRef p, rtObjectRef& o)
{
  rtString url = p.get<rtString>("url");
  o = pxImageManager::getImage(url);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createFontResource(rtObjectRef p, rtObjectRef& o)
{
  rtString url = p.get<rtString>("url");
  o = pxFontManager::getFont(url);
  return RT_OK;
}

rtError pxScene2d::createScene(rtObjectRef p, rtObjectRef& o)
{
  o = new pxSceneContainer(this);
  o.set(p);
  o.send("init");
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
  mTestView = new testView;
  c->setView(mTestView);
  o = c.getPtr();
  o.set(p);
  o.send("init");
  return RT_OK;
}

rtError pxScene2d::createWayland(rtObjectRef p, rtObjectRef& o)
{
#if defined(ENABLE_DFB) || defined(DISABLE_WAYLAND)
  UNUSED_PARAM(p);
  UNUSED_PARAM(o);

  return RT_FAIL;
#else
  rtRefT<pxWaylandContainer> c = new pxWaylandContainer(this);
  c->setView(new pxWayland);
  o = c.getPtr();
  o.set(p);
  o.send("init");
  return RT_OK;
#endif //ENABLE_DFB
}

void pxScene2d::draw()
{
#ifdef DEBUG_SKIP_DRAW
#warning " 'DEBUG_SKIP_DRAW' is Enabled"
  return;
#endif

  //rtLogInfo("pxScene2d::draw()\n");
  #ifdef PX_DIRTY_RECTANGLES
  int x = mDirtyRect.left();
  int y = mDirtyRect.top();
  int w = mDirtyRect.right() - x+1;
  int h = mDirtyRect.bottom() - y+1;

  static bool previousShowDirtyRect = false;

  if (mShowDirtyRectangle || previousShowDirtyRect)
  {
    context.enableDirtyRectangles(false);
  }

  if (mTop)
  {
    if (mShowDirtyRectangle)
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
    context.pushState();

    mRoot->drawInternal(true);

    context.popState();
    mDirtyRect.setEmpty();
  }

  if (mTop && mShowDirtyRectangle)
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
  previousShowDirtyRect = mShowDirtyRectangle;

#else // Not ... PX_DIRTY_RECTANGLES

  if (mTop)
  {
    context.clear(mWidth, mHeight);
  }

  if (mRoot)
  {
    pxMatrix4f m;
    context.pushState();

    mRoot->drawInternal(true); // mask it !
    context.popState();
  }
  #endif //PX_DIRTY_RECTANGLES

  #ifdef USE_SCENE_POINTER
  if (mPointerTexture.getPtr() == NULL)
  {
    mPointerTexture= ((rtImageResource*)mPointerResource.getPtr())->getTexture();
    if (mPointerTexture.getPtr() != NULL)
    {
      mPointerW = mPointerTexture->width();
      mPointerH = mPointerTexture->height();
    }
  }
  if ( (mPointerTexture.getPtr() != NULL) &&
       !mPointerHidden )
  {
     context.drawImage( mPointerX-mPointerHotSpotX, mPointerY-mPointerHotSpotY,
                        mPointerW, mPointerH,
                        mPointerTexture, mNullTexture);
  }
#endif //USE_SCENE_POINTER
}

void pxScene2d::onUpdate(double t)
{
  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateEnter();
  }
  #endif //ENABLE_RT_NODE
  // TODO if (mTop) check??
 // pxTextureCacheObject::checkForCompletedDownloads();
  //pxFont::checkForCompletedDownloads();

  // Dispatch various tasks on the main UI thread
  gUIThreadQueue.process(0.01);

  if (start == 0)
  {
    start = pxSeconds();
  }

  double start_frame = pxSeconds(); //##

  update(t);

  sigma_update += (pxSeconds() - start_frame); //##

  if (mDirty)
  {
    mDirty = false;
    if (mContainer)
      mContainer->invalidateRect(NULL);
  }
  // TODO get rid of mTop somehow
  if (mTop)
  {
    unsigned int target_frame_ms = 60;
    int targetFPS = (1.0 / ((double) target_frame_ms)) * 1000;

    if (frameCount >= targetFPS)
    {
      end2 = pxSeconds();

    double fps = rint((double)frameCount/(end2-start));

#ifdef USE_RENDER_STATS
      double   dpf = rint( (double) gDrawCalls    / (double) frameCount ); // e.g.   glDraw*()           - calls per frame
      double   bpf = rint( (double) gTexBindCalls / (double) frameCount ); // e.g.   glBindTexture()     - calls per frame
      double   fpf = rint( (double) gFboBindCalls / (double) frameCount ); // e.g.   glBindFramebuffer() - calls per frame

      // TODO:  update / render times need some work...

      // double draw_ms   = ( (double) sigma_draw     / (double) frameCount ) * 1000.0f; // Average frame  time
      // double update_ms = ( (double) sigma_update   / (double) frameCount ) * 1000.0f; // Average update time

      // printf("%g fps   pxObjects: %d   Draw: %g   Tex: %g   Fbo: %g     draw_ms: %0.04g   update_ms: %0.04g\n",
      //     fps, pxObjectCount, dpf, bpf, fpf, draw_ms, update_ms );

      rtLogDebug("%g fps   pxObjects: %d   Draw: %g   Tex: %g   Fbo: %g \n", fps, pxObjectCount, dpf, bpf, fpf);

      gDrawCalls    = 0;
      gTexBindCalls = 0;
      gFboBindCalls = 0;

      sigma_draw   = 0;
      sigma_update = 0;
#else
    rtLogDebug("%g fps   pxObjects: %d\n", fps, pxObjectCount);
#endif //USE_RENDER_STATS

    // TODO FUTURES... might be nice to have "struct" style object's that get copied
    // at the interop layer so we don't get remoted calls back to the render thread
    // for accessing the values (events would be the primary usecase)
    rtObjectRef e = new rtMapObject;
    e.set("fps", fps);
    mEmit.send("onFPS", e);

      start = end2; // start of frame
    frameCount = 0;
  }

  frameCount++;
  }
  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateExit();
  }
  #endif //ENABLE_RT_NODE
}

void pxScene2d::onDraw()
{
//  printf("**** drawing \n");

  if (mTop)
  {
    #ifdef ENABLE_RT_NODE
    rtWrapperSceneUpdateEnter();
    #endif //ENABLE_RT_NODE
    context.setSize(mWidth, mHeight);
  }
#if 1

#ifdef USE_RENDER_STATS
  double start_draw = pxSeconds(); //##
#endif //USE_RENDER_STATS

  draw();

#ifdef USE_RENDER_STATS
  sigma_draw += (pxSeconds() - start_draw); //##
#endif //USE_RENDER_STATS

#endif
  #ifdef ENABLE_RT_NODE
  if (mTop)
  {
    rtWrapperSceneUpdateExit();
  }
  #endif //ENABLE_RT_NODE
}

// Does not draw updates scene to time t
// t is assumed to be monotonically increasing
void pxScene2d::update(double t)
{
  if (mRoot)
  {
#ifdef PX_DIRTY_RECTANGLES
      context.pushState();
#endif //PX_DIRTY_RECTANGLES

#ifndef DEBUG_SKIP_UPDATE
      mRoot->update(t);
#else
      UNUSED_PARAM(t);
#endif

#ifdef PX_DIRTY_RECTANGLES
      context.popState();
#endif //PX_DIRTY_RECTANGLES
  }
}

pxObject* pxScene2d::getRoot() const
{
  return mRoot;
}

void pxScene2d::onComplete()
{
  rtObjectRef e = new rtMapObject;
  e.set("name", "onComplete");
  mEmit.send("onComplete", e);
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

#if 0 // JRJR... this shouldn't crash
  if (mContainer)
    mContainer->invalidateRect(NULL);
#endif
}

bool pxScene2d::onMouseDown(int32_t x, int32_t y, uint32_t flags)
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
  return false;
}

bool pxScene2d::onMouseUp(int32_t x, int32_t y, uint32_t flags)
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
  return false;
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
/** This function is not exposed to javascript; it is called when
 * mFocus = true is set for a pxObject whose parent scene is this scene
 **/
rtError pxScene2d::setFocus(rtObjectRef o)
{
  rtLogInfo("pxScene2d::setFocus");
  if(mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    ((pxObject*)mFocusObj.get<voidPtr>("_pxObject"))->setFocusInternal(false);
    e.set("target",mFocusObj);
    rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    //t->mEmit.send("onBlur",e);
    bubbleEvent(e,t,"onPreBlur","onBlur");
  }

  if (o)
  {
	  mFocusObj = o;
  }
  else
  {
	  mFocusObj = getRoot();
  }
  rtObjectRef e = new rtMapObject;
  ((pxObject*)mFocusObj.get<voidPtr>("_pxObject"))->setFocusInternal(true);
  e.set("target",mFocusObj);
  rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
  //t->mEmit.send("onFocus",e);
  bubbleEvent(e,t,"onPreFocus","onFocus");

  return RT_OK;
}

bool pxScene2d::onMouseEnter()
{
  return false;
}

bool pxScene2d::onMouseLeave()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onMouseLeave");
  mEmit.send("onMouseLeave", e);

  mMouseDown = NULL;
  setMouseEntered(NULL);
  return false;
}

bool pxScene2d::onFocus()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onFocus");
  mEmit.send("onFocus", e);
  return false;
}

bool pxScene2d::onBlur()
{
  // top level scene event
  rtObjectRef e = new rtMapObject;
  e.set("name", "onBlur");
  mEmit.send("onBlur", e);
  return false;
}

bool gStopPropagation;
rtError stopPropagation2(int /*numArgs*/, const rtValue* /*args*/, rtValue* /*result*/, void* ctx)
{
  bool& stopProp = *(bool*)ctx;
  stopProp = true;
  return RT_OK;
}

bool pxScene2d::bubbleEvent(rtObjectRef e, rtRefT<pxObject> t,
                            const char* preEvent, const char* event)
{
  bool consumed = false;
  mStopPropagation = false;
  rtValue stop;
  if (e && t)
  {
    AddRef();  // TODO refactor? make sure scene stays alive while we bubble since we're using the address of mStopPropagation
//    e.set("stopPropagation", get<rtFunctionRef>("stopPropagation"));
    e.set("stopPropagation", new rtFunctionCallback(stopPropagation2, (void*)&mStopPropagation));

    vector<rtRefT<pxObject> > l;
    while(t)
    {
      l.push_back(t);
      t = t->parent();
    }

//    printf("before %s bubble\n", preEvent);
    e.set("name", preEvent);
    for (vector<rtRefT<pxObject> >::reverse_iterator it = l.rbegin();!mStopPropagation && it != l.rend();++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      if (emit)
        emit.sendReturns(preEvent,e,stop);
      if (mStopPropagation)
        break;
    }
//    printf("after %s bubble\n", preEvent);

//    printf("before %s bubble\n", event);
    e.set("name", event);
    for (vector<rtRefT<pxObject> >::iterator it = l.begin();!mStopPropagation && it != l.end();++it)
    {
      // TODO a bit messy
      rtFunctionRef emit = (*it)->mEmit.getPtr();
      // TODO: As we bubble onMouseMove we need to keep adjusting the coordinates into the
      // coordinate space of the successive parents object ??
      // JRJR... not convinced on this comment please discus with me first.
      if (emit)
        emit.sendReturns(event,e,stop);
//      printf("mStopPropagation %d\n", mStopPropagation);
      if (mStopPropagation)
      {
        printf("Event bubble aborted\n");
        break;
      }
    }
//    printf("after %s bubble\n", event);
    consumed = mStopPropagation;
    Release();
  }
  return consumed;
}

bool pxScene2d::onMouseMove(int32_t x, int32_t y)
{
  #ifdef USE_SCENE_POINTER
  mPointerX= x;
  mPointerY= y;
  invalidateRect(NULL);
  mDirty= true;
  #endif
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
        if (fabs(validate.x()-(float)x)> 0.01 ||
            fabs(validate.y()-(float)y) > 0.01)
        {
          rtLogInfo("Error in point transformation (%d,%d) != (%f,%f); (%f, %f)",
                 x,y,validate.x(),validate.y(),to.x(),to.y());
        }
      }

      {
        pxVector4f validate;
        pxObject::transformPointFromObjectToObject(mMouseDown, mMouseDown, to, validate);
        if (fabs(validate.x()-(float)to.x())> 0.01 ||
            fabs(validate.y()-(float)to.y()) > 0.01)
        {
          rtLogInfo("Error in point transformation (o2o) (%f,%f) != (%f,%f)",
                 to.x(),to.y(),validate.x(),validate.y());
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
    e.set("x", to.x());
    e.set("y", to.y());
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
  return false;
}

bool pxScene2d::onKeyDown(uint32_t keyCode, uint32_t flags)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreKeyDown", "onKeyDown");
  }
  return false;
}

bool pxScene2d::onKeyUp(uint32_t keyCode, uint32_t flags)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("keyCode", keyCode);
    e.set("flags", (uint32_t)flags);
    rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreKeyUp", "onKeyUp");
  }
  return false;
}

bool pxScene2d::onChar(uint32_t c)
{
  if (mFocusObj)
  {
    rtObjectRef e = new rtMapObject;
    e.set("target",mFocusObj);
    e.set("charCode", c);
    rtRefT<pxObject> t = (pxObject*)mFocusObj.get<voidPtr>("_pxObject");
    return bubbleEvent(e, t, "onPreChar", "onChar");
  }
  return false;
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
  v=mShowDirtyRectangle;
  return RT_OK;
}

rtError pxScene2d::setShowDirtyRect(bool v)
{
  mShowDirtyRectangle = v;
  return RT_OK;
}

rtError pxScene2d::screenshot(rtString type, rtString& pngData)
{
  // Is this a type we support?
  if (type == "image/png;base64")
  {
    pxOffscreen o;
    context.snapshot(o);

    rtData pngData2;
    if (pxStorePNGImage(o, pngData2) == RT_OK)
    {

//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK 
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK 
#if 0
    FILE *myFile = fopen("/mnt/nfs/env/snap.png", "wb");
    if( myFile != NULL)
    {
      fwrite( pngData2.data(), sizeof(char), pngData2.length(),myFile);
      fclose(myFile);
    }
#endif    
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK 
//HACK JUNK HACK JUNK HACK JUNK HACK JUNK HACK JUNK 
    
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

rtError pxScene2d::clipboardSet(rtString type, rtString clipString)
{
//    printf("\n ##########   clipboardSet()  >> %s ", type.cString() ); fflush(stdout);

    pxClipboard::instance()->setString(type.cString(), clipString.cString());

    return RT_OK;
}

rtError pxScene2d::clipboardGet(rtString type, rtString &retString)
{
//    printf("\n ##########   clipboardGet()  >> %s ", type.cString() ); fflush(stdout);
    std::string retVal = pxClipboard::instance()->getString(type.cString());

    retString = rtString(retVal.c_str());

    return RT_OK;
}

rtDefineObject(pxScene2d, rtObject);
rtDefineProperty(pxScene2d, root);
rtDefineProperty(pxScene2d, w);
rtDefineProperty(pxScene2d, h);
rtDefineProperty(pxScene2d, showOutlines);
rtDefineProperty(pxScene2d, showDirtyRect);
rtDefineMethod(pxScene2d, create);
rtDefineMethod(pxScene2d, clock);
//rtDefineMethod(pxScene2d, createWayland);
rtDefineMethod(pxScene2d, addListener);
rtDefineMethod(pxScene2d, delListener);
rtDefineMethod(pxScene2d, getFocus);
//rtDefineMethod(pxScene2d, stopPropagation);
rtDefineMethod(pxScene2d, screenshot);

rtDefineMethod(pxScene2d, clipboardGet);
rtDefineMethod(pxScene2d, clipboardSet);

rtDefineMethod(pxScene2d, loadArchive);
rtDefineProperty(pxScene2d, ctx);
rtDefineProperty(pxScene2d, api);
//rtDefineProperty(pxScene2d, emit);
// Properties for access to Constants
rtDefineProperty(pxScene2d,animation);
rtDefineProperty(pxScene2d,stretch);
rtDefineProperty(pxScene2d,alignVertical);
rtDefineProperty(pxScene2d,alignHorizontal);
rtDefineProperty(pxScene2d,truncation);
rtDefineMethod(pxScene2d, dispose);

rtError pxScene2dRef::Get(const char* name, rtValue* value) const
{
  return (*this)->Get(name, value);
}

rtError pxScene2dRef::Get(uint32_t i, rtValue* value) const
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

void pxViewContainer::invalidateRect(pxRect* r)
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
#ifdef PX_DIRTY_RECTANGLES
    pxRect screenRect = convertToScreenCoordinates(r);
    mScene->invalidateRect(&screenRect);
#else
    mScene->invalidateRect(NULL);
    UNUSED_PARAM(r);    
#endif //PX_DIRTY_RECTANGLES
  }
}

void pxScene2d::invalidateRect(pxRect* r)
{
#ifdef PX_DIRTY_RECTANGLES
  if (r != NULL)
  {
    mDirtyRect.unionRect(*r);
    mDirty = true;
  }
#else
  UNUSED_PARAM(r); 
#endif //PX_DIRTY_RECTANGLES
  if (mContainer && !mTop)
  {
#ifdef PX_DIRTY_RECTANGLES
    mContainer->invalidateRect(&mDirtyRect);
#else
    mContainer->invalidateRect(NULL);
#endif //PX_DIRTY_RECTANGLES
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
rtDefineProperty(pxSceneContainer, ready);
rtDefineMethod(pxSceneContainer, releaseResources);
//rtDefineMethod(pxSceneContainer, makeReady);   // DEPRECATED ?


rtError pxSceneContainer::setUrl(rtString url)
{
  // If old promise is still unfulfilled reject it
  // and create a new promise for the context of this Url
  mReady.send("reject", this);
  mReady = new rtPromise( std::string("pxSceneContainer >> ") + std::string(url) );

  mUrl = url;
  setScriptView(new pxScriptView(url.cString(),""));

  return RT_OK;
}

rtError pxSceneContainer::api(rtValue& v) const
{
//  return mScene->api(v);
  if (mScriptView)
    return mScriptView->api(v);
  else
    return RT_FAIL;
}

rtError pxSceneContainer::ready(rtObjectRef& o) const
{
  if (mScriptView)
    return mScriptView->ready(o);
  return RT_FAIL;
}

rtError pxSceneContainer::setScriptView(pxScriptView* scriptView)
{
  mScriptView = scriptView;
  setView(scriptView);
  return RT_OK;
}


// rtError pxSceneContainer::makeReady(bool ready)
// {
//   //DEPRECATED ?
//   rtLogInfo("make ready: %d", ready);
//   mReady.send(ready?"resolve":"reject", this);
//   return RT_OK;
// }


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

pxScriptView::pxScriptView(const char* url, const char* /*lang*/) 
     : mWidth(-1), mHeight(-1), mViewContainer(NULL), mRefCount(0)
{
  #ifdef ENABLE_RT_NODE
  mCtx = script.createContext();
  if (mCtx)
  {
    mCtx->add("getScene",  new rtFunctionCallback(getScene,  this));
    mCtx->add("makeReady", new rtFunctionCallback(makeReady, this));

    mCtx->add("getContextID", new rtFunctionCallback(getContextID, this));

    mReady = new rtPromise();

    mCtx->runFile("init.js");

    char buffer[1024];
    sprintf(buffer, "loadUrl(\"%s\");", url);
    mCtx->runScript(buffer);
  }
  #endif //ENABLE_RT_NODE
}

rtError pxScriptView::getScene(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    if (numArgs == 1)
    {
      rtString sceneType = args[0].toString();
      // JR Todo can specify what scene version/type to create in args
      if (!v->mScene)
      {
        static bool top = true;
        pxScene2dRef scene = new pxScene2d(top);
        top = false;
        v->mView = scene;
        v->mScene = scene;

        v->mView->setViewContainer(v->mViewContainer);
        v->mView->onSize(v->mWidth,v->mHeight);
      }

      if (result)
      {
        *result = v->mScene;
        return RT_OK;
      }
    }
  }
  return RT_FAIL;
}



rtError pxScriptView::getContextID(int numArgs, const rtValue* args, rtValue* result, void* ctx)
{
  UNUSED_PARAM(numArgs);
  UNUSED_PARAM(args);

#ifdef ENABLE_RT_NODE
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    Locker                locker(v->mCtx->getIsolate());
    Isolate::Scope isolate_scope(v->mCtx->getIsolate());
    HandleScope     handle_scope(v->mCtx->getIsolate());

    Local<Context> ctx = v->mCtx->getLocalContext();
    uint32_t ctx_id = GetContextId( ctx );

    if (result)
    {
      *result = rtValue(ctx_id);
      return RT_OK;
    }
  }
#endif //ENABLE_RT_NODE

  rtLogError("############# printContextID() >> ctx: BAD !!\n");  fflush(stdout); // JUNK

  return RT_FAIL;
}


rtError pxScriptView::makeReady(int numArgs, const rtValue* args, rtValue* /*result*/, void* ctx)
{
  if (ctx)
  {
    pxScriptView* v = (pxScriptView*)ctx;

    if (numArgs >= 1)
    {
      if (args[0].toBool())
      {
        if (numArgs >= 2)
        {
          v->mApi = args[1].toObject();
        }

        v->mReady.send("resolve", v->mApi);
      }
      else
      {
        v->mReady.send("reject", new rtObject); // TODO JRJR  Why does this fail if I leave the argment as null...
      }

      return RT_OK;
    }
  }
  return RT_FAIL;
}

