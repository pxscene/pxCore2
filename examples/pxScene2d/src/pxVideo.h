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

// pxRectangle.h

#ifndef PX_VIDEO_H
#define PX_VIDEO_H

#include "pxScene2d.h"
#include "pxObject.h"

class pxVideo: public pxObject
{
public:
  rtDeclareObject(pxVideo, pxObject);

  rtReadOnlyProperty(availableAudioLanguages, availableAudioLanguages, rtObjectRef);
  rtReadOnlyProperty(availableClosedCaptionsLanguages, availableClosedCaptionsLanguages, rtObjectRef);
  rtReadOnlyProperty(availableSpeeds, availableSpeeds, rtObjectRef);
  rtReadOnlyProperty(duration, duration, float);
  rtProperty(zoom, zoom, setZoom, rtString);
  rtProperty(volume, volume, setVolume, uint32_t);
  rtProperty(closedCaptionsOptions, closedCaptionsOptions, setClosedCaptionsOptions, rtObjectRef);
  rtProperty(closedCaptionsLanguage, closedCaptionsLanguage, setClosedCaptionsLanguage, rtString);
  rtProperty(contentOptions, contentOptions, setContentOptions, rtObjectRef);
  rtProperty(speed, speed, setSpeedProperty, float);
  rtProperty(position, position, setPosition, float);
  rtProperty(audioLanguage, audioLanguage, setAudioLanguage, rtString);
  rtProperty(secondaryAudioLanguage, secondaryAudioLanguage, setSecondaryAudioLanguage, rtString);
  rtProperty(url, url, setUrl, rtString);
  rtProperty(tsbEnabled, tsbEnabled, setTsbEnabled, bool);
  rtProperty(closedCaptionsEnabled, closedCaptionsEnabled, setClosedCaptionsEnabled, bool);
  rtProperty(autoPlay, autoPlay, setAutoPlay, bool);

  rtMethodNoArgAndNoReturn("play", play);
  rtMethodNoArgAndNoReturn("pause", pause);
  rtMethodNoArgAndNoReturn("stop", stop);
  rtMethod2ArgAndNoReturn("setSpeed", setSpeed, float, float);
  rtMethod1ArgAndNoReturn("setPositionRelative", setPositionRelative, float);
  rtMethodNoArgAndNoReturn("requestStatus", requestStatus);
  rtMethod1ArgAndNoReturn("setAdditionalAuth", setAdditionalAuth, rtObjectRef);


  pxVideo(pxScene2d* scene);

  virtual void onInit();

  //properties
  rtError availableAudioLanguages(rtObjectRef& v) const;
  rtError availableClosedCaptionsLanguages(rtObjectRef& v) const;
  rtError availableSpeeds(rtObjectRef& v) const;
  rtError duration(float& v) const;
  rtError zoom(rtString& v) const;
  rtError setZoom(const char* s);
  rtError volume(uint32_t& v) const;
  rtError setVolume(uint32_t v);
  rtError closedCaptionsOptions(rtObjectRef& v) const;
  rtError setClosedCaptionsOptions(rtObjectRef v);
  rtError closedCaptionsLanguage(rtString& v) const;
  rtError setClosedCaptionsLanguage(const char* s);
  rtError contentOptions(rtObjectRef& v) const;
  rtError setContentOptions(rtObjectRef v);
  rtError speed(float& v) const;
  rtError setSpeedProperty(float v);
  rtError position(float& v) const;
  rtError setPosition(float v);
  rtError audioLanguage(rtString& v) const;
  rtError setAudioLanguage(const char* s);
  rtError secondaryAudioLanguage(rtString& v) const;
  rtError setSecondaryAudioLanguage(const char* s);
  rtError url(rtString& v) const;
  rtError setUrl(const char* s);
  rtError tsbEnabled(bool& v) const;
  rtError setTsbEnabled(bool v);
  rtError closedCaptionsEnabled(bool& v) const;
  rtError setClosedCaptionsEnabled(bool v);
  rtError autoPlay(bool& v) const;
  rtError setAutoPlay(bool v);

  //methods
  rtError play();
  rtError pause();
  rtError stop();
  rtError setSpeed(float speed, float overshootCorrection );
  rtError setPositionRelative(float seconds);
  rtError requestStatus();
  rtError setAdditionalAuth(rtObjectRef params);
  
  virtual void draw();

private:
    bool isRotated();
    pxTextureRef mVideoTexture;
    bool mEnablePunchThrough;
};

#endif // PX_VIDEO_H