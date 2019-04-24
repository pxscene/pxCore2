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

#include "rtPromise.h"

#include "pxCore.h"
#include "pxObject.h"

#include "pxContext.h"
#include "pxAnimate.h"
#include "pxScene2d.h"

#include "pxRectangle.h"
#include "pxFont.h"
#include "pxText.h"
#include "pxTextBox.h"
#include "pxImage.h"

#include <algorithm>

using namespace std;

class pxObjectChildren; //fwd

extern pxContext  context;
extern rtScript   script;
extern bool gDirtyRectsEnabled;

int pxObjectCount = 0;

////////////////////////////////////////////////////////////////////////////////////////////////

// Small helper class that vends the children of a pxObject as a collection
class pxObjectChildren: public rtObject {
public:

  rtDeclareObject(pxObjectChildren, rtObject);

  pxObjectChildren(pxObject* o)
  {
    mObject = o;
  }

  virtual rtError Get(const char* name, rtValue* value) const
  {
    if (!value) return RT_FAIL;
    if (!strcmp(name, "length"))
    {
      value->setUInt32( (uint32_t) mObject->numChildren());
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

  virtual rtError Set(const char* name, const rtValue* value)
  {
    (void)name;
    (void)value;
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

  virtual rtError Set(uint32_t i, const rtValue* value)
  {
    (void)i;
    (void)value;
    // readonly property
    return RT_PROP_NOT_FOUND;
  }

private:
  rtRef<pxObject> mObject;
};

rtDefineObject(pxObjectChildren, rtObject);

////////////////////////////////////////////////////////////////////////////////////////////////

// pxObject methods
pxObject::pxObject(pxScene2d* scene): 
  rtObject(), mParent(NULL), mpx(0), mpy(0), mcx(0), mcy(0), mx(0), my(0), ma(1.0), mr(0),

#ifdef ANIMATION_ROTATE_XYZ
    mrx(0), mry(0), mrz(1.0),
#endif //ANIMATION_ROTATE_XYZ

    msx(1), msy(1), mw(0), mh(0),
    mInteractive(true),
    mSnapshotRef(), mPainting(true), mClip(false), mMask(false), mDraw(true), mHitTest(true), mReady(),
    mFocus(false),mClipSnapshotRef(),mCancelInSet(true),mRepaint(true)
    , mIsDirty(true), mRenderMatrix(), mLastRenderMatrix(), mScreenCoordinates(), mDirtyRect(), mScene(NULL)
    ,mDrawableSnapshotForMask(), mMaskSnapshot(), mIsDisposed(false), mSceneSuspended(false)
  {
    pxObjectCount++;
    mScene = scene;
    mReady = new rtPromise;
    mEmit = new rtEmit;
  }

pxObject::~pxObject()
{
    #ifdef ENABLE_PXOBJECT_TRACKING
    rtLogInfo("pxObjectTracking DESTRUCTION [%p]",this);
    #endif
//    rtString d;
    // TODO... why is this bad
//    sendReturns<rtString>("description",d);
    //rtLogDebug("**************** pxObject destroyed: %s\n",getMap()->className);
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      (*it)->mParent = NULL;  // setParent mutates the mChildren collection
    }
    mChildren.clear();
    pxObjectCount--;
    clearSnapshot(mSnapshotRef);
    clearSnapshot(mClipSnapshotRef);
    clearSnapshot(mDrawableSnapshotForMask);
    clearSnapshot(mMaskSnapshot);
    mSnapshotRef = NULL;
    mClipSnapshotRef = NULL;
    mDrawableSnapshotForMask = NULL;
    mMaskSnapshot = NULL;
    pxScene2d::updateObject(this, false);
}

void pxObject::onInit()
{
  triggerUpdate();
}

void pxObject::sendPromise()
{
  if(mInitialized && !((rtPromise*)mReady.getPtr())->status())
  {
    mReady.send("resolve",this);
  }
}

void pxObject::dispose(bool pumpJavascript)
{
  if (!mIsDisposed)
  {
    //rtLogInfo(__FUNCTION__);
    mIsDisposed = true;
    rtValue nullValue;
    vector<animation>::iterator it = mAnimations.begin();
    for(;it != mAnimations.end();it++)
    {
      if ((*it).promise)
      {
	  (*it).promise.send("reject",nullValue);
      }
    }

    mReady.send("reject",nullValue);

    mAnimations.clear();
    mEmit->clearListeners();
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      (*it)->mParent = NULL;  // setParent mutates the mChildren collection
      (*it)->dispose(false);
    }
    mChildren.clear();
    clearSnapshot(mSnapshotRef);
    clearSnapshot(mClipSnapshotRef);
    clearSnapshot(mDrawableSnapshotForMask);
    clearSnapshot(mMaskSnapshot);
    mSnapshotRef = NULL;
    mClipSnapshotRef = NULL;
    mDrawableSnapshotForMask = NULL;
    mMaskSnapshot = NULL;
    if (mScene)
    {
      mScene->innerpxObjectDisposed(this);
    }
#ifdef ENABLE_RT_NODE
    if (pumpJavascript)
    {
      script.pump();
    }
#else
    (void)pumpJavascript;
#endif
 }
}

/** since this is a boolean, we have to handle if someone sets it to
 * false - for now, it will mean "set focus to my parent scene" */
rtError pxObject::setFocus(bool v)
{
  rtLogDebug("pxObject::setFocus v=%d\n",v);
  if(v) {
    return mScene->setFocus(this);
  }
  else {
    return mScene->setFocus(NULL);
  }

}

rtError pxObject::Set(uint32_t i, const rtValue* value)
{
  (void)i;
  (void)value;
  rtLogError("pxObject::Set(uint32_t, const rtValue*) - not implemented");
  return RT_ERROR_NOT_IMPLEMENTED;
}

rtError pxObject::Set(const char* name, const rtValue* value)
{
  markDirty();

  if (strcmp(name, "x") != 0 && strcmp(name, "y") != 0 &&  strcmp(name, "a") != 0)
  {
    repaint();
  }
  repaintParents();
  mScene->mDirty = true;
  return rtObject::Set(name, value);
}

static double getDurationSeconds(rtString str)
{
  const char* pStr = str.cString();
  char*    p = (char*) pStr;
  
  while( *p++ != '\0') // find Units ... default to Seconds if not found
  {
    if(*p == 'm' || *p == 'M' || *p == 'S' || *p == 's') break;
  };
  
  double d = 0;
  if(sscanf(pStr,"%lf", &d) == 1)
  {
    if(*p == 'm' || *p == 'M') {  d /= 1000.0; } // 'sS' seconds, 'mM' milliseconds ... Convert 'ms' to 's'
  }
  
  return d;
}

// TODO Cleanup animateTo methods... animateTo animateToP2 etc...
rtError pxObject::animateToP2(rtObjectRef props, rtValue duration,
                              uint32_t interp, uint32_t options,
                              int32_t count, rtObjectRef& promise)
{
  if (mIsDisposed)
  {
    rtLogWarn("animation is performed on disposed object !!!!");
    promise = new rtPromise();
    rtValue nullValue;
    promise.send("reject",nullValue);
    return RT_OK;
  }

  if (!props) return RT_FAIL;

  // TODO JR... not sure that we should do an early out here... thinking
  // we should still return a resolved promise given time...
  // just going to get exceptions if you try to do a .then on the return result
  //if (!props) return RT_OK;
  // Default to Linear, Loop and count==1
  if (!interp)  { interp = pxConstantsAnimation::TWEEN_LINEAR;}
  if (!options) {options = pxConstantsAnimation::OPTION_LOOP;}
  if (!count)   {  count = 1;}

  double duration2 = 0;

  if(duration.getType() == RT_stringType)
  {
    rtString str;
    if(duration.getString(str) == RT_OK)
    {
      duration2 = getDurationSeconds(str);
    }
  }
  else
  {
    duration.getDouble(duration2);
  }

  promise = new rtPromise();

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateTo(key, props.get<float>(key), duration2, interp, options, count,(i==0)?promise:rtObjectRef());
    }
  }

  return RT_OK;
}

