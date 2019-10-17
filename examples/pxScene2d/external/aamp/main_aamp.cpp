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

#include <sys/time.h>
#ifndef DISABLE_DASH
#include "fragmentcollector_mpd.h"
#endif
#include "fragmentcollector_hls.h"
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
#include "AampCacheHandler.h"
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
    
	AsyncEventDescriptor( const AsyncEventDescriptor & newObj) : AsyncEventDescriptor()
	{
		this->event = newObj.event;
		this->aamp = newObj.aamp;
	}

	AsyncEventDescriptor& operator=(const AsyncEventDescriptor& newObj)
	{
		this->event = newObj.event;
		this->aamp = newObj.aamp;

		return *this;
	}

	AAMPEvent event;
	PrivateInstanceAAMP* aamp;

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

static TuneFailureMap tuneFailureMap[] =
{
	{AAMP_TUNE_INIT_FAILED, 10, "AAMP: init failed"}, //"Fragmentcollector initialization failed"
	{AAMP_TUNE_MANIFEST_REQ_FAILED, 10, "AAMP: Manifest Download failed"}, //"Playlist refresh failed"
	{AAMP_TUNE_AUTHORISATION_FAILURE, 40, "AAMP: Authorization failure"},
	{AAMP_TUNE_FRAGMENT_DOWNLOAD_FAILURE, 10, "AAMP: fragment download failures"},
	{AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, 10, "AAMP: init fragment download failed"},
	{AAMP_TUNE_UNTRACKED_DRM_ERROR, 50, "AAMP: DRM error untracked error"},
	{AAMP_TUNE_DRM_INIT_FAILED, 50, "AAMP: DRM Initialization Failed"},
	{AAMP_TUNE_DRM_DATA_BIND_FAILED, 50, "AAMP: InitData-DRM Binding Failed"},
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
    {AAMP_TUNE_INVALID_MANIFEST_FAILURE, 10, "AAMP: Invalid Manifest, parse failed"},
	{AAMP_TUNE_FAILED_PTS_ERROR, 80, "AAMP: Playback failed due to PTS error"},
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
        logprintf("%s(): Parameter '%s' not within INTEGER limit. Using default value instead.\n", __FUNCTION__, param_name); \
        param_value = default_value; \
    }

