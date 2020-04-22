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
 * @file main_aamp.cpp
 * @brief Advanced Adaptive Media Player (AAMP)
 */
#include "iso639map.h"
#include <sys/time.h>
#ifndef DISABLE_DASH
#include "fragmentcollector_mpd.h"
#include "admanager_mpd.h"
#endif
#include "fragmentcollector_hls.h"
#include "fragmentcollector_progressive.h"
#include "_base64.h"
#include "base16.h"
#include "aampgstplayer.h"
#include "AampDRMSessionManager.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "drm.h"
#include <unistd.h>
#include "priv_aamp.h"
#include <signal.h>
#include <semaphore.h>
#include <glib.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <algorithm>
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include <math.h>
#include "AampCacheHandler.h"
#ifdef USE_OPENCDM // AampOutputProtection is compiled when this  flag is enabled 
#include "aampoutputprotection.h"
#endif
#include <uuid/uuid.h>
static const char* strAAMPPipeName = "/tmp/ipc_aamp";
#ifdef WIN32
#include "conio.h"
#else
#include <termios.h>
#include <errno.h>
#include <regex>

#ifdef IARM_MGR
#include "host.hpp"
#include "manager.hpp"
#include "libIBus.h"
#include "libIBusDaemon.h"

#include <hostIf_tr69ReqHandler.h>
#include <sstream>

/**
 * @brief
 * @param paramName
 * @param iConfigLen
 * @retval
 */
char * GetTR181AAMPConfig(const char * paramName, size_t & iConfigLen);
#endif

static const char* STRBGPLAYER = "BACKGROUND";
static const char* STRFGPLAYER = "FOREGROUND";

static int PLAYERID_CNTR = 0;

//Stringification of Macro :  use two levels of macros
#define MACRO_TO_STRING(s) X_STR(s)
#define X_STR(s) #s

/**
 * @brief get a character for console
 * @retval user input character
 */
int getch(void)
{ // for linux
	struct termios oldattr, newattr;
	int ch;
	tcgetattr(STDIN_FILENO, &oldattr);
	newattr = oldattr;
	newattr.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newattr);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldattr);
	return ch;
}
#endif

#define ARRAY_SIZE(A) ((int)(sizeof(A)/sizeof(A[0])))

/**
 * @struct AsyncEventDescriptor
 * @brief Used in asynchronous event notification logic
 */
struct AsyncEventDescriptor
{
	AsyncEventDescriptor() : event(), aamp(NULL)
	{
	}

	AAMPEvent event;
	PrivateInstanceAAMP* aamp;

	AsyncEventDescriptor(const AsyncEventDescriptor &other) = delete;

	AsyncEventDescriptor& operator=(const AsyncEventDescriptor& other) = delete;
	virtual ~AsyncEventDescriptor() {}
};

/**
 * @struct AsyncVideoEndEventDescriptor
 * @brief Used in asynchronous aamp data event notification logic
 * new struct created to handle memory deletion
 */
struct AsyncMetricsEventDescriptor : public AsyncEventDescriptor
{
	AsyncMetricsEventDescriptor(MetricsDataType type, char * data, std::string & uuid)
	{
		event.type = AAMP_EVENT_REPORT_METRICS_DATA;
		event.data.metricsData.data = data;
		event.data.metricsData.type = type;

		if(!uuid.empty() && (uuid.size() < METRIC_UUID_BUFF_LEN))
		{
			strcpy(event.data.metricsData.metricUUID,uuid.c_str());
		}
		else
		{
			event.data.metricsData.metricUUID[0] = 0;
		}
	}

	virtual ~AsyncMetricsEventDescriptor()
	{
		if(event.data.metricsData.data)
		{
			free(event.data.metricsData.data);
		}
	}
};

/**
 * @struct AsyncMicroEventDescriptor
 * @brief Used in asynchronous aamp data event for micro event
 * new struct created to handle memory deletion
 */
struct AsyncMicroEventDescriptor : public AsyncEventDescriptor
{
	AsyncMicroEventDescriptor(const char * data)
	{
		event.type = AAMP_EVENT_TUNE_PROFILING;
		char *eventData = new char[strlen(data) + 1];
		strcpy(eventData, data);
		event.data.tuneProfile.microData = eventData;
	}

	virtual ~AsyncMicroEventDescriptor()
	{
		if(event.data.tuneProfile.microData)
		{
			delete[] event.data.tuneProfile.microData;
		}
	}
};


static TuneFailureMap tuneFailureMap[] =
{
	{AAMP_TUNE_INIT_FAILED, 10, "AAMP: init failed"}, //"Fragmentcollector initialization failed"
	{AAMP_TUNE_INIT_FAILED_MANIFEST_DNLD_ERROR, 10, "AAMP: init failed (unable to download manifest)"},
	{AAMP_TUNE_INIT_FAILED_MANIFEST_CONTENT_ERROR, 10, "AAMP: init failed (manifest missing tracks)"},
	{AAMP_TUNE_INIT_FAILED_MANIFEST_PARSE_ERROR, 10, "AAMP: init failed (corrupt/invalid manifest)"},
	{AAMP_TUNE_INIT_FAILED_PLAYLIST_VIDEO_DNLD_ERROR, 10, "AAMP: init failed (unable to download video playlist)"},
	{AAMP_TUNE_INIT_FAILED_PLAYLIST_AUDIO_DNLD_ERROR, 10, "AAMP: init failed (unable to download audio playlist)"},
	{AAMP_TUNE_INIT_FAILED_TRACK_SYNC_ERROR, 10, "AAMP: init failed (unsynchronized tracks)"},
	{AAMP_TUNE_MANIFEST_REQ_FAILED, 10, "AAMP: Manifest Download failed"}, //"Playlist refresh failed"
	{AAMP_TUNE_AUTHORISATION_FAILURE, 40, "AAMP: Authorization failure"},
	{AAMP_TUNE_FRAGMENT_DOWNLOAD_FAILURE, 10, "AAMP: fragment download failures"},
	{AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, 10, "AAMP: init fragment download failed"},
	{AAMP_TUNE_UNTRACKED_DRM_ERROR, 50, "AAMP: DRM error untracked error"},
	{AAMP_TUNE_DRM_INIT_FAILED, 50, "AAMP: DRM Initialization Failed"},
	{AAMP_TUNE_DRM_DATA_BIND_FAILED, 50, "AAMP: InitData-DRM Binding Failed"},
	{AAMP_TUNE_DRM_SESSIONID_EMPTY, 50, "AAMP: DRM Session ID Empty"},
	{AAMP_TUNE_DRM_CHALLENGE_FAILED, 50, "AAMP: DRM License Challenge Generation Failed"},
	{AAMP_TUNE_LICENCE_TIMEOUT, 50, "AAMP: DRM License Request Timed out"},
	{AAMP_TUNE_LICENCE_REQUEST_FAILED, 50, "AAMP: DRM License Request Failed"},
	{AAMP_TUNE_INVALID_DRM_KEY, 50, "AAMP: Invalid Key Error, from DRM"},
	{AAMP_TUNE_UNSUPPORTED_STREAM_TYPE, 50, "AAMP: Unsupported Stream Type"}, //"Unable to determine stream type for DRM Init"
	{AAMP_TUNE_UNSUPPORTED_AUDIO_TYPE, 50, "AAMP: No supported Audio Types in Manifest"},
	{AAMP_TUNE_FAILED_TO_GET_KEYID, 50, "AAMP: Failed to parse key id from PSSH"},
	{AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN, 50, "AAMP: Failed to get access token from Auth Service"},
	{AAMP_TUNE_CORRUPT_DRM_DATA, 51, "AAMP: DRM failure due to Corrupt DRM files"},
	{AAMP_TUNE_CORRUPT_DRM_METADATA, 50, "AAMP: DRM failure due to Bad DRMMetadata in stream"},
	{AAMP_TUNE_DRM_DECRYPT_FAILED, 50, "AAMP: DRM Decryption Failed for Fragments"},
	{AAMP_TUNE_GST_PIPELINE_ERROR, 80, "AAMP: Error from gstreamer pipeline"},
	{AAMP_TUNE_PLAYBACK_STALLED, 7600, "AAMP: Playback was stalled due to lack of new fragments"},
	{AAMP_TUNE_CONTENT_NOT_FOUND, 20, "AAMP: Resource was not found at the URL(HTTP 404)"},
	{AAMP_TUNE_DRM_KEY_UPDATE_FAILED, 50, "AAMP: Failed to process DRM key"},
	{AAMP_TUNE_DEVICE_NOT_PROVISIONED, 52, "AAMP: Device not provisioned"},
	{AAMP_TUNE_HDCP_COMPLIANCE_ERROR, 53, "AAMP: HDCP Compliance Check Failure"},
    	{AAMP_TUNE_INVALID_MANIFEST_FAILURE, 10, "AAMP: Invalid Manifest, parse failed"},
	{AAMP_TUNE_FAILED_PTS_ERROR, 80, "AAMP: Playback failed due to PTS error"},
	{AAMP_TUNE_MP4_INIT_FRAGMENT_MISSING, 10, "AAMP: init fragments missing in playlist"},
	{AAMP_TUNE_FAILURE_UNKNOWN, 100, "AAMP: Unknown Failure"}
};


static constexpr const char *ADEVENT_STR[] =
{
	(const char *)"AAMP_EVENT_AD_RESERVATION_START",
	(const char *)"AAMP_EVENT_AD_RESERVATION_END",
	(const char *)"AAMP_EVENT_AD_PLACEMENT_START",
	(const char *)"AAMP_EVENT_AD_PLACEMENT_END",
	(const char *)"AAMP_EVENT_AD_PLACEMENT_ERROR",
	(const char *)"AAMP_EVENT_AD_PLACEMENT_PROGRESS"
};

#define ADEVENT2STRING(id) ADEVENT_STR[id - AAMP_EVENT_AD_RESERVATION_START]

/**
 * @struct ChannelInfo 
 * @brief Holds information of a channel
 */
struct ChannelInfo
{
	ChannelInfo() : name(), uri()
	{
	}
	std::string name;
	std::string uri;
};

static std::list<ChannelInfo> mChannelOverrideMap;

GlobalConfigAAMP *gpGlobalConfig;

#define LOCAL_HOST_IP       "127.0.0.1"
#define STR_PROXY_BUFF_SIZE  64
#define AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS (20*1000LL)

#define VALIDATE_INT(param_name, param_value, default_value)        \
    if ((param_value <= 0) || (param_value > INT_MAX))  { \
        logprintf("%s(): Parameter '%s' not within INTEGER limit. Using default value instead.", __FUNCTION__, param_name); \
        param_value = default_value; \
    }

#define VALIDATE_LONG(param_name, param_value, default_value)        \
    if ((param_value <= 0) || (param_value > LONG_MAX))  { \
        logprintf("%s(): Parameter '%s' not within LONG INTEGER limit. Using default value instead.", __FUNCTION__, param_name); \
        param_value = default_value; \
    }

#define VALIDATE_DOUBLE(param_name, param_value, default_value)        \
    if ((param_value <= 0) || (param_value > DBL_MAX))  { \
        logprintf("%s(): Parameter '%s' not within DOUBLE limit. Using default value instead.", __FUNCTION__, param_name); \
        param_value = default_value; \
    }

#define ERROR_STATE_CHECK_VOID() \
	PrivAAMPState state; \
	aamp->GetState(state); \
	if( state == eSTATE_ERROR){ \
		logprintf("%s() operation is not allowed when player in eSTATE_ERROR state !", __FUNCTION__ );\
		return; \
	}

#define ERROR_STATE_CHECK_VAL(val) \
	PrivAAMPState state; \
	aamp->GetState(state); \
	if( state == eSTATE_ERROR){ \
		logprintf("%s() operation is not allowed when player in eSTATE_ERROR state !", __FUNCTION__ );\
		return val; \
	}

#define ERROR_OR_IDLE_STATE_CHECK_VOID() \
	PrivAAMPState state; \
	aamp->GetState(state); \
	if( state == eSTATE_ERROR || state == eSTATE_IDLE){ \
		logprintf("%s() operation is not allowed when player in %s state !", __FUNCTION__ ,\
		(state == eSTATE_ERROR) ? "eSTATE_ERROR" : "eSTATE_IDLE" );\
		return; \
	}

#define NOT_IDLE_AND_NOT_RELEASED_STATE_CHECK_VOID() \
	PrivAAMPState state; \
	aamp->GetState(state); \
	if( state != eSTATE_IDLE && state != eSTATE_RELEASED){ \
		logprintf("%s() operation is not allowed when player not in eSTATE_IDLE or eSTATE_RELEASED state !", __FUNCTION__ );\
		return; \
	}

#define ERROR_OR_IDLE_STATE_CHECK_VAL(val) \
	PrivAAMPState state; \
	aamp->GetState(state); \
	if( state == eSTATE_ERROR || state == eSTATE_IDLE){ \
		logprintf("%s() operation is not allowed in %s state !", __FUNCTION__ ,\
		(state == eSTATE_ERROR) ? "eSTATE_ERROR" : "eSTATE_IDLE" );\
		return val; \
	}

#define FOG_REASON_STRING			"Fog-Reason:"
#define CURLHEADER_X_REASON			"X-Reason:"
#define BITRATE_HEADER_STRING			"X-Bitrate:"
#define CONTENTLENGTH_STRING 			"Content-Length:"

#define STRLEN_LITERAL(STRING) (sizeof(STRING)-1)
#define STARTS_WITH_IGNORE_CASE(STRING, PREFIX) (0 == strncasecmp(STRING, PREFIX, STRLEN_LITERAL(PREFIX)))

/**
 * New state for treating a VOD asset as a "virtual linear" stream
 */
// Note that below state/impl currently assumes single profile, and so until fixed should be tested with "abr" in aamp.cfg to disable ABR
static long long simulation_start; // time at which main manifest was downloaded.
// Simulation_start is used to calculate elapsed time, used to advance virtual live window
static char *full_playlist_video_ptr = NULL; // Cache of initial full vod video playlist
static size_t full_playlist_video_len = 0; // Size (bytes) of initial full vod video playlist
static char *full_playlist_audio_ptr = NULL; // Cache of initial full vod audio playlist
static size_t full_playlist_audio_len = 0; // Size (bytes) of initial full vod audio playlist

struct gActivePrivAAMP_t
{
	PrivateInstanceAAMP* pAAMP;
	bool reTune;
	int numPtsErrors;
};

static std::list<gActivePrivAAMP_t> gActivePrivAAMPs = std::list<gActivePrivAAMP_t>();

static pthread_mutex_t gMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t gCond = PTHREAD_COND_INITIALIZER;


/**
 * @brief Get ID of DRM system
 * @param drmSystem drm system
 * @retval ID of the DRM system, empty string if not supported
 */
const char * GetDrmSystemID(DRMSystems drmSystem)
{
	if(drmSystem == eDRM_WideVine)
		return "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
	else if(drmSystem == eDRM_PlayReady)
		return "9a04f079-9840-4286-ab92-e65be0885f95";
	else if(drmSystem == eDRM_CONSEC_agnostic)
		return "afbcb50e-bf74-3d13-be8f-13930c783962";
	else
		return "";
}


/**
 * @brief Get name of DRM system
 * @param drmSystem drm system
 * @retval Name of the DRM system, empty string if not supported
 */
const char * GetDrmSystemName(DRMSystems drmSystem)
{
	switch(drmSystem)
	{
	case eDRM_WideVine:
		return "Widevine";
	case eDRM_PlayReady:
		return "PlayReady";
	case eDRM_CONSEC_agnostic:
		return "Consec Agnostic";
	case eDRM_Adobe_Access:
		return "Adobe Access";
	case eDRM_Vanilla_AES:
		return "Vanilla AES";
	case eDRM_NONE:
	case eDRM_ClearKey:
	case eDRM_MAX_DRMSystems:
		return "";
	}
}

/**
 * @brief Get current time stamp
 * @retval current clock time as milliseconds
 */
long long aamp_GetCurrentTimeMS(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (long long)(t.tv_sec*1e3 + t.tv_usec*1e-3);
}


/**
 * @brief Report progress event to listeners
 */
void PrivateInstanceAAMP::ReportProgress(void)
{
	//if (mPlayerState.durationMilliseconds > 0)
	if (mDownloadsEnabled)
	{
		ReportAdProgress();

		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_PROGRESS;

		eventData.data.progress.positionMiliseconds = GetPositionMilliseconds();
		eventData.data.progress.durationMiliseconds = durationSeconds*1000.0;

		//If tsb is not available for linear send -1  for start and end
		// so that xre detect this as tsbless playabck
		if( mContentType == ContentType_LINEAR && !mTSBEnabled)
		{
			eventData.data.progress.startMiliseconds = -1;
			eventData.data.progress.endMiliseconds = -1;
		}
		else
		{
		    eventData.data.progress.startMiliseconds = culledSeconds*1000.0;
		    eventData.data.progress.endMiliseconds = eventData.data.progress.startMiliseconds + eventData.data.progress.durationMiliseconds;
		}

		eventData.data.progress.playbackSpeed = pipeline_paused ? 0 : rate;

		if (eventData.data.progress.positionMiliseconds > eventData.data.progress.endMiliseconds)
		{ // clamp end
			//logprintf("aamp clamp end");
			eventData.data.progress.positionMiliseconds = eventData.data.progress.endMiliseconds;
		}
		else if (eventData.data.progress.positionMiliseconds < eventData.data.progress.startMiliseconds)
		{ // clamp start
			//logprintf("aamp clamp start");
			eventData.data.progress.positionMiliseconds = eventData.data.progress.startMiliseconds;
		}

		if(gpGlobalConfig->bReportVideoPTS)
		{
				/*For HLS, tsprocessor.cpp removes the base PTS value and sends to gstreamer.
				**In order to report PTS of video currently being played out, we add the base PTS
				**to video PTS received from gstreamer
				*/
				/*For DASH,mVideoBasePTS value will be zero */
				eventData.data.progress.videoPTS = mStreamSink->GetVideoPTS() + mVideoBasePTS;
		}
		else
		{
			eventData.data.progress.videoPTS = -1; // if -1 , this parameter wont be added to JSPP event 
		}
        
		if (gpGlobalConfig->logging.progress)
		{
			static int tick;
			if ((tick++ % 4) == 0)
			{
				logprintf("aamp pos: [%ld..%ld..%ld..%lld]",
					(long)(eventData.data.progress.startMiliseconds / 1000),
					(long)(eventData.data.progress.positionMiliseconds / 1000),
					(long)(eventData.data.progress.endMiliseconds / 1000),
					(long long) eventData.data.progress.videoPTS);
			}
		}
		mReportProgressPosn = eventData.data.progress.positionMiliseconds;
		SendEventSync(eventData);
		mReportProgressTime = aamp_GetCurrentTimeMS();
	}
}

 /*
 * @brief Report Ad progress event to listeners
 *
 * Sending Ad progress percentage to JSPP
 */
void PrivateInstanceAAMP::ReportAdProgress(void)
{
	if (mDownloadsEnabled && !mAdProgressId.empty())
	{
		long long curTime = NOW_STEADY_TS_MS;
		if (!pipeline_paused)
		{
			//Update the percentage only if the pipeline is in playing.
			mAdCurOffset += (uint32_t)(curTime - mAdPrevProgressTime);
			if(mAdCurOffset > mAdDuration) mAdCurOffset = mAdDuration;
		}
		mAdPrevProgressTime = curTime;

		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_AD_PLACEMENT_PROGRESS;
		strncpy(eventData.data.adPlacement.adId, mAdProgressId.c_str(), AD_ID_LENGTH);
		eventData.data.adPlacement.position = (uint32_t)(mAdCurOffset*100)/mAdDuration;
		SendEventSync(eventData);
	}
}

/**
 * @brief Update duration of stream.
 *
 * Called from fragmentcollector_hls::IndexPlaylist to update TSB duration
 *
 * @param[in] seconds Duration in seconds
 */
void PrivateInstanceAAMP::UpdateDuration(double seconds)
{
	AAMPLOG_INFO("aamp_UpdateDuration(%f)", seconds);
	durationSeconds = seconds;
}


/**
 * @brief Idle task to resume aamp
 * @param ptr pointer to PrivateInstanceAAMP object
 * @retval G_SOURCE_REMOVE
 */
static gboolean PrivateInstanceAAMP_Resume(gpointer ptr)
{
	bool retValue = true;
	PrivateInstanceAAMP* aamp = (PrivateInstanceAAMP* )ptr;
	aamp->NotifyFirstBufferProcessed();
	if (aamp->pipeline_paused)
	{
		if (aamp->rate == AAMP_NORMAL_PLAY_RATE)
		{
			retValue = aamp->mStreamSink->Pause(false);
			aamp->pipeline_paused = false;
		}
		else
		{
			aamp->rate = AAMP_NORMAL_PLAY_RATE;
			aamp->pipeline_paused = false;
			aamp->TuneHelper(eTUNETYPE_SEEK);
		}
		aamp->ResumeDownloads();

		if(retValue)
		{
			aamp->NotifySpeedChanged(aamp->rate);
		}
	}
	return G_SOURCE_REMOVE;
}


/**
 * @brief Update culling state in case of TSB
 * @param culledSecs culled duration in seconds
 */
void PrivateInstanceAAMP::UpdateCullingState(double culledSecs)
{
	if (culledSecs == 0)
	{
		return;
	}

	if((!this->culledSeconds) && culledSecs)
	{
		logprintf("PrivateInstanceAAMP::%s - culling started, first value %f", __FUNCTION__, culledSecs);
	}

	this->culledSeconds += culledSecs;
	long long limitMs = (long long) std::round(this->culledSeconds * 1000.0);

	for (auto iter = timedMetadata.begin(); iter != timedMetadata.end(); )
	{
		// If the timed metadata has expired due to playlist refresh, remove it from local cache
		// For X-CONTENT-IDENTIFIER, -X-IDENTITY-ADS, X-MESSAGE_REF in DASH which has _timeMS as 0
		if (iter->_timeMS != 0 && iter->_timeMS < limitMs)
		{
			//logprintf("ERASE(limit:%lld) aamp_ReportTimedMetadata(%lld, '%s', '%s', nb)", limitMs,iter->_timeMS, iter->_name.c_str(), iter->_content.c_str());
			iter = timedMetadata.erase(iter);
		}
		else
		{
			iter++;
		}
	}

	// Check if we are paused and culled past paused playback position
	// AAMP internally caches fragments in sw and gst buffer, so we should be good here
	if (pipeline_paused && mpStreamAbstractionAAMP)
	{
		double position = GetPositionMilliseconds() / 1000.0; // in seconds
		double minPlaylistPositionToResume = (position < maxRefreshPlaylistIntervalSecs) ? position : (position - maxRefreshPlaylistIntervalSecs);
		if (this->culledSeconds >= position)
		{
			logprintf("%s(): Resume playback since playlist start position(%f) has moved past paused position(%f) ", __FUNCTION__, this->culledSeconds, position);
			g_idle_add(PrivateInstanceAAMP_Resume, (gpointer)this);
		}
		else if (this->culledSeconds >= minPlaylistPositionToResume)
		{
			// Here there is a chance that paused position will be culled after next refresh playlist
			// AAMP internally caches fragments in sw bufffer after paused position, so we are at less risk
			// Make sure that culledSecs is within the limits of maxRefreshPlaylistIntervalSecs
			// This check helps us to avoid initial culling done by FOG after channel tune

			if (culledSecs <= maxRefreshPlaylistIntervalSecs)
			{
				logprintf("%s(): Resume playback since start position(%f) moved very close to minimum resume position(%f) ", __FUNCTION__, this->culledSeconds, minPlaylistPositionToResume);
				g_idle_add(PrivateInstanceAAMP_Resume, (gpointer)this);
			}
		}

	}
}


/**
 * @brief Update playlist refresh interval
 * @param maxIntervalSecs refresh interval in seconds
 */
void PrivateInstanceAAMP::UpdateRefreshPlaylistInterval(float maxIntervalSecs)
{
	AAMPLOG_INFO("%s(): maxRefreshPlaylistIntervalSecs (%f)", __FUNCTION__, maxIntervalSecs);
	maxRefreshPlaylistIntervalSecs = maxIntervalSecs;
}


/**
 * @brief Add listener to aamp events
 * @param eventType type of event to subscribe
 * @param eventListener listener
 */
void PrivateInstanceAAMP::AddEventListener(AAMPEventType eventType, AAMPEventListener* eventListener)
{
	//logprintf("[AAMP_JS] %s(%d, %p)", __FUNCTION__, eventType, eventListener);
	if ((eventListener != NULL) && (eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
	{
		ListenerData* pListener = new ListenerData;
		if (pListener)
		{
			//logprintf("[AAMP_JS] %s(%d, %p) new %p", __FUNCTION__, eventType, eventListener, pListener);
			pthread_mutex_lock(&mLock);
			pListener->eventListener = eventListener;
			pListener->pNext = mEventListeners[eventType];
			mEventListeners[eventType] = pListener;
			pthread_mutex_unlock(&mLock);
		}
	}
}


/**
 * @brief Remove listener to aamp events
 * @param eventType type of event to unsubscribe
 * @param eventListener listener
 */
void PrivateInstanceAAMP::RemoveEventListener(AAMPEventType eventType, AAMPEventListener* eventListener)
{
	logprintf("[AAMP_JS] %s(%d, %p)", __FUNCTION__, eventType, eventListener);
	if ((eventListener != NULL) && (eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
	{
		pthread_mutex_lock(&mLock);
		ListenerData** ppLast = &mEventListeners[eventType];
		while (*ppLast != NULL)
		{
			ListenerData* pListener = *ppLast;
			if (pListener->eventListener == eventListener)
			{
				*ppLast = pListener->pNext;
				pthread_mutex_unlock(&mLock);
				logprintf("[AAMP_JS] %s(%d, %p) delete %p", __FUNCTION__, eventType, eventListener, pListener);
				delete pListener;
				return;
			}
			ppLast = &(pListener->pNext);
		}
		pthread_mutex_unlock(&mLock);
	}
}


/**
 * @brief Handles DRM errors and sends events to application if required.
 * @param tuneFailure Reason of error
 * @param error_code Drm error code (http, curl or secclient)
 */
void PrivateInstanceAAMP::SendDrmErrorEvent(AAMPTuneFailure tuneFailure,long error_code, bool isRetryEnabled)
{

	if(AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN == tuneFailure || AAMP_TUNE_LICENCE_REQUEST_FAILED == tuneFailure)
	{
		char description[128] = {};

		if(AAMP_TUNE_LICENCE_REQUEST_FAILED == tuneFailure && error_code < 100)
		{
#ifdef USE_SECCLIENT
			snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "%s : Secclient Error Code %ld", tuneFailureMap[tuneFailure].description, error_code);
#else
			snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "%s : Curl Error Code %ld", tuneFailureMap[tuneFailure].description, error_code);
#endif
		}
		else if (AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN == tuneFailure && eAUTHTOKEN_TOKEN_PARSE_ERROR == (AuthTokenErrors)error_code)
		{
			snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "%s : Access Token Parse Error", tuneFailureMap[tuneFailure].description);
		}
		else if(AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN == tuneFailure && eAUTHTOKEN_INVALID_STATUS_CODE == (AuthTokenErrors)error_code)
		{
			snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "%s : Invalid status code", tuneFailureMap[tuneFailure].description);
		}
		else
		{
			snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "%s : Http Error Code %ld", tuneFailureMap[tuneFailure].description, error_code);
		}
		SendErrorEvent(tuneFailure, description, isRetryEnabled);
	}
	else if(tuneFailure >= 0 && tuneFailure < AAMP_TUNE_FAILURE_UNKNOWN)
	{
		SendErrorEvent(tuneFailure, NULL, isRetryEnabled);
	}
	else
	{
		logprintf("%s:%d : Received unknown error event %d", __FUNCTION__, __LINE__, tuneFailure);
		SendErrorEvent(AAMP_TUNE_FAILURE_UNKNOWN);
	}
}


/**
 * @brief Handles download errors and sends events to application if required.
 * @param tuneFailure Reason of error
 * @param error_code HTTP error code/ CURLcode
 */
void PrivateInstanceAAMP::SendDownloadErrorEvent(AAMPTuneFailure tuneFailure,long error_code)
{
	AAMPTuneFailure actualFailure = tuneFailure;

	if(tuneFailure >= 0 && tuneFailure < AAMP_TUNE_FAILURE_UNKNOWN)
	{
		char description[128] = {};
		if (((error_code >= PARTIAL_FILE_CONNECTIVITY_AAMP) && (error_code <= PARTIAL_FILE_START_STALL_TIMEOUT_AAMP)) || error_code == CURLE_OPERATION_TIMEDOUT)
		{
			switch(error_code)
			{
				case PARTIAL_FILE_DOWNLOAD_TIME_EXPIRED_AAMP:
						error_code = CURLE_PARTIAL_FILE;
				case CURLE_OPERATION_TIMEDOUT:
						sprintf(description, "%s : Curl Error Code %ld, Download time expired", tuneFailureMap[tuneFailure].description, error_code);
						break;
				case PARTIAL_FILE_START_STALL_TIMEOUT_AAMP:
						sprintf(description, "%s : Curl Error Code %d, Start/Stall timeout", tuneFailureMap[tuneFailure].description, CURLE_PARTIAL_FILE);
						break;
				case OPERATION_TIMEOUT_CONNECTIVITY_AAMP:
						sprintf(description, "%s : Curl Error Code %d, Connectivity failure", tuneFailureMap[tuneFailure].description, CURLE_OPERATION_TIMEDOUT);
						break;
				case PARTIAL_FILE_CONNECTIVITY_AAMP:
						sprintf(description, "%s : Curl Error Code %d, Connectivity failure", tuneFailureMap[tuneFailure].description, CURLE_PARTIAL_FILE);
						break;
			}
		}
		else if(error_code < 100)
		{
			sprintf(description, "%s : Curl Error Code %ld", tuneFailureMap[tuneFailure].description, error_code);
		}
		else
		{
			sprintf(description, "%s : Http Error Code %ld", tuneFailureMap[tuneFailure].description, error_code);
			if (error_code == 404)
			{
				actualFailure = AAMP_TUNE_CONTENT_NOT_FOUND;
			}
		}
		if( IsTSBSupported() )
		{
			strcat(description, "(FOG)");
		}

		SendErrorEvent(actualFailure, description);
	}
	else
	{
		logprintf("%s:%d : Received unknown error event %d", __FUNCTION__, __LINE__, tuneFailure);
		SendErrorEvent(AAMP_TUNE_FAILURE_UNKNOWN);
	}
}

/**
 * @brief Sends Anomaly Error/warning messages
 *
 * @param[in] type - severity of message
 * @param[in] format - format string
 * args [in]  - multiple arguments based on format
 * @return void
 */
void PrivateInstanceAAMP::SendAnomalyEvent(AAMPAnomalyMessageType type, const char* format, ...)
{
    if(NULL != format)
    {
        va_list args;
        va_start(args, format);

        AAMPEvent e;
        e.type = AAMP_EVENT_REPORT_ANOMALY;
        char * msgData = e.data.anomalyReport.msg;

        msgData[(MAX_ANOMALY_BUFF_SIZE-1)] = 0;
        vsnprintf(msgData, (MAX_ANOMALY_BUFF_SIZE-1), format, args);


        e.data.anomalyReport.severity = (int)type;
        AAMPLOG_INFO("Anomaly evt:%d msg:%s",e.data.anomalyReport.severity,msgData);
        SendEventAsync(e);
    }
}

/**
 * @brief Sends UnderFlow Event messages
 *
 * @param[in] bufferingStopped- Flag to indicate buffering stopped (Underflow started true else false)
 * @return void
 */
void PrivateInstanceAAMP::SendBufferChangeEvent(bool bufferingStopped)
{
	AAMPEvent e;

	e.type = AAMP_EVENT_BUFFERING_CHANGED;

	SetBufUnderFlowStatus(bufferingStopped);

	e.data.bufferingChanged.buffering = !(bufferingStopped);   /* False if Buffering End, True if Buffering Start*/
	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Sending Buffer Change event status (Buffering): %s", __FUNCTION__, __LINE__, ((e.data.bufferingChanged.buffering) ? "Start": "End"));

	SendEventAsync(e);
}

/**
 * @brief To change the the gstreamer pipeline to pause/play
 *
 * @param[in] pause- true for pause and false for play
 * @return true on success
 */
bool PrivateInstanceAAMP::PausePipeline(bool pause)
{
	if (true != mStreamSink->Pause(pause))
	{
		return false;
	}
	pipeline_paused = pause;
	return true;
}

/**
 * @brief Handles errors and sends events to application if required.
 * For download failures, use SendDownloadErrorEvent instead.
 * @param tuneFailure Reason of error
 * @param description Optional description of error
 */
void PrivateInstanceAAMP::SendErrorEvent(AAMPTuneFailure tuneFailure, const char * description, bool isRetryEnabled)
{
	bool sendErrorEvent = false;
	pthread_mutex_lock(&mLock);
	if(mState != eSTATE_ERROR)
	{
		sendErrorEvent = true;
		mState = eSTATE_ERROR;
	}
	pthread_mutex_unlock(&mLock);
	if (sendErrorEvent)
	{
		AAMPEvent e;
		e.type = AAMP_EVENT_TUNE_FAILED;
		e.data.mediaError.shouldRetry = isRetryEnabled;
		const char *errorDescription = NULL;
		DisableDownloads();
		if(tuneFailure >= 0 && tuneFailure < AAMP_TUNE_FAILURE_UNKNOWN)
		{
			if (tuneFailure == AAMP_TUNE_PLAYBACK_STALLED)
			{ // allow config override for stall detection error code
				e.data.mediaError.code = gpGlobalConfig->stallErrorCode;
			}
			else
			{
				e.data.mediaError.code = tuneFailureMap[tuneFailure].code;
			}
			if(description)
			{
				errorDescription = description;
			}
			else
			{
				errorDescription = tuneFailureMap[tuneFailure].description;
			}
		}
		else
		{
			e.data.mediaError.code = tuneFailureMap[AAMP_TUNE_FAILURE_UNKNOWN].code;
			errorDescription = tuneFailureMap[AAMP_TUNE_FAILURE_UNKNOWN].description;
		}

		strncpy(e.data.mediaError.description, errorDescription, MAX_ERROR_DESCRIPTION_LENGTH);
		e.data.mediaError.description[MAX_ERROR_DESCRIPTION_LENGTH - 1] = '\0';
		SendAnomalyEvent(ANOMALY_ERROR,"Error[%d]:%s",tuneFailure,e.data.mediaError.description);
		if (!mAppName.empty())
		{
			logprintf("APP: %s PLAYER[%d] Sending error %s ", mAppName.c_str(), mPlayerId, e.data.mediaError.description);
		}
		else
		{
			logprintf("PLAYER[%d] Sending error %s ", mPlayerId, e.data.mediaError.description);
		}
		SendEventAsync(e);
	}
	else
	{
		logprintf("PrivateInstanceAAMP::%s:%d Ignore error %d[%s]", __FUNCTION__, __LINE__, (int)tuneFailure, description);
	}
}


/**
 * @brief Send event asynchronously to listeners
 * @param e event
 */
void PrivateInstanceAAMP::SendEventAsync(const AAMPEvent &e)
{
	if (mEventListener || mEventListeners[0] || mEventListeners[e.type])
	{
		AsyncEventDescriptor* aed = new AsyncEventDescriptor();
		aed->event = e;
		ScheduleEvent(aed);
		if(e.type != AAMP_EVENT_PROGRESS)
			AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d event type  %d", __FUNCTION__, __LINE__,e.type);
	}
	else
	{
			AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Failed to send event type  %d", __FUNCTION__, __LINE__,e.type);
	}
}


/**
 * @brief Send event synchronously to listeners
 * @param e event
 */
void PrivateInstanceAAMP::SendEventSync(const AAMPEvent &e)
{
	if(e.type != AAMP_EVENT_PROGRESS)
	{
		if (e.type != AAMP_EVENT_STATE_CHANGED)
		{
			AAMPLOG_INFO("[AAMP_JS] %s(type=%d)", __FUNCTION__, e.type);
		}
		else
		{
			AAMPLOG_WARN("[AAMP_JS] %s(type=%d)(state=%d)", __FUNCTION__, e.type, e.data.stateChanged.state);
		}
	}

	//TODO protect mEventListener
	if (mEventListener)
	{
		mEventListener->Event(e);
	}

	AAMPEventType eventType = e.type;
	if ((eventType < 0) && (eventType >= AAMP_MAX_NUM_EVENTS))
		return;

	// Build list of registered event listeners.
	ListenerData* pList = NULL;
	pthread_mutex_lock(&mLock);
	ListenerData* pListener = mEventListeners[eventType];
	while (pListener != NULL)
	{
		ListenerData* pNew = new ListenerData;
		pNew->eventListener = pListener->eventListener;
		pNew->pNext = pList;
		pList = pNew;
		pListener = pListener->pNext;
	}
	pListener = mEventListeners[0];  // listeners registered for "all" event types
	while (pListener != NULL)
	{
		ListenerData* pNew = new ListenerData;
		pNew->eventListener = pListener->eventListener;
		pNew->pNext = pList;
		pList = pNew;
		pListener = pListener->pNext;
	}
	pthread_mutex_unlock(&mLock);

	// After releasing the lock, dispatch each of the registered listeners.
	// This allows event handlers to add/remove listeners for future events.
	while (pList != NULL)
	{
		ListenerData* pCurrent = pList;
		if (pCurrent->eventListener != NULL)
		{
			//logprintf("[AAMP_JS] %s(type=%d) listener=%p", __FUNCTION__, eventType, pCurrent->eventListener);
			pCurrent->eventListener->Event(e);
		}
		pList = pCurrent->pNext;
		delete pCurrent;
	}
}

/**
 * @brief Notify bitrate change event to listeners
 * @param bitrate new bitrate
 * @param description description of rate change
 * @param width new width in pixels
 * @param height new height in pixels
 * @param GetBWIndex get bandwidth index - used for logging
 */
void PrivateInstanceAAMP::NotifyBitRateChangeEvent(int bitrate ,const char *description ,int width ,int height,double frameRate, bool GetBWIndex)
{
	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_BITRATE_CHANGED])
	{
		AsyncEventDescriptor* e = new AsyncEventDescriptor();
		e->event.type = AAMP_EVENT_BITRATE_CHANGED;
		e->event.data.bitrateChanged.time               =       (int)aamp_GetCurrentTimeMS();
		e->event.data.bitrateChanged.bitrate            =       bitrate;
		strncpy(e->event.data.bitrateChanged.description,description,sizeof(e->event.data.bitrateChanged.description));
		e->event.data.bitrateChanged.width              =       width;
		e->event.data.bitrateChanged.height             =       height;
		e->event.data.bitrateChanged.framerate          =       frameRate;
		

		/* START: Added As Part of DELIA-28363 and DELIA-28247 */
		if(GetBWIndex && (mpStreamAbstractionAAMP != NULL))
		{
			logprintf("NotifyBitRateChangeEvent :: bitrate:%d desc:%s width:%d height:%d fps:%f IndexFromTopProfile: %d%s",bitrate,description,width,height,frameRate,mpStreamAbstractionAAMP->GetBWIndex(bitrate), (IsTSBSupported()? ", fog": " "));
		}
		else
		{
			logprintf("NotifyBitRateChangeEvent :: bitrate:%d desc:%s width:%d height:%d fps:%f %s",bitrate,description,width,height,frameRate,(IsTSBSupported()? ", fog": " "));
		}
		/* END: Added As Part of DELIA-28363 and DELIA-28247 */

		ScheduleEvent(e);
	}
	else
	{
		/* START: Added As Part of DELIA-28363 and DELIA-28247 */
		if(GetBWIndex && (mpStreamAbstractionAAMP != NULL))
		{
			logprintf("NotifyBitRateChangeEvent ::NO LISTENERS bitrate:%d desc:%s width:%d height:%d, fps:%f IndexFromTopProfile: %d%s",bitrate,description,width,height,frameRate,mpStreamAbstractionAAMP->GetBWIndex(bitrate), (IsTSBSupported()? ", fog": " "));
		}
		else
		{
			logprintf("NotifyBitRateChangeEvent ::NO LISTENERS bitrate:%d desc:%s width:%d height:%d fps:%f %s",bitrate,description,width,height,frameRate,(IsTSBSupported()? ", fog": " "));
		}
		/* END: Added As Part of DELIA-28363 and DELIA-28247 */
	}
}


/**
 * @brief Notify rate change event to listeners
 * @param rate new speed
 */
void PrivateInstanceAAMP::NotifySpeedChanged(int rate)
{
	if (rate == 0)
	{
		SetState(eSTATE_PAUSED);
	}
	else if (rate == AAMP_NORMAL_PLAY_RATE)
	{
		SetState(eSTATE_PLAYING);
	}

	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_SPEED_CHANGED])
	{
		AsyncEventDescriptor* e = new AsyncEventDescriptor();
		e->event.type = AAMP_EVENT_SPEED_CHANGED;
		e->event.data.speedChanged.rate = rate;
		ScheduleEvent(e);
	}
}


void PrivateInstanceAAMP::SendDRMMetaData(const AAMPEvent &e)
{

        SendEventAsync(e);
        logprintf("SendDRMMetaData name = %s value = %x",e.data.dash_drmmetadata.accessStatus,e.data.dash_drmmetadata.accessStatus_value);
}




/**
 * @brief
 * @param ptr
 * @retval
 */
static gboolean PrivateInstanceAAMP_ProcessDiscontinuity(gpointer ptr)
{
	PrivateInstanceAAMP* aamp = (PrivateInstanceAAMP*) ptr;

	if (!g_source_is_destroyed(g_main_current_source()))
	{
		bool ret = aamp->ProcessPendingDiscontinuity();
		// This is to avoid calling cond signal, in case Stop() interrupts the ProcessPendingDiscontinuity
		if (ret)
		{
			aamp->SyncBegin();
			aamp->mDiscontinuityTuneOperationId = 0;
			pthread_cond_signal(&aamp->mCondDiscontinuity);
			aamp->SyncEnd();
		}
	}
	return G_SOURCE_REMOVE;
}


/**
 * @brief Check if discontinuity processing is pending
 * @retval true if discontinuity processing is pending
 */
bool PrivateInstanceAAMP::IsDiscontinuityProcessPending()
{
	return (mProcessingDiscontinuity[eMEDIATYPE_AUDIO] || mProcessingDiscontinuity[eMEDIATYPE_VIDEO]);
}


/**
 * @brief Process pending discontinuity and continue playback of stream after discontinuity
 *
 * @return true if pending discontinuity was processed successful, false if interrupted
 */
bool PrivateInstanceAAMP::ProcessPendingDiscontinuity()
{
	bool ret = true;
	SyncBegin();
	if (mDiscontinuityTuneOperationInProgress)
	{
		SyncEnd();
		logprintf("PrivateInstanceAAMP::%s:%d Discontinuity Tune Operation already in progress", __FUNCTION__, __LINE__);
		return ret; // true so that PrivateInstanceAAMP_ProcessDiscontinuity can cleanup properly
	}
	SyncEnd();

	if (!(mProcessingDiscontinuity[eMEDIATYPE_VIDEO] && mProcessingDiscontinuity[eMEDIATYPE_AUDIO]))
	{
		AAMPLOG_ERR("PrivateInstanceAAMP::%s:%d Discontinuity status of video - (%d) and audio - (%d)", __FUNCTION__, __LINE__, mProcessingDiscontinuity[eMEDIATYPE_VIDEO], mProcessingDiscontinuity[eMEDIATYPE_AUDIO]);
		return ret; // true so that PrivateInstanceAAMP_ProcessDiscontinuity can cleanup properly
	}

	SyncBegin();
	mDiscontinuityTuneOperationInProgress = true;
	SyncEnd();

	if (mProcessingDiscontinuity[eMEDIATYPE_AUDIO] && mProcessingDiscontinuity[eMEDIATYPE_VIDEO])
	{
		bool continueDiscontProcessing = true;
		logprintf("PrivateInstanceAAMP::%s:%d mProcessingDiscontinuity set", __FUNCTION__, __LINE__);
		lastUnderFlowTimeMs[eMEDIATYPE_VIDEO] = 0;
		lastUnderFlowTimeMs[eMEDIATYPE_AUDIO] = 0;
		{
			double newPosition = GetPositionMilliseconds() / 1000.0;
			double injectedPosition = seek_pos_seconds + mpStreamAbstractionAAMP->GetLastInjectedFragmentPosition();
			AAMPLOG_WARN("PrivateInstanceAAMP::%s:%d last injected position:%f position calcualted: %f", __FUNCTION__, __LINE__, injectedPosition, newPosition);

			// Reset with injected position from StreamAbstractionAAMP. This ensures that any drift in
			// GStreamer position reporting is taken care of.
			if (injectedPosition != 0 && fabs(injectedPosition - newPosition) < 5.0)
			{
				seek_pos_seconds = injectedPosition;
			}
			else
			{
				seek_pos_seconds = newPosition;
			}
			AAMPLOG_WARN("PrivateInstanceAAMP::%s:%d Updated seek_pos_seconds:%f", __FUNCTION__, __LINE__, seek_pos_seconds);
		}
		trickStartUTCMS = -1;

		SyncBegin();
		mProgressReportFromProcessDiscontinuity = true;
		SyncEnd();

		// To notify app of discontinuity processing complete
		ReportProgress();

		// There is a chance some other operation maybe invoked from JS/App because of the above ReportProgress
		// Make sure we have still mDiscontinuityTuneOperationInProgress set
		SyncBegin();
		AAMPLOG_WARN("%s:%d Progress event sent as part of ProcessPendingDiscontinuity, mDiscontinuityTuneOperationInProgress:%d", __FUNCTION__, __LINE__, mDiscontinuityTuneOperationInProgress);
		mProgressReportFromProcessDiscontinuity = false;
		continueDiscontProcessing = mDiscontinuityTuneOperationInProgress;
		SyncEnd();

		if (continueDiscontProcessing)
		{
			mpStreamAbstractionAAMP->StopInjection();
#ifndef AAMP_STOP_SINK_ON_SEEK
			if (mMediaFormat != eMEDIAFORMAT_HLS_MP4) // Avoid calling flush for fmp4 playback.
			{
				mStreamSink->Flush(mpStreamAbstractionAAMP->GetFirstPTS(), rate);
			}
#else
			mStreamSink->Stop(true);
#endif
			mpStreamAbstractionAAMP->GetStreamFormat(mVideoFormat, mAudioFormat);
			mStreamSink->Configure(mVideoFormat, mAudioFormat, false);
			mpStreamAbstractionAAMP->StartInjection();
			mStreamSink->Stream();
			mProcessingDiscontinuity[eMEDIATYPE_AUDIO] = false;
			mProcessingDiscontinuity[eMEDIATYPE_VIDEO] = false;
			mLastDiscontinuityTimeMs = 0;
		}
		else
		{
			ret = false;
			AAMPLOG_WARN("PrivateInstanceAAMP::%s:%d mDiscontinuityTuneOperationInProgress was reset during operation, since other command received from app!", __FUNCTION__, __LINE__);
		}
	}

	if (ret)
	{
		SyncBegin();
		mDiscontinuityTuneOperationInProgress = false;
		SyncEnd();
	}

	return ret;
}

/**
 * @brief Process EOS from Sink and notify listeners if required
 */
void PrivateInstanceAAMP::NotifyEOSReached()
{
	logprintf("%s: Enter . processingDiscontinuity %d",__FUNCTION__, (mProcessingDiscontinuity[eMEDIATYPE_VIDEO] || mProcessingDiscontinuity[eMEDIATYPE_AUDIO]));
	if (!IsDiscontinuityProcessPending())
	{
		if (!mpStreamAbstractionAAMP->IsEOSReached())
		{
			AAMPLOG_ERR("%s: Bogus EOS event received from GStreamer, discarding it!", __FUNCTION__);
			return;
		}
		if (!IsLive() && rate > 0)
		{
			SetState(eSTATE_COMPLETE);
			SendEventAsync(AAMP_EVENT_EOS);
			if (ContentType_EAS == mContentType) //Fix for DELIA-25590
			{
				mStreamSink->Stop(false);
			}
			SendAnomalyEvent(ANOMALY_TRACE, "Generating EOS event");
			return;
		}

		if (rate < 0)
		{
			seek_pos_seconds = culledSeconds;
			logprintf("%s:%d Updated seek_pos_seconds %f ", __FUNCTION__,__LINE__, seek_pos_seconds);
			rate = AAMP_NORMAL_PLAY_RATE;
			TuneHelper(eTUNETYPE_SEEK);
		}
		else
		{
			rate = AAMP_NORMAL_PLAY_RATE;
			TuneHelper(eTUNETYPE_SEEKTOLIVE);
		}

		NotifySpeedChanged(rate);
	}
	else
	{
		ProcessPendingDiscontinuity();
		DeliverAdEvents();
		logprintf("PrivateInstanceAAMP::%s:%d  EOS due to discontinuity handled", __FUNCTION__, __LINE__);
	}
}

/**
 * @brief Notify entering live event to listeners
 */
void PrivateInstanceAAMP::NotifyOnEnteringLive()
{
	if (discardEnteringLiveEvt)
	{
		return;
	}
	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_ENTERING_LIVE])
	{
		SendEventAsync(AAMP_EVENT_ENTERING_LIVE);
	}
}

static void AsyncEventDestroyNotify(gpointer user_data)
{
	AsyncEventDescriptor* e = (AsyncEventDescriptor*)user_data;
	if (e->event.type == AAMP_EVENT_WEBVTT_CUE_DATA)
	{
		delete e->event.data.cue.cueData;
	}
	delete e;
}


/**
 * @brief Idle task for sending asynchronous event
 * @param user_data pointer to AsyncEventDescriptor object
 * @retval G_SOURCE_REMOVE
 */
static gboolean SendAsynchronousEvent(gpointer user_data)
{
	//TODO protect mEventListener
	AsyncEventDescriptor* e = (AsyncEventDescriptor*)user_data;
	if(e->event.type != AAMP_EVENT_PROGRESS)
		AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d event type  %d", __FUNCTION__, __LINE__,e->event.type);
	//Get current idle handler's id
	gint callbackID = g_source_get_id(g_main_current_source());
	e->aamp->SetCallbackAsDispatched(callbackID);

	e->aamp->SendEventSync(e->event);
	return G_SOURCE_REMOVE;
}


/**
 * @brief Schedule asynchronous event
 * @param e event descriptor
 */
void PrivateInstanceAAMP::ScheduleEvent(AsyncEventDescriptor* e)
{
	//TODO protect mEventListener
	e->aamp = this;
	gint callbackID = g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, SendAsynchronousEvent, e, AsyncEventDestroyNotify);
	SetCallbackAsPending(callbackID);
}

/**
 * @brief Send tune events to receiver
 *
 * @param[in] success - Tune status
 * @return void
 */
void PrivateInstanceAAMP::sendTuneMetrics(bool success)
{
	std::stringstream eventsJSON;
	profiler.getTuneEventsJSON(eventsJSON, getStreamTypeString(),GetTunedManifestUrl(),success);
	std::string jsonStr = eventsJSON.str();
	SendMessage2Receiver(E_AAMP2Receiver_EVENTS,jsonStr.c_str());

	//for now, avoid use of logprintf, to avoid potential truncation when URI in tune profiling or
	//extra events push us over limit
	logprintf("tune-profiling: %s", jsonStr.c_str());

	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_TUNE_PROFILING])
	{
		AsyncMicroEventDescriptor* e = new AsyncMicroEventDescriptor(jsonStr.c_str());
		ScheduleEvent(e);
	}
}

/**
 * @brief Notify tune end for profiling/logging
 */
void PrivateInstanceAAMP::LogTuneComplete(void)
{
	bool success = true; // TODO
	int streamType = getStreamType();
	profiler.TuneEnd(success, mContentType, streamType, mFirstTune, mAppName);

	//update tunedManifestUrl if FOG was NOT used as manifestUrl might be updated with redirected url.
    if(!IsTSBSupported())
    {
        SetTunedManifestUrl(); /* Redirect URL in case on VOD */
    }

	if (!mTuneCompleted)
	{
		char classicTuneStr[AAMP_MAX_PIPE_DATA_SIZE];
		profiler.GetClassicTuneTimeInfo(success, mTuneAttempts, mfirstTuneFmt, mPlayerLoadTime, streamType, IsLive(), durationSeconds, classicTuneStr);
		SendMessage2Receiver(E_AAMP2Receiver_TUNETIME,classicTuneStr);
		if(gpGlobalConfig->enableMicroEvents) sendTuneMetrics(success);
		mTuneCompleted = true;
		mFirstTune = false;

		AAMPAnomalyMessageType eMsgType = AAMPAnomalyMessageType::ANOMALY_TRACE;
		if(mTuneAttempts > 1 )
		{
		    eMsgType = AAMPAnomalyMessageType::ANOMALY_WARNING;
		}
		std::string playbackType = GetContentTypString();

		if(mContentType == ContentType_LINEAR)
		{
		    if(mTSBEnabled)
		    {
		        playbackType.append(":TSB=true");
		    }
		    else
		    {
		        playbackType.append(":TSB=false");
		    }
		}

		SendAnomalyEvent(eMsgType, "Tune attempt#%d. %s:%s URL:%s", mTuneAttempts,playbackType.c_str(),getStreamTypeString().c_str(),GetTunedManifestUrl());
	}

	gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_WARN);
}


/**
 * @brief Notifies profiler that first frame is presented
 */
void PrivateInstanceAAMP::LogFirstFrame(void)
{
	profiler.ProfilePerformed(PROFILE_BUCKET_FIRST_FRAME);
}


/**
 * @brief Notifies profiler that drm initialization is complete
 */
void PrivateInstanceAAMP::LogDrmInitComplete(void)
{
	profiler.ProfileEnd(PROFILE_BUCKET_LA_TOTAL);
}

/**
 * @brief Notifies profiler that decryption has started
 * @param bucketType profiler bucket type
 */
void PrivateInstanceAAMP::LogDrmDecryptBegin(ProfilerBucketType bucketType)
{
	profiler.ProfileBegin(bucketType);
}

/**
 * @brief Notifies profiler that decryption has ended
 * @param bucketType profiler bucket type
 */
void PrivateInstanceAAMP::LogDrmDecryptEnd(ProfilerBucketType bucketType)
{
	profiler.ProfileEnd(bucketType);
}


/**
 * @brief Log errors.
 * @param msg Error message
 */
void aamp_Error(const char *msg)
{
	logprintf("aamp ERROR: %s", msg);
	//exit(1);
}


/**
 * @brief Convert custom curl errors to original
 *
 * @param[in] http_error - Error code
 * @return error code
 */
long aamp_GetOriginalCurlError(long http_error)
{
	long ret = http_error;
	if (http_error >= PARTIAL_FILE_CONNECTIVITY_AAMP && http_error <= PARTIAL_FILE_START_STALL_TIMEOUT_AAMP)
	{
			if (http_error == OPERATION_TIMEOUT_CONNECTIVITY_AAMP)
			{
				ret = CURLE_OPERATION_TIMEDOUT;
			}
			else
			{
				ret = CURLE_PARTIAL_FILE;
			}
	}
	// return original error code
	return ret;
}


/**
 * @brief Free memory allocated by aamp_Malloc
 * @param[in][out] pptr Pointer to allocated memory
 */
void aamp_Free(char **pptr)
{
	void *ptr = *pptr;
	if (ptr)
	{
		g_free(ptr);
		*pptr = NULL;
	}
}


/**
 * @brief Stop downloads of all tracks.
 * Used by aamp internally to manage states
 */
void PrivateInstanceAAMP::StopDownloads()
{
	traceprintf ("PrivateInstanceAAMP::%s", __FUNCTION__);
	if (!mbDownloadsBlocked)
	{
		pthread_mutex_lock(&mLock);
		mbDownloadsBlocked = true;
		pthread_mutex_unlock(&mLock);
	}
}


/**
 * @brief Resume downloads of all tracks.
 * Used by aamp internally to manage states
 */
void PrivateInstanceAAMP::ResumeDownloads()
{
	traceprintf ("PrivateInstanceAAMP::%s", __FUNCTION__);
	if (mbDownloadsBlocked)
	{
		pthread_mutex_lock(&mLock);
		mbDownloadsBlocked = false;
		//log_current_time("gstreamer-needs-data");
		pthread_mutex_unlock(&mLock);
	}
}


/**
 * @brief Stop downloads for a track.
 * Called from StreamSink to control flow
 * @param type media type of the track
 */
void PrivateInstanceAAMP::StopTrackDownloads(MediaType type)
{ // called from gstreamer main event loop
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d", __FUNCTION__, (int) type);
	}
#endif
	if (!mbTrackDownloadsBlocked[type])
	{
		AAMPLOG_TRACE("gstreamer-enough-data from %s source", (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mbTrackDownloadsBlocked[type] = true;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Enter. type = %d", __FUNCTION__, (int) type);
}


/**
 * @brief Resume downloads for a track.
 * Called from StreamSink to control flow
 * @param type media type of the track
 */
void PrivateInstanceAAMP::ResumeTrackDownloads(MediaType type)
{ // called from gstreamer main event loop
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d", __FUNCTION__, (int) type);
	}
#endif
	if (mbTrackDownloadsBlocked[type])
	{
		AAMPLOG_TRACE("gstreamer-needs-data from %s source", (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mbTrackDownloadsBlocked[type] = false;
		//log_current_time("gstreamer-needs-data");
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d", __FUNCTION__, (int) type);
}

/**
 * @brief block until gstreamer indicates pipeline wants more data
 * @param cb callback called periodically, if non-null
 * @param periodMs delay between callbacks
 * @param track track index
 */
void PrivateInstanceAAMP::BlockUntilGstreamerWantsData(void(*cb)(void), int periodMs, int track)
{ // called from FragmentCollector thread; blocks until gstreamer wants data
	traceprintf("PrivateInstanceAAMP::%s Enter. type = %d and downloads:%d", __FUNCTION__, track, mbTrackDownloadsBlocked[track]);
	int elapsedMs = 0;
	while (mbDownloadsBlocked || mbTrackDownloadsBlocked[track])
	{
		if (!mDownloadsEnabled || mTrackInjectionBlocked[track])
		{
			logprintf("PrivateInstanceAAMP::%s interrupted. mDownloadsEnabled:%d mTrackInjectionBlocked:%d", __FUNCTION__, mDownloadsEnabled, mTrackInjectionBlocked[track]);
			break;
		}
		if (cb && periodMs)
		{ // support for background tasks, i.e. refreshing manifest while gstreamer doesn't need additional data
			if (elapsedMs >= periodMs)
			{
				cb();
				elapsedMs -= periodMs;
			}
			elapsedMs += 10;
		}
		InterruptableMsSleep(10);
	}
	traceprintf("PrivateInstanceAAMP::%s Exit. type = %d", __FUNCTION__, track);
}


/**
 * @brief Allocate memory to growable buffer
 * @param buffer growable buffer
 * @param len size of memory to be allocated
 */
void aamp_Malloc(struct GrowableBuffer *buffer, size_t len)
{
	assert(!buffer->ptr && !buffer->avail );
	buffer->ptr = (char *)g_malloc(len);
	buffer->avail = len;
}


/**
 * @brief Append data to buffer
 * @param buffer Growable buffer object pointer
 * @param ptr Buffer to append
 * @param len Buffer size
 */
void aamp_AppendBytes(struct GrowableBuffer *buffer, const void *ptr, size_t len)
{
	size_t required = buffer->len + len;
	if (required > buffer->avail)
	{
		if(buffer->avail > (128*1024))
		{
			logprintf("%s:%d WARNING - realloc. buf %p avail %d required %d", __FUNCTION__, __LINE__, buffer, (int)buffer->avail, (int)required);
		}
		buffer->avail = required * 2; // grow generously to minimize realloc overhead
		char *ptr = (char *)g_realloc(buffer->ptr, buffer->avail);
		assert(ptr);
		if (ptr)
		{
			if (buffer->ptr == NULL)
			{ // first alloc (not a realloc)
			}
			buffer->ptr = ptr;
		}
	}
	if (buffer->ptr)
	{
		memcpy(&buffer->ptr[buffer->len], ptr, len);
		buffer->len = required;
	}
}

/**
 * @struct CurlCallbackContext
 * @brief context during curl callbacks
 */
struct CurlCallbackContext
{
	PrivateInstanceAAMP *aamp;
	MediaType fileType;
	std::vector<std::string> allResponseHeadersForErrorLogging;
	GrowableBuffer *buffer;
	httpRespHeaderData *responseHeaderData;
	long bitrate;
	bool downloadIsEncoded;

	CurlCallbackContext() : aamp(NULL), buffer(NULL), responseHeaderData(NULL),bitrate(0),downloadIsEncoded(false), fileType(eMEDIATYPE_DEFAULT), allResponseHeadersForErrorLogging{""}
	{

	}

	~CurlCallbackContext() {}

	CurlCallbackContext(const CurlCallbackContext &other) = delete;
	CurlCallbackContext& operator=(const CurlCallbackContext& other) = delete;
};

/**
 * @struct CurlProgressCbContext
 * @brief context during curl progress callbacks
 */
struct CurlProgressCbContext
{
	PrivateInstanceAAMP *aamp;
	long long downloadStartTime;
	long long downloadUpdatedTime;
	long startTimeout;
	long stallTimeout;
	double downloadSize;
	CurlAbortReason abortReason;
};

/**
 * @brief write callback to be used by CURL
 * @param ptr pointer to buffer containing the data
 * @param size size of the buffer
 * @param nmemb number of bytes
 * @param userdata CurlCallbackContext pointer
 * @retval size consumed or 0 if interrupted
 */
static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	size_t ret = 0;
	CurlCallbackContext *context = (CurlCallbackContext *)userdata;
	pthread_mutex_lock(&context->aamp->mLock);
	if (context->aamp->mDownloadsEnabled)
	{
		size_t numBytesForBlock = size*nmemb;
		aamp_AppendBytes(context->buffer, ptr, numBytesForBlock);
		ret = numBytesForBlock;
	}
	else
	{
		logprintf("write_callback - interrupted");
	}
	pthread_mutex_unlock(&context->aamp->mLock);
	return ret;
}

/**
 * @brief function to print header response during download failure and latency.
 * @param type current media type
 */
static void print_headerResponse(std::vector<std::string> &allResponseHeadersForErrorLogging, MediaType fileType)
{
	if (gpGlobalConfig->logging.curlHeader && (eMEDIATYPE_VIDEO == fileType || eMEDIATYPE_PLAYLIST_VIDEO == fileType))
	{
		int size = allResponseHeadersForErrorLogging.size();
		logprintf("################ Start :: Print Header response ################");
		for (int i=0; i < size; i++)
		{
			logprintf("* %s", allResponseHeadersForErrorLogging.at(i).c_str());
		}
		logprintf("################ End :: Print Header response ################");
	}

	allResponseHeadersForErrorLogging.clear();
}

/**
 * @brief callback invoked on http header by curl
 * @param ptr pointer to buffer containing the data
 * @param size size of the buffer
 * @param nmemb number of bytes
 * @param user_data  CurlCallbackContext pointer
 * @retval
 */
static size_t header_callback(char *ptr, size_t size, size_t nmemb, void *user_data)
{
	//std::string *httpHeaders = static_cast<std::string *>(user_data);
	CurlCallbackContext *context = static_cast<CurlCallbackContext *>(user_data);
	httpRespHeaderData *httpHeader = context->responseHeaderData;
	size_t len = nmemb * size;
	int startPos = 0;
	bool isBitrateHeader = false;

	if( len<2 || ptr[len-2] != '\r' || ptr[len-1] != '\n' )
	{ // only proceed if this is a CRLF terminated curl header, as expected
		return len;
	}

	ptr[len-2] = 0x00; // replace the unprintable \r, and convert to NUL-terminated C-String

	if (gpGlobalConfig->logging.curlHeader &&
			ptr[0] &&
			(eMEDIATYPE_VIDEO == context->fileType || eMEDIATYPE_PLAYLIST_VIDEO == context->fileType))
	{
		context->allResponseHeadersForErrorLogging.push_back(ptr);
	}

    // As per Hypertext Transfer Protocol ==> Field names are case-insensitive
    // HTTP/1.1 4.2 Message Headers : Each header field consists of a name followed by a colon (":") and the field value. Field names are case-insensitive
    if (STARTS_WITH_IGNORE_CASE(ptr, FOG_REASON_STRING))
    {
        httpHeader->type = eHTTPHEADERTYPE_FOG_REASON;
        startPos = STRLEN_LITERAL(FOG_REASON_STRING);
    }
	else if (STARTS_WITH_IGNORE_CASE(ptr, CURLHEADER_X_REASON))
	{
		httpHeader->type = eHTTPHEADERTYPE_XREASON;
		startPos = STRLEN_LITERAL(CURLHEADER_X_REASON);
	}
	else if (STARTS_WITH_IGNORE_CASE(ptr, BITRATE_HEADER_STRING))
	{
		startPos = STRLEN_LITERAL(BITRATE_HEADER_STRING);
		isBitrateHeader = true;
	}
	else if (STARTS_WITH_IGNORE_CASE(ptr, "Set-Cookie:"))
	{
		httpHeader->type = eHTTPHEADERTYPE_COOKIE;
		startPos = STRLEN_LITERAL("Set-Cookie:");
	}
	else if (STARTS_WITH_IGNORE_CASE(ptr, "Location:"))
	{
		httpHeader->type = eHTTPHEADERTYPE_EFF_LOCATION;
		startPos = STRLEN_LITERAL("Location:");
	}
	else if (STARTS_WITH_IGNORE_CASE(ptr, "Content-Encoding:"))
	{
		// Enabled IsEncoded as Content-Encoding header is present
		// The Content-Encoding entity header incidcates media is compressed
		context->downloadIsEncoded = true;
	}
	else if (0 == context->buffer->avail)
	{
		if (STARTS_WITH_IGNORE_CASE(ptr, CONTENTLENGTH_STRING))
		{
			int contentLengthStartPosition = STRLEN_LITERAL(CONTENTLENGTH_STRING);
			char* contentLengthStr = ptr + contentLengthStartPosition;
			int contentLength = atoi(contentLengthStr);

			if(gpGlobalConfig->logging.trace)
			{
				traceprintf("%s:%d header %s contentLengthStr %s  contentLength %d",__FUNCTION__,__LINE__, ptr, contentLengthStr, contentLength);
			}

			/*contentLength can be zero for redirects*/
			if (contentLength > 0)
			{
				/*Add 2 additional characters to take care of extra characters inserted by aamp_AppendNulTerminator*/
				aamp_Malloc(context->buffer, contentLength + 2);
			}
		}
	}
	
	if(startPos > 0)
	{
		//Find the first character after the http header name
		int endPos = strlen(ptr) - 1;
		while ((ptr[endPos] == ' ') && (endPos >= startPos))
		{
			endPos--;
		}
		while ((ptr[startPos] == ' ') && (startPos <= endPos))
		{
			startPos++;
		}

		if(isBitrateHeader)
		{
			char* strBitrate = ptr + startPos;
			context->bitrate = atol(strBitrate);
			traceprintf("Parsed HTTP %s: %ld\n", isBitrateHeader? "Bitrate": "False", context->bitrate);
		}
		else
		{
			httpHeader->data = string((ptr + startPos), (endPos - startPos +1));
			if(httpHeader->type != eHTTPHEADERTYPE_EFF_LOCATION)
			{
				//Append a delimiter ";"
			 	httpHeader->data += ';';
			}
		}

		if(gpGlobalConfig->logging.trace)
		{
			traceprintf("Parsed HTTP %s header: %s", httpHeader->type==eHTTPHEADERTYPE_COOKIE? "Cookie": "X-Reason", httpHeader->data.c_str());
		}
	}
	return len;
}

/**
 * @brief
 * @param clientp app-specific as optionally set with CURLOPT_PROGRESSDATA
 * @param dltotal total bytes expected to download
 * @param dlnow downloaded bytes so far
 * @param ultotal total bytes expected to upload
 * @param ulnow uploaded bytes so far
 * @retval
 */
static int progress_callback(
	void *clientp, // app-specific as optionally set with CURLOPT_PROGRESSDATA
	double dltotal, // total bytes expected to download
	double dlnow, // downloaded bytes so far
	double ultotal, // total bytes expected to upload
	double ulnow // uploaded bytes so far
	)
{
	CurlProgressCbContext *context = (CurlProgressCbContext *)clientp;
	int rc = 0;
	context->aamp->SyncBegin();
	if (!context->aamp->mDownloadsEnabled)
	{
		rc = -1; // CURLE_ABORTED_BY_CALLBACK
	}
	context->aamp->SyncEnd();
	if( rc==0 )
	{ // only proceed if not an aborted download
		if (dlnow > 0 && context->stallTimeout > 0)
		{
			if (context->downloadSize == -1)
			{ // first byte(s) downloaded
				context->downloadSize = dlnow;
				context->downloadUpdatedTime = NOW_STEADY_TS_MS;
			}
			else
			{
				if (dlnow == context->downloadSize)
				{ // no change in downloaded bytes - check time since last update to infer stall
					double timeElapsedSinceLastUpdate = (NOW_STEADY_TS_MS - context->downloadUpdatedTime) / 1000.0; //in secs
					if (timeElapsedSinceLastUpdate >= context->stallTimeout)
					{ // no change for at least <stallTimeout> seconds - consider download stalled and abort
						logprintf("Abort download as mid-download stall detected for %.2f seconds, download size:%.2f bytes", timeElapsedSinceLastUpdate, dlnow);
						context->abortReason = eCURL_ABORT_REASON_STALL_TIMEDOUT;
						rc = -1;
					}
				}
				else
				{ // received additional bytes - update state to track new size/time
					context->downloadSize = dlnow;
					context->downloadUpdatedTime = NOW_STEADY_TS_MS;
				}
			}
		}
		else if (dlnow == 0 && context->startTimeout > 0)
		{ // check to handle scenario where <startTimeout> seconds delay occurs without any bytes having been downloaded (stall at start)
			double timeElapsedInSec = (NOW_STEADY_TS_MS - context->downloadStartTime) / 1000; //in secs
			if (timeElapsedInSec >= context->startTimeout)
			{
				logprintf("Abort download as no data received for %.2f seconds", timeElapsedInSec);
				context->abortReason = eCURL_ABORT_REASON_START_TIMEDOUT;
				rc = -1;
			}
		}
	}
	return rc;
}

static int eas_curl_debug_callback(CURL *handle, curl_infotype type, char *data, size_t size, void *userp)
{
	(void)handle;
	(void)userp;
	(void)size;

	if(type == CURLINFO_TEXT || type == CURLINFO_HEADER_IN)
	{
	//remove unwanted trailing line feeds from log
	for(int i = (int)size-1; i >= 0; i--)
	{
	    if(data[i] == '\n' || data[i] == '\r')
		data[i] = '\0';
	    else
		break;
	}

	//limit log spam to only TEXT and HEADER_IN
	switch (type) {
		case CURLINFO_TEXT:
		logprintf("curl: %s", data);
		break;
		case CURLINFO_HEADER_IN:
		logprintf("curl header: %s", data);
		break;
	    default:
		break;
	}
	}
	return 0;
}

/**
 * @brief
 * @param curl ptr to CURL instance
 * @param ssl_ctx SSL context used by CURL
 * @param user_ptr data pointer set as param to CURLOPT_SSL_CTX_DATA
 * @retval CURLcode CURLE_OK if no errors, otherwise corresponding CURL code
 */
CURLcode ssl_callback(CURL *curl, void *ssl_ctx, void *user_ptr)
{
	PrivateInstanceAAMP *context = (PrivateInstanceAAMP *)user_ptr;
	CURLcode rc = CURLE_OK;
	pthread_mutex_lock(&context->mLock);
	if (!context->mDownloadsEnabled)
	{
		rc = CURLE_ABORTED_BY_CALLBACK ; // CURLE_ABORTED_BY_CALLBACK
	}
	pthread_mutex_unlock(&context->mLock);
	return rc;
}

/**
 * @brief Initialize curl instances
 * @param startIdx start index
 * @param instanceCount count of instances
 */
void PrivateInstanceAAMP::CurlInit(AampCurlInstance startIdx, unsigned int instanceCount, const char *proxy)
{
	int instanceEnd = startIdx + instanceCount;
	assert (instanceEnd <= eCURLINSTANCE_MAX);
	for (unsigned int i = startIdx; i < instanceEnd; i++)
	{
		if (!curl[i])
		{
			curl[i] = curl_easy_init();
			if (gpGlobalConfig->logging.curl)
			{
				curl_easy_setopt(curl[i], CURLOPT_VERBOSE, 1L);
			}
			curl_easy_setopt(curl[i], CURLOPT_NOSIGNAL, 1L);
			//curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback); // unused
			curl_easy_setopt(curl[i], CURLOPT_PROGRESSFUNCTION, progress_callback);
			curl_easy_setopt(curl[i], CURLOPT_HEADERFUNCTION, header_callback);
			curl_easy_setopt(curl[i], CURLOPT_WRITEFUNCTION, write_callback);
			curl_easy_setopt(curl[i], CURLOPT_TIMEOUT, DEFAULT_CURL_TIMEOUT);
			curl_easy_setopt(curl[i], CURLOPT_CONNECTTIMEOUT, DEFAULT_CURL_CONNECTTIMEOUT);
			curl_easy_setopt(curl[i], CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
			curl_easy_setopt(curl[i], CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl[i], CURLOPT_NOPROGRESS, 0L); // enable progress meter (off by default)
			curl_easy_setopt(curl[i], CURLOPT_USERAGENT, gpGlobalConfig->pUserAgentString);
			curl_easy_setopt(curl[i], CURLOPT_ACCEPT_ENCODING, "");//Enable all the encoding formats supported by client
			curl_easy_setopt(curl[i], CURLOPT_SSL_CTX_FUNCTION, ssl_callback); //Check for downloads disabled in btw ssl handshake
			curl_easy_setopt(curl[i], CURLOPT_SSL_CTX_DATA, this);

			curlDLTimeout[i] = DEFAULT_CURL_TIMEOUT * 1000;


			// dev override in cfg file takes priority to App Setting 
			if(gpGlobalConfig->httpProxy != NULL)
			{
				proxy = gpGlobalConfig->httpProxy;				
			}

			if (proxy != NULL)
			{
				/* use this proxy */
				curl_easy_setopt(curl[i], CURLOPT_PROXY, proxy);
				/* allow whatever auth the proxy speaks */
				curl_easy_setopt(curl[i], CURLOPT_PROXYAUTH, CURLAUTH_ANY);
			}

			if(ContentType_EAS == mContentType)
			{
				//enable verbose logs so we can debug field issues
				curl_easy_setopt(curl[i], CURLOPT_VERBOSE, 1);
				curl_easy_setopt(curl[i], CURLOPT_DEBUGFUNCTION, eas_curl_debug_callback);
				//set eas specific timeouts to handle faster cycling through bad hosts and faster total timeout
				curl_easy_setopt(curl[i], CURLOPT_TIMEOUT, EAS_CURL_TIMEOUT);
				curl_easy_setopt(curl[i], CURLOPT_CONNECTTIMEOUT, EAS_CURL_CONNECTTIMEOUT);

				curlDLTimeout[i] = EAS_CURL_TIMEOUT * 1000;

				//on ipv6 box force curl to use ipv6 mode only (DELIA-20209)
				struct stat tmpStat;
				bool isv6(::stat( "/tmp/estb_ipv6", &tmpStat) == 0);
				if(isv6)
					curl_easy_setopt(curl[i], CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
				logprintf("aamp eas curl config: timeout=%d, connecttimeout%d, ipv6=%d", EAS_CURL_TIMEOUT, EAS_CURL_CONNECTTIMEOUT, isv6);
			}
			//log_current_time("curl initialized");
		}
	}
}

/**
 *   @brief Converts lang index to Audio Track type
 *
 *   @param[in] int - Audio Lang Index
 *   @return VideoStatTrackType
 */
VideoStatTrackType PrivateInstanceAAMP::ConvertAudioIndexToVideoStatTrackType(int Index)
{
	VideoStatTrackType type = VideoStatTrackType::STAT_UNKNOWN;
	switch(Index)
	{
		case 0 :
		{
			type = VideoStatTrackType::STAT_AUDIO_1;
		}
		break;
		case 1 :
		{
			type = VideoStatTrackType::STAT_AUDIO_2;
		}
		break;
		case 2:
		{
			type = VideoStatTrackType::STAT_AUDIO_3;
		}
		break;
		case 3 :
		{
			type = VideoStatTrackType::STAT_AUDIO_4;
		}
		break;
		case 4 :
		{
			type = VideoStatTrackType::STAT_AUDIO_5;
		}
		break;

		default:
			break;
	}
	return type;
}

/**
 * @brief Store language list of stream
 * @param maxLangCount count of language item to be stored
 * @param langlist Array of languges
 */
void PrivateInstanceAAMP::StoreLanguageList(int maxLangCount , char langlist[][MAX_LANGUAGE_TAG_LENGTH])
{
	// store the language list
	if (maxLangCount > MAX_LANGUAGE_COUNT)
	{
		maxLangCount = MAX_LANGUAGE_COUNT; //boundary check
	}
	mMaxLanguageCount = maxLangCount;
	for (int cnt=0; cnt < maxLangCount; cnt ++)
	{
		strncpy(mLanguageList[cnt],langlist[cnt],MAX_LANGUAGE_TAG_LENGTH);
		mLanguageList[cnt][MAX_LANGUAGE_TAG_LENGTH-1] = 0;
		if( this->mVideoEnd )
		{
			mVideoEnd->Setlanguage(ConvertAudioIndexToVideoStatTrackType(cnt),langlist[cnt]);
		}
	}
}


/**
 * @brief Check if audio language is supported
 * @param checkLanguage language string to be checked
 * @retval true if supported, false if not supported
 */
bool PrivateInstanceAAMP::IsAudioLanguageSupported (const char *checkLanguage)
{
	bool retVal =false;
	for (int cnt=0; cnt < mMaxLanguageCount; cnt ++)
	{
		if(strncmp(mLanguageList[cnt],checkLanguage,MAX_LANGUAGE_TAG_LENGTH) == 0)
		{
			retVal = true;
			break;
		}
	}

	if(mMaxLanguageCount == 0)
	{
                logprintf("IsAudioLanguageSupported No Audio language stored !!!");
	}
	else if(!retVal)
	{
		logprintf("IsAudioLanguageSupported lang[%s] not available in list",checkLanguage);
	}
	return retVal;
}


/**
 * @brief Set curl timeout (CURLOPT_TIMEOUT)
 * @param timeout maximum time  in seconds curl request is allowed to take
 * @param instance index of instance to which timeout to be set
 */
void PrivateInstanceAAMP::SetCurlTimeout(long timeoutMS, AampCurlInstance instance)
{
	if(ContentType_EAS == mContentType)
		return;
	if(instance < eCURLINSTANCE_MAX && curl[instance])
	{
		curl_easy_setopt(curl[instance], CURLOPT_TIMEOUT_MS, timeoutMS);
		curlDLTimeout[instance] = timeoutMS;
	}
	else
	{
		logprintf("Failed to update timeout for curl instace %d",instance);
	}
}

/**
 * @brief Terminate curl instances
 * @param startIdx start index
 * @param instanceCount count of instances
 */
void PrivateInstanceAAMP::CurlTerm(AampCurlInstance startIdx, unsigned int instanceCount)
{
	int instanceEnd = startIdx + instanceCount;
	assert (instanceEnd <= eCURLINSTANCE_MAX);
	for (unsigned int i = startIdx; i < instanceEnd; i++)
	{
		if (curl[i])
		{
			curl_easy_cleanup(curl[i]);
			curl[i] = NULL;
			curlDLTimeout[i] = 0;
		}
	}
}

/**
 * @brief GetPlaylistCurlInstance - Function to return the curl instance for playlist download
 * Considers parallel download to decide the curl instance 
 * @param MediaType - Playlist type 
 * @param Init/Refresh - When playlist download is done 
 * @retval AampCurlInstance - Curl instance for playlist download
 */
AampCurlInstance PrivateInstanceAAMP::GetPlaylistCurlInstance(MediaType type, bool isInitialDownload)
{
	AampCurlInstance retType = eCURLINSTANCE_MANIFEST_PLAYLIST;
	bool indivCurlInstanceFlag = false;

	//DELIA-41646
	// logic behind this function :
	// a. This function gets called during Init and during Refresh of playlist .So need to decide who called
	// b. Based on the decision flag is considerd . mParallelFetchPlaylist for Init and mParallelFetchPlaylistRefresh
	//	  for refresh
	// c. If respective configuration is enabled , then associate separate curl for each track type
	// d. If parallel fetch is disabled , then single curl instance is used to fetch all playlist(eCURLINSTANCE_MANIFEST_PLAYLIST)

	indivCurlInstanceFlag = isInitialDownload ? mParallelFetchPlaylist : mParallelFetchPlaylistRefresh;
	if(indivCurlInstanceFlag)
	{
		switch(type)
		{
			case eMEDIATYPE_PLAYLIST_VIDEO:
				retType = eCURLINSTANCE_VIDEO;
				break;
			case eMEDIATYPE_PLAYLIST_AUDIO:
				retType = eCURLINSTANCE_AUDIO;
				break;
			case eMEDIATYPE_PLAYLIST_SUBTITLE:
				retType = eCURLINSTANCE_SUBTITLE;
				break;
			default:
				break;
		}
	}
	return retType;
}



/**
 * @brief called when tuning - reset artificially
 * low for quicker tune times
 * @param bitsPerSecond
 * @param trickPlay
 * @param profile
 */
void PrivateInstanceAAMP::ResetCurrentlyAvailableBandwidth(long bitsPerSecond , bool trickPlay,int profile)
{
	pthread_mutex_lock(&mLock);
	if (mAbrBitrateData.size())
	{
		mAbrBitrateData.erase(mAbrBitrateData.begin(),mAbrBitrateData.end());
	}
	pthread_mutex_unlock(&mLock);
}

/**
 * @brief estimate currently available bandwidth, 
 * using most recently recorded 3 samples
 * @retval currently available bandwidth
 */
long PrivateInstanceAAMP::GetCurrentlyAvailableBandwidth(void)
{
	long avg = 0;
	long ret = -1;
	// 1. Check for any old bitrate beyond threshold time . remove those before calculation
	// 2. Sort and get median 
	// 3. if any outliers  , remove those entries based on a threshold value.
	// 4. Get the average of remaining data. 
	// 5. if no item in the list , return -1 . Caller to ignore bandwidth based processing
	
	std::vector< std::pair<long long,long> >::iterator bitrateIter;
	std::vector< long> tmpData;
	std::vector< long>::iterator tmpDataIter;
	long long presentTime = aamp_GetCurrentTimeMS();
	pthread_mutex_lock(&mLock);
	for (bitrateIter = mAbrBitrateData.begin(); bitrateIter != mAbrBitrateData.end();)
	{
		//logprintf("[%s][%d] Sz[%d] TimeCheck Pre[%lld] Sto[%lld] diff[%lld] bw[%ld] ",__FUNCTION__,__LINE__,mAbrBitrateData.size(),presentTime,(*bitrateIter).first,(presentTime - (*bitrateIter).first),(long)(*bitrateIter).second);
		if ((bitrateIter->first <= 0) || (presentTime - bitrateIter->first > gpGlobalConfig->abrCacheLife))
		{
			//logprintf("[%s][%d] Threadshold time reached , removing bitrate data ",__FUNCTION__,__LINE__);
			bitrateIter = mAbrBitrateData.erase(bitrateIter);
		}
		else
		{
			tmpData.push_back(bitrateIter->second);
			bitrateIter++;
		}
	}
	pthread_mutex_unlock(&mLock);

	if (tmpData.size())
	{	
		long medianbps=0;

		std::sort(tmpData.begin(),tmpData.end());
		if (tmpData.size() %2)
		{
			medianbps = tmpData.at(tmpData.size()/2);
		}
		else
		{
			long m1 = tmpData.at(tmpData.size()/2);
			long m2 = tmpData.at(tmpData.size()/2)+1;
			medianbps = (m1+m2)/2;
		} 
	
		long diffOutlier = 0;
		avg = 0;
		for (tmpDataIter = tmpData.begin();tmpDataIter != tmpData.end();)
		{
			diffOutlier = (*tmpDataIter) > medianbps ? (*tmpDataIter) - medianbps : medianbps - (*tmpDataIter);
			if (diffOutlier > gpGlobalConfig->abrOutlierDiffBytes)
			{
				//logprintf("[%s][%d] Outlier found[%ld]>[%ld] erasing ....",__FUNCTION__,__LINE__,diffOutlier,gpGlobalConfig->abrOutlierDiffBytes);
				tmpDataIter = tmpData.erase(tmpDataIter);
			}
			else
			{
				avg += (*tmpDataIter);
				tmpDataIter++;	
			}
		}
		if (tmpData.size())
		{
			//logprintf("[%s][%d] NwBW with newlogic size[%d] avg[%ld] ",__FUNCTION__,__LINE__,tmpData.size(), avg/tmpData.size());
			ret = (avg/tmpData.size());
			mAvailableBandwidth = ret;
		}	
		else
		{
			//logprintf("[%s][%d] No prior data available for abr , return -1 ",__FUNCTION__,__LINE__);
			ret = -1;
		}
	}
	else
	{
		//logprintf("[%s][%d] No data available for bitrate check , return -1 ",__FUNCTION__,__LINE__);
		ret = -1;
	}
	
	return ret;
}

/**
 * @brief Get MediaType as String
 */
const char* PrivateInstanceAAMP::MediaTypeString(MediaType fileType)
{
	switch(fileType)
	{
		case eMEDIATYPE_VIDEO:
		case eMEDIATYPE_INIT_VIDEO:
			return "VIDEO";
		case eMEDIATYPE_AUDIO:
		case eMEDIATYPE_INIT_AUDIO:
			return "AUDIO";
		case eMEDIATYPE_SUBTITLE:
		case eMEDIATYPE_INIT_SUBTITLE:
			return "SUBTITLE";
		case eMEDIATYPE_MANIFEST:
			return "MANIFEST";
		case eMEDIATYPE_LICENCE:
			return "LICENCE";
		case eMEDIATYPE_IFRAME:
			return "IFRAME";
		case eMEDIATYPE_PLAYLIST_VIDEO:
			return "PLAYLIST_VIDEO";
		case eMEDIATYPE_PLAYLIST_AUDIO:
			return "PLAYLIST_AUDIO";
		case eMEDIATYPE_PLAYLIST_SUBTITLE:
			return "PLAYLIST_SUBTITLE";
		default:
			return "Unknown";
	}
}

/**
 * @brief Simulate VOD asset as a "virtual linear" stream.
 */
static void SimulateLinearWindow( struct GrowableBuffer *buffer, const char *ptr, size_t len )
{
	// Calculate elapsed time in seconds since virtual linear stream started
	float cull = (aamp_GetCurrentTimeMS() - simulation_start)/1000.0;
	buffer->len = 0; // Reset Growable Buffer length
	float window = 20.0; // Virtual live window size; can be increased/decreasedint
	const char *fin = ptr+len;
	bool wroteHeader = false; // Internal state used to decide whether HLS playlist header has already been output
	int seqNo = 0;

	while (ptr < fin)
	{
		int count = 0;
		char line[1024];
		float fragmentDuration;

		for(;;)
		{
			char c = *ptr++;
			line[count++] = c;
			if( ptr>=fin || c<' ' ) break;
		}

		line[count] = 0x00;

		if (sscanf(line,"#EXTINF:%f",&fragmentDuration) == 1)
		{
			if (cull > 0)
			{
				cull -= fragmentDuration;
				seqNo++;
				continue; // Not yet in active window
			}

			if (!wroteHeader)
			{
				// Write a simple linear HLS header, without the type:VOD, and with dynamic media sequence number
				wroteHeader = true;
				char header[1024];
				sprintf( header,
					"#EXTM3U\n"
					"#EXT-X-VERSION:3\n"
					"#EXT-X-TARGETDURATION:2\n"
					"#EXT-X-MEDIA-SEQUENCE:%d\n", seqNo );
				aamp_AppendBytes(buffer, header, strlen(header) );
			}

			window -= fragmentDuration;

			if (window < 0.0)
			{
				// Finished writing virtual linear window
				break;
			}
		}

		if (wroteHeader)
		{
			aamp_AppendBytes(buffer, line, count );
		}
	}

	// Following can be used to debug
	// aamp_AppendNulTerminator( buffer );
	// printf( "Virtual Linear Playlist:\n%s\n***\n", buffer->ptr );
}

/**
 * @brief Fetch a file from CDN
 * @param remoteUrl url of the file
 * @param[out] buffer pointer to buffer abstraction
 * @param[out] effectiveUrl last effective URL
 * @param http_error error code in case of failure
 * @param range http range
 * @param curlInstance instance to be used to fetch
 * @param resetBuffer true to reset buffer before fetch
 * @param fileType media type of the file
 * @param fragmentDurationSeconds to know the current fragment length in case fragment fetch
 * @retval true if success
 */
bool PrivateInstanceAAMP::GetFile(std::string remoteUrl,struct GrowableBuffer *buffer, std::string& effectiveUrl, 
				long * http_error, const char *range, unsigned int curlInstance, 
				bool resetBuffer, MediaType fileType, long *bitrate, int * fogError,
				double fragmentDurationSeconds 	)
{
	MediaType simType = fileType; // remember the requested specific file type; fileType gets overridden later with simple VIDEO/AUDIO
	long http_code = -1;
	bool ret = false;
	int downloadAttempt = 0;
	int maxDownloadAttempt = 1;
	CURL* curl = this->curl[curlInstance];
	struct curl_slist* httpHeaders = NULL;
	CURLcode res = CURLE_OK;
	long long fragmentDurationMs = 0;

	if (simType == eMEDIATYPE_INIT_VIDEO || simType == eMEDIATYPE_INIT_AUDIO)
	{
		maxDownloadAttempt += mInitFragmentRetryCount;
	}
	else
	{
		maxDownloadAttempt += DEFAULT_DOWNLOAD_RETRY_COUNT;
	}

	pthread_mutex_lock(&mLock);
	if (resetBuffer)
	{
		if(buffer->avail)
        	{
            		AAMPLOG_TRACE("%s:%d reset buffer %p avail %d", __FUNCTION__, __LINE__, buffer, (int)buffer->avail);
        	}	
		memset(buffer, 0x00, sizeof(*buffer));
	}
	if (mDownloadsEnabled)
	{
		long long downloadTimeMS = 0;
		bool isDownloadStalled = false;
		CurlAbortReason abortReason = eCURL_ABORT_REASON_NONE;
		double connectTime = 0;
		pthread_mutex_unlock(&mLock);

		// append custom uri parameter with remoteUrl at the end before curl request if curlHeader logging enabled.
		if (gpGlobalConfig->logging.curlHeader && gpGlobalConfig->uriParameter && simType == eMEDIATYPE_MANIFEST)
		{
			if (remoteUrl.find("?") == std::string::npos)
			{
				gpGlobalConfig->uriParameter[0] = '?';
			}

			remoteUrl.append(gpGlobalConfig->uriParameter);
			//printf ("URL after appending uriParameter :: %s\n", remoteUrl.c_str());
		}

		AAMPLOG_INFO("aamp url: %s", remoteUrl.c_str());
		CurlCallbackContext context;
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, remoteUrl.c_str());

			context.aamp = this;
			context.buffer = buffer;
			context.responseHeaderData = &httpRespHeaders[curlInstance];
			context.fileType = simType;
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &context);
			if(gpGlobalConfig->disableSslVerifyPeer)
			{
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
			}
			else
			{
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
			}

			CurlProgressCbContext progressCtx;
			progressCtx.aamp = this;
			//Disable download stall detection checks for FOG playback done by JS PP
			if(simType == eMEDIATYPE_MANIFEST || simType == eMEDIATYPE_PLAYLIST_VIDEO || 
				simType == eMEDIATYPE_PLAYLIST_AUDIO || simType == eMEDIATYPE_PLAYLIST_SUBTITLE ||
				simType == eMEDIATYPE_PLAYLIST_IFRAME)
			{				
				progressCtx.startTimeout = 0;
			}
			else
			{
				progressCtx.stallTimeout = gpGlobalConfig->curlStallTimeout;
			}
			progressCtx.stallTimeout = gpGlobalConfig->curlStallTimeout;
                  
			// note: win32 curl lib doesn't support multi-part range
			curl_easy_setopt(curl, CURLOPT_RANGE, range);

			if ((httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_COOKIE) && (httpRespHeaders[curlInstance].data.length() > 0))
			{
				traceprintf("Appending cookie headers to HTTP request");
				//curl_easy_setopt(curl, CURLOPT_COOKIE, cookieHeaders[curlInstance].c_str());
				curl_easy_setopt(curl, CURLOPT_COOKIE, httpRespHeaders[curlInstance].data.c_str());
			}
			if (mCustomHeaders.size() > 0)
			{
				std::string customHeader;
				std::string headerValue;
				for (std::unordered_map<std::string, std::vector<std::string>>::iterator it = mCustomHeaders.begin();
									it != mCustomHeaders.end(); it++)
				{
					customHeader.clear();
					headerValue.clear();
					customHeader.insert(0, it->first);
					customHeader.push_back(' ');
					headerValue = it->second.at(0);
					if (it->first.compare("X-MoneyTrace:") == 0)
					{
						if (mIsLocalPlayback && !mIsFirstRequestToFOG)
						{
							continue;
						}
						char buf[512];
						memset(buf, '\0', 512);
						if (it->second.size() >= 2)
						{
							snprintf(buf, 512, "trace-id=%s;parent-id=%s;span-id=%lld",
									(const char*)it->second.at(0).c_str(),
									(const char*)it->second.at(1).c_str(),
									aamp_GetCurrentTimeMS());
						}
						else if (it->second.size() == 1)
						{
							snprintf(buf, 512, "trace-id=%s;parent-id=%lld;span-id=%lld",
									(const char*)it->second.at(0).c_str(),
									aamp_GetCurrentTimeMS(),
									aamp_GetCurrentTimeMS());
						}
						headerValue = buf;
					}
					customHeader.append(headerValue);
					httpHeaders = curl_slist_append(httpHeaders, customHeader.c_str());
				}

				if (gpGlobalConfig->logging.curlHeader && (eMEDIATYPE_VIDEO == simType || eMEDIATYPE_PLAYLIST_VIDEO == simType))
				{
					int size = gpGlobalConfig->customHeaderStr.size();
					for (int i=0; i < size; i++)
					{
						if (!gpGlobalConfig->customHeaderStr.at(i).empty())
						{
							//logprintf ("Custom Header Data: Index( %d ) Data( %s )", i, gpGlobalConfig->customHeaderStr.at(i).c_str());
							httpHeaders = curl_slist_append(httpHeaders, gpGlobalConfig->customHeaderStr.at(i).c_str());
						}
					}
				}

				if (httpHeaders != NULL)
				{
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpHeaders);
				}
			}

			while(downloadAttempt < maxDownloadAttempt)
			{
				progressCtx.downloadStartTime = NOW_STEADY_TS_MS;
				progressCtx.downloadUpdatedTime = -1;
				progressCtx.downloadSize = -1;
				progressCtx.abortReason = eCURL_ABORT_REASON_NONE;
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressCtx);
				if(buffer->ptr != NULL)
				{
					traceprintf("%s:%d reset length. buffer %p avail %d", __FUNCTION__, __LINE__, buffer, (int)buffer->avail);
					buffer->len = 0;
				}

				isDownloadStalled = false;
				abortReason = eCURL_ABORT_REASON_NONE;

				long long tStartTime = NOW_STEADY_TS_MS;
				CURLcode res = curl_easy_perform(curl); // synchronous; callbacks allow interruption

//				InterruptableMsSleep( 250 ); // this can be uncommented to locally induce extra per-download latency

				long long tEndTime = NOW_STEADY_TS_MS;
				downloadAttempt++;

				downloadTimeMS = tEndTime - tStartTime;
				bool loopAgain = false;
				if (res == CURLE_OK)
				{ // all data collected
					curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
					char *effectiveUrlPtr = NULL;
					if (http_code != 200 && http_code != 204 && http_code != 206)
					{
						AAMP_LOG_NETWORK_ERROR (remoteUrl.c_str(), AAMPNetworkErrorHttp, (int)http_code, simType);
						print_headerResponse(context.allResponseHeadersForErrorLogging, simType);

						if((http_code >= 500 && http_code != 502) && downloadAttempt < maxDownloadAttempt)
						{
							InterruptableMsSleep(gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS);
							logprintf("Download failed due to Server error. Retrying Attempt:%d!", downloadAttempt);
							loopAgain = true;
						}
					}
					if(http_code == 204)
					{
						if ( (httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_EFF_LOCATION) && (httpRespHeaders[curlInstance].data.length() > 0) )
						{
							logprintf("%s:%d Received Location header: '%s'",__FUNCTION__,__LINE__, httpRespHeaders[curlInstance].data.c_str());
							effectiveUrlPtr =  const_cast<char *>(httpRespHeaders[curlInstance].data.c_str());
						}
					}
					else
					{
						res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrlPtr);
					}
					effectiveUrl.assign(effectiveUrlPtr);

					// check if redirected url is pointing to fog / local ip
					if(mIsFirstRequestToFOG)
					{
					    if( effectiveUrl.find(LOCAL_HOST_IP) == std::string::npos )
					    {
					        // oops, TSB is not working, we got redirected away from fog
					        mIsLocalPlayback = false;
					        mTSBEnabled = false;
					        logprintf("NO_TSB_AVAILABLE playing from:%s ", effectiveUrl.c_str());
					    }
					    // updating here because, tune request can be for fog but fog may redirect to cdn in some cases
					    this->UpdateVideoEndTsbStatus(mTSBEnabled);
					}

					/*
					 * Latency should be printed in the case of successful download which exceeds the download threshold value,
					 * other than this case is assumed as network error and those will be logged with AAMP_LOG_NETWORK_ERROR.
					 */
					if (fragmentDurationSeconds != 0.0)
					{ 
						/*in case of fetch fragment this will be non zero value */
						fragmentDurationMs = (long long)(fragmentDurationSeconds*1000);/*convert to MS */
						if (downloadTimeMS > fragmentDurationMs )
						{
							AAMP_LOG_NETWORK_LATENCY (effectiveUrl.c_str(), downloadTimeMS, fragmentDurationMs, simType);
						}
					}
					else if (downloadTimeMS > FRAGMENT_DOWNLOAD_WARNING_THRESHOLD )
					{
						AAMP_LOG_NETWORK_LATENCY (effectiveUrl.c_str(), downloadTimeMS, FRAGMENT_DOWNLOAD_WARNING_THRESHOLD, simType);
						print_headerResponse(context.allResponseHeadersForErrorLogging, simType);
					}
				}
				else
				{
					long curlDownloadTimeoutMS = curlDLTimeout[curlInstance]; // curlDLTimeout is in msec
					//abortReason for progress_callback exit scenarios
					// curl sometimes exceeds the wait time by few milliseconds.Added buffer of 10msec
					isDownloadStalled = ((res == CURLE_OPERATION_TIMEDOUT || res == CURLE_PARTIAL_FILE ||
									(progressCtx.abortReason != eCURL_ABORT_REASON_NONE)) &&
									(buffer->len >= 0) &&
									((downloadTimeMS-10) <= curlDownloadTimeoutMS));
					// set flag if download aborted with start/stall timeout.
					abortReason = progressCtx.abortReason;

					/* Curl 23 and 42 is not a real network error, so no need to log it here */
					//Log errors due to curl stall/start detection abort
					if (AAMP_IS_LOG_WORTHY_ERROR(res) || progressCtx.abortReason != eCURL_ABORT_REASON_NONE)
					{
						AAMP_LOG_NETWORK_ERROR (remoteUrl.c_str(), AAMPNetworkErrorCurl, (int)(progressCtx.abortReason == eCURL_ABORT_REASON_NONE ? res : CURLE_PARTIAL_FILE), simType);
						print_headerResponse(context.allResponseHeadersForErrorLogging, simType);
					}

					//Attempt retry for local playback since rampdown is disabled for FOG
					//Attempt retry for partial downloads, which have a higher chance to succeed
					if((res == CURLE_COULDNT_CONNECT || (res == CURLE_OPERATION_TIMEDOUT && mIsLocalPlayback) || isDownloadStalled) && downloadAttempt < maxDownloadAttempt)
					{
						if(mpStreamAbstractionAAMP)
						{
							if( simType == eMEDIATYPE_MANIFEST ||
								simType == eMEDIATYPE_AUDIO ||
							    simType == eMEDIATYPE_INIT_VIDEO ||
							    simType == eMEDIATYPE_INIT_AUDIO )
							{ // always retry small, critical fragments on timeout
								loopAgain = true;
							}
							else
							{
								double buffer = mpStreamAbstractionAAMP->GetBufferedDuration();
								// buffer is -1 when sesssion not created . buffer is 0 when session created but playlist not downloaded
								if( buffer == -1.0 || buffer == 0 || (buffer*1000 > curlDownloadTimeoutMS) )
								{
									// GetBuffer will return -1 if session is not created
									// Check if buffer is available and more than timeout interval then only reattempt
									// Not to retry download if there is no buffer left
									loopAgain = true;
								}
							}
						}						
						logprintf("Download failed due to curl timeout or isDownloadStalled:%d Retrying:%d Attempt:%d", isDownloadStalled, loopAgain, downloadAttempt);
					}

					/*
					* Assigning curl error to http_code, for sending the error code as
					* part of error event if required
					* We can distinguish curl error and http error based on value
					*curl errors are below 100 and http error starts from 100
					*/
					http_code = res;

					if (isDownloadStalled)
					{
						AAMPLOG_INFO("Curl download stall detected - curl result:%d abortReason:%d downloadTimeMS:%lld curlTimeout:%ld", res, progressCtx.abortReason,
									downloadTimeMS, curlDownloadTimeoutMS);
						//To avoid updateBasedonFragmentCached being called on rampdown and to be discarded from ABR
						http_code = CURLE_PARTIAL_FILE;
					}
				}

				if(gpGlobalConfig->enableMicroEvents && fileType != eMEDIATYPE_DEFAULT) //Unknown filetype
				{
					profiler.addtuneEvent(mediaType2Bucket(fileType),tStartTime,downloadTimeMS,(int)(http_code));
				}

				double total, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize;
				long reqSize;
				AAMP_LogLevel reqEndLogLevel = eLOGLEVEL_INFO;

				curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME , &total);
				curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect);
				connectTime = connect;
				if(res != CURLE_OK || http_code == 0 || http_code >= 400 || total > 2.0 /*seconds*/)
				{
					reqEndLogLevel = eLOGLEVEL_WARN;
				}
				if (gpGlobalConfig->logging.isLogLevelAllowed(reqEndLogLevel))
				{
					double totalPerformRequest = (double)(downloadTimeMS)/1000;
					curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &resolve);
					curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &appConnect);
					curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &preTransfer);
					curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &startTransfer);
					curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &redirect);
					curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dlSize);
					curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &reqSize);
					AAMPLOG(reqEndLogLevel, "HttpRequestEnd: {\"url\":\"%.500s\",\"curlTime\":%2.4f,\"times\":{\"total\":%2.4f,\"connect\":%2.4f,\"startTransfer\":%2.4f,\"resolve\":%2.4f,\"appConnect\":%2.4f,\"preTransfer\":%2.4f,\"redirect\":%2.4f,\"dlSz\":%g,\"ulSz\":%ld},\"responseCode\":%ld,\"type\":%d}",
						((res == CURLE_OK) ? effectiveUrl.c_str() : remoteUrl.c_str()), // Effective URL could be different than remoteURL and it is updated only for CURLE_OK case
						totalPerformRequest,
						total, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize, reqSize, http_code, simType );
				}
				
				if(!loopAgain)
					break;
			}
		}

		if (http_code == 200 || http_code == 206 || http_code == CURLE_OPERATION_TIMEDOUT)
		{
			if (http_code == CURLE_OPERATION_TIMEDOUT && buffer->len > 0)
			{
				logprintf("Download timedout and obtained a partial buffer of size %d for a downloadTime=%lld and isDownloadStalled:%d", buffer->len, downloadTimeMS, isDownloadStalled);
			}

			if (downloadTimeMS > 0 && fileType == eMEDIATYPE_VIDEO && gpGlobalConfig->bEnableABR)
			{
				{
					pthread_mutex_lock(&mLock);
					long downloadbps = ((long)(buffer->len / downloadTimeMS)*8000);
					long currentProfilebps  = mpStreamAbstractionAAMP->GetVideoBitrate();
					// extra coding to avoid picking lower profile
					AAMPLOG_INFO("%s downloadbps:%ld currentProfilebps:%ld downloadTimeMS:%lld fragmentDurationMs:%lld",__FUNCTION__,downloadbps,currentProfilebps,downloadTimeMS,fragmentDurationMs);
					if(fragmentDurationMs && downloadTimeMS < fragmentDurationMs/2 && downloadbps < currentProfilebps)
					{
						downloadbps = currentProfilebps;
					}
					
					mAbrBitrateData.push_back(std::make_pair(aamp_GetCurrentTimeMS() ,downloadbps));
					//logprintf("CacheSz[%d]ConfigSz[%d] Storing Size [%d] bps[%ld]",mAbrBitrateData.size(),gpGlobalConfig->abrCacheLength, buffer->len, ((long)(buffer->len / downloadTimeMS)*8000));
					if(mAbrBitrateData.size() > gpGlobalConfig->abrCacheLength)
						mAbrBitrateData.erase(mAbrBitrateData.begin());
					pthread_mutex_unlock(&mLock);
				}
			}
		}
		if (http_code == 200 || http_code == 206)
		{
#ifdef SAVE_DOWNLOADS_TO_DISK
			const char *fname = remoteUrl;
			for (;;)
			{
				const char *next = strchr(fname, '/');
				if (next)
				{
					next++;
					fname = next;
				}
				else
				{
					break;
				}
			}
			char path[1024];
			snprintf(path,sizeof(path),"C:/Users/pstrof200/Downloads/%s", fname);
			FILE *f = fopen(path, "wb");
			fwrite(buffer->ptr, 1, buffer->len, f);
			fclose(f);
#endif
			double expectedContentLength = 0;
			if ((!context.downloadIsEncoded) && CURLE_OK==curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &expectedContentLength) && ((int)expectedContentLength>0) && ((int)expectedContentLength != (int)buffer->len))
			{
				//Note: For non-compressed data, Content-Length header and buffer size should be same. For gzipped data, 'Content-Length' will be <= deflated data.
				AAMPLOG_WARN("AAMP Content-Length=%d actual=%d", (int)expectedContentLength, (int)buffer->len);
				http_code       =       416; // Range Not Satisfiable
				ret             =       false; // redundant, but harmless
				if (buffer->ptr)
				{
					aamp_Free(&buffer->ptr);
				}
				memset(buffer, 0x00, sizeof(*buffer));
			}
			else
			{
				if(fileType == eMEDIATYPE_MANIFEST)
				{
					fileType = (MediaType)curlInstance;
				}
				else if (remoteUrl.find("iframe") != std::string::npos)
				{
					fileType = eMEDIATYPE_IFRAME;
				}
				ret = true;
			}
		}
		else
		{
			if (AAMP_IS_LOG_WORTHY_ERROR(res))
			{
				logprintf("BAD URL:%s", remoteUrl.c_str());
			}
			if (buffer->ptr)
			{
				aamp_Free(&buffer->ptr);
			}
			memset(buffer, 0x00, sizeof(*buffer));

			if (rate != 1.0)
			{
				fileType = eMEDIATYPE_IFRAME;
			}

			// dont generate anomaly reports for write and aborted errors
			// these are generated after trick play options,
			if( !(http_code == CURLE_ABORTED_BY_CALLBACK || http_code == CURLE_WRITE_ERROR || http_code == 204))
			{
				SendAnomalyEvent(ANOMALY_WARNING, "%s:%s,%s-%d url:%s", (mTSBEnabled ? "FOG" : "CDN"),
					MediaTypeString(fileType), (http_code < 100) ? "Curl" : "HTTP", http_code, remoteUrl.c_str());
			}
            
			if ( (httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_XREASON) && (httpRespHeaders[curlInstance].data.length() > 0) )
			{
				logprintf("Received X-Reason header from %s: '%s'", mTSBEnabled?"Fog":"CDN Server", httpRespHeaders[curlInstance].data.c_str());
				SendAnomalyEvent(ANOMALY_WARNING, "%s X-Reason:%s", mTSBEnabled ? "Fog" : "CDN", httpRespHeaders[curlInstance].data.c_str());
			}
			else if ( (httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_FOG_REASON) && (httpRespHeaders[curlInstance].data.length() > 0) )
			{
				//extract error and url used by fog to download content from cdn
				// it is part of fog-reason
				if(fogError)
				{
					std::regex errRegx("-(.*),");
					std::smatch match;
					if (std::regex_search(httpRespHeaders[curlInstance].data, match, errRegx) && match.size() > 1) {
						if (!match.str(1).empty())
						{
							*fogError = std::stoi(match.str(1));
							AAMPLOG_INFO("Received FOG-Reason fogError: '%d'", *fogError);
						}
					}
				}

				//	get failed url from fog reason and update effectiveUrl
				if(!effectiveUrl.empty())
				{
					std::regex fromRegx("from:(.*),");
					std::smatch match;

					if (std::regex_search(httpRespHeaders[curlInstance].data, match, fromRegx) && match.size() > 1) {
						if (!match.str(1).empty())
						{
							effectiveUrl.assign(match.str(1).c_str());
							AAMPLOG_INFO("Received FOG-Reason effectiveUrl: '%s'", effectiveUrl.c_str());
						}
					}
				}


                logprintf("Received FOG-Reason header: '%s'", httpRespHeaders[curlInstance].data.c_str());
                SendAnomalyEvent(ANOMALY_WARNING, "FOG-Reason:%s", httpRespHeaders[curlInstance].data.c_str());
            }
		}

		if (bitrate && (context.bitrate > 0))
		{
			AAMPLOG_INFO("Received getfile Bitrate : %ld", context.bitrate);
			*bitrate = context.bitrate;
		}

		if(simType == eMEDIATYPE_PLAYLIST_VIDEO || simType == eMEDIATYPE_PLAYLIST_AUDIO || simType == eMEDIATYPE_INIT_AUDIO || simType == eMEDIATYPE_INIT_VIDEO )
		{
			// send customized error code for curl 28 and 18 playlist/init fragment download failures
			if (connectTime == 0.0)
			{
				//curl connection is failure
				if(CURLE_PARTIAL_FILE == http_code)
				{
					http_code = PARTIAL_FILE_CONNECTIVITY_AAMP;
				}
				else if(CURLE_OPERATION_TIMEDOUT == http_code)
				{
					http_code = OPERATION_TIMEOUT_CONNECTIVITY_AAMP;
				}
			}
			else if(abortReason != eCURL_ABORT_REASON_NONE)
			{
				http_code = PARTIAL_FILE_START_STALL_TIMEOUT_AAMP;
			}
			else if (CURLE_PARTIAL_FILE == http_code)
			{
				// download time expired with partial file for playlists/init fragments
				http_code = PARTIAL_FILE_DOWNLOAD_TIME_EXPIRED_AAMP;
			}
		}
		pthread_mutex_lock(&mLock);
	}
	else
	{
		logprintf("downloads disabled");
	}
	pthread_mutex_unlock(&mLock);
	if (http_error)
	{
		*http_error = http_code;
	}
	if (httpHeaders != NULL)
	{
		curl_slist_free_all(httpHeaders);
	}
	if (mIsFirstRequestToFOG)
	{
		mIsFirstRequestToFOG = false;
	}

    if (gpGlobalConfig->useLinearSimulator)
	{
		// NEW! note that for simulated playlists, ideally we'd not bother re-downloading playlist above

		AAMPLOG_INFO("*** Simulated Linear URL: %s\n", remoteUrl.c_str()); // Log incoming request

		switch( simType )
		{
			case eMEDIATYPE_MANIFEST:
			{
				// Reset state after requesting main manifest
				if( full_playlist_video_ptr )
				{
					// Flush old cached video playlist

					free( full_playlist_video_ptr );
					full_playlist_video_ptr = NULL;
					full_playlist_video_len = 0;
				}

				if( full_playlist_audio_ptr )
				{
					// Flush old cached audio playlist

					free( full_playlist_audio_ptr );
					full_playlist_audio_ptr = NULL;
					full_playlist_audio_len = 0;
				}

				simulation_start = aamp_GetCurrentTimeMS();
			}
				break; /* eMEDIATYPE_MANIFEST */

			case eMEDIATYPE_PLAYLIST_AUDIO:
			{
				if( !full_playlist_audio_ptr )
				{
					// Cache the full vod audio playlist

					full_playlist_audio_len = buffer->len;
					full_playlist_audio_ptr = (char *)malloc(buffer->len);
					memcpy( full_playlist_audio_ptr, buffer->ptr, buffer->len );
				}

				SimulateLinearWindow( buffer, full_playlist_audio_ptr, full_playlist_audio_len );
			}
				break; /* eMEDIATYPE_PLAYLIST_AUDIO */

			case eMEDIATYPE_PLAYLIST_VIDEO:
			{
				if( !full_playlist_video_ptr )
				{
					// Cache the full vod video playlist

					full_playlist_video_len = buffer->len;
					full_playlist_video_ptr = (char *)malloc(buffer->len);
					memcpy( full_playlist_video_ptr, buffer->ptr, buffer->len );
				}

				SimulateLinearWindow( buffer, full_playlist_video_ptr, full_playlist_video_len );
			}
				break; /* eMEDIATYPE_PLAYLIST_VIDEO */

			default:
				break;
		}
	}

	return ret;
}

/**
 * @brief Append nul character to buffer
 * @param buffer buffer in which nul to be append
 */
void aamp_AppendNulTerminator(struct GrowableBuffer *buffer)
{
	char zeros[2] = { 0, 0 }; // append two bytes, to distinguish between internal inserted 0x00's and a final 0x00 0x00
	aamp_AppendBytes(buffer, zeros, 2);
}



#if 0
/**
* @brief return pointer to start and end of substring, if found
*/
static const char *FindUriAttr(const char *uri, const char *attrName, const char **pfin)
{
	const char *attrValue = NULL;
	const char *qsStart = strchr(uri, '?');
	if (qsStart)
	{
		attrValue = strstr(qsStart, attrName);
		if (attrValue)
		{
			attrValue += strlen(attrName);
			assert(*attrValue == '=');
			attrValue++;
			const char *fin = strchr(attrValue, '&');
			if (!fin)
			{
				fin = attrValue + strlen(attrValue);
			}
			*pfin = fin;
		}
	}
	return attrValue;
}


/**
 * @brief
 * @param ptr
 * @retval
 */
static int ReadDecimalFourDigit(const char *ptr)
{
	return
		(ptr[0] - '0') * 1000 +
		(ptr[1] - '0') * 100 +
		(ptr[2] - '0') * 10 +
		(ptr[3] - '0');
}

/**
 * @brief
 * @param ptr
 * @retval
 */
static int ReadDecimalTwoDigit(const char *ptr)
{
	return
		(ptr[0] - '0') * 10 +
		(ptr[1] - '0');
}

/**
 * @struct DateTime
 * @brief
 */
struct DateTime
{
	int YYYY;
	int MM;
	int DD;
	int hh;
	int mm;
	float ss;
	int Z;
};

/**
 * @brief Parse ISO8601 time
 * @param[out] datetime Parsed output
 * @param[in] ptr ISO8601 C string
 */
static void ParseISO8601(struct DateTime *datetime, const char *ptr)
{
	datetime->YYYY = ReadDecimalFourDigit(ptr);
	ptr += 4;
	if (*ptr == '-') ptr++;

	datetime->MM = ReadDecimalTwoDigit(ptr);
	ptr += 2;
	if (*ptr == '-') ptr++;

	datetime->DD = ReadDecimalTwoDigit(ptr);
	ptr += 2;
	if (*ptr == 'T') ptr++;

	datetime->hh = ReadDecimalTwoDigit(ptr);
	ptr += 2;
	if (*ptr == ':') ptr++;

	datetime->mm = ReadDecimalTwoDigit(ptr);
	ptr += 2;
	if (*ptr == ':') ptr++;

	datetime->ss = (float)ReadDecimalTwoDigit(ptr);
	ptr += 2;
	if (*ptr == '.' || *ptr == ',')
	{
		ptr++;
		int denominator = 1;
		int numerator = 0;
		while (*ptr >= '0' && *ptr <= '9')
		{
			numerator = (numerator * 10) + (*ptr - '0');
			denominator *= 10;
			ptr++;
		}
		datetime->ss += numerator / (float)denominator;
	}

	char zone = *ptr++;
	if (zone != 'Z')
	{
		datetime->Z = 600 * (ptr[0] - '0'); // hour, tens
		datetime->Z += 60 * (ptr[1] - '0'); // hour, ones
		ptr += 2;
		if (*ptr == ':') ptr++;
		if (*ptr >= '0'&& *ptr <= '9')
		{
			datetime->Z += 10 * (ptr[0] - '0'); // seconds, tens
			datetime->Z += (ptr[1] - '0'); // seconds, ones
			ptr += 2;
		}
		if (zone == '-')
		{
			datetime->Z = -datetime->Z;
		}
	}
	else
	{
		datetime->Z = 0;
	}
}
#endif

/**
 * @brief comparing strings
 * @param[in] inputStr - Input string
 * @param[in] prefix - substring to be searched
 * @retval TRUE if substring is found in bigstring
 */
bool aamp_StartsWith( const char *inputStr, const char *prefix )
{
	bool rc = true;
	while( *prefix )
	{
		if( *inputStr++ != *prefix++ )
		{
			rc = false;
			break;
		}
	}
	return rc;
}

/**
 * @brief Resolve URL from base and uri
 * @param[out] dst Destination buffer
 * @param base Base URL
 * @param uri manifest/ fragment uri
 */
void aamp_ResolveURL(std::string& dst, std::string base, const char *uri)
{
	if (memcmp(uri, "http://", 7) != 0 && memcmp(uri, "https://", 8) != 0) // explicit endpoint - needed for DAI playlist
	{
		dst = base;

		std::size_t pos;
		if (uri[0] == '/')
		{	// absolute path; preserve only endpoint http://<endpoint>:<port>/
			//e.g uri = "/vod/video/00000001.ts"
			// base = "https://host.com/folder1/manifest.m3u8"
			// dst = "https://host.com/vod/video/00000001.ts"
			pos = dst.find("://");
			if (pos != std::string::npos)
			{
				pos = dst.find('/', pos + 3);
			}
			pos--; // skip the "/" as uri starts with "/" , this is done to avoid double "//" in URL which sometimes gives HTTP-404
		}
		else
		{	// relative path; include base directory
			// e.g base = "http://127.0.0.1:9080/manifests/video1/manifest.m3u8"
			// uri = "frag-787563519.ts"
			// dst = http://127.0.0.1:9080/manifests/video1/frag-787563519.ts
			pos = dst.rfind('/');
		}

		assert(pos!=std::string::npos);
		dst.replace(pos+1, std::string::npos, uri);

		if (strchr(uri, '?') == 0)//if uri doesn't already have url parameters, then copy from the parents(if they exist)
		{
			pos = base.find('?');
			if (pos != std::string::npos)
			{
				std::string params = base.substr(pos);
				dst.append(params);
			}
		}
	}
	else
		dst = uri; // uri = "http://host.com/video/manifest.m3u8"
}

/**
 * @brief
 * @param url
 * @retval
 */
std::string aamp_getHostFromURL(std::string url)
{
	std::string host = "comcast.net";
	std::string protos = "https";
	std::string proto = "http";
	std::size_t start_pos = std::string::npos;
	if (url.compare(0, protos.length(), protos) == 0)
	{
		start_pos = protos.length() + 3;
	}
	else if (url.compare(0, proto.length(), proto) == 0)
	{
		start_pos = proto.length() + 3;
	}

	if (start_pos != std::string::npos)
	{
		std::size_t pos = url.find('/', start_pos);
		if (pos != std::string::npos)
		host = url.substr(start_pos, (pos - start_pos));
	}
	return host;
}


/**
 * @brief
 * @param mainManifestUrl
 * @retval
 */
static const char *RemapManifestUrl(const char *mainManifestUrl)
{
	if (!mChannelOverrideMap.empty())
	{
		for (std::list<ChannelInfo>::iterator it = mChannelOverrideMap.begin(); it != mChannelOverrideMap.end(); ++it)
		{
			ChannelInfo &pChannelInfo = *it;
			if (strstr(mainManifestUrl, pChannelInfo.name.c_str()))
			{
				logprintf("override!");
				return pChannelInfo.uri.c_str();
			}
		}
	}
	return NULL;
}

#ifdef IARM_MGR

/**
 * @brief
 * @param paramName
 * @param iConfigLen
 * @retval
 */
char *  GetTR181AAMPConfig(const char * paramName, size_t & iConfigLen)
{
    char *  strConfig = NULL;
    IARM_Result_t result; 
    HOSTIF_MsgData_t param;
    memset(&param,0,sizeof(param));
    snprintf(param.paramName,TR69HOSTIFMGR_MAX_PARAM_LEN,"%s",paramName);
    param.reqType = HOSTIF_GET;

    result = IARM_Bus_Call(IARM_BUS_TR69HOSTIFMGR_NAME,IARM_BUS_TR69HOSTIFMGR_API_GetParams,
                    (void *)&param,	sizeof(param));
    if(result  == IARM_RESULT_SUCCESS)
    {
        if(fcNoFault == param.faultCode)
        {
            if(param.paramtype == hostIf_StringType && param.paramLen > 0 )
            {
                std::string strforLog(param.paramValue,param.paramLen);

                iConfigLen = param.paramLen;
                const char *src = (const char*)(param.paramValue);
                strConfig = (char * ) base64_Decode(src,&iConfigLen);
                
                logprintf("GetTR181AAMPConfig: Got:%s En-Len:%d Dec-len:%d",strforLog.c_str(),param.paramLen,iConfigLen);
            }
            else
            {
                logprintf("GetTR181AAMPConfig: Not a string param type=%d or Invalid len:%d ",param.paramtype, param.paramLen);
            }
        }
    }
    else
    {
        logprintf("GetTR181AAMPConfig: Failed to retrieve value result=%d",result);
    }
    return strConfig;
}
#endif


/**
 * @brief trim a string
 * @param[in][out] cmd Buffer containing string
 */
static void trim(std::string& src)
{
	size_t first = src.find_first_not_of(' ');
	if (first != std::string::npos)
	{
		size_t last = src.find_last_not_of(" \r\n");
		std::string dst = src.substr(first, (last - first + 1));
		src = dst;
	}
}

/**
* @brief helper function to avoid dependency on unsafe sscanf while reading strings
* @param bufPtr pointer to CString buffer to scan
* @param prefixPtr - prefix string to match in bufPtr
* @param valueCopyPtr receives allocated copy of string following prefix (skipping delimiting whitesace) if prefix found
* @retval 0 if prefix not present or error
* @retval 1 if string extracted/copied to valueCopyPtr
*/
static int ReadConfigStringHelper(std::string buf, const char *prefixPtr, const char **valueCopyPtr)
{
	int ret = 0;
	if (buf.find(prefixPtr) != std::string::npos)
	{
		std::size_t pos = strlen(prefixPtr);
		if (*valueCopyPtr != NULL)
		{
			free((void *)*valueCopyPtr);
			*valueCopyPtr = NULL;
		}
		*valueCopyPtr = strdup(buf.substr(pos).c_str());
		if (*valueCopyPtr)
		{
			ret = 1;
		}
	}
	return ret;
}

/**
* @brief helper function to extract numeric value from given buf after removing prefix
* @param buf String buffer to scan
* @param prefixPtr - prefix string to match in bufPtr
* @param value - receives numeric value after extraction
* @retval 0 if prefix not present or error
* @retval 1 if string converted to numeric value
*/
template<typename T>
int ReadConfigNumericHelper(std::string buf, const char* prefixPtr, T& value)
{
	int ret=0;
	std::size_t pos = buf.find(prefixPtr);
	if (pos != std::string::npos)
	{
		pos += strlen(prefixPtr);
		std::string valStr = buf.substr(pos);
		if (std::is_same<T, int>::value)
			value = std::stoi(valStr);
		else if (std::is_same<T, long>::value)
			value = std::stol(valStr);
		else if (std::is_same<T, float>::value)
			value = std::stof(valStr);
		else
			value = std::stod(valStr);
		ret = 1;
	}

	return ret;
}

template<typename T>
int ReadConfigNumericHelper(std::string buf, const char* prefixPtr, T& value1, T& value2, T& value3, T& value4)
{
	int ret = false;
	std::size_t pos = buf.find(prefixPtr);
	if (pos != std::string::npos)
	{
		pos += strlen(prefixPtr);
		std::string valStr = buf.substr(pos);
		std::stringstream ss(valStr);
		ss >> value1 >> value2 >> value3 >> value4;
		ret = true;
	}
	return ret;
}

/**
 * @brief Process config entries,i and update gpGlobalConfig params
 *        based on the config setting.
 * @param cfg config to process
 */
 static void ProcessConfigEntry(std::string cfg)
{
	if (!cfg.empty() && cfg.at(0) != '#')
	{ // ignore comments

		//Removing unnecessary spaces and newlines
		trim(cfg);

		double seconds = 0;
		double inputTimeout;
		int value;
		char * tmpValue = NULL;
		if (ReadConfigNumericHelper(cfg, "map-mpd=", gpGlobalConfig->mapMPD) == 1)
		{
			logprintf("map-mpd=%d", gpGlobalConfig->mapMPD);
		}
		else if (ReadConfigNumericHelper(cfg, "fragmp4-license-prefetch=", value) == 1)
		{
			gpGlobalConfig->fragmp4LicensePrefetch = (value != 0);
			logprintf("fragmp4-license-prefetch=%d", gpGlobalConfig->fragmp4LicensePrefetch);
		}
		else if (ReadConfigNumericHelper(cfg, "fog-dash=", value) == 1)
		{
			gpGlobalConfig->fogSupportsDash = (value != 0);
			logprintf("fog-dash=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "enable_videoend_event=", value) == 1)
		{
			gpGlobalConfig->mEnableVideoEndEvent = (value==1);
			logprintf("enable_videoend_event=%d", gpGlobalConfig->mEnableVideoEndEvent);
		}
		else if (ReadConfigNumericHelper(cfg, "fog=", value) == 1)
		{
			gpGlobalConfig->noFog = (value==0);
			logprintf("fog=%d", value);
		}
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		else if (ReadConfigNumericHelper(cfg, "harvest=", gpGlobalConfig->harvest) == 1)
		{
			logprintf("harvest=%d", gpGlobalConfig->harvest);
		}
                else if (ReadConfigStringHelper(cfg, "harvestpath=", (const char**)&gpGlobalConfig->harvestpath))
                {
                        logprintf("harvestpath=%s\n", gpGlobalConfig->harvestpath);
                }
#endif
		else if (ReadConfigNumericHelper(cfg, "forceEC3=", gpGlobalConfig->forceEC3) == 1)
		{
			logprintf("forceEC3=%d", gpGlobalConfig->forceEC3);
		}
		else if (ReadConfigNumericHelper(cfg, "disableEC3=", gpGlobalConfig->disableEC3) == 1)
		{
			logprintf("disableEC3=%d", gpGlobalConfig->disableEC3);
		}
		else if (ReadConfigNumericHelper(cfg, "disableATMOS=", gpGlobalConfig->disableATMOS) == 1)
		{
			logprintf("disableATMOS=%d", gpGlobalConfig->disableATMOS);
		}
		else if (ReadConfigNumericHelper(cfg, "cdvrlive-offset=", gpGlobalConfig->cdvrliveOffset) == 1)
		{
                        VALIDATE_INT("cdvrlive-offset", gpGlobalConfig->cdvrliveOffset, AAMP_CDVR_LIVE_OFFSET)
			logprintf("cdvrlive-offset=%d", gpGlobalConfig->cdvrliveOffset);
		}
		else if (ReadConfigNumericHelper(cfg, "live-offset=", gpGlobalConfig->liveOffset) == 1)
		{
                        VALIDATE_INT("live-offset", gpGlobalConfig->liveOffset, AAMP_LIVE_OFFSET)
			logprintf("live-offset=%d", gpGlobalConfig->liveOffset);
		}
		else if (ReadConfigNumericHelper(cfg, "disablePlaylistIndexEvent=", gpGlobalConfig->disablePlaylistIndexEvent) == 1)
		{
			logprintf("disablePlaylistIndexEvent=%d", gpGlobalConfig->disablePlaylistIndexEvent);
		}
		else if (cfg.compare("enableSubscribedTags") == 0)
		{
			gpGlobalConfig->enableSubscribedTags = true;
			logprintf("enableSubscribedTags set");
		}
		else if (cfg.compare("disableSubscribedTags") == 0)
		{
			gpGlobalConfig->enableSubscribedTags = false;
			logprintf("disableSubscribedTags set");
		}
		else if (ReadConfigNumericHelper(cfg, "enableSubscribedTags=", gpGlobalConfig->enableSubscribedTags) == 1)
		{
			logprintf("enableSubscribedTags=%d", gpGlobalConfig->enableSubscribedTags);
		}
		else if (ReadConfigNumericHelper(cfg, "networkTimeout=", inputTimeout) == 1)
		{
			VALIDATE_DOUBLE("networkTimeout", inputTimeout, CURL_FRAGMENT_DL_TIMEOUT)
			gpGlobalConfig->networkTimeoutMs = (long)CONVERT_SEC_TO_MS(inputTimeout);
			logprintf("networkTimeout=%ld", gpGlobalConfig->networkTimeoutMs);
		}
		else if (ReadConfigNumericHelper(cfg, "manifestTimeout=", inputTimeout) == 1)
		{
			VALIDATE_DOUBLE("manifestTimeout", inputTimeout, CURL_FRAGMENT_DL_TIMEOUT)
			gpGlobalConfig->manifestTimeoutMs = (long)CONVERT_SEC_TO_MS(inputTimeout);
			logprintf("manifestTimeout=%ld ms", gpGlobalConfig->manifestTimeoutMs);
		}
		else if (ReadConfigNumericHelper(cfg, "playlistTimeout=", inputTimeout) == 1)
		{
			VALIDATE_DOUBLE("playlist", inputTimeout, CURL_FRAGMENT_DL_TIMEOUT)
			gpGlobalConfig->playlistTimeoutMs = (long)CONVERT_SEC_TO_MS(inputTimeout);
			logprintf("playlistTimeout=%ld ms", gpGlobalConfig->playlistTimeoutMs);
		}
		else if (cfg.compare("dash-ignore-base-url-if-slash") == 0)
		{
			gpGlobalConfig->dashIgnoreBaseURLIfSlash = true;
			logprintf("dash-ignore-base-url-if-slash set");
		}
		else if (cfg.compare("license-anonymous-request") == 0)
		{
			gpGlobalConfig->licenseAnonymousRequest = true;
			logprintf("license-anonymous-request set");
		}
		else if (cfg.compare("useLinearSimulator") == 0)
		{
			gpGlobalConfig->useLinearSimulator = true;
			logprintf("useLinearSimulator set");
		}
		else if ((cfg.compare("info") == 0) && (!gpGlobalConfig->logging.debug))
		{
			gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_INFO);
			gpGlobalConfig->logging.info = true;
			logprintf("info logging %s", gpGlobalConfig->logging.info ? "on" : "off");
		}
		else if (cfg.compare("failover") == 0)
		{
			gpGlobalConfig->logging.failover = true;
			logprintf("failover logging %s", gpGlobalConfig->logging.failover ? "on" : "off");
		}
		else if (cfg.compare("curlHeader") == 0)
		{
			gpGlobalConfig->logging.curlHeader = true;
			logprintf("curlHeader logging %s", gpGlobalConfig->logging.curlHeader ? "on" : "off");
		}
		else if(cfg.compare("logMetadata") == 0)
		{
			gpGlobalConfig->logging.logMetadata = true;
			logprintf("logMetadata logging %s", gpGlobalConfig->logging.logMetadata ? "on" : "off");
		}
		else if (ReadConfigStringHelper(cfg, "customHeader=", (const char**)&tmpValue))
		{
			if (tmpValue)
			{
				gpGlobalConfig->customHeaderStr.push_back(tmpValue);
				logprintf("customHeader = %s", tmpValue);

				free(tmpValue);
				tmpValue = NULL;
			}
		}
		else if (ReadConfigStringHelper(cfg, "uriParameter=", (const char**)&gpGlobalConfig->uriParameter))
		{
			logprintf("uriParameter = %s", gpGlobalConfig->uriParameter);
		}
		else if (cfg.compare("gst") == 0)
		{
			gpGlobalConfig->logging.gst = !gpGlobalConfig->logging.gst;
			logprintf("gst logging %s", gpGlobalConfig->logging.gst ? "on" : "off");
		}
		else if (cfg.compare("progress") == 0)
		{
			gpGlobalConfig->logging.progress = !gpGlobalConfig->logging.progress;
			logprintf("progress logging %s", gpGlobalConfig->logging.progress ? "on" : "off");
		}
		else if (cfg.compare("debug") == 0)
		{
			gpGlobalConfig->logging.info = false;
			gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_TRACE);
			gpGlobalConfig->logging.debug = true;
			logprintf("debug logging %s", gpGlobalConfig->logging.debug ? "on" : "off");
		}
		else if (cfg.compare("trace") == 0)
		{
			gpGlobalConfig->logging.trace = !gpGlobalConfig->logging.trace;
			logprintf("trace logging %s", gpGlobalConfig->logging.trace ? "on" : "off");
		}
		else if (cfg.compare("curl") == 0)
		{
			gpGlobalConfig->logging.curl = !gpGlobalConfig->logging.curl;
			logprintf("curl logging %s", gpGlobalConfig->logging.curl ? "on" : "off");
		}
		else if (ReadConfigNumericHelper(cfg, "default-bitrate=", gpGlobalConfig->defaultBitrate) == 1)
		{
			VALIDATE_LONG("default-bitrate",gpGlobalConfig->defaultBitrate, DEFAULT_INIT_BITRATE)
			logprintf("aamp default-bitrate: %ld", gpGlobalConfig->defaultBitrate);
		}
		else if (ReadConfigNumericHelper(cfg, "default-bitrate-4k=", gpGlobalConfig->defaultBitrate4K) == 1)
		{
			VALIDATE_LONG("default-bitrate-4k", gpGlobalConfig->defaultBitrate4K, DEFAULT_INIT_BITRATE_4K)
			logprintf("aamp default-bitrate-4k: %ld", gpGlobalConfig->defaultBitrate4K);
		}
		else if (cfg.compare("abr") == 0)
		{
			gpGlobalConfig->bEnableABR = !gpGlobalConfig->bEnableABR;
			logprintf("abr %s", gpGlobalConfig->bEnableABR ? "on" : "off");
		}
		else if (ReadConfigNumericHelper(cfg, "abr-cache-life=", gpGlobalConfig->abrCacheLife) == 1)
		{
			gpGlobalConfig->abrCacheLife *= 1000;
			logprintf("aamp abr cache lifetime: %ldmsec", gpGlobalConfig->abrCacheLife);
		}
		else if (ReadConfigNumericHelper(cfg, "abr-cache-length=", gpGlobalConfig->abrCacheLength) == 1)
		{
			VALIDATE_INT("abr-cache-length", gpGlobalConfig->abrCacheLength, DEFAULT_ABR_CACHE_LENGTH)
			logprintf("aamp abr cache length: %ld", gpGlobalConfig->abrCacheLength);
		}
		else if (ReadConfigNumericHelper(cfg, "useNewABR=", value) == 1)
		{
			gpGlobalConfig->abrBufferCheckEnabled  = (TriState)(value != 0);
			logprintf("useNewABR =%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "useNewAdBreaker=", value) == 1)
		{
			gpGlobalConfig->useNewDiscontinuity  = (TriState)(value != 0);
			logprintf("useNewAdBreaker =%d", value);
		}
		else if (cfg.compare("reportvideopts") == 0)
		{
			gpGlobalConfig->bReportVideoPTS = true;
			logprintf("reportvideopts:%s", gpGlobalConfig->bReportVideoPTS ? "on" : "off");
		}
		else if (cfg.compare("decoderunavailablestrict") == 0)
		{
			gpGlobalConfig->decoderUnavailableStrict = true;
			logprintf("decoderunavailablestrict:%s", gpGlobalConfig->decoderUnavailableStrict ? "on" : "off");
		}
		else if( cfg.compare("descriptiveaudiotrack") == 0 )
		{
			gpGlobalConfig->bDescriptiveAudioTrack  = true;
			logprintf("descriptiveaudiotrack:%s", gpGlobalConfig->bDescriptiveAudioTrack ? "on" : "off");
		}
		else if( ReadConfigNumericHelper( cfg, "langcodepref=", value) == 1 )
		{
			const char *langCodePrefName[] =
			{
				"ISO639_NO_LANGCODE_PREFERENCE",
				"ISO639_PREFER_3_CHAR_BIBLIOGRAPHIC_LANGCODE",
				"ISO639_PREFER_3_CHAR_TERMINOLOGY_LANGCODE",
				"ISO639_PREFER_2_CHAR_LANGCODE"
			};
			if( value>=0 && value<4 )
			{
				gpGlobalConfig->langCodePreference = (LangCodePreference)value;
				logprintf("langcodepref:%s\n", langCodePrefName[gpGlobalConfig->langCodePreference] );
			}
		}
		else if (cfg.compare("appSrcForProgressivePlayback") == 0)
		{
			gpGlobalConfig->useAppSrcForProgressivePlayback = true;
			logprintf("appSrcForProgressivePlayback:%s\n", gpGlobalConfig->useAppSrcForProgressivePlayback ? "on" : "off");
		}
		else if (ReadConfigNumericHelper(cfg, "abr-cache-outlier=", gpGlobalConfig->abrOutlierDiffBytes) == 1)
		{
			VALIDATE_INT("abr-cache-outlier", gpGlobalConfig->abrOutlierDiffBytes, DEFAULT_ABR_OUTLIER)
			logprintf("aamp abr outlier in bytes: %ld", gpGlobalConfig->abrOutlierDiffBytes);
		}
		else if (ReadConfigNumericHelper(cfg, "abr-skip-duration=", gpGlobalConfig->abrSkipDuration) == 1)
		{
			VALIDATE_INT("abr-skip-duration",gpGlobalConfig->abrSkipDuration, DEFAULT_ABR_SKIP_DURATION)
			logprintf("aamp abr skip duration: %d", gpGlobalConfig->abrSkipDuration);
		}
		else if (ReadConfigNumericHelper(cfg, "abr-nw-consistency=", gpGlobalConfig->abrNwConsistency) == 1)
		{
			VALIDATE_INT("abr-nw-consistency", gpGlobalConfig->abrNwConsistency, DEFAULT_ABR_NW_CONSISTENCY_CNT)
			logprintf("aamp abr NetworkConsistencyCnt: %d", gpGlobalConfig->abrNwConsistency);
		}
		else if (ReadConfigNumericHelper(cfg, "min-buffer-rampdown=", gpGlobalConfig->minABRBufferForRampDown) == 1)
		{
			VALIDATE_INT("min-buffer-rampdown", gpGlobalConfig->minABRBufferForRampDown, AAMP_LOW_BUFFER_BEFORE_RAMPDOWN)
			logprintf("aamp abr low buffer for rampdown: %d", gpGlobalConfig->minABRBufferForRampDown);
		}
		else if (ReadConfigNumericHelper(cfg, "max-buffer-rampup=", gpGlobalConfig->maxABRBufferForRampUp) == 1)
		{
			logprintf("aamp abr high buffer for rampup: %d", gpGlobalConfig->maxABRBufferForRampUp);
		}
		else if (ReadConfigNumericHelper(cfg, "flush=", gpGlobalConfig->gPreservePipeline) == 1)
		{
			logprintf("aamp flush=%d", gpGlobalConfig->gPreservePipeline);
		}
		else if (ReadConfigNumericHelper(cfg, "demux-hls-audio-track=", gpGlobalConfig->gAampDemuxHLSAudioTsTrack) == 1)
		{ // default 1, set to 0 for hw demux audio ts track
			logprintf("demux-hls-audio-track=%d", gpGlobalConfig->gAampDemuxHLSAudioTsTrack);
		}
		else if (ReadConfigNumericHelper(cfg, "demux-hls-video-track=", gpGlobalConfig->gAampDemuxHLSVideoTsTrack) == 1)
		{ // default 1, set to 0 for hw demux video ts track
			logprintf("demux-hls-video-track=%d", gpGlobalConfig->gAampDemuxHLSVideoTsTrack);
		}
		else if (ReadConfigNumericHelper(cfg, "demux-hls-video-track-tm=", gpGlobalConfig->demuxHLSVideoTsTrackTM) == 1)
		{ // default 0, set to 1 to demux video ts track during trickmodes
			logprintf("demux-hls-video-track-tm=%d", gpGlobalConfig->demuxHLSVideoTsTrackTM);
		}
		else if (ReadConfigNumericHelper(cfg, "demuxed-audio-before-video=", gpGlobalConfig->demuxedAudioBeforeVideo) == 1)
		{ // default 0, set to 1 to send audio es before video in case of s/w demux.
			logprintf("demuxed-audio-before-video=%d", gpGlobalConfig->demuxedAudioBeforeVideo);
		}
		else if (ReadConfigNumericHelper(cfg, "throttle=", gpGlobalConfig->gThrottle) == 1)
		{ // default is true; used with restamping?
			logprintf("aamp throttle=%d", gpGlobalConfig->gThrottle);
		}
		else if (ReadConfigNumericHelper(cfg, "min-vod-cache=", gpGlobalConfig->minVODCacheSeconds) == 1)
		{ // override for VOD cache
			VALIDATE_INT("min-vod-cache", gpGlobalConfig->minVODCacheSeconds, DEFAULT_MINIMUM_CACHE_VOD_SECONDS)
			logprintf("min-vod-cache=%d", gpGlobalConfig->minVODCacheSeconds);
		}
		else if (ReadConfigNumericHelper(cfg, "buffer-health-monitor-delay=", gpGlobalConfig->bufferHealthMonitorDelay) == 1)
		{ // override for buffer health monitor delay after tune/ seek
			VALIDATE_INT("buffer-health-monitor-delay", gpGlobalConfig->bufferHealthMonitorDelay, DEFAULT_BUFFER_HEALTH_MONITOR_DELAY)
			logprintf("buffer-health-monitor-delay=%d", gpGlobalConfig->bufferHealthMonitorDelay);
		}
		else if (ReadConfigNumericHelper(cfg, "buffer-health-monitor-interval=", gpGlobalConfig->bufferHealthMonitorInterval) == 1)
		{ // override for buffer health monitor interval
			VALIDATE_INT("buffer-health-monitor-interval", gpGlobalConfig->bufferHealthMonitorInterval, DEFAULT_BUFFER_HEALTH_MONITOR_INTERVAL)
			logprintf("buffer-health-monitor-interval=%d", gpGlobalConfig->bufferHealthMonitorInterval);
		}
		else if (ReadConfigNumericHelper(cfg, "preferred-drm=", value) == 1)
		{ // override for preferred drm value
			if(value <= eDRM_NONE || value > eDRM_PlayReady)
			{
				logprintf("preferred-drm=%d is unsupported", value);
			}
			else
			{
				gpGlobalConfig->isUsingLocalConfigForPreferredDRM = true;
				gpGlobalConfig->preferredDrm = (DRMSystems) value;
			}
			logprintf("preferred-drm=%s", GetDrmSystemName(gpGlobalConfig->preferredDrm));
		}
		else if (ReadConfigNumericHelper(cfg, "playready-output-protection=", value) == 1)
		{
			gpGlobalConfig->enablePROutputProtection = (value != 0);
			logprintf("playready-output-protection is %s", (value ? "on" : "off"));
		}
		else if (ReadConfigNumericHelper(cfg, "live-tune-event=", value) == 1)
                { // default is 0; set 1 for sending tuned for live
                        logprintf("live-tune-event = %d", value);
                        if (value >= 0 && value < eTUNED_EVENT_MAX)
                        {
                                gpGlobalConfig->tunedEventConfigLive = (TunedEventConfig)(value);
                        }
                }
                else if (ReadConfigNumericHelper(cfg, "vod-tune-event=", value) == 1)
                { // default is 0; set 1 for sending tuned event for vod
                        logprintf("vod-tune-event = %d", value);
                        if (value >= 0 && value < eTUNED_EVENT_MAX)
                        {
                                gpGlobalConfig->tunedEventConfigVOD = (TunedEventConfig)(value);
                        }
                }
		else if (ReadConfigNumericHelper(cfg, "playlists-parallel-fetch=", value) == 1)
		{
			gpGlobalConfig->playlistsParallelFetch = (TriState)(value != 0);
			logprintf("playlists-parallel-fetch=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "parallelPlaylistRefresh=", value) == 1)
		{
			gpGlobalConfig->parallelPlaylistRefresh  = (TriState)(value != 0);
			logprintf("parallelPlaylistRefresh =%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "bulk-timedmeta-report=", value) == 1)
		{
			gpGlobalConfig->enableBulkTimedMetaReport = (TriState)(value != 0);
			logprintf("bulk-timedmeta-report=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "useRetuneForUnpairedDiscontinuity=", value) == 1)
		{
			gpGlobalConfig->useRetuneForUnpairedDiscontinuity = (TriState)(value != 0);
			logprintf("useRetuneForUnpairedDiscontinuity=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "async-tune=", value) == 1)
		{
			gpGlobalConfig->mAsyncTuneConfig = (TriState)(value != 0);
			logprintf("async-tune=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "useWesterosSink=", value) == 1)
		{
			gpGlobalConfig->mWesterosSinkConfig = (TriState)(value != 0);
			logprintf("useWesterosSink=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "pre-fetch-iframe-playlist=", value) == 1)
		{
			gpGlobalConfig->prefetchIframePlaylist = (value != 0);
			logprintf("pre-fetch-iframe-playlist=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "hls-av-sync-use-start-time=", value) == 1)
		{
			gpGlobalConfig->hlsAVTrackSyncUsingStartTime = (value != 0);
			logprintf("hls-av-sync-use-start-time=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "mpd-discontinuity-handling=", value) == 1)
		{
			gpGlobalConfig->mpdDiscontinuityHandling = (value != 0);
			logprintf("mpd-discontinuity-handling=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "mpd-discontinuity-handling-cdvr=", value) == 1)
		{
			gpGlobalConfig->mpdDiscontinuityHandlingCdvr = (value != 0);
			logprintf("mpd-discontinuity-handling-cdvr=%d", value);
		}
		else if(ReadConfigStringHelper(cfg, "ck-license-server-url=", (const char**)&gpGlobalConfig->ckLicenseServerURL))
		{
			logprintf("Clear Key license-server-url=%s", gpGlobalConfig->ckLicenseServerURL);
		}
		else if(ReadConfigStringHelper(cfg, "license-server-url=", (const char**)&gpGlobalConfig->licenseServerURL))
		{
			gpGlobalConfig->licenseServerLocalOverride = true;
			logprintf("license-server-url=%s", gpGlobalConfig->licenseServerURL);
		}
		else if(ReadConfigNumericHelper(cfg, "vod-trickplay-fps=", gpGlobalConfig->vodTrickplayFPS) == 1)
		{
			VALIDATE_INT("vod-trickplay-fps", gpGlobalConfig->vodTrickplayFPS, TRICKPLAY_NETWORK_PLAYBACK_FPS)
			if(gpGlobalConfig->vodTrickplayFPS != TRICKPLAY_NETWORK_PLAYBACK_FPS)
				gpGlobalConfig->vodTrickplayFPSLocalOverride = true;
			logprintf("vod-trickplay-fps=%d", gpGlobalConfig->vodTrickplayFPS);
		}
		else if(ReadConfigNumericHelper(cfg, "linear-trickplay-fps=", gpGlobalConfig->linearTrickplayFPS) == 1)
		{
			VALIDATE_INT("linear-trickplay-fps", gpGlobalConfig->linearTrickplayFPS, TRICKPLAY_TSB_PLAYBACK_FPS)
			if (gpGlobalConfig->linearTrickplayFPS != TRICKPLAY_TSB_PLAYBACK_FPS)
				gpGlobalConfig->linearTrickplayFPSLocalOverride = true;
			logprintf("linear-trickplay-fps=%d", gpGlobalConfig->linearTrickplayFPS);
		}
		else if (ReadConfigNumericHelper(cfg, "report-progress-interval=", gpGlobalConfig->reportProgressInterval) == 1)
		{
			// Progress report input in milliSec 
			VALIDATE_INT("report-progress-interval", gpGlobalConfig->reportProgressInterval, DEFAULT_REPORT_PROGRESS_INTERVAL)
			logprintf("report-progress-interval=%d", gpGlobalConfig->reportProgressInterval);
		}
		else if (ReadConfigStringHelper(cfg, "http-proxy=", &gpGlobalConfig->httpProxy))
		{
			logprintf("http-proxy=%s", gpGlobalConfig->httpProxy);
		}
		else if (cfg.compare("force-http") == 0)
		{
			gpGlobalConfig->bForceHttp = !gpGlobalConfig->bForceHttp;
			logprintf("force-http: %s", gpGlobalConfig->bForceHttp ? "on" : "off");
		}
		else if (ReadConfigNumericHelper(cfg, "internal-retune=", value) == 1)
		{
			gpGlobalConfig->internalReTune = (value != 0);
			logprintf("internal-retune=%d", (int)value);
		}
		else if (ReadConfigNumericHelper(cfg, "gst-buffering-before-play=", value) == 1)
		{
			gpGlobalConfig->gstreamerBufferingBeforePlay = (value != 0);
			logprintf("gst-buffering-before-play=%d", (int)gpGlobalConfig->gstreamerBufferingBeforePlay);
		}
		else if (ReadConfigNumericHelper(cfg, "re-tune-on-buffering-timeout=", value) == 1)
		{
			gpGlobalConfig->reTuneOnBufferingTimeout = (value != 0);
			logprintf("re-tune-on-buffering-timeout=%d", (int)gpGlobalConfig->reTuneOnBufferingTimeout);
		}
		else if (ReadConfigNumericHelper(cfg, "iframe-default-bitrate=", gpGlobalConfig->iframeBitrate) == 1)
		{
			VALIDATE_LONG("iframe-default-bitrate",gpGlobalConfig->iframeBitrate, 0)
			logprintf("aamp iframe-default-bitrate: %ld", gpGlobalConfig->iframeBitrate);
		}
		else if (ReadConfigNumericHelper(cfg, "iframe-default-bitrate-4k=", gpGlobalConfig->iframeBitrate4K) == 1)
		{
			VALIDATE_LONG("iframe-default-bitrate-4k",gpGlobalConfig->iframeBitrate4K, 0)
			logprintf("aamp iframe-default-bitrate-4k: %ld", gpGlobalConfig->iframeBitrate4K);
		}
		else if (cfg.compare("aamp-audio-only-playback") == 0)
		{
			gpGlobalConfig->bAudioOnlyPlayback = true;
			logprintf("aamp-audio-only-playback is %s", gpGlobalConfig->bAudioOnlyPlayback ? "enabled" : "disabled");
		}
		else if (ReadConfigNumericHelper(cfg, "license-retry-wait-time=", gpGlobalConfig->licenseRetryWaitTime) == 1)
		{
			logprintf("license-retry-wait-time: %d", gpGlobalConfig->licenseRetryWaitTime);
		}
		else if (ReadConfigNumericHelper(cfg, "fragment-cache-length=", gpGlobalConfig->maxCachedFragmentsPerTrack) == 1)
		{
			VALIDATE_INT("fragment-cache-length", gpGlobalConfig->maxCachedFragmentsPerTrack, DEFAULT_CACHED_FRAGMENTS_PER_TRACK)
			logprintf("aamp fragment cache length: %d", gpGlobalConfig->maxCachedFragmentsPerTrack);
		}
		else if (ReadConfigNumericHelper(cfg, "pts-error-threshold=", gpGlobalConfig->ptsErrorThreshold) == 1)
		{
			VALIDATE_INT("pts-error-threshold", gpGlobalConfig->ptsErrorThreshold, MAX_PTS_ERRORS_THRESHOLD)
			logprintf("aamp pts-error-threshold: %d", gpGlobalConfig->ptsErrorThreshold);
		}
		else if (ReadConfigNumericHelper(cfg, "enable_setvideorectangle=", value) == 1)
		{
			gpGlobalConfig->mEnableRectPropertyCfg = (TriState)(value != 0);
			logprintf("AAMP Rectangle property for sink element: %d\n", value);
		}
		else if(ReadConfigNumericHelper(cfg, "max-playlist-cache=", gpGlobalConfig->gMaxPlaylistCacheSize) == 1)
		{
			// Read value in KB , convert it to bytes
			gpGlobalConfig->gMaxPlaylistCacheSize = gpGlobalConfig->gMaxPlaylistCacheSize * 1024;
			VALIDATE_INT("max-playlist-cache", gpGlobalConfig->gMaxPlaylistCacheSize, MAX_PLAYLIST_CACHE_SIZE);
			logprintf("aamp max-playlist-cache: %ld", gpGlobalConfig->gMaxPlaylistCacheSize);
		}
		else if(sscanf(cfg.c_str(), "dash-max-drm-sessions=%d", &gpGlobalConfig->dash_MaxDRMSessions) == 1)
		{
			// Read value in KB , convert it to bytes
			if(gpGlobalConfig->dash_MaxDRMSessions < MIN_DASH_DRM_SESSIONS || gpGlobalConfig->dash_MaxDRMSessions > MAX_DASH_DRM_SESSIONS)
			{
				logprintf("Out of range value for dash-max-drm-sessions, setting to %d;Expected Range (%d - %d)",
						MIN_DASH_DRM_SESSIONS, MIN_DASH_DRM_SESSIONS, MAX_DASH_DRM_SESSIONS);
				gpGlobalConfig->dash_MaxDRMSessions = MIN_DASH_DRM_SESSIONS;
			}
			logprintf("aamp dash-max-drm-sessions: %d", gpGlobalConfig->dash_MaxDRMSessions);
		}
		else if (ReadConfigStringHelper(cfg, "user-agent=", (const char**)&tmpValue))
		{
			if(tmpValue)
			{
				if(strlen(tmpValue) < AAMP_USER_AGENT_MAX_CONFIG_LEN)
				{
					logprintf("user-agent=%s", tmpValue);
					gpGlobalConfig->aamp_SetBaseUserAgentString(tmpValue);
				}
				else
				{
					logprintf("user-agent len is more than %d , Hence Ignoring ", AAMP_USER_AGENT_MAX_CONFIG_LEN);
				}
 
				free(tmpValue);
				tmpValue = NULL;
			}
		}
		else if (ReadConfigNumericHelper(cfg, "wait-time-before-retry-http-5xx-ms=", gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS) == 1)
		{
			VALIDATE_INT("wait-time-before-retry-http-5xx-ms", gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS, DEFAULT_WAIT_TIME_BEFORE_RETRY_HTTP_5XX_MS);
			logprintf("aamp wait-time-before-retry-http-5xx-ms: %d", gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS);
		}
		else if (ReadConfigNumericHelper(cfg, "preplaybuffercount=", gpGlobalConfig->preplaybuffercount) == 1)
		{
			VALIDATE_INT("preplaybuffercount", gpGlobalConfig->preplaybuffercount, 10);
			logprintf("preplaybuffercount : %d ",gpGlobalConfig->preplaybuffercount);
		}
		else if (ReadConfigNumericHelper(cfg, "sslverifypeer=", value) == 1)
		{
			gpGlobalConfig->disableSslVerifyPeer = (value != 1);
			logprintf("ssl verify peer is %s", gpGlobalConfig->disableSslVerifyPeer? "disabled" : "enabled");
		}
		else if (ReadConfigNumericHelper(cfg, "curl-stall-timeout=", gpGlobalConfig->curlStallTimeout) == 1)
		{
			//Not calling VALIDATE_LONG since zero is supported
			logprintf("aamp curl-stall-timeout: %ld", gpGlobalConfig->curlStallTimeout);
		}
		else if (ReadConfigNumericHelper(cfg, "curl-download-start-timeout=", gpGlobalConfig->curlDownloadStartTimeout) == 1)
		{
			//Not calling VALIDATE_LONG since zero is supported
			logprintf("aamp curl-download-start-timeout: %ld", gpGlobalConfig->curlDownloadStartTimeout);
		}
		else if (ReadConfigNumericHelper(cfg, "discontinuity-timeout=", gpGlobalConfig->discontinuityTimeout) == 1)
		{
			//Not calling VALIDATE_LONG since zero is supported
			logprintf("aamp discontinuity-timeout: %ld", gpGlobalConfig->discontinuityTimeout);
		}
		else if (ReadConfigNumericHelper(cfg, "client-dai=", value) == 1)
		{
			gpGlobalConfig->enableClientDai = (value == 1);
			logprintf("Client side DAI: %s", gpGlobalConfig->enableClientDai ? "ON" : "OFF");
		}
		else if (ReadConfigNumericHelper(cfg, "ad-from-cdn-only=", value) == 1)
		{
			gpGlobalConfig->playAdFromCDN = (value == 1);
			logprintf("Ad playback from CDN only: %s", gpGlobalConfig->playAdFromCDN ? "ON" : "OFF");
		}
		else if (ReadConfigNumericHelper(cfg, "aamp-abr-threshold-size=", gpGlobalConfig->aampAbrThresholdSize) == 1)
                {
                        VALIDATE_INT("aamp-abr-threshold-size", gpGlobalConfig->aampAbrThresholdSize, DEFAULT_AAMP_ABR_THRESHOLD_SIZE);
                        logprintf("aamp aamp-abr-threshold-size: %d\n", gpGlobalConfig->aampAbrThresholdSize);
                }

		else if(ReadConfigStringHelper(cfg, "subtitle-language=", (const char**)&tmpValue))
		{
			if(tmpValue)
			{
				if(strlen(tmpValue) < MAX_LANGUAGE_TAG_LENGTH)
				{
					logprintf("subtitle-language=%s", tmpValue);
					gpGlobalConfig->mSubtitleLanguage = std::string(tmpValue);
				}
				else
				{
					logprintf("subtitle-language len is more than %d , Hence Ignoring ", MAX_LANGUAGE_TAG_LENGTH);
				}
				free(tmpValue);
				tmpValue = NULL;
			}
		}
		else if (ReadConfigNumericHelper(cfg, "reportbufferevent=", value) == 1)
		{
			gpGlobalConfig->reportBufferEvent = (value != 0);
			logprintf("reportbufferevent=%d", (int)gpGlobalConfig->reportBufferEvent);
		}
		else if (ReadConfigNumericHelper(cfg, "enable-tune-profiling=", value) == 1)
		{
			gpGlobalConfig->enableMicroEvents = (value!=0);
			logprintf( "enable-tune-profiling=%d", value );
		}
		else if (ReadConfigNumericHelper(cfg, "gst-position-query-enable=", value) == 1)
		{
			gpGlobalConfig->bPositionQueryEnabled = (value == 1);
			logprintf("Position query based progress events: %s", gpGlobalConfig->bPositionQueryEnabled ? "ON" : "OFF");
		}
		else if (ReadConfigNumericHelper(cfg, "remove_Persistent=", gpGlobalConfig->aampRemovePersistent) == 1)
		{
			logprintf("remove_Persistent=%d", gpGlobalConfig->aampRemovePersistent);
		}
		else if (ReadConfigNumericHelper(cfg, "avgbwforABR=", value) == 1)
		{
			gpGlobalConfig->mUseAverageBWForABR= (TriState)(value != 0);
			logprintf("avgbwforABR=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "preCachePlaylistTime=", value) == 1)
		{	
			// time window in Minutes			
			gpGlobalConfig->mPreCacheTimeWindow= value;
			logprintf("preCachePlaylistTime=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "fragmentRetryLimit=", value) == 1)
		{
			gpGlobalConfig->rampdownLimit = value;
			logprintf("fragmentRetryLimit=%d", value);
		}
		else if (ReadConfigNumericHelper(cfg, "segmentInjectFailThreshold=", gpGlobalConfig->segInjectFailCount) == 1)
		{
			VALIDATE_INT("segmentInjectFailThreshold", gpGlobalConfig->segInjectFailCount, MAX_SEG_INJECT_FAIL_COUNT);
			logprintf("segmentInjectFailThreshold=%d", gpGlobalConfig->segInjectFailCount);
		}
		else if (ReadConfigNumericHelper(cfg, "drmDecryptFailThreshold=", gpGlobalConfig->drmDecryptFailCount) ==1)
		{
			VALIDATE_INT("drmDecryptFailThreshold", gpGlobalConfig->drmDecryptFailCount, MAX_SEG_DRM_DECRYPT_FAIL_COUNT);
			logprintf("drmDecryptFailThreshold=%d", gpGlobalConfig->drmDecryptFailCount);
		}
		else if (ReadConfigNumericHelper(cfg, "minBitrate=", gpGlobalConfig->minBitrate) ==1)
		{
			VALIDATE_LONG("minBitrate", gpGlobalConfig->minBitrate, 0);
			logprintf("minBitrate=%d", gpGlobalConfig->minBitrate);
		}
		else if (ReadConfigNumericHelper(cfg, "maxBitrate=", gpGlobalConfig->maxBitrate) ==1)
		{
			VALIDATE_LONG("maxBitrate", gpGlobalConfig->maxBitrate, LONG_MAX);
			logprintf("maxBitrate=%d", gpGlobalConfig->maxBitrate);
		}
		else if (ReadConfigNumericHelper(cfg, "initFragmentRetryCount=", gpGlobalConfig->initFragmentRetryCount) == 1)
		{
			logprintf("initFragmentRetryCount=%d", gpGlobalConfig->initFragmentRetryCount);
		}
		else if (cfg.at(0) == '*')
		{
			std::size_t pos = cfg.find_first_of(' ');
			if (pos != std::string::npos)
			{
				//Populate channel map from aamp.cfg
				// new wildcard matching for overrides - allows *HBO to remap any url including "HBO"
				logprintf("aamp override:\n%s\n", cfg.c_str());
				ChannelInfo channelInfo;
				std::stringstream iss(cfg.substr(1));
				std::string token;
				while (getline(iss, token, ' '))
				{
					if (token.compare(0,4,"http") == 0)
						channelInfo.uri = token;
					else
						channelInfo.name = token;
				}
				mChannelOverrideMap.push_back(channelInfo);
			}
		}
	}
}

/**
 * @brief Load AAMP configuration file
 */
void PrivateInstanceAAMP::LazilyLoadConfigIfNeeded(void)
{
	std::string cfgPath = "";
	if (!gpGlobalConfig)
	{
#ifdef AAMP_BUILD_INFO
		std::string tmpstr = MACRO_TO_STRING(AAMP_BUILD_INFO);
		logprintf(" AAMP_BUILD_INFO: %s",tmpstr.c_str());
#endif
		gpGlobalConfig = new GlobalConfigAAMP();
#ifdef IARM_MGR 
        logprintf("LazilyLoadConfigIfNeeded calling  GetTR181AAMPConfig  ");
        size_t iConfigLen = 0;
        char *  cloudConf = GetTR181AAMPConfig("Device.DeviceInfo.X_RDKCENTRAL-COM_RFC.Feature.AAMP_CFG.b64Config", iConfigLen);
        if(cloudConf && (iConfigLen > 0))
        {
            bool bCharCompliant = true;
            for (int i = 0; i < iConfigLen; i++)
            {
                if (!( cloudConf[i] == 0xD || cloudConf[i] == 0xA) && // ignore LF and CR chars
                    ((cloudConf[i] < 0x20) || (cloudConf[i] > 0x7E)))
                {
                    bCharCompliant = false;
                    logprintf("LazilyLoadConfigIfNeeded Non Compliant char[0x%X] found, Ignoring whole config  ",cloudConf[i]);
                    break;
                }
            }

			if (bCharCompliant)
			{
				std::string strCfg(cloudConf,iConfigLen);
				std::istringstream iSteam(strCfg);
				std::string line;
				while (std::getline(iSteam, line)) {
				if (line.length() > 0)
				{
					//ProcessConfigEntry takes char * and line.c_str() returns const string hence copy of line is created
					logprintf("LazilyLoadConfigIfNeeded aamp-cmd:[%s]", line.c_str());
					ProcessConfigEntry(line);
				}
			}
		}
		free(cloudConf); // allocated by base64_Decode in GetTR181AAMPConfig
	}
#endif

#ifdef WIN32
		AampLogManager mLogManager;
		cfgPath.assign(mLogManager.getAampCfgPath());
#elif defined(__APPLE__)
		std::string cfgPath(getenv("HOME"));
		cfgPath += "/aamp.cfg";
#else

#ifdef AAMP_CPC // Comcast builds
        // AAMP_ENABLE_OPT_OVERRIDE is only added for PROD builds.
        const char *env_aamp_enable_opt = getenv("AAMP_ENABLE_OPT_OVERRIDE");
#else
        const char *env_aamp_enable_opt = "true";
#endif

        if(env_aamp_enable_opt)
        {
            cfgPath = "/opt/aamp.cfg";
        }
#endif
		if (!cfgPath.empty())
		{
			std::ifstream f(cfgPath, std::ifstream::in | std::ifstream::binary);
			if (f.good())
			{
				logprintf("opened aamp.cfg");
				std::string buf;
				while (f.good())
				{
					std::getline(f, buf);
					ProcessConfigEntry(buf);
				}
				f.close();
			}
		}

		const char *env_aamp_force_aac = getenv("AAMP_FORCE_AAC");
		if(env_aamp_force_aac)
		{
			logprintf("AAMP_FORCE_AAC present: Changing preference to AAC over ATMOS & DD+");
			gpGlobalConfig->disableEC3 = 1;
			gpGlobalConfig->disableATMOS = 1;
		}

		const char *env_aamp_min_vod_cache = getenv("AAMP_MIN_VOD_CACHE");
		if(env_aamp_min_vod_cache)
		{
			int minVodCache = 0;
			if(sscanf(env_aamp_min_vod_cache,"%d",&minVodCache))
			{
				logprintf("AAMP_MIN_VOD_CACHE present: Changing min vod cache to %d seconds",minVodCache);
				gpGlobalConfig->minVODCacheSeconds = minVodCache;
			}
		}

		const char *env_enable_micro_events = getenv("TUNE_MICRO_EVENTS");
		if(env_enable_micro_events)
		{
			logprintf("TUNE_MICRO_EVENTS present: Enabling TUNE_MICRO_EVENTS.");
			gpGlobalConfig->enableMicroEvents = true;
		}

		const char *env_enable_cdai = getenv("CLIENT_SIDE_DAI");
		if(env_enable_cdai)
		{
			logprintf("CLIENT_SIDE_DAI present: Enabling CLIENT_SIDE_DAI.");
			gpGlobalConfig->enableClientDai = true;
		}

		const char *env_enable_westoros_sink = getenv("AAMP_ENABLE_WESTEROS_SINK");

		if(env_enable_westoros_sink)
		{
			int iValue = atoi(env_enable_westoros_sink);
			bool bValue = (strcasecmp(env_enable_westoros_sink,"true") == 0);

			logprintf("AAMP_ENABLE_WESTEROS_SINK present, Value = %d", (bValue ? bValue : (iValue ? iValue : 0)));

			if(iValue || bValue)
			{
				mWesterosSinkEnabled = true;
			}
		}
	}
}


/**
 * @brief Executes tear down sequence
 * @param newTune true if operation is a new tune
 */
void PrivateInstanceAAMP::TeardownStream(bool newTune)
{
	pthread_mutex_lock(&mLock);
	//Have to perfom this for trick and stop operations but avoid ad insertion related ones
	AAMPLOG_WARN("%s:%d mProgressReportFromProcessDiscontinuity:%d mDiscontinuityTuneOperationId:%d newTune:%d", __FUNCTION__, __LINE__, mProgressReportFromProcessDiscontinuity, mDiscontinuityTuneOperationId, newTune);
	if ((mDiscontinuityTuneOperationId != 0) && (!newTune || mState == eSTATE_IDLE))
	{
		bool waitForDiscontinuityProcessing = true;
		if (mProgressReportFromProcessDiscontinuity)
		{
			AAMPLOG_WARN("%s:%d TeardownStream invoked while mProgressReportFromProcessDiscontinuity and mDiscontinuityTuneOperationId[%d] set!", __FUNCTION__, __LINE__, mDiscontinuityTuneOperationId);
			gint callbackID = 0;
			GSource *source = g_main_current_source();
			if (source != NULL)
			{
				callbackID = g_source_get_id(source);
			}
			if (callbackID != 0 && mDiscontinuityTuneOperationId == callbackID)
			{
				AAMPLOG_WARN("%s:%d TeardownStream idle callback id[%d] and mDiscontinuityTuneOperationId[%d] match. Ignore further discontinuity processing!", __FUNCTION__, __LINE__, callbackID, mDiscontinuityTuneOperationId);
				waitForDiscontinuityProcessing = false; // to avoid deadlock
				mDiscontinuityTuneOperationInProgress = false;
				mDiscontinuityTuneOperationId = 0;
			}
		}
		if (waitForDiscontinuityProcessing)
		{
			//wait for discont tune operation to finish before proceeding with stop
			if (mDiscontinuityTuneOperationInProgress)
			{
				pthread_cond_wait(&mCondDiscontinuity, &mLock);
			}
			else
			{
				g_source_remove(mDiscontinuityTuneOperationId);
				mDiscontinuityTuneOperationId = 0;
			}
		}
	}
	// Maybe mDiscontinuityTuneOperationId is 0, ProcessPendingDiscontinuity can be invoked from NotifyEOSReached too
	else if (mProgressReportFromProcessDiscontinuity)
	{
		AAMPLOG_WARN("%s:%d TeardownStream invoked while mProgressReportFromProcessDiscontinuity set!", __FUNCTION__, __LINE__);
		mDiscontinuityTuneOperationInProgress = false;
	}

	//reset discontinuity related flags
	mProcessingDiscontinuity[eMEDIATYPE_VIDEO] = false;
	mProcessingDiscontinuity[eMEDIATYPE_AUDIO] = false;
	pthread_mutex_unlock(&mLock);

	if (mpStreamAbstractionAAMP)
	{
		mpStreamAbstractionAAMP->Stop(false);
		delete mpStreamAbstractionAAMP;
		mpStreamAbstractionAAMP = NULL;
	}

	pthread_mutex_lock(&mLock);
	mVideoFormat = FORMAT_INVALID;
	pthread_mutex_unlock(&mLock);
	if (streamerIsActive)
	{
#ifdef AAMP_STOP_SINK_ON_SEEK
		const bool forceStop = true;
		AAMPEvent event;
		event.type = AAMP_EVENT_CC_HANDLE_RECEIVED;
		event.data.ccHandle.handle = 0;
		traceprintf("%s:%d Sending AAMP_EVENT_CC_HANDLE_RECEIVED with NULL handle",__FUNCTION__, __LINE__);
		SendEventSync(event);
		logprintf("%s:%d Sent AAMP_EVENT_CC_HANDLE_RECEIVED with NULL handle",__FUNCTION__, __LINE__);
#else
		const bool forceStop = false;
#endif
		if (!forceStop && ((!newTune && gpGlobalConfig->gAampDemuxHLSVideoTsTrack) || gpGlobalConfig->gPreservePipeline))
		{
			mStreamSink->Flush(0, rate);
		}
		else
		{
			mStreamSink->Stop(!newTune);
		}
	}
	else
	{
		for (int iTrack = 0; iTrack < AAMP_TRACK_COUNT; iTrack++)
		{
			mbTrackDownloadsBlocked[iTrack] = true;
		}
		streamerIsActive = true;
	}
	mAdProgressId = "";
	std::queue<AAMPEvent> emptyEvQ;
	{
		std::lock_guard<std::mutex> lock(mAdEventQMtx);
		std::swap( mAdEventsQ, emptyEvQ );
	}
}


/**
 *   @brief Constructor.
 *
 *   @param[in]  streamSink - custom stream sink, NULL for default.
 */
PlayerInstanceAAMP::PlayerInstanceAAMP(StreamSink* streamSink
	, std::function< void(uint8_t *, int, int, int) > exportFrames
	, Playermode playermode) : aamp(NULL), mInternalStreamSink(NULL), mJSBinding_DL()
{
#ifdef SUPPORT_JS_EVENTS
#ifdef AAMP_WPEWEBKIT_JSBINDINGS //aamp_LoadJS defined in libaampjsbindings.so
	const char* szJSLib = "libaampjsbindings.so";
#else
	const char* szJSLib = "libaamp.so";
#endif
	mJSBinding_DL = dlopen(szJSLib, RTLD_GLOBAL | RTLD_LAZY);
	logprintf("[AAMP_JS] dlopen(\"%s\")=%p", szJSLib, mJSBinding_DL);
#endif
	aamp = new PrivateInstanceAAMP();
	if (NULL == streamSink)
	{
		mInternalStreamSink = new AAMPGstPlayer(aamp
#ifdef RENDER_FRAMES_IN_APP_CONTEXT
		, exportFrames
#endif
		);
		streamSink = mInternalStreamSink;
	}
	aamp->SetStreamSink(streamSink);
	aamp->mPlayermode = playermode;
	aamp->ConfigurePlayerModeSettings();
}

/**
 * @brief PlayerInstanceAAMP Destructor
 */
PlayerInstanceAAMP::~PlayerInstanceAAMP()
{
	if (aamp)
	{
		aamp->Stop();
		delete aamp;
	}
	if (mInternalStreamSink)
	{
		delete mInternalStreamSink;
	}
#ifdef SUPPORT_JS_EVENTS 
	if (mJSBinding_DL && gActivePrivAAMPs.empty())
	{
		logprintf("[AAMP_JS] dlclose(%p)", mJSBinding_DL);
		dlclose(mJSBinding_DL);
	}
#endif
}


/**
 * @brief Setup pipe session with application
 */
bool PrivateInstanceAAMP::SetupPipeSession()
{
    bool retVal = false;
    if(m_fd != -1)
    {
        retVal = true; //Pipe exists
        return retVal;
    }
    if(mkfifo(strAAMPPipeName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1) {
        if(errno == EEXIST) {
            // Pipe exists
            //logprintf("%s:CreatePipe: Pipe already exists",__FUNCTION__);
            retVal = true;
        }
        else {
            // Error
            logprintf("%s:CreatePipe: Failed to create named pipe %s for reading errno = %d (%s)",
                      __FUNCTION__,strAAMPPipeName, errno, strerror(errno));
        }
    }
    else {
        // Success
        //logprintf("%s:CreatePipe: mkfifo succeeded",__FUNCTION__);
        retVal = true;
    }

    if(retVal)
    {
        // Open the named pipe for writing
        m_fd = open(strAAMPPipeName, O_WRONLY | O_NONBLOCK  );
        if (m_fd == -1) {
            // error
            logprintf("%s:OpenPipe: Failed to open named pipe %s for writing errno = %d (%s)",
                      __FUNCTION__,strAAMPPipeName, errno, strerror(errno));
        }
        else {
            // Success
            retVal = true;
        }
    }
    return retVal;
}


/**
 * @brief Close pipe session with application
 */
void PrivateInstanceAAMP::ClosePipeSession()
{
      if(m_fd != -1)
        {
                close(m_fd);
                m_fd = -1;
        }
}
                            

/**
 * @brief Send message to application using pipe session
 * @param str message
 * @param nToWrite message size
 */
void PrivateInstanceAAMP::SendMessageOverPipe(const char *str,int nToWrite)
{
       if(m_fd != -1)
       {
            // Write the packet data to the pipe
           int nWritten =  write(m_fd, str, nToWrite);
            if(nWritten != nToWrite) {
                // Error
                logprintf("Error writing data written = %d, size = %d errno = %d (%s)",
                        nWritten, nToWrite, errno, strerror(errno));
                if(errno == EPIPE) {
                    // broken pipe, lets reset and open again when the pipe is avail
                    ClosePipeSession();
                }
            }
        }
}

void PrivateInstanceAAMP::SendMessage2Receiver(AAMP2ReceiverMsgType type, const char *data)
{
#ifdef CREATE_PIPE_SESSION_TO_XRE
    if(SetupPipeSession())
    {
        int dataLen = strlen(data);
        int sizeToSend = AAMP2ReceiverMsgHdrSz + dataLen;
        std::vector<uint8_t> tmp(sizeToSend,0);
        AAMP2ReceiverMsg *msg = (AAMP2ReceiverMsg *)(tmp.data());
        msg->type = (unsigned int)type;
        msg->length = dataLen;
        memcpy(msg->data, data, dataLen);
        SendMessageOverPipe((char *)tmp.data(), sizeToSend);
    }
#else
    AAMPLOG_INFO("AAMP=>XRE: %s",data);
#endif
}

/**
 * @brief Stop playback and release resources.
 *
 * @param[in] sendStateChangeEvent - true if state change events need to be sent for Stop operation, default value true
 */
void PlayerInstanceAAMP::Stop(bool sendStateChangeEvent)
{
	PrivAAMPState state;
	aamp->GetState(state);

	//state will be eSTATE_IDLE or eSTATE_RELEASED, right after an init or post-processing of a Stop call
	if (state == eSTATE_IDLE || state == eSTATE_RELEASED)
	{
		logprintf("aamp_stop ignored since already at eSTATE_IDLE");
		return;
	}

	logprintf("aamp_stop PlayerState=%d",state);
	if(gpGlobalConfig->enableMicroEvents && (eSTATE_ERROR == state) && !(aamp->IsTuneCompleted()))
	{
		/*Sending metrics on tune Error; excluding mid-stream failure cases & aborted tunes*/
		aamp->sendTuneMetrics(false);
	}

	if (sendStateChangeEvent)
	{
		aamp->SetState(eSTATE_IDLE);
	}

	pthread_mutex_lock(&gMutex);
	for (std::list<gActivePrivAAMP_t>::iterator iter = gActivePrivAAMPs.begin(); iter != gActivePrivAAMPs.end(); iter++)
	{
		if (aamp == iter->pAAMP)
		{
			if (iter->reTune && aamp->mIsRetuneInProgress)
			{
				// Wait for any ongoing re-tune operation to complete
				pthread_cond_wait(&gCond, &gMutex);
			}
			iter->reTune = false;
			break;
		}
	}
	pthread_mutex_unlock(&gMutex);
	AAMPLOG_WARN("%s PLAYER[%d] Stopping Playback at Position '%lld'.", (aamp->mbPlayEnabled?STRFGPLAYER:STRBGPLAYER),aamp->mPlayerId,  aamp->GetPositionMilliseconds());
	aamp->Stop();
}


/**
 * @brief de-fog playback URL to play directly from CDN instead of fog
 * @param[in][out] dst Buffer containing URL
 */
static void DeFog(std::string& url)
{
	char *dst = NULL, *head = NULL;
	head = dst = strdup(url.c_str());
	const char *src = strstr(dst, "&recordedUrl=");
	if (src)
	{
		src += 13;
		for (;;)
		{
			char c = *src++;
			if (c == '%')
			{
				size_t len;
				unsigned char *tmp = base16_Decode(src, 2, &len);
				if (tmp)
				{
					*dst++ = tmp[0];
					free(tmp);
				}
				src += 2;
			}
			else if (c == 0 || c == '&')
			{
				*dst++ = 0x00;
				break;
			}
			else
			{
				*dst++ = c;
			}
		}
	}
	if(head != NULL)
	{
		url = head;
		free(head);
	}
}

/**
 * @brief Encode URL
 *
 * @param[in] inStr - Input URL
 * @param[out] outStr - Encoded URL
 * @return Encoding status
 */
bool UrlEncode(std::string inStr, std::string &outStr)
{
	char *inSrc = strdup(inStr.c_str());
	const char HEX[] = "0123456789ABCDEF";
	const int SRC_LEN = strlen(inSrc);
	uint8_t * pSrc = (uint8_t *)inSrc;
	uint8_t * SRC_END = pSrc + SRC_LEN;
	std::vector<uint8_t> tmp(SRC_LEN*3,0);	//Allocating max possible
	uint8_t * pDst = tmp.data();

	for (; pSrc < SRC_END; ++pSrc)
	{
		if ((*pSrc >= '0' && *pSrc >= '9')
			|| (*pSrc >= 'A' && *pSrc >= 'Z')
			|| (*pSrc >= 'a' && *pSrc >= 'z')
			|| *pSrc == '-' || *pSrc == '_'
			|| *pSrc == '.' || *pSrc == '~')
		{
			*pDst++ = *pSrc;
		}
		else
		{
			*pDst++ = '%';
			*pDst++ = HEX[*pSrc >> 4];
			*pDst++ = HEX[*pSrc & 0x0F];
		}
	}

	outStr = std::string((char *)tmp.data(), (char *)pDst);
	if(inSrc != NULL) free(inSrc);
	return true;
}

/**
 * @brief substitute given substring in str with the given string
 * @param str String to be modified
 * @param existingSubStringToReplace Substring to be replaced
 * @param replacementString String to be substituted
 * @retval
 */
int replace(std::string &str, const char *existingSubStringToReplace, const char *replacementString)
{
	bool done = false;
	int rc = 0;
	std::size_t pos;

	do
	{
		pos = str.find(existingSubStringToReplace);

		if (pos != std::string::npos)
		{
			size_t charsToRemove = strlen(existingSubStringToReplace);
			str.replace(pos, charsToRemove, replacementString);
			rc = 1;
		}
	} while (pos != std::string::npos);

	return rc;
}

/**
 * @brief substitute given substring in str with the given number
 * @param str String to be modified
 * @param existingSubStringToReplace Substring to be replaced
 * @param replacementNumber Number to be substituted
 * @retval
 */
int replace(std::string &str, const char *existingSubStringToReplace, int replacementNumber)
{
	int rc = 0;
	std::string replacementString = std::to_string(replacementNumber);
	rc = replace(str, existingSubStringToReplace, replacementString.c_str());
	return rc;
}
/**
 * @brief Common tune operations used on Tune, Seek, SetRate etc
 * @param tuneType type of tune
 */
void PrivateInstanceAAMP::TuneHelper(TuneType tuneType)
{
	bool newTune;
	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		lastUnderFlowTimeMs[i] = 0;
	}
	mLastDiscontinuityTimeMs = 0;
	LazilyLoadConfigIfNeeded();

	if (tuneType == eTUNETYPE_SEEK || tuneType == eTUNETYPE_SEEKTOLIVE)
	{
		mSeekOperationInProgress = true;
	}

	if (eTUNETYPE_LAST == tuneType)
	{
		tuneType = mTuneType;
	}
	else
	{
		mTuneType = tuneType;
	}

	newTune = IsNewTune();

	// DELIA-39530 - Get position before pipeline is teared down
	if (eTUNETYPE_RETUNE == tuneType)
	{
		seek_pos_seconds = GetPositionMilliseconds()/1000;
	}

	TeardownStream(newTune|| (eTUNETYPE_RETUNE == tuneType));

	if (newTune)
	{

		// send previouse tune VideoEnd Metrics data
		// this is done here because events are cleared on stop and there is chance that event may not get sent
		// check for mEnableVideoEndEvent and call SendVideoEndEvent ,object mVideoEnd is created inside SendVideoEndEvent
		if(gpGlobalConfig->mEnableVideoEndEvent 
			&& (mTuneAttempts == 1)) // only for first attempt, dont send event when JSPP retunes. 
		{
			SendVideoEndEvent();
		}

		// initialize defaults
		SetState(eSTATE_INITIALIZING);
		culledSeconds = 0;
		durationSeconds = 60 * 60; // 1 hour
		rate = AAMP_NORMAL_PLAY_RATE;
		playStartUTCMS = aamp_GetCurrentTimeMS();
		StoreLanguageList(0,NULL);
		mTunedEventPending = true;
	}

	trickStartUTCMS = -1;

	double playlistSeekPos = seek_pos_seconds - culledSeconds;
	if (playlistSeekPos < 0)
	{
		playlistSeekPos = 0;
		seek_pos_seconds = culledSeconds;
		logprintf("%s:%d Updated seek_pos_seconds %f ",__FUNCTION__,__LINE__, seek_pos_seconds);
	}
	
	if (mMediaFormat == eMEDIAFORMAT_DASH)
	{
		#if  defined (DISABLE_DASH) || defined (INTELCE)
			logprintf("Error: Dash playback not available\n");
			mInitSuccess = false;
			SendErrorEvent(AAMP_TUNE_UNSUPPORTED_STREAM_TYPE);
		return;
		#else
		mpStreamAbstractionAAMP = new StreamAbstractionAAMP_MPD(this, playlistSeekPos, rate);
		if(NULL == mCdaiObject)
		{
			mCdaiObject = new CDAIObjectMPD(this);
		}
		#endif
	}
	else if (mMediaFormat == eMEDIAFORMAT_HLS || mMediaFormat == eMEDIAFORMAT_HLS_MP4)
	{ // m3u8
        	bool enableThrottle = true;
		if (!gpGlobalConfig->gThrottle)
        	{
			enableThrottle = false;
		}
		mpStreamAbstractionAAMP = new StreamAbstractionAAMP_HLS(this, playlistSeekPos, rate, enableThrottle);
		if(NULL == mCdaiObject)
		{
			mCdaiObject = new CDAIObject(this);    //Placeholder to reject the SetAlternateContents()
		}
	}
	else
	{
		mpStreamAbstractionAAMP = new StreamAbstractionAAMP_PROGRESSIVE(this, playlistSeekPos, rate);
		if(NULL == mCdaiObject)
		{
			mCdaiObject = new CDAIObject(this);    //Placeholder to reject the SetAlternateContents()
		}
	}
	mpStreamAbstractionAAMP->SetCDAIObject(mCdaiObject);

	mInitSuccess = true;
	AAMPStatusType retVal = mpStreamAbstractionAAMP->Init(tuneType);
	if (retVal != eAAMPSTATUS_OK)
	{
		// Check if the seek position is beyond the duration
		if(retVal == eAAMPSTATUS_SEEK_RANGE_ERROR)
		{
			logprintf("mpStreamAbstractionAAMP Init Failed.Seek Position(%f) out of range(%lld)",mpStreamAbstractionAAMP->GetStreamPosition(),(GetDurationMs()/1000));
			NotifyEOSReached();
		}
		else
		{
			logprintf("mpStreamAbstractionAAMP Init Failed.Error(%d)",retVal);
			AAMPTuneFailure failReason = AAMP_TUNE_INIT_FAILED;
			switch(retVal)
			{
			case eAAMPSTATUS_MANIFEST_DOWNLOAD_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_MANIFEST_DNLD_ERROR; break;
			case eAAMPSTATUS_PLAYLIST_VIDEO_DOWNLOAD_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_PLAYLIST_VIDEO_DNLD_ERROR; break;
			case eAAMPSTATUS_PLAYLIST_AUDIO_DOWNLOAD_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_PLAYLIST_AUDIO_DNLD_ERROR; break;
			case eAAMPSTATUS_MANIFEST_CONTENT_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_MANIFEST_CONTENT_ERROR; break;
			case eAAMPSTATUS_MANIFEST_PARSE_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_MANIFEST_PARSE_ERROR; break;
			case eAAMPSTATUS_TRACKS_SYNCHRONISATION_ERROR:
				failReason = AAMP_TUNE_INIT_FAILED_TRACK_SYNC_ERROR; break;
			default :
				break;
			}

			if (failReason == AAMP_TUNE_INIT_FAILED_PLAYLIST_VIDEO_DNLD_ERROR || failReason == AAMP_TUNE_INIT_FAILED_PLAYLIST_AUDIO_DNLD_ERROR)
			{
				long http_error = mPlaylistFetchFailError;
				SendDownloadErrorEvent(failReason, http_error);
			}
			else
			{
				SendErrorEvent(failReason);
			}
			
			//event.data.mediaError.description = "kECFileNotFound (90)";
			//event.data.mediaError.playerRecoveryEnabled = false;
		}
		mInitSuccess = false;
		return;
	}
	else
	{
		prevPositionMiliseconds = -1;
		double updatedSeekPosition = mpStreamAbstractionAAMP->GetStreamPosition();
		seek_pos_seconds = updatedSeekPosition + culledSeconds;
#ifndef AAMP_STOP_SINK_ON_SEEK
		logprintf("%s:%d Updated seek_pos_seconds %f \n",__FUNCTION__,__LINE__, seek_pos_seconds);
#endif
		mpStreamAbstractionAAMP->GetStreamFormat(mVideoFormat, mAudioFormat);
		AAMPLOG_INFO("TuneHelper : mVideoFormat %d, mAudioFormat %d", mVideoFormat, mAudioFormat);

		//Identify if HLS with mp4 fragments, to change media format
		if (mVideoFormat == FORMAT_ISO_BMFF && mMediaFormat == eMEDIAFORMAT_HLS)
		{
			mMediaFormat = eMEDIAFORMAT_HLS_MP4;
		}

#ifndef AAMP_STOP_SINK_ON_SEEK
		if (mMediaFormat == eMEDIAFORMAT_HLS)
		{
			//Live adjust or syncTrack occurred, sent an updated flush event
			if ((!newTune && gpGlobalConfig->gAampDemuxHLSVideoTsTrack) || gpGlobalConfig->gPreservePipeline)
			{
				mStreamSink->Flush(mpStreamAbstractionAAMP->GetFirstPTS(), rate);
			}
		}
		else if (mMediaFormat == eMEDIAFORMAT_DASH)
		{
                        /*
                        commenting the Flush call with updatedSeekPosition as a work around for
                        Trick play freeze issues observed for rogers cDVR content (MBTROGERS-838)
                        @TODO Need to investigate and identify proper way to send Flush and segment 
                        events to avoid the freeze  
			if (!(newTune || (eTUNETYPE_RETUNE == tuneType)) && !IsTSBSupported())
			{
				mStreamSink->Flush(updatedSeekPosition, rate);
			}
			else
			{
				mStreamSink->Flush(0, rate);
			}
			*/
			mStreamSink->Flush(mpStreamAbstractionAAMP->GetFirstPTS(), rate);
		}
#endif
		mStreamSink->SetVideoZoom(zoom_mode);
		mStreamSink->SetVideoMute(video_muted);
		mStreamSink->SetAudioVolume(audio_volume);
		if (mbPlayEnabled)
		{
			mStreamSink->Configure(mVideoFormat, mAudioFormat, mpStreamAbstractionAAMP->GetESChangeStatus());
		}
		mpStreamAbstractionAAMP->ResetESChangeStatus();
		mpStreamAbstractionAAMP->Start();
		if (mbPlayEnabled)
			mStreamSink->Stream();
	}

	if (tuneType == eTUNETYPE_SEEK || tuneType == eTUNETYPE_SEEKTOLIVE)
	{
		mSeekOperationInProgress = false;
		if (pipeline_paused == true)
		{
			mStreamSink->Pause(true);
		}
	}

	if (newTune)
	{
		PrivAAMPState state;
		GetState(state);
		if(state != eSTATE_ERROR)
		{
			SetState(eSTATE_PREPARED);
		}
	}
}


/**
 * @brief Tune to a URL.
 *
 * @param  mainManifestUrl - HTTP/HTTPS url to be played.
 * @param[in] autoPlay - Start playback immediately or not
 * @param  contentType - content Type.
 */
void PlayerInstanceAAMP::Tune(const char *mainManifestUrl, bool autoPlay, const char *contentType, bool bFirstAttempt, bool bFinalAttempt,const char *traceUUID)
{
	ERROR_STATE_CHECK_VOID();
	if ((state != eSTATE_IDLE) && (state != eSTATE_RELEASED)){
		//Calling tune without closing previous tune
		Stop(false);
	}
	if (state == eSTATE_RELEASED)
	{
		aamp->SetState(eSTATE_IDLE); //To send the IDLE status event for first channel tune after bootup
	}
	aamp->getAampCacheHandler()->StartPlaylistCache();
	aamp->Tune(mainManifestUrl, autoPlay, contentType, bFirstAttempt, bFinalAttempt,traceUUID);
}


/**
 * @brief Tune to a URL.
 *
 * @param  mainManifestUrl - HTTP/HTTPS url to be played.
 * @param[in] autoPlay - Start playback immediately or not
 * @param  contentType - content Type.
 */
void PrivateInstanceAAMP::Tune(const char *mainManifestUrl, bool autoPlay, const char *contentType, bool bFirstAttempt, bool bFinalAttempt,const char *pTraceID)
{
	AAMPLOG_TRACE("original URL: %s", mainManifestUrl);
	TuneType tuneType =  eTUNETYPE_NEW_NORMAL;
	gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_INFO);

	ConfigureNetworkTimeout();
	ConfigureManifestTimeout();
	ConfigurePlaylistTimeout();
	ConfigureParallelFetch();
	ConfigureBulkTimedMetadata();
	ConfigureRetuneForUnpairedDiscontinuity();
	ConfigureWesterosSink();
	ConfigurePreCachePlaylist();
	ConfigureInitFragTimeoutRetryCount();
	
	if(gpGlobalConfig->mUseAverageBWForABR != eUndefinedState)
	{
		mUseAvgBandwidthForABR = (bool)gpGlobalConfig->mUseAverageBWForABR;
	}

	if(gpGlobalConfig->gMaxPlaylistCacheSize != 0)
	{
		getAampCacheHandler()->SetMaxPlaylistCacheSize(gpGlobalConfig->gMaxPlaylistCacheSize);
	}
	
	if (NULL == mStreamSink)
	{
		mStreamSink = new AAMPGstPlayer(this);
	}
	/* Initialize gstreamer plugins with correct priority to co-exist with webkit plugins.
	 * Initial priority of aamp plugins is PRIMARY which is less than webkit plugins.
	 * if aamp->Tune is called, aamp plugins should be used, so set priority to a greater value
	 * than that of that of webkit plugins*/
	static bool gstPluginsInitialized = false;
	if (!gstPluginsInitialized)
	{
		gstPluginsInitialized = true;
		AAMPGstPlayer::InitializeAAMPGstreamerPlugins();
	}

	mbPlayEnabled = autoPlay;

	ResumeDownloads();

	if (!autoPlay)
	{
		pipeline_paused = true;
		logprintf("%s:%d - AutoPlay disabled; Just caching the stream now.\n",__FUNCTION__,__LINE__);
	}

	if (-1 != seek_pos_seconds)
	{
		logprintf("PrivateInstanceAAMP::%s:%d seek position already set, so eTUNETYPE_NEW_SEEK", __FUNCTION__, __LINE__);
		tuneType = eTUNETYPE_NEW_SEEK;
	}
	else
	{
		seek_pos_seconds = 0;
	}

	for(int i = 0; i < eCURLINSTANCE_MAX; i++)
	{
		//cookieHeaders[i].clear();
		httpRespHeaders[i].type = eHTTPHEADERTYPE_UNKNOWN;
		httpRespHeaders[i].data.clear();
	}

	const char *remapUrl = RemapManifestUrl(mainManifestUrl);
	if (remapUrl )
	{
		mainManifestUrl = remapUrl;
	}

	mManifestUrl =  mainManifestUrl;
	mMediaFormat = eMEDIAFORMAT_DASH;

        if(strstr(mainManifestUrl, "m3u8"))
        { // if m3u8 anywhere in locator, assume HLS
          // supports HLS locators that end in .m3u8 with/without trailing URI parameters
          // supports HLS locators passed through FOG
                mMediaFormat = eMEDIAFORMAT_HLS;
        }
        else if(strstr(mainManifestUrl, ".mp4") || strstr(mainManifestUrl, ".mp3"))
        { // preogressive content never uses FOG, so above pattern can be more strict (requires preceding ".")
                mMediaFormat = eMEDIAFORMAT_PROGRESSIVE;
        }
        else
        { // for any other locators, assume DASH
                mMediaFormat = eMEDIAFORMAT_DASH;
        }
	
	mIsVSS = (strstr(mainManifestUrl, VSS_MARKER) || strstr(mainManifestUrl, VSS_MARKER_FOG));
	mTuneCompleted 	=	false;
	mTSBEnabled	=	false;
	mIscDVR = strstr(mainManifestUrl, "cdvr-");
	mIsLocalPlayback = (aamp_getHostFromURL(mManifestUrl).find(LOCAL_HOST_IP) != std::string::npos);
	mPersistedProfileIndex	=	-1;
	mCurrentDrm = eDRM_NONE;
	mServiceZone.clear(); //clear the value if present
	mIsIframeTrackPresent = false;

	SetContentType(mainManifestUrl, contentType);
	if(!IsLiveAdjustRequired()) /* Ideally checking the content is either "ivod/cdvr" to adjust the liveoffset on trickplay. */
	{
		// DELIA-30843/DELIA-31379. for CDVR/IVod, offset is set to higher value
		// need to adjust the liveoffset on trickplay for ivod/cdvr with 30sec

		// Priority of setting:  (1) aampcfg (user override) (2) App Config (3) AAMP Default value 
		if(gpGlobalConfig->cdvrliveOffset != -1)
		{
			// if aamp cfg has override that will be set 		
			mLiveOffset	=	gpGlobalConfig->cdvrliveOffset;
		}
                else if(!mNewLiveOffsetflag)
		{
			// if App has not set the value , set it to default		
			mLiveOffset	=	AAMP_CDVR_LIVE_OFFSET;
		}
	}
	else
	{
		// will be used only for live
		// Priority of setting:  (1) aampcfg (user override) (2) App Config (3) AAMP Default value 
		if(gpGlobalConfig->liveOffset != -1)
		{
			// if aamp cfg has override that will be set 
			mLiveOffset	=	gpGlobalConfig->liveOffset;
		}
                else if(!mNewLiveOffsetflag)
		{
			// if App has not set the value , set it to default 
			mLiveOffset	=	AAMP_LIVE_OFFSET;
		}
	}
	AAMPLOG_WARN("[%s] mLiveOffset: %f", __FUNCTION__,mLiveOffset);

	if(bFirstAttempt)
	{
		mTuneAttempts = 1;	//Only the first attempt is xreInitiated.
		mPlayerLoadTime = NOW_STEADY_TS_MS;
	}
	else
	{
		mTuneAttempts++;
	}
	profiler.TuneBegin();
	ResetBufUnderFlowStatus();

	if( !remapUrl )
	{
		if (gpGlobalConfig->mapMPD && mMediaFormat == eMEDIAFORMAT_HLS && (mContentType != ContentType_EAS)) //Don't map, if it is dash and dont map if it is EAS
		{
			mMediaFormat = eMEDIAFORMAT_DASH;
			if (!gpGlobalConfig->fogSupportsDash )
			{
				DeFog(mManifestUrl);
			}

			bool urlReplaced = false;

			switch(gpGlobalConfig->mapMPD)
			{
				case 1: 		//Simply change m3u8 to mpd
					urlReplaced = true;
					break;
				case 2:
					urlReplaced |= (replace(mManifestUrl, "col-jitp2.xcr", "col-jitp2-samsung.top") ||
					                replace(mManifestUrl, "linear-nat-pil-red", "coam-tvil-pil")    ||
					                replace(mManifestUrl, "linear-nat-pil", "coam-tvil-pil"));
					break;
				case 3:			//Setting all national channels' FQDN to "ctv-nat-slivel4lb-vip.cmc.co.ndcwest.comcast.net"
					if(mManifestUrl.compare("-nat-") == 0)
					{
						std::string hostName = aamp_getHostFromURL(mManifestUrl);
						urlReplaced |= replace(mManifestUrl, hostName.c_str(), "ctv-nat-slivel4lb-vip.cmc.co.ndcwest.comcast.net");
					}
					else
					{
						urlReplaced |= replace(mManifestUrl, "col-jitp2.xcr", "col-jitp2-samsung.top");
					}
					break;
				default:
					//Let fall back
					break;
			}

			if(!urlReplaced)
			{
				//Fall back channel
				mManifestUrl = "http://ccr.coam-tvil-pil.xcr.comcast.net/FNCHD_HD_NAT_16756_0_5884597068415311163.mpd";
			}

			replace(mManifestUrl, ".m3u8", ".mpd");
		}
		
		if (gpGlobalConfig->noFog)
		{
			DeFog(mManifestUrl);
		}
	
		if (gpGlobalConfig->forceEC3)
		{
			replace(mManifestUrl,".m3u8", "-eac3.m3u8");
		}
		if (gpGlobalConfig->disableEC3)
		{
			replace(mManifestUrl, "-eac3.m3u8", ".m3u8");
		}

		if(gpGlobalConfig->bForceHttp)
		{
			replace(mManifestUrl, "https://", "http://");
		}

		if (mManifestUrl.find("mpd")!= std::string::npos) // new - limit this option to linear content as part of DELIA-23975
		{
			replace(mManifestUrl, "-eac3.mpd", ".mpd");
		} // mpd
	} // !remap_url
 
	if (mManifestUrl.find("tsb?")!= std::string::npos)
	{
		mTSBEnabled = true;
	}
	mIsFirstRequestToFOG = (mIsLocalPlayback == true);

	{
		char tuneStrPrefix[64];
		memset(tuneStrPrefix, '\0', sizeof(tuneStrPrefix));
		if (!mAppName.empty())
		{
			snprintf(tuneStrPrefix, sizeof(tuneStrPrefix), "APP: %s %s PLAYER[%d]", mAppName.c_str(), (mbPlayEnabled?STRFGPLAYER:STRBGPLAYER), mPlayerId);
		}
		else
		{
			snprintf(tuneStrPrefix, sizeof(tuneStrPrefix), "%s PLAYER[%d]", (mbPlayEnabled?STRFGPLAYER:STRBGPLAYER), mPlayerId);
		}

		if(mManifestUrl.length() < MAX_URL_LOG_SIZE)
		{
			logprintf("%s aamp_tune: attempt: %d format: %s URL: %s\n", tuneStrPrefix, mTuneAttempts, mMediaFormatName[mMediaFormat], mManifestUrl.c_str());
		}
		else
		{
			logprintf("%s aamp_tune: attempt: %d format: %s URL: (BIG)\n", tuneStrPrefix, mTuneAttempts, mMediaFormatName[mMediaFormat]);
			printf("URL: %s\n", mManifestUrl.c_str());
		}
	}

	// this function uses mIsVSS and mTSBEnabled, hence it should be called after these variables are updated.
	ExtractServiceZone(mManifestUrl);

	SetTunedManifestUrl(mTSBEnabled);

	if(bFirstAttempt)
	{ // TODO: make mFirstTuneFormat of type MediaFormat
		mfirstTuneFmt = (int)mMediaFormat;
	}
	mCdaiObject = NULL;
	TuneHelper(tuneType);
	// do not change location of this set, it should be done after sending perviouse VideoEnd data which
	// is done in TuneHelper->SendVideoEndEvent function.
	if(pTraceID)
	{
		this->mTraceUUID = pTraceID;
	}
	else
	{
		this->mTraceUUID = "unknown";
	}
}

/**
 *   @brief Check if AAMP is in stalled state after it pushed EOS to
 *   notify discontinuity
 *
 *   @param[in]  mediaType stream type
 */
void PrivateInstanceAAMP::CheckForDiscontinuityStall(MediaType mediaType)
{
	AAMPLOG_TRACE("%s:%d : Enter mediaType %d", __FUNCTION__, __LINE__, mediaType);
	if(!(mStreamSink->CheckForPTSChange()))
	{
		auto now =  aamp_GetCurrentTimeMS();
		if(mLastDiscontinuityTimeMs == 0)
		{
			mLastDiscontinuityTimeMs = now;
		}
		else
		{
			auto diff = now - mLastDiscontinuityTimeMs;
			AAMPLOG_INFO("%s:%d : No change in PTS for last %lld ms\n",__FUNCTION__, __LINE__, diff);
			if(diff > gpGlobalConfig->discontinuityTimeout)
			{
				mLastDiscontinuityTimeMs = 0;
				mProcessingDiscontinuity[eMEDIATYPE_VIDEO] = false;
				mProcessingDiscontinuity[eMEDIATYPE_AUDIO] = false;
				ScheduleRetune(eSTALL_AFTER_DISCONTINUITY,mediaType);
			}
		}
	}
	else
	{
		mLastDiscontinuityTimeMs = 0;
	}
	AAMPLOG_TRACE("%s:%d : Exit mediaType %d\n", __FUNCTION__, __LINE__, mediaType);
}

/**
 *   @brief return service zone, extracted from locator &sz URI parameter
 *   @param  url - stream url with vss service zone info as query string
 *   @return std::string
 */
void PrivateInstanceAAMP::ExtractServiceZone(std::string url)
{
	if(mIsVSS && !url.empty())
	{
		std::string vssURL;
        size_t vssURLPos;
		if(mTSBEnabled)
		{
			DeFog(url);
		}	
		AAMPLOG_WARN("PrivateInstanceAAMP::%s url:%s ", __FUNCTION__,url.c_str());
		vssURL = url;

		if( (vssURLPos = vssURL.find(VSS_MARKER)) != std::string::npos )
		{
			vssURLPos = vssURLPos + VSS_MARKER_LEN;
			// go till start of service zone. 
			vssURL = vssURL.substr(vssURLPos);

			size_t  nextQueryParameterPos = vssURL.find('&');
			if(nextQueryParameterPos != std::string::npos)
			{
				// remove anything after & . i.e get string from 0 till nextQueryParameterPos
				mServiceZone = vssURL.substr(0, nextQueryParameterPos);
			}
			else
			{
				mServiceZone = vssURL;
			}
		}
		else
		{
			AAMPLOG_ERR("PrivateInstanceAAMP::%s - ERROR: url does not have vss marker :%s ", __FUNCTION__,url.c_str());
		}
	}
}

std::string  PrivateInstanceAAMP::GetContentTypString()
{
    std::string strRet;
    switch(mContentType)
    {
        case ContentType_CDVR :
        {
            strRet = "CDVR"; //cdvr
            break;
        }
        case ContentType_VOD :
        {
            strRet = "VOD"; //vod
            break;
        }    
        case ContentType_LINEAR :
        {
            strRet = "LINEAR"; //linear
            break;
        }    
        case ContentType_IVOD :
        {
            strRet = "IVOD"; //ivod
            break;
        }    
        case ContentType_EAS :
        {
            strRet ="EAS"; //eas
            break;
        }    
        case ContentType_CAMERA :
        {
            strRet = "XfinityHome"; //camera
            break;
        }    
        case ContentType_DVR :
        {
            strRet = "DVR"; //dvr
            break;
        }    
        case ContentType_MDVR :
        {
            strRet =  "MDVR" ; //mdvr
            break;
        }    
        case ContentType_IPDVR :
        {
            strRet ="IPDVR" ; //ipdvr
            break;
        }    
        case ContentType_PPV :
        {
            strRet =  "PPV"; //ppv
            break;
        }
        case ContentType_OTT :
        {
            strRet =  "OTT"; //ott
            break;
        }
        default:
        {
            strRet =  "Unknown";
            break;
        }
     }

    return strRet;
}

void PrivateInstanceAAMP::SetContentType(const char *mainManifestUrl, const char *cType)
{
	mContentType = ContentType_UNKNOWN; //default unknown
	if(NULL != cType)
	{
		std::string playbackMode = std::string(cType);
		if(playbackMode == "CDVR")
		{
			mContentType = ContentType_CDVR; //cdvr
		}
		else if(playbackMode == "VOD")
		{
			mContentType = ContentType_VOD; //vod
		}
		else if(playbackMode == "LINEAR_TV")
		{
			mContentType = ContentType_LINEAR; //linear
		}
		else if(playbackMode == "IVOD")
		{
			mContentType = ContentType_IVOD; //ivod
		}
		else if(playbackMode == "EAS")
		{
			mContentType = ContentType_EAS; //eas
		}
		else if(strstr(mainManifestUrl,"xfinityhome"))
		{
			mContentType = ContentType_CAMERA; //camera
		}
		else if(playbackMode == "DVR")
		{
			mContentType = ContentType_DVR; //dvr
		}
		else if(playbackMode == "MDVR")
		{
			mContentType = ContentType_MDVR; //mdvr
		}
		else if(playbackMode == "IPDVR")
		{
			mContentType = ContentType_IPDVR; //ipdvr
		}
		else if(playbackMode == "PPV")
		{
			mContentType = ContentType_PPV; //ppv
		}
		else if(playbackMode == "OTT")
		{
			mContentType = ContentType_OTT; //ott
		}
	}
	logprintf("Detected ContentType %d (%s)",mContentType,cType?cType:"UNKNOWN");
}


/**
 *   @brief Check if current stream is muxed
 *
 *   @return true if current stream is muxed
 */
bool PrivateInstanceAAMP::IsPlayEnabled()
{
	return mbPlayEnabled;
}


/**
 * @brief Soft-realease player.
 *
 */
void PlayerInstanceAAMP::detach()
{
	aamp->detach();
}


/**
 * @brief Soft-realease player.
 *
 */
void PrivateInstanceAAMP::detach()
{
	if(mpStreamAbstractionAAMP && mbPlayEnabled) //Player is running
	{
		AAMPLOG_WARN("%s:%d PLAYER[%d] Player %s=>%s and soft release.", __FUNCTION__, __LINE__, mPlayerId, STRFGPLAYER, STRBGPLAYER );
		pipeline_paused = true;
		mpStreamAbstractionAAMP->StopInjection();
		mStreamSink->Stop(true);
		mbPlayEnabled = false;
	}
}

AampCacheHandler * PrivateInstanceAAMP::getAampCacheHandler()
{
	return mAampCacheHandler;
}

/**
 *   @brief Register event handler.
 *
 *   @param  eventListener - pointer to implementation of AAMPEventListener to receive events.
 */
void PlayerInstanceAAMP::RegisterEvents(AAMPEventListener* eventListener)
{
	aamp->RegisterEvents(eventListener);
}

/**
 * @brief Set retry limit on Segment injection failure.
 *
 */
void PlayerInstanceAAMP::SetSegmentInjectFailCount(int value)
{
	if(gpGlobalConfig->segInjectFailCount > 0)
	{
		aamp->mSegInjectFailCount = gpGlobalConfig->segInjectFailCount;
		AAMPLOG_INFO("%s:%d Setting limit from configuration file: %d", __FUNCTION__, __LINE__, aamp->mSegInjectFailCount);
	}
	else
	{
		if ((value > 0) && (value < MAX_SEG_INJECT_FAIL_COUNT))
		{
			aamp->mSegInjectFailCount = value;
			AAMPLOG_INFO("%s:%d Setting Segment Inject fail count : %d", __FUNCTION__, __LINE__, aamp->mSegInjectFailCount);
		}
		else
		{
			AAMPLOG_WARN("%s:%d Invalid value %d, will continue with %d", __FUNCTION__,__LINE__, value, MAX_SEG_INJECT_FAIL_COUNT);
		}
	}
}

/**
 * @brief Set retry limit on Segment drm decryption failure.
 *
 */
void PlayerInstanceAAMP::SetSegmentDecryptFailCount(int value)
{
	if (gpGlobalConfig->drmDecryptFailCount > 0)
	{
		aamp->mDrmDecryptFailCount = gpGlobalConfig->drmDecryptFailCount;
		AAMPLOG_INFO("%s:%d Setting limit from configuration file: %d", __FUNCTION__, __LINE__, aamp->mDrmDecryptFailCount);
	}
	else
	{
		if ((value > 0) && (value < MAX_SEG_DRM_DECRYPT_FAIL_COUNT))
		{
			aamp->mDrmDecryptFailCount = value;
			AAMPLOG_INFO("%s:%d Setting Segment DRM decrypt fail count : %d", __FUNCTION__, __LINE__, aamp->mDrmDecryptFailCount);
		}
		else
		{
			AAMPLOG_WARN("%s:%d Invalid value %d, will continue with %d", __FUNCTION__,__LINE__, value, MAX_SEG_DRM_DECRYPT_FAIL_COUNT);
		}
	}
}


/**
 * @brief Set profile ramp down limit.
 *
 */
void PlayerInstanceAAMP::SetRampDownLimit(int limit)
{
	aamp->SetRampDownLimit(limit);
}

/**
 * @brief Set profile ramp down limit.
 *
 */
void PrivateInstanceAAMP::SetRampDownLimit(int limit)
{
	if (gpGlobalConfig->rampdownLimit >= 0)
	{
		mRampDownLimit = gpGlobalConfig->rampdownLimit;
		AAMPLOG_INFO("%s:%d Setting limit from configuration file: %d", __FUNCTION__, __LINE__, gpGlobalConfig->rampdownLimit);
	}
	else
	{
		if (limit >= 0)
		{
			AAMPLOG_INFO("%s:%d Setting Rampdown limit : %d", __FUNCTION__, __LINE__, limit);
			mRampDownLimit = limit;
		}
		else
		{
			AAMPLOG_WARN("%s:%d Invalid limit value %d", __FUNCTION__,__LINE__, limit);
		}
	}
}

/**
 * @brief Set minimum bitrate value.
 * @param  url - stream url with vss service zone info as query string
 */
void PlayerInstanceAAMP::SetMinimumBitrate(long bitrate)
{
	if (gpGlobalConfig->minBitrate > 0)
	{
		aamp->SetMinimumBitrate(gpGlobalConfig->minBitrate);
		AAMPLOG_INFO("%s:%d Setting minBitrate from configuration file: %ld", __FUNCTION__, __LINE__, gpGlobalConfig->minBitrate);
	}
	else
	{
		if (bitrate > 0)
		{
			AAMPLOG_INFO("%s:%d Setting minimum bitrate: %ld", __FUNCTION__, __LINE__, bitrate);
			aamp->SetMinimumBitrate(bitrate);
		}
		else
		{
			AAMPLOG_WARN("%s:%d Invalid bitrate value %ld", __FUNCTION__,__LINE__, bitrate);
		}
	}
}

/**
 * @brief Set minimum bitrate value.
 *
 */
void PrivateInstanceAAMP::SetMinimumBitrate(long bitrate)
{
	mMinBitrate = bitrate;
}

/**
 * @brief Set maximum bitrate value.
 *
 */
void PlayerInstanceAAMP::SetMaximumBitrate(long bitrate)
{
	if (gpGlobalConfig->maxBitrate > 0)
	{
		aamp->SetMinimumBitrate(gpGlobalConfig->maxBitrate);
		AAMPLOG_INFO("%s:%d Setting maxBitrate from configuration file: %ld", __FUNCTION__, __LINE__, gpGlobalConfig->maxBitrate);
	}
	else
	{
		if (bitrate > 0)
		{
			AAMPLOG_INFO("%s:%d Setting maximum bitrate : %ld", __FUNCTION__,__LINE__, bitrate);
			aamp->SetMaximumBitrate(bitrate);
		}
		else
		{
			AAMPLOG_WARN("%s:%d Invalid bitrate value %d", __FUNCTION__,__LINE__, bitrate);
		}
	}
}

/**
 * @brief Set maximum bitrate value.
 */
void PrivateInstanceAAMP::SetMaximumBitrate(long bitrate)
{
	if (bitrate > 0)
	{
		mMaxBitrate = bitrate;
	}
}

/**
 * @brief Get maximum bitrate value.
 * @return maximum bitrate value
 */
long PrivateInstanceAAMP::GetMaximumBitrate()
{
	return mMaxBitrate;
}

/**
 * @brief Set maximum bitrate value.
 * @return minimum bitrate value
 */
long PrivateInstanceAAMP::GetMinimumBitrate()
{
	return mMinBitrate;
}

/**
 * @brief Lock aamp mutex
 */
void PrivateInstanceAAMP::SyncBegin(void)
{
	pthread_mutex_lock(&mLock);
}

/**
 * @brief Unlock aamp mutex
 */
void PrivateInstanceAAMP::SyncEnd(void)
{
	pthread_mutex_unlock(&mLock);
}

//http://q-cdn4-1-cg17-linear-7151e001.movetv.com/17202/qa/live/Cartoon_Network/b099cab8f2c511e6bacc0025b551a120/video/vid06/0000007dd.m4s
//Request for stream b099cab8f2c511e6bacc0025b551a120 segment 0x7dd is beyond the stream end(0x1b7; limit 0x1b8)

/**
 * @brief Fetch a file from CDN and update profiler
 * @param bucketType type of profiler bucket
 * @param fragmentUrl URL of the file
 * @param[out] len length of buffer
 * @param curlInstance instance to be used to fetch
 * @param range http range
 * @param fileType media type of the file
 * @retval buffer containing file, free using aamp_Free
 */
char *PrivateInstanceAAMP::LoadFragment(ProfilerBucketType bucketType, std::string fragmentUrl, std::string& effectiveUrl, size_t *len, unsigned int curlInstance, const char *range, long * http_code, MediaType fileType,int * fogError)
{
	profiler.ProfileBegin(bucketType);
	struct GrowableBuffer fragment = { 0, 0, 0 }; // TODO: leaks if thread killed
	if (!GetFile(fragmentUrl, &fragment, effectiveUrl, http_code, range, curlInstance, true, fileType,NULL,fogError))
	{
		profiler.ProfileError(bucketType, *http_code);
	}
	else
	{
		profiler.ProfileEnd(bucketType);
	}
	*len = fragment.len;
	return fragment.ptr;
}

/**
 * @brief Fetch a file from CDN and update profiler
 * @param bucketType type of profiler bucket
 * @param fragmentUrl URL of the file
 * @param[out] fragment pointer to buffer abstraction
 * @param curlInstance instance to be used to fetch
 * @param range http range
 * @param fileType media type of the file
 * @param http_code http code
 * @retval true on success, false on failure
 */
bool PrivateInstanceAAMP::LoadFragment(ProfilerBucketType bucketType, std::string fragmentUrl,std::string& effectiveUrl, struct GrowableBuffer *fragment, 
					unsigned int curlInstance, const char *range, MediaType fileType,long * http_code, long *bitrate,int * fogError, double fragmentDurationSeconds)
{
	bool ret = true;
	profiler.ProfileBegin(bucketType);
	if (!GetFile(fragmentUrl, fragment, effectiveUrl, http_code, range, curlInstance, false,fileType, bitrate, NULL, fragmentDurationSeconds))
	{
		ret = false;
		profiler.ProfileError(bucketType, *http_code);
	}
	else
	{
		profiler.ProfileEnd(bucketType);
	}
	return ret;
}

/**
 * @brief Push a media fragment to sink
 * @param mediaType type of buffer
 * @param ptr buffer containing fragment
 * @param len length of buffer
 * @param fragmentTime PTS of fragment in seconds
 * @param fragmentDuration duration of fragment in seconds
 */
void PrivateInstanceAAMP::PushFragment(MediaType mediaType, char *ptr, size_t len, double fragmentTime, double fragmentDuration)
{
	BlockUntilGstreamerWantsData(NULL, 0, 0);
	SyncBegin();
	mStreamSink->Send(mediaType, ptr, len, fragmentTime, fragmentTime, fragmentDuration);
	SyncEnd();
}


/**
 * @brief Push a media fragment to sink
 * @note Takes ownership of buffer
 * @param mediaType type of fragment
 * @param buffer contains data
 * @param fragmentTime PTS of fragment in seconds
 * @param fragmentDuration duration of fragment in seconds
 */
void PrivateInstanceAAMP::PushFragment(MediaType mediaType, GrowableBuffer* buffer, double fragmentTime, double fragmentDuration)
{
	BlockUntilGstreamerWantsData(NULL, 0, 0);
	SyncBegin();
	mStreamSink->Send(mediaType, buffer, fragmentTime, fragmentTime, fragmentDuration);
	SyncEnd();
}


/**
 * @brief Notifies EOS to sink
 * @param mediaType Type of media
 */
void PrivateInstanceAAMP::EndOfStreamReached(MediaType mediaType)
{
	if (mediaType != eMEDIATYPE_SUBTITLE)
	{
		SyncBegin();
		mStreamSink->EndOfStreamReached(mediaType);
		SyncEnd();
	}
}


/**
 * @brief Get seek base position
 * @retval seek base position
 */
double PrivateInstanceAAMP::GetSeekBase(void)
{
	return seek_pos_seconds;
}


/**
 *   @brief Set playback rate.
 *
 *   @param  rate - Rate of playback.
 *   @param  overshootcorrection - overshoot correction in milliseconds.
 */
void PlayerInstanceAAMP::SetRate(int rate,int overshootcorrection)
{
	AAMPLOG_INFO("%s:%d PLAYER[%d] rate=%d.", __FUNCTION__, __LINE__, aamp->mPlayerId, rate);

	ERROR_STATE_CHECK_VOID();

	if (aamp->mpStreamAbstractionAAMP)
	{
		if (!aamp->mIsIframeTrackPresent && rate != AAMP_NORMAL_PLAY_RATE && rate != 0)
		{
			AAMPLOG_WARN("%s:%d Ignoring trickplay. No iframe tracks in stream", __FUNCTION__, __LINE__);
			return;
		}
		if(!(aamp->mbPlayEnabled) && aamp->pipeline_paused && (AAMP_NORMAL_PLAY_RATE == rate))
		{
			AAMPLOG_WARN("%s:%d PLAYER[%d] Player %s=>%s.", __FUNCTION__, __LINE__, aamp->mPlayerId, STRBGPLAYER, STRFGPLAYER );
			aamp->mbPlayEnabled = true;
			aamp->mStreamSink->Configure(aamp->mVideoFormat, aamp->mAudioFormat, aamp->mpStreamAbstractionAAMP->GetESChangeStatus());
			aamp->mpStreamAbstractionAAMP->StartInjection();
			aamp->mStreamSink->Stream();
			aamp->pipeline_paused = false;
			return;
		}
		bool retValue = true;
		if (rate > 0 && aamp->IsLive() && aamp->mpStreamAbstractionAAMP->IsStreamerAtLivePoint() && aamp->rate >= AAMP_NORMAL_PLAY_RATE)
		{
			AAMPLOG_WARN("%s(): Already at logical live point, hence skipping operation", __FUNCTION__);
			aamp->NotifyOnEnteringLive();
			return;
		}

		// DELIA-39691 If input rate is same as current playback rate, skip duplicate operation
		// Additional check for pipeline_paused is because of 0(PAUSED) -> 1(PLAYING), where aamp->rate == 1.0 in PAUSED state
		if ((!aamp->pipeline_paused && rate == aamp->rate) || (rate == 0 && aamp->pipeline_paused))
		{
			AAMPLOG_WARN("%s(): Already running at playback rate(%d) pipeline_paused(%d), hence skipping set rate for (%d)", __FUNCTION__, aamp->rate, aamp->pipeline_paused, rate);
			return;
		}


		//DELIA-30274  -- Get the trick play to a closer position 
		//Logic adapted 
		// XRE gives fixed overshoot position , not suited for aamp . So ignoring overshoot correction value 
			// instead use last reported posn vs the time player get play command
		// a. During trickplay , last XRE reported position is stored in aamp->mReportProgressPosn
					/// and last reported time is stored in aamp->mReportProgressTime
		// b. Calculate the time delta  from last reported time
		// c. Using this diff , calculate the best/nearest match position (works out 70-80%)
		// d. If time delta is < 100ms ,still last video fragment rendering is not removed ,but position updated very recently
			// So switch last displayed position - NewPosn -= Posn - ((aamp->rate/4)*1000) 
		// e. If time delta is > 950ms , possibility of next frame to come by the time play event is processed . 
			//So go to next fragment which might get displayed
		// f. If none of above ,maintain the last displayed position .
		// 
		// h. TODO (again trial n error) - for 3x/4x , within 1sec there might multiple frame displayed . Can use timedelta to calculate some more near,to be tried

		int  timeDeltaFromProgReport = (aamp_GetCurrentTimeMS() - aamp->mReportProgressTime);

		//Skip this logic for either going to paused to coming out of paused scenarios with HLS
		//What we would like to avoid here is the update of seek_pos_seconds because gstreamer position will report proper position
		//Check for 1.0 -> 0.0 and 0.0 -> 1.0 usecase and avoid below logic
		if (!((aamp->rate == AAMP_NORMAL_PLAY_RATE && rate == 0) || (aamp->pipeline_paused && rate == AAMP_NORMAL_PLAY_RATE)))
		{
			double newSeekPosInSec = -1;
			// when switching from trick to play mode only 
			if(aamp->rate && rate == AAMP_NORMAL_PLAY_RATE && !aamp->pipeline_paused)
			{
				if(timeDeltaFromProgReport > 950) // diff > 950 mSec
				{
					// increment by 1x trickplay frame , next possible displayed frame
					newSeekPosInSec = (aamp->mReportProgressPosn+(aamp->rate*1000))/1000;
				}
				else if(timeDeltaFromProgReport > 100) // diff > 100 mSec
				{
					// Get the last shown frame itself 
					newSeekPosInSec = aamp->mReportProgressPosn/1000;
				}
				else
				{
					// Go little back to last shown frame 
					newSeekPosInSec = (aamp->mReportProgressPosn-(aamp->rate*1000))/1000;
				}

				if (newSeekPosInSec >= 0)
				{
					aamp->seek_pos_seconds = newSeekPosInSec;
				}
				else
				{
					AAMPLOG_WARN("%s:%d new seek_pos_seconds calculated is invalid(%f), discarding it!", __FUNCTION__, __LINE__, newSeekPosInSec);
				}
			}
			else
			{
				// Coming out of pause mode(aamp->rate=0) or when going into pause mode (rate=0)
				// Show the last position 
				aamp->seek_pos_seconds = aamp->GetPositionMilliseconds()/1000;
			}

			aamp->trickStartUTCMS = -1;
		}
		else
		{
			// DELIA-39530 - For 1.0->0.0 and 0.0->1.0 if bPositionQueryEnabled is enabled, GStreamer position query will give proper value
			// Fallback case added for when bPositionQueryEnabled is disabled, since we will be using elapsedTime to calculate position and
			// trickStartUTCMS has to be reset
			if (!gpGlobalConfig->bPositionQueryEnabled)
			{
				aamp->seek_pos_seconds = aamp->GetPositionMilliseconds()/1000;
				aamp->trickStartUTCMS = -1;
			}
		}

		logprintf("aamp_SetRate(%d)overshoot(%d) ProgressReportDelta:(%d) ", rate,overshootcorrection,timeDeltaFromProgReport);
		logprintf("aamp_SetRate Adj position: %f", aamp->seek_pos_seconds); // current position relative to tune time
		logprintf("aamp_SetRate rate(%d)->(%d)", aamp->rate,rate);
		logprintf("aamp_SetRate cur pipeline: %s", aamp->pipeline_paused ? "paused" : "playing");

		if (rate == aamp->rate)
		{ // no change in desired play rate
			if (aamp->pipeline_paused && rate != 0)
			{ // but need to unpause pipeline
				AAMPLOG_INFO("Resuming Playback at Position '%lld'.", aamp->GetPositionMilliseconds());
				aamp->mpStreamAbstractionAAMP->NotifyPlaybackPaused(false);
				retValue = aamp->mStreamSink->Pause(false);
				aamp->NotifyFirstBufferProcessed(); //required since buffers are already cached in paused state
				aamp->pipeline_paused = false;
				aamp->ResumeDownloads();
			}
		}
		else if (rate == 0)
		{
			if (!aamp->pipeline_paused)
			{
				AAMPLOG_INFO("Pausing Playback at Position '%lld'.", aamp->GetPositionMilliseconds());
				aamp->mpStreamAbstractionAAMP->NotifyPlaybackPaused(true);
				aamp->StopDownloads();
				retValue = aamp->mStreamSink->Pause(true);
				aamp->pipeline_paused = true;
			}
		}
		else
		{
			aamp->rate = rate;
			aamp->pipeline_paused = false;
			aamp->ResumeDownloads();
			aamp->TuneHelper(eTUNETYPE_SEEK); // this unpauses pipeline as side effect
		}

		if(retValue)
		{
			aamp->NotifySpeedChanged(aamp->pipeline_paused ? 0 : aamp->rate);
		}
	}
	else
	{
		AAMPLOG_WARN("%s:%d aamp_SetRate not changed, remains in same rate[%d] - mpStreamAbstractionAAMP[%p] state[%d]", __FUNCTION__, __LINE__, aamp->rate, aamp->mpStreamAbstractionAAMP, state);
	}
}

/**
 *   @brief Seek to a time.
 *
 *   @param  ptr - Aamp  instance,
 *           Return true on success
 */
static gboolean  SeekAfterPrepared(gpointer ptr)
{
	PrivateInstanceAAMP* aamp = (PrivateInstanceAAMP*) ptr;
	bool sentSpeedChangedEv = false;
	bool isSeekToLive = false;
	TuneType tuneType = eTUNETYPE_SEEK;
	PrivAAMPState state;
        aamp->GetState(state);
        if( state == eSTATE_ERROR){
                logprintf("%s() operation is not allowed when player in eSTATE_ERROR state !", __FUNCTION__ );\
                return false;
        }

	if (AAMP_SEEK_TO_LIVE_POSITION == aamp->seek_pos_seconds )
	{
		isSeekToLive = true;
		tuneType = eTUNETYPE_SEEKTOLIVE;
	}

	logprintf("aamp_Seek(%f) and seekToLive(%d)", aamp->seek_pos_seconds, isSeekToLive);

	if (isSeekToLive && !aamp->IsLive())
	{
		logprintf("%s:%d - Not live, skipping seekToLive",__FUNCTION__,__LINE__);
		return false;
	}

	if (aamp->IsLive() && aamp->mpStreamAbstractionAAMP && aamp->mpStreamAbstractionAAMP->IsStreamerAtLivePoint())
	{
		double currPositionSecs = aamp->GetPositionMilliseconds() / 1000.00;
		if (isSeekToLive || aamp->seek_pos_seconds >= currPositionSecs)
		{
			logprintf("%s():Already at live point, skipping operation since requested position(%f) >= currPosition(%f) or seekToLive(%d)", __FUNCTION__, aamp->seek_pos_seconds, currPositionSecs, isSeekToLive);
			aamp->NotifyOnEnteringLive();
			return false;
		}
	}

	if (aamp->pipeline_paused)
	{
		// resume downloads and clear paused flag. state change will be done
		// on streamSink configuration.
		logprintf("%s(): paused state, so resume downloads", __FUNCTION__);
		aamp->pipeline_paused = false;
		aamp->ResumeDownloads();
		sentSpeedChangedEv = true;
	}

	if (tuneType == eTUNETYPE_SEEK)
	{
		logprintf("%s(): tune type is SEEK", __FUNCTION__);
	}
	if (aamp->rate != AAMP_NORMAL_PLAY_RATE)
	{
		aamp->rate = AAMP_NORMAL_PLAY_RATE;
		sentSpeedChangedEv = true;
	}
	if (aamp->mpStreamAbstractionAAMP)
	{ // for seek while streaming
		aamp->SetState(eSTATE_SEEKING);
		aamp->TuneHelper(tuneType);
		if (sentSpeedChangedEv)
		{
			aamp->NotifySpeedChanged(aamp->rate);
		}
		else
		{
			aamp->SetState(eSTATE_PLAYING);
		}
	}
	return true;
}
/**
 *   @brief Seek to a time.
 *
 *   @param  secondsRelativeToTuneTime - Seek position for VOD,
 *           relative position from first tune command.
 */
void PlayerInstanceAAMP::Seek(double secondsRelativeToTuneTime)
{
	bool sentSpeedChangedEv = false;
	bool isSeekToLive = false;
	TuneType tuneType = eTUNETYPE_SEEK;

	ERROR_STATE_CHECK_VOID();

	if ((aamp->mMediaFormat == eMEDIAFORMAT_HLS || aamp->mMediaFormat == eMEDIAFORMAT_HLS_MP4) && (eSTATE_INITIALIZING == state)  && aamp->mpStreamAbstractionAAMP)
	{
		logprintf("Seeking to %lf at the middle of tune, no fragments downloaded yet.", secondsRelativeToTuneTime);
		aamp->mpStreamAbstractionAAMP->SeekPosUpdate(secondsRelativeToTuneTime);
	}
	else if (eSTATE_INITIALIZED == state || eSTATE_PREPARING == state)
	{
		logprintf("Seek will be called after preparing the content.");
		aamp->seek_pos_seconds = secondsRelativeToTuneTime ;
		g_idle_add(SeekAfterPrepared, (gpointer)aamp);
	}
	else
	{
		if (secondsRelativeToTuneTime == AAMP_SEEK_TO_LIVE_POSITION)
		{
			isSeekToLive = true;
			tuneType = eTUNETYPE_SEEKTOLIVE;
		}

		logprintf("aamp_Seek(%f) and seekToLive(%d)", secondsRelativeToTuneTime, isSeekToLive);

		if (isSeekToLive && !aamp->IsLive())
		{
			logprintf("%s:%d - Not live, skipping seekToLive",__FUNCTION__,__LINE__);
			return;
		}

		if (aamp->IsLive() && aamp->mpStreamAbstractionAAMP && aamp->mpStreamAbstractionAAMP->IsStreamerAtLivePoint())
		{
			double currPositionSecs = aamp->GetPositionMilliseconds() / 1000.00;
			if (isSeekToLive || secondsRelativeToTuneTime >= currPositionSecs)
			{
				logprintf("%s():Already at live point, skipping operation since requested position(%f) >= currPosition(%f) or seekToLive(%d)", __FUNCTION__, secondsRelativeToTuneTime, currPositionSecs, isSeekToLive);
				aamp->NotifyOnEnteringLive();
				return;
			}
		}

		if (aamp->pipeline_paused)
		{
			// resume downloads and clear paused flag. state change will be done
			// on streamSink configuration.
			logprintf("%s(): paused state, so resume downloads", __FUNCTION__);
			aamp->pipeline_paused = false;
			aamp->ResumeDownloads();
			sentSpeedChangedEv = true;
		}

		if (tuneType == eTUNETYPE_SEEK)
		{
			aamp->seek_pos_seconds = secondsRelativeToTuneTime;
		}
		if (aamp->rate != AAMP_NORMAL_PLAY_RATE)
		{
			aamp->rate = AAMP_NORMAL_PLAY_RATE;
			sentSpeedChangedEv = true;
		}
		if (aamp->mpStreamAbstractionAAMP)
		{ // for seek while streaming
			aamp->SetState(eSTATE_SEEKING);
			aamp->TuneHelper(tuneType);
			if (sentSpeedChangedEv)
			{
				aamp->NotifySpeedChanged(aamp->rate);
			}
			else
			{
				aamp->SetState(eSTATE_PLAYING);
			}
		}
	}
}


/**
 *   @brief Seek to live point.
 */
void PlayerInstanceAAMP::SeekToLive()
{
	Seek(AAMP_SEEK_TO_LIVE_POSITION);
}


/**
 *   @brief Seek to a time and playback with a new rate.
 *
 *   @param  rate - Rate of playback.
 *   @param  secondsRelativeToTuneTime - Seek position for VOD,
 *           relative position from first tune command.
 */
void PlayerInstanceAAMP::SetRateAndSeek(int rate, double secondsRelativeToTuneTime)
{
	ERROR_OR_IDLE_STATE_CHECK_VOID();
	logprintf("aamp_SetRateAndSeek(%d)(%f)", rate, secondsRelativeToTuneTime);
	aamp->TeardownStream(false);
	aamp->seek_pos_seconds = secondsRelativeToTuneTime;
	aamp->rate = rate;
	aamp->TuneHelper(eTUNETYPE_SEEK);
}


/**
 *   @brief Set video rectangle.
 *
 *   @param  x - horizontal start position.
 *   @param  y - vertical start position.
 *   @param  w - width.
 *   @param  h - height.
 */
void PlayerInstanceAAMP::SetVideoRectangle(int x, int y, int w, int h)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetVideoRectangle(x, y, w, h);
}


/**
 *   @brief Set video zoom.
 *
 *   @param  zoom - zoom mode.
 */
void PlayerInstanceAAMP::SetVideoZoom(VideoZoomMode zoom)
{
	ERROR_STATE_CHECK_VOID();
	aamp->zoom_mode = zoom;
	if (aamp->mpStreamAbstractionAAMP ){
		aamp->SetVideoZoom(zoom);
	}else{
		AAMPLOG_WARN("%s:%d Player is in state (%s) , value has been cached",
		__FUNCTION__, __LINE__, "eSTATE_IDLE");
	}
}


/**
 *   @brief Enable/ Disable Video.
 *
 *   @param  muted - true to disable video, false to enable video.
 */
void PlayerInstanceAAMP::SetVideoMute(bool muted)
{
	ERROR_STATE_CHECK_VOID();
	aamp->video_muted = muted;
	if (aamp->mpStreamAbstractionAAMP){
		aamp->SetVideoMute(muted);
	}else{
		AAMPLOG_WARN("%s:%d Player is in state (%s) , value has been cached",
		__FUNCTION__, __LINE__, "eSTATE_IDLE");
	}
}


/**
 *   @brief Set Audio Volume.
 *
 *   @param  volume - Minimum 0, maximum 100.
 */
void PlayerInstanceAAMP::SetAudioVolume(int volume)
{
	ERROR_STATE_CHECK_VOID();
	aamp->audio_volume = volume;
	if (aamp->mpStreamAbstractionAAMP){
		aamp->SetAudioVolume(volume);
	}else{
		AAMPLOG_WARN("%s:%d Player is in state (%s) , value has been cached",
		__FUNCTION__, __LINE__, "eSTATE_IDLE");
	}
}


/**
 *   @brief Set Audio language.
 *
 *   @param  language - Language of audio track.
 */
void PlayerInstanceAAMP::SetLanguage(const char* language)
{
	ERROR_STATE_CHECK_VOID();

	logprintf("aamp_SetLanguage(%s)->(%s)",aamp->language, language);

	// There is no active playback session, save the language for later
	if (state == eSTATE_IDLE || state == eSTATE_RELEASED)
	{
		aamp->languageSetByUser = true;
		aamp->UpdateAudioLanguageSelection(language);
		logprintf("aamp_SetLanguage(%s) Language set prior to tune start", language);
	}
	// check if language is supported in manifest languagelist
	else if((aamp->IsAudioLanguageSupported(language)) || (!aamp->mMaxLanguageCount))
	{
		aamp->languageSetByUser = true;
		aamp->UpdateAudioLanguageSelection(language);
		logprintf("aamp_SetLanguage(%s) Language set", language);
		if (aamp->mpStreamAbstractionAAMP)
		{
			logprintf("aamp_SetLanguage(%s) retuning", language);

			aamp->discardEnteringLiveEvt = true;

			aamp->seek_pos_seconds = aamp->GetPositionMilliseconds()/1000.0;
			aamp->TeardownStream(false);
			aamp->TuneHelper(eTUNETYPE_SEEK);

			aamp->discardEnteringLiveEvt = false;
		}
	}
	else
		logprintf("aamp_SetLanguage(%s) not supported in manifest", language);
}


/**
 *   @brief Set array of subscribed tags.
 *
 *   @param  subscribedTags - Array of subscribed tags.
 */
void PlayerInstanceAAMP::SetSubscribedTags(std::vector<std::string> subscribedTags)
{
	ERROR_STATE_CHECK_VOID();

	logprintf("aamp_SetSubscribedTags()");
	aamp->subscribedTags = subscribedTags;

	for (int i=0; i < aamp->subscribedTags.size(); i++) {
	        logprintf("    subscribedTags[%d] = '%s'", i, subscribedTags.at(i).data());
	}
}

#ifdef SUPPORT_JS_EVENTS 

/**
 *   @brief Load AAMP JS object in the specified JS context.
 *
 *   @param  context - JS context.
 */
void PlayerInstanceAAMP::LoadJS(void* context)
{
	logprintf("[AAMP_JS] %s(%p)", __FUNCTION__, context);
	if (mJSBinding_DL) {
		void(*loadJS)(void*, void*);
		const char* szLoadJS = "aamp_LoadJS";
		loadJS = (void(*)(void*, void*))dlsym(mJSBinding_DL, szLoadJS);
		if (loadJS) {
			logprintf("[AAMP_JS] %s() dlsym(%p, \"%s\")=%p", __FUNCTION__, mJSBinding_DL, szLoadJS, loadJS);
			loadJS(context, this);
		}
	}
}


/**
 *   @brief Unoad AAMP JS object in the specified JS context.
 *
 *   @param  context - JS context.
 */
void PlayerInstanceAAMP::UnloadJS(void* context)
{
	logprintf("[AAMP_JS] %s(%p)", __FUNCTION__, context);
	if (mJSBinding_DL) {
		void(*unloadJS)(void*);
		const char* szUnloadJS = "aamp_UnloadJS";
		unloadJS = (void(*)(void*))dlsym(mJSBinding_DL, szUnloadJS);
		if (unloadJS) {
			logprintf("[AAMP_JS] %s() dlsym(%p, \"%s\")=%p", __FUNCTION__, mJSBinding_DL, szUnloadJS, unloadJS);
			unloadJS(context);
		}
	}
}
#endif


/**
 *   @brief Support multiple listeners for multiple event type
 *
 *   @param  eventType - type of event.
 *   @param  eventListener - listener for the eventType.
 */
void PlayerInstanceAAMP::AddEventListener(AAMPEventType eventType, AAMPEventListener* eventListener)
{
	aamp->AddEventListener(eventType, eventListener);
}


/**
 *   @brief Remove event listener for eventType.
 *
 *   @param  eventType - type of event.
 *   @param  eventListener - listener to be removed for the eventType.
 */
void PlayerInstanceAAMP::RemoveEventListener(AAMPEventType eventType, AAMPEventListener* eventListener)
{
	aamp->RemoveEventListener(eventType, eventListener);
}


/**
 *   @brief To check playlist type.
 *
 *   @return bool - True if live content, false otherwise
 */
bool PlayerInstanceAAMP::IsLive()
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(false);
	return aamp->IsLive();
}


/**
 *   @brief Get current audio language.
 *
 *   @return current audio language
 */
const char* PlayerInstanceAAMP::GetCurrentAudioLanguage(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL("");
	return aamp->language;
}

/**
 *   @brief Get current drm
 *
 *   @return current drm
 */
const char* PlayerInstanceAAMP::GetCurrentDRM(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL("");
	DRMSystems currentDRM = aamp->GetCurrentDRM();
	const char *drmName = "";
	switch(currentDRM)
	{
		case eDRM_WideVine:
			drmName = "Widevine";
			break;
		case eDRM_CONSEC_agnostic:
			drmName = "CONSEC_agnostic";
			break;
		case eDRM_PlayReady:
			drmName = "PlayReady";
			break;
		case eDRM_Adobe_Access:
			drmName = "Adobe_Access";
			break;
		case eDRM_Vanilla_AES:
			drmName = "Vanilla_AES";
			break;
		default:
			break;
	}
	return drmName;
}


/**
 *   @brief Get current drm
 *
 *   @return current drm
 */
DRMSystems PrivateInstanceAAMP::GetCurrentDRM(void)
{
	return mCurrentDrm;
}

/**
 *   @brief Add/Remove a custom HTTP header and value.
 *
 *   @param  headerName - Name of custom HTTP header
 *   @param  headerValue - Value to be passed along with HTTP header.
 *   @param  isLicenseHeader - true if header is for license request
 */
void PlayerInstanceAAMP::AddCustomHTTPHeader(std::string headerName, std::vector<std::string> headerValue, bool isLicenseHeader)
{
	ERROR_STATE_CHECK_VOID();
	aamp->AddCustomHTTPHeader(headerName, headerValue, isLicenseHeader);
}

/**
 *   @brief Set License Server URL.
 *
 *   @param  url - URL of the server to be used for license requests
 *   @param  type - DRM Type(PR/WV) for which the server URL should be used, global by default
 */
void PlayerInstanceAAMP::SetLicenseServerURL(const char *url, DRMSystems type)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetLicenseServerURL(url, type);
}


/**
 *   @brief Indicates if session token has to be used with license request or not.
 *
 *   @param  isAnonymous - True if session token should be blank and false otherwise.
 */
void PlayerInstanceAAMP::SetAnonymousRequest(bool isAnonymous)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetAnonymousRequest(isAnonymous);
}

/**
 *   @brief Indicates average BW to be used for ABR Profiling.
 *
 *   @param  useAvgBW - Flag for true / false
 */
void PlayerInstanceAAMP::SetAvgBWForABR(bool useAvgBW)
{
	aamp->SetAvgBWForABR(useAvgBW);
}

/**
*   @brief SetPreCacheTimeWindow Function to Set PreCache Time
*
*   @param  Time in minutes - Max PreCache Time 
*/
void PlayerInstanceAAMP::SetPreCacheTimeWindow(int nTimeWindow)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetPreCacheTimeWindow(nTimeWindow);
}


/**
 *   @brief Set VOD Trickplay FPS.
 *
 *   @param  vodTrickplayFPS - FPS to be used for VOD Trickplay
 */
void PlayerInstanceAAMP::SetVODTrickplayFPS(int vodTrickplayFPS)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetVODTrickplayFPS(vodTrickplayFPS);
}


/**
 *   @brief Set Linear Trickplay FPS.
 *
 *   @param  linearTrickplayFPS - FPS to be used for Linear Trickplay
 */
void PlayerInstanceAAMP::SetLinearTrickplayFPS(int linearTrickplayFPS)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetLinearTrickplayFPS(linearTrickplayFPS);
}

/**
 *   @brief Set Live Offset.
 *
 *   @param  liveoffset- Live Offset
 */
void PlayerInstanceAAMP::SetLiveOffset(int liveoffset)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetLiveOffset(liveoffset);
}


/**
 *   @brief To set the error code to be used for playback stalled error.
 *
 *   @param  errorCode - error code for playback stall errors.
 */
void PlayerInstanceAAMP::SetStallErrorCode(int errorCode)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetStallErrorCode(errorCode);
}


/**
 *   @brief To set the timeout value to be used for playback stall detection.
 *
 *   @param  timeoutMS - timeout in milliseconds for playback stall detection.
 */
void PlayerInstanceAAMP::SetStallTimeout(int timeoutMS)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetStallTimeout(timeoutMS);
}


/**
 *   @brief Set report interval duration
 *
 *   @param  reportIntervalMS - report interval duration in MS
 */
void PlayerInstanceAAMP::SetReportInterval(int reportIntervalMS)
{
	ERROR_STATE_CHECK_VOID();
	if(reportIntervalMS > 0)
	{
		aamp->SetReportInterval(reportIntervalMS);
	}
}

/**
 *   @brief To set the max retry attempts for init frag curl timeout failures
 *
 *   @param  count - max attempt for timeout retry count
 */
void PlayerInstanceAAMP::SetInitFragTimeoutRetryCount(int count)
{
	if(count >= 0)
	{
		aamp->SetInitFragTimeoutRetryCount(count);
	}
}

/**
 *   @brief To get the current playback position.
 *
 *   @ret current playback position in seconds
 */
double PlayerInstanceAAMP::GetPlaybackPosition()
{
	ERROR_STATE_CHECK_VAL(0.00);
	return (aamp->GetPositionMilliseconds() / 1000.00);
}


/**
*   @brief To get the current asset's duration.
*
*   @ret duration in seconds
*/
double PlayerInstanceAAMP::GetPlaybackDuration()
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(0.00);
	return (aamp->GetDurationMs() / 1000.00);
}


/**
 *   @brief To get the current AAMP state.
 *
 *   @ret current AAMP state
 */
PrivAAMPState PlayerInstanceAAMP::GetState(void)
{
	PrivAAMPState currentState;
	aamp->GetState(currentState);
	return currentState;
}


/**
 *   @brief To get the bitrate of current video profile.
 *
 *   @ret bitrate of video profile
 */
long PlayerInstanceAAMP::GetVideoBitrate(void)
{
	long bitrate = 0;
	ERROR_OR_IDLE_STATE_CHECK_VAL(0);
	if (aamp->mpStreamAbstractionAAMP)
	{
		bitrate = aamp->mpStreamAbstractionAAMP->GetVideoBitrate();
	}
	return bitrate;
}


/**
 *   @brief To set a preferred bitrate for video profile.
 *
 *   @param[in] preferred bitrate for video profile
 */
void PlayerInstanceAAMP::SetVideoBitrate(long bitrate)
{
	ERROR_OR_IDLE_STATE_CHECK_VOID();
	aamp->SetVideoBitrate(bitrate);
}

/**
 *   @brief Set a preferred bitrate for video.
 *
 *   @param[in] preferred bitrate.
 */
void PrivateInstanceAAMP::SetVideoBitrate(long bitrate)
{
	if (bitrate == 0)
	{
		mABREnabled = true;
	}
	else
	{
		mABREnabled = false;
		mUserRequestedBandwidth = bitrate;
	}
}

/**
 *   @brief Get preferred bitrate for video.
 *
 *   @return preferred bitrate.
 */
long PrivateInstanceAAMP::GetVideoBitrate()
{
	return mUserRequestedBandwidth;
}


/**
 *   @brief To get the bitrate of current audio profile.
 *
 *   @ret bitrate of audio profile
 */
long PlayerInstanceAAMP::GetAudioBitrate(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(0);
	return aamp->mpStreamAbstractionAAMP->GetAudioBitrate();
}


/**
 *   @brief To set a preferred bitrate for audio profile.
 *
 *   @param[in] preferred bitrate for audio profile
 */
void PlayerInstanceAAMP::SetAudioBitrate(long bitrate)
{
	//no-op for now
}


/**
 *   @brief To get the current audio volume.
 *
 *   @ret audio volume
 */
int PlayerInstanceAAMP::GetAudioVolume(void)
{
	ERROR_STATE_CHECK_VAL(0);
	if (eSTATE_IDLE == state) 
	{
		AAMPLOG_WARN("%s:%d GetAudioVolume is returning cached value since player is at %s",
		__FUNCTION__, __LINE__,"eSTATE_IDLE");
	}
	return aamp->audio_volume;
}


/**
 *   @brief To get the current playback rate.
 *
 *   @ret current playback rate
 */
int PlayerInstanceAAMP::GetPlaybackRate(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(0);
	return (aamp->pipeline_paused ? 0 : aamp->rate);
}


/**
 *   @brief To get the available video bitrates.
 *
 *   @ret available video bitrates
 */
std::vector<long> PlayerInstanceAAMP::GetVideoBitrates(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(std::vector<long>());
	return aamp->mpStreamAbstractionAAMP->GetVideoBitrates();
}


/**
 *   @brief To get the available audio bitrates.
 *
 *   @ret available audio bitrates
 */
std::vector<long> PlayerInstanceAAMP::GetAudioBitrates(void)
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(std::vector<long>());
	return aamp->mpStreamAbstractionAAMP->GetAudioBitrates();
}


/**
 *   @brief To set the initial bitrate value.
 *
 *   @param[in] initial bitrate to be selected
 */
void PlayerInstanceAAMP::SetInitialBitrate(long bitrate)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetInitialBitrate(bitrate);
	
}


/**
 *   @brief To set the initial bitrate value for 4K assets.
 *
 *   @param[in] initial bitrate to be selected for 4K assets
 */
void PlayerInstanceAAMP::SetInitialBitrate4K(long bitrate4K)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetInitialBitrate4K(bitrate4K);
}

/**
 *   @brief To set the vod-tune-event according to the player.
 *
 *   @param[in] preferred tune event type
 */
void PrivateInstanceAAMP::SetTuneEventConfig( TunedEventConfig tuneEventType)
{
	if(gpGlobalConfig->tunedEventConfigVOD == eTUNED_EVENT_MAX)
	{
		mTuneEventConfigVod = tuneEventType;
	}
	else
	{
		mTuneEventConfigVod = gpGlobalConfig->tunedEventConfigVOD;
	}

	if(gpGlobalConfig->tunedEventConfigLive == eTUNED_EVENT_MAX)
	{
                mTuneEventConfigLive = tuneEventType;
	}
	else
	{
		mTuneEventConfigLive = gpGlobalConfig->tunedEventConfigLive;
	}
}
/**
 *   @brief To set the network download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PlayerInstanceAAMP::SetNetworkTimeout(double timeout)
{
        ERROR_STATE_CHECK_VOID();
        aamp->SetNetworkTimeout(timeout);
}

/**
 *   @brief To set the manifest download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PlayerInstanceAAMP::SetManifestTimeout(double timeout)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetManifestTimeout(timeout);
}

/**
 *   @brief To set the playlist download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PlayerInstanceAAMP::SetPlaylistTimeout(double timeout)
{
        ERROR_STATE_CHECK_VOID();
        aamp->SetPlaylistTimeout(timeout);
}

/**
 *   @brief To set the download buffer size value
 *
 *   @param[in] preferred download buffer size
 */
void PlayerInstanceAAMP::SetDownloadBufferSize(int bufferSize)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetDownloadBufferSize(bufferSize);
}


/**
 *   @brief Set preferred DRM.
 *
 *   @param[in] drmType - preferred DRM type
 */
void PlayerInstanceAAMP::SetPreferredDRM(DRMSystems drmType)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetPreferredDRM(drmType);
}

/**
 *   @brief Set Stereo Only Playback.
 */
void PlayerInstanceAAMP::SetStereoOnlyPlayback(bool bValue)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetStereoOnlyPlayback(bValue);
}


/**
 *   @brief Set BulkTimedMetadata Reporting flag
 */
void PlayerInstanceAAMP::SetBulkTimedMetaReport(bool bValue)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetBulkTimedMetaReport(bValue);
}

/**
 *   @brief Set unpaired discontinuity retune flag
 */
void PlayerInstanceAAMP::SetRetuneForUnpairedDiscontinuity(bool bValue)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetRetuneForUnpairedDiscontinuity(bValue);
}

/**
 *   @brief Setting the alternate contents' (Ads/blackouts) URL.
 *
 *   @param[in] Adbreak's unique identifier.
 *   @param[in] Individual Ad's id
 *   @param[in] Ad URL
 */
void PlayerInstanceAAMP::SetAlternateContents(const std::string &adBreakId, const std::string &adId, const std::string &url)
{
	ERROR_OR_IDLE_STATE_CHECK_VOID();
	aamp->SetAlternateContents(adBreakId, adId, url);
}

/**
 *   @brief To set the network proxy
 *
 *   @param[in] network proxy to use
 */
void PlayerInstanceAAMP::SetNetworkProxy(const char * proxy)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetNetworkProxy(proxy);
}


/**
 *   @brief To set the proxy for license request
 *
 *   @param[in] proxy to use for license request
 */
void PlayerInstanceAAMP::SetLicenseReqProxy(const char * licenseProxy)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetLicenseReqProxy(licenseProxy);
}


/**
 *   @brief To set the curl stall timeout value
 *
 *   @param[in] curl stall timeout
 */
void PlayerInstanceAAMP::SetDownloadStallTimeout(long stallTimeout)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetDownloadStallTimeout(stallTimeout);
}


/**
 *   @brief To set the curl download start timeout value
 *
 *   @param[in] curl download start timeout
 */
void PlayerInstanceAAMP::SetDownloadStartTimeout(long startTimeout)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetDownloadStartTimeout(startTimeout);
}

/**
 *   @brief Set preferred subtitle language.
 *
 *   @param[in]  language - Language of text track.
 *   @return void
 */
void PlayerInstanceAAMP::SetPreferredSubtitleLanguage(const char* language)
{
	ERROR_STATE_CHECK_VOID();
        AAMPLOG_WARN("PlayerInstanceAAMP::%s():%d (%s)->(%s)", __FUNCTION__, __LINE__, aamp->mSubLanguage, language);

	if (strncmp(language, aamp->mSubLanguage, MAX_LANGUAGE_TAG_LENGTH) == 0)
		return;

	
	if (state == eSTATE_IDLE || state == eSTATE_RELEASED)
	{
		aamp->UpdateSubtitleLanguageSelection(language);
		AAMPLOG_WARN("PlayerInstanceAAMP::%s():%d \"%s\" language set prior to tune start", __FUNCTION__, __LINE__, language);
	}
	else
	{
		AAMPLOG_WARN("PlayerInstanceAAMP::%s():%d discard \"%s\" language set, since in the middle of playback", __FUNCTION__, __LINE__, language);
	}
}

/**
 *   @brief Set parallel playlist download config value.
 *   @param[in] bValue - true if a/v playlist to be downloaded in parallel
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetParallelPlaylistDL(bool bValue)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetParallelPlaylistDL(bValue);
}

/**
 *   @brief Set parallel playlist download config value for linear.
 *   @param[in] bValue - true if a/v playlist to be downloaded in parallel during refresh
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetParallelPlaylistRefresh(bool bValue)
{
	ERROR_STATE_CHECK_VOID();
	aamp->SetParallelPlaylistRefresh(bValue);
}


/**
 *   @brief Set Async Tune Configuration
 *   @param[in] bValue - true if async tune enabled
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetAsyncTuneConfig(bool bValue)
{
	aamp->SetAsyncTuneConfig(bValue);
}
/**
 *   @brief Set Async Tune Configuration
 *   @param[in] bValue - true if async tune enabled
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetAsyncTuneConfig(bool bValue)
{
	if(gpGlobalConfig->mAsyncTuneConfig == eUndefinedState)
	{
		mAsyncTuneEnabled = bValue;
	}
	else
	{
		mAsyncTuneEnabled = (bool)gpGlobalConfig->mAsyncTuneConfig;
	}
	AAMPLOG_INFO("%s:%d Async Tune Config : %s ",__FUNCTION__,__LINE__,(mAsyncTuneEnabled)?"True":"False");
}

/**
 *   @brief Get Async Tune configuration
 *
 *   @return bool - true if config set
 */
bool PlayerInstanceAAMP::GetAsyncTuneConfig()
{
        return aamp->GetAsyncTuneConfig();
}

/**
 *   @brief Get Async Tune configuration
 *
 *   @return bool - true if config set 
 */
bool PrivateInstanceAAMP::GetAsyncTuneConfig()
{
        return mAsyncTuneEnabled;
}

/**
 *   @brief Set Westeros sink Configuration
 *   @param[in] bValue - true if westeros sink enabled
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetWesterosSinkConfig(bool bValue)
{
	aamp->SetWesterosSinkConfig(bValue);
}


/**
 *   @brief Configure New ABR Enable/Disable
 *   @param[in] bValue - true if new ABR enabled
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetNewABRConfig(bool bValue)
{
	aamp->SetNewABRConfig(bValue);
}

/**
 *   @brief Set optional preferred language list
 *   @param[in] languageList - string with comma-delimited language list in ISO-639
 *             from most to least preferred. Set NULL to clear current list.
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetPreferredLanguages(const char *languageList)
{
	NOT_IDLE_AND_NOT_RELEASED_STATE_CHECK_VOID();

	aamp->preferredLanguagesString.clear();
	aamp->preferredLanguagesList.clear();

	if(languageList != NULL)
	{
		aamp->preferredLanguagesString = std::string(languageList);
		std::istringstream ss(aamp->preferredLanguagesString);
		std::string lng;
		while(std::getline(ss, lng, ','))
		{
			aamp->preferredLanguagesList.push_back(lng);
			AAMPLOG_INFO("%s:%d: Parsed preferred lang: %s", __FUNCTION__, __LINE__,
					lng.c_str());
		}

		aamp->preferredLanguagesString = std::string(languageList);

		// If user has not yet called SetLanguage(), force to use
		// preferred languages over default language
		if(!aamp->languageSetByUser)
			aamp->noExplicitUserLanguageSelection = true;
	}

	AAMPLOG_INFO("%s:%d: Number of preferred languages: %d", __FUNCTION__, __LINE__,
			aamp->preferredLanguagesList.size());
}

/**
 *   @brief Get current preferred language list
 *
 *   @return  const char* - current comma-delimited language list or NULL if not set
 *
 */
const char* PlayerInstanceAAMP::GetPreferredLanguages()
{
	if(!aamp->preferredLanguagesString.empty())
	{
		return aamp->preferredLanguagesString.c_str();
	}

	return NULL;
}
/*
 *   @brief Configure New AdBreaker Enable/Disable
 *   @param[in] bValue - true if new AdBreaker enabled
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetNewAdBreakerConfig(bool bValue)
{
	aamp->SetNewAdBreakerConfig(bValue);
}



/**
 *   @brief Set Westeros sink Configuration
 *   @param[in] bValue - true if westeros sink enabled
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetWesterosSinkConfig(bool bValue)
{
	if(gpGlobalConfig->mWesterosSinkConfig == eUndefinedState)
	{
		mWesterosSinkEnabled = bValue;
	}
	else
	{
		mWesterosSinkEnabled = (bool)gpGlobalConfig->mWesterosSinkConfig;
	}
	AAMPLOG_INFO("%s:%d Westeros Sink Config : %s ",__FUNCTION__,__LINE__,(mWesterosSinkEnabled)?"True":"False");
}


/**
 *   @brief Sends an ID3 metadata event.
 *
 *   @param[in] data pointer to ID3 metadata
 *   @param[in] length length of ID3 metadata
 */
void PlayerInstanceAAMP::SendId3MetadataEvent(uint8_t* data, int32_t length)
{
	aamp->SendId3MetadataEvent(data, length);
}


/**
 *   @brief Configure New ABR Enable/Disable
 *   @param[in] bValue - true if new ABR enabled
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetNewABRConfig(bool bValue)
{
	if(gpGlobalConfig->abrBufferCheckEnabled == eUndefinedState)
	{
		mABRBufferCheckEnabled = bValue;
	}
	else
	{
		mABRBufferCheckEnabled = (bool)gpGlobalConfig->abrBufferCheckEnabled;
	}
	AAMPLOG_INFO("%s:%d New ABR Config : %s ",__FUNCTION__,__LINE__,(mABRBufferCheckEnabled)?"True":"False");

	// temp code until its enabled in Peacock App - Remove it later.
	if(gpGlobalConfig->useNewDiscontinuity == eUndefinedState)
	{
		mNewAdBreakerEnabled = bValue;
		gpGlobalConfig->hlsAVTrackSyncUsingStartTime = bValue;	
	}
	else
	{
		mNewAdBreakerEnabled = (bool)gpGlobalConfig->useNewDiscontinuity;		
	}
	AAMPLOG_INFO("%s:%d New AdBreaker Config : %s ",__FUNCTION__,__LINE__,(mNewAdBreakerEnabled)?"True":"False");
}

/**
 *   @brief Configure New AdBreaker Enable/Disable
 *   @param[in] bValue - true if new ABR enabled
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetNewAdBreakerConfig(bool bValue)
{
	if(gpGlobalConfig->useNewDiscontinuity == eUndefinedState)
	{
		mNewAdBreakerEnabled = bValue;
		gpGlobalConfig->hlsAVTrackSyncUsingStartTime = bValue;
	}
	else
	{
		mNewAdBreakerEnabled = (bool)gpGlobalConfig->useNewDiscontinuity;
	}
	AAMPLOG_INFO("%s:%d New AdBreaker Config : %s ",__FUNCTION__,__LINE__,(mNewAdBreakerEnabled)?"True":"False");
}


/**
 *   @brief Set video rectangle.
 *
 *   @param  x - horizontal start position.
 *   @param  y - vertical start position.
 *   @param  w - width.
 *   @param  h - height.
 */
void PrivateInstanceAAMP::SetVideoRectangle(int x, int y, int w, int h)
{
	mStreamSink->SetVideoRectangle(x, y, w, h);
}


/**
 *   @brief Set video zoom.
 *
 *   @param  zoom - zoom mode.
 */
void PrivateInstanceAAMP::SetVideoZoom(VideoZoomMode zoom)
{
	mStreamSink->SetVideoZoom(zoom);
}


/**
 *   @brief Enable/ Disable Video.
 *
 *   @param  muted - true to disable video, false to enable video.
 */
void PrivateInstanceAAMP::SetVideoMute(bool muted)
{
	mStreamSink->SetVideoMute(muted);
}


/**
 *   @brief Set Audio Volume.
 *
 *   @param  volume - Minimum 0, maximum 100.
 */
void PrivateInstanceAAMP::SetAudioVolume(int volume)
{
	mStreamSink->SetAudioVolume(volume);
}


/**
 * @brief abort ongoing downloads and returns error on future downloads
 * called while stopping fragment collector thread
 */
void PrivateInstanceAAMP::DisableDownloads(void)
{
	pthread_mutex_lock(&mLock);
	mDownloadsEnabled = false;
	pthread_cond_broadcast(&mDownloadsDisabled);
	pthread_mutex_unlock(&mLock);
}


/**
 * @brief Check if downloads are enabled
 * @retval true if downloads are enabled
 */
bool PrivateInstanceAAMP::DownloadsAreEnabled(void)
{
	return mDownloadsEnabled; // needs mutex protection?
}


/**
 * @brief Enable downloads
 */
void PrivateInstanceAAMP::EnableDownloads()
{
	pthread_mutex_lock(&mLock);
	mDownloadsEnabled = true;
	pthread_mutex_unlock(&mLock);
}


/**
 * @brief Sleep until timeout is reached or interrupted
 * @param timeInMs timeout in milliseconds
 */
void PrivateInstanceAAMP::InterruptableMsSleep(int timeInMs)
{
	if (timeInMs > 0)
	{
		struct timespec ts;
		struct timeval tv;
		int ret;
		gettimeofday(&tv, NULL);
		ts.tv_sec = time(NULL) + timeInMs / 1000;
		ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000));
		ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
		ts.tv_nsec %= (1000 * 1000 * 1000);
		pthread_mutex_lock(&mLock);
		if (mDownloadsEnabled)
		{
			ret = pthread_cond_timedwait(&mDownloadsDisabled, &mLock, &ts);
			if (0 == ret)
			{
				logprintf("sleep interrupted!");
			}
#ifndef WIN32
			else if (ETIMEDOUT != ret)
			{
				logprintf("sleep - condition wait failed %s", strerror(ret));
			}
#endif
		}
		pthread_mutex_unlock(&mLock);
	}
}


/**
 * @brief Get stream duration
 * @retval duration is milliseconds
 */
long long PrivateInstanceAAMP::GetDurationMs()
{
	return (long long)(durationSeconds*1000.0);
}


/**
 * @brief Get current stream position
 * @retval current stream position in ms
 */
long long PrivateInstanceAAMP::GetPositionMs()
{
	return (prevPositionMiliseconds!=-1)?prevPositionMiliseconds:GetPositionMilliseconds();
}


/**
 * @brief Get current stream position
 * @retval current stream position in ms
 */
long long PrivateInstanceAAMP::GetPositionMilliseconds()
{
	long long positionMiliseconds = seek_pos_seconds * 1000.0;
	if (trickStartUTCMS >= 0)
	{
		//DELIA-39530 - Audio only playback is un-tested. Hence disabled for now
		if (gpGlobalConfig->bPositionQueryEnabled && !gpGlobalConfig->bAudioOnlyPlayback)
		{
			positionMiliseconds += mStreamSink->GetPositionMilliseconds();
		}
		else
		{
			long long elapsedTime = aamp_GetCurrentTimeMS() - trickStartUTCMS;
			positionMiliseconds += (((elapsedTime > 1000) ? elapsedTime : 0) * rate);
		}

		if ((-1 != prevPositionMiliseconds) && (AAMP_NORMAL_PLAY_RATE == rate))
		{
			long long diff = positionMiliseconds - prevPositionMiliseconds;

			if ((diff > MAX_DIFF_BETWEEN_PTS_POS_MS) || (diff < 0))
			{
				AAMPLOG_WARN("%s:%d diff %lld prev-pos-ms %lld current-pos-ms %lld, restore prev-pos as current-pos!!", __FUNCTION__, __LINE__, diff, prevPositionMiliseconds, positionMiliseconds);
				positionMiliseconds = prevPositionMiliseconds;
			}
		}

		if (positionMiliseconds < 0)
		{
			AAMPLOG_WARN("%s : Correcting positionMiliseconds %lld to zero", __FUNCTION__, positionMiliseconds);
			positionMiliseconds = 0;
		}
		else if (mpStreamAbstractionAAMP)
		{
			if (!mIsLive)
			{
				long long durationMs  = GetDurationMs();
				if(positionMiliseconds > durationMs)
				{
					AAMPLOG_WARN("%s : Correcting positionMiliseconds %lld to duration %lld", __FUNCTION__, positionMiliseconds, durationMs);
					positionMiliseconds = durationMs;
				}
			}
			else
			{
				long long tsbEndMs = GetDurationMs() + (culledSeconds * 1000.0);
				if(positionMiliseconds > tsbEndMs)
				{
					AAMPLOG_WARN("%s : Correcting positionMiliseconds %lld to tsbEndMs %lld", __FUNCTION__, positionMiliseconds, tsbEndMs);
					positionMiliseconds = tsbEndMs;
				}
			}
		}
	}

	prevPositionMiliseconds = positionMiliseconds;
	return positionMiliseconds;
}

/**
 * @brief Sends media buffer to sink
 * @param mediaType type of media
 * @param ptr buffer containing media data
 * @param len length of buffer
 * @param fpts pts in seconds
 * @param fdts dts in seconds
 * @param fDuration duration of buffer
 */
void PrivateInstanceAAMP::SendStream(MediaType mediaType, const void *ptr, size_t len, double fpts, double fdts, double fDuration)
{
	profiler.ProfilePerformed(PROFILE_BUCKET_FIRST_BUFFER);
	mStreamSink->Send(mediaType, ptr, len, fpts, fdts, fDuration);
}


/**
 * @brief Sends media buffer to sink
 * @note  Ownership of buffer is transferred.
 * @param mediaType type of media
 * @param buffer - media data
 * @param fpts pts in seconds
 * @param fdts dts in seconds
 * @param fDuration duration of buffer
 */
void PrivateInstanceAAMP::SendStream(MediaType mediaType, GrowableBuffer* buffer, double fpts, double fdts, double fDuration)
{
	profiler.ProfilePerformed(PROFILE_BUCKET_FIRST_BUFFER);
	mStreamSink->Send(mediaType, buffer, fpts, fdts, fDuration);
}


/**
 * @brief Set stream sink
 * @param streamSink pointer of sink object
 */
void PrivateInstanceAAMP::SetStreamSink(StreamSink* streamSink)
{
	mStreamSink = streamSink;
}


/**
 * @brief Check if stream is live
 * @retval true if stream is live, false if not
 */
bool PrivateInstanceAAMP::IsLive()
{
	return mIsLive;
}


/**
 * @brief Stop playback and release resources.
 *
 */
void PrivateInstanceAAMP::Stop()
{
	DisableDownloads();
	// Stopping the playback, release all DRM context
	if (mpStreamAbstractionAAMP)
	{
		mpStreamAbstractionAAMP->Stop(true);
	}

	TeardownStream(true);
	pthread_mutex_lock(&mLock);
	if (mPendingAsyncEvents.size() > 0)
	{
		logprintf("PrivateInstanceAAMP::%s() - mPendingAsyncEvents.size - %d", __FUNCTION__, mPendingAsyncEvents.size());
		for (std::map<gint, bool>::iterator it = mPendingAsyncEvents.begin(); it != mPendingAsyncEvents.end(); it++)
		{
			if (it->first != 0)
			{
				if (it->second)
				{
					logprintf("PrivateInstanceAAMP::%s() - remove id - %d", __FUNCTION__, (int) it->first);
					g_source_remove(it->first);
				}
				else
				{
					logprintf("PrivateInstanceAAMP::%s() - Not removing id - %d as not pending", __FUNCTION__, (int) it->first);
				}
			}
		}
		mPendingAsyncEvents.clear();
	}
	if (timedMetadata.size() > 0)
	{
		logprintf("PrivateInstanceAAMP::%s() - timedMetadata.size - %d", __FUNCTION__, timedMetadata.size());
		timedMetadata.clear();
	}

	pthread_mutex_unlock(&mLock);
	seek_pos_seconds = -1;
	culledSeconds = 0;
	durationSeconds = 0;
	rate = 1;
	// Set the state to released as all resources are released for the session
	// directly setting state variable . Calling SetState will trigger event :(
	mState = eSTATE_RELEASED;
	mSeekOperationInProgress = false;
	mMaxLanguageCount = 0; // reset language count
	// send signal to any thread waiting for play
	pthread_mutex_lock(&mMutexPlaystart);
	pthread_cond_broadcast(&waitforplaystart);
	pthread_mutex_unlock(&mMutexPlaystart);
	if(mPreCachePlaylistThreadFlag)
	{
		pthread_join(mPreCachePlaylistThreadId,NULL);
		mPreCachePlaylistThreadFlag=false;
		mPreCachePlaylistThreadId = 0;
	}
	getAampCacheHandler()->StopPlaylistCache();
	if(NULL != mCdaiObject)
	{
		delete mCdaiObject;
		mCdaiObject = NULL;
	}
	if (pipeline_paused)
	{
		pipeline_paused = false;
	}
	EnableDownloads();
}

/**
 * @brief SaveTimedMetadata Function to store Metadata for bulk reporting during Initialization 
 *
 */
void PrivateInstanceAAMP::SaveTimedMetadata(long long timeMilliseconds, const char* szName, const char* szContent, int nb, const char* id, double durationMS)
{
	std::string content(szContent, nb);
	reportMetadata.push_back(TimedMetadata(timeMilliseconds, std::string((szName == NULL) ? "" : szName), content, std::string((id == NULL) ? "" : id), durationMS));
}

/**
 * @brief ReportBulkTimedMetadata Function to send bulk timedMetadata in json format 
 *
 */
void PrivateInstanceAAMP::ReportBulkTimedMetadata()
{
	std::vector<TimedMetadata>::iterator iter;
	if(gpGlobalConfig->enableSubscribedTags && reportMetadata.size())
	{
		AAMPLOG_INFO("%s:%d Sending bulk Timed Metadata",__FUNCTION__,__LINE__);

		cJSON *root;
		cJSON *item;
		root = cJSON_CreateArray();
		if(root)
		{
			for (iter = reportMetadata.begin(); iter != reportMetadata.end(); iter++)
			{
				cJSON_AddItemToArray(root, item = cJSON_CreateObject());
				cJSON_AddStringToObject(item, "name", iter->_name.c_str());
				cJSON_AddStringToObject(item, "id", iter->_id.c_str());
				cJSON_AddNumberToObject(item, "timeMs", iter->_timeMS);
				cJSON_AddNumberToObject (item, "durationMs",iter->_durationMS);
				cJSON_AddStringToObject(item, "data", iter->_content.c_str());
			}

			char* bulkData = cJSON_PrintUnformatted(root);
			if(bulkData)
			{

				AAMPEvent eventData;
				eventData.type = AAMP_EVENT_BULK_TIMED_METADATA;
				eventData.data.bulktimedMetadata.szMetaContent = bulkData;
				AAMPLOG_INFO("%s:%d:: Sending bulkTimedData", __FUNCTION__, __LINE__);
				if (gpGlobalConfig->logging.logMetadata)
				{
					printf("%s:%d:: bulkTimedData : %s\n", __FUNCTION__, __LINE__, bulkData);
				}
				// Sending BulkTimedMetaData event as synchronous event.
				// SCTE35 events are async events in TimedMetadata, and this event is sending only from HLS
				SendEventSync(eventData);
				free(bulkData);
			}
			cJSON_Delete(root);
		}
	}
}


/**
 * @brief Report TimedMetadata events
 * szName should be the tag name and szContent should be tag value, excluding delimiter ":"
 * @param timeMilliseconds time in milliseconds
 * @param szName name of metadata
 * @param szContent  metadata content
 * @param id - Identifier of the TimedMetadata
 * @param bSyncCall - Sync or Async Event
 * @param durationMS - Duration in milliseconds
 * @param nb unused
 */
void PrivateInstanceAAMP::ReportTimedMetadata(long long timeMilliseconds, const char* szName, const char* szContent, int nb,bool bSyncCall, const char* id, double durationMS)
{
	std::string content(szContent, nb);
	bool bFireEvent = false;

	// Check if timedMetadata was already reported
	std::vector<TimedMetadata>::iterator i;
	bool ignoreMetaAdd = false;
	for (i = timedMetadata.begin(); i != timedMetadata.end(); i++)
	{
		if (i->_timeMS < timeMilliseconds)
		{
			continue;
		}

		// Add a boundary check of 1 sec for rounding correction
		if ((timeMilliseconds >= i->_timeMS-1000 && timeMilliseconds <= i->_timeMS+1000 ) &&
			(i->_name.compare(szName) == 0)	&&
			(i->_content.compare(content) == 0))
		{
			// Already same exists , ignore
			ignoreMetaAdd = true;
			break;
		}

		if (i->_timeMS > timeMilliseconds)
		{
			timedMetadata.insert(i, TimedMetadata(timeMilliseconds, szName, content, id, durationMS));
			bFireEvent = true;
			ignoreMetaAdd = true;
			break;
		}
	}

	if(!ignoreMetaAdd && i == timedMetadata.end())
	{
		// Comes here for
		// 1.No entry in the table
		// 2.Entries available which is only having time < NewMetatime
		timedMetadata.push_back(TimedMetadata(timeMilliseconds, szName, content, id, durationMS));
		bFireEvent = true;
	}

	if (bFireEvent)
	{
		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_TIMED_METADATA;
		eventData.data.timedMetadata.timeMilliseconds = timeMilliseconds;
		eventData.data.timedMetadata.durationMilliSeconds = durationMS;

		//Temporary fix for DELIA-37985:
		eventData.additionalEventData.push_back(std::string((szName == NULL) ? "" : szName));
		eventData.additionalEventData.push_back(std::string((id == NULL) ? "" : id));
		eventData.additionalEventData.push_back(std::string(content.c_str()));

		eventData.data.timedMetadata.szName = eventData.additionalEventData[0].c_str();
		eventData.data.timedMetadata.id = eventData.additionalEventData[1].c_str();
		//DELIA-40019: szContent should not contain any tag name and ":" delimiter. This is not checked in JS event listeners
		eventData.data.timedMetadata.szContent = eventData.additionalEventData[2].c_str();

		if (gpGlobalConfig->logging.logMetadata)
		{
			logprintf("aamp timedMetadata: [%ld] '%s'",
				(long)(eventData.data.timedMetadata.timeMilliseconds),
				eventData.data.timedMetadata.szContent);
		}


		if(!strcmp(eventData.data.timedMetadata.szName,"SCTE35") || !bSyncCall )
		{
			SendEventAsync(eventData);
		}
		else
		{
			SendEventSync(eventData);
		}
	}
}

#ifdef AAMP_HARVEST_SUPPORT_ENABLED

/**
 * @brief Check if harvest is required
 * @param modifyCount true to decrement harvest value
 * @retval true if harvest is required
 */
bool PrivateInstanceAAMP::HarvestFragments(bool modifyCount)
{
	if (gpGlobalConfig->harvest)
	{
		logprintf("aamp harvest: %d", gpGlobalConfig->harvest);
		if(modifyCount)
		{
			gpGlobalConfig->harvest--;
			if(!gpGlobalConfig->harvest)
			{
				logprintf("gpGlobalConfig->harvest zero, no more harvesting");
			}
		}
		return true;
	}
	return false;
}
#endif

/**
 * @brief Notify first frame is displayed. Sends CC handle event to listeners.
 */
void PrivateInstanceAAMP::NotifyFirstFrameReceived()
{
	SetState(eSTATE_PLAYING);
	pthread_mutex_lock(&mMutexPlaystart);
	pthread_cond_broadcast(&waitforplaystart);
	pthread_mutex_unlock(&mMutexPlaystart);

	TunedEventConfig tunedEventConfig = IsLive() ? mTuneEventConfigLive : mTuneEventConfigVod;
	if (eTUNED_EVENT_ON_GST_PLAYING == tunedEventConfig)
	{
		// This is an idle callback, so we can sent event synchronously
		if (SendTunedEvent())
		{
			logprintf("aamp: - sent tune event on Tune Completion.");
		}
	}
#ifdef AAMP_STOP_SINK_ON_SEEK
	/*Do not send event on trickplay as CC is not enabled*/
	if (AAMP_NORMAL_PLAY_RATE != rate)
	{
		logprintf("PrivateInstanceAAMP::%s:%d : not sending cc handle as rate = %f", __FUNCTION__, __LINE__, rate);
		return;
	}
#endif
	if (mStreamSink != NULL)
	{
		AAMPEvent event;
		event.type = AAMP_EVENT_CC_HANDLE_RECEIVED;
		event.data.ccHandle.handle = mStreamSink->getCCDecoderHandle();
		SendEventSync(event);
	}
}

/**
 * @brief Signal discontinuity of track.
 * Called from StreamAbstractionAAMP to signal discontinuity
 * @param track MediaType of the track
 * @retval true if discontinuity is handled.
 */
bool PrivateInstanceAAMP::Discontinuity(MediaType track)
{
	bool ret;
	SyncBegin();
	ret = mStreamSink->Discontinuity(track);
	SyncEnd();
	if (ret)
	{
		mProcessingDiscontinuity[track] = true;
	}
	return ret;
}


/**
 * @brief Tune again to currently viewing asset. Used for internal error handling
 * @param ptr pointer to PrivateInstanceAAMP object
 * @retval G_SOURCE_REMOVE
 */
static gboolean PrivateInstanceAAMP_Retune(gpointer ptr)
{
	PrivateInstanceAAMP* aamp = (PrivateInstanceAAMP*) ptr;
	bool activeAAMPFound = false;
	bool reTune = false;
	gActivePrivAAMP_t *gAAMPInstance = NULL;
	pthread_mutex_lock(&gMutex);
	for (std::list<gActivePrivAAMP_t>::iterator iter = gActivePrivAAMPs.begin(); iter != gActivePrivAAMPs.end(); iter++)
	{
		if (aamp == iter->pAAMP)
		{
			gAAMPInstance = &(*iter);
			activeAAMPFound = true;
			reTune = gAAMPInstance->reTune;
			break;
		}
	}
	if (!activeAAMPFound)
	{
		logprintf("PrivateInstanceAAMP::%s : %p not in Active AAMP list", __FUNCTION__, aamp);
	}
	else if (!reTune)
	{
		logprintf("PrivateInstanceAAMP::%s : %p reTune flag not set", __FUNCTION__, aamp);
	}
	else
	{
		aamp->mIsRetuneInProgress = true;
		pthread_mutex_unlock(&gMutex);

		aamp->TuneHelper(eTUNETYPE_RETUNE);

		pthread_mutex_lock(&gMutex);
		aamp->mIsRetuneInProgress = false;
		gAAMPInstance->reTune = false;
		pthread_cond_signal(&gCond);
	}
	pthread_mutex_unlock(&gMutex);
	return G_SOURCE_REMOVE;
}


/**
 * @brief Schedules retune or discontinuity processing based on state.
 * @param errorType type of playback error
 * @param trackType media type
 */
void PrivateInstanceAAMP::ScheduleRetune(PlaybackErrorType errorType, MediaType trackType)
{
	if (AAMP_NORMAL_PLAY_RATE == rate && ContentType_EAS != mContentType)
	{
		PrivAAMPState state;
		GetState(state);
		if (((state != eSTATE_PLAYING) && (eGST_ERROR_VIDEO_BUFFERING != errorType)) || mSeekOperationInProgress)
		{
			logprintf("PrivateInstanceAAMP::%s:%d: Not processing reTune since state = %d, mSeekOperationInProgress = %d",
						__FUNCTION__, __LINE__, state, mSeekOperationInProgress);
			return;
		}

		/*If underflow is caused by a discontinuity processing, continue playback from discontinuity*/
		if (IsDiscontinuityProcessPending())
		{
			pthread_mutex_lock(&mLock);
			if (mDiscontinuityTuneOperationId != 0 || mDiscontinuityTuneOperationInProgress)
			{
				pthread_mutex_unlock(&mLock);
				logprintf("PrivateInstanceAAMP::%s:%d: Discontinuity Tune handler already spawned(%d) or inprogress(%d)",
					__FUNCTION__, __LINE__, mDiscontinuityTuneOperationId, mDiscontinuityTuneOperationInProgress);
				return;
			}
			mDiscontinuityTuneOperationId = g_idle_add(PrivateInstanceAAMP_ProcessDiscontinuity, (gpointer) this);
			pthread_mutex_unlock(&mLock);

			logprintf("PrivateInstanceAAMP::%s:%d: Underflow due to discontinuity handled", __FUNCTION__, __LINE__);
			return;
		}
		else if (mpStreamAbstractionAAMP->IsStreamerStalled())
		{
			logprintf("PrivateInstanceAAMP::%s:%d: Ignore reTune due to playback stall", __FUNCTION__, __LINE__);
			return;
		}
		else if (!gpGlobalConfig->internalReTune)
		{
			logprintf("PrivateInstanceAAMP::%s:%d: Ignore reTune as disabled in configuration", __FUNCTION__, __LINE__);
			return;
		}

		if((gpGlobalConfig->reportBufferEvent) && (errorType == eGST_ERROR_UNDERFLOW) && (trackType == eMEDIATYPE_VIDEO))
		{
			SendBufferChangeEvent(true);  // Buffer state changed, buffer Under flow started
			if ( false == pipeline_paused )
			{
				if ( true != PausePipeline(true) )
				{
					AAMPLOG_ERR("%s(): Failed to pause the Pipeline", __FUNCTION__);
				}
			}
		}

		const char* errorString  =  (errorType == eGST_ERROR_PTS) ? "PTS ERROR" :
									(errorType == eGST_ERROR_UNDERFLOW) ? "Underflow" :
									(errorType == eSTALL_AFTER_DISCONTINUITY) ? "Stall After Discontinuity" : "STARTTIME RESET";

		SendAnomalyEvent(ANOMALY_WARNING, "%s %s", (trackType == eMEDIATYPE_VIDEO ? "VIDEO" : "AUDIO"), errorString);
		bool activeAAMPFound = false;
		pthread_mutex_lock(&gMutex);
		for (std::list<gActivePrivAAMP_t>::iterator iter = gActivePrivAAMPs.begin(); iter != gActivePrivAAMPs.end(); iter++)
		{
			if (this == iter->pAAMP)
			{
				gActivePrivAAMP_t *gAAMPInstance = &(*iter);
				if (gAAMPInstance->reTune)
				{
					logprintf("PrivateInstanceAAMP::%s:%d: Already scheduled", __FUNCTION__, __LINE__);
				}
				else
				{

					if(eGST_ERROR_PTS == errorType || eGST_ERROR_UNDERFLOW == errorType)
					{
						long long now = aamp_GetCurrentTimeMS();
						long long lastErrorReportedTimeMs = lastUnderFlowTimeMs[trackType];
						if (lastErrorReportedTimeMs)
						{
							long long diffMs = (now - lastErrorReportedTimeMs);
							if (diffMs < AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS)
							{
								gAAMPInstance->numPtsErrors++;
								logprintf("PrivateInstanceAAMP::%s:%d: numPtsErrors %d, ptsErrorThreshold %d",
									__FUNCTION__, __LINE__, gAAMPInstance->numPtsErrors, gpGlobalConfig->ptsErrorThreshold);
								if (gAAMPInstance->numPtsErrors >= gpGlobalConfig->ptsErrorThreshold)
								{
									gAAMPInstance->numPtsErrors = 0;
									gAAMPInstance->reTune = true;
									logprintf("PrivateInstanceAAMP::%s:%d: Schedule Retune. diffMs %lld < threshold %lld",
										__FUNCTION__, __LINE__, diffMs, AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS);
									g_idle_add(PrivateInstanceAAMP_Retune, (gpointer)this);
								}
							}
							else
							{
								gAAMPInstance->numPtsErrors = 0;
								logprintf("PrivateInstanceAAMP::%s:%d: Not scheduling reTune since (diff %lld > threshold %lld) numPtsErrors %d, ptsErrorThreshold %d.",
									__FUNCTION__, __LINE__, diffMs, AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS,
									gAAMPInstance->numPtsErrors, gpGlobalConfig->ptsErrorThreshold);
							}
						}
						else
						{
							gAAMPInstance->numPtsErrors = 0;
							logprintf("PrivateInstanceAAMP::%s:%d: Not scheduling reTune since first %s.", __FUNCTION__, __LINE__, errorString);
						}
						lastUnderFlowTimeMs[trackType] = now;
					}
					else
					{
						logprintf("PrivateInstanceAAMP::%s:%d: Schedule Retune errorType %d error %s", __FUNCTION__, __LINE__, errorType, errorString);
						gAAMPInstance->reTune = true;
						g_idle_add(PrivateInstanceAAMP_Retune, (gpointer) this);
					}
				}
				activeAAMPFound = true;
				break;
			}
		}
		pthread_mutex_unlock(&gMutex);
		if (!activeAAMPFound)
		{
			logprintf("PrivateInstanceAAMP::%s:%d: %p not in Active AAMP list", __FUNCTION__, __LINE__, this);
		}
	}
}


/**
 * @brief PrivateInstanceAAMP Constructor
 */
PrivateInstanceAAMP::PrivateInstanceAAMP() : mAbrBitrateData(), mLock(), mMutexAttr(),
	mpStreamAbstractionAAMP(NULL), mInitSuccess(false), mVideoFormat(FORMAT_INVALID), mAudioFormat(FORMAT_INVALID), mDownloadsDisabled(),
	mDownloadsEnabled(true), mStreamSink(NULL), profiler(), licenceFromManifest(false), previousAudioType(eAUDIO_UNKNOWN),
	mbDownloadsBlocked(false), streamerIsActive(false), mTSBEnabled(false), mIscDVR(false), mLiveOffset(AAMP_LIVE_OFFSET), mNewLiveOffsetflag(false),
	fragmentCollectorThreadID(0), seek_pos_seconds(-1), rate(0), pipeline_paused(false), mMaxLanguageCount(0), zoom_mode(VIDEO_ZOOM_FULL),
	video_muted(false), audio_volume(100), subscribedTags(), timedMetadata(), IsTuneTypeNew(false), trickStartUTCMS(-1),
	playStartUTCMS(0), durationSeconds(0.0), culledSeconds(0.0), maxRefreshPlaylistIntervalSecs(DEFAULT_INTERVAL_BETWEEN_PLAYLIST_UPDATES_MS/1000), initialTuneTimeMs(0),
	mEventListener(NULL), mReportProgressPosn(0.0), mReportProgressTime(0), discardEnteringLiveEvt(false),
	mIsRetuneInProgress(false), mCondDiscontinuity(), mDiscontinuityTuneOperationId(0), mIsVSS(false),
	m_fd(-1), mIsLive(false), mTuneCompleted(false), mFirstTune(true), mfirstTuneFmt(-1), mTuneAttempts(0), mPlayerLoadTime(0),
	mState(eSTATE_RELEASED), mMediaFormat(eMEDIAFORMAT_HLS), mCurrentDrm(eDRM_NONE), mPersistedProfileIndex(0), mAvailableBandwidth(0),
	mDiscontinuityTuneOperationInProgress(false), mContentType(), mTunedEventPending(false),
	mSeekOperationInProgress(false), mPendingAsyncEvents(), mCustomHeaders(),
	mManifestUrl(""), mTunedManifestUrl(""), mServiceZone(),
	mCurrentLanguageIndex(0), noExplicitUserLanguageSelection(true), languageSetByUser(false), preferredLanguagesString(), preferredLanguagesList(),
	mVideoEnd(NULL),mTimeToTopProfile(0),mTimeAtTopProfile(0),mPlaybackDuration(0),mTraceUUID(),
	mIsFirstRequestToFOG(false), mIsLocalPlayback(false), mABREnabled(false), mUserRequestedBandwidth(0), mNetworkProxy(NULL), mLicenseProxy(NULL),mTuneType(eTUNETYPE_NEW_NORMAL)
	,mCdaiObject(NULL), mAdEventsQ(),mAdEventQMtx(), mAdPrevProgressTime(0), mAdCurOffset(0), mAdDuration(0), mAdProgressId("")
	,mLastDiscontinuityTimeMs(0), mBufUnderFlowStatus(false), mVideoBasePTS(0)
#ifdef PLACEMENT_EMULATION
	,mNumAds2Place(0), sampleAdBreakId("")
#endif
	,mCustomLicenseHeaders(), mIsIframeTrackPresent(false), mManifestTimeoutMs(-1), mNetworkTimeoutMs(-1)
	,mBulkTimedMetadata(false), reportMetadata(), mbPlayEnabled(true), mPlayerId(PLAYERID_CNTR++),mAampCacheHandler(new AampCacheHandler())
	,mAsyncTuneEnabled(false), mWesterosSinkEnabled(false), mEnableRectPropertyEnabled(true), waitforplaystart()
	,mTuneEventConfigLive(eTUNED_EVENT_ON_PLAYLIST_INDEXED), mTuneEventConfigVod(eTUNED_EVENT_ON_PLAYLIST_INDEXED)
	,mUseAvgBandwidthForABR(false), mParallelFetchPlaylistRefresh(true), mParallelFetchPlaylist(false)
	,mRampDownLimit(-1), mMinBitrate(0), mMaxBitrate(LONG_MAX), mSegInjectFailCount(MAX_SEG_INJECT_FAIL_COUNT), mDrmDecryptFailCount(MAX_SEG_DRM_DECRYPT_FAIL_COUNT)
	,mPlaylistTimeoutMs(-1)
	,mMutexPlaystart()
#ifdef AAMP_HLS_DRM
    , fragmentCdmEncrypted(false) ,drmParserMutex(), aesCtrAttrDataList()
	, drmSessionThreadStarted(false), createDRMSessionThreadID(0)
#endif
	, mPlayermode(PLAYERMODE_JSPLAYER)
	, mPreCachePlaylistThreadId(0)
	, mPreCachePlaylistThreadFlag(false)
	, mPreCacheDnldList()
	, mPreCacheDnldTimeWindow(0)
	, mReportProgressInterval(DEFAULT_REPORT_PROGRESS_INTERVAL)
#if defined(AAMP_MPD_DRM) || defined(AAMP_HLS_DRM)
	, mDRMSessionManager(NULL)
#endif
	, mParallelPlaylistFetchLock()
	, mAppName()
	, mABRBufferCheckEnabled(false)
	, mNewAdBreakerEnabled(false)
	, mProgressReportFromProcessDiscontinuity(false)
	, mUseRetuneForUnpairedDiscontinuity(true)
	, prevPositionMiliseconds(-1)
	, mInitFragmentRetryCount(-1)
	, mPlaylistFetchFailError(0L)
{
	LazilyLoadConfigIfNeeded();
#if defined(AAMP_MPD_DRM) || defined(AAMP_HLS_DRM)
	mDRMSessionManager = new AampDRMSessionManager();
#endif
	pthread_cond_init(&mDownloadsDisabled, NULL);
	strcpy(language,"en");
    iso639map_NormalizeLanguageCode( language, GetLangCodePreference() );
    
	memset(mSubLanguage, '\0', MAX_LANGUAGE_TAG_LENGTH);
	strncpy(mSubLanguage, gpGlobalConfig->mSubtitleLanguage.c_str(), MAX_LANGUAGE_TAG_LENGTH - 1);
	pthread_mutexattr_init(&mMutexAttr);
	pthread_mutexattr_settype(&mMutexAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mLock, &mMutexAttr);
	pthread_mutex_init(&mParallelPlaylistFetchLock, &mMutexAttr);


	for (int i = 0; i < eCURLINSTANCE_MAX; i++)
	{
		curl[i] = NULL;
		//cookieHeaders[i].clear();
		httpRespHeaders[i].type = eHTTPHEADERTYPE_UNKNOWN;
		httpRespHeaders[i].data.clear();
		curlDLTimeout[i] = 0;
	}
	for (int i = 0; i < AAMP_MAX_NUM_EVENTS; i++)
	{
		mEventListeners[i] = NULL;
	}

	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		mbTrackDownloadsBlocked[i] = false;
		mTrackInjectionBlocked[i] = false;
		lastUnderFlowTimeMs[i] = 0;
		mProcessingDiscontinuity[i] = false;
	}

	pthread_mutex_lock(&gMutex);
	gActivePrivAAMP_t gAAMPInstance = { this, false, 0 };
	gActivePrivAAMPs.push_back(gAAMPInstance);
	pthread_mutex_unlock(&gMutex);
	discardEnteringLiveEvt = false;
	licenceFromManifest = false;
	mTunedEventPending = false;
	mPendingAsyncEvents.clear();

	// Add Connection: Keep-Alive custom header - DELIA-26832
	mCustomHeaders["Connection:"] = std::vector<std::string> { "Keep-Alive" };
	pthread_cond_init(&mCondDiscontinuity, NULL);
	pthread_cond_init(&waitforplaystart, NULL);
	pthread_mutex_init(&mMutexPlaystart, NULL);
	mABREnabled = gpGlobalConfig->bEnableABR;
	mUserRequestedBandwidth = gpGlobalConfig->defaultBitrate;
	mNetworkProxy = NULL;
	mLicenseProxy = NULL;
	mCdaiObject = NULL;
	mAdPrevProgressTime = 0;
	mAdProgressId = "";
	SetAsyncTuneConfig(false);
	if(gpGlobalConfig->rampdownLimit >= 0)
	{
		mRampDownLimit = gpGlobalConfig->rampdownLimit;
	}
	if(gpGlobalConfig->minBitrate > 0)
	{
		mMinBitrate = gpGlobalConfig->minBitrate;
	}
	if(gpGlobalConfig->maxBitrate > 0)
	{
		mMaxBitrate = gpGlobalConfig->maxBitrate;
	}
	if(gpGlobalConfig->segInjectFailCount > 0)
	{
		mSegInjectFailCount = gpGlobalConfig->segInjectFailCount;
	}
	if(gpGlobalConfig->drmDecryptFailCount > 0)
	{
		mDrmDecryptFailCount = gpGlobalConfig->drmDecryptFailCount;
	}
	if(gpGlobalConfig->abrBufferCheckEnabled != eUndefinedState)
		mABRBufferCheckEnabled = (bool)gpGlobalConfig->abrBufferCheckEnabled;
	if(gpGlobalConfig->useNewDiscontinuity != eUndefinedState)
		mNewAdBreakerEnabled	= (bool)gpGlobalConfig->useNewDiscontinuity;
#ifdef AAMP_HLS_DRM
	memset(&aesCtrAttrDataList, 0, sizeof(aesCtrAttrDataList));
	pthread_mutex_init(&drmParserMutex, NULL);
#endif
}


/**
 * @brief PrivateInstanceAAMP Destructor
 */
PrivateInstanceAAMP::~PrivateInstanceAAMP()
{
	pthread_mutex_lock(&gMutex);
	for (std::list<gActivePrivAAMP_t>::iterator iter = gActivePrivAAMPs.begin(); iter != gActivePrivAAMPs.end(); iter++)
	{
		if (this == iter->pAAMP)
		{
			gActivePrivAAMPs.erase(iter);
			break;
		}
	}
	pthread_mutex_unlock(&gMutex);

	pthread_mutex_lock(&mLock);
	for (int i = 0; i < AAMP_MAX_NUM_EVENTS; i++)
	{
		while (mEventListeners[i] != NULL)
		{
			ListenerData* pListener = mEventListeners[i];
			mEventListeners[i] = pListener->pNext;
			delete pListener;
		}
	}

	if (mNetworkProxy)
	{
		free(mNetworkProxy);
	}

	if (mLicenseProxy)
	{
		free(mLicenseProxy);
	}

	if (mVideoEnd)
	{
		delete mVideoEnd;
          	mVideoEnd = NULL;
	}
	pthread_mutex_unlock(&mLock);

	pthread_cond_destroy(&mDownloadsDisabled);
	pthread_cond_destroy(&mCondDiscontinuity);
	pthread_cond_destroy(&waitforplaystart);
	pthread_mutex_destroy(&mMutexPlaystart);
	pthread_mutex_destroy(&mLock);
	pthread_mutex_destroy(&mParallelPlaylistFetchLock);
#ifdef AAMP_HLS_DRM
	aesCtrAttrDataList.clear();
	pthread_mutex_destroy(&drmParserMutex);
#endif
	delete mAampCacheHandler;
#if defined(AAMP_MPD_DRM) || defined(AAMP_HLS_DRM)
	delete mDRMSessionManager;
#endif
}


/**
 * @brief Sets aamp state
 * @param state state to be set
 */
void PrivateInstanceAAMP::SetState(PrivAAMPState state)
{
	if (mState == state)
	{ // noop
		return;
	}

	if( state == eSTATE_PLAYING && mState == eSTATE_SEEKING )
	{
		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_SEEKED;
		eventData.data.seeked.positionMiliseconds = GetPositionMilliseconds();
		SendEventSync(eventData);
	}

	pthread_mutex_lock(&mLock);
	mState = state;
	pthread_mutex_unlock(&mLock);

	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_STATE_CHANGED])
	{
		if (mState == eSTATE_PREPARING)
		{
			AAMPEvent eventData;
			eventData.type = AAMP_EVENT_STATE_CHANGED;
			eventData.data.stateChanged.state = eSTATE_INITIALIZED;
			SendEventSync(eventData);
		}

		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_STATE_CHANGED;
		eventData.data.stateChanged.state = mState;
		SendEventSync(eventData);
	}
}


/**
 * @brief Get aamp state
 * @param[out] state current state of aamp
 */
void PrivateInstanceAAMP::GetState(PrivAAMPState& state)
{
	pthread_mutex_lock(&mLock);
	state = mState;
	pthread_mutex_unlock(&mLock);
}


/**
 * @brief Add idle task
 * @note task shall return 0 to be removed, 1 to be repeated
 * @param task task function pointer
 * @param arg passed as parameter during idle task execution
 */
void PrivateInstanceAAMP::AddIdleTask(IdleTask task, void* arg)
{
	g_idle_add(task, (gpointer)arg);
}


/**
 * @brief Add high priority idle task
 *
 * @note task shall return 0 to be removed, 1 to be repeated
 *
 * @param[in] task task function pointer
 * @param[in] arg passed as parameter during idle task execution
 */
gint PrivateInstanceAAMP::AddHighIdleTask(IdleTask task, void* arg,DestroyTask dtask)
{
	gint callbackID = g_idle_add_full(G_PRIORITY_HIGH_IDLE, task, (gpointer)arg, dtask);
	return callbackID;
}

/**
 * @brief Check if sink cache is empty
 * @param mediaType type of track
 * @retval true if sink cache is empty
 */
bool PrivateInstanceAAMP::IsSinkCacheEmpty(MediaType mediaType)
{
	return mStreamSink->IsCacheEmpty(mediaType);
}

/**
 * @brief Notification on completing fragment caching
 */
void PrivateInstanceAAMP::NotifyFragmentCachingComplete()
{
	mStreamSink->NotifyFragmentCachingComplete();
}


/**
 * @brief Send tuned event to listeners if required
 * @retval true if event is scheduled, false if discarded
 */
bool PrivateInstanceAAMP::SendTunedEvent(bool isSynchronous)
{
	bool ret = false;

	// Required for synchronising btw audio and video tracks in case of cdmidecryptor
	pthread_mutex_lock(&mLock);

	ret = mTunedEventPending;
	mTunedEventPending = false;

	pthread_mutex_unlock(&mLock);

	if(ret)
	{
		if (isSynchronous)
		{
			SendEventSync(AAMP_EVENT_TUNED);
		}
		else
		{
			SendEventAsync(AAMP_EVENT_TUNED);
		}
	}
	return ret;
}

/**
 *   @brief Send VideoEndEvent
 *
 *   @return success or failure
 */
bool PrivateInstanceAAMP::SendVideoEndEvent()
{
	bool ret = false;
	char * strVideoEndJson = NULL;
	// Required for protecting mVideoEnd object
	pthread_mutex_lock(&mLock);
	if(mVideoEnd)
	{
		//Update VideoEnd Data
		if(mTimeAtTopProfile > 0)
		{
			// Losing milisecons of data in conversion from double to long
			mVideoEnd->SetTimeAtTopProfile(mTimeAtTopProfile);
			mVideoEnd->SetTimeToTopProfile(mTimeToTopProfile);
		}
		mVideoEnd->SetTotalDuration(mPlaybackDuration);

		// re initialize for next tune collection
		mTimeToTopProfile = 0;
		mTimeAtTopProfile = 0;
		mPlaybackDuration = 0;
        mCurrentLanguageIndex = 0;

		//Memory of this string will be deleted after sending event by destructor of AsyncMetricsEventDescriptor
		strVideoEndJson = mVideoEnd->ToJsonString();

		delete mVideoEnd;
		
		if(strVideoEndJson)
		{
			AAMPLOG_INFO("VideoEnd:%s", strVideoEndJson);
		}
	}
	
	mVideoEnd = new CVideoStat();
#ifdef USE_OPENCDM // AampOutputProtection is compiled when this  flag is enabled 
	//Collect Display resoluation and store in videoEndObject, TBD: If DisplayResolution changes during playback, its not taken care. not in scope for now. 
	int iDisplayWidth = 0 , iDisplayHeight = 0;
	AampOutputProtection::GetAampOutputProcectionInstance()->GetDisplayResolution(iDisplayWidth,iDisplayHeight);
	mVideoEnd->SetDisplayResolution(iDisplayWidth,iDisplayHeight);
#endif 
	pthread_mutex_unlock(&mLock);

	if(strVideoEndJson)
	{
		if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_REPORT_METRICS_DATA])
		{
			AsyncMetricsEventDescriptor* e = new AsyncMetricsEventDescriptor(MetricsDataType::AAMP_DATA_VIDEO_END,strVideoEndJson,this->mTraceUUID);

			ScheduleEvent(e);
			ret = true;
		}
		else
		{
			free(strVideoEndJson);
		}
	}
	
	return ret;

}

    /**   @brief updates  profile Resolution to VideoStat object
    *
    *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
    *   @param[in]  bitrate - bitrate ( bits per sec )
    *   @param[in]  width - Frame width
    *   @param[in]  Height - Frame Height
    *   @return void
    */
    void PrivateInstanceAAMP::UpdateVideoEndProfileResolution(MediaType mediaType, long bitrate, int width, int height)
    {
        if(gpGlobalConfig->mEnableVideoEndEvent) // avoid mutex mLock lock if disabled.
        {
            pthread_mutex_lock(&mLock);
            if(mVideoEnd)
            {
                VideoStatTrackType trackType = VideoStatTrackType::STAT_VIDEO;
                if(mediaType == eMEDIATYPE_IFRAME)
                {
                    trackType = VideoStatTrackType::STAT_IFRAME;
                }
                mVideoEnd->SetProfileResolution(trackType,bitrate,width,height);
            }
            pthread_mutex_unlock(&mLock);
        }
    }
/**
 *   @brief updates download metrics to VideoStat object, this is used for VideoFragment as it takes duration for calcuation purpose.
 *
 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
 *   @param[in]  bitrate - bitrate ( bits per sec )
 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
 *   @param[in]  strUrl :  URL in case of faulures
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, std::string& strUrl, double duration)
{
    UpdateVideoEndMetrics(mediaType, bitrate, curlOrHTTPCode, strUrl,duration,false,false);
}

/**
 *   @brief updates time shift buffer status
 *
 *   @param[in]  btsbAvailable - true if TSB supported
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndTsbStatus(bool btsbAvailable)
{
	if(gpGlobalConfig->mEnableVideoEndEvent) // avoid mutex mLock lock if disabled.
	{
		pthread_mutex_lock(&mLock);
		if(mVideoEnd)
		{

			mVideoEnd->SetTsbStatus(btsbAvailable);
		}
		pthread_mutex_unlock(&mLock);
	}
}
    

/**
 *   @brief updates download metrics to VideoStat object, this is used for VideoFragment as it takes duration for calcuation purpose.
 *
 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
 *   @param[in]  bitrate - bitrate ( bits per sec )
 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
 *   @param[in]  strUrl :  URL in case of faulures
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, std::string& strUrl, double duration, bool keyChanged, bool isEncrypted)
{
	if(gpGlobalConfig->mEnableVideoEndEvent)
	{
		AAMPLOG_INFO("UpdateVideoEnd:T:%d  br:%ld err:%d dur:%f taTop:%f ttTop:%f tot:%f keyChan:%d encry:%d",
			mediaType, bitrate , curlOrHTTPCode,(float)duration,(float)mTimeAtTopProfile , (float) mTimeToTopProfile, (float) mPlaybackDuration,keyChanged,isEncrypted );

		// ignore for write and aborted errors
		// these are generated after trick play options,
		if( curlOrHTTPCode > 0 &&  !(curlOrHTTPCode == CURLE_ABORTED_BY_CALLBACK || curlOrHTTPCode == CURLE_WRITE_ERROR) )
		{


			VideoStatDataType dataType = VideoStatDataType::VE_DATA_UNKNOWN;

			VideoStatTrackType trackType = VideoStatTrackType::STAT_UNKNOWN;
			VideoStatCountType eCountType = VideoStatCountType::COUNT_UNKNOWN;

		/*	COUNT_UNKNOWN,
			COUNT_LIC_TOTAL,
			COUNT_LIC_ENC_TO_CLR,
			COUNT_LIC_CLR_TO_ENC,
			COUNT_STALL,
			COUNT_4XX,
			COUNT_5XX,
			COUNT_CURL, // all other curl errors except timeout
			COUNT_CURL_TIMEOUT,
			COUNT_SUCCESS*/

			if (curlOrHTTPCode < 100)
			{

				if( curlOrHTTPCode == CURLE_OPERATION_TIMEDOUT)
				{
					eCountType = COUNT_CURL_TIMEOUT;
				}
				else
				{
					eCountType = COUNT_CURL;
				}
			}
			else if (curlOrHTTPCode == 200 || curlOrHTTPCode == 206)
			{
				//success
				eCountType = COUNT_SUCCESS;
			}
			else if (curlOrHTTPCode >= 500 )
			{
				eCountType = COUNT_5XX;
			}
			else // everything else is 4XX
			{
				eCountType = COUNT_4XX;
			}


			switch(mediaType)
			{
				case eMEDIATYPE_MANIFEST:
				{
					dataType = VideoStatDataType::VE_DATA_MANIFEST;
					trackType = VideoStatTrackType::STAT_MAIN;
				}
					break;

				case eMEDIATYPE_PLAYLIST_VIDEO:
				{
					dataType = VideoStatDataType::VE_DATA_MANIFEST;
					trackType = VideoStatTrackType::STAT_VIDEO;
				}
					break;

				case eMEDIATYPE_PLAYLIST_AUDIO:
				{
					dataType = VideoStatDataType::VE_DATA_MANIFEST;
					trackType = ConvertAudioIndexToVideoStatTrackType(mCurrentLanguageIndex);
				}
					break;

				case eMEDIATYPE_PLAYLIST_IFRAME:
				{
					dataType = VideoStatDataType::VE_DATA_MANIFEST;
					trackType = STAT_IFRAME;
				}
					break;

				case eMEDIATYPE_VIDEO:
				{
					dataType = VideoStatDataType::VE_DATA_FRAGMENT;
					trackType = VideoStatTrackType::STAT_VIDEO;
					// always Video fragment will be from same thread so mutex required

	// !!!!!!!!!! To Do : Support this stats for Audio Only streams !!!!!!!!!!!!!!!!!!!!!
					//Is success
					if (eCountType == COUNT_SUCCESS  && duration > 0)
					{
						long maxBitrateSupported = mpStreamAbstractionAAMP->GetMaxBitrate();
						if(maxBitrateSupported == bitrate)
						{
							mTimeAtTopProfile += duration;
						}

						if(mTimeAtTopProfile == 0) // we havent achived top profile yet
						{
							mTimeToTopProfile += duration; // started at top profile
						}

						mPlaybackDuration += duration;
					}

				}
					break;
				case eMEDIATYPE_AUDIO:
				{
					dataType = VideoStatDataType::VE_DATA_FRAGMENT;
					trackType = ConvertAudioIndexToVideoStatTrackType(mCurrentLanguageIndex);
				}
					break;
				case eMEDIATYPE_IFRAME:
				{
					dataType = VideoStatDataType::VE_DATA_FRAGMENT;
					trackType = VideoStatTrackType::STAT_IFRAME;
				}
					break;

				case eMEDIATYPE_INIT_IFRAME:
				{
					dataType = VideoStatDataType::VE_DATA_INIT_FRAGMENT;
					trackType = VideoStatTrackType::STAT_IFRAME;
				}
					break;

				case eMEDIATYPE_INIT_VIDEO:
				{
					dataType = VideoStatDataType::VE_DATA_INIT_FRAGMENT;
					trackType = VideoStatTrackType::STAT_VIDEO;
				}
					break;

				case eMEDIATYPE_INIT_AUDIO:
				{
					dataType = VideoStatDataType::VE_DATA_INIT_FRAGMENT;
					trackType = ConvertAudioIndexToVideoStatTrackType(mCurrentLanguageIndex);
				}
					break;
				default:
					break;
			}


			// Required for protecting mVideoStat object
			if( dataType != VideoStatDataType::VE_DATA_UNKNOWN
					&& trackType != VideoStatTrackType::STAT_UNKNOWN
					&& eCountType != VideoStatCountType::COUNT_UNKNOWN )
			{
				pthread_mutex_lock(&mLock);
				if(mVideoEnd)
				{
					mVideoEnd->Increment_Data(dataType,trackType,eCountType,bitrate);
					if(eCountType != COUNT_SUCCESS && strUrl.c_str())
					{
						//set failure url
						mVideoEnd->SetFailedFragmentUrl(trackType,bitrate,strUrl);
					}
					if(dataType == VideoStatDataType::VE_DATA_FRAGMENT)
					{
						mVideoEnd->Record_License_EncryptionStat(trackType,isEncrypted,keyChanged);
					}
				}
				pthread_mutex_unlock(&mLock);

			}
			else
			{
				AAMPLOG_INFO("PrivateInstanceAAMP::%s - Could Not update VideoEnd Event dataType:%d trackType:%d eCountType:%d", __FUNCTION__,
						dataType,trackType,eCountType);
			}
		}
	}
}


/**
 *   @brief updates abr metrics to VideoStat object,
 *
 *   @param[in]  AAMPAbrInfo - abr info
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(AAMPAbrInfo & info)
{
	if(gpGlobalConfig->mEnableVideoEndEvent)
	{
		//only for Ramp down case
		if(info.desiredProfileIndex < info.currentProfileIndex)
		{
			AAMPLOG_INFO("UpdateVideoEnd:abrinfo currIdx:%d desiredIdx:%d for:%d",  info.currentProfileIndex,info.desiredProfileIndex,info.abrCalledFor);

			if(info.abrCalledFor == AAMPAbrType::AAMPAbrBandwidthUpdate)
			{
				pthread_mutex_lock(&mLock);
				if(mVideoEnd)
				{
					mVideoEnd->Increment_AbrNetworkDropCount();
				}
				pthread_mutex_unlock(&mLock);
			}
			else if (info.abrCalledFor == AAMPAbrType::AAMPAbrFragmentDownloadFailed
					|| info.abrCalledFor == AAMPAbrType::AAMPAbrFragmentDownloadFailed)
			{
				pthread_mutex_lock(&mLock);
				if(mVideoEnd)
				{
					mVideoEnd->Increment_AbrErrorDropCount();
				}
				pthread_mutex_unlock(&mLock);
			}
		}
	}
}

/**
 *   @brief Get available audio tracks.
 *
 *   @return std::string JSON formatted list of audio tracks
 */
std::string PlayerInstanceAAMP::GetAvailableAudioTracks()
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(std::string());

	return aamp->GetAvailableAudioTracks();
}

/**
 *   @brief Get available text tracks.
 *
 *   @return std::string JSON formatted list of text tracks
 */
std::string PlayerInstanceAAMP::GetAvailableTextTracks()
{
	ERROR_OR_IDLE_STATE_CHECK_VAL(std::string());

	return aamp->GetAvailableTextTracks();
}

/*
 *   @brief Get the video window co-ordinates
 *
 *   @return current video co-ordinates in x,y,w,h format
 */
std::string PlayerInstanceAAMP::GetVideoRectangle()
{
	ERROR_STATE_CHECK_VAL(std::string());

	return aamp->GetVideoRectangle();
}

/*
 *   @brief Set the application name which has created PlayerInstanceAAMP, for logging purposes
 *
 *   @return void
 */
void PlayerInstanceAAMP::SetAppName(std::string name)
{
	aamp->SetAppName(name);
}

/**
 *   @brief updates download metrics to VideoEnd object,
 *
 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
 *   @param[in]  bitrate - bitrate
 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
 *   @param[in]  strUrl :  URL in case of faulures
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, std::string& strUrl)
{
	UpdateVideoEndMetrics(mediaType, bitrate, curlOrHTTPCode, strUrl,0,false,false);
}

/**
 * @brief Check if fragment Buffering is required before playing.
 *
 * @retval true if buffering is required.
 */
bool PrivateInstanceAAMP::IsFragmentBufferingRequired()
{
	return mpStreamAbstractionAAMP->IsFragmentBufferingRequired();
}


/**
 * @brief Get video display's width and height
 * @param width
 * @param height
 */
void PrivateInstanceAAMP::GetPlayerVideoSize(int &width, int &height)
{
	mStreamSink->GetVideoSize(width, height);
}


/**
 * @brief Set an idle callback to dispatched state
 * @param id Idle task Id
 */
void PrivateInstanceAAMP::SetCallbackAsDispatched(gint id)
{
	pthread_mutex_lock(&mLock);
	std::map<gint, bool>::iterator  itr = mPendingAsyncEvents.find(id);
	if(itr != mPendingAsyncEvents.end())
	{
		assert (itr->second);
		mPendingAsyncEvents.erase(itr);
	}
	else
	{
		logprintf("%s:%d id not in mPendingAsyncEvents, insert and mark as not pending", __FUNCTION__, __LINE__, id);
		mPendingAsyncEvents[id] = false;
	}
	pthread_mutex_unlock(&mLock);
}


/**
 * @brief Set an idle callback to pending state
 * @param id Idle task Id
 */
void PrivateInstanceAAMP::SetCallbackAsPending(gint id)
{
	pthread_mutex_lock(&mLock);
	std::map<gint, bool>::iterator  itr = mPendingAsyncEvents.find(id);
	if(itr != mPendingAsyncEvents.end())
	{
		assert (!itr->second);
		logprintf("%s:%d id already in mPendingAsyncEvents and completed, erase it", __FUNCTION__, __LINE__, id);
		mPendingAsyncEvents.erase(itr);
	}
	else
	{
		mPendingAsyncEvents[id] = true;
	}
	pthread_mutex_unlock(&mLock);
}


/**
 *   @brief Add/Remove a custom HTTP header and value.
 *
 *   @param  headerName - Name of custom HTTP header
 *   @param  headerValue - Value to be pased along with HTTP header.
 *   @param  isLicenseHeader - true, if header is to be used for a license request.
 */
void PrivateInstanceAAMP::AddCustomHTTPHeader(std::string headerName, std::vector<std::string> headerValue, bool isLicenseHeader)
{
	// Header name should be ending with :
	if(headerName.back() != ':')
	{
		headerName += ':';
	}

	if (isLicenseHeader)
	{
		if (headerValue.size() != 0)
		{
			mCustomLicenseHeaders[headerName] = headerValue;
		}
		else
		{
			mCustomLicenseHeaders.erase(headerName);
		}
	}
	else
	{
		if (headerValue.size() != 0)
		{
			mCustomHeaders[headerName] = headerValue;
		}
		else
		{
			mCustomHeaders.erase(headerName);
		}
	}
}

/**
 *   @brief Set License Server URL.
 *
 *   @param  url - URL of the server to be used for license requests
 *   @param  type - DRM Type(PR/WV) for which the server URL should be used, global by default
 */
void PrivateInstanceAAMP::SetLicenseServerURL(const char *url, DRMSystems type)
{
	char **serverUrl = &(gpGlobalConfig->licenseServerURL);
	if (type == eDRM_MAX_DRMSystems)
	{
		// Local aamp.cfg config trumps JS PP config
		if (gpGlobalConfig->licenseServerLocalOverride)
		{
			return;
		}
	}
	else if (type == eDRM_PlayReady)
	{
		serverUrl = &(gpGlobalConfig->prLicenseServerURL);
	}
	else if (type == eDRM_WideVine)
	{
		serverUrl = &(gpGlobalConfig->wvLicenseServerURL);
	}
	else if(type == eDRM_ClearKey)
	{
		serverUrl = &(gpGlobalConfig->ckLicenseServerURL);
	}
	else
	{
		AAMPLOG_ERR("PrivateInstanceAAMP::%s - invalid drm type received.", __FUNCTION__);
		return;
	}

	AAMPLOG_INFO("PrivateInstanceAAMP::%s - set license url - %s for type - %d", __FUNCTION__, url, type);
	if (*serverUrl != NULL)
	{
		free(*serverUrl);
	}
	*serverUrl = strdup(url);
}


/**
 *   @brief Indicates if session token has to be used with license request or not.
 *
 *   @param  isAnonymous - True if session token should be blank and false otherwise.
 */
void PrivateInstanceAAMP::SetAnonymousRequest(bool isAnonymous)
{
	gpGlobalConfig->licenseAnonymousRequest = isAnonymous;
}


/**
 *   @brief Indicates average BW to be used for ABR Profiling.
 *
 *   @param  useAvgBW - Flag for true / false
 */
void PrivateInstanceAAMP::SetAvgBWForABR(bool useAvgBW)
{
	mUseAvgBandwidthForABR = useAvgBW;
}

/**
 *   @brief Set Max TimeWindow for PreCaching Playlist
 *
 *   @param  maxTime - Time for PreCaching in Minutes
 */
void PrivateInstanceAAMP::SetPreCacheTimeWindow(int nTimeWindow)
{
	if(nTimeWindow > 0)
	{
		mPreCacheDnldTimeWindow = nTimeWindow;
		AAMPLOG_WARN("%s Playlist PreCaching enabled with timewindow:%d",__FUNCTION__,nTimeWindow);
	}
}


/**
 *   @brief Set VOD Trickplay FPS.
 *
 *   @param  vodTrickplayFPS - FPS to be used for VOD Trickplay
 */
void PrivateInstanceAAMP::SetVODTrickplayFPS(int vodTrickplayFPS)
{
	// Local aamp.cfg config trumps JS PP config
	if (gpGlobalConfig->vodTrickplayFPSLocalOverride)
	{
		return;
	}

	gpGlobalConfig->vodTrickplayFPS = vodTrickplayFPS;
	logprintf("PrivateInstanceAAMP::%s(), vodTrickplayFPS %d", __FUNCTION__, vodTrickplayFPS);
}


/**
 *   @brief Set Linear Trickplay FPS.
 *
 *   @param  linearTrickplayFPS - FPS to be used for Linear Trickplay
 */
void PrivateInstanceAAMP::SetLinearTrickplayFPS(int linearTrickplayFPS)
{
	// Local aamp.cfg config trumps JS PP config
	if (gpGlobalConfig->linearTrickplayFPSLocalOverride)
	{
		return;
	}

	gpGlobalConfig->linearTrickplayFPS = linearTrickplayFPS;
	logprintf("PrivateInstanceAAMP::%s(), linearTrickplayFPS %d", __FUNCTION__, linearTrickplayFPS);
}


/**
 *   @brief Set live offset [Sec]
 *
 *   @param SetLiveOffset - Live Offset
 */
void PrivateInstanceAAMP::SetLiveOffset(int liveoffset)
{
	if(liveoffset > 0 )
        {
		mLiveOffset = liveoffset;
		mNewLiveOffsetflag = true;
		logprintf("PrivateInstanceAAMP::%s(), liveoffset %d", __FUNCTION__, liveoffset);
        }
	else
        {
		logprintf("PrivateInstanceAAMP::%s(), liveoffset beyond limits %d", __FUNCTION__, liveoffset);
        }  
}


/**
 *   @brief To set the error code to be used for playback stalled error.
 *
 *   @param  errorCode - error code for playback stall errors.
 */
void PrivateInstanceAAMP::SetStallErrorCode(int errorCode)
{
	gpGlobalConfig->stallErrorCode = errorCode;
}


/**
 *   @brief To set the timeout value to be used for playback stall detection.
 *
 *   @param  timeoutMS - timeout in milliseconds for playback stall detection.
 */
void PrivateInstanceAAMP::SetStallTimeout(int timeoutMS)
{
	gpGlobalConfig->stallTimeoutInMS = timeoutMS;
}

/**
 *	 @brief To set the Playback Position reporting interval.
 *
 *	 @param  reportIntervalMS - playback reporting interval in milliseconds.
 */
void PrivateInstanceAAMP::SetReportInterval(int reportIntervalMS)
{
	if(gpGlobalConfig->reportProgressInterval != 0)
	{
		mReportProgressInterval = gpGlobalConfig->reportProgressInterval;
	}
	else
	{
		mReportProgressInterval = reportIntervalMS;
	}
	AAMPLOG_WARN("%s Progress Interval configured %d",__FUNCTION__,mReportProgressInterval);		
}

/**
 *   @brief To set the max retry attempts for init frag curl timeout failures
 *
 *   @param  count - max attempt for timeout retry count
 */
void PrivateInstanceAAMP::SetInitFragTimeoutRetryCount(int count)
{
	if(-1 == gpGlobalConfig->initFragmentRetryCount)
	{
		mInitFragmentRetryCount = count;
		AAMPLOG_WARN("%s Init frag timeout retry count configured %d", __FUNCTION__, mInitFragmentRetryCount);
	}
}

/**
 * @brief Send stalled event to listeners
 */
void PrivateInstanceAAMP::SendStalledErrorEvent()
{
	char description[MAX_ERROR_DESCRIPTION_LENGTH];
	memset(description, '\0', MAX_ERROR_DESCRIPTION_LENGTH);
	snprintf(description, MAX_ERROR_DESCRIPTION_LENGTH - 1, "Playback has been stalled for more than %d ms", gpGlobalConfig->stallTimeoutInMS);
	SendErrorEvent(AAMP_TUNE_PLAYBACK_STALLED, description);
}

/**
 * @brief Notifiy first buffer is processed
 */
void PrivateInstanceAAMP::NotifyFirstBufferProcessed()
{
	trickStartUTCMS = aamp_GetCurrentTimeMS();
}


/**
 * @brief Update audio language selection
 * @param lang string corresponding to language
 */
void PrivateInstanceAAMP::UpdateAudioLanguageSelection(const char *lang)
{
	strncpy(language, lang, MAX_LANGUAGE_TAG_LENGTH);
	language[MAX_LANGUAGE_TAG_LENGTH-1] = '\0';
	noExplicitUserLanguageSelection = false;

	for (int cnt=0; cnt < mMaxLanguageCount; cnt ++)
	{
		if(strncmp(mLanguageList[cnt],language,MAX_LANGUAGE_TAG_LENGTH) == 0)
		{
			mCurrentLanguageIndex = cnt; // needed?
			break;
		}
	}
}


/**
 * @brief Update subtitle language selection
 * @param lang string corresponding to language
 */
void PrivateInstanceAAMP::UpdateSubtitleLanguageSelection(const char *lang)
{
	strncpy(mSubLanguage, lang, MAX_LANGUAGE_TAG_LENGTH);
	language[MAX_LANGUAGE_TAG_LENGTH-1] = '\0';
}

/**
 * @brief Get current stream type
 * @retval 10 - HLS/Clear
 * @retval 11 - HLS/Consec
 * @retval 12 - HLS/Access
 * @retval 13 - HLS/Vanilla AES
 * @retval 20 - DASH/Clear
 * @retval 21 - DASH/WV
 * @retval 22 - DASH/PR
 */
int PrivateInstanceAAMP::getStreamType()
{

	int type;

	if(mMediaFormat == eMEDIAFORMAT_DASH)
	{
		type = 20;
	}
	else if( mMediaFormat == eMEDIAFORMAT_HLS)
	{
		type = 10;
	}
	else if (mMediaFormat == eMEDIAFORMAT_PROGRESSIVE)// eMEDIAFORMAT_PROGRESSIVE
	{
		type = 30;
	}
	else if (mMediaFormat == eMEDIAFORMAT_HLS_MP4)
	{
		type = 40;
	}
	else
	{
		type = 0;
	}

	switch(mCurrentDrm)
	{
		case eDRM_WideVine:
		case eDRM_CONSEC_agnostic:
			type += 1;	// 11 or 21
			break;
		case eDRM_PlayReady:
		case eDRM_Adobe_Access:
			type += 2;	// 12 or 22
			break;
		case eDRM_Vanilla_AES:
			type += 3;	// 13
			break;
		default:
			break; //Clear
	}
	return type;
}

#ifdef AAMP_MPD_DRM
/**
 * @brief GetMoneyTraceString - Extracts / Generates MoneyTrace string 
 * @param[out] customHeader - Generated moneytrace is stored  
 *
 * @retval None
*/
void PrivateInstanceAAMP::GetMoneyTraceString(std::string &customHeader)
{
	char moneytracebuf[512];
	memset(moneytracebuf, 0, sizeof(moneytracebuf));

	if (mCustomHeaders.size() > 0)
	{
		for (std::unordered_map<std::string, std::vector<std::string>>::iterator it = mCustomHeaders.begin();
			it != mCustomHeaders.end(); it++)
		{
			if (it->first.compare("X-MoneyTrace:") == 0)
			{
				if (it->second.size() >= 2)
				{
					snprintf(moneytracebuf, sizeof(moneytracebuf), "trace-id=%s;parent-id=%s;span-id=%lld",
					(const char*)it->second.at(0).c_str(),
					(const char*)it->second.at(1).c_str(),
					aamp_GetCurrentTimeMS());
				}
				else if (it->second.size() == 1)
				{
					snprintf(moneytracebuf, sizeof(moneytracebuf), "trace-id=%s;parent-id=%lld;span-id=%lld",
						(const char*)it->second.at(0).c_str(),
						aamp_GetCurrentTimeMS(),
						aamp_GetCurrentTimeMS());
				}
				customHeader.append(moneytracebuf);
				break;
			}
		}
	}
	// No money trace is available in customheader from JS , create a new moneytrace locally
	if(customHeader.size() == 0)
	{
		// No Moneytrace info available in tune data 
		logprintf("No Moneytrace info available in tune request,need to generate one");
		uuid_t uuid;
		uuid_generate(uuid);
		char uuidstr[128];
		uuid_unparse(uuid, uuidstr);
		for (char *ptr = uuidstr; *ptr; ++ptr) {
			*ptr = tolower(*ptr);
		}
		snprintf(moneytracebuf,sizeof(moneytracebuf),"trace-id=%s;parent-id=%lld;span-id=%lld",uuidstr,aamp_GetCurrentTimeMS(),aamp_GetCurrentTimeMS());
		customHeader.append(moneytracebuf);
	}	
	AAMPLOG_TRACE("[GetMoneyTraceString] MoneyTrace[%s]",customHeader.c_str());
}
#endif /* AAMP_MPD_DRM */

/**
 * @brief Send tuned event if configured to sent after decryption
 */
void PrivateInstanceAAMP::NotifyFirstFragmentDecrypted()
{
	if(mTunedEventPending)
	{
		TunedEventConfig tunedEventConfig =  IsLive() ? mTuneEventConfigLive : mTuneEventConfigVod;
		if (eTUNED_EVENT_ON_FIRST_FRAGMENT_DECRYPTED == tunedEventConfig)
		{
			// For HLS - This is invoked by fetcher thread, so we have to sent asynchronously
			if (SendTunedEvent(false))
			{
				logprintf("aamp: %s - sent tune event after first fragment fetch and decrypt\n", mMediaFormatName[mMediaFormat]);
			}
		}
	}
}

/**
 *   @brief  Get PTS of first sample.
 *
 *   @return PTS of first sample
 */
double PrivateInstanceAAMP::GetFirstPTS()
{
	assert(NULL != mpStreamAbstractionAAMP);
	return mpStreamAbstractionAAMP->GetFirstPTS();
}

/**
 *   @brief Check if Live Adjust is required for current content. ( For "vod/ivod/ip-dvr/cdvr/eas", Live Adjust is not required ).
 *
 *   @return False if the content is either vod/ivod/cdvr/ip-dvr/eas
 */
bool PrivateInstanceAAMP::IsLiveAdjustRequired()
{
	bool retValue;

	switch (mContentType)
	{
		case ContentType_IVOD:
		case ContentType_VOD:
		case ContentType_CDVR:
		case ContentType_IPDVR:
		case ContentType_EAS:
			retValue = false;
			break;

		default:
			retValue = true;
			break;
	}

	return retValue;
}

/**
 *   @brief  Generate media metadata event based on args passed.
 *
 *   @param[in] durationMs - duration of playlist in milliseconds
 *   @param[in] langList - list of audio language available in asset
 *   @param[in] bitrateList - list of video bitrates available in asset
 *   @param[in] hasDrm - indicates if asset is encrypted/clear
 *   @param[in] isIframeTrackPresent - indicates if iframe tracks are available in asset
 */
void PrivateInstanceAAMP::SendMediaMetadataEvent(double durationMs, std::set<std::string>langList, std::vector<long> bitrateList, bool hasDrm, bool isIframeTrackPresent)
{
	AAMPEvent event;
	std::vector<int> supportedPlaybackSpeeds { -64, -32, -16, -4, -1, 0, 1, 4, 16, 32, 64 };
	int langCount = 0;
	int bitrateCount = 0;
	int supportedSpeedCount = 0;

	event.type = AAMP_EVENT_MEDIA_METADATA;
	event.data.metadata.durationMiliseconds = durationMs;
	memset(event.data.metadata.bitrates, 0, sizeof(event.data.metadata.bitrates));
	memset(event.data.metadata.supportedSpeeds, 0, sizeof(event.data.metadata.supportedSpeeds));

	for (std::set<std::string>::iterator iter = langList.begin();
			(iter != langList.end() && langCount < MAX_LANGUAGE_COUNT) ; iter++)
	{
		char *dst = event.data.metadata.languages[langCount];
		const char *src = (*iter).c_str();
		size_t len = strlen(src);
		if( len>0 )
		{
			assert( len<MAX_LANGUAGE_TAG_LENGTH-1 );
			memcpy( dst, src, len );
			dst[len] = 0x00;
			langCount++;
		}
	}
	event.data.metadata.languageCount = langCount;
	StoreLanguageList(langCount, event.data.metadata.languages);

	for (int i = 0; (i < bitrateList.size() && bitrateCount < MAX_BITRATE_COUNT); i++)
	{
		event.data.metadata.bitrates[bitrateCount++] = bitrateList[i];
	}
	event.data.metadata.bitrateCount = bitrateCount;
	event.data.metadata.width = 1280;
	event.data.metadata.height = 720;
	GetPlayerVideoSize(event.data.metadata.width, event.data.metadata.height);
	event.data.metadata.hasDrm = hasDrm;

	//Iframe track present and hence playbackRate change is supported
	if (isIframeTrackPresent)
	{
		for(int i = 0; i < supportedPlaybackSpeeds.size() && supportedSpeedCount < MAX_SUPPORTED_SPEED_COUNT; i++)
		{
			event.data.metadata.supportedSpeeds[supportedSpeedCount++] = supportedPlaybackSpeeds[i];
		}
	}
	else
	{
		//Supports only pause and play
		event.data.metadata.supportedSpeeds[supportedSpeedCount++] = 0;
		event.data.metadata.supportedSpeeds[supportedSpeedCount++] = 1;
	}
	event.data.metadata.supportedSpeedCount = supportedSpeedCount;

	logprintf("aamp: sending metadata event and duration update %f", ((double)durationMs)/1000);
	SendEventAsync(event);
	//TODO: Send the list of available audio tracks and text tracks if listeners are registered.
}

/**
 *   @brief  Generate supported speeds changed event based on arg passed.
 *
 *   @param[in] isIframeTrackPresent - indicates if iframe tracks are available in asset
 */
void PrivateInstanceAAMP::SendSupportedSpeedsChangedEvent(bool isIframeTrackPresent)
{
	AAMPEvent event;
	std::vector<int> supportedPlaybackSpeeds { -64, -32, -16, -4, -1, 0, 1, 4, 16, 32, 64 };
	int supportedSpeedCount = 0;

	event.type = AAMP_EVENT_SPEEDS_CHANGED;
	//Iframe track present and hence playbackRate change is supported
	if (isIframeTrackPresent)
	{
		for(int i = 0; i < supportedPlaybackSpeeds.size() && supportedSpeedCount < MAX_SUPPORTED_SPEED_COUNT; i++)
		{
			event.data.speedsChanged.supportedSpeeds[supportedSpeedCount++] = supportedPlaybackSpeeds[i];
		}
	}
	else
	{
		//Supports only pause and play
		event.data.speedsChanged.supportedSpeeds[supportedSpeedCount++] = 0;
		event.data.speedsChanged.supportedSpeeds[supportedSpeedCount++] = 1;
	}
	event.data.speedsChanged.supportedSpeedCount = supportedSpeedCount;

	logprintf("aamp: sending supported speeds changed event with count %d", supportedSpeedCount);
	SendEventAsync(event);
}


/**
 *   @brief To set the initial bitrate value.
 *
 *   @param[in] initial bitrate to be selected
 */
void PrivateInstanceAAMP::SetInitialBitrate(long bitrate)
{
	if (bitrate > 0)
	{
		gpGlobalConfig->defaultBitrate = bitrate;
	}
}


/**
 *   @brief To set the initial bitrate value for 4K assets.
 *
 *   @param[in] initial bitrate to be selected for 4K assets
 */
void PrivateInstanceAAMP::SetInitialBitrate4K(long bitrate4K)
{
	if (bitrate4K > 0)
	{
		gpGlobalConfig->defaultBitrate4K = bitrate4K;
	}
}


/**
 *   @brief To set the network download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PrivateInstanceAAMP::SetNetworkTimeout(double timeout)
{
	if(timeout > 0)
	{
		mNetworkTimeoutMs = (long)CONVERT_SEC_TO_MS(timeout);
		AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d network timeout set to - %ld ms", __FUNCTION__, __LINE__, mNetworkTimeoutMs);
	}
}

/**
 *   @brief To set the network timeout to based on the priority
 *
 */
void PrivateInstanceAAMP::ConfigureNetworkTimeout()
{
	// If aamp.cfg has value , then set it as priority
	if(gpGlobalConfig->networkTimeoutMs != -1)
	{
		mNetworkTimeoutMs = gpGlobalConfig->networkTimeoutMs;
	}
	else if(mNetworkTimeoutMs == -1)
	{
		// if App has not set the value , then set default value 
		mNetworkTimeoutMs = (long)CONVERT_SEC_TO_MS(CURL_FRAGMENT_DL_TIMEOUT);
	}
	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d network timeout set to - %ld ms", __FUNCTION__, __LINE__, mNetworkTimeoutMs);
}

/**
 *   @brief To set the manifest download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PrivateInstanceAAMP::SetManifestTimeout(double timeout)
{
	if (timeout > 0)
	{
		mManifestTimeoutMs = (long)CONVERT_SEC_TO_MS(timeout);
		AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d manifest timeout set to - %ld ms", __FUNCTION__, __LINE__, mManifestTimeoutMs);
	}
}

/**
 *   @brief To set the playlist download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PrivateInstanceAAMP::SetPlaylistTimeout(double timeout)
{
        if (timeout > 0)
	{
		mPlaylistTimeoutMs = (long)CONVERT_SEC_TO_MS(timeout);
		AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Playlist timeout set to - %ld ms", __FUNCTION__, __LINE__, mPlaylistTimeoutMs);
	}
}

/**
 *   @brief To set the manifest timeout as per priority
 *
 */
void PrivateInstanceAAMP::ConfigureManifestTimeout()
{
	if(gpGlobalConfig->manifestTimeoutMs != -1)
	{
		mManifestTimeoutMs = gpGlobalConfig->manifestTimeoutMs;
	}
	else if(mManifestTimeoutMs == -1)
	{
		mManifestTimeoutMs = mNetworkTimeoutMs;
	}
	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d manifest timeout set to - %ld ms", __FUNCTION__, __LINE__, mManifestTimeoutMs);
}

/**
 *   @brief To set the playlist timeout as per priority
 *
 */
void PrivateInstanceAAMP::ConfigurePlaylistTimeout()
{
        if(gpGlobalConfig->playlistTimeoutMs != -1)
        {
                mPlaylistTimeoutMs = gpGlobalConfig->playlistTimeoutMs;
        }
        else if(mPlaylistTimeoutMs == -1)
        {
                mPlaylistTimeoutMs = mNetworkTimeoutMs;
        }
        AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d playlist timeout set to - %ld ms", __FUNCTION__, __LINE__, mPlaylistTimeoutMs);
}


/**
 *   @brief To set Parallel Download configuration
 *
 */
void PrivateInstanceAAMP::ConfigureParallelFetch()
{
	// for VOD playlist fetch
	if(gpGlobalConfig->playlistsParallelFetch != eUndefinedState)
	{
		mParallelFetchPlaylist = (bool)gpGlobalConfig->playlistsParallelFetch;
	}

	// for linear playlist fetch
	if(gpGlobalConfig->parallelPlaylistRefresh  != eUndefinedState)
	{
		mParallelFetchPlaylistRefresh = (bool)gpGlobalConfig->parallelPlaylistRefresh ;
	}

	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Parallel playlist download Init [%d] Refresh [%d]", __FUNCTION__, __LINE__, mParallelFetchPlaylist,mParallelFetchPlaylistRefresh);

}

/**
 *   @brief To set bulk timedMetadata reporting configuration
 *
 */
void PrivateInstanceAAMP::ConfigureBulkTimedMetadata()
{
        if(gpGlobalConfig->enableBulkTimedMetaReport != eUndefinedState)
        {
                mBulkTimedMetadata = (bool)gpGlobalConfig->enableBulkTimedMetaReport;
        }
        AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Bulk TimedMetadata [%d]", __FUNCTION__, __LINE__, mBulkTimedMetadata);
}

/**
 *   @brief To set unpaired discontinuity retune configuration
 *
 */
void PrivateInstanceAAMP::ConfigureRetuneForUnpairedDiscontinuity()
{
    if(gpGlobalConfig->useRetuneForUnpairedDiscontinuity != eUndefinedState)
    {
            mUseRetuneForUnpairedDiscontinuity = (bool)gpGlobalConfig->useRetuneForUnpairedDiscontinuity;
    }
    AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Retune For Unpaired Discontinuity [%d]", __FUNCTION__, __LINE__, mUseRetuneForUnpairedDiscontinuity);
}

/**
 *   @brief Set unpaired discontinuity retune flag
 *   @param[in] bValue - true if unpaired discontinuity retune set
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetRetuneForUnpairedDiscontinuity(bool bValue)
{
    mUseRetuneForUnpairedDiscontinuity = bValue;
    AAMPLOG_INFO("%s:%d Retune For Unpaired Discontinuity Config from App : %d " ,__FUNCTION__,__LINE__,bValue);
}

/**
 *   @brief Function to Configure PreCache Playlist functionality
 *
 */
void PrivateInstanceAAMP::ConfigurePreCachePlaylist()
{
	if(gpGlobalConfig->mPreCacheTimeWindow > 0)
	{
		mPreCacheDnldTimeWindow = gpGlobalConfig->mPreCacheTimeWindow;
		AAMPLOG_WARN("%s Playlist PreCaching configured from config  time %d Mins",__FUNCTION__,mPreCacheDnldTimeWindow);
	}
}

/**
 *   @brief Function to set the max retry attempts for init frag curl timeout failures
 *
 */
void PrivateInstanceAAMP::ConfigureInitFragTimeoutRetryCount()
{
	if(gpGlobalConfig->initFragmentRetryCount >= 0)
	{
		// given priority - if specified in /opt/aamp.cfg
		mInitFragmentRetryCount = gpGlobalConfig->initFragmentRetryCount;
	}
	else if (-1 == mInitFragmentRetryCount)
	{
		mInitFragmentRetryCount = DEFAULT_DOWNLOAD_RETRY_COUNT;
	}
	AAMPLOG_WARN("%s Init frag timeout retry count configured %d", __FUNCTION__, mInitFragmentRetryCount);
}

/**
 *   @brief To set Westeros sink configuration
 *
 */
void PrivateInstanceAAMP::ConfigureWesterosSink()
{
    if (gpGlobalConfig->mWesterosSinkConfig != eUndefinedState)
    {
        mWesterosSinkEnabled = (bool)gpGlobalConfig->mWesterosSinkConfig;
    }

    if (PLAYERMODE_MEDIAPLAYER == mPlayermode)
    {
        mEnableRectPropertyEnabled = ((gpGlobalConfig->mEnableRectPropertyCfg != eUndefinedState) ? ((bool)gpGlobalConfig->mEnableRectPropertyCfg) : true);
    }
    else
    {
        mEnableRectPropertyEnabled = ((gpGlobalConfig->mEnableRectPropertyCfg != eUndefinedState) ? ((bool)gpGlobalConfig->mEnableRectPropertyCfg) : (!mWesterosSinkEnabled));
    }

    if (mWesterosSinkEnabled)
    {
        AAMPLOG_WARN("Enabling Westeros Sink");
    }
    else
    {
        AAMPLOG_WARN("Disabling Westeros Sink");
    }

    AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d Westeros Sink state [%d] Video scaling rect property state [%s]", __FUNCTION__, __LINE__, mWesterosSinkEnabled, ((mEnableRectPropertyEnabled)?"True":"False"));
}

/**
 * @brief Set Playermode config for JSPP / Mediaplayer.
 *
 * @param[in] playermode - either JSPP and Mediaplayer.
 *
 */
void PrivateInstanceAAMP::ConfigurePlayerModeSettings()
{
	switch (mPlayermode)
	{
		case PLAYERMODE_MEDIAPLAYER:
		{
			AAMPLOG_WARN("%s:%d Player Mode :: Media Player",__FUNCTION__,__LINE__);
			SetTuneEventConfig(eTUNED_EVENT_ON_GST_PLAYING);
		}
			break; /* PLAYERMODE_MEDIAPLAYER */

		case PLAYERMODE_JSPLAYER:
		{
			AAMPLOG_WARN("%s:%d Player Mode :: JS Player",__FUNCTION__,__LINE__);
			SetTuneEventConfig(eTUNED_EVENT_ON_PLAYLIST_INDEXED);
		}
			break; /* PLAYERMODE_JSPLAYER */
	}
}

/**
 *   @brief To set the download buffer size value
 *
 *   @param[in] preferred download buffer size
 */
void PrivateInstanceAAMP::SetDownloadBufferSize(int bufferSize)
{
	if (bufferSize > 0)
	{
		gpGlobalConfig->maxCachedFragmentsPerTrack = bufferSize;
	}
}

bool PrivateInstanceAAMP::IsTuneCompleted()
{
	return mTuneCompleted;
}

/**
 *   @brief Set Preferred DRM.
 *
 *   @param[in] drmType - Preferred DRM type
 */
void PrivateInstanceAAMP::SetPreferredDRM(DRMSystems drmType)
{
    // if Preferred DRM is set using /opt/aamp.cfg or via RFC then
    // ignore this function setting
    if(gpGlobalConfig->isUsingLocalConfigForPreferredDRM)
    {
        AAMPLOG_INFO("%s:%d Ignoring Preferred drm: %d setting as localConfig for Preferred DRM is set to :%d", __FUNCTION__, __LINE__, drmType,gpGlobalConfig->preferredDrm);
    }
    else
    {
        AAMPLOG_INFO("%s:%d set Preferred drm: %d", __FUNCTION__, __LINE__, drmType);
        gpGlobalConfig->preferredDrm = drmType;
    }
}

/**
 *   @brief Set Stereo Only Playback.
 */
void PrivateInstanceAAMP::SetStereoOnlyPlayback(bool bValue)
{
    // If Stereo Only Mode is true, then disable DD+ and ATMOS (or) make if enable
	gpGlobalConfig->disableEC3 = bValue;
	gpGlobalConfig->disableATMOS = bValue;
	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d ATMOS and EC3 is : %s", __FUNCTION__, __LINE__, (bValue)? "Disabled" : "Enabled");
}


#ifdef PLACEMENT_EMULATION
	static int sampleAdIdx = 0;
	static const std::string sampleAds[] = {"http://ccr.ip-ads.xcr.comcast.net/omg08/UNI_Packaging_-_Production/316269638415/6563a411-908b-4abd-b3ae-23dc910fd136/563/237/CSNF8700103700100001_mezz_LVLH07.mpd",
											"http://ccr.ip-ads.xcr.comcast.net/omg05/UNI_Packaging_-_Production/450921542127/e5c6fac0-74a4-4301-807b-4ecdca384d86/977/301/CSAF8000010270110001_mezz_LVLH07.mpd"
											};
#endif

/**
 *   @brief Notification from the stream abstraction that a new SCTE35 event is found.
 *
 *   @param[in] Adbreak's unique identifier.
 *   @param[in] Break start time in milli seconds.
 *   @param[in] Break duration in milli seconds
 *   @param[in] SCTE35 binary object.
 */
void PrivateInstanceAAMP::FoundSCTE35(const std::string &adBreakId, uint64_t startMS, uint32_t breakdur, std::string &scte35)
{
	if(gpGlobalConfig->enableClientDai && !adBreakId.empty())
	{
//		gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_INFO);
		AAMPLOG_WARN("%s:%d [CDAI] Found Adbreak on period[%s] Duration[%d]", __FUNCTION__, __LINE__, adBreakId.c_str(), breakdur);
		std::string adId("");
		std::string url("");

		mCdaiObject->SetAlternateContents(adBreakId, adId, url, startMS, breakdur);	//A placeholder to avoid multiple scte35 event firing for the same adbreak
#ifdef PLACEMENT_EMULATION
		mNumAds2Place = (breakdur /1000)/30;
		if(mNumAds2Place > 0)
		{
			sampleAdBreakId = adBreakId;
			mNumAds2Place--;
			std::string adId = sampleAdBreakId+"-"+std::to_string(mNumAds2Place);
			std::string url = sampleAds[sampleAdIdx];
			sampleAdIdx = 1 - sampleAdIdx;
			mCdaiObject->SetAlternateContents(sampleAdBreakId, adId, url);
		}
#else
		ReportTimedMetadata(aamp_GetCurrentTimeMS(), "SCTE35", scte35.c_str(), scte35.size(), false, adBreakId.c_str(), breakdur);
#endif
	}
}

/**
 *   @brief Setting the alternate contents' (Ads/blackouts) URLs
 *
 *   @param[in] Adbreak's unique identifier.
 *   @param[in] Individual Ad's id
 *   @param[in] Ad URL
 */
void PrivateInstanceAAMP::SetAlternateContents(const std::string &adBreakId, const std::string &adId, const std::string &url)
{
	if(gpGlobalConfig->enableClientDai)
	{
		mCdaiObject->SetAlternateContents(adBreakId, adId, url);
	}
	else
	{
		AAMPLOG_WARN("%s:%d is called! CDAI not enabled!! Rejecting the promise.", __FUNCTION__, __LINE__);
		SendAdResolvedEvent(adId, false, 0, 0);
	}
}

/**
 *   @brief Send status of Ad manifest downloading & parsing
 *
 *   @param[in] Ad's unique identifier.
 *   @param[in] Manifest status (success/Failure)
 *   @param[in] Ad playback start time in milliseconds
 *   @param[in] Ad's duration in milliseconds
 */
void PrivateInstanceAAMP::SendAdResolvedEvent(const std::string &adId, bool status, uint64_t startMS, uint64_t durationMs)
{
	AAMPEvent e;
	if (mDownloadsEnabled)	//Send it, only if Stop not called
	{
#ifdef PLACEMENT_EMULATION
		if(mNumAds2Place > 0)
		{
			mNumAds2Place--;
			std::string adId = sampleAdBreakId+"-"+std::to_string(mNumAds2Place);
			std::string url = sampleAds[sampleAdIdx];
			sampleAdIdx = 1 - sampleAdIdx;
			mCdaiObject->SetAlternateContents(sampleAdBreakId, adId, url);
		}
#else
		e.type = AAMP_EVENT_AD_RESOLVED;
		strncpy(e.data.adResolved.adId, adId.c_str(), AD_ID_LENGTH);
		e.data.adResolved.adId[AD_ID_LENGTH-1] = '\0';
		e.data.adResolved.resolveStatus = status;
		e.data.adResolved.startMS = startMS;
		e.data.adResolved.durationMs = durationMs;
		AAMPLOG_WARN("PrivateInstanceAAMP::%s():%d, [CDAI] Sent resolved status=%d for adId[%s]", __FUNCTION__, __LINE__, e.data.adResolved.resolveStatus, adId.c_str());
		SendEventAsync(e);
#endif
	}
}

/**
 *   @brief Deliver pending Ad events to JSPP
 */
void PrivateInstanceAAMP::DeliverAdEvents(bool immediate)
{
	std::lock_guard<std::mutex> lock(mAdEventQMtx);
	while (!mAdEventsQ.empty())
	{
		AAMPEvent &e = mAdEventsQ.front();
		if(immediate)
		{
			SendEventAsync(e); 	//Need to send all events from gst idle thread.
		}
		else
		{
			SendEventSync(e);	//Already from gst idle thread
		}
		AAMPLOG_WARN("PrivateInstanceAAMP::%s():%d, [CDAI] Delivered AdEvent[%s] to JSPP.", __FUNCTION__, __LINE__, ADEVENT2STRING(e.type));
		if(AAMP_EVENT_AD_PLACEMENT_START == e.type)
		{
			mAdProgressId = e.data.adPlacement.adId;
			mAdPrevProgressTime = NOW_STEADY_TS_MS;
			mAdCurOffset        = e.data.adPlacement.offset;
			mAdDuration         = e.data.adPlacement.duration;
		}
		else if(AAMP_EVENT_AD_PLACEMENT_END == e.type || AAMP_EVENT_AD_PLACEMENT_ERROR == e.type)
		{
			mAdProgressId = "";
		}
		mAdEventsQ.pop();
	}
}

/**
 *   @brief Send Ad reservation event
 *
 *   @param[in] type - Event type
 *   @param[in] adBreakId - Reservation Id
 *   @param[in] position - Event position in terms of channel's timeline
 *   @param[in] immediate - Send it immediate or not
 */
void PrivateInstanceAAMP::SendAdReservationEvent(AAMPEventType type, const std::string &adBreakId, uint64_t position, bool immediate)
{
	if(AAMP_EVENT_AD_RESERVATION_START == type || AAMP_EVENT_AD_RESERVATION_END == type)
	{
		AAMPEvent e;
		e.type = type;
		strncpy(e.data.adReservation.adBreakId, adBreakId.c_str(), AD_ID_LENGTH);
		e.data.adReservation.position = position;
		AAMPLOG_INFO("PrivateInstanceAAMP::%s():%d, [CDAI] Pushed [%s] of adBreakId[%s] to Queue.", __FUNCTION__, __LINE__, ADEVENT2STRING(type), adBreakId.c_str());

		{
			{
				std::lock_guard<std::mutex> lock(mAdEventQMtx);
				mAdEventsQ.push(e);
			}
			if(immediate)
			{
				//Despatch all ad events now
				DeliverAdEvents(true);
			}
		}
	}
}

/**
 *   @brief Send Ad placement event
 *
 *   @param[in] type - Event type
 *   @param[in] adId - Placement Id
 *   @param[in] position - Event position wrt to the corresponding adbreak start
 *   @param[in] adOffset - Offset point of the current ad
 *   @param[in] adDuration - Duration of the current ad
 *   @param[in] immediate - Send it immediate or not
 *   @param[in] error_code - Error code (in case of placment error)
 */
void PrivateInstanceAAMP::SendAdPlacementEvent(AAMPEventType type, const std::string &adId, uint32_t position, uint32_t adOffset, uint32_t adDuration, bool immediate, long error_code)
{
	if(AAMP_EVENT_AD_PLACEMENT_START <= type && AAMP_EVENT_AD_PLACEMENT_ERROR >= type)
	{
		AAMPEvent e;
		e.type = type;
		strncpy(e.data.adPlacement.adId, adId.c_str(), AD_ID_LENGTH);
		e.data.adPlacement.position = position;
		e.data.adPlacement.offset = adOffset * 1000; //To MS
		e.data.adPlacement.duration = adDuration;
		e.data.adPlacement.errorCode = error_code;
		AAMPLOG_INFO("PrivateInstanceAAMP::%s():%d, [CDAI] Pushed [%s] of adId[%s] to Queue.", __FUNCTION__, __LINE__, ADEVENT2STRING(type), adId.c_str());

		{
			{
				std::lock_guard<std::mutex> lock(mAdEventQMtx);
				mAdEventsQ.push(e);
			}
			if(immediate)
			{
				//Despatch all ad events now
				DeliverAdEvents(true);
			}
		}
	}
}

std::string PrivateInstanceAAMP::getStreamTypeString()
{
	std::string type = mMediaFormatName[mMediaFormat];


	if(mInitSuccess) //Incomplete Init won't be set the DRM
	{
		switch(mCurrentDrm)
		{
			case eDRM_WideVine:
				type += "/WV";
				break;
			case eDRM_CONSEC_agnostic:
				type += "/Consec";
				break;
			case eDRM_PlayReady:
				type += "/PR";
				break;
			case eDRM_Adobe_Access:
				type += "/Access";
				break;
			case eDRM_Vanilla_AES:
				type += "/VanillaAES";
				break;
			default:
				type += "/Clear";
				break;
		}
	}
	else {
		type += "/Unknown";
	}
	return type;
}

ProfilerBucketType PrivateInstanceAAMP::mediaType2Bucket(MediaType fileType)
{
	ProfilerBucketType pbt;
	switch(fileType)
	{
		case eMEDIATYPE_VIDEO:
			pbt = PROFILE_BUCKET_FRAGMENT_VIDEO;
			break;
		case eMEDIATYPE_AUDIO:
			pbt = PROFILE_BUCKET_FRAGMENT_AUDIO;
			break;
		case eMEDIATYPE_SUBTITLE:
			pbt = PROFILE_BUCKET_FRAGMENT_SUBTITLE;
			break;
		case eMEDIATYPE_MANIFEST:
			pbt = PROFILE_BUCKET_MANIFEST;
			break;
		case eMEDIATYPE_INIT_VIDEO:
			pbt = PROFILE_BUCKET_INIT_VIDEO;
			break;
		case eMEDIATYPE_INIT_AUDIO:
			pbt = PROFILE_BUCKET_INIT_AUDIO;
			break;
		case eMEDIATYPE_INIT_SUBTITLE:
			pbt = PROFILE_BUCKET_INIT_SUBTITLE;
			break;
		case eMEDIATYPE_PLAYLIST_VIDEO:
			pbt = PROFILE_BUCKET_PLAYLIST_VIDEO;
			break;
		case eMEDIATYPE_PLAYLIST_AUDIO:
			pbt = PROFILE_BUCKET_PLAYLIST_AUDIO;
			break;
		case eMEDIATYPE_PLAYLIST_SUBTITLE:
			pbt = PROFILE_BUCKET_PLAYLIST_SUBTITLE;
			break;
		default:
			pbt = (ProfilerBucketType)fileType;
			break;
	}
	return pbt;
}


/**
 *   @brief Sets Recorded URL from Manifest received form XRE.
 *   @param[in] isrecordedUrl - flag to check for recordedurl in Manifest
 */
void PrivateInstanceAAMP::SetTunedManifestUrl(bool isrecordedUrl)
{
	mTunedManifestUrl.assign(mManifestUrl);
	traceprintf("%s::mManifestUrl: %s",__FUNCTION__,mManifestUrl.c_str());
	if(isrecordedUrl)
	{
		DeFog(mTunedManifestUrl);
		mTunedManifestUrl.replace(0,4,"_fog");
	}

	traceprintf("PrivateInstanceAAMP::%s, tunedManifestUrl:%s ", __FUNCTION__, mTunedManifestUrl.c_str());
}

/**
 *   @brief Gets Recorded URL from Manifest received form XRE.
 *   @param[out] manifestUrl - for VOD and recordedUrl for FOG enabled
 */
const char* PrivateInstanceAAMP::GetTunedManifestUrl()
{
	traceprintf("PrivateInstanceAAMP::%s, tunedManifestUrl:%s ", __FUNCTION__, mTunedManifestUrl.c_str());
	return mTunedManifestUrl.c_str();
}

/**
 *   @brief To set the network proxy
 *
 *   @param[in] network proxy to use
 */
void PrivateInstanceAAMP::SetNetworkProxy(const char * proxy)
{
	pthread_mutex_lock(&mLock);
	if(mNetworkProxy)
	{
		free(mNetworkProxy);
	}
	mNetworkProxy = strdup(proxy);
	pthread_mutex_unlock(&mLock);
}

const char* PrivateInstanceAAMP::GetNetworkProxy() const
{
	return mNetworkProxy;
}

/**
 *   @brief To set the proxy for license request
 *
 *   @param[in] proxy to use for license request
 */
void PrivateInstanceAAMP::SetLicenseReqProxy(const char * licenseProxy)
{
	pthread_mutex_lock(&mLock);
	if(mLicenseProxy)
	{
		free(mLicenseProxy);
	}
	mLicenseProxy = strdup(licenseProxy);
	pthread_mutex_unlock(&mLock);
}

/**
 *   @brief Signal trick mode discontinuity to stream sink
 *
 *   @return void
 */
void PrivateInstanceAAMP::SignalTrickModeDiscontinuity()
{
	if (mStreamSink)
	{
		mStreamSink->SignalTrickModeDiscontinuity();
	}
}

/**
 *   @brief Check if current stream is muxed
 *
 *   @return true if current stream is muxed
 */
bool PrivateInstanceAAMP::IsMuxedStream()
{
	bool ret = false;
	if (mpStreamAbstractionAAMP)
	{
		ret = mpStreamAbstractionAAMP->IsMuxedStream();
	}
	return ret;
}

/**
 *   @brief To set the curl stall timeout value
 *
 *   @param[in] curl stall timeout
 */
void PrivateInstanceAAMP::SetDownloadStallTimeout(long stallTimeout)
{
	if (stallTimeout >= 0)
	{
		gpGlobalConfig->curlStallTimeout = stallTimeout;
	}
}

/**
 *   @brief To set the curl download start timeout value
 *
 *   @param[in] curl download start timeout
 */
void PrivateInstanceAAMP::SetDownloadStartTimeout(long startTimeout)
{
	if (startTimeout >= 0)
	{
		gpGlobalConfig->curlDownloadStartTimeout = startTimeout;
	}
}

/**
 * @brief Stop injection for a track.
 * Called from StopInjection
 *
 * @param[in] Media type
 * @return void
 */
void PrivateInstanceAAMP::StopTrackInjection(MediaType type)
{
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d", __FUNCTION__, (int) type);
	}
#endif
	if (!mTrackInjectionBlocked[type])
	{
		AAMPLOG_TRACE("PrivateInstanceAAMP::%s for type %s", __FUNCTION__, (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mTrackInjectionBlocked[type] = true;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d", __FUNCTION__, (int) type);
}

/**
 * @brief Resume injection for a track.
 * Called from StartInjection
 *
 * @param[in] Media type
 * @return void
 */
void PrivateInstanceAAMP::ResumeTrackInjection(MediaType type)
{
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d", __FUNCTION__, (int) type);
	}
#endif
	if (mTrackInjectionBlocked[type])
	{
		AAMPLOG_TRACE("PrivateInstanceAAMP::%s for type %s", __FUNCTION__, (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mTrackInjectionBlocked[type] = false;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d", __FUNCTION__, (int) type);
}

/**
 *   @brief Receives first video PTS of the current playback
 *
 *   @param[in]  pts - pts value
 */
void PrivateInstanceAAMP::NotifyFirstVideoPTS(unsigned long long pts)
{
	if (mpStreamAbstractionAAMP)
	{
		mpStreamAbstractionAAMP->NotifyFirstVideoPTS(pts);
	}
}

/**
 *   @brief Notifies base PTS of the HLS video playback
 *
 *   @param[in]  pts - base pts value
 */
void PrivateInstanceAAMP::NotifyVideoBasePTS(unsigned long long basepts)
{
		mVideoBasePTS = basepts;
		logprintf("mVideoBasePTS::%llu\n",mVideoBasePTS);
}

/**
 *   @brief To send webvtt cue as an event
 *
 *   @param[in]  cue - vtt cue object
 */
void PrivateInstanceAAMP::SendVTTCueDataAsEvent(VTTCue* cue)
{
	//This function is called from an idle handler and hence we call SendEventSync
	if (mEventListener || mEventListeners[0] || mEventListeners[AAMP_EVENT_WEBVTT_CUE_DATA])
	{
		AAMPEvent ev;
		ev.type = AAMP_EVENT_WEBVTT_CUE_DATA;
		ev.data.cue.cueData = cue;
		SendEventSync(ev);
	}
}

/**
 *   @brief To check if subtitles are enabled
 *
 *   @return bool - true if subtitles are enabled
 */
bool PrivateInstanceAAMP::IsSubtitleEnabled(void)
{
	// Subtitle disabled for DASH
	return (!IsDashAsset() && (mEventListener || mEventListeners[AAMP_EVENT_WEBVTT_CUE_DATA]));

}

/**
 *   @brief To get any custom license HTTP headers that was set by application
 *
 *   @param[out] headers - curl header structure
 */
void PrivateInstanceAAMP::GetCustomLicenseHeaders(struct curl_slist **headers)
{
	struct curl_slist *httpHeaders = *headers;
	if (mCustomLicenseHeaders.size() > 0)
	{
		std::string customHeader;
		for (auto it = mCustomLicenseHeaders.begin(); it != mCustomLicenseHeaders.end(); it++)
		{
			customHeader.clear();
			customHeader.insert(0, it->first);
			customHeader.push_back(' ');
			// For scenarios with multiple header values, its most likely a custom defined.
			// Below code will have to extended to support the same (eg: money trace headers)
			customHeader.append(it->second.at(0));
			AAMPLOG_INFO("PrivateInstanceAAMP::%s():%d Inserting custom header to license request - %s", __FUNCTION__, __LINE__, customHeader.c_str());
			httpHeaders = curl_slist_append(httpHeaders, customHeader.c_str());
		}
		*headers = httpHeaders;
	}
}

/**
 *   @brief Set parallel playlist download config value.
 *   @param[in] bValue - true if a/v playlist to be downloaded in parallel
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetParallelPlaylistDL(bool bValue)
{
	mParallelFetchPlaylist = bValue;
	AAMPLOG_INFO("%s:%d Parallel playlist DL Config from App : %d " ,__FUNCTION__,__LINE__,bValue);
}


/**
 *   @brief Sends an ID3 metadata event.
 *
 *   @param[in] data pointer to ID3 metadata.
 *   @param[in] length length of ID3 metadata.
 */
void PrivateInstanceAAMP::SendId3MetadataEvent(uint8_t* data, int32_t length)
{
	AAMPEvent e;

	e.type = AAMP_EVENT_ID3_METADATA;
	e.data.id3Metadata.data = data;
	e.data.id3Metadata.length = length;

	SendEventSync(e);

	g_free(data);
	data = NULL;
}


/**
 * @brief Gets the listener registration status of a given event
 * @param[in] eventType - type of the event to be checked
 *
 * @retval bool - True if an event listener for the event type exists
 */
bool PrivateInstanceAAMP::GetEventListenerStatus(AAMPEventType eventType)
{
	if (mEventListeners[eventType] != NULL)
	{
		return true;
	}

	return false;
}


/**
 *	 @brief Set parallel playlist download config value for linear .
 *	 @param[in] bValue - true if a/v playlist to be downloaded in parallel for linear
 *
 *	 @return void
 */
void PrivateInstanceAAMP::SetParallelPlaylistRefresh(bool bValue)
{
	mParallelFetchPlaylistRefresh = bValue;
	AAMPLOG_INFO("%s:%d Parallel playlist Refresh Fetch  Config from App : %d " ,__FUNCTION__,__LINE__,bValue);
}


/**
 *   @brief Set Bulk TimedMetadata reporting flag 
 *   @param[in] bValue - true if Application supports bulk reporting 
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetBulkTimedMetaReport(bool bValue)
{
        mBulkTimedMetadata = bValue;
        AAMPLOG_INFO("%s:%d Bulk TimedMetadata report Config from App : %d " ,__FUNCTION__,__LINE__,bValue);
}

void PrivateInstanceAAMP::FlushStreamSink(double position, double rate)
{
#ifndef AAMP_STOP_SINK_ON_SEEK
	if (mStreamSink)
	{
		mStreamSink->SeekStreamSink(position, rate);
	}
#endif
}


/**
 *   @brief PreCachePlaylistDownloadTask Thread function for PreCaching Playlist 
 *
 *   @return void
 */
void PrivateInstanceAAMP::PreCachePlaylistDownloadTask()
{
	// This is the thread function to download all the HLS Playlist in a 
	// differed manner
	int maxWindowforDownload = mPreCacheDnldTimeWindow*60; // convert to seconds  
	int szPlaylistCount = mPreCacheDnldList.size();
	if(szPlaylistCount)
	{
		PrivAAMPState state;
		// First wait for Tune to complete to start this functionality
		pthread_mutex_lock(&mMutexPlaystart);
		pthread_cond_wait(&waitforplaystart, &mMutexPlaystart);
		pthread_mutex_unlock(&mMutexPlaystart);
		// May be Stop is called to release all resources .
		// Before download , check the state 
		GetState(state);
		if(state != eSTATE_RELEASED)
		{
			CurlInit(eCURLINSTANCE_PLAYLISTPRECACHE,1,GetNetworkProxy());
			SetCurlTimeout(mPlaylistTimeoutMs,eCURLINSTANCE_PLAYLISTPRECACHE);
			// calculate the cache size, consider 1 MB/playlist
			int maxCacheSz = szPlaylistCount * 1024*1024;
			// get the current cache max size , to restore later 
			int currMaxCacheSz =getAampCacheHandler()->GetMaxPlaylistCacheSize();
			// set new playlistCacheSize; 
			getAampCacheHandler()->SetMaxPlaylistCacheSize(maxCacheSz);
			int sleepTimeBetweenDnld = (maxWindowforDownload/szPlaylistCount)*1000; // time in milliSec 
			int idx=0;
			do
			{
				InterruptableMsSleep(sleepTimeBetweenDnld);
				if(DownloadsAreEnabled())
				{
					// First check if the file is already in Cache
					PreCacheUrlStruct newelem = mPreCacheDnldList.at(idx);
					
					// check if url cached ,if not download
					if(getAampCacheHandler()->IsUrlCached(newelem.url)==false)
					{
						AAMPLOG_WARN("%s Downloading Playlist Type:%d for PreCaching:%s",__FUNCTION__,
							newelem.type,newelem.url.c_str());
						std::string playlistUrl;
						std::string playlistEffectiveUrl;
						GrowableBuffer playlistStore;
						long http_error;
						if(GetFile(newelem.url, &playlistStore, playlistEffectiveUrl, &http_error, NULL, eCURLINSTANCE_PLAYLISTPRECACHE, true, newelem.type))
						{
							// If successful download , then insert into Cache 
							getAampCacheHandler()->InsertToPlaylistCache(newelem.url, &playlistStore, playlistEffectiveUrl,false,newelem.type);
							aamp_Free(&playlistStore.ptr);
						}	
					}
					idx++;
				}
				else
				{
					// this can come here if trickplay is done or play started late
					if(state == eSTATE_SEEKING || eSTATE_PREPARED)
					{
						// wait for seek to complete 
						sleep(1);
					}
				}
				GetState(state);
			}while(idx < mPreCacheDnldList.size() && state != eSTATE_RELEASED && state != eSTATE_IDLE);
			// restore old cache size
			getAampCacheHandler()->SetMaxPlaylistCacheSize(currMaxCacheSz);
			mPreCacheDnldList.clear();
			CurlTerm(eCURLINSTANCE_PLAYLISTPRECACHE);
		}
	}
	AAMPLOG_WARN("%s End of PreCachePlaylistDownloadTask ",__FUNCTION__);
}

/**
 *   @brief SetPreCacheDownloadList - Function to assign the PreCaching file list
 *   @param[in] Playlist Download list  
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetPreCacheDownloadList(PreCacheUrlList &dnldListInput)
{
	mPreCacheDnldList = dnldListInput;
	if(mPreCacheDnldList.size())
	{
		AAMPLOG_WARN("%s:%d Got Playlist PreCache list of Size : %d",__FUNCTION__,__LINE__,mPreCacheDnldList.size());
	}
	
}

/**
 *   @brief Get available audio tracks.
 *
 *   @return std::string JSON formatted string of available audio tracks
 */
std::string PrivateInstanceAAMP::GetAvailableAudioTracks()
{
	std::string tracks;

	if (mpStreamAbstractionAAMP)
	{
		std::vector<AudioTrackInfo> trackInfo = mpStreamAbstractionAAMP->GetAvailableAudioTracks();
		if (!trackInfo.empty())
		{
			//Convert to JSON format
			cJSON *root;
			cJSON *item;
			root = cJSON_CreateArray();
			if(root)
			{
				for (auto iter = trackInfo.begin(); iter != trackInfo.end(); iter++)
				{
					cJSON_AddItemToArray(root, item = cJSON_CreateObject());
					// Per spec, name and rendition/group-id is required
					cJSON_AddStringToObject(item, "name", iter->name.c_str());
					if (!iter->language.empty())
					{
						cJSON_AddStringToObject(item, "language", iter->language.c_str());
					}
					cJSON_AddStringToObject(item, "codec", iter->codec.c_str());
					cJSON_AddStringToObject(item, "rendition", iter->rendition.c_str());
					if (!iter->characteristics.empty())
					{
						cJSON_AddStringToObject(item, "characteristics", iter->characteristics.c_str());
					}
					if (iter->channels != 0)
					{
						cJSON_AddNumberToObject(item, "channels", iter->channels);
					}
				}
				char *jsonStr = cJSON_Print(root);
				if (jsonStr)
				{
					tracks.assign(jsonStr);
					free(jsonStr);
				}
				cJSON_Delete(root);
			}
		}
		else
		{
			AAMPLOG_ERR("PrivateInstanceAAMP::%s() %d No available audio track information!", __FUNCTION__, __LINE__);
		}
	}
	return tracks;
}

/**
 *   @brief Get available text tracks.
 *
 *   @return const char* JSON formatted string of available text tracks
 */
std::string PrivateInstanceAAMP::GetAvailableTextTracks()
{
	std::string tracks;

	if (mpStreamAbstractionAAMP)
	{
		std::vector<TextTrackInfo> trackInfo = mpStreamAbstractionAAMP->GetAvailableTextTracks();
		if (!trackInfo.empty())
		{
			//Convert to JSON format
			cJSON *root;
			cJSON *item;
			root = cJSON_CreateArray();
			if(root)
			{
				for (auto iter = trackInfo.begin(); iter != trackInfo.end(); iter++)
				{
					cJSON_AddItemToArray(root, item = cJSON_CreateObject());
					// Per spec, name and rendition/group-id is required
					cJSON_AddStringToObject(item, "name", iter->name.c_str());
					if (iter->isCC)
					{
						cJSON_AddStringToObject(item, "type", "CLOSED-CAPTIONS");
					}
					else
					{
						cJSON_AddStringToObject(item, "type", "SUBTITLES");
					}
					if (!iter->language.empty())
					{
						cJSON_AddStringToObject(item, "language", iter->language.c_str());
					}
					cJSON_AddStringToObject(item, "rendition", iter->rendition.c_str());
					if (!iter->instreamId.empty())
					{
						cJSON_AddStringToObject(item, "instreamId", iter->instreamId.c_str());
					}
					if (!iter->characteristics.empty())
					{
						cJSON_AddStringToObject(item, "characteristics", iter->characteristics.c_str());
					}
				}
				char *jsonStr = cJSON_Print(root);
				if (jsonStr)
				{
					tracks.assign(jsonStr);
					free(jsonStr);
				}
				cJSON_Delete(root);
			}
		}
		else
		{
			AAMPLOG_ERR("PrivateInstanceAAMP::%s() %d No available text track information!", __FUNCTION__, __LINE__);
		}
	}
	return tracks;
}

/*
 *   @brief Get the video window co-ordinates
 *
 *   @return current video co-ordinates in x,y,w,h format
 */
std::string PrivateInstanceAAMP::GetVideoRectangle()
{
	return mStreamSink->GetVideoRectangle();
}

/*
 *   @brief Set the application name which has created PlayerInstanceAAMP, for logging purposes
 *
 *   @return void
 */
void PrivateInstanceAAMP::SetAppName(std::string name)
{
	mAppName = name;
}

/**
 * @}
 */

/**
 * @brief Check if track can inject data into GStreamer.
 * Called from MonitorBufferHealth
 *
 * @param[in] Media type
 * @return bool true if track can inject data, false otherwise
 */
bool PrivateInstanceAAMP::TrackDownloadsAreEnabled(MediaType type)
{
	bool ret = true;
	if (type > AAMP_TRACK_COUNT)
	{
		AAMPLOG_ERR("%s:%d type[%d] is un-supported, returning default as false!", __FUNCTION__, __LINE__, type);
		ret = false;
	}
	else
	{
		pthread_mutex_lock(&mLock);
		// If blocked, track downloads are disabled
		ret = !mbTrackDownloadsBlocked[type];
		pthread_mutex_unlock(&mLock);
	}
	return ret;
}

/**
 * @brief Stop buffering in AAMP and un-pause pipeline.
 * Called from MonitorBufferHealth
 *
 * @param[in] forceStop - stop buffering forcefully
 * @return void
 */
void PrivateInstanceAAMP::StopBuffering(bool forceStop)
{
	mStreamSink->StopBuffering(forceStop);
}