rtError pxObject::animateToObj(rtObjectRef props, rtValue duration,
                              uint32_t interp, uint32_t options,
                              int32_t count, rtObjectRef& animateObj)
{

  if (!props) return RT_FAIL;
  // TODO JR... not sure that we should do an early out here... thinking
  // we should still return a resolved promise given time...
  // just going to get exceptions if you try to do a .then on the return result
  //if (!props) return RT_OK;
  // Default to Linear, Loop and count==1

  if (!interp)  {  interp = pxConstantsAnimation::TWEEN_LINEAR;}
  if (!options) { options = pxConstantsAnimation::OPTION_LOOP; }
  if (!count)   {   count = 1;}

  double duration2 = 0;
  if(duration.getType() == RT_stringType)
  {
    rtString str;
    if(duration.getString(str) == RT_OK)
    {
      duration2 = getDurationSeconds(str);
    }
  }
  else
  {
    duration.getDouble(duration2);
  }

  rtObjectRef promise = new rtPromise();
  animateObj = new pxAnimate(props, interp, (pxConstantsAnimation::animationOptions)options, duration2, count, promise, this);
  if (mIsDisposed)
  {
    rtLogWarn("animation is performed on disposed object !!!!");
    rtValue nullValue;
    promise.send("reject",nullValue);
    return RT_OK;
  }

  rtObjectRef keys = props.get<rtObjectRef>("allKeys");
  if (keys)
  {
    uint32_t len = keys.get<uint32_t>("length");
    for (uint32_t i = 0; i < len; i++)
    {
      rtString key = keys.get<rtString>(i);
      animateToInternal(key, props.get<float>(key), duration2, ((pxConstantsAnimation*)CONSTANTS.animationConstants.getPtr())->getInterpFunc(interp), (pxConstantsAnimation::animationOptions)options, count,(i==0)?promise:rtObjectRef(),animateObj);
    }
  }
  if (NULL != animateObj.getPtr())
    ((pxAnimate*)animateObj.getPtr())->setStatus(pxConstantsAnimation::STATUS_INPROGRESS);
  return RT_OK;
}

