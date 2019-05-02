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

// pxText.h

#include "pxVideo.h"
#include "pxContext.h"

extern pxContext context;

pxVideo::pxVideo(pxScene2d* scene):pxObject(scene), mVideoTexture()
#ifdef ENABLE_SPARK_VIDEO_PUNCHTHROUGH
 ,mEnablePunchThrough(true)
#else
, mEnablePunchThrough(false)
#endif //ENABLE_SPARK_VIDEO_PUNCHTHROUGH
{
}

void pxVideo::onInit()
{
  mReady.send("resolve",this);
  pxObject::onInit();
}

void pxVideo::draw()
{
  if (!isRotated() && mEnablePunchThrough)
  {
    int screenX = 0;
    int screenY = 0;
    context.mapToScreenCoordinates(0,0,screenX, screenY);
    context.punchThrough(screenX,screenY,mw, mh);
  }
  else
  {
    // TODO - remove red rectangle code and uncomment code below when using video texture
    static float redColor[4] = {1.0, 0.0, 0.0, 1.0};
    context.drawRect(mw, mh, 1.0, redColor, redColor);

    //TODO - uncomment code below and remove red rectangle when adding video texture support
    /*
    static pxTextureRef nullMaskRef;
    context.drawImage(0, 0, mw, mh, mVideoTexture, nullMaskRef);
    }*/
  }
}

bool pxVideo::isRotated()
{
  pxMatrix4f matrix = context.getMatrix();
  float *f = matrix.data();
  const float e= 1.0e-2;

  if ( (fabsf(f[1]) > e) ||
       (fabsf(f[2]) > e) ||
       (fabsf(f[4]) > e) ||
       (fabsf(f[6]) > e) ||
       (fabsf(f[8]) > e) ||
       (fabsf(f[9]) > e) )
  {
    return true;
  }

  return false;
}

//properties
rtError pxVideo::availableAudioLanguages(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::availableClosedCaptionsLanguages(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::availableSpeeds(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::duration(float& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::zoom(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setZoom(const char* /*s*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::volume(uint32_t& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setVolume(uint32_t /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::closedCaptionsOptions(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setClosedCaptionsOptions(rtObjectRef /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::closedCaptionsLanguage(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setClosedCaptionsLanguage(const char* /*s*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::contentOptions(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setContentOptions(rtObjectRef /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::speed(float& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setSpeedProperty(float /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::position(float& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setPosition(float /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::audioLanguage(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setAudioLanguage(const char* /*s*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::secondaryAudioLanguage(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setSecondaryAudioLanguage(const char* /*s*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::url(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setUrl(const char* /*s*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::tsbEnabled(bool& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setTsbEnabled(bool /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::closedCaptionsEnabled(bool& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setClosedCaptionsEnabled(bool /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::autoPlay(bool& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setAutoPlay(bool /*v*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::play()
{
  //TODO
  return RT_OK;
}

rtError pxVideo::pause()
{
  //TODO
  return RT_OK;
}

rtError pxVideo::stop()
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setSpeed(float /*speed*/, float /*overshootCorrection*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setPositionRelative(float /*seconds*/)
{
  //TODO
  return RT_OK;
}

rtError pxVideo::requestStatus()
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setAdditionalAuth(rtObjectRef /*params*/)
{
  //TODO
  return RT_OK;
}

rtDefineObject(pxVideo, pxObject);
rtDefineProperty(pxVideo, availableAudioLanguages);
rtDefineProperty(pxVideo, availableClosedCaptionsLanguages);
rtDefineProperty(pxVideo, availableSpeeds);
rtDefineProperty(pxVideo, duration);
rtDefineProperty(pxVideo, zoom);
rtDefineProperty(pxVideo, volume);
rtDefineProperty(pxVideo, closedCaptionsOptions);
rtDefineProperty(pxVideo, closedCaptionsLanguage);
rtDefineProperty(pxVideo, contentOptions);
rtDefineProperty(pxVideo, speed);
rtDefineProperty(pxVideo, position);
rtDefineProperty(pxVideo, audioLanguage);
rtDefineProperty(pxVideo, secondaryAudioLanguage);
rtDefineProperty(pxVideo, url);
rtDefineProperty(pxVideo, tsbEnabled);
rtDefineProperty(pxVideo, closedCaptionsEnabled);
rtDefineProperty(pxVideo, autoPlay);
rtDefineMethod(pxVideo, play);
rtDefineMethod(pxVideo, pause);
rtDefineMethod(pxVideo, stop);
rtDefineMethod(pxVideo, setSpeed);
rtDefineMethod(pxVideo, setPositionRelative);
rtDefineMethod(pxVideo, requestStatus);
rtDefineMethod(pxVideo, setAdditionalAuth);

