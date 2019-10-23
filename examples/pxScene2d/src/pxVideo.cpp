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

#include <iostream>
#include <string>
#include "pxVideo.h"

using namespace std::placeholders;

pxVideo *pxVideo::pxVideoObj = NULL;
GMainLoop *pxVideo::AAMPGstPlayerMainLoop;

extern pxContext context;
extern rtThreadQueue* gUIThreadQueue;

/**
 * @brief Thread to run mainloop (for standalone mode)
 * @param[in] arg user_data
 * @retval void pointer
 */
void* pxVideo::AAMPGstPlayer_StreamThread(void *arg)
{
  if (AAMPGstPlayerMainLoop)
  {
    g_main_loop_run(AAMPGstPlayerMainLoop); // blocks
    printf("AAMPGstPlayer_StreamThread: exited main event loop\n");
  }
  g_main_loop_unref(AAMPGstPlayerMainLoop);
  AAMPGstPlayerMainLoop = NULL;
  return NULL;
}

/**
 * @brief To initialize Gstreamer and start mainloop (for standalone mode)
 * @param[in] argc number of arguments
 * @param[in] argv array of arguments
 */
void pxVideo::InitPlayerLoop()
{
  if (!initialized)
  {
    initialized = true;
    gst_init(NULL, NULL);
    AAMPGstPlayerMainLoop = g_main_loop_new(NULL, FALSE);
    aampMainLoopThread = g_thread_new("AAMPGstPlayerLoop", &pxVideo::AAMPGstPlayer_StreamThread, NULL );
  }
}

void pxVideo::TermPlayerLoop()
{
	if(AAMPGstPlayerMainLoop)
	{
		g_main_loop_quit(AAMPGstPlayerMainLoop);
		g_thread_join(aampMainLoopThread);
		gst_deinit ();
		printf("%s(): Exited GStreamer MainLoop.\n", __FUNCTION__);
	}
}

pxVideo::pxVideo(pxScene2d* scene):pxObject(scene)
#ifdef ENABLE_SPARK_VIDEO_PUNCHTHROUGH
 ,mEnablePunchThrough(true)
#else
, mEnablePunchThrough(false)
#endif //ENABLE_SPARK_VIDEO_PUNCHTHROUGH
,mAutoPlay(false)
,mUrl("")
{
	  aampMainLoopThread = NULL;
	  AAMPGstPlayerMainLoop = NULL;
	  InitPlayerLoop();

	  std::function< void(uint8_t *, int, int, int) > cbExportFrames = nullptr;
	  if(!mEnablePunchThrough)
	  {
		  //Keeping this block to dynamically turn punch through on/off
		  //Spark will render frames
		  cbExportFrames = std::bind(&pxVideo::updateYUVFrame, this, _1, _2, _3, _4);
	  }
	  mAamp = new PlayerInstanceAAMP(NULL
#ifndef ENABLE_SPARK_VIDEO_PUNCHTHROUGH //TODO: Remove this check, once the official builds contain the second argument to PlayerInstanceAAMP
			  , cbExportFrames
#endif
			  );
	  assert (nullptr != mAamp);
	  pxVideo::pxVideoObj = this;
	  mYuvBuffer.buffer = NULL;
	  registerMediaMetadataEventListener();
	  registerSpeedsChangedEventListener();
}

pxVideo::~pxVideo()
{
	mAamp->Stop();
	unregisterEventsListeners();
	delete mAamp;
	TermPlayerLoop();
}

void pxVideo::onInit()
{
	rtLogError("%s:%d.",__FUNCTION__,__LINE__);
	if(mAutoPlay)
	{
		play();
	}
  mReady.send("resolve",this);
  pxObject::onInit();
}

void pxVideo::newAampFrame(void* context, void* data)
{
	pxVideo* videoObj = reinterpret_cast<pxVideo*>(context);
	if (videoObj)
	{
		videoObj->onTextureReady();
	}
}

inline unsigned char RGB_ADJUST(double tmp)
{
	return (unsigned char)((tmp >= 0 && tmp <= 255)?tmp:(tmp < 0 ? 0 : 255));
}