void pxObject::setParent(rtRef<pxObject>& parent)
{
  if (mParent != parent)
  {
    remove();
    mParent = parent;
    if (parent)
      parent->mChildren.push_back(this);

    markDirty();
    if (mScene != NULL)
    {
      mScene->invalidateRect(NULL);
    }
    triggerUpdate();
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
    for(vector<rtRef<pxObject> >::iterator it = mParent->mChildren.begin();
        it != mParent->mChildren.end(); ++it)
    {
      if ((it)->getPtr() == this)
      {
        pxObject* parent = mParent;
        mParent->mChildren.erase(it);
        mParent = NULL;

        parent->markDirty();
        parent->repaint();
        parent->repaintParents();
        mScene->mDirty = true;
        return RT_OK;
      }
    }
  }
  return RT_OK;
}

rtError pxObject::removeAll()
{
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    mScene->clearMouseObject(*it);
    (*it)->mParent = NULL;
    bool isTracked = mScene->isObjectTracked((*it).getPtr());
    int refCount = (*it)->mRefCount;
    // reference count will be 2 here only if the remaining reference is in mInnerObjects.  clearing here will fix a leak
    // TODO - revisit when removing the need for mInnerObjects
    if ((isTracked == true) && (refCount == 2))
    {
      (*it)->dispose(false);
    }
  }
  mChildren.clear();

  markDirty();
  repaint();
  repaintParents();

  mScene->mDirty = true;
  return RT_OK;
}

