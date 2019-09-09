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

#include <gst/gst.h>
#include "main_aamp.h"
#include "pxScene2d.h"
#include "pxObject.h"
#include "pxContext.h"

//#define AAMP_USE_SHADER 1

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
  virtual ~pxVideo();

  virtual void onInit();

  //properties
  virtual rtError availableAudioLanguages(rtObjectRef& v) const;
  virtual rtError availableClosedCaptionsLanguages(rtObjectRef& v) const;
  virtual rtError availableSpeeds(rtObjectRef& v) const;
  virtual rtError duration(float& v) const;
  virtual rtError zoom(rtString& v) const;
  virtual rtError setZoom(const char* s);
  virtual rtError volume(uint32_t& v) const;
  virtual rtError setVolume(uint32_t v);
  virtual rtError closedCaptionsOptions(rtObjectRef& v) const;
  virtual rtError setClosedCaptionsOptions(rtObjectRef v);
  virtual rtError closedCaptionsLanguage(rtString& v) const;
  virtual rtError setClosedCaptionsLanguage(const char* s);
  virtual rtError contentOptions(rtObjectRef& v) const;
  virtual rtError setContentOptions(rtObjectRef v);
  virtual rtError speed(float& v) const;
  virtual rtError setSpeedProperty(float v);
  virtual rtError position(float& v) const;
  virtual rtError setPosition(float v);
  virtual rtError audioLanguage(rtString& v) const;
  virtual rtError setAudioLanguage(const char* s);
  virtual rtError secondaryAudioLanguage(rtString& v) const;
  virtual rtError setSecondaryAudioLanguage(const char* s);
  virtual rtError url(rtString& url) const;
  virtual rtError setUrl(const char* s);
  virtual rtError tsbEnabled(bool& v) const;
  virtual rtError setTsbEnabled(bool v);
  virtual rtError closedCaptionsEnabled(bool& v) const;
  virtual rtError setClosedCaptionsEnabled(bool v);
  virtual rtError autoPlay(bool& autoPlay) const;
  virtual rtError setAutoPlay(bool v);

  //methods
  virtual rtError play();
  virtual rtError pause();
  virtual rtError stop();
  virtual rtError setSpeed(float speed, float overshootCorrection );
  virtual rtError setPositionRelative(float seconds);
  virtual rtError requestStatus();
  virtual rtError setAdditionalAuth(rtObjectRef params);
  
  virtual void draw();

  void updateYUVFrame(uint8_t *yuvBuffer, int size, int pixel_w, int pixel_h);

private:
  void InitPlayerLoop();
  void TermPlayerLoop();
  static void* AAMPGstPlayer_StreamThread(void *arg);
  static void newAampFrame(void* context, void* data);

private:
    static GMainLoop *AAMPGstPlayerMainLoop;

    bool isRotated();
    bool mEnablePunchThrough;
    bool mAutoPlay;
    rtString mUrl;

    class PlayerInstanceAAMP* mAamp;

    rtMutex mYuvFrameMutex;
    pxOffscreen mOffscreen;
    struct YUVBUFFER{
    	uint8_t *buffer;
    	int size;
    	int pixel_w;
    	int pixel_h;
    };
    YUVBUFFER mYuvBuffer;
    bool initialized = false;
    GThread *aampMainLoopThread;

public:
    static pxVideo *pxVideoObj; //This object
};

#endif // PX_VIDEO_H