#define VALIDATE_LONG(param_name, param_value, default_value)        \
    if ((param_value <= 0) || (param_value > LONG_MAX))  { \
        logprintf("%s(): Parameter '%s' not within LONG INTEGER limit. Using default value instead.\n", __FUNCTION__, param_name); \
        param_value = default_value; \
    }

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
	switch(drmSystem)
	{
	case eDRM_WideVine:
		return "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed";
	case eDRM_PlayReady:
		return "9a04f079-9840-4286-ab92-e65be0885f95";
	case eDRM_CONSEC_agnostic:
		return "afbcb50e-bf74-3d13-be8f-13930c783962";
	}
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
		return "WideWine";
	case eDRM_PlayReady:
		return "PlayReady";
	case eDRM_CONSEC_agnostic:
		return "Consec Agnostic";
	case eDRM_Adobe_Access:
		return "Adobe Access";
	case eDRM_Vanilla_AES:
		return "Vanilla AES";
	}
	return "";
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

		eventData.data.progress.positionMiliseconds = (seek_pos_seconds) * 1000.0;
		if (!pipeline_paused && trickStartUTCMS >= 0)
		{
			long long elapsedTime = aamp_GetCurrentTimeMS() - trickStartUTCMS;
			eventData.data.progress.positionMiliseconds += elapsedTime * rate;
			// note, using StreamSink::GetPositionMilliseconds() instead of elapsedTime
			// would likely be more accurate, but would need to be tested to accomodate
			// and compensate for FF/REW play rates
		}
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
			//logprintf("aamp clamp end\n");
			eventData.data.progress.positionMiliseconds = eventData.data.progress.endMiliseconds;
		}
		else if (eventData.data.progress.positionMiliseconds < eventData.data.progress.startMiliseconds)
		{ // clamp start
			//logprintf("aamp clamp start\n");
			eventData.data.progress.positionMiliseconds = eventData.data.progress.startMiliseconds;
		}

		if (gpGlobalConfig->logging.progress)
		{
			static int tick;
			if ((tick++ & 3) == 0)
			{
				logprintf("aamp pos: [%ld..%ld..%ld]\n",
					(long)(eventData.data.progress.startMiliseconds / 1000),
					(long)(eventData.data.progress.positionMiliseconds / 1000),
					(long)(eventData.data.progress.endMiliseconds / 1000));
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
	AAMPLOG_INFO("aamp_UpdateDuration(%f)\n", seconds);
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
		logprintf("PrivateInstanceAAMP::%s - culling started, first value %f\n", __FUNCTION__, culledSecs);
	}
	this->culledSeconds += culledSecs;

	// Check if we are paused and culled past paused playback position
	// AAMP internally caches fragments in sw and gst buffer, so we should be good here
	if (pipeline_paused && mpStreamAbstractionAAMP)
	{
		double minPlaylistPositionToResume = (seek_pos_seconds < maxRefreshPlaylistIntervalSecs) ? seek_pos_seconds : (seek_pos_seconds - maxRefreshPlaylistIntervalSecs);
		if (this->culledSeconds >= seek_pos_seconds)
		{
			logprintf("%s(): Resume playback since playlist start position(%f) has moved past paused position(%f) \n", __FUNCTION__, this->culledSeconds, seek_pos_seconds);
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
				logprintf("%s(): Resume playback since start position(%f) moved very close to minimum resume position(%f) \n", __FUNCTION__, this->culledSeconds, minPlaylistPositionToResume);
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
	AAMPLOG_INFO("%s(): maxRefreshPlaylistIntervalSecs (%f)\n", __FUNCTION__, maxIntervalSecs);
	maxRefreshPlaylistIntervalSecs = maxIntervalSecs;
}


/**
 * @brief Add listener to aamp events
 * @param eventType type of event to subscribe
 * @param eventListener listener
 */
void PrivateInstanceAAMP::AddEventListener(AAMPEventType eventType, AAMPEventListener* eventListener)
{
	//logprintf("[AAMP_JS] %s(%d, %p)\n", __FUNCTION__, eventType, eventListener);
	if ((eventListener != NULL) && (eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
	{
		ListenerData* pListener = new ListenerData;
		if (pListener)
		{
			//logprintf("[AAMP_JS] %s(%d, %p) new %p\n", __FUNCTION__, eventType, eventListener, pListener);
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
	logprintf("[AAMP_JS] %s(%d, %p)\n", __FUNCTION__, eventType, eventListener);
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
				logprintf("[AAMP_JS] %s(%d, %p) delete %p\n", __FUNCTION__, eventType, eventListener, pListener);
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
		logprintf("%s:%d : Received unknown error event %d\n", __FUNCTION__, __LINE__, tuneFailure);
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
		if(error_code < 100)
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
		logprintf("%s:%d : Received unknown error event %d\n", __FUNCTION__, __LINE__, tuneFailure);
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
        AAMPLOG_INFO("Anomaly evt:%d msg:%s\n",e.data.anomalyReport.severity,msgData);
        SendEventAsync(e);
    }
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
		logprintf("Sending error %s \n",e.data.mediaError.description);
		SendEventAsync(e);
		SendAnomalyEvent(ANOMALY_ERROR,"Error[%d]:%s",tuneFailure,e.data.mediaError.description);
	}
	else
	{
		logprintf("PrivateInstanceAAMP::%s:%d Ignore error %d[%s]\n", __FUNCTION__, __LINE__, (int)tuneFailure, description);
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
			AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d event type  %d\n", __FUNCTION__, __LINE__,e.type);
	}
}


/**
 * @brief Send event synchronously to listeners
 * @param e event
 */
void PrivateInstanceAAMP::SendEventSync(const AAMPEvent &e)
{
	if(e.type != AAMP_EVENT_PROGRESS)
		AAMPLOG_INFO("[AAMP_JS] %s(type=%d)\n", __FUNCTION__, e.type);

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
			//logprintf("[AAMP_JS] %s(type=%d) listener=%p\n", __FUNCTION__, eventType, pCurrent->eventListener);
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
void PrivateInstanceAAMP::NotifyBitRateChangeEvent(int bitrate ,const char *description ,int width ,int height, bool GetBWIndex)
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

		/* START: Added As Part of DELIA-28363 and DELIA-28247 */
		if(GetBWIndex && (mpStreamAbstractionAAMP != NULL))
		{
			logprintf("NotifyBitRateChangeEvent :: bitrate:%d desc:%s width:%d height:%d, IndexFromTopProfile: %d%s\n",bitrate,description,width,height, mpStreamAbstractionAAMP->GetBWIndex(bitrate), (IsTSBSupported()? ", fog": " "));
		}
		else
		{
			logprintf("NotifyBitRateChangeEvent :: bitrate:%d desc:%s width:%d height:%d%s\n",bitrate,description,width,height, (IsTSBSupported()? ", fog": " "));
		}
		/* END: Added As Part of DELIA-28363 and DELIA-28247 */

		ScheduleEvent(e);
	}
	else
	{
		/* START: Added As Part of DELIA-28363 and DELIA-28247 */
		if(GetBWIndex && (mpStreamAbstractionAAMP != NULL))
		{
			logprintf("NotifyBitRateChangeEvent ::NO LISTENERS bitrate:%d desc:%s width:%d height:%d, IndexFromTopProfile: %d%s\n",bitrate,description,width,height, mpStreamAbstractionAAMP->GetBWIndex(bitrate), (IsTSBSupported()? ", fog": " "));
		}
		else
		{
			logprintf("NotifyBitRateChangeEvent ::NO LISTENERS bitrate:%d desc:%s width:%d height:%d%s\n",bitrate,description,width,height, (IsTSBSupported()? ", fog": " "));
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

        SendEventSync(e);
        logprintf("SendDRMMetaData name = %s value = %x\n",e.data.dash_drmmetadata.accessStatus,e.data.dash_drmmetadata.accessStatus_value);
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
		aamp->ProcessPendingDiscontinuity();

		aamp->SyncBegin();
		aamp->mDiscontinuityTuneOperationId = 0;
		pthread_cond_signal(&aamp->mCondDiscontinuity);
		aamp->SyncEnd();
	}
	return G_SOURCE_REMOVE;
}


/**
 * @brief Check if discontinuity processing is pending
 * @retval true if discontinuity processing is pending
 */
bool PrivateInstanceAAMP::IsDiscontinuityProcessPending()
{
	return mProcessingDiscontinuity;
}


/**
 * @brief Process pending discontinuity and continue playback of stream after discontinuity
 */
void PrivateInstanceAAMP::ProcessPendingDiscontinuity()
{
	SyncBegin();
	if (mDiscontinuityTuneOperationInProgress)
	{
		SyncEnd();
		logprintf("PrivateInstanceAAMP::%s:%d Discontinuity Tune Operation already in progress\n", __FUNCTION__, __LINE__);
		return;
	}
	SyncEnd();

	if (!mProcessingDiscontinuity)
	{
		return;
	}

	SyncBegin();
	mDiscontinuityTuneOperationInProgress = true;
	SyncEnd();

	if (mProcessingDiscontinuity)
	{
		logprintf("PrivateInstanceAAMP::%s:%d mProcessingDiscontinuity set\n", __FUNCTION__, __LINE__);
		lastUnderFlowTimeMs[eMEDIATYPE_VIDEO] = 0;
		lastUnderFlowTimeMs[eMEDIATYPE_AUDIO] = 0;
		mpStreamAbstractionAAMP->StopInjection();
#ifndef AAMP_STOP_SINK_ON_SEEK
		mStreamSink->Flush(mpStreamAbstractionAAMP->GetFirstPTS(), rate);
#else
		mStreamSink->Stop(true);
#endif
		mpStreamAbstractionAAMP->GetStreamFormat(mFormat, mAudioFormat);
		mStreamSink->Configure(mFormat, mAudioFormat, false);
		mpStreamAbstractionAAMP->StartInjection();
		mStreamSink->Stream();
		mProcessingDiscontinuity = false;
	}

	SyncBegin();
	mDiscontinuityTuneOperationInProgress = false;
	SyncEnd();
}

/**
 * @brief Process EOS from Sink and notify listeners if required
 */
void PrivateInstanceAAMP::NotifyEOSReached()
{
	logprintf("%s: Enter . processingDiscontinuity %d\n",__FUNCTION__, mProcessingDiscontinuity);
	if (!IsLive() && rate > 0 && (!mProcessingDiscontinuity))
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
	if (!IsDiscontinuityProcessPending())
	{
		if (rate < 0)
		{
			seek_pos_seconds = culledSeconds;
			logprintf("%s:%d Updated seek_pos_seconds %f \n", __FUNCTION__,__LINE__, seek_pos_seconds);
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
		logprintf("PrivateInstanceAAMP::%s:%d  EOS due to discontinuity handled\n", __FUNCTION__, __LINE__);
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
		AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d event type  %d\n", __FUNCTION__, __LINE__,e->event.type);
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
	SendMessage2Receiver(E_AAMP2Receiver_EVENTS,eventsJSON.str().c_str());
}

/**
 * @brief Notify tune end for profiling/logging
 */
void PrivateInstanceAAMP::LogTuneComplete(void)
{
	bool success = true; // TODO
	int streamType = getStreamType();
	profiler.TuneEnd(success, mContentType, streamType, mFirstTune);

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
		TunedEventConfig tunedEventConfig = IsLive() ? gpGlobalConfig->tunedEventConfigLive : gpGlobalConfig->tunedEventConfigVOD;
		if (eTUNED_EVENT_ON_GST_PLAYING == tunedEventConfig)
		{
			if (SendTunedEvent())
			{
				logprintf("aamp: - sent tune event on Tune Completion.\n");
			}
		}

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
	logprintf("aamp ERROR: %s\n", msg);
	//exit(1);
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
	traceprintf ("PrivateInstanceAAMP::%s\n", __FUNCTION__);
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
	traceprintf ("PrivateInstanceAAMP::%s\n", __FUNCTION__);
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
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, (int) type);
	}
#endif
	if (!mbTrackDownloadsBlocked[type])
	{
		AAMPLOG_TRACE("gstreamer-enough-data from %s source\n", (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mbTrackDownloadsBlocked[type] = true;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, (int) type);
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
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, (int) type);
	}
#endif
	if (mbTrackDownloadsBlocked[type])
	{
		AAMPLOG_TRACE("gstreamer-needs-data from %s source\n", (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mbTrackDownloadsBlocked[type] = false;
		//log_current_time("gstreamer-needs-data");
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d\n", __FUNCTION__, (int) type);
}

/**
 * @brief block until gstreamer indicates pipeline wants more data
 * @param cb callback called periodically, if non-null
 * @param periodMs delay between callbacks
 * @param track track index
 */
void PrivateInstanceAAMP::BlockUntilGstreamerWantsData(void(*cb)(void), int periodMs, int track)
{ // called from FragmentCollector thread; blocks until gstreamer wants data
	traceprintf( "PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, track);
	int elapsedMs = 0;
	while (mbDownloadsBlocked || mbTrackDownloadsBlocked[track])
	{
		if (!mDownloadsEnabled || mTrackInjectionBlocked[track])
		{
			logprintf("PrivateInstanceAAMP::%s interrupted. mDownloadsEnabled:%d mTrackInjectionBlocked:%d\n", __FUNCTION__, mDownloadsEnabled, mTrackInjectionBlocked[track]);
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
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d\n", __FUNCTION__, track);
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
			logprintf("%s:%d WARNING - realloc. buf %p avail %d required %d\n", __FUNCTION__, __LINE__, buffer, (int)buffer->avail, (int)required);
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
	GrowableBuffer *buffer;
	httpRespHeaderData *responseHeaderData;
	long bitrate;

	CurlCallbackContext() : aamp(NULL), buffer(NULL), responseHeaderData(NULL),bitrate(0)
	{

	}
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
		logprintf("write_callback - interrupted\n");
	}
	pthread_mutex_unlock(&context->aamp->mLock);
	return ret;
}

#define FOG_REASON_STRING           "Fog-Reason:"

/**
 * @brief callback invoked on http header by curl
 * @param ptr pointer to buffer containing the data
 * @param size size of the buffer
 * @param nmemb number of bytes
 * @param user_data  CurlCallbackContext pointer
 * @retval
 */
static size_t header_callback(void *ptr, size_t size, size_t nmemb, void *user_data)
{
	//std::string *httpHeaders = static_cast<std::string *>(user_data);
	CurlCallbackContext *context = static_cast<CurlCallbackContext *>(user_data);
	httpRespHeaderData *httpHeader = context->responseHeaderData;
	size_t len = nmemb * size;
	int startPos = 0;
	int endPos = 0;
	bool isBitrateHeader = false;

	std::string header((const char *)ptr, 0, len);

    if (std::string::npos != header.find(FOG_REASON_STRING))
    {
        httpHeader->type = eHTTPHEADERTYPE_FOG_REASON;
        logprintf("%s:%d %s\n", __FUNCTION__, __LINE__, header.c_str());
        startPos = header.find(FOG_REASON_STRING) + strlen(FOG_REASON_STRING);
        endPos = header.length() - 1;
    }
	else if (std::string::npos != header.find("X-Reason:"))
	{
		httpHeader->type = eHTTPHEADERTYPE_XREASON;
		logprintf("%s:%d %s\n", __FUNCTION__, __LINE__, header.c_str());
		startPos = header.find("X-Reason:") + strlen("X-Reason:");
		endPos = header.length() - 1;
	}
	else if (std::string::npos != header.find("X-Bitrate:"))
	{
		logprintf("%s:%d %s\n", __FUNCTION__, __LINE__, header.c_str());
		startPos = header.find("X-Bitrate:") + strlen("X-Bitrate:");
		endPos = header.length() - 1;
		isBitrateHeader = true;
	}
	else if (std::string::npos != header.find("Set-Cookie:"))
	{
		httpHeader->type = eHTTPHEADERTYPE_COOKIE;
		startPos = header.find("Set-Cookie:") + strlen("Set-Cookie:");
		endPos = header.length() - 1;
	}
	else if (std::string::npos != header.find("Location:"))
	{
		httpHeader->type = eHTTPHEADERTYPE_EFF_LOCATION;
		logprintf("%s:%d %s\n", __FUNCTION__, __LINE__, header.c_str());
		startPos = header.find("Location:") + strlen("Location:");
		endPos = header.length() - 1;
	}
	else if (0 == context->buffer->avail)
	{
		size_t headerStart = header.find("Content-Length:");
		if (std::string::npos != headerStart )
		{
			int contentLengthStartPosition = headerStart + (sizeof("Content-Length:")-1);
			while (isspace(header[contentLengthStartPosition]) && (contentLengthStartPosition <= header.length()))
			{
				contentLengthStartPosition++;
			}
			while(isspace(header.back()))
			{
				header.pop_back();
			}
			std::string contentLengthStr = header.substr(contentLengthStartPosition);
			int contentLength = std::stoi(contentLengthStr);
			traceprintf("%s:%d header %s contentLengthStr %s  contentLength %d\n",__FUNCTION__,__LINE__, header.c_str(), contentLengthStr.c_str(), contentLength);
			/*contentLength can be zero for redirects*/
			if (contentLength > 0)
			{
				/*Add 2 additional characters to take care of extra characters inserted by aamp_AppendNulTerminator*/
				aamp_Malloc(context->buffer, contentLength + 2);
			}
		}
	}

	if((startPos > 0) && (endPos > 0))
	{
		//Find the first character after the http header name
		while ((header[startPos] == ' ') && (startPos <= header.length()))
		{
			startPos++;
		}
		while ((header[endPos] == '\r' || header[endPos] == '\n' || header[endPos] == ';') && (endPos >= 0))
		{
			endPos--;
		}

		if(isBitrateHeader)
		{
			std::string strBitrate = header.substr(startPos, (endPos - startPos + 1));
			context->bitrate = std::stol("0" + strBitrate);
		}
		else
		{
			httpHeader->data = header.substr(startPos, (endPos - startPos + 1));
			if(httpHeader->type != eHTTPHEADERTYPE_EFF_LOCATION)
			{
				//Append a delimiter ";"
				httpHeader->data += ';';
			}

			traceprintf("Parsed HTTP %s header: %s\n", httpHeader->type==eHTTPHEADERTYPE_COOKIE? "Cookie": "X-Reason", httpHeader->data.c_str());
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
						logprintf("Abort download as mid-download stall detected for %.2f seconds, download size:%.2f bytes\n", timeElapsedSinceLastUpdate, dlnow);
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
				logprintf("Abort download as no data received for %.2f seconds\n", timeElapsedInSec);
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
		logprintf("curl: %s\n", data);
		break;
		case CURLINFO_HEADER_IN:
		logprintf("curl header: %s\n", data);
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
void PrivateInstanceAAMP::CurlInit(int startIdx, unsigned int instanceCount)
{
	int instanceEnd = startIdx + instanceCount;
	assert (instanceEnd <= MAX_CURL_INSTANCE_COUNT);
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

			curlDLTimeout[i] = DEFAULT_CURL_TIMEOUT;

			if (mNetworkProxy || mLicenseProxy || gpGlobalConfig->httpProxy)
			{
				const char *proxy = NULL;
				if (mNetworkProxy || mLicenseProxy)
				{
					if (i < AAMP_TRACK_COUNT)
					{
						proxy = mNetworkProxy;
					}
					else
					{
						proxy = mLicenseProxy;
					}
				}
				else
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
			}
			if(ContentType_EAS == mContentType)
			{
				//enable verbose logs so we can debug field issues
				curl_easy_setopt(curl[i], CURLOPT_VERBOSE, 1);
				curl_easy_setopt(curl[i], CURLOPT_DEBUGFUNCTION, eas_curl_debug_callback);
				//set eas specific timeouts to handle faster cycling through bad hosts and faster total timeout
				curl_easy_setopt(curl[i], CURLOPT_TIMEOUT, EAS_CURL_TIMEOUT);
				curl_easy_setopt(curl[i], CURLOPT_CONNECTTIMEOUT, EAS_CURL_CONNECTTIMEOUT);

				curlDLTimeout[i] = EAS_CURL_TIMEOUT;

				//on ipv6 box force curl to use ipv6 mode only (DELIA-20209)
				struct stat tmpStat;
				bool isv6(::stat( "/tmp/estb_ipv6", &tmpStat) == 0);
				if(isv6)
					curl_easy_setopt(curl[i], CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
				logprintf("aamp eas curl config: timeout=%d, connecttimeout%d, ipv6=%d\n", EAS_CURL_TIMEOUT, EAS_CURL_CONNECTTIMEOUT, isv6);
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
                logprintf("IsAudioLanguageSupported No Audio language stored !!!\n");
	}
	else if(!retVal)
	{
		logprintf("IsAudioLanguageSupported lang[%s] not available in list\n",checkLanguage);
	}
	return retVal;
}


/**
 * @brief Set curl timeout (CURLOPT_TIMEOUT)
 * @param timeout maximum time  in seconds curl request is allowed to take
 * @param instance index of instance to which timeout to be set
 */
void PrivateInstanceAAMP::SetCurlTimeout(long timeout, unsigned int instance)
{
	if(ContentType_EAS == mContentType)
		return;
	if(instance < MAX_CURL_INSTANCE_COUNT && curl[instance])
	{
		curl_easy_setopt(curl[instance], CURLOPT_TIMEOUT, timeout);
		curlDLTimeout[instance] = timeout;
	}
	else
	{
		logprintf("Failed to update timeout for curl instace %d\n",instance);
	}
}


/**
 * @brief Terminate curl instances
 * @param startIdx start index
 * @param instanceCount count of instances
 */
void PrivateInstanceAAMP::CurlTerm(int startIdx, unsigned int instanceCount)
{
	int instanceEnd = startIdx + instanceCount;
	assert (instanceEnd <= MAX_CURL_INSTANCE_COUNT);
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
 * @brief called when tuning - reset artificially
 * low for quicker tune times
 * @param bitsPerSecond
 * @param trickPlay
 * @param profile
 */
void PrivateInstanceAAMP::ResetCurrentlyAvailableBandwidth(long bitsPerSecond , bool trickPlay,int profile)
{
	if (mAbrBitrateData.size())
	{
		mAbrBitrateData.erase(mAbrBitrateData.begin(),mAbrBitrateData.end());
	}
	AAMPLOG_WARN("ABRMonitor-Reset::{\"Reason\":\"%s\",\"Bandwidth\":%ld,\"Profile\":%d}\n",(trickPlay)?"TrickPlay":"Tune",bitsPerSecond,profile);
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
	for (bitrateIter = mAbrBitrateData.begin(); bitrateIter != mAbrBitrateData.end();)
	{
		//logprintf("[%s][%d] Sz[%d] TimeCheck Pre[%lld] Sto[%lld] diff[%lld] bw[%ld] \n",__FUNCTION__,__LINE__,mAbrBitrateData.size(),presentTime,(*bitrateIter).first,(presentTime - (*bitrateIter).first),(long)(*bitrateIter).second);
		if ((bitrateIter->first <= 0) || (presentTime - bitrateIter->first > gpGlobalConfig->abrCacheLife))
		{
			//logprintf("[%s][%d] Threadshold time reached , removing bitrate data \n",__FUNCTION__,__LINE__);
			bitrateIter = mAbrBitrateData.erase(bitrateIter);
		}
		else
		{
			tmpData.push_back(bitrateIter->second);
			bitrateIter++;
		}
	}

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
				//logprintf("[%s][%d] Outlier found[%ld]>[%ld] erasing ....\n",__FUNCTION__,__LINE__,diffOutlier,gpGlobalConfig->abrOutlierDiffBytes);
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
			//logprintf("[%s][%d] NwBW with newlogic size[%d] avg[%ld] \n",__FUNCTION__,__LINE__,tmpData.size(), avg/tmpData.size());
			ret = (avg/tmpData.size());
			mAvailableBandwidth = ret;
		}	
		else
		{
			//logprintf("[%s][%d] No prior data available for abr , return -1 \n",__FUNCTION__,__LINE__);
			ret = -1;
		}
	}
	else
	{
		//logprintf("[%s][%d] No data available for bitrate check , return -1 \n",__FUNCTION__,__LINE__);
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
		default:
			return "Unknown";
	}
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
 * @retval true if success
 */
bool PrivateInstanceAAMP::GetFile(const char *remoteUrl, struct GrowableBuffer *buffer, char effectiveUrl[MAX_URI_LENGTH], long * http_error, const char *range, unsigned int curlInstance, bool resetBuffer, MediaType fileType, long *bitrate, int * fogError)
{
	long http_code = -1;
	bool ret = false;
	int downloadAttempt = 0;
	CURL* curl = this->curl[curlInstance];
	struct curl_slist* httpHeaders = NULL;
	CURLcode res = CURLE_OK;

	pthread_mutex_lock(&mLock);
	if (resetBuffer)
	{
		if(buffer->avail)
        	{
            		AAMPLOG_TRACE("%s:%d reset buffer %p avail %d\n", __FUNCTION__, __LINE__, buffer, (int)buffer->avail);
        	}	
		memset(buffer, 0x00, sizeof(*buffer));
	}
	if (mDownloadsEnabled)
	{
		long long downloadTimeMS = 0;
		bool isDownloadStalled = false;
		pthread_mutex_unlock(&mLock);
		AAMPLOG_INFO("aamp url: %s\n", remoteUrl);
		CurlCallbackContext context;

		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, remoteUrl);

			context.aamp = this;
			context.buffer = buffer;
			context.responseHeaderData = &httpRespHeaders[curlInstance];
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &context);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &context);
			curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

			CurlProgressCbContext progressCtx;
			progressCtx.aamp = this;
			//Disable download stall detection checks for FOG playback done by JS PP
			progressCtx.stallTimeout = gpGlobalConfig->curlStallTimeout;
			progressCtx.startTimeout = gpGlobalConfig->curlDownloadStartTimeout;

			// note: win32 curl lib doesn't support multi-part range
			curl_easy_setopt(curl, CURLOPT_RANGE, range);

			if ((httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_COOKIE) && (httpRespHeaders[curlInstance].data.length() > 0))
			{
				traceprintf("Appending cookie headers to HTTP request\n");
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
				if (httpHeaders != NULL)
				{
					curl_easy_setopt(curl, CURLOPT_HTTPHEADER, httpHeaders);
				}
			}

			while(downloadAttempt < 2)
			{
				progressCtx.downloadStartTime = NOW_STEADY_TS_MS;
				progressCtx.downloadUpdatedTime = -1;
				progressCtx.downloadSize = -1;
				progressCtx.abortReason = eCURL_ABORT_REASON_NONE;
				curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, &progressCtx);
				if(buffer->ptr != NULL)
				{
					traceprintf("%s:%d reset length. buffer %p avail %d\n", __FUNCTION__, __LINE__, buffer, (int)buffer->avail);
					buffer->len = 0;
				}

				isDownloadStalled = false;

				long long tStartTime = NOW_STEADY_TS_MS;
				CURLcode res = curl_easy_perform(curl); // synchronous; callbacks allow interruption
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
#if 0 /* Commented since the same is supported via AAMP_LOG_NETWORK_ERROR */
						logprintf("HTTP RESPONSE CODE: %ld\n", http_code);
#else
						AAMP_LOG_NETWORK_ERROR (remoteUrl, AAMPNetworkErrorHttp, (int)http_code);
#endif /* 0 */
						if((http_code >= 500 && http_code != 502) && downloadAttempt < 2)
						{
							InterruptableMsSleep(gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS);
							logprintf("Download failed due to Server error. Retrying!\n");
							loopAgain = true;
						}
					}
					if(http_code == 204)
					{
						if ( (httpRespHeaders[curlInstance].type == eHTTPHEADERTYPE_EFF_LOCATION) && (httpRespHeaders[curlInstance].data.length() > 0) )
						{
							logprintf("%s:%d Received Location header: '%s'\n",__FUNCTION__,__LINE__, httpRespHeaders[curlInstance].data.c_str());
							effectiveUrlPtr =  const_cast<char *>(httpRespHeaders[curlInstance].data.c_str());
						}
					}
					else
					{
						res = curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &effectiveUrlPtr);
					}
					strncpy(effectiveUrl, effectiveUrlPtr, MAX_URI_LENGTH-1);
					effectiveUrl[MAX_URI_LENGTH-1] = '\0';

					// check if redirected url is pointing to fog / local ip
					if(mIsFirstRequestToFOG)
					{
					    if( strstr(effectiveUrl,LOCAL_HOST_IP) == NULL )
					    {
					        // oops, TSB is not working, we got redirected away from fog
					        mIsLocalPlayback = false;
					        mTSBEnabled = false;
					        logprintf("NO_TSB_AVAILABLE playing from:%s \n", effectiveUrl);
					    }
					    // updating here because, tune request can be for fog but fog may redirect to cdn in some cases
					    this->UpdateVideoEndTsbStatus(mTSBEnabled);
					}

					/*
					 * Latency should be printed in the case of successful download which exceeds the download threshold value,
					 * other than this case is assumed as network error and those will be logged with AAMP_LOG_NETWORK_ERROR.
					 */
					if (downloadTimeMS > FRAGMENT_DOWNLOAD_WARNING_THRESHOLD )
					{
						AAMP_LOG_NETWORK_LATENCY (effectiveUrl, downloadTimeMS, FRAGMENT_DOWNLOAD_WARNING_THRESHOLD );
					}
				}
				else
				{
					long curlDownloadTimeoutMS = curlDLTimeout[curlInstance] * 1000;
					//use a delta of 100ms for edge cases
					//abortReason for progress_callback exit scenarios
					isDownloadStalled = ((res == CURLE_OPERATION_TIMEDOUT || res == CURLE_PARTIAL_FILE ||
									(progressCtx.abortReason != eCURL_ABORT_REASON_NONE)) &&
									(buffer->len >= 0) &&
									(downloadTimeMS < curlDownloadTimeoutMS - 100));

					/* Curl 23 and 42 is not a real network error, so no need to log it here */
					//Log errors due to curl stall/start detection abort
					if (AAMP_IS_LOG_WORTHY_ERROR(res) || progressCtx.abortReason != eCURL_ABORT_REASON_NONE)
					{
						AAMP_LOG_NETWORK_ERROR (remoteUrl, AAMPNetworkErrorCurl, (int)(progressCtx.abortReason == eCURL_ABORT_REASON_NONE ? res : CURLE_PARTIAL_FILE));
					}
					//Attempt retry for local playback since rampdown is disabled for FOG
					//Attempt retry for partial downloads, which have a higher chance to succeed
					if((res == CURLE_COULDNT_CONNECT || (res == CURLE_OPERATION_TIMEDOUT && mIsLocalPlayback) || isDownloadStalled) && downloadAttempt < 2)
					{
						logprintf("Download failed due to curl timeout or isDownloadStalled:%d. Retrying!\n", isDownloadStalled);
						loopAgain = true;
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
						AAMPLOG_INFO("Curl download stall detected - curl result:%d abortReason:%d downloadTimeMS:%lld curlTimeout:%ld \n", res, progressCtx.abortReason, downloadTimeMS, curlDownloadTimeoutMS);
						//To avoid updateBasedonFragmentCached being called on rampdown and to be discarded from ABR
						http_code = CURLE_PARTIAL_FILE;
					}
				}

				if(gpGlobalConfig->enableMicroEvents && fileType != eMEDIATYPE_DEFAULT) //Unknown filetype
				{
					profiler.addtuneEvent(mediaType2Bucket(fileType),tStartTime,downloadTimeMS,(int)(http_code));
				}

				if(loopAgain) continue;

				double total, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize;
				long reqSize;
				AAMP_LogLevel reqEndLogLevel = eLOGLEVEL_INFO;

				curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME , &total);
				if(res != CURLE_OK || http_code == 0 || http_code >= 400 || total > 2.0 /*seconds*/)
				{
					reqEndLogLevel = eLOGLEVEL_WARN;
				}
				if (gpGlobalConfig->logging.isLogLevelAllowed(reqEndLogLevel))
				{
					double totalPerformRequest = (double)(downloadTimeMS)/1000;
					curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &resolve);
					curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect);
					curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &appConnect);
					curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &preTransfer);
					curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &startTransfer);
					curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &redirect);
					curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dlSize);
					curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &reqSize);
					AAMPLOG(reqEndLogLevel, "HttpRequestEnd: {\"url\":\"%.500s\",\"curlTime\":%2.4f,\"times\":{\"total\":%2.4f,\"connect\":%2.4f,\"startTransfer\":%2.4f,\"resolve\":%2.4f,\"appConnect\":%2.4f,\"preTransfer\":%2.4f,\"redirect\":%2.4f,\"dlSz\":%g,\"ulSz\":%ld},\"responseCode\":%ld}\n",
						((res == CURLE_OK) ? effectiveUrl : remoteUrl), // Effective URL could be different than remoteURL and it is updated only for CURLE_OK case
						totalPerformRequest,
						total, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize, reqSize, http_code);
				}
				break;
			}
		}

		if (http_code == 200 || http_code == 206 || http_code == CURLE_OPERATION_TIMEDOUT)
		{
			if (http_code == CURLE_OPERATION_TIMEDOUT && buffer->len > 0)
			{
				logprintf("Download timedout and obtained a partial buffer of size %d for a downloadTime=%lld and isDownloadStalled:%d\n", buffer->len, downloadTimeMS, isDownloadStalled);
			}

			if (downloadTimeMS > 0 && fileType == eMEDIATYPE_VIDEO && gpGlobalConfig->bEnableABR && (buffer->len > AAMP_ABR_THRESHOLD_SIZE))
			{
				{
					mAbrBitrateData.push_back(std::make_pair(aamp_GetCurrentTimeMS() ,((long)(buffer->len / downloadTimeMS)*8000)));
					//logprintf("CacheSz[%d]ConfigSz[%d] Storing Size [%d] bps[%ld]\n",mAbrBitrateData.size(),gpGlobalConfig->abrCacheLength, buffer->len, ((long)(buffer->len / downloadTimeMS)*8000));
					if(mAbrBitrateData.size() > gpGlobalConfig->abrCacheLength)
						mAbrBitrateData.erase(mAbrBitrateData.begin());
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
			if (CURLE_OK==curl_easy_getinfo(curl, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &expectedContentLength) && ((int)expectedContentLength>0) && ((int)expectedContentLength > (int)buffer->len))
			{
				//Note: For non-compressed data, Content-Length header and buffer size should be same. For gzipped data, 'Content-Length' will be <= deflated data.
				logprintf("AAMP content length mismatch expected %d got %d\n",(int)expectedContentLength, (int)buffer->len);
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
				else if (strstr(remoteUrl,"iframe"))
				{
					fileType = eMEDIATYPE_IFRAME;
				}

				if((downloadTimeMS > FRAGMENT_DOWNLOAD_WARNING_THRESHOLD) || (gpGlobalConfig->logging.latencyLogging[fileType] == true))
				{
					long long SequenceNo = GetSeqenceNumberfromURL(remoteUrl);
					logprintf("aampabr#T:%s,s:%lld,d:%lld,sz:%d,r:%ld,cerr:%d,hcode:%ld,n:%lld,estr:%ld,url:%s\n",MediaTypeString(fileType),(aamp_GetCurrentTimeMS()-downloadTimeMS),downloadTimeMS,int(buffer->len),mpStreamAbstractionAAMP->GetCurProfIdxBW(),res,http_code,SequenceNo,GetCurrentlyAvailableBandwidth(),remoteUrl);
				}
				ret             =       true;
			}
		}
		else
		{
			if (AAMP_IS_LOG_WORTHY_ERROR(res))
			{
				logprintf("BAD URL:%s\n", remoteUrl);
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
					MediaTypeString(fileType), (http_code < 100) ? "Curl" : "HTTP", http_code, remoteUrl);
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
							logprintf("Received FOG-Reason fogError: '%d'", *fogError);
						}
					}
				}

				//	get failed url from fog reason and update effectiveUrl
				if(effectiveUrl)
				{
					std::regex fromRegx("from:(.*),");
					std::smatch match;

					if (std::regex_search(httpRespHeaders[curlInstance].data, match, fromRegx) && match.size() > 1) {
						if (!match.str(1).empty())
						{
							strncpy(effectiveUrl, match.str(1).c_str(), MAX_URI_LENGTH-1);
							logprintf("Received FOG-Reason effectiveUrl: '%s'", effectiveUrl);
						}
					}
				}


                logprintf("Received FOG-Reason header: '%s'", httpRespHeaders[curlInstance].data.c_str());
                SendAnomalyEvent(ANOMALY_WARNING, "FOG-Reason:%s", httpRespHeaders[curlInstance].data.c_str());
            }
		}

		if (bitrate && (context.bitrate > 0))
		{
			logprintf("Received getfile Bitrate : %ld", context.bitrate);
			*bitrate = context.bitrate;
		}
		pthread_mutex_lock(&mLock);
	}
	else
	{
		logprintf("downloads disabled\n");
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
 * @brief Resolve URL from base and uri
 * @param[out] dst Destination buffer
 * @param base Base URL
 * @param uri manifest/ fragment uri
 */
void aamp_ResolveURL(char *dst, const char *base, const char *uri)
{
	if (memcmp(uri, "http://", 7) != 0 && memcmp(uri, "https://", 8) != 0) // explicit endpoint - needed for DAI playlist
	{
		strcpy(dst, base);

		if (uri[0] == '/')
		{ // absolute path; preserve only endpoint http://<endpoint>:<port>/
			dst = strstr(dst, "://");
			assert(dst);
			if (dst)
			{
				dst = strchr(dst + 3, '/');
			}
		}
		else
		{ // relative path; include base directory
			dst = strchr(dst, '/');
			assert(dst);
			for (;;)
			{
				char *next = strchr(dst + 1, '/');
				if (!next)
				{
					break;
				}
				dst = next;
			}
			dst++;
		}

		strcpy(dst, uri);

		if (strchr(uri, '?') == 0)//if uri doesn't already have url parameters, then copy from the parents(if they exist)
		{
			const char* params = strchr(base, '?');
			if (params)
				strcat(dst, params);
		}
	}
	else
		strcpy(dst, uri);
}

/**
 * @brief
 * @param url
 * @retval
 */
std::string aamp_getHostFromURL(char *url)
{
    std::string host = "comcast.net";
    int delimCnt = 0;
    char *ptr = url;
    char *hostStPtr = NULL;
    char *hostEndPtr = NULL;
    while(*ptr != '\0'){
        if(*ptr == '/')
        {
            delimCnt++;
            if(delimCnt == 2) hostStPtr=ptr+1;
            if(delimCnt == 3)
            {
                hostEndPtr=ptr;
                break;
            }
        }
        ptr++;
    }
    if((hostStPtr != hostEndPtr) && (hostStPtr != NULL) && (hostEndPtr != NULL))
    {
        host = std::string(hostStPtr,hostEndPtr-hostStPtr);
    }
    return host;
}

#define MAX_OVERRIDE 10

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
				logprintf("override!\n");
				return pChannelInfo.uri.c_str();
			}
		}
	}
	return NULL;
}