rtError pxObject::moveToFront()
{
  pxObject* parent = this->parent();

  if(!parent) return RT_OK;

  // If this pxObject is already at the front (last child),
  // make this a no-op
  uint32_t size = (uint32_t) parent->mChildren.size();
  rtRef<pxObject> lastChild = parent->mChildren[size-1];
  if( lastChild.getPtr() == this) {
    return RT_OK;
  }

  remove();
  setParent(parent);
  markDirty();

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

rtError pxObject::moveToBack()
{
  pxObject* parent = this->parent();

  if(!parent) return RT_OK;

  // If this pxObject is already at the back (first child),
  // make this a no-op
  rtRef<pxObject> firstChild = parent->mChildren[0];
  if( firstChild.getPtr() == this) {
    return RT_OK;
  }

  remove();
  mParent = parent;
  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin();
  parent->mChildren.insert(it, this);

  markDirty();

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

/**
 * moveForward: Move this child in front of its next closest sibling in z-order, which means
 *              moving it toward end of array because last item is at top of z-order
 **/
rtError pxObject::moveForward()
{
  pxObject* parent = this->parent();

  if(!parent)
      return RT_OK;

  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin(), it_prev;
  while( it != parent->mChildren.end() )
  {
      if( it->getPtr() == this )
      {
        it_prev = it++;
        break;
      }
      it++;
  }

  if( it == parent->mChildren.end() )
      return RT_OK;

  std::iter_swap(it_prev, it);

  markDirty();

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

/**
 * moveBackward: Move this child behind its next closest sibling in z-order, which means
 *               moving it toward beginning of array because first item is at bottom of z-order
 **/
rtError pxObject::moveBackward()
{
  pxObject* parent = this->parent();

  if(!parent)
      return RT_OK;

  std::vector<rtRef<pxObject> >::iterator it = parent->mChildren.begin(), it_prev;
  while( it != parent->mChildren.end() )
  {
      if( it->getPtr() == this )
      {
          break;
      }
      it_prev = it++;
  }
  if( it == parent->mChildren.begin() )
      return RT_OK;

  std::iter_swap(it_prev, it);

  markDirty();

  parent->repaint();
  parent->repaintParents();
  mScene->mDirty = true;

  return RT_OK;
}

rtError pxObject::animateTo(const char* prop, double to, double duration,
                             uint32_t interp, uint32_t options,
                            int32_t count, rtObjectRef promise)
{
  if (mIsDisposed)
  {
    return RT_OK;
  }
  animateToInternal(prop, to, duration, ((pxConstantsAnimation*)CONSTANTS.animationConstants.getPtr())->getInterpFunc(interp),
            (pxConstantsAnimation::animationOptions)options, count, promise, rtObjectRef());
  return RT_OK;
}

// Dont fastforward when calling from set* methods since that will
// recurse indefinitely and crash and we're going to change the value in
// the set* method anyway.
void pxObject::cancelAnimation(const char* prop, bool fastforward, bool rewind)
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
      pxAnimate* pAnimateObj = (pxAnimate*) a.animateObj.getPtr();

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
        if (a.promise && a.promise.getPtr() != NULL)
        {
          a.promise.send("resolve", this);

          if (NULL != pAnimateObj)
          {
            pAnimateObj->setStatus(pxConstantsAnimation::STATUS_CANCELLED);
          }
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

      if (NULL != pAnimateObj)
      {
        pAnimateObj->update(prop, &a, pxConstantsAnimation::STATUS_CANCELLED);
      }
    }
    ++it;
  }
  mCancelInSet = f;
}

void pxObject::animateToInternal(const char* prop, double to, double duration,
                         pxInterp interp, pxConstantsAnimation::animationOptions options,
                         int32_t count, rtObjectRef promise, rtObjectRef animateObj)
{
  cancelAnimation(prop,(options & pxConstantsAnimation::OPTION_FASTFORWARD),
                       (options & pxConstantsAnimation::OPTION_REWIND));

  // schedule animation
  animation a;

  a.cancelled = false;
  a.prop     = prop;
  a.from     = get<float>(prop);
  a.to       = static_cast<float>(to);
  a.start    = -1;
  a.duration = duration;
  a.interpFunc  = interp ? interp : pxInterpLinear;
  a.options     = options;
  a.count    = count;
  a.actualCount = 0;
  a.reversing = false;
//  a.ended = onEnd;
  a.promise = promise;
  a.animateObj = animateObj;

  mAnimations.push_back(a);
  triggerUpdate();

  pxAnimate *animObj = (pxAnimate *)a.animateObj.getPtr();

  if (NULL != animObj)
  {
    animObj->update(prop, &a, pxConstantsAnimation::STATUS_INPROGRESS);
  }

  // resolve promise immediately if this is COUNT_FOREVER
  if( count == pxConstantsAnimation::COUNT_FOREVER)
  {
    if (a.ended)
      a.ended.send(this);
    if (a.promise)
      a.promise.send("resolve",this);
  }
}

void pxObject::update(double t, bool updateChildren)
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

    pxAnimate *animObj = (pxAnimate *)a.animateObj.getPtr();

    if (a.start < 0) a.start = t;
    double end = a.start + a.duration;

    // if duration has elapsed, increment the count for this animation
    if( t >=end && a.count != pxConstantsAnimation::COUNT_FOREVER
        && !(a.options & pxConstantsAnimation::OPTION_OSCILLATE))
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
      cancelAnimation(a.prop, true, false);
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
        {
          a.promise.send("resolve",this);
          if (NULL != animObj)
          {
            animObj->setStatus(pxConstantsAnimation::STATUS_ENDED);
          }
        }
        // Erase making sure to push the iterator forward before
        a.cancelled = true;
        if (NULL != animObj)
        {
          animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_ENDED);
        }
        it = mAnimations.erase(it);
        continue;
      }
