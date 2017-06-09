/*

 pxCore Copyright 2005-2017 John Robinson

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

// pxAnimate.cpp

#include "pxAnimate.h"
#include "pxScene2d.h"

static rtString mapStatus(pxConstantsAnimation::animationStatus status)
{
  switch(status) 
  { 
     case pxConstantsAnimation::STATUS_IDLE:        return "IDLE"; // aka "STOPPED"
     case pxConstantsAnimation::STATUS_CANCELLED:   return "CANCELLED";
     case pxConstantsAnimation::STATUS_ENDED:       return "ENDED";
 
    default: 
      return "UNKNOWN"; 
  }
}

pxAnimate::pxAnimate (rtObjectRef props, uint32_t interp, pxConstantsAnimation::animationOptions type, double duration, int32_t count, rtObjectRef promise, rtRef<pxObject> animateObj):mProps(props), mInterp(interp), mType(type), mProvisionedDuration(duration), mProvisionedCount(count), mCancelled(false), mStatus(pxConstantsAnimation::STATUS_IDLE), mDonePromise(promise), mAnimatedObj(animateObj)
{
  if (NULL != props.getPtr())
  {
    mCurrDetails = new rtMapObject;
    rtObjectRef keys = mProps.get<rtObjectRef>("allKeys");
    if (keys)
    {
      uint32_t len = keys.get<uint32_t>("length");
      for (uint32_t i = 0; i < len; i++)
      {
        rtObjectRef params = new pxAnimate::pxAnimationParams();
        pxAnimate::pxAnimationParams* propParamsPtr = (pxAnimate::pxAnimationParams*) params.getPtr();
        if (NULL != propParamsPtr)
        {
          rtString key = keys.get<rtString>(i);

          propParamsPtr->mStatus    = pxConstantsAnimation::STATUS_IDLE;
          propParamsPtr->mCancelled = false;
          propParamsPtr->mCount     = 0;
          propParamsPtr->mFrom      = 0;
          propParamsPtr->mTo        = 0;
          propParamsPtr->mDuration  = 0;

          mCurrDetails.set(key,params);
        }
      }//FOR
    }
  }
}

pxAnimate::~pxAnimate()
{
}

rtError pxAnimate::cancel ()
{
  mCancelled = true;
  if (NULL != mProps.getPtr())
  {
    // cancel all the properties of animation
    rtObjectRef keys = mProps.get<rtObjectRef>("allKeys");
    if (keys)
    {
      uint32_t len = keys.get<uint32_t>("length");
      for (uint32_t i = 0; i < len; i++)
      {
        rtString key = keys.get<rtString>(i);
        mAnimatedObj.getPtr()->cancelAnimation(key, (mType & pxConstantsAnimation::OPTION_FASTFORWARD), (mType & pxConstantsAnimation::OPTION_REWIND), true);
      }
    }
  }
  mStatus = pxConstantsAnimation::STATUS_CANCELLED;
  return RT_OK;
}

void pxAnimate::setStatus (pxConstantsAnimation::animationStatus status)
{
  if (status > mStatus)
  {
    mStatus = status;
  }
}

void pxAnimate::update (const char* prop, struct animation* params, pxConstantsAnimation::animationStatus status)
{
  if (NULL != mCurrDetails.getPtr())
  {
    rtObjectRef propParams = mCurrDetails.get<rtObjectRef>(prop);
    pxAnimate::pxAnimationParams* propParamsPtr = (pxAnimate::pxAnimationParams*) propParams.getPtr();

    if (propParamsPtr != NULL)
    {
      if (status > propParamsPtr->mStatus)
      {
        propParamsPtr->mStatus = status;
      }

      if (propParamsPtr->mStatus != pxConstantsAnimation::STATUS_ENDED)
      {
        propParamsPtr->mCancelled = params->cancelled;
      }

      propParamsPtr->mCount    = params->actualCount;
      propParamsPtr->mDuration = params->duration;
      propParamsPtr->mFrom     = params->from;
      propParamsPtr->mTo       = params->to;
    }
  }
}

rtError pxAnimate::status(rtString& v) const
{ 
  v = mapStatus(mStatus);
  return RT_OK;
};

rtError pxAnimate::pxAnimationParams::status(rtString& v) const
{
  v = mapStatus(mStatus);
  return RT_OK;
};

rtDefineObject(pxAnimate, rtObject);

rtDefineProperty(pxAnimate, status);
rtDefineProperty(pxAnimate, done);
rtDefineProperty(pxAnimate, props);
rtDefineProperty(pxAnimate, interp);
rtDefineProperty(pxAnimate, type);
rtDefineProperty(pxAnimate, provduration);
rtDefineProperty(pxAnimate, provcount);
rtDefineProperty(pxAnimate, cancelled);
rtDefineProperty(pxAnimate, details);

rtDefineMethod(pxAnimate, cancel);

rtDefineObject(pxAnimate::pxAnimationParams, rtObject);

rtDefineProperty(pxAnimate::pxAnimationParams, status);
rtDefineProperty(pxAnimate::pxAnimationParams, from);
rtDefineProperty(pxAnimate::pxAnimationParams, to);
rtDefineProperty(pxAnimate::pxAnimationParams, duration);
rtDefineProperty(pxAnimate::pxAnimationParams, cancelled);
rtDefineProperty(pxAnimate::pxAnimationParams, count);