#ifdef IARM_MGR
//Enable below line while merging https://gerrit.teamccp.com/#/c/171105/ (
//XRE-12586 - Move aamp recipes from meta-rdk-video-comcast to meta-rdk-video)
//#ifdef IARM_MGR

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
                
                logprintf("GetTR181AAMPConfig: Got:%s En-Len:%d Dec-len:%d\n",strforLog.c_str(),param.paramLen,iConfigLen);
            }
            else
            {
                logprintf("GetTR181AAMPConfig: Not a string param type=%d or Invalid len:%d \n",param.paramtype, param.paramLen);
            }
        }
    }
    else
    {
        logprintf("GetTR181AAMPConfig: Failed to retrieve value result=%d\n",result);
    }
    return strConfig;
}
#endif


/**
 * @brief trim a string
 * @param[in][out] cmd Buffer containing string
 */
static void trim(char **cmd)
{
	std::string src = *cmd;
	size_t first = src.find_first_not_of(' ');
	if (first != std::string::npos)
	{
		size_t last = src.find_last_not_of(" \r\n");
		std::string dst = src.substr(first, (last - first + 1));
		strncpy(*cmd, (char*)dst.c_str(), dst.size());
		(*cmd)[dst.size()] = '\0';
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
static int ReadConfigStringHelper(const char *bufPtr, const char *prefixPtr, const char **valueCopyPtr)
{
    int rc = 0;
    size_t prefixLen = strlen(prefixPtr);
    size_t bufLen = strlen(bufPtr);
    if (bufLen > prefixLen && memcmp(bufPtr, prefixPtr, prefixLen) == 0)
    {
        bufPtr += prefixLen;
        while (*bufPtr == ' ')
        { // skip any whitespace
            bufPtr++;
        }
        *valueCopyPtr = strdup(bufPtr);
        if (*valueCopyPtr)
        {
            rc = 1;
        }
    }
    return rc;
}

/**
 * @brief Process config entries,i and update gpGlobalConfig params
 *        based on the config setting.
 * @param cfg config to process
 */
static void ProcessConfigEntry(char *cfg)
{
	if (cfg[0] != '#')
	{ // ignore comments

		//Removing unnecessary spaces and newlines
		trim(&cfg);

		double seconds = 0;
		int value;
        	char * tempUserAgent =NULL;
		if (sscanf(cfg, "map-mpd=%d\n", &gpGlobalConfig->mapMPD) == 1)
		{
			logprintf("map-mpd=%d\n", gpGlobalConfig->mapMPD);
		}
		else if (sscanf(cfg, "fog-dash=%d\n", &value) == 1)
		{
			gpGlobalConfig->fogSupportsDash = (value != 0);
			logprintf("fog-dash=%d\n", value);
		}
		else if (sscanf(cfg, "fog=%d\n", &value) == 1)
		{
			gpGlobalConfig->noFog = (value==0);
			logprintf("fog=%d\n", value);
		}
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		else if (sscanf(cfg, "harvest=%d", &gpGlobalConfig->harvest) == 1)
		{
			logprintf("harvest=%d\n", gpGlobalConfig->harvest);
		}
#endif
		else if (sscanf(cfg, "forceEC3=%d", &gpGlobalConfig->forceEC3) == 1)
		{
			logprintf("forceEC3=%d\n", gpGlobalConfig->forceEC3);
		}
		else if (sscanf(cfg, "disableEC3=%d", &gpGlobalConfig->disableEC3) == 1)
		{
			logprintf("disableEC3=%d\n", gpGlobalConfig->disableEC3);
		}
		else if (sscanf(cfg, "disableATMOS=%d", &gpGlobalConfig->disableATMOS) == 1)
		{
			logprintf("disableATMOS=%d\n", gpGlobalConfig->disableATMOS);
		}
		else if (sscanf(cfg, "live-offset=%d", &gpGlobalConfig->liveOffset) == 1)
		{
			VALIDATE_INT("live-offset", gpGlobalConfig->liveOffset, AAMP_LIVE_OFFSET)
			logprintf("live-offset=%d\n", gpGlobalConfig->liveOffset);
		}
		else if (sscanf(cfg, "cdvrlive-offset=%d", &gpGlobalConfig->cdvrliveOffset) == 1)
		{
			VALIDATE_INT("cdvrlive-offset", gpGlobalConfig->cdvrliveOffset, AAMP_CDVR_LIVE_OFFSET)
			logprintf("cdvrlive-offset=%d\n", gpGlobalConfig->cdvrliveOffset);
		}
		else if (sscanf(cfg, "disablePlaylistIndexEvent=%d", &gpGlobalConfig->disablePlaylistIndexEvent) == 1)
		{
			logprintf("disablePlaylistIndexEvent=%d\n", gpGlobalConfig->disablePlaylistIndexEvent);
		}
		else if (strcmp(cfg, "enableSubscribedTags") == 0)
		{
			gpGlobalConfig->enableSubscribedTags = true;
			logprintf("enableSubscribedTags set\n");
		}
		else if (strcmp(cfg, "disableSubscribedTags") == 0)
		{
			gpGlobalConfig->enableSubscribedTags = false;
			logprintf("disableSubscribedTags set\n");
		}
		else if (sscanf(cfg, "enableSubscribedTags=%d", &gpGlobalConfig->enableSubscribedTags) == 1)
		{
			logprintf("enableSubscribedTags=%d\n", gpGlobalConfig->enableSubscribedTags);
		}
		else if (sscanf(cfg, "networkTimeout=%ld", &gpGlobalConfig->networkTimeout) == 1)
		{
			VALIDATE_LONG("networkTimeout", gpGlobalConfig->networkTimeout, CURL_FRAGMENT_DL_TIMEOUT)
			logprintf("networkTimeout=%ld\n", gpGlobalConfig->networkTimeout);
		}
		else if (strcmp(cfg, "dash-ignore-base-url-if-slash") == 0)
		{
			gpGlobalConfig->dashIgnoreBaseURLIfSlash = true;
			logprintf("dash-ignore-base-url-if-slash set\n");
		}
		else if (strcmp(cfg, "license-anonymous-request") == 0)
		{
			gpGlobalConfig->licenseAnonymousRequest = true;
			logprintf("license-anonymous-request set\n");
		}
		else if ((strcmp(cfg, "info") == 0) && (!gpGlobalConfig->logging.debug))
		{
			gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_INFO);
			gpGlobalConfig->logging.info = true;
			logprintf("info logging %s\n", gpGlobalConfig->logging.info ? "on" : "off");
		}
		else if (strcmp(cfg, "failover") == 0)
		{
			gpGlobalConfig->logging.failover = true;
			logprintf("failover logging %s\n", gpGlobalConfig->logging.failover ? "on" : "off");
		}
		else if (strcmp(cfg, "gst") == 0)
		{
			gpGlobalConfig->logging.gst = !gpGlobalConfig->logging.gst;
			logprintf("gst logging %s\n", gpGlobalConfig->logging.gst ? "on" : "off");
		}
		else if (strcmp(cfg, "progress") == 0)
		{
			gpGlobalConfig->logging.progress = !gpGlobalConfig->logging.progress;
			logprintf("progress logging %s\n", gpGlobalConfig->logging.progress ? "on" : "off");
		}
		else if (strcmp(cfg, "debug") == 0)
		{
			gpGlobalConfig->logging.info = false;
			gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_TRACE);
			gpGlobalConfig->logging.debug = true;
			logprintf("debug logging %s\n", gpGlobalConfig->logging.debug ? "on" : "off");
		}
		else if (strcmp(cfg, "trace") == 0)
		{
			gpGlobalConfig->logging.trace = !gpGlobalConfig->logging.trace;
			logprintf("trace logging %s\n", gpGlobalConfig->logging.trace ? "on" : "off");
		}
		else if (strcmp(cfg, "curl") == 0)
		{
			gpGlobalConfig->logging.curl = !gpGlobalConfig->logging.curl;
			logprintf("curl logging %s\n", gpGlobalConfig->logging.curl ? "on" : "off");
		}
		else if (sscanf(cfg, "default-bitrate=%ld", &gpGlobalConfig->defaultBitrate) == 1)
		{
			VALIDATE_LONG("default-bitrate",gpGlobalConfig->defaultBitrate, DEFAULT_INIT_BITRATE)
			logprintf("aamp default-bitrate: %ld\n", gpGlobalConfig->defaultBitrate);
		}
		else if (sscanf(cfg, "default-bitrate-4k=%ld", &gpGlobalConfig->defaultBitrate4K) == 1)
		{
			VALIDATE_LONG("default-bitrate-4k", gpGlobalConfig->defaultBitrate4K, DEFAULT_INIT_BITRATE_4K)
			logprintf("aamp default-bitrate-4k: %ld\n", gpGlobalConfig->defaultBitrate4K);
		}
		else if (strcmp(cfg, "abr") == 0)
		{
			gpGlobalConfig->bEnableABR = !gpGlobalConfig->bEnableABR;
			logprintf("abr %s\n", gpGlobalConfig->bEnableABR ? "on" : "off");
		}
		else if (sscanf(cfg, "abr-cache-life=%d", &gpGlobalConfig->abrCacheLife) == 1)
		{
			gpGlobalConfig->abrCacheLife *= 1000;
			logprintf("aamp abr cache lifetime: %ldmsec\n", gpGlobalConfig->abrCacheLife);
		}
		else if (sscanf(cfg, "abr-cache-length=%d", &gpGlobalConfig->abrCacheLength) == 1)
		{
			VALIDATE_INT("abr-cache-length", gpGlobalConfig->abrCacheLength, DEFAULT_ABR_CACHE_LENGTH)
			logprintf("aamp abr cache length: %ld\n", gpGlobalConfig->abrCacheLength);
		}
		else if (sscanf(cfg, "abr-cache-outlier=%d", &gpGlobalConfig->abrOutlierDiffBytes) == 1)
		{
			VALIDATE_LONG("abr-cache-outlier", gpGlobalConfig->abrOutlierDiffBytes, DEFAULT_ABR_OUTLIER)
			logprintf("aamp abr outlier in bytes: %ld\n", gpGlobalConfig->abrOutlierDiffBytes);
		}
		else if (sscanf(cfg, "abr-skip-duration=%d", &gpGlobalConfig->abrSkipDuration) == 1)
		{
			VALIDATE_INT("abr-skip-duration",gpGlobalConfig->abrSkipDuration, DEFAULT_ABR_SKIP_DURATION)
			logprintf("aamp abr skip duration: %d\n", gpGlobalConfig->abrSkipDuration);
		}
		else if (sscanf(cfg, "abr-nw-consistency=%d", &gpGlobalConfig->abrNwConsistency) == 1)
		{
			VALIDATE_LONG("abr-nw-consistency", gpGlobalConfig->abrNwConsistency, DEFAULT_ABR_NW_CONSISTENCY_CNT)
			logprintf("aamp abr NetworkConsistencyCnt: %d\n", gpGlobalConfig->abrNwConsistency);
		}
		else if (sscanf(cfg, "flush=%d", &gpGlobalConfig->gPreservePipeline) == 1)
		{
			logprintf("aamp flush=%d\n", gpGlobalConfig->gPreservePipeline);
		}
		else if (sscanf(cfg, "demux-hls-audio-track=%d", &gpGlobalConfig->gAampDemuxHLSAudioTsTrack) == 1)
		{ // default 1, set to 0 for hw demux audio ts track
			logprintf("demux-hls-audio-track=%d\n", gpGlobalConfig->gAampDemuxHLSAudioTsTrack);
		}
		else if (sscanf(cfg, "demux-hls-video-track=%d", &gpGlobalConfig->gAampDemuxHLSVideoTsTrack) == 1)
		{ // default 1, set to 0 for hw demux video ts track
			logprintf("demux-hls-video-track=%d\n", gpGlobalConfig->gAampDemuxHLSVideoTsTrack);
		}
		else if (sscanf(cfg, "demux-hls-video-track-tm=%d", &gpGlobalConfig->demuxHLSVideoTsTrackTM) == 1)
		{ // default 0, set to 1 to demux video ts track during trickmodes
			logprintf("demux-hls-video-track-tm=%d\n", gpGlobalConfig->demuxHLSVideoTsTrackTM);
		}
		else if (sscanf(cfg, "demuxed-audio-before-video=%d", &gpGlobalConfig->demuxedAudioBeforeVideo) == 1)
		{ // default 0, set to 1 to send audio es before video in case of s/w demux.
			logprintf("demuxed-audio-before-video=%d\n", gpGlobalConfig->demuxedAudioBeforeVideo);
		}
		else if (sscanf(cfg, "throttle=%d", &gpGlobalConfig->gThrottle) == 1)
		{ // default is true; used with restamping?
			logprintf("aamp throttle=%d\n", gpGlobalConfig->gThrottle);
		}
		else if (sscanf(cfg, "min-vod-cache=%d", &gpGlobalConfig->minVODCacheSeconds) == 1)
		{ // override for VOD cache
			VALIDATE_INT("min-vod-cache", gpGlobalConfig->minVODCacheSeconds, DEFAULT_MINIMUM_CACHE_VOD_SECONDS)
			logprintf("min-vod-cache=%d\n", gpGlobalConfig->minVODCacheSeconds);
		}
		else if (sscanf(cfg, "buffer-health-monitor-delay=%d", &gpGlobalConfig->bufferHealthMonitorDelay) == 1)
		{ // override for buffer health monitor delay after tune/ seek
			VALIDATE_INT("buffer-health-monitor-delay", gpGlobalConfig->bufferHealthMonitorDelay, DEFAULT_BUFFER_HEALTH_MONITOR_DELAY)
			logprintf("buffer-health-monitor-delay=%d\n", gpGlobalConfig->bufferHealthMonitorDelay);
		}
		else if (sscanf(cfg, "buffer-health-monitor-interval=%d", &gpGlobalConfig->bufferHealthMonitorInterval) == 1)
		{ // override for buffer health monitor interval
			VALIDATE_INT("buffer-health-monitor-interval", gpGlobalConfig->bufferHealthMonitorInterval, DEFAULT_BUFFER_HEALTH_MONITOR_INTERVAL)
			logprintf("buffer-health-monitor-interval=%d\n", gpGlobalConfig->bufferHealthMonitorInterval);
		}
		else if (sscanf(cfg, "preferred-drm=%d", &value) == 1)
		{ // override for preferred drm value
			if(value <= eDRM_NONE || value > eDRM_PlayReady)
			{
				logprintf("preferred-drm=%d is unsupported\n", value);
			}
			else
			{
				gpGlobalConfig->isUsingLocalConfigForPreferredDRM = true;
				gpGlobalConfig->preferredDrm = (DRMSystems) value;
			}
			logprintf("preferred-drm=%s\n", GetDrmSystemName(gpGlobalConfig->preferredDrm));
		}
		else if (sscanf(cfg, "playready-output-protection=%d\n", &value) == 1)
		{
			gpGlobalConfig->enablePROutputProtection = (value != 0);
			logprintf("playready-output-protection is %s\n", (value ? "on" : "off"));
		}
		else if (sscanf(cfg, "live-tune-event = %d", &value) == 1)
                { // default is 0; set 1 for sending tuned for live
                        logprintf("live-tune-event = %d\n", value);
                        if (value >= 0 && value < eTUNED_EVENT_MAX)
                        {
                                gpGlobalConfig->tunedEventConfigLive = (TunedEventConfig)(value);
                        }
                }
                else if (sscanf(cfg, "vod-tune-event = %d", &value) == 1)
                { // default is 0; set 1 for sending tuned event for vod
                        logprintf("vod-tune-event = %d\n", value);
                        if (value >= 0 && value < eTUNED_EVENT_MAX)
                        {
                                gpGlobalConfig->tunedEventConfigVOD = (TunedEventConfig)(value);
                        }
                }
		else if (sscanf(cfg, "playlists-parallel-fetch=%d\n", &value) == 1)
		{
			gpGlobalConfig->playlistsParallelFetch = (value != 0);
			logprintf("playlists-parallel-fetch=%d\n", value);
		}
		else if (sscanf(cfg, "pre-fetch-iframe-playlist=%d\n", &value) == 1)
		{
			gpGlobalConfig->prefetchIframePlaylist = (value != 0);
			logprintf("pre-fetch-iframe-playlist=%d\n", value);
		}
		else if (sscanf(cfg, "hls-av-sync-use-start-time=%d\n", &value) == 1)
		{
			gpGlobalConfig->hlsAVTrackSyncUsingStartTime = (value != 0);
			logprintf("hls-av-sync-use-start-time=%d\n", value);
		}
		else if (sscanf(cfg, "mpd-discontinuity-handling=%d\n", &value) == 1)
		{
			gpGlobalConfig->mpdDiscontinuityHandling = (value != 0);
			logprintf("mpd-discontinuity-handling=%d\n", value);
		}
		else if (sscanf(cfg, "mpd-discontinuity-handling-cdvr=%d\n", &value) == 1)
		{
			gpGlobalConfig->mpdDiscontinuityHandlingCdvr = (value != 0);
			logprintf("mpd-discontinuity-handling-cdvr=%d\n", value);
		}
		else if(ReadConfigStringHelper(cfg, "license-server-url=", (const char**)&gpGlobalConfig->licenseServerURL))
		{
			gpGlobalConfig->licenseServerLocalOverride = true;
			logprintf("license-server-url=%s\n", gpGlobalConfig->licenseServerURL);
		}
		else if(sscanf(cfg, "vod-trickplay-fps=%d\n", &gpGlobalConfig->vodTrickplayFPS) == 1)
		{
			VALIDATE_INT("vod-trickplay-fps", gpGlobalConfig->vodTrickplayFPS, TRICKPLAY_NETWORK_PLAYBACK_FPS)
			if(gpGlobalConfig->vodTrickplayFPS != TRICKPLAY_NETWORK_PLAYBACK_FPS)
				gpGlobalConfig->vodTrickplayFPSLocalOverride = true;
			logprintf("vod-trickplay-fps=%d\n", gpGlobalConfig->vodTrickplayFPS);
		}
		else if(sscanf(cfg, "linear-trickplay-fps=%d\n", &gpGlobalConfig->linearTrickplayFPS) == 1)
		{
			VALIDATE_INT("linear-trickplay-fps", gpGlobalConfig->linearTrickplayFPS, TRICKPLAY_TSB_PLAYBACK_FPS)
			if (gpGlobalConfig->linearTrickplayFPS != TRICKPLAY_TSB_PLAYBACK_FPS)
				gpGlobalConfig->linearTrickplayFPSLocalOverride = true;
			logprintf("linear-trickplay-fps=%d\n", gpGlobalConfig->linearTrickplayFPS);
		}
		else if (sscanf(cfg, "report-progress-interval=%d\n", &gpGlobalConfig->reportProgressInterval) == 1)
		{
			VALIDATE_INT("report-progress-interval", gpGlobalConfig->reportProgressInterval, DEFAULT_REPORT_PROGRESS_INTERVAL)
			logprintf("report-progress-interval=%d\n", gpGlobalConfig->reportProgressInterval);
		}
		else if (ReadConfigStringHelper(cfg, "http-proxy=", &gpGlobalConfig->httpProxy))
		{
			logprintf("http-proxy=%s\n", gpGlobalConfig->httpProxy);
		}
		else if (strcmp(cfg, "force-http") == 0)
		{
			gpGlobalConfig->bForceHttp = !gpGlobalConfig->bForceHttp;
			logprintf("force-http: %s\n", gpGlobalConfig->bForceHttp ? "on" : "off");
		}
		else if (sscanf(cfg, "internal-retune=%d\n", &value) == 1)
		{
			gpGlobalConfig->internalReTune = (value != 0);
			logprintf("internal-retune=%d\n", (int)value);
		}
		else if (sscanf(cfg, "gst-buffering-before-play=%d\n", &value) == 1)
		{
			gpGlobalConfig->gstreamerBufferingBeforePlay = (value != 0);
			logprintf("gst-buffering-before-play=%d\n", (int)gpGlobalConfig->gstreamerBufferingBeforePlay);
		}
		else if (sscanf(cfg, "re-tune-on-buffering-timeout=%d\n", &value) == 1)
		{
			gpGlobalConfig->reTuneOnBufferingTimeout = (value != 0);
			logprintf("re-tune-on-buffering-timeout=%d\n", (int)gpGlobalConfig->reTuneOnBufferingTimeout);
		}
		else if (strcmp(cfg, "audioLatencyLogging") == 0)
		{
			gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_AUDIO] = true;
			logprintf("audioLatencyLogging is %s\n", gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_AUDIO]? "enabled" : "disabled");
		}
		else if (strcmp(cfg, "videoLatencyLogging") == 0)
		{
			gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_VIDEO] = true;
			logprintf("videoLatencyLogging is %s\n", gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_VIDEO]? "enabled" : "disabled");
		}
		else if (strcmp(cfg, "manifestLatencyLogging") == 0)
		{
			gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_MANIFEST] = true;
			logprintf("manifestLatencyLogging is %s\n", gpGlobalConfig->logging.latencyLogging[eMEDIATYPE_MANIFEST]? "enabled" : "disabled");
		}
		else if (sscanf(cfg, "iframe-default-bitrate=%ld", &gpGlobalConfig->iframeBitrate) == 1)
		{
			VALIDATE_LONG("iframe-default-bitrate",gpGlobalConfig->iframeBitrate, 0)
			logprintf("aamp iframe-default-bitrate: %ld\n", gpGlobalConfig->iframeBitrate);
		}
		else if (sscanf(cfg, "iframe-default-bitrate-4k=%ld", &gpGlobalConfig->iframeBitrate4K) == 1)
		{
			VALIDATE_LONG("iframe-default-bitrate-4k",gpGlobalConfig->iframeBitrate4K, 0)
			logprintf("aamp iframe-default-bitrate-4k: %ld\n", gpGlobalConfig->iframeBitrate4K);
		}
		else if (strcmp(cfg, "aamp-audio-only-playback") == 0)
		{
			gpGlobalConfig->bAudioOnlyPlayback = true;
			logprintf("aamp-audio-only-playback is %s\n", gpGlobalConfig->bAudioOnlyPlayback ? "enabled" : "disabled");
		}
		else if (sscanf(cfg, "license-retry-wait-time=%d", &gpGlobalConfig->licenseRetryWaitTime) == 1)
		{
			logprintf("license-retry-wait-time: %d\n", gpGlobalConfig->licenseRetryWaitTime);
		}
		else if (sscanf(cfg, "fragment-cache-length=%d", &gpGlobalConfig->maxCachedFragmentsPerTrack) == 1)
		{
			VALIDATE_INT("fragment-cache-length", gpGlobalConfig->maxCachedFragmentsPerTrack, DEFAULT_CACHED_FRAGMENTS_PER_TRACK)
			logprintf("aamp fragment cache length: %d\n", gpGlobalConfig->maxCachedFragmentsPerTrack);
		}
		else if (sscanf(cfg, "pts-error-threshold=%d", &gpGlobalConfig->ptsErrorThreshold) == 1)
		{
			VALIDATE_INT("pts-error-threshold", gpGlobalConfig->ptsErrorThreshold, MAX_PTS_ERRORS_THRESHOLD)
			logprintf("aamp pts-error-threshold: %d\n", gpGlobalConfig->ptsErrorThreshold);
		}
		else if(sscanf(cfg, "max-playlist-cache=%ld", &gpGlobalConfig->gMaxPlaylistCacheSize) == 1)
		{
			// Read value in KB , convert it to bytes
			gpGlobalConfig->gMaxPlaylistCacheSize = gpGlobalConfig->gMaxPlaylistCacheSize * 1024;
			VALIDATE_INT("max-playlist-cache", gpGlobalConfig->gMaxPlaylistCacheSize, MAX_PLAYLIST_CACHE_SIZE);
			logprintf("aamp max-playlist-cache: %ld\n", gpGlobalConfig->gMaxPlaylistCacheSize);
		}
		else if(sscanf(cfg, "dash-max-drm-sessions=%d", &gpGlobalConfig->dash_MaxDRMSessions) == 1)
		{
			// Read value in KB , convert it to bytes
			if(gpGlobalConfig->dash_MaxDRMSessions < MIN_DASH_DRM_SESSIONS || gpGlobalConfig->dash_MaxDRMSessions > MAX_DASH_DRM_SESSIONS)
			{
				logprintf("Out of range value for dash-max-drm-sessions, setting to %d;Expected Range (%d - %d)\n",
						MIN_DASH_DRM_SESSIONS, MIN_DASH_DRM_SESSIONS, MAX_DASH_DRM_SESSIONS);
				gpGlobalConfig->dash_MaxDRMSessions = MIN_DASH_DRM_SESSIONS;
			}
			logprintf("aamp dash-max-drm-sessions: %d\n", gpGlobalConfig->dash_MaxDRMSessions);
		}
		else if (ReadConfigStringHelper(cfg, "user-agent=", (const char**)&tempUserAgent))
		{
			if(tempUserAgent)
			{
				if(strlen(tempUserAgent) < AAMP_USER_AGENT_MAX_CONFIG_LEN)
				{
					logprintf("user-agent=%s\n", tempUserAgent);
					gpGlobalConfig->aamp_SetBaseUserAgentString(tempUserAgent);
				}
				else
				{
					logprintf("user-agent len is more than %d , Hence Ignoring \n", AAMP_USER_AGENT_MAX_CONFIG_LEN);
				}
 
				free(tempUserAgent);
				tempUserAgent = NULL;
			}
		}
		else if (sscanf(cfg, "wait-time-before-retry-http-5xx-ms=%d", &gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS) == 1)
		{
			VALIDATE_INT("wait-time-before-retry-http-5xx-ms", gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS, DEFAULT_WAIT_TIME_BEFORE_RETRY_HTTP_5XX_MS);
			logprintf("aamp wait-time-before-retry-http-5xx-ms: %d\n", gpGlobalConfig->waitTimeBeforeRetryHttp5xxMS);
		}
		else if (sscanf(cfg, "curl-stall-timeout=%ld", &gpGlobalConfig->curlStallTimeout) == 1)
		{
			//Not calling VALIDATE_LONG since zero is supported
			logprintf("aamp curl-stall-timeout: %ld\n", gpGlobalConfig->curlStallTimeout);
		}
		else if (sscanf(cfg, "curl-download-start-timeout=%ld", &gpGlobalConfig->curlDownloadStartTimeout) == 1)
		{
			//Not calling VALIDATE_LONG since zero is supported
			logprintf("aamp curl-download-start-timeout: %ld\n", gpGlobalConfig->curlDownloadStartTimeout);
		}
		else if (sscanf(cfg, "client-dai=%d\n", &value) == 1)
		{
			gpGlobalConfig->enableClientDai = (value == 1);
			logprintf("Client side DAI: %s\n", gpGlobalConfig->enableClientDai ? "ON" : "OFF");
		}
		else if (sscanf(cfg, "ad-from-cdn-only=%d\n", &value) == 1)
		{
			gpGlobalConfig->playAdFromCDN = (value == 1);
			logprintf("Ad playback from CDN only: %s\n", gpGlobalConfig->playAdFromCDN ? "ON" : "OFF");
		}
		else if (mChannelOverrideMap.size() < MAX_OVERRIDE)
		{
			if (cfg[0] == '*')
			{
				char *delim = strchr(cfg, ' ');
				if (delim)
				{
					//Populate channel map from aamp.cfg
					// new wildcard matching for overrides - allows *HBO to remap any url including "HBO"
					logprintf("aamp override:\n%s\n", cfg);
					ChannelInfo channelInfo;
					char *channelStr = &cfg[1];
					char *token = strtok(channelStr, " ");
					while (token != NULL)
					{
						if (memcmp(token, "http", 4) == 0)
							channelInfo.uri = token;
						else
							channelInfo.name = token;
						token = strtok(NULL, " ");
					}
					mChannelOverrideMap.push_back(channelInfo);
				}
			}
		}
	}
}