#endif

    }

    if (a.cancelled)
    {
      if (NULL != animObj)
      {
        animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_CANCELLED);
      }

      it = mAnimations.erase(it);  // returns next element
      continue;
    }

    double t1 = (t-a.start)/a.duration; // Some of this could be pushed into the end handling
    double t2 = floor(t1);
    t1 = t1-t2; // 0-1

    double d = a.interpFunc(t1);
    float from = a.from;
    float   to = a.to;

    if (a.options & pxConstantsAnimation::OPTION_OSCILLATE)
    {
      bool justReverseChange = false;
      double toVal = a.to;
      if( (fmod(t2,2) != 0))  // TODO perf chk ?
      {
        if(!a.reversing)
        {
          a.reversing = true;
          justReverseChange = true;
          a.actualCount++;
        }
        from = a.to;
        to   = a.from;
      }
      else if( a.reversing && (fmod(t2,2) == 0))
      {
        toVal = a.from;
        justReverseChange = true;
        a.reversing = false;
        a.actualCount++;
        a.start = -1;
      }
      // Prevent one more loop through oscillate
      if(a.count != pxConstantsAnimation::COUNT_FOREVER && a.actualCount >= a.count )
      {
          // if(a.actualCount == a.count)
          // {
          //   justReverseChange = false;
          // }

          if (true == justReverseChange)
          {
            mCancelInSet = false;
            set(a.prop, toVal);
            mCancelInSet = true;
          }

        if (NULL != animObj)
        {
          animObj->setStatus(pxConstantsAnimation::STATUS_ENDED);
        }
        cancelAnimation(a.prop, false, false);

        if (NULL != animObj)
        {
          animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_ENDED);
        }

        it = mAnimations.erase(it);
        continue;
      }

    }

    float v = static_cast<float> (from + (to - from) * d);
    assert(mCancelInSet);
    mCancelInSet = false;
    set(a.prop, v);
    mCancelInSet = true;
    if (NULL != animObj)
    {
      animObj->update(a.prop, &a, pxConstantsAnimation::STATUS_INPROGRESS);
    }
    ++it;
  }

    pxMatrix4f m;
    if (gDirtyRectsEnabled) {
        applyMatrix(m);
        context.setMatrix(m);
        if (mIsDirty)
        {
            pxRect dirtyRect = getBoundingRectInScreenCoordinates();
            if (!dirtyRect.isEqual(mScreenCoordinates))
            {
                dirtyRect.unionRect(mScreenCoordinates);
            }
            mLastRenderMatrix = context.getMatrix();
            mScreenCoordinates = getBoundingRectInScreenCoordinates();
            if (!dirtyRect.isEqual(mScreenCoordinates))
            {
                dirtyRect.unionRect(mScreenCoordinates);
            }

            mScene->invalidateRect(&dirtyRect);
            mLastRenderMatrix = context.getMatrix();
            setDirtyRect(&dirtyRect);


        }
    }

  if (updateChildren)
  {
    // Recursively update children
    for (vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if (gDirtyRectsEnabled)
      {
        if (mIsDirty && mScreenCoordinates.isOverlapping((*it)->mScreenCoordinates))
          (*it)->markDirty();

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
        context.pushState();
      }
      // JR TODO  this lock looks suspicious... why do we need it?
      ENTERSCENELOCK()
      (*it)->update(t);
      EXITSCENELOCK()
      if (gDirtyRectsEnabled)
      {
        context.popState();
      }
    }
  }

    if (gDirtyRectsEnabled) {
        //context.setMatrix(m);
        mRenderMatrix = m;
        mIsDirty = false;
    }

  // Send promise
  sendPromise();
}