void CONVERT_YUV420PtoRGBA32(unsigned char* yuv420buf,unsigned char* rgbOutBuf,int nWidth,int nHeight)
{
	unsigned char Y,U,V,R,G,B;
	unsigned char* yPlane,*uPlane,*vPlane;
	int rgb_width , u_width;
	rgb_width = nWidth * 4;
	u_width = (nWidth >> 1);
	int offSet = 0;

	yPlane = yuv420buf;
	uPlane = yuv420buf + nWidth*nHeight;
	vPlane = uPlane + nWidth*nHeight/4;

	for(int i = 0; i < nHeight; i++)
	{
		for(int j = 0; j < nWidth; j ++)
		{
			Y = *(yPlane + nWidth * i + j);
			offSet = (i>>1) * (u_width) + (j>>1);
			V = *(uPlane + offSet);
			U = *(vPlane + offSet);

			//  R,G,B values
			R = RGB_ADJUST((Y + (1.4075 * (V - 128))));
			G = RGB_ADJUST((Y - (0.3455 * (U - 128) - 0.7169 * (V - 128))));
			B = RGB_ADJUST((Y + (1.7790 * (U - 128))));
			offSet = rgb_width * i + j * 4;

			rgbOutBuf[offSet] = B;
			rgbOutBuf[offSet + 1] = G;
			rgbOutBuf[offSet + 2] = R;
			rgbOutBuf[offSet + 3] = 255;
		}
	}
}

void pxVideo::updateYUVFrame(uint8_t *yuvBuffer, int size, int pixel_w, int pixel_h)
{
	/** Input in I420 (YUV420) format.
	  * Buffer structure:
	  * ----------
	  * |        |
	  * |   Y    | size = w*h
	  * |        |
	  * |________|
	  * |   U    |size = w*h/4
	  * |--------|
	  * |   V    |size = w*h/4
	  * ----------*
	  */
	if(yuvBuffer)
	{
		mYuvFrameMutex.lock();
		if (mYuvBuffer.buffer == NULL)
		{
			uint8_t *buffer = (uint8_t *) malloc(size);
			memcpy(buffer, yuvBuffer, size);
			mYuvBuffer.buffer = buffer;
			mYuvBuffer.size = size;
			mYuvBuffer.pixel_w = pixel_w;
			mYuvBuffer.pixel_h = pixel_h;
		}
		mYuvFrameMutex.unlock();

		gUIThreadQueue->addTask(newAampFrame, pxVideo::pxVideoObj, NULL);
	}
}