/**
 * @brief Load AAMP configuration file
 */
void PrivateInstanceAAMP::LazilyLoadConfigIfNeeded(void)
{
	if (!gpGlobalConfig)
	{
		gpGlobalConfig = new GlobalConfigAAMP();
#ifdef IARM_MGR 
        logprintf("LazilyLoadConfigIfNeeded calling  GetTR181AAMPConfig  \n");
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
                    logprintf("LazilyLoadConfigIfNeeded Non Compliant char[0x%X] found, Ignoring whole config  \n",cloudConf[i]);
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
                        char * cstrCmd = (char *)malloc(line.length() + 1);
                        if (cstrCmd)
                        {
                            strcpy(cstrCmd, line.c_str());
                            logprintf("LazilyLoadConfigIfNeeded aamp-cmd:[%s]\n", cstrCmd);
                            ProcessConfigEntry(cstrCmd);
                            free(cstrCmd);
                        }
                    }
                }
            }
            free(cloudConf); // allocated by base64_Decode in GetTR181AAMPConfig
        }
#endif

#ifdef WIN32
		AampLogManager mLogManager;
		FILE *f = fopen(mLogManager.getAampCfgPath(), "rb");
#elif defined(__APPLE__)
		std::string cfgPath(getenv("HOME"));
		cfgPath += "/aamp.cfg";
		FILE *f = fopen(cfgPath.c_str(), "rb");