void pxObject::releaseData(bool sceneSuspended)
{
  clearSnapshot(mClipSnapshotRef);
  clearSnapshot(mDrawableSnapshotForMask);
  clearSnapshot(mMaskSnapshot);
  mSceneSuspended = sceneSuspended;
  // Recursively suspend the children
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->releaseData(sceneSuspended);
  }
}

void pxObject::reloadData(bool sceneSuspended)
{
  mSceneSuspended = sceneSuspended;
  mRepaint = true;
  // Recursively resume the children
  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    (*it)->reloadData(sceneSuspended);
  }
}

uint64_t pxObject::textureMemoryUsage(std::vector<rtObject*> &objectsCounted)
{
  uint64_t textureMemory = 0;
  if (std::find(objectsCounted.begin(), objectsCounted.end(), this) == objectsCounted.end() )
  {
    if (mClipSnapshotRef.getPtr() != NULL)
    {
      textureMemory += (mClipSnapshotRef->width() * mClipSnapshotRef->height() * 4);
    }
    if (mDrawableSnapshotForMask.getPtr() != NULL)
    {
      textureMemory += (mDrawableSnapshotForMask->width() * mDrawableSnapshotForMask->height() * 4);
    }
    if (mSnapshotRef.getPtr() != NULL)
    {
      textureMemory += (mSnapshotRef->width() * mSnapshotRef->height() * 4);
    }
    if (mMaskSnapshot.getPtr() != NULL)
    {
      textureMemory += (mMaskSnapshot->width() * mMaskSnapshot->height() * 4);
    }
    objectsCounted.push_back(this);
  }

  for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
  {
    textureMemory += (*it)->textureMemoryUsage(objectsCounted);
  }
  return textureMemory;
}

bool pxObject::needsUpdate()
{
  if ((mParent != NULL && mAnimations.size() > 0) || !((rtPromise*)mReady.getPtr())->status())
  {
    return true;
  }
  return false;
}

void pxObject::triggerUpdate()
{
  pxScene2d::updateObject(this, true);
}

//#ifdef PX_DIRTY_RECTANGLES
void pxObject::setDirtyRect(pxRect *r)
{
  if (r != NULL)
  {
    mDirtyRect.unionRect(*r);
    if (mParent != NULL)
    {
      mParent->setDirtyRect(&mDirtyRect);
    }
  }
}

void pxObject::markDirty()
{
    if (gDirtyRectsEnabled)
    {
        mIsDirty = true;
    }
}

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
//#endif //PX_DIRTY_RECTANGLES

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

  context.setAlpha(ma);

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

    if (gDirtyRectsEnabled) {
        m = mRenderMatrix;
    } else {
        applyMatrix(m); // ANIMATE !!!
    }
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

  rtLogDebug("drawInternal: %s\n", mId.cString());
  m.dump();

  pxVector4f v1(mx+w, my, 0, 1);
  rtLogDebug("Print vector top\n");
  v1.dump();

  pxVector4f result1 = m.multiply(v1);
  rtLogDebug("Print vector top after\n");
  result1.dump();

  pxVector4f v2(mx+w, my+mh, 0, 1);
  rtLogDebug("Print vector bottom\n");
  v2.dump();

  pxVector4f result2 = m.multiply(v2);
  rtLogDebug("Print vector bottom after\n");
  result2.dump();

