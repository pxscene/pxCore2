/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

/**
 * @file aampgstplayer.h
 * @brief Gstreamer based player for AAMP
 */

#ifndef AAMPGSTPLAYER_H
#define AAMPGSTPLAYER_H

#include <stddef.h>
#include <functional>
#include <gst/gst.h>
#include "priv_aamp.h"

/**
 * @struct AAMPGstPlayerPriv
 * @brief forward declaration of AAMPGstPlayerPriv
 */
struct AAMPGstPlayerPriv;

/**
 * @class AAMPGstPlayer
 * @brief Class declaration of Gstreamer based player
 */
class AAMPGstPlayer : public StreamSink
{
public:
	class PrivateInstanceAAMP *aamp;
	void Configure(StreamOutputFormat format, StreamOutputFormat audioFormat, bool bESChangeStatus);
	void Send(MediaType mediaType, const void *ptr, size_t len, double fpts, double fdts, double duration);
	void Send(MediaType mediaType, GrowableBuffer* buffer, double fpts, double fdts, double duration);
	void EndOfStreamReached(MediaType type);
	void Stream(void);
	void Stop(bool keepLastFrame);
	void DumpStatus(void);
	void Flush(double position, int rate);
	void SelectAudio(int index);
	bool Pause(bool pause);
	long GetPositionMilliseconds(void);
	unsigned long getCCDecoderHandle(void);
	void SetVideoRectangle(int x, int y, int w, int h);
	bool Discontinuity( MediaType mediaType);
	void SetVideoZoom(VideoZoomMode zoom);
	void SetVideoMute(bool muted);
	void SetAudioVolume(int volume);
	void setVolumeOrMuteUnMute(void);
	bool IsCacheEmpty(MediaType mediaType);
	void NotifyFragmentCachingComplete();
	void GetVideoSize(int &w, int &h);
	void QueueProtectionEvent(const char *protSystemId, const void *ptr, size_t len);
	void ClearProtectionEvent();

	struct AAMPGstPlayerPriv *privateContext;
	AAMPGstPlayer(PrivateInstanceAAMP *aamp
#ifdef AAMP_RENDER_IN_APP
	, std::function< void(uint8_t *, int, int, int) > exportFrames = nullptr
#endif
	);
	AAMPGstPlayer(const AAMPGstPlayer&) = delete;
	AAMPGstPlayer& operator=(const AAMPGstPlayer&) = delete;
	~AAMPGstPlayer();
	static void InitializeAAMPGstreamerPlugins();
	void NotifyEOS();
	void NotifyFirstFrame(MediaType type);
	void DumpDiagnostics();
	void SignalTrickModeDiscontinuity();

#ifdef AAMP_RENDER_IN_APP
	std::function< void(uint8_t *, int, int, int) > cbExportYUVFrame;
	static GstFlowReturn AAMPGstPlayer_OnVideoSample(GstElement* object, AAMPGstPlayer * _this);
#endif
private:
	void PauseAndFlush(bool playAfterFlush);
	void TearDownStream(MediaType mediaType);
	bool CreatePipeline();
	void DestroyPipeline();
	static bool initialized;
	void Flush(void);
	void DisconnectCallbacks();
};

#endif // AAMPGSTPLAYER_H