#else
#ifdef AAMP_CPC // Comcast builds
        // AAMP_ENABLE_OPT_OVERRIDE is only added for PROD builds.
        const char *env_aamp_enable_opt = getenv("AAMP_ENABLE_OPT_OVERRIDE");
#else
        const char *env_aamp_enable_opt = "true";
#endif

        FILE *f = NULL;
        if(env_aamp_enable_opt)
        {
            f = fopen("/opt/aamp.cfg", "rb");
        }
#endif
		if (f)
		{
			logprintf("opened aamp.cfg\n");
			char buf[MAX_URI_LENGTH * 2];
			while (fgets(buf, sizeof(buf), f))
			{
				ProcessConfigEntry(buf);
			}
			fclose(f);
		}
		else
		{
			logprintf("Failed to open aamp.cfg\n");
		}

		const char *env_aamp_force_aac = getenv("AAMP_FORCE_AAC");
		if(env_aamp_force_aac)
		{
			logprintf("AAMP_FORCE_AAC present: Changing preference to AAC over ATMOS & DD+\n");
			gpGlobalConfig->disableEC3 = 1;
			gpGlobalConfig->disableATMOS = 1;
		}

		const char *env_aamp_min_vod_cache = getenv("AAMP_MIN_VOD_CACHE");
		if(env_aamp_min_vod_cache)
		{
			int minVodCache = 0;
			if(sscanf(env_aamp_min_vod_cache,"%d",&minVodCache))
			{
				logprintf("AAMP_MIN_VOD_CACHE present: Changing min vod cache to %d seconds\n",minVodCache);
				gpGlobalConfig->minVODCacheSeconds = minVodCache;
			}
		}

		const char *env_enable_micro_events = getenv("TUNE_MICRO_EVENTS");
		if(env_enable_micro_events)
		{
			logprintf("TUNE_MICRO_EVENTS present: Enabling TUNE_MICRO_EVENTS.\n");
			gpGlobalConfig->enableMicroEvents = true;
		}

		const char *env_enable_cdai = getenv("CLIENT_SIDE_DAI");
		if(env_enable_cdai)
		{
			logprintf("CLIENT_SIDE_DAI present: Enabling CLIENT_SIDE_DAI.\n");
			gpGlobalConfig->enableClientDai = true;
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
	if ((mDiscontinuityTuneOperationId != 0) && (!newTune || mState == eSTATE_IDLE))
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
	//reset discontinuity related flags
	mProcessingDiscontinuity = false;
	pthread_mutex_unlock(&mLock);

	if (mpStreamAbstractionAAMP)
	{
		mpStreamAbstractionAAMP->Stop(false);
		delete mpStreamAbstractionAAMP;
		mpStreamAbstractionAAMP = NULL;
	}

	pthread_mutex_lock(&mLock);
	mFormat = FORMAT_INVALID;
	pthread_mutex_unlock(&mLock);
	if (streamerIsActive)
	{
#ifdef AAMP_STOP_SINK_ON_SEEK
		const bool forceStop = true;
		AAMPEvent event;
		event.type = AAMP_EVENT_CC_HANDLE_RECEIVED;
		event.data.ccHandle.handle = 0;
		traceprintf("%s:%d Sending AAMP_EVENT_CC_HANDLE_RECEIVED with NULL handle\n",__FUNCTION__, __LINE__);
		SendEventSync(event);
		logprintf("%s:%d Sent AAMP_EVENT_CC_HANDLE_RECEIVED with NULL handle\n",__FUNCTION__, __LINE__);
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
	) : aamp(NULL), mInternalStreamSink(NULL), mJSBinding_DL()
{
#ifdef SUPPORT_JS_EVENTS
#ifdef AAMP_WPEWEBKIT_JSBINDINGS //aamp_LoadJS defined in libaampjsbindings.so
	const char* szJSLib = "libaampjsbindings.so";
#else
	const char* szJSLib = "libaamp.so";
#endif
	mJSBinding_DL = dlopen(szJSLib, RTLD_GLOBAL | RTLD_LAZY);
	logprintf("[AAMP_JS] dlopen(\"%s\")=%p\n", szJSLib, mJSBinding_DL);
#endif
	aamp = new PrivateInstanceAAMP();
	if (NULL == streamSink)
	{
		mInternalStreamSink = new AAMPGstPlayer(aamp
#ifdef AAMP_RENDER_IN_APP
		, exportFrames
#endif
		);
		streamSink = mInternalStreamSink;
	}
	aamp->SetStreamSink(streamSink);

}


/**
 * @brief PlayerInstanceAAMP Destructor
 */
PlayerInstanceAAMP::~PlayerInstanceAAMP()
{
	if (aamp)
	{
		aamp->Stop();
#ifdef AAMP_MPD_DRM
		//Clear session data on clean up of last PlayerInstanceAAMP
		if (gActivePrivAAMPs.size() == 1)
		{
			AampDRMSessionManager::getInstance()->clearSessionData();
		}
#endif /*AAMP_MPD_DRM*/
		delete aamp;
	}
	if (mInternalStreamSink)
	{
		delete mInternalStreamSink;
	}
#ifdef SUPPORT_JS_EVENTS 
	if (mJSBinding_DL && gActivePrivAAMPs.empty())
	{
		logprintf("[AAMP_JS] dlclose(%p)\n", mJSBinding_DL);
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
        goto EXIT;
    }
    if(mkfifo(strAAMPPipeName, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) == -1) {
        if(errno == EEXIST) {
            // Pipe exists
            //logprintf("%s:CreatePipe: Pipe already exists",__FUNCTION__);
            retVal = true;
        }
        else {
            // Error
            logprintf("%s:CreatePipe: Failed to create named pipe %s for reading errno = %d (%s)\n",
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
            logprintf("%s:OpenPipe: Failed to open named pipe %s for writing errno = %d (%s)\n",
                      __FUNCTION__,strAAMPPipeName, errno, strerror(errno));
        }
        else {
            // Success
            retVal = true;
        }
    }
EXIT:
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
                logprintf("Error writing data written = %d, size = %d errno = %d (%s)\n",
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
    logprintf("AAMP=>XRE: %s\n",data);
#endif
}

/**
 * @brief Stop playback and release resources.
 *
 */
void PlayerInstanceAAMP::Stop(void)
{
	PrivAAMPState state;
	aamp->GetState(state);

	//state will be eSTATE_IDLE or eSTATE_RELEASED, right after an init or post-processing of a Stop call
	if (state == eSTATE_IDLE || state == eSTATE_RELEASED)
	{
		logprintf("aamp_stop ignored since already at eSTATE_IDLE\n");
		return;
	}

	logprintf("aamp_stop PlayerState=%d\n",state);
	if(gpGlobalConfig->enableMicroEvents && (eSTATE_ERROR == state) && !(aamp->IsTuneCompleted()))
	{
		/*Sending metrics on tune Error; excluding mid-stream failure cases & aborted tunes*/
		aamp->sendTuneMetrics(false);
	}
	aamp->SetState(eSTATE_IDLE);

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
	AAMPLOG_INFO("Stopping Playback at Position '%lld'.\n", aamp->GetPositionMs());
	aamp->Stop();
}


/**
 * @brief de-fog playback URL to play directly from CDN instead of fog
 * @param[in][out] dst Buffer containing URL
 */
static void DeFog(char *dst)
{
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
}

/**
 * @brief Encode URL
 *
 * @param[in] inSrc - Input URL
 * @param[out] outStr - Encoded URL
 * @return Encoding status
 */
bool UrlEncode(const char *inSrc, std::string &outStr)
{
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
	return true;
}

/**
 * @brief
 * @param string
 * @param existingSubStringToReplace
 * @param replacementString
 * @retval
 */
static bool replace_cstring( char *string, const char *existingSubStringToReplace, const char *replacementString )
{
	char *insertionPtr = strstr(string, existingSubStringToReplace);
	if (insertionPtr)
	{
		size_t charsToInsert = strlen(replacementString);
		size_t charsToRemove = strlen(existingSubStringToReplace);
		size_t charsToKeep = strlen(&insertionPtr[charsToRemove]);
		memmove(&insertionPtr[charsToInsert], &insertionPtr[charsToRemove], charsToKeep);
          	insertionPtr[charsToInsert+charsToKeep] = 0x00;
		memcpy(insertionPtr, replacementString, charsToInsert);
		return true;
	}
	return false;
}
/**
 * @brief Common tune operations used on Tune, Seek, SetRate etc
 * @param tuneType type of tune
 */
void PrivateInstanceAAMP::TuneHelper(TuneType tuneType)
{
	bool newTune;
	lastUnderFlowTimeMs[eMEDIATYPE_VIDEO] = 0;
	lastUnderFlowTimeMs[eMEDIATYPE_AUDIO] = 0;
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

	TeardownStream(newTune|| (eTUNETYPE_RETUNE == tuneType));

	if (eTUNETYPE_RETUNE == tuneType)
	{
		seek_pos_seconds = GetPositionMs()/1000;
	}


	if (newTune)
	{
		// send previouse tune VideoEnd Metrics data
		// this is done here because events are cleared on stop and there is chance that event may not get sent
		SendVideoEndEvent();

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
		logprintf("%s:%d Updated seek_pos_seconds %f \n",__FUNCTION__,__LINE__, seek_pos_seconds);
	}

	if (mIsDash)
	{ // mpd
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
		mpStreamAbstractionAAMP->SetCDAIObject(mCdaiObject);
#endif
	}
	else
	{ // m3u8
		bool enableThrottle = true;
		if (!gpGlobalConfig->gThrottle)
		{
			enableThrottle = false;
		}
		mpStreamAbstractionAAMP = new StreamAbstractionAAMP_HLS(this, playlistSeekPos, rate, enableThrottle);
	}
	mInitSuccess = true;
	AAMPStatusType retVal = mpStreamAbstractionAAMP->Init(tuneType);
	if (retVal != eAAMPSTATUS_OK)
	{
		// Check if the seek position is beyond the duration
		if(retVal == eAAMPSTATUS_SEEK_RANGE_ERROR)
		{
			logprintf("mpStreamAbstractionAAMP Init Failed.Seek Position(%f) out of range(%lld)\n",mpStreamAbstractionAAMP->GetStreamPosition(),(GetDurationMs()/1000));
			NotifyEOSReached();
		}
		else
		{
			logprintf("mpStreamAbstractionAAMP Init Failed.Error(%d)\n",retVal);
			SendErrorEvent(AAMP_TUNE_INIT_FAILED);
			//event.data.mediaError.description = "kECFileNotFound (90)";
			//event.data.mediaError.playerRecoveryEnabled = false;
		}
		mInitSuccess = false;
		return;
	}
	else
	{
		double updatedSeekPosition = mpStreamAbstractionAAMP->GetStreamPosition();
		seek_pos_seconds = updatedSeekPosition + culledSeconds;
#ifndef AAMP_STOP_SINK_ON_SEEK
		logprintf("%s:%d Updated seek_pos_seconds %f \n",__FUNCTION__,__LINE__, seek_pos_seconds);
		if (!mIsDash)
		{
			//Live adjust or syncTrack occurred, sent an updated flush event
			if ((!newTune && gpGlobalConfig->gAampDemuxHLSVideoTsTrack) || gpGlobalConfig->gPreservePipeline)
			{
				mStreamSink->Flush(mpStreamAbstractionAAMP->GetFirstPTS(), rate);
			}
		}
		else
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

		mpStreamAbstractionAAMP->GetStreamFormat(mFormat, mAudioFormat);
		AAMPLOG_INFO("TuneHelper : mFormat %d, mAudioFormat %d\n", mFormat, mAudioFormat);
		mStreamSink->SetVideoZoom(zoom_mode);
		mStreamSink->SetVideoMute(video_muted);
		mStreamSink->SetAudioVolume(audio_volume);
		mStreamSink->Configure(mFormat, mAudioFormat, mpStreamAbstractionAAMP->GetESChangeStatus());
		mpStreamAbstractionAAMP->ResetESChangeStatus();
		mpStreamAbstractionAAMP->Start();
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
 * @param  contentType - content Type.
 */
void PlayerInstanceAAMP::Tune(const char *mainManifestUrl, const char *contentType, bool bFirstAttempt, bool bFinalAttempt,const char *traceUUID)
{
	PrivAAMPState state;
	aamp->GetState(state);
	if (state == eSTATE_RELEASED)
	{
		aamp->SetState(eSTATE_IDLE); //To send the IDLE status event for first channel tune after bootup
	}
	AampCacheHandler::GetInstance()->StartPlaylistCache();
	aamp->Tune(mainManifestUrl, contentType, bFirstAttempt, bFinalAttempt,traceUUID);
}


/**
 * @brief Tune to a URL.
 *
 * @param  mainManifestUrl - HTTP/HTTPS url to be played.
 * @param  contentType - content Type.
 */
void PrivateInstanceAAMP::Tune(const char *mainManifestUrl, const char *contentType, bool bFirstAttempt, bool bFinalAttempt,const char *pTraceID)
{
	AAMPLOG_TRACE("aamp_tune: original URL: %s\n", mainManifestUrl);

	TuneType tuneType =  eTUNETYPE_NEW_NORMAL;
	gpGlobalConfig->logging.setLogLevel(eLOGLEVEL_INFO);
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

	if (pipeline_paused)
	{
		// resume downloads and clear paused flag. state change will be done
		// on streamSink configuration.
		pipeline_paused = false;
		ResumeDownloads();
	}

	if (-1 != seek_pos_seconds)
	{
		logprintf("PrivateInstanceAAMP::%s:%d seek position already set, so eTUNETYPE_NEW_SEEK\n", __FUNCTION__, __LINE__);
		tuneType = eTUNETYPE_NEW_SEEK;
	}
	else
	{
		seek_pos_seconds = 0;
	}

	for(int i = 0; i < MAX_CURL_INSTANCE_COUNT; i++)
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

	strncpy(manifestUrl, mainManifestUrl, MAX_URI_LENGTH);
	manifestUrl[MAX_URI_LENGTH-1] = '\0';

	mIsDash = !strstr(mainManifestUrl, "m3u8");
	mIsVSS = (strstr(mainManifestUrl, VSS_MARKER) || strstr(mainManifestUrl, VSS_MARKER_FOG));
	mTuneCompleted 	=	false;
	mTSBEnabled	=	false;
	mIscDVR = strstr(mainManifestUrl, "cdvr-");
	mIsLocalPlayback = (aamp_getHostFromURL(manifestUrl).find(LOCAL_HOST_IP) != std::string::npos);
	mPersistedProfileIndex	=	-1;
	mCurrentDrm = eDRM_NONE;
	mServiceZone.clear(); //clear the value if present

	SetContentType(mainManifestUrl, contentType);
	if(!IsLiveAdjustRequired()) /* Ideally checking the content is either "ivod/cdvr" to adjust the liveoffset on trickplay. */
	{
		// DELIA-30843/DELIA-31379. for CDVR/IVod, offset is set to higher value
		// need to adjust the liveoffset on trickplay for ivod/cdvr with 30sec
		if(!mNewLiveOffsetflag)
		{
			mLiveOffset	=	gpGlobalConfig->cdvrliveOffset;
		}
	}
	else
	{
		// will be used only for live
		if(!mNewLiveOffsetflag)
		{
			mLiveOffset	=	gpGlobalConfig->liveOffset;
		}
	}
	logprintf("mLiveOffset: %d", mLiveOffset);

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

	if( !remapUrl )
	{
		if (gpGlobalConfig->mapMPD && !mIsDash && (mContentType != ContentType_EAS)) //Don't map, if it is dash and dont map if it is EAS
		{
			mIsDash = true;
			if (!gpGlobalConfig->fogSupportsDash )
			{
				DeFog(manifestUrl);
			}

			bool urlReplaced = false;

			switch(gpGlobalConfig->mapMPD)
			{
				case 1: 		//Simply change m3u8 to mpd
					urlReplaced = true;
					break;
				case 2:
					urlReplaced |= (replace_cstring(manifestUrl, "col-jitp2.xcr", "col-jitp2-samsung.top") ||
					                replace_cstring(manifestUrl, "linear-nat-pil-red", "coam-tvil-pil")    ||
					                replace_cstring(manifestUrl, "linear-nat-pil", "coam-tvil-pil"));
					break;
				case 3:			//Setting all national channels' FQDN to "ctv-nat-slivel4lb-vip.cmc.co.ndcwest.comcast.net"
					if(strstr(manifestUrl,"-nat-"))
					{
						std::string hostName = aamp_getHostFromURL(manifestUrl);
						urlReplaced |= replace_cstring(manifestUrl, hostName.c_str(), "ctv-nat-slivel4lb-vip.cmc.co.ndcwest.comcast.net");
					}
					else
					{
						urlReplaced |= replace_cstring(manifestUrl, "col-jitp2.xcr", "col-jitp2-samsung.top");
					}
					break;
				default:
					//Let fall back
					break;
			}

			if(!urlReplaced)
			{
				//Fall back channel
				strcpy(manifestUrl, "http://ccr.coam-tvil-pil.xcr.comcast.net/FNCHD_HD_NAT_16756_0_5884597068415311163.mpd");
			}

			replace_cstring(manifestUrl, ".m3u8", ".mpd");
		}
		
		if (gpGlobalConfig->noFog)
		{
			DeFog(manifestUrl);
		}
	
		if (gpGlobalConfig->forceEC3)
		{
			replace_cstring(manifestUrl,".m3u8", "-eac3.m3u8");
		}
		if (gpGlobalConfig->disableEC3 && strstr(manifestUrl,"tsb?") ) // new - limit this option to linear content as part of DELIA-23975
		{
			replace_cstring(manifestUrl, "-eac3.m3u8", ".m3u8");
		}

		if(gpGlobalConfig->bForceHttp)
		{
			replace_cstring(manifestUrl, "https://", "http://");
		}

		if (strstr(manifestUrl,"mpd") ) // new - limit this option to linear content as part of DELIA-23975
		{
			replace_cstring(manifestUrl, "-eac3.mpd", ".mpd");
		} // mpd
	} // !remap_url
 
	if (strstr(manifestUrl,"tsb?"))
	{
		mTSBEnabled = true;
	}
	mIsFirstRequestToFOG = (mIsLocalPlayback == true);
	logprintf("aamp_tune: attempt: %d format: %s URL: %s\n", mTuneAttempts, mIsDash?"DASH":"HLS" ,manifestUrl);

	// this function uses mIsVSS and mTSBEnabled, hence it should be called after these variables are updated.
	ExtractServiceZone(manifestUrl);

	SetTunedManifestUrl(mTSBEnabled);

	if(bFirstAttempt)
	{
		mfirstTuneFmt = mIsDash?1:0;
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
 *   @brief return service zone, extracted from locator &sz URI parameter
 *   @param  url - stream url with vss service zone info as query string
 *   @return std::string
 */
void PrivateInstanceAAMP::ExtractServiceZone(const char *url)
{
	if(mIsVSS && url)
	{
		const char * vssURL = NULL;
		char * tempRedirectedURL = NULL;

		if(mTSBEnabled)
		{
			tempRedirectedURL = strdup(url);
			DeFog(tempRedirectedURL);
			vssURL = (const char *) tempRedirectedURL;
		}
		else
		{
			vssURL = url;
		}

		vssURL = strstr(vssURL, VSS_MARKER);

		if(vssURL)
		{
			vssURL += strlen(VSS_MARKER);
			const char * nextQueryParameter = strstr(vssURL, "&");
			if(nextQueryParameter)
			{
				int iServiceZoneLen = (nextQueryParameter - vssURL);
				mServiceZone = string(vssURL,iServiceZoneLen);
			}
			else
			{
				mServiceZone = vssURL;
			}
		}
		else
		{
			AAMPLOG_ERR("PrivateInstanceAAMP::%s - ERROR: url does not have vss marker :%s \n", __FUNCTION__,url);
		}

		if(tempRedirectedURL)
		{
			free(tempRedirectedURL);
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
        caseContentType_PPV :
        {
            strRet =  "PPV"; //ppv
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
	}
	logprintf("Detected ContentType %d (%s)\n",mContentType,cType?cType:"UNKNOWN");
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
char *PrivateInstanceAAMP::LoadFragment(ProfilerBucketType bucketType, const char *fragmentUrl, char *effectiveUrl, size_t *len, unsigned int curlInstance, const char *range, long * http_code,MediaType fileType,int * fogError)
{
	profiler.ProfileBegin(bucketType);

	struct GrowableBuffer fragment = { 0, 0, 0 }; // TODO: leaks if thread killed
	if (!GetFile(fragmentUrl, &fragment, effectiveUrl, http_code, range, curlInstance, true, fileType,NULL,fogError))
	{
		profiler.ProfileError(bucketType);
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
bool PrivateInstanceAAMP::LoadFragment(ProfilerBucketType bucketType, const char *fragmentUrl,char *effectiveUrl, struct GrowableBuffer *fragment, unsigned int curlInstance, const char *range, MediaType fileType, long * http_code, long *bitrate,int * fogError)
{
	bool ret = true;
	profiler.ProfileBegin(bucketType);

	if (!GetFile(fragmentUrl, fragment, effectiveUrl, http_code, range, curlInstance, false, fileType, bitrate,fogError))
	{
		ret = false;
		profiler.ProfileError(bucketType);
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
	SyncBegin();
	mStreamSink->EndOfStreamReached(mediaType);
	SyncEnd();
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
	PrivAAMPState state;
	aamp->GetState(state);
	if (aamp->mpStreamAbstractionAAMP && state != eSTATE_ERROR)
	{
		bool retValue = true;
		if (rate > 0 && aamp->IsLive() && aamp->mpStreamAbstractionAAMP->IsStreamerAtLivePoint() && aamp->rate >= AAMP_NORMAL_PLAY_RATE)
		{
			logprintf("%s(): Already at logical live point, hence skipping operation\n", __FUNCTION__);
			aamp->NotifyOnEnteringLive();
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
		// when switching from trick to play mode only 
		if(aamp->rate && rate== AAMP_NORMAL_PLAY_RATE)
		{
			if(timeDeltaFromProgReport > 950) // diff > 950 mSec
			{
				// increment by 1x trickplay frame , next possible displayed frame
				aamp->seek_pos_seconds = (aamp->mReportProgressPosn+(aamp->rate*1000))/1000;
			}
			else if(timeDeltaFromProgReport > 100) // diff > 100 mSec
			{
				// Get the last shown frame itself 
				aamp->seek_pos_seconds = aamp->mReportProgressPosn/1000;
			}
			else
			{
				// Go little back to last shown frame 
				aamp->seek_pos_seconds = (aamp->mReportProgressPosn-(aamp->rate*1000))/1000;
			}
		}
		else
		{
			// Coming out of pause mode(aamp->rate=0) or when going into pause mode (rate=0)
			// Show the last position 
			aamp->seek_pos_seconds = aamp->GetPositionMs()/1000;
		}

		aamp->trickStartUTCMS = -1;

		logprintf("aamp_SetRate(%d)overshoot(%d) ProgressReportDelta:(%d) ", rate,overshootcorrection,timeDeltaFromProgReport);
		logprintf("aamp_SetRate Adj position: %f\n", aamp->seek_pos_seconds); // current position relative to tune time
		logprintf("aamp_SetRate rate(%d)->(%d)\n", aamp->rate,rate);
		logprintf("aamp_SetRate cur pipeline: %s\n", aamp->pipeline_paused ? "paused" : "playing");

		if (rate == aamp->rate)
		{ // no change in desired play rate
			if (aamp->pipeline_paused && rate != 0)
			{ // but need to unpause pipeline
				AAMPLOG_INFO("Resuming Playback at Position '%lld'.\n", aamp->GetPositionMs());
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
				AAMPLOG_INFO("Pausing Playback at Position '%lld'.\n", aamp->GetPositionMs());
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
		aamp->rate = rate;
	}
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

	if (secondsRelativeToTuneTime == AAMP_SEEK_TO_LIVE_POSITION)
	{
		isSeekToLive = true;
		tuneType = eTUNETYPE_SEEKTOLIVE;
	}

	logprintf("aamp_Seek(%f) and seekToLive(%d)\n", secondsRelativeToTuneTime, isSeekToLive);

	if (isSeekToLive && !aamp->IsLive())
	{
		logprintf("%s:%d - Not live, skipping seekToLive\n",__FUNCTION__,__LINE__);
		return;
	}

	if (aamp->IsLive() && aamp->mpStreamAbstractionAAMP && aamp->mpStreamAbstractionAAMP->IsStreamerAtLivePoint())
	{
		double currPositionSecs = aamp->GetPositionMs() / 1000.00;
		if (isSeekToLive || secondsRelativeToTuneTime >= currPositionSecs)
		{
			logprintf("%s():Already at live point, skipping operation since requested position(%f) >= currPosition(%f) or seekToLive(%d)\n", __FUNCTION__, secondsRelativeToTuneTime, currPositionSecs, isSeekToLive);
			aamp->NotifyOnEnteringLive();
			return;
		}
	}

	if (aamp->pipeline_paused)
	{
		// resume downloads and clear paused flag. state change will be done
		// on streamSink configuration.
		logprintf("%s(): paused state, so resume downloads\n", __FUNCTION__);
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
	logprintf("aamp_SetRateAndSeek(%d)(%f)\n", rate, secondsRelativeToTuneTime);
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
	aamp->SetVideoRectangle(x, y, w, h);
}


/**
 *   @brief Set video zoom.
 *
 *   @param  zoom - zoom mode.
 */
void PlayerInstanceAAMP::SetVideoZoom(VideoZoomMode zoom)
{
	aamp->zoom_mode = zoom;
	if (aamp->mpStreamAbstractionAAMP)
		aamp->SetVideoZoom(zoom);
}


/**
 *   @brief Enable/ Disable Video.
 *
 *   @param  muted - true to disable video, false to enable video.
 */
void PlayerInstanceAAMP::SetVideoMute(bool muted)
{
	aamp->video_muted = muted;
	if (aamp->mpStreamAbstractionAAMP)
		aamp->SetVideoMute(muted);
}


/**
 *   @brief Set Audio Volume.
 *
 *   @param  volume - Minimum 0, maximum 100.
 */
void PlayerInstanceAAMP::SetAudioVolume(int volume)
{
	aamp->audio_volume = volume;
	if (aamp->mpStreamAbstractionAAMP)
		aamp->SetAudioVolume(volume);
}


/**
 *   @brief Set Audio language.
 *
 *   @param  language - Language of audio track.
 */
void PlayerInstanceAAMP::SetLanguage(const char* language)
{
	logprintf("aamp_SetLanguage(%s)->(%s)\n",aamp->language, language);

        if (strncmp(language, aamp->language, MAX_LANGUAGE_TAG_LENGTH) == 0)
                return;

	PrivAAMPState state;
	aamp->GetState(state);
	// There is no active playback session, save the language for later
	if (state == eSTATE_IDLE)
	{
		aamp->UpdateAudioLanguageSelection(language);
		logprintf("aamp_SetLanguage(%s) Language set prior to tune start\n", language);
		return;
	}

	// check if language is supported in manifest languagelist
	if((aamp->IsAudioLanguageSupported(language)) || (!aamp->mMaxLanguageCount))
	{
		aamp->UpdateAudioLanguageSelection(language);
		logprintf("aamp_SetLanguage(%s) Language set\n", language);
		if (aamp->mpStreamAbstractionAAMP)
		{
			logprintf("aamp_SetLanguage(%s) retuning\n", language);

			aamp->discardEnteringLiveEvt = true;

			aamp->seek_pos_seconds = aamp->GetPositionMs()/1000.0;
			aamp->TeardownStream(false);
			aamp->TuneHelper(eTUNETYPE_SEEK);

			aamp->discardEnteringLiveEvt = false;
		}
	}
	else
		logprintf("aamp_SetLanguage(%s) not supported in manifest\n", language);

}


/**
 *   @brief Set array of subscribed tags.
 *
 *   @param  subscribedTags - Array of subscribed tags.
 */
void PlayerInstanceAAMP::SetSubscribedTags(std::vector<std::string> subscribedTags)
{
	logprintf("aamp_SetSubscribedTags()\n");

	aamp->subscribedTags = subscribedTags;

	for (int i=0; i < aamp->subscribedTags.size(); i++) {
	        logprintf("    subscribedTags[%d] = '%s'\n", i, subscribedTags.at(i).data());
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
	logprintf("[AAMP_JS] %s(%p)\n", __FUNCTION__, context);
	if (mJSBinding_DL) {
		void(*loadJS)(void*, void*);
		const char* szLoadJS = "aamp_LoadJS";
		loadJS = (void(*)(void*, void*))dlsym(mJSBinding_DL, szLoadJS);
		if (loadJS) {
			logprintf("[AAMP_JS] %s() dlsym(%p, \"%s\")=%p\n", __FUNCTION__, mJSBinding_DL, szLoadJS, loadJS);
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
	logprintf("[AAMP_JS] %s(%p)\n", __FUNCTION__, context);
	if (mJSBinding_DL) {
		void(*unloadJS)(void*);
		const char* szUnloadJS = "aamp_UnloadJS";
		unloadJS = (void(*)(void*))dlsym(mJSBinding_DL, szUnloadJS);
		if (unloadJS) {
			logprintf("[AAMP_JS] %s() dlsym(%p, \"%s\")=%p\n", __FUNCTION__, mJSBinding_DL, szUnloadJS, unloadJS);
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
	PrivAAMPState state;
	aamp->GetState(state);
	if (state == eSTATE_ERROR)
	{
		logprintf("IsLive is ignored since the player is at eSTATE_ERROR\n");
		return false;
	}
	else
	{
		return aamp->IsLive();
	}
}


/**
 *   @brief Get current audio language.
 *
 *   @return current audio language
 */
char* PlayerInstanceAAMP::GetCurrentAudioLanguage(void)
{
	return aamp->language;
}

/**
 *   @brief Get current drm
 *
 *   @return current drm
 */
const char* PlayerInstanceAAMP::GetCurrentDRM(void)
{
	DRMSystems currentDRM = aamp->GetCurrentDRM();
	const char *drmName = "";
	switch(currentDRM)
	{
		case eDRM_WideVine:
			drmName = "WideVine";
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
 */
void PlayerInstanceAAMP::AddCustomHTTPHeader(std::string headerName, std::vector<std::string> headerValue)
{
	aamp->AddCustomHTTPHeader(headerName, headerValue);
}

/**
 *   @brief Set License Server URL.
 *
 *   @param  url - URL of the server to be used for license requests
 *   @param  type - DRM Type(PR/WV) for which the server URL should be used, global by default
 */
void PlayerInstanceAAMP::SetLicenseServerURL(const char *url, DRMSystems type)
{
	aamp->SetLicenseServerURL(url, type);
}


/**
 *   @brief Indicates if session token has to be used with license request or not.
 *
 *   @param  isAnonymous - True if session token should be blank and false otherwise.
 */
void PlayerInstanceAAMP::SetAnonymousRequest(bool isAnonymous)
{
	aamp->SetAnonymousRequest(isAnonymous);
}


/**
 *   @brief Set VOD Trickplay FPS.
 *
 *   @param  vodTrickplayFPS - FPS to be used for VOD Trickplay
 */
void PlayerInstanceAAMP::SetVODTrickplayFPS(int vodTrickplayFPS)
{
	aamp->SetVODTrickplayFPS(vodTrickplayFPS);
}


/**
 *   @brief Set Linear Trickplay FPS.
 *
 *   @param  linearTrickplayFPS - FPS to be used for Linear Trickplay
 */
void PlayerInstanceAAMP::SetLinearTrickplayFPS(int linearTrickplayFPS)
{
	aamp->SetLinearTrickplayFPS(linearTrickplayFPS);
}

/**
 *   @brief Set Live Offset.
 *
 *   @param  liveoffset- Live Offset
 */
void PlayerInstanceAAMP::SetLiveOffset(int liveoffset)
{
	aamp->SetLiveOffset(liveoffset);
}


/**
 *   @brief To set the error code to be used for playback stalled error.
 *
 *   @param  errorCode - error code for playback stall errors.
 */
void PlayerInstanceAAMP::SetStallErrorCode(int errorCode)
{
	aamp->SetStallErrorCode(errorCode);
}


/**
 *   @brief To set the timeout value to be used for playback stall detection.
 *
 *   @param  timeoutMS - timeout in milliseconds for playback stall detection.
 */
void PlayerInstanceAAMP::SetStallTimeout(int timeoutMS)
{
	aamp->SetStallTimeout(timeoutMS);
}


/**
 *   @brief Set report interval duration
 *
 *   @param  reportIntervalMS - report interval duration in MS
 */
void PlayerInstanceAAMP::SetReportInterval(int reportIntervalMS)
{
	aamp->SetReportInterval(reportIntervalMS);
}


/**
 *   @brief To get the current playback position.
 *
 *   @ret current playback position in seconds
 */
double PlayerInstanceAAMP::GetPlaybackPosition()
{
	return (aamp->GetPositionMs() / 1000.00);
}


/**
*   @brief To get the current asset's duration.
*
*   @ret duration in seconds
*/
double PlayerInstanceAAMP::GetPlaybackDuration()
{
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
	long bitrate = 0;
	if (aamp->mpStreamAbstractionAAMP)
	{
		bitrate = aamp->mpStreamAbstractionAAMP->GetAudioBitrate();
	}
	return bitrate;
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
	return aamp->audio_volume;
}


/**
 *   @brief To get the current playback rate.
 *
 *   @ret current playback rate
 */
int PlayerInstanceAAMP::GetPlaybackRate(void)
{
	return aamp->rate;
}


/**
 *   @brief To get the available video bitrates.
 *
 *   @ret available video bitrates
 */
std::vector<long> PlayerInstanceAAMP::GetVideoBitrates(void)
{
	if (aamp->mpStreamAbstractionAAMP)
	{
		return aamp->mpStreamAbstractionAAMP->GetVideoBitrates();
	}
	return std::vector<long>();
}


/**
 *   @brief To get the available audio bitrates.
 *
 *   @ret available audio bitrates
 */
std::vector<long> PlayerInstanceAAMP::GetAudioBitrates(void)
{
	if (aamp->mpStreamAbstractionAAMP)
	{
		return aamp->mpStreamAbstractionAAMP->GetAudioBitrates();
	}
	return std::vector<long>();
}


/**
 *   @brief To set the initial bitrate value.
 *
 *   @param[in] initial bitrate to be selected
 */
void PlayerInstanceAAMP::SetInitialBitrate(long bitrate)
{
	aamp->SetInitialBitrate(bitrate);
}


/**
 *   @brief To set the initial bitrate value for 4K assets.
 *
 *   @param[in] initial bitrate to be selected for 4K assets
 */
void PlayerInstanceAAMP::SetInitialBitrate4K(long bitrate4K)
{
	aamp->SetInitialBitrate4K(bitrate4K);
}


/**
 *   @brief To set the network download timeout value.
 *
 *   @param[in] preferred timeout value
 */
void PlayerInstanceAAMP::SetNetworkTimeout(long timeout)
{
	aamp->SetNetworkTimeout(timeout);
}


/**
 *   @brief To set the download buffer size value
 *
 *   @param[in] preferred download buffer size
 */
void PlayerInstanceAAMP::SetDownloadBufferSize(int bufferSize)
{
	aamp->SetDownloadBufferSize(bufferSize);
}


/**
 *   @brief Set preferred DRM.
 *
 *   @param[in] drmType - preferred DRM type
 */
void PlayerInstanceAAMP::SetPreferredDRM(DRMSystems drmType)
{
	aamp->SetPreferredDRM(drmType);
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
	aamp->SetAlternateContents(adBreakId, adId, url);
}

/**
 *   @brief To set the network proxy
 *
 *   @param[in] network proxy to use
 */
void PlayerInstanceAAMP::SetNetworkProxy(const char * proxy)
{
	aamp->SetNetworkProxy(proxy);
}


/**
 *   @brief To set the proxy for license request
 *
 *   @param[in] proxy to use for license request
 */
void PlayerInstanceAAMP::SetLicenseReqProxy(const char * licenseProxy)
{
	aamp->SetLicenseReqProxy(licenseProxy);
}


/**
 *   @brief To set the curl stall timeout value
 *
 *   @param[in] curl stall timeout
 */
void PlayerInstanceAAMP::SetDownloadStallTimeout(long stallTimeout)
{
	aamp->SetDownloadStallTimeout(stallTimeout);
}


/**
 *   @brief To set the curl download start timeout value
 *
 *   @param[in] curl download start timeout
 */
void PlayerInstanceAAMP::SetDownloadStartTimeout(long startTimeout)
{
	aamp->SetDownloadStartTimeout(startTimeout);
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
				logprintf("sleep interrupted!\n");
			}
#ifndef WIN32
			else if (ETIMEDOUT != ret)
			{
				logprintf("sleep - condition wait failed %s\n", strerror(ret));
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
	long long positionMiliseconds = (seek_pos_seconds)* 1000.0;
	if (!pipeline_paused && trickStartUTCMS >= 0)
	{
		long long elapsedTime = aamp_GetCurrentTimeMS() - trickStartUTCMS;
		positionMiliseconds += elapsedTime*rate;

		if (positionMiliseconds < 0)
		{
			logprintf("%s : Correcting positionMiliseconds %lld to zero\n", __FUNCTION__, positionMiliseconds);
			positionMiliseconds = 0;
		}
		else if (mpStreamAbstractionAAMP)
		{
			if (!mIsLive)
			{
				long long durationMs  = GetDurationMs();
				if(positionMiliseconds > durationMs)
				{
					logprintf("%s : Correcting positionMiliseconds %lld to duration %lld\n", __FUNCTION__, positionMiliseconds, durationMs);
					positionMiliseconds = durationMs;
				}
			}
			else
			{
				long long tsbEndMs = GetDurationMs() + (culledSeconds * 1000.0);
				if(positionMiliseconds > tsbEndMs)
				{
					logprintf("%s : Correcting positionMiliseconds %lld to duration %lld\n", __FUNCTION__, positionMiliseconds, tsbEndMs);
					positionMiliseconds = tsbEndMs;
				}
			}
		}
		// note, using mStreamerInterface->GetPositionMilliseconds() instead of elapsedTime
		// would likely be more accurate, but would need to be tested to accomodate
		// and compensate for FF/REW play rates
	}
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
	// Stopping the playback, release all DRM context
	if (mpStreamAbstractionAAMP)
	{
		mpStreamAbstractionAAMP->Stop(true);
	}

	TeardownStream(true);
	pthread_mutex_lock(&mLock);
	if (mPendingAsyncEvents.size() > 0)
	{
		logprintf("PrivateInstanceAAMP::%s() - mPendingAsyncEvents.size - %d\n", __FUNCTION__, mPendingAsyncEvents.size());
		for (std::map<gint, bool>::iterator it = mPendingAsyncEvents.begin(); it != mPendingAsyncEvents.end(); it++)
		{
			if (it->first != 0)
			{
				if (it->second)
				{
					logprintf("PrivateInstanceAAMP::%s() - remove id - %d\n", __FUNCTION__, (int) it->first);
					g_source_remove(it->first);
				}
				else
				{
					logprintf("PrivateInstanceAAMP::%s() - Not removing id - %d as not pending\n", __FUNCTION__, (int) it->first);
				}
			}
		}
		mPendingAsyncEvents.clear();
	}
	if (timedMetadata.size() > 0)
	{
		logprintf("PrivateInstanceAAMP::%s() - timedMetadata.size - %d\n", __FUNCTION__, timedMetadata.size());
		timedMetadata.clear();
	}

	pthread_mutex_unlock(&mLock);
	seek_pos_seconds = -1;
	culledSeconds = 0;
	durationSeconds = 0;
	rate = 1;
	AampCacheHandler::GetInstance()->StopPlaylistCache();
	mSeekOperationInProgress = false;
	mMaxLanguageCount = 0; // reset language count

	if(NULL != mCdaiObject)
	{
		delete mCdaiObject;
		mCdaiObject = NULL;
	}
}



/**
 * @brief Report TimedMetadata events
 * @param timeMilliseconds time in milliseconds
 * @param szName name of metadata
 * @param szContent  metadata content
 * @param id - Identifier of the TimedMetadata
 * @param durationMS - Duration in milliseconds
 * @param nb unused
 */
void PrivateInstanceAAMP::ReportTimedMetadata(double timeMilliseconds, const char* szName, const char* szContent, int nb, const char* id, double durationMS)
{
	std::string content(szContent, nb);
	bool bFireEvent = false;

	// Check if timedMetadata was already reported
	std::vector<TimedMetadata>::iterator i;
	for (i=timedMetadata.begin(); i != timedMetadata.end(); i++)
	{
		if (i->_timeMS < timeMilliseconds)
			continue;

		// Does an entry already exist?
		if ((i->_timeMS == timeMilliseconds) && (i->_name.compare(szName) == 0))
		{
			if (i->_content.compare(content) == 0)
			{
				//logprintf("aamp_ReportTimedMetadata(%ld, '%s', '%s', nb) DUPLICATE\n", (long)timeMilliseconds, szName, content.data(), nb);
			} else {
				//logprintf("aamp_ReportTimedMetadata(%ld, '%s', '%s', nb) REPLACE\n", (long)timeMilliseconds, szName, content.data(), nb);
				i->_content = content;
				bFireEvent = true;
			}
			break;
		}

		if (i->_timeMS > timeMilliseconds)
		{
			//logprintf("aamp_ReportTimedMetadata(%ld, '%s', '%s', nb) INSERT\n", (long)timeMilliseconds, szName, content.data(), nb);
			timedMetadata.insert(i, TimedMetadata(timeMilliseconds, szName, content, id, durationMS));
			bFireEvent = true;
			break;
		}
	}

	if (i == timedMetadata.end())
	{
		//logprintf("aamp_ReportTimedMetadata(%ld, '%s', '%s', nb) APPEND\n", (long)timeMilliseconds, szName, content.data(), nb);
		timedMetadata.push_back(TimedMetadata(timeMilliseconds, szName, content, id, durationMS));
		bFireEvent = true;
	}

	if (bFireEvent)
	{
		AAMPEvent eventData;
		eventData.type = AAMP_EVENT_TIMED_METADATA;
		eventData.data.timedMetadata.timeMilliseconds = timeMilliseconds;
		eventData.data.timedMetadata.id = (id == NULL) ? "" : id;
		eventData.data.timedMetadata.durationMilliSeconds = durationMS;
		eventData.data.timedMetadata.szName = (szName == NULL) ? "" : szName;
		eventData.data.timedMetadata.szContent = content.data();

		if (gpGlobalConfig->logging.progress)
		{
			logprintf("aamp timedMetadata: [%ld] '%s'\n",
				(long)(eventData.data.timedMetadata.timeMilliseconds),
				eventData.data.timedMetadata.szContent);
		}
		SendEventSync(eventData);
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
		logprintf("aamp harvest: %d\n", gpGlobalConfig->harvest);
		if(modifyCount)
		{
			gpGlobalConfig->harvest--;
			if(!gpGlobalConfig->harvest)
			{
				logprintf("gpGlobalConfig->harvest zero, no more harvesting\n");
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
#ifdef AAMP_STOP_SINK_ON_SEEK
	/*Do not send event on trickplay as CC is not enabled*/
	if (AAMP_NORMAL_PLAY_RATE != rate)
	{
		logprintf("PrivateInstanceAAMP::%s:%d : not sending cc handle as rate = %f\n", __FUNCTION__, __LINE__, rate);
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
		mProcessingDiscontinuity = true;
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
		logprintf("PrivateInstanceAAMP::%s : %p not in Active AAMP list\n", __FUNCTION__, aamp);
	}
	else if (!reTune)
	{
		logprintf("PrivateInstanceAAMP::%s : %p reTune flag not set\n", __FUNCTION__, aamp);
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
			logprintf("PrivateInstanceAAMP::%s:%d: Not processing reTune since state = %d, mSeekOperationInProgress = %d\n",
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
				logprintf("PrivateInstanceAAMP::%s:%d: Discontinuity Tune handler already spawned(%d) or inprogress(%d)\n",
					__FUNCTION__, __LINE__, mDiscontinuityTuneOperationId, mDiscontinuityTuneOperationInProgress);
				return;
			}
			mDiscontinuityTuneOperationId = g_idle_add(PrivateInstanceAAMP_ProcessDiscontinuity, (gpointer) this);
			pthread_mutex_unlock(&mLock);

			logprintf("PrivateInstanceAAMP::%s:%d: Underflow due to discontinuity handled\n", __FUNCTION__, __LINE__);
			return;
		}
		else if (mpStreamAbstractionAAMP->IsStreamerStalled())
		{
			logprintf("PrivateInstanceAAMP::%s:%d: Ignore reTune due to playback stall\n", __FUNCTION__, __LINE__);
			return;
		}
		else if (!gpGlobalConfig->internalReTune)
		{
			logprintf("PrivateInstanceAAMP::%s:%d: Ignore reTune as disabled in configuration\n", __FUNCTION__, __LINE__);
			return;
		}
		SendAnomalyEvent(ANOMALY_WARNING, "%s %s", (trackType == eMEDIATYPE_VIDEO ? "VIDEO" : "AUDIO"),
		        (errorType == eGST_ERROR_PTS) ? "PTS ERROR" :
		        (errorType == eGST_ERROR_UNDERFLOW) ? "Underflow" : "STARTTIME RESET");
		bool activeAAMPFound = false;
		pthread_mutex_lock(&gMutex);
		for (std::list<gActivePrivAAMP_t>::iterator iter = gActivePrivAAMPs.begin(); iter != gActivePrivAAMPs.end(); iter++)
		{
			if (this == iter->pAAMP)
			{
				gActivePrivAAMP_t *gAAMPInstance = &(*iter);
				if (gAAMPInstance->reTune)
				{
					logprintf("PrivateInstanceAAMP::%s:%d: Already scheduled\n", __FUNCTION__, __LINE__);
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
								logprintf("PrivateInstanceAAMP::%s:%d: numPtsErrors %d, ptsErrorThreshold %d\n",
									__FUNCTION__, __LINE__, gAAMPInstance->numPtsErrors, gpGlobalConfig->ptsErrorThreshold);
								if (gAAMPInstance->numPtsErrors >= gpGlobalConfig->ptsErrorThreshold)
								{
									gAAMPInstance->numPtsErrors = 0;
									gAAMPInstance->reTune = true;
									logprintf("PrivateInstanceAAMP::%s:%d: Schedule Retune. diffMs %lld < threshold %lld\n",
										__FUNCTION__, __LINE__, diffMs, AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS);
									g_idle_add(PrivateInstanceAAMP_Retune, (gpointer)this);
								}
							}
							else
							{
								gAAMPInstance->numPtsErrors = 0;
								logprintf("PrivateInstanceAAMP::%s:%d: Not scheduling reTune since (diff %lld > threshold %lld) numPtsErrors %d, ptsErrorThreshold %d.\n",
									__FUNCTION__, __LINE__, diffMs, AAMP_MAX_TIME_BW_UNDERFLOWS_TO_TRIGGER_RETUNE_MS,
									gAAMPInstance->numPtsErrors, gpGlobalConfig->ptsErrorThreshold);
							}
						}
						else
						{
							const char* errorString = (errorType == eGST_ERROR_UNDERFLOW) ? "underflow" : "pts error";
							gAAMPInstance->numPtsErrors = 0;
							logprintf("PrivateInstanceAAMP::%s:%d: Not scheduling reTune since first %s.\n", __FUNCTION__, __LINE__, errorString);
						}
						lastUnderFlowTimeMs[trackType] = now;
					}
					else
					{
						logprintf("PrivateInstanceAAMP::%s:%d: Schedule Retune errorType %d\n", __FUNCTION__, __LINE__, errorType);
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
			logprintf("PrivateInstanceAAMP::%s:%d: %p not in Active AAMP list\n", __FUNCTION__, __LINE__, this);
		}
	}
}


/**
 * @brief PrivateInstanceAAMP Constructor
 */
PrivateInstanceAAMP::PrivateInstanceAAMP() : mAbrBitrateData(), mLock(), mMutexAttr(),
	mpStreamAbstractionAAMP(NULL), mInitSuccess(false), mFormat(FORMAT_INVALID), mAudioFormat(FORMAT_INVALID), mDownloadsDisabled(),
	mDownloadsEnabled(true), mStreamSink(NULL), profiler(), licenceFromManifest(false), previousAudioType(eAUDIO_UNKNOWN),
	mbDownloadsBlocked(false), streamerIsActive(false), mTSBEnabled(false), mIscDVR(false), mLiveOffset(AAMP_LIVE_OFFSET), mNewLiveOffsetflag(false),
	fragmentCollectorThreadID(0), seek_pos_seconds(-1), rate(0), pipeline_paused(false), mMaxLanguageCount(0), zoom_mode(VIDEO_ZOOM_FULL),
	video_muted(false), audio_volume(100), subscribedTags(), timedMetadata(), IsTuneTypeNew(false), trickStartUTCMS(-1),
	playStartUTCMS(0), durationSeconds(0.0), culledSeconds(0.0), maxRefreshPlaylistIntervalSecs(DEFAULT_INTERVAL_BETWEEN_PLAYLIST_UPDATES_MS/1000), initialTuneTimeMs(0),
	mEventListener(NULL), mReportProgressPosn(0.0), mReportProgressTime(0), discardEnteringLiveEvt(false),
	mIsRetuneInProgress(false), mCondDiscontinuity(), mDiscontinuityTuneOperationId(0), mIsVSS(false),
	m_fd(-1), mIsLive(false), mTuneCompleted(false), mFirstTune(true), mfirstTuneFmt(-1), mTuneAttempts(0), mPlayerLoadTime(0),
	mState(eSTATE_RELEASED), mIsDash(false), mCurrentDrm(eDRM_NONE), mPersistedProfileIndex(0), mAvailableBandwidth(0), mProcessingDiscontinuity(false),
	mDiscontinuityTuneOperationInProgress(false), mContentType(), mTunedEventPending(false),
	mSeekOperationInProgress(false), mPendingAsyncEvents(), mCustomHeaders(),
	mServiceZone(),
	mCurrentLanguageIndex(0),mVideoEnd(NULL),mTimeToTopProfile(0),mTimeAtTopProfile(0),mPlaybackDuration(0),mTraceUUID(),
	mIsFirstRequestToFOG(false), mIsLocalPlayback(false), mABREnabled(false), mUserRequestedBandwidth(0), mNetworkProxy(NULL), mLicenseProxy(NULL), mTuneType(eTUNETYPE_NEW_NORMAL)
	,mCdaiObject(NULL), mAdEventsQ(),mAdEventQMtx(), mAdPrevProgressTime(0), mAdCurOffset(0), mAdDuration(0), mAdProgressId("")
#ifdef PLACEMENT_EMULATION
	,mNumAds2Place(0), sampleAdBreakId("")
#endif
{
	LazilyLoadConfigIfNeeded();
	pthread_cond_init(&mDownloadsDisabled, NULL);
	strcpy(language,"en");
	pthread_mutexattr_init(&mMutexAttr);
	pthread_mutexattr_settype(&mMutexAttr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&mLock, &mMutexAttr);

	for (int i = 0; i < MAX_CURL_INSTANCE_COUNT; i++)
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

	mABREnabled = gpGlobalConfig->bEnableABR;
	mUserRequestedBandwidth = gpGlobalConfig->defaultBitrate;
	mNetworkProxy = NULL;
	mLicenseProxy = NULL;
	mCdaiObject = NULL;
	mAdPrevProgressTime = 0;
	mAdProgressId = "";
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

	pthread_cond_destroy(&mDownloadsDisabled);
	pthread_cond_destroy(&mCondDiscontinuity);
	pthread_mutex_destroy(&mLock);
}


/**
 * @brief Sets aamp state
 * @param state state to be set
 */
void PrivateInstanceAAMP::SetState(PrivAAMPState state)
{
	if (mState == state)
	{
		return;
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
bool PrivateInstanceAAMP::SendTunedEvent()
{
	bool ret = false;

	// Required for synchronising btw audio and video tracks in case of cdmidecryptor
	pthread_mutex_lock(&mLock);

	ret = mTunedEventPending;
	mTunedEventPending = false;

	pthread_mutex_unlock(&mLock);

	if(ret)
	{
		SendEventAsync(AAMP_EVENT_TUNED);
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

/**
 *   @brief updates download metrics to VideoStat object, this is used for VideoFragment as it takes duration for calcuation purpose.
 *
 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
 *   @param[in]  bitrate - bitrate ( bits per sec )
 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
 *   @param[in]  strUrl :  URL in case of faulures
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl, double duration)
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
	pthread_mutex_lock(&mLock);
	if(mVideoEnd)
	{

		mVideoEnd->SetTsbStatus(btsbAvailable);
	}
	pthread_mutex_unlock(&mLock);
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
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl, double duration, bool keyChanged, bool isEncrypted)
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
				if(eCountType != COUNT_SUCCESS && strUrl)
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
			AAMPLOG_INFO("PrivateInstanceAAMP::%s - Could Not update VideoEnd Event dataType:%d trackType:%d eCountType:%d\n", __FUNCTION__,
					dataType,trackType,eCountType);
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



/**
 *   @brief updates download metrics to VideoEnd object,
 *
 *   @param[in]  mediaType - MediaType ( Manifest/Audio/Video etc )
 *   @param[in]  bitrate - bitrate
 *   @param[in]  curlOrHTTPErrorCode - download curl or http error
 *   @param[in]  strUrl :  URL in case of faulures
 *   @return void
 */
void PrivateInstanceAAMP::UpdateVideoEndMetrics(MediaType mediaType, long bitrate, int curlOrHTTPCode, const char * strUrl)
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
		logprintf("%s:%d id not in mPendingAsyncEvents, insert and mark as not pending\n", __FUNCTION__, __LINE__, id);
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
		logprintf("%s:%d id already in mPendingAsyncEvents and completed, erase it\n", __FUNCTION__, __LINE__, id);
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
 */
void PrivateInstanceAAMP::AddCustomHTTPHeader(std::string headerName, std::vector<std::string> headerValue)
{
	// Header name should be ending with :
	if(headerName.back() != ':')
	{
		headerName += ':';
	}

	if (headerValue.size() != 0)
	{
		mCustomHeaders[headerName] = headerValue;
	}
	else
	{
		mCustomHeaders.erase(headerName);
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
	else
	{
		AAMPLOG_ERR("PrivateInstanceAAMP::%s - invalid drm type received.\n", __FUNCTION__);
		return;
	}

	AAMPLOG_INFO("PrivateInstanceAAMP::%s - set license url - %s for type - %d\n", __FUNCTION__, url, type);
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
	logprintf("PrivateInstanceAAMP::%s(), vodTrickplayFPS %d\n", __FUNCTION__, vodTrickplayFPS);
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
	logprintf("PrivateInstanceAAMP::%s(), linearTrickplayFPS %d\n", __FUNCTION__, linearTrickplayFPS);
}


/**
 *   @brief Set live offset [Sec]
 *
 *   @param SetLiveOffset - Live Offset
 */
void PrivateInstanceAAMP::SetLiveOffset(int liveoffset)
{
	mLiveOffset = liveoffset;
	mNewLiveOffsetflag = true;
	logprintf("PrivateInstanceAAMP::%s(), liveoffset %d\n", __FUNCTION__, liveoffset);
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

void PrivateInstanceAAMP::SetReportInterval(int reportIntervalMS)
{
	gpGlobalConfig->reportProgressInterval = reportIntervalMS;
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

	for (int cnt=0; cnt < mMaxLanguageCount; cnt ++)
	{
		if(strncmp(mLanguageList[cnt],language,MAX_LANGUAGE_TAG_LENGTH) == 0)
		{
			mCurrentLanguageIndex = cnt;
			break;
		}
	}
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

	int type = 10; //HLS

	if(mIsDash){
		type = 20;
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
					snprintf(moneytracebuf, sizeof(moneytracebuf), "trace-id=%s;parent-id=%u;span-id=%lld",
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
		logprintf("No Moneytrace info available in tune request,need to generate one\n");
		uuid_t uuid;
		uuid_generate(uuid);
		char uuidstr[128];
		uuid_unparse(uuid, uuidstr);
		for (char *ptr = uuidstr; *ptr; ++ptr) {
			*ptr = tolower(*ptr);
		}
		snprintf(moneytracebuf,sizeof(moneytracebuf),"trace-id=%s;parent-id=%u;span-id=%lld",uuidstr,aamp_GetCurrentTimeMS(),aamp_GetCurrentTimeMS());
		customHeader.append(moneytracebuf);
	}	
	AAMPLOG_TRACE("[GetMoneyTraceString] MoneyTrace[%s]\n",customHeader.c_str());
}
#endif /* AAMP_MPD_DRM */

/**
 * @brief Send tuned event if configured to sent after decryption
 */
void PrivateInstanceAAMP::NotifyFirstFragmentDecrypted()
{
	if(mTunedEventPending)
	{
		TunedEventConfig tunedEventConfig =  IsLive() ? gpGlobalConfig->tunedEventConfigLive : gpGlobalConfig->tunedEventConfigVOD;
		if (eTUNED_EVENT_ON_FIRST_FRAGMENT_DECRYPTED == tunedEventConfig)
		{
			if (SendTunedEvent())
			{
				logprintf("aamp: %s - sent tune event after first fragment fetch and decrypt\n", mIsDash ? "mpd" : "hls");
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
		std::string langEntry = *iter;
		if (!langEntry.empty())
		{
			strncpy(event.data.metadata.languages[langCount], langEntry.c_str(), MAX_LANGUAGE_TAG_LENGTH);
			event.data.metadata.languages[langCount][MAX_LANGUAGE_TAG_LENGTH-1] = 0;
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

	logprintf("aamp: sending metadata event and duration update %f\n", ((double)durationMs)/1000);
	SendEventAsync(event);
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

	logprintf("aamp: sending supported speeds changed event with count %d\n", supportedSpeedCount);
	SendEventAsync(event);
}


/**
 *   @brief  Get Sequence Number from URL
 *
 *   @param[in] fragmentUrl fragment Url
 *   @returns Sequence Number if found in fragment Url else 0
 */
long long PrivateInstanceAAMP::GetSeqenceNumberfromURL(const char *fragmentUrl)
{

	long long seqNumber = 0;
	const char *pos = strstr(fragmentUrl, "-frag-");
	const char *ptr;
	if (pos)
	{
		seqNumber = atoll(pos + 6);
	}
	else if (ptr = strstr(fragmentUrl, ".seg"))
	{
		if( (ptr-fragmentUrl >= 5) && (memcmp(ptr - 5, "-init", 5) == 0))
		{
			seqNumber = -1;
		}
		else
		{
			while (ptr > fragmentUrl)
			{
				if (*ptr == '/')
				{
					break;
				}
				ptr--;
			}
			seqNumber = atoll(ptr + 1);
		}
	}
	else if ((strstr(fragmentUrl, ".mpd")) || (strstr(fragmentUrl, ".m3u8")))
	{
		seqNumber = -1;
	}
	return seqNumber;
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
void PrivateInstanceAAMP::SetNetworkTimeout(long timeout)
{
	if (timeout > 0)
	{
		gpGlobalConfig->networkTimeout = timeout;
	}
	AAMPLOG_INFO("PrivateInstanceAAMP::%s:%d network timeout set to - %ld\n", __FUNCTION__, __LINE__, timeout);
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
        AAMPLOG_INFO("%s:%d Ignoring Preferred drm: %d setting as localConfig for Preferred DRM is set to :%d\n", __FUNCTION__, __LINE__, drmType,gpGlobalConfig->preferredDrm);
    }
    else
    {
        AAMPLOG_INFO("%s:%d set Preferred drm: %d\n", __FUNCTION__, __LINE__, drmType);
        gpGlobalConfig->preferredDrm = drmType;
    }
}

#ifdef PLACEMENT_EMULATION
	static int sampleAdIdx = 0;
	static const std::string sampleAds[] = {"http://ccr.ip-ads.xcr.comcast.net/omg04/354092102255/nbcuni.comNBCU2019012500009006/HD_VOD_DAI_XFS09004000H_0125_LVLH03.mpd",
											"http://ccr.ip-ads.xcr.comcast.net/omg07/346241094255/nbcuni.comNBCU2019010200010506/HD_VOD_DAI_QAOA5052100H_0102_LVLH06.mpd"
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
		AAMPLOG_WARN("%s:%d [CDAI] Found Adbreak on period[%s] Duration[%d]\n", __FUNCTION__, __LINE__, adBreakId.c_str(), breakdur);
		std::string adId("");
		std::string url("");

		mCdaiObject->SetAlternateContents(adBreakId, adId, url, startMS);	//A placeholder to avoid multiple scte35 event firing for the same adbreak
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
		ReportTimedMetadata(aamp_GetCurrentTimeMS(), "SCTE35", scte35.c_str(), scte35.size(), adBreakId.c_str(), breakdur);
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
	mCdaiObject->SetAlternateContents(adBreakId, adId, url);
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
		e.data.adResolved.adId = adId.c_str();
		e.data.adResolved.resolveStatus = status;
		e.data.adResolved.startMS = startMS;
		e.data.adResolved.durationMs = durationMs;
		AAMPLOG_WARN("PrivateInstanceAAMP::%s():%d, [CDAI] Sent resolved status of adId[%s]=%d\n", __FUNCTION__, __LINE__, adId.c_str(), e.data.adResolved.resolveStatus);
		SendEventAsync(e);
#endif
	}
}

/**
 *   @brief Deliver pending Ad events to JSPP
 */
void PrivateInstanceAAMP::DeliverAdEvents()
{
	std::lock_guard<std::mutex> lock(mAdEventQMtx);
	while (!mAdEventsQ.empty())
	{
		AAMPEvent &e = mAdEventsQ.front();
		SendEventSync(e);
		AAMPLOG_WARN("PrivateInstanceAAMP::%s():%d, [CDAI] Delivered AdEvent[%s] to JSPP.\n", __FUNCTION__, __LINE__, ADEVENT2STRING(e.type));
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
		AAMPLOG_INFO("PrivateInstanceAAMP::%s():%d, [CDAI] Pushed [%s] of adBreakId[%s] to Queue.\n", __FUNCTION__, __LINE__, ADEVENT2STRING(type), adBreakId.c_str());

		{
			{
				std::lock_guard<std::mutex> lock(mAdEventQMtx);
				mAdEventsQ.push(e);
			}
			if(immediate)
			{
				//Despatch all ad events now
				DeliverAdEvents();
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
		AAMPLOG_INFO("PrivateInstanceAAMP::%s():%d, [CDAI] Pushed [%s] of adId[%s] to Queue.\n", __FUNCTION__, __LINE__, ADEVENT2STRING(type), adId.c_str());

		{
			{
				std::lock_guard<std::mutex> lock(mAdEventQMtx);
				mAdEventsQ.push(e);
			}
			if(immediate)
			{
				//Despatch all ad events now
				DeliverAdEvents();
			}
		}
	}
}

std::string PrivateInstanceAAMP::getStreamTypeString()
{
	std::string type;

	if(mIsDash)
	{
		type = "DASH";
	}
	else
	{
		type = "HLS";
	}

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
		case eMEDIATYPE_MANIFEST:
			pbt = PROFILE_BUCKET_MANIFEST;
			break;
		case eMEDIATYPE_INIT_VIDEO:
			pbt = PROFILE_BUCKET_INIT_VIDEO;
			break;
		case eMEDIATYPE_INIT_AUDIO:
			pbt = PROFILE_BUCKET_INIT_AUDIO;
			break;
		case eMEDIATYPE_PLAYLIST_VIDEO:
			pbt = PROFILE_BUCKET_PLAYLIST_VIDEO;
			break;
		case eMEDIATYPE_PLAYLIST_AUDIO:
			pbt = PROFILE_BUCKET_PLAYLIST_AUDIO;
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
	strncpy(tunedManifestUrl, manifestUrl, MAX_URI_LENGTH-1);
	tunedManifestUrl[MAX_URI_LENGTH-1] = 0;

	if(isrecordedUrl)
	{
		DeFog(tunedManifestUrl);
		// replace leading http:// or https:// with _fog:// or _fogs://
		tunedManifestUrl[0] = '_';
		tunedManifestUrl[1] = 'f';
		tunedManifestUrl[2] = 'o';
		tunedManifestUrl[3] = 'g';
	}

	char *streamPtr = strchr(tunedManifestUrl, '?');
	if(streamPtr)
	{ // strip URI params for more compact URI in microevent logging
		*streamPtr = 0x0;
	}

	traceprintf("PrivateInstanceAAMP::%s, tunedManifestUrl:%s \n", __FUNCTION__, tunedManifestUrl);
}

/**
 *   @brief Gets Recorded URL from Manifest received form XRE.
 *   @param[out] manifestUrl - for VOD and recordedUrl for FOG enabled
 */
const char* PrivateInstanceAAMP::GetTunedManifestUrl()
{
	traceprintf("PrivateInstanceAAMP::%s, tunedManifestUrl:%s \n", __FUNCTION__, tunedManifestUrl);
	return tunedManifestUrl;
}

/**
 *   @brief To set the network proxy
 *
 *   @param[in] network proxy to use
 */
void PrivateInstanceAAMP::SetNetworkProxy(const char * proxy)
{
	if(mNetworkProxy)
	{
		free(mNetworkProxy);
	}
	mNetworkProxy = strdup(proxy);
}

/**
 *   @brief To set the proxy for license request
 *
 *   @param[in] proxy to use for license request
 */
void PrivateInstanceAAMP::SetLicenseReqProxy(const char * licenseProxy)
{
	if(mLicenseProxy)
	{
		free(mLicenseProxy);
	}
	mLicenseProxy = strdup(licenseProxy);
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
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, (int) type);
	}
#endif
	if (!mTrackInjectionBlocked[type])
	{
		AAMPLOG_TRACE("PrivateInstanceAAMP::%s for type %s\n", __FUNCTION__, (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mTrackInjectionBlocked[type] = true;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d\n", __FUNCTION__, (int) type);
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
		logprintf ("PrivateInstanceAAMP::%s Enter. type = %d\n", __FUNCTION__, (int) type);
	}
#endif
	if (mTrackInjectionBlocked[type])
	{
		AAMPLOG_TRACE("PrivateInstanceAAMP::%s for type %s\n", __FUNCTION__, (type == eMEDIATYPE_AUDIO) ? "audio" : "video");
		pthread_mutex_lock(&mLock);
		mTrackInjectionBlocked[type] = false;
		pthread_mutex_unlock(&mLock);
	}
	traceprintf ("PrivateInstanceAAMP::%s Exit. type = %d\n", __FUNCTION__, (int) type);
}

/**
 * @}
 */