#endif

  context.setMatrix(m);

  if ((mClip && !context.isObjectOnScreen(0,0,w,h)) || mSceneSuspended)
  {
    //rtLogInfo("pxObject::drawInternal returning because object is not on screen mw=%f mh=%f\n", mw, mh);
    return;
  }

  if (gDirtyRectsEnabled) {
    mLastRenderMatrix = context.getMatrix();
    //mScreenCoordinates = getBoundingRectInScreenCoordinates();
  }

  float c[4] = {1, 0, 0, 1};
  context.drawDiagRect(0, 0, w, h, c);

  //rtLogInfo("pxObject::drawInternal mPainting=%d mw=%f mh=%f\n", mPainting, mw, mh);
  if (mPainting)
  {
    pxConstantsMaskOperation::constants maskOp = pxConstantsMaskOperation::NORMAL; // default

    // MASKING ? ---------------------------------------------------------------------------------------------------
    bool maskFound = false;
    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      if ((*it)->mask())
      {
        //rtLogInfo("pxObject::drawInternal mask is true mw=%f mh=%f\n", mw, mh);
        maskFound = true;

        pxImage *img = dynamic_cast<pxImage *>( &*it->getPtr() ) ;
        if(img)
        {
          int32_t val;
          img->maskOp(val); // get mask operation

          maskOp = (pxConstantsMaskOperation::constants) val;
        }

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
      context.setAlpha(ma);
      //rtLogInfo("context.drawImage\n");

      context.drawImageMasked(0, 0, w, h, maskOp, mDrawableSnapshotForMask->getTexture(), mMaskSnapshot->getTexture());
    }
    // CLIPPING ? ---------------------------------------------------------------------------------------------------
    else if (mClip)
    {
      //rtLogInfo("calling createSnapshot for mw=%f mh=%f\n", mw, mh);
      if (mRepaint)
      {
        createSnapshot(mClipSnapshotRef);
        context.setMatrix(m);
        context.setAlpha(ma);
      }

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
      for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
      {
        if((*it)->drawEnabled() == false)
        {
          continue;
        }
        context.pushState();
        //rtLogInfo("calling drawInternal() mw=%f mh=%f\n", (*it)->mw, (*it)->mh);
        (*it)->drawInternal();
/*if (gDirtyRectsEnabled) {
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
 }*/
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
  if (gDirtyRectsEnabled) {
    mDirtyRect.setEmpty();
  }
}


bool pxObject::hitTestInternal(pxMatrix4f m, pxPoint2f& pt, rtRef<pxObject>& hit,
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
    for(vector<rtRef<pxObject> >::reverse_iterator it = mChildren.rbegin(); it != mChildren.rend(); ++it)
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

rtError pxObject::setPainting(bool v)
{
  mPainting = v;
  if (!mPainting)
  {
    //rtLogInfo("in setPainting and calling createSnapshot mw=%f mh=%f\n", mw, mh);
#ifdef RUNINMAIN
    createSnapshot(mSnapshotRef, false, true);
#else
    createSnapshot(mSnapshotRef, true, true);
#endif //RUNINMAIN
  }
  else
  {
    clearSnapshot(mSnapshotRef);
  }
  return RT_OK;
}


void pxObject::createSnapshot(pxContextFramebufferRef& fbo, bool separateContext,
                              bool antiAliasing)
{
  pxMatrix4f m;

//  float parentAlpha = ma;

  float parentAlpha = 1.0;
  static pxSharedContextRef sharedContext;
  if (separateContext)
  {
    if (sharedContext.getPtr() == NULL)
    {
      sharedContext = context.createSharedContext();
    }
    sharedContext->makeCurrent(true);
  }

  context.setMatrix(m);
  context.setAlpha(parentAlpha);

  float w = getOnscreenWidth();
  float h = getOnscreenHeight();

//#ifdef PX_DIRTY_RECTANGLES
  bool fullFboRepaint = false;
//#endif //PX_DIRTY_RECTANGLES

  //rtLogInfo("createSnapshot  w=%f h=%f\n", w, h);
  if (fbo.getPtr() == NULL || fbo->width() != floor(w) || fbo->height() != floor(h))
  {
    clearSnapshot(fbo);
    //rtLogInfo("createFramebuffer  mw=%f mh=%f\n", w, h);
    fbo = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)), antiAliasing);
    if (gDirtyRectsEnabled) {
       fullFboRepaint = true;
    }
  }
  else
  {
    //rtLogInfo("updateFramebuffer  mw=%f mh=%f\n", w, h);
    context.updateFramebuffer(fbo, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }
  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (mRepaint && context.setFramebuffer(fbo) == PX_OK)
  {
    //context.clear(static_cast<int>(w), static_cast<int>(h));
    if (gDirtyRectsEnabled) {
    int clearX = mDirtyRect.left();
    int clearY = mDirtyRect.top();
    int clearWidth = mDirtyRect.right() - clearX+1;
    int clearHeight = mDirtyRect.bottom() - clearY+1;

    if (!mIsDirty)
        context.clear(static_cast<int>(w), static_cast<int>(h));

    if (fullFboRepaint)
    {
        clearX = 0;
        clearY = 0;
        clearWidth = w;
        clearHeight = h;
        context.clear(clearX, clearY, clearWidth, clearHeight);
    }
   } else {
    context.clear(static_cast<int>(w), static_cast<int>(h));
   }
    draw();

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
    {
      context.pushState();
      (*it)->drawInternal();
      context.popState();
    }
  }
  context.setFramebuffer(previousRenderSurface);
  if (separateContext)
  {
    sharedContext->makeCurrent(false);
  }
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
    mDrawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }
  else
  {
    context.updateFramebuffer(mDrawableSnapshotForMask, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }

  if (mMaskSnapshot.getPtr() == NULL || mMaskSnapshot->width() != floor(w) || mMaskSnapshot->height() != floor(h))
  {
    mMaskSnapshot = context.createFramebuffer(static_cast<int>(floor(w)), static_cast<int>(floor(h)), false, true);
  }
  else
  {
    context.updateFramebuffer(mMaskSnapshot, static_cast<int>(floor(w)), static_cast<int>(floor(h)));
  }

  pxContextFramebufferRef previousRenderSurface = context.getCurrentFramebuffer();
  if (context.setFramebuffer(mMaskSnapshot) == PX_OK)
  {
    context.clear(static_cast<int>(w), static_cast<int>(h));

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
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
    context.clear(static_cast<int>(w), static_cast<int>(h));

    for(vector<rtRef<pxObject> >::iterator it = mChildren.begin(); it != mChildren.end(); ++it)
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

void pxObject::clearSnapshot(pxContextFramebufferRef fbo)
{
  if (fbo.getPtr() != NULL)
  {
    fbo->resetFbo();
  }
}



bool pxObject::onTextureReady()
{
  repaint();
  repaintParents();
  if (mScene != NULL)
  {
    mScene->invalidateRect(NULL);
  }

  markDirty();
  return false;
}

void pxObject::repaintParents()
{
  pxObject* parent = mParent;
  while (parent)
  {
    parent->repaint();
    parent = parent->parent();
  }
}

#if 0
rtDefineObject(rtPromise, rtObject);
rtDefineMethod(rtPromise, then);
rtDefineMethod(rtPromise, resolve);
rtDefineMethod(rtPromise, reject);
#endif

rtDefineObject(pxObject, rtObject);
rtDefineProperty(pxObject, _pxObject);
rtDefineProperty(pxObject, parent);
rtDefineProperty(pxObject, children);
rtDefineProperty(pxObject, x);
rtDefineProperty(pxObject, y);
rtDefineProperty(pxObject, w);
rtDefineProperty(pxObject, h);
rtDefineProperty(pxObject, px);
rtDefineProperty(pxObject, py);
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
rtDefineMethod(pxObject, moveToBack);
rtDefineMethod(pxObject, moveForward);
rtDefineMethod(pxObject, moveBackward);
rtDefineMethod(pxObject, releaseResources);
//rtDefineMethod(pxObject, animateTo);
#if 0
//TODO - remove
rtDefineMethod(pxObject, animateToF2);
#endif
rtDefineMethod(pxObject, animateToP2);
rtDefineMethod(pxObject, animateToObj);
rtDefineMethod(pxObject, addListener);
rtDefineMethod(pxObject, delListener);
//rtDefineProperty(pxObject, emit);
//rtDefineProperty(pxObject, onReady);
rtDefineMethod(pxObject, getObjectById);