void pxVideo::draw()
{
  if (mEnablePunchThrough && !isRotated())
  {
    int screenX = 0;
    int screenY = 0;
    context.mapToScreenCoordinates(0,0,screenX, screenY);
    context.punchThrough(screenX,screenY,mw, mh);
  }
  else
  {
	YUVBUFFER yuvBuffer{NULL,0,0,0};
	mYuvFrameMutex.lock();
	if(mYuvBuffer.buffer)
	{
		yuvBuffer = mYuvBuffer;
		mYuvBuffer.buffer = NULL;
	}
	mYuvFrameMutex.unlock();

	if(yuvBuffer.buffer)
	{
		static pxTextureRef nullMaskRef;

		int rgbLen = yuvBuffer.pixel_w * yuvBuffer.pixel_h*4;
		uint8_t *buffer_convert = (uint8_t *) malloc(rgbLen);
		CONVERT_YUV420PtoRGBA32(yuvBuffer.buffer,buffer_convert,yuvBuffer.pixel_w, yuvBuffer.pixel_h);

		mOffscreen.init(yuvBuffer.pixel_w, yuvBuffer.pixel_h);
		mOffscreen.setBase(buffer_convert);
		pxTextureRef videoFrame = context.createTexture(mOffscreen);
		context.drawImage(0, 0, mw, mh,  videoFrame, nullMaskRef, false, NULL, pxConstantsStretch::STRETCH, pxConstantsStretch::STRETCH);
		free(yuvBuffer.buffer);
	}
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
rtError pxVideo::availableAudioLanguages(rtObjectRef& languages) const
{
	rtRef<rtArrayObject> array = new rtArrayObject;
	for (int i = 0; i < mPlaybackMetadata.languageCount; i++)
	{
		array->pushBack(mPlaybackMetadata.languages[i]);
	}

	languages = array;
  return RT_OK;
}

rtError pxVideo::availableClosedCaptionsLanguages(rtObjectRef& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::availableSpeeds(rtObjectRef& speeds) const
{
	rtRef<rtArrayObject> array = new rtArrayObject;
	for (int i = 0; i < mPlaybackMetadata.supportedSpeedCount; i++)
	{
		array->pushBack(mPlaybackMetadata.supportedSpeeds[i]);
	}

	speeds = array;
	return RT_OK;
}

rtError pxVideo::duration(float& duration) const
{
	if (mAamp)
	{
		duration = mAamp->GetPlaybackDuration();
	}

	return RT_OK;
}

rtError pxVideo::zoom(rtString& /*v*/) const
{
  //TODO
  return RT_OK;
}

rtError pxVideo::setZoom(const char* zoom)
{
	if (!mAamp || NULL == zoom)
	{
		return RT_OK;
	}

	VideoZoomMode zoomMode = VIDEO_ZOOM_FULL;
	if (0 != strcmp(zoom, "full"))
	{
		zoomMode = VIDEO_ZOOM_NONE;
	}

	rtLogInfo("%s:%d: zoom mode: %s",__FUNCTION__,__LINE__, zoomMode == VIDEO_ZOOM_FULL ? "Full" : "None");
	mAamp->SetVideoZoom(zoomMode);

	return RT_OK;
}

rtError pxVideo::volume(int& volume) const
{
	if (mAamp)
	{
		volume = mAamp->GetAudioVolume();
	}
	return RT_OK;
}

rtError pxVideo::setVolume(int volume)
{
	if (mAamp)
	{
		mAamp->SetAudioVolume(volume);
	}
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

rtError pxVideo::speed(int& speed) const
{
	if (mAamp)
	{
		speed = mAamp->GetPlaybackRate();
	}

  return RT_OK;
}

rtError pxVideo::setSpeedProperty(int speed)
{
	if(mAamp)
	{
		 mAamp->SetRate(speed);
	}
	return RT_OK;
}

rtError pxVideo::position(double& position) const
{
	if (mAamp)
	{
		position = mAamp->GetPlaybackPosition();
	}

	return RT_OK;
}

rtError pxVideo::setPosition(double position)
{
	if (mAamp)
	{
		double playbackDuration = mAamp->GetPlaybackDuration();

		if (position < 0)
		{
			position = 0;
		}
		else if (position > playbackDuration)
		{
			position = playbackDuration;
		}

		rtLogInfo("%s:%d: Seek to %.2fs",__FUNCTION__,__LINE__, position);
		mAamp->Seek(position);
	}
	return RT_OK;
}

rtError pxVideo::audioLanguage(rtString& language) const
{
	if (mAamp)
	{
		language = mAamp->GetCurrentAudioLanguage();
	}
	return RT_OK;
}

rtError pxVideo::setAudioLanguage(const char* language)
{
	if (mAamp && language != NULL)
	{
		mAamp->SetLanguage(language);
	}
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

rtError pxVideo::url(rtString& url) const
{
	url = mUrl;
	return RT_OK;
}

rtError pxVideo::setUrl(const char* url)
{
	bool changingURL = false;
	if(!mUrl.isEmpty())
	{
		changingURL = true;
		stop();
	}

	mUrl = rtString(url);

	if(changingURL && mAutoPlay)
	{
		play();
	}
	rtLogError("%s:%d: URL[%s].",__FUNCTION__,__LINE__,url);
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

rtError pxVideo::autoPlay(bool& autoPlay) const
{
	autoPlay = mAutoPlay;
	return RT_OK;
}

rtError pxVideo::setAutoPlay(bool value)
{
	mAutoPlay = value;
	rtLogError("%s:%d: autoPlay[%s].",__FUNCTION__,__LINE__,value?"TRUE":"FALSE");
	return RT_OK;
}

rtError pxVideo::play()
{
	rtLogError("%s:%d.",__FUNCTION__,__LINE__);
	if(!mUrl.isEmpty())
	{
		mAamp->Tune(mUrl.cString());
	}
	return RT_OK;
}

rtError pxVideo::pause()
{
	if(mAamp)
	{
		mAamp->SetRate(0);
	}
	return RT_OK;
}

rtError pxVideo::stop()
{
	if(mAamp)
	{
		mAamp->Stop();
	}
	return RT_OK;
}

rtError pxVideo::setSpeed(int speed, int overshootCorrection)
{
	if(mAamp)
	{
		mAamp->SetRate(speed, overshootCorrection);
	}
	return RT_OK;
}

rtError pxVideo::setPositionRelative(double relativePosition)
{
	if (mAamp)
	{
		double currentPosition = mAamp->GetPlaybackPosition();
		double playbackDuration = mAamp->GetPlaybackDuration();
		double relativePositionToTuneTime = currentPosition + relativePosition;

		if (relativePositionToTuneTime < 0)
		{
			relativePositionToTuneTime = 0;
		}
		else if (relativePositionToTuneTime > playbackDuration)
		{
			relativePositionToTuneTime = playbackDuration;
		}

		rtLogInfo("%s:%d: Seek %.2f seconds from %.2fs to %.2fs",__FUNCTION__,__LINE__, relativePosition, currentPosition, relativePositionToTuneTime);
		mAamp->Seek(relativePositionToTuneTime);
	}
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

class MediaMetadataListener : public AAMPEventListener
{
public:

	MediaMetadataListener(PlaybackMetadata& metadata)
	: mMetadata(metadata) {	}

	~MediaMetadataListener() = default;

	void Event(const AAMPEvent& event) override
	{
		assert(AAMP_EVENT_MEDIA_METADATA == event.type);

		mMetadata.languageCount = event.data.metadata.languageCount;
		for (int i = 0; i < mMetadata.languageCount; i++)
		{
			strncpy(mMetadata.languages[i], event.data.metadata.languages[i], MAX_LANGUAGE_TAG_LENGTH);
		}

		mMetadata.supportedSpeedCount = event.data.metadata.supportedSpeedCount;
		for (int i = 0; i < mMetadata.supportedSpeedCount; i++)
		{
			mMetadata.supportedSpeeds[i] = event.data.metadata.supportedSpeeds[i];
		}
	}

	private:

	PlaybackMetadata& mMetadata;
};

class SpeedsChangeListener : public AAMPEventListener
{
public:

	SpeedsChangeListener(PlaybackMetadata& metadata)
	: mMetadata(metadata) {	}

	~SpeedsChangeListener() = default;

	void Event(const AAMPEvent& event) override
	{
		assert(AAMP_EVENT_SPEEDS_CHANGED == event.type);

		mMetadata.supportedSpeedCount = event.data.speedsChanged.supportedSpeedCount;
		for (int i = 0; i < mMetadata.supportedSpeedCount; i++)
		{
			mMetadata.supportedSpeeds[i] = event.data.speedsChanged.supportedSpeeds[i];
		}
	}

	private:

	PlaybackMetadata& mMetadata;
};

void pxVideo::registerMediaMetadataEventListener()
{
	if (mAamp)
	{
		auto listener = std::make_unique<MediaMetadataListener>(mPlaybackMetadata);

		mAamp->AddEventListener(AAMPEventType::AAMP_EVENT_MEDIA_METADATA,  listener.get());
		mEventsListeners[AAMPEventType::AAMP_EVENT_MEDIA_METADATA] = std::move(listener);
	}
}

void pxVideo::registerSpeedsChangedEventListener()
{
	if (mAamp)
	{
		auto listener = std::make_unique<SpeedsChangeListener>(mPlaybackMetadata);

		mAamp->AddEventListener(AAMPEventType::AAMP_EVENT_SPEEDS_CHANGED,  listener.get());
		mEventsListeners[AAMPEventType::AAMP_EVENT_SPEEDS_CHANGED] = std::move(listener);
	}
}


void pxVideo::unregisterEventsListeners()
{
	if (mAamp)
	{
		for (auto it = mEventsListeners.begin(); it != mEventsListeners.end(); it++)
		{
			mAamp->RemoveEventListener(it->first, it->second.get());
			mEventsListeners.erase(it);
		}
	}
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

