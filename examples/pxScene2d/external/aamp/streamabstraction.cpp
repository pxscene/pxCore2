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
 * @file streamabstraction.cpp
 * @brief Definition of common class functions used by fragment collectors.
 */

#include "StreamAbstractionAAMP.h"
#include <assert.h>
#include <errno.h>
#include <math.h>
#include <iterator>
#include <sys/time.h>

#ifdef USE_MAC_FOR_RANDOM_GEN
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include "base16.h"
#endif

#define AAMP_DEFAULT_BANDWIDTH_BYTES_PREALLOC (256*1024/8)
#define AAMP_STALL_CHECK_TOLERANCE 2
#define AAMP_BUFFER_MONITOR_GREEN_THRESHOLD 4 //2 fragments for Comcast linear streams.
#define DEFER_DRM_LIC_OFFSET_FROM_START 5
#define DEFER_DRM_LIC_OFFSET_TO_UPPER_BOUND 5
#define MAC_STRING_LEN 12
#define URAND_STRING_LEN 16
#define RAND_STRING_LEN (MAC_STRING_LEN + 2*URAND_STRING_LEN)

using namespace std;

/**
 * @brief Thread funtion for Buffer Health Monitoring
 *
 * @return NULL
 */
static void* BufferHealthMonitor(void* user_data)
{
	MediaTrack *track = (MediaTrack *)user_data;
	if(aamp_pthread_setname(pthread_self(), "aampBuffHealth"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
	}
	track->MonitorBufferHealth();
	return NULL;
}

/**
 * @brief Get string corresponding to buffer status.
 *
 * @return string representation of buffer status
 */
const char* MediaTrack::GetBufferHealthStatusString(BufferHealthStatus status)
{
	const char* ret = NULL;
	switch (status)
	{
		default:
		case BUFFER_STATUS_GREEN:
			ret = "GREEN";
			break;
		case BUFFER_STATUS_YELLOW:
			ret = "YELLOW";
			break;
		case BUFFER_STATUS_RED:
			ret = "RED";
			break;
	}
	return ret;
}

/**
 * @brief Monitors buffer health of track
 */
void MediaTrack::MonitorBufferHealth()
{
	assert(gpGlobalConfig->bufferHealthMonitorDelay >= gpGlobalConfig->bufferHealthMonitorInterval);
	unsigned int bufferMontiorSceduleTime = gpGlobalConfig->bufferHealthMonitorDelay - gpGlobalConfig->bufferHealthMonitorInterval;
	aamp->InterruptableMsSleep(bufferMontiorSceduleTime *1000);
	int monitorInterval = gpGlobalConfig->bufferHealthMonitorInterval  * 1000;
	bool keepRunning = true;
	while(keepRunning)
	{
		aamp->InterruptableMsSleep(monitorInterval);
		pthread_mutex_lock(&mutex);
		if (aamp->DownloadsAreEnabled() && !abort)
		{
			if ( numberOfFragmentsCached > 0)
			{
				bufferStatus = BUFFER_STATUS_GREEN;
			}
			else
			{
				double bufferedTime = totalInjectedDuration - GetContext()->GetElapsedTime();
				if (bufferedTime > AAMP_BUFFER_MONITOR_GREEN_THRESHOLD)
				{
					bufferStatus = BUFFER_STATUS_GREEN;
				}
				else
				{
					logprintf("%s:%d [%s] bufferedTime %f totalInjectedDuration %f elapsed time %f\n",__FUNCTION__, __LINE__,
							name, bufferedTime, totalInjectedDuration, GetContext()->GetElapsedTime());
					if (bufferedTime <= 0)
					{
						bufferStatus = BUFFER_STATUS_RED;
					}
					else
					{
						bufferStatus = BUFFER_STATUS_YELLOW;
					}
				}
			}
			if (bufferStatus != prevBufferStatus)
			{
				logprintf("aamp: track[%s] buffering %s->%s\n", name, GetBufferHealthStatusString(prevBufferStatus),
						GetBufferHealthStatusString(bufferStatus));
				prevBufferStatus = bufferStatus;
			}
			else
			{
				traceprintf("%s:%d track[%s] No Change [%s]\n", __FUNCTION__, __LINE__, name,
						GetBufferHealthStatusString(bufferStatus));
			}
		}
		else
		{
			keepRunning = false;
		}
		pthread_mutex_unlock(&mutex);
	}
}

/**
 * @brief Updates internal state after a fragment inject
 */
void MediaTrack::UpdateTSAfterInject()
{
	pthread_mutex_lock(&mutex);
	aamp_Free(&cachedFragment[fragmentIdxToInject].fragment.ptr);
	memset(&cachedFragment[fragmentIdxToInject], 0, sizeof(CachedFragment));
	fragmentIdxToInject++;
	if (fragmentIdxToInject == gpGlobalConfig->maxCachedFragmentsPerTrack)
	{
		fragmentIdxToInject = 0;
	}
	numberOfFragmentsCached--;
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] updated fragmentIdxToInject = %d numberOfFragmentsCached %d\n", __FUNCTION__, __LINE__,
		        name, fragmentIdxToInject, numberOfFragmentsCached);
	}
#endif
	pthread_cond_signal(&fragmentInjected);
	pthread_mutex_unlock(&mutex);
}


/**
 * @brief Updates internal state after a fragment fetch
 */
void MediaTrack::UpdateTSAfterFetch()
{
	bool notifyCacheCompleted = false;
	pthread_mutex_lock(&mutex);
	cachedFragment[fragmentIdxToFetch].profileIndex = GetContext()->profileIdxForBandwidthNotification;
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] before update fragmentIdxToFetch = %d numberOfFragmentsCached %d\n",
		        __FUNCTION__, __LINE__, name, fragmentIdxToFetch, numberOfFragmentsCached);
	}
#endif
	totalFetchedDuration += cachedFragment[fragmentIdxToFetch].duration;

	if((eTRACK_VIDEO == type) && aamp->IsFragmentBufferingRequired())
	{
		if(!notifiedCachingComplete)
		{
			cacheDurationSeconds += cachedFragment[fragmentIdxToFetch].duration;
			if(cacheDurationSeconds >= gpGlobalConfig->minVODCacheSeconds)
			{
				logprintf("## %s:%d [%s] Caching Complete cacheDuration %d minVODCacheSeconds %d##\n", __FUNCTION__, __LINE__, name, cacheDurationSeconds, gpGlobalConfig->minVODCacheSeconds);
				notifyCacheCompleted = true;
			}
			else
			{
				logprintf("## %s:%d [%s] Caching Ongoing cacheDuration %d minVODCacheSeconds %d##\n", __FUNCTION__, __LINE__, name, cacheDurationSeconds, gpGlobalConfig->minVODCacheSeconds);
			}
		}
	}
	fragmentIdxToFetch++;
	if (fragmentIdxToFetch == gpGlobalConfig->maxCachedFragmentsPerTrack)
	{
		fragmentIdxToFetch = 0;
	}
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		if (numberOfFragmentsCached == 0)
		{
			logprintf("## %s:%d [%s] Caching fragment for track when numberOfFragmentsCached is 0 ##\n", __FUNCTION__, __LINE__, name);
		}
	}
#endif
	numberOfFragmentsCached++;
	assert(numberOfFragmentsCached <= gpGlobalConfig->maxCachedFragmentsPerTrack);
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] updated fragmentIdxToFetch = %d numberOfFragmentsCached %d\n",
			__FUNCTION__, __LINE__, name, fragmentIdxToFetch, numberOfFragmentsCached);
	}
#endif
	totalFragmentsDownloaded++;
	pthread_cond_signal(&fragmentFetched);
	pthread_mutex_unlock(&mutex);
	if(notifyCacheCompleted)
	{
		aamp->NotifyFragmentCachingComplete();
		notifiedCachingComplete = true;
	}
}


/**
 * @brief Wait until a free fragment is available.
 * @note To be called before fragment fetch by subclasses
 * @param timeoutMs - timeout in milliseconds. -1 for infinite wait
 * @retval true if fragment available, false on abort.
 */
bool MediaTrack::WaitForFreeFragmentAvailable( int timeoutMs)
{
	bool ret = true;
	int pthreadReturnValue = 0;

	pthread_mutex_lock(&mutex);
	if(abort)
	{
		ret = false;
	}
	else if (numberOfFragmentsCached == gpGlobalConfig->maxCachedFragmentsPerTrack)
	{
		if (timeoutMs >= 0)
		{
			struct timespec tspec;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			tspec.tv_sec = time(NULL) + timeoutMs / 1000;
			tspec.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (timeoutMs % 1000));
			tspec.tv_sec += tspec.tv_nsec / (1000 * 1000 * 1000);
			tspec.tv_nsec %= (1000 * 1000 * 1000);

			pthreadReturnValue = pthread_cond_timedwait(&fragmentInjected, &mutex, &tspec);

			if (ETIMEDOUT == pthreadReturnValue)
			{
				ret = false;
			}
			else if (0 != pthreadReturnValue)
			{
				logprintf("%s:%d [%s] pthread_cond_timedwait returned %s\n", __FUNCTION__, __LINE__, name, strerror(pthreadReturnValue));
				ret = false;
			}
		}
		else
		{
#ifdef AAMP_DEBUG_FETCH_INJECT
			if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
			{
				logprintf("%s:%d [%s] waiting for fragmentInjected condition\n", __FUNCTION__, __LINE__, name);
			}
#endif
			pthreadReturnValue = pthread_cond_wait(&fragmentInjected, &mutex);

			if (0 != pthreadReturnValue)
			{
				logprintf("%s:%d [%s] pthread_cond_wait returned %s\n", __FUNCTION__, __LINE__, name, strerror(pthreadReturnValue));
				ret = false;
			}
#ifdef AAMP_DEBUG_FETCH_INJECT
			if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
			{
				logprintf("%s:%d [%s] wait complete for fragmentInjected\n", __FUNCTION__, __LINE__, name);
			}
#endif
		}
		if(abort)
		{
#ifdef AAMP_DEBUG_FETCH_INJECT
			if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
			{
				logprintf("%s:%d [%s] abort set, returning false\n", __FUNCTION__, __LINE__, name);
			}
#endif
			ret = false;
		}
	}
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] fragmentIdxToFetch = %d numberOfFragmentsCached %d\n",
			__FUNCTION__, __LINE__, name, fragmentIdxToFetch, numberOfFragmentsCached);
	}
#endif
	pthread_mutex_unlock(&mutex);
	return ret;
}

/**
 * @brief Wait until a cached fragment is available.
 * @retval true if fragment available, false on abort.
 */
bool MediaTrack::WaitForCachedFragmentAvailable()
{
	bool ret;
	pthread_mutex_lock(&mutex);
	if ((numberOfFragmentsCached == 0) && !(abort || abortInject))
	{
#ifdef AAMP_DEBUG_FETCH_INJECT
		if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
		{
			logprintf("## %s:%d [%s] Waiting for CachedFragment to be available, eosReached=%d ##\n", __FUNCTION__, __LINE__, name, eosReached);
		}
#endif
		if (!eosReached)
		{
			pthread_cond_wait(&fragmentFetched, &mutex);
		}
	}
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] fragmentIdxToInject = %d numberOfFragmentsCached %d\n",
			__FUNCTION__, __LINE__, name, fragmentIdxToInject, numberOfFragmentsCached);
	}
#endif
	ret = !(abort || abortInject || (numberOfFragmentsCached == 0));
	pthread_mutex_unlock(&mutex);
	return ret;
}


/**
 * @brief Aborts wait for fragment.
 * @param[in] immediate Indicates immediate abort as in a seek/ stop
 */
void MediaTrack::AbortWaitForCachedAndFreeFragment(bool immediate)
{
	pthread_mutex_lock(&mutex);
	if (immediate)
	{
		abort = true;
#ifdef AAMP_DEBUG_FETCH_INJECT
		if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
		{
			logprintf("%s:%d [%s] signal fragmentInjected condition\n", __FUNCTION__, __LINE__, name);
		}
#endif
		pthread_cond_signal(&fragmentInjected);
	}
	pthread_cond_signal(&fragmentFetched);
	pthread_mutex_unlock(&mutex);
}


/**
 * @brief Aborts wait for cached fragment.
 */
void MediaTrack::AbortWaitForCachedFragment()
{
	pthread_mutex_lock(&mutex);
	abortInject = true;
#ifdef AAMP_DEBUG_FETCH_INJECT
	if ((1 << type) & AAMP_DEBUG_FETCH_INJECT)
	{
		logprintf("%s:%d [%s] signal fragmentInjected condition\n", __FUNCTION__, __LINE__, name);
	}
#endif
	pthread_cond_signal(&fragmentFetched);
	pthread_mutex_unlock(&mutex);
}


/**
 * @brief Inject next cached fragment
 */
bool MediaTrack::InjectFragment()
{
	bool ret = true;
	aamp->BlockUntilGstreamerWantsData(NULL, 0, type);

	if (WaitForCachedFragmentAvailable())
	{
		bool stopInjection = false;
		bool fragmentDiscarded = false;
		CachedFragment* cachedFragment = &this->cachedFragment[fragmentIdxToInject];
#ifdef TRACE
		logprintf("%s:%d [%s] - fragmentIdxToInject %d cachedFragment %p ptr %p\n", __FUNCTION__, __LINE__,
				name, fragmentIdxToInject, cachedFragment, cachedFragment->fragment.ptr);
#endif
		if (cachedFragment->fragment.ptr)
		{
			StreamAbstractionAAMP* context = GetContext();
#ifdef AAMP_DEBUG_INJECT
			if ((1 << type) & AAMP_DEBUG_INJECT)
			{
				if (cachedFragment->discontinuity)
				{
					logprintf("%s:%d [%s] Discontinuity present. uri %s\n", __FUNCTION__, __LINE__, name, cachedFragment->uri);
				}
			}
#endif
			if ((cachedFragment->discontinuity || ptsError) && (AAMP_NORMAL_PLAY_RATE == context->aamp->rate))
			{
				logprintf("%s:%d - track %s - notifying aamp discontinuity\n", __FUNCTION__, __LINE__, name);
				cachedFragment->discontinuity = false;
				ptsError = false;
				stopInjection = aamp->Discontinuity((MediaType) type);

				/*For muxed streams, give discontinuity for audio track as well*/
				MediaTrack* audio = context->GetMediaTrack(eTRACK_AUDIO);

				if (audio && !audio->enabled)
				{
					aamp->Discontinuity(eMEDIATYPE_AUDIO);
				}

				if (stopInjection)
				{
					ret = false;
					discontinuityProcessed = true;
					logprintf("%s:%d - stopping injection\n", __FUNCTION__, __LINE__);
				}
				else
				{
					logprintf("%s:%d - continuing injection\n", __FUNCTION__, __LINE__);
				}
			}
			else if (cachedFragment->discontinuity)
			{
				SignalTrickModeDiscontinuity();
			}

			if (!stopInjection)
			{
#ifdef AAMP_DEBUG_INJECT
				if ((1 << type) & AAMP_DEBUG_INJECT)
				{
					logprintf("%s:%d [%s] Inject uri %s\n", __FUNCTION__, __LINE__, name, cachedFragment->uri);
				}
#endif
#ifndef SUPRESS_DECODE
#ifndef FOG_HAMMER_TEST // support aamp stress-tests of fog without video decoding/presentation
				InjectFragmentInternal(cachedFragment, fragmentDiscarded);
#endif
#endif
				if (GetContext()->mIsFirstBuffer && !fragmentDiscarded)
				{
					GetContext()->mIsFirstBuffer = false;
					aamp->NotifyFirstBufferProcessed();
				}
				if (eTRACK_VIDEO == type)
				{
					GetContext()->NotifyBitRateUpdate(cachedFragment->profileIndex);
				}
				AAMPLOG_TRACE("%s:%d [%p] - %s - injected cached uri at pos %f dur %f\n", __FUNCTION__, __LINE__, this, name, cachedFragment->position, cachedFragment->duration);
				if (!fragmentDiscarded)
				{
					totalInjectedDuration += cachedFragment->duration;
					mSegInjectFailCount = 0;
				}
				else
				{
					logprintf("%s:%d [%s] - Not updating totalInjectedDuration since fragment is Discarded\n", __FUNCTION__, __LINE__, name);
					mSegInjectFailCount++;
					if(MAX_SEG_INJECT_FAIL_COUNT <= mSegInjectFailCount)
					{
						ret	= false;
						logprintf("%s:%d [%s] Reached max inject failure count, stopping playback\n",__FUNCTION__, __LINE__, name);
						aamp->SendErrorEvent(AAMP_TUNE_FAILED_PTS_ERROR);
					}
					
				}
				UpdateTSAfterInject();
			}
		}
		else
		{
			if (eosReached)
			{
				//Save the playback rate prior to sending EOS
				int rate = GetContext()->aamp->rate;
				aamp->EndOfStreamReached((MediaType)type);
				/*For muxed streams, provide EOS for audio track as well since
				 * no separate MediaTrack for audio is present*/
				MediaTrack* audio = GetContext()->GetMediaTrack(eTRACK_AUDIO);
				if (audio && !audio->enabled && rate == AAMP_NORMAL_PLAY_RATE)
				{
					aamp->EndOfStreamReached(eMEDIATYPE_AUDIO);
				}
			}
			else
			{
				logprintf("%s:%d - %s - NULL ptr to inject. fragmentIdxToInject %d\n", __FUNCTION__, __LINE__, name, fragmentIdxToInject);
			}
			ret = false;
		}
	}
	else
	{
		logprintf("WaitForCachedFragmentAvailable %s aborted\n", name);
		if (eosReached)
		{
			//Save the playback rate prior to sending EOS
			int rate = GetContext()->aamp->rate;
			aamp->EndOfStreamReached((MediaType)type);
			/*For muxed streams, provide EOS for audio track as well since
			 * no separate MediaTrack for audio is present*/
			MediaTrack* audio = GetContext()->GetMediaTrack(eTRACK_AUDIO);
			if (audio && !audio->enabled && rate == AAMP_NORMAL_PLAY_RATE)
			{
				aamp->EndOfStreamReached(eMEDIATYPE_AUDIO);
			}
		}
		ret = false;
	}
	return ret;
} // InjectFragment



/**
 * @brief Fragment injector thread
 * @param arg Pointer to MediaTrack
 * @retval NULL
 */
static void *FragmentInjector(void *arg)
{
	MediaTrack *track = (MediaTrack *)arg;
	if(aamp_pthread_setname(pthread_self(), "aampInjector"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
	}
	track->RunInjectLoop();
	return NULL;
}



/**
 * @brief Starts inject loop of track
 */
void MediaTrack::StartInjectLoop()
{
	abort = false;
	abortInject = false;
	discontinuityProcessed = false;
	assert(!fragmentInjectorThreadStarted);
	if (0 == pthread_create(&fragmentInjectorThreadID, NULL, &FragmentInjector, this))
	{
		fragmentInjectorThreadStarted = true;
	}
	else
	{
		logprintf("Failed to create FragmentInjector thread\n");
	}
}


/**
 * @brief Injection loop - use internally by injection logic
 */
void MediaTrack::RunInjectLoop()
{
	const bool isAudioTrack = (eTRACK_AUDIO == type);
	bool notifyFirstFragment = true;
	bool keepInjecting = true;
	if ((AAMP_NORMAL_PLAY_RATE == aamp->rate) && !bufferMonitorThreadStarted )
	{
		if (0 == pthread_create(&bufferMonitorThreadID, NULL, &BufferHealthMonitor, this))
		{
			bufferMonitorThreadStarted = true;
		}
		else
		{
			logprintf("Failed to create BufferHealthMonitor thread errno = %d, %s\n", errno, strerror(errno));
		}
	}
	totalInjectedDuration = 0;
	while (aamp->DownloadsAreEnabled() && keepInjecting)
	{
		if (!InjectFragment())
		{
			keepInjecting = false;
		}
		if (notifyFirstFragment)
		{
			notifyFirstFragment = false;
			GetContext()->NotifyFirstFragmentInjected();
		}
		// BCOM-2959  -- Disable audio video balancing for CDVR content .. 
		// CDVR Content includes eac3 audio, the duration of audio doesnt match with video
		// and hence balancing fetch/inject not needed for CDVR
		if(!gpGlobalConfig->bAudioOnlyPlayback && !aamp->IsCDVRContent())
		{
			if(isAudioTrack)
			{
				GetContext()->WaitForVideoTrackCatchup();
			}
			else
			{
				GetContext()->ReassessAndResumeAudioTrack(false);
			}
		}
	}
	abortInject = true;
	AAMPLOG_WARN("fragment injector done. track %s\n", name);
}


/**
 * @brief Stop inject loop of track
 */
void MediaTrack::StopInjectLoop()
{
	if (fragmentInjectorThreadStarted)
	{
		int rc = pthread_join(fragmentInjectorThreadID, NULL);
		if (rc != 0)
		{
			logprintf("***pthread_join fragmentInjectorThread returned %d(%s)\n", rc, strerror(rc));
		}
#ifdef TRACE
		else
		{
			logprintf("joined fragmentInjectorThread\n");
		}
#endif
	}
	fragmentInjectorThreadStarted = false;
}


/**
 * @brief Check if a track is enabled
 * @retval true if enabled, false if disabled
 */
bool MediaTrack::Enabled()
{
	return enabled;
}


/**
 * @brief Get buffer to fetch and cache next fragment.
 * @param[in] initialize true to initialize the fragment.
 * @retval Pointer to fragment buffer.
 */
CachedFragment* MediaTrack::GetFetchBuffer(bool initialize)
{
	/*Make sure fragmentDurationSeconds updated before invoking this*/
	CachedFragment* cachedFragment = &this->cachedFragment[fragmentIdxToFetch];
	if(initialize)
	{
		if (cachedFragment->fragment.ptr)
		{
			logprintf("%s:%d fragment.ptr already set - possible memory leak\n", __FUNCTION__, __LINE__);
		}
		memset(&cachedFragment->fragment, 0x00, sizeof(GrowableBuffer));
	}
	return cachedFragment;
}


/**
 * @brief Set current bandwidth of track
 * @param bandwidthBps bandwidth in bits per second
 */
void MediaTrack::SetCurrentBandWidth(int bandwidthBps)
{
	this->bandwidthBytesPerSecond = bandwidthBps/8;
}

/**
 * @brief Get current bandwidth of track
 * @return bandwidth in bytes per second
 */
int MediaTrack::GetCurrentBandWidth()
{
	return this->bandwidthBytesPerSecond;
}


/**
 * @brief MediaTrack Constructor
 * @param type Type of track
 * @param aamp Pointer to associated aamp instance
 * @param name Name of the track
 */
MediaTrack::MediaTrack(TrackType type, PrivateInstanceAAMP* aamp, const char* name) :
		eosReached(false), enabled(false), numberOfFragmentsCached(0), fragmentIdxToInject(0),
		fragmentIdxToFetch(0), abort(false), fragmentInjectorThreadID(0), bufferMonitorThreadID(0), totalFragmentsDownloaded(0),
		fragmentInjectorThreadStarted(false), bufferMonitorThreadStarted(false), totalInjectedDuration(0), cacheDurationSeconds(0),
		notifiedCachingComplete(false), fragmentDurationSeconds(0), segDLFailCount(0),segDrmDecryptFailCount(0),mSegInjectFailCount(0),
		bufferStatus(BUFFER_STATUS_GREEN), prevBufferStatus(BUFFER_STATUS_GREEN),
		bandwidthBytesPerSecond(AAMP_DEFAULT_BANDWIDTH_BYTES_PREALLOC), totalFetchedDuration(0),
		discontinuityProcessed(false), ptsError(false), cachedFragment(NULL), name(name), type(type), aamp(aamp),
		mutex(), fragmentFetched(), fragmentInjected(), abortInject(false)
{
	cachedFragment = new CachedFragment[gpGlobalConfig->maxCachedFragmentsPerTrack];
	for(int X =0; X< gpGlobalConfig->maxCachedFragmentsPerTrack; ++X){
		memset(&cachedFragment[X], 0, sizeof(CachedFragment));
	}
	pthread_cond_init(&fragmentFetched, NULL);
	pthread_cond_init(&fragmentInjected, NULL);
	pthread_mutex_init(&mutex, NULL);
}


/**
 * @brief MediaTrack Destructor
 */
MediaTrack::~MediaTrack()
{
	if (bufferMonitorThreadStarted)
	{
		int rc = pthread_join(bufferMonitorThreadID, NULL);
		if (rc != 0)
		{
			logprintf("***pthread_join bufferMonitorThreadID returned %d(%s)\n", rc, strerror(rc));
		}
#ifdef TRACE
		else
		{
			logprintf("joined bufferMonitorThreadID\n");
		}
#endif
	}
	for (int j=0; j< gpGlobalConfig->maxCachedFragmentsPerTrack; j++)
	{
		aamp_Free(&cachedFragment[j].fragment.ptr);
	}
	if(cachedFragment)
	{
		delete [] cachedFragment;
		cachedFragment = NULL;
	}
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&fragmentFetched);
	pthread_cond_destroy(&fragmentInjected);
}

/**
 *   @brief Unblocks track if caught up with video or downloads are stopped
 *
 */
void StreamAbstractionAAMP::ReassessAndResumeAudioTrack(bool abort)
{
	MediaTrack *audio = GetMediaTrack(eTRACK_AUDIO);
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	if( audio && video )
	{
		pthread_mutex_lock(&mLock);
		double audioDuration = audio->GetTotalInjectedDuration();
		double videoDuration = video->GetTotalInjectedDuration();
		if(audioDuration < (videoDuration + (2 * video->fragmentDurationSeconds)) || !aamp->DownloadsAreEnabled() || video->IsDiscontinuityProcessed() || abort)
		{
			pthread_cond_signal(&mCond);
#ifdef AAMP_DEBUG_FETCH_INJECT
			logprintf("\n%s:%d signalling cond - audioDuration %f videoDuration %f\n",
				__FUNCTION__, __LINE__, audioDuration, videoDuration);
#endif
		}
		pthread_mutex_unlock(&mLock);
	}
}


/**
 *   @brief Waits track injection until caught up with video track.
 *   Used internally by injection logic
 */
void StreamAbstractionAAMP::WaitForVideoTrackCatchup()
{
	MediaTrack *audio = GetMediaTrack(eTRACK_AUDIO);
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);

	struct timespec ts;
	struct timeval tv;
	int waitTimeInMs = 100;
	int ret = 0;

	pthread_mutex_lock(&mLock);
	double audioDuration = audio->GetTotalInjectedDuration();
	double videoDuration = video->GetTotalInjectedDuration();

	while ((audioDuration > (videoDuration + video->fragmentDurationSeconds)) && aamp->DownloadsAreEnabled() && !audio->IsDiscontinuityProcessed() && !video->IsInjectionAborted())
	{
#ifdef AAMP_DEBUG_FETCH_INJECT
		logprintf("\n%s:%d waiting for cond - audioDuration %f videoDuration %f\n",
			__FUNCTION__, __LINE__, audioDuration, videoDuration);
#endif
		gettimeofday(&tv, NULL);
		ts.tv_sec = time(NULL) + waitTimeInMs / 1000;
		ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (waitTimeInMs % 1000));
		ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
		ts.tv_nsec %= (1000 * 1000 * 1000);

		ret = pthread_cond_timedwait(&mCond, &mLock, &ts);

		if (ret == 0)
		{
			break;
		}
#ifndef WIN32
		if (ret != ETIMEDOUT)
		{
			logprintf("%s:%d error while calling pthread_cond_timedwait - %s\n", __FUNCTION__, __LINE__, strerror(ret));
		}
#endif
	}
	pthread_mutex_unlock(&mLock);
}


/**
 * @brief StreamAbstractionAAMP Constructor
 * @param[in] aamp pointer to PrivateInstanceAAMP object associated with stream
 */
StreamAbstractionAAMP::StreamAbstractionAAMP(PrivateInstanceAAMP* aamp):
		trickplayMode(false), currentProfileIndex(0), mCurrentBandwidth(0),
		mTsbBandwidth(0),mNwConsistencyBypass(true), profileIdxForBandwidthNotification(0),
		hasDrm(false), mIsAtLivePoint(false), mIsFirstBuffer(true), mESChangeStatus(false),
		mNetworkDownDetected(false), mTotalPausedDurationMS(0), mIsPaused(false),
		mStartTimeStamp(-1),mLastPausedTimeStamp(-1), aamp(aamp),
		mIsPlaybackStalled(false), mCheckForRampdown(false), mTuneType(), mLock(),
		mCond(), mLastVideoFragCheckedforABR(0), mLastVideoFragParsedTimeMS(0),
		mAbrManager()
{
	mLastVideoFragParsedTimeMS = aamp_GetCurrentTimeMS();
	traceprintf("StreamAbstractionAAMP::%s\n", __FUNCTION__);
	pthread_mutex_init(&mLock, NULL);
	pthread_cond_init(&mCond, NULL);

	// Set default init bitrate according to the config.
	mAbrManager.setDefaultInitBitrate(gpGlobalConfig->defaultBitrate);
	if (gpGlobalConfig->iframeBitrate > 0)
	{
		mAbrManager.setDefaultIframeBitrate(gpGlobalConfig->iframeBitrate);
	}

}


/**
 * @brief StreamAbstractionAAMP Destructor
 */
StreamAbstractionAAMP::~StreamAbstractionAAMP()
{
	traceprintf("StreamAbstractionAAMP::%s\n", __FUNCTION__);
	pthread_cond_destroy(&mCond);
	pthread_mutex_destroy(&mLock);
	AAMPLOG_INFO("Exit StreamAbstractionAAMP::%s\n", __FUNCTION__);
}

/**
 * @brief Get the last video fragment parsed time.
 *
 *	 @param None
 *	 @return Last video fragment parsed time.
 */
double StreamAbstractionAAMP::LastVideoFragParsedTimeMS(void)
{
	return mLastVideoFragParsedTimeMS;
}

/**
 *   @brief Get the desired profile to start fetching.
 *
 *   @param getMidProfile
 *   @retval profile index to be used for the track.
 */
int StreamAbstractionAAMP::GetDesiredProfile(bool getMidProfile)
{
	int desiredProfileIndex;
	if (this->trickplayMode && ABRManager::INVALID_PROFILE != mAbrManager.getLowestIframeProfile())
	{
		desiredProfileIndex = GetIframeTrack();
	}
	else
	{
		desiredProfileIndex = mAbrManager.getInitialProfileIndex(getMidProfile);
	}
	profileIdxForBandwidthNotification = desiredProfileIndex;
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	if(video)
	{
		video->SetCurrentBandWidth(GetStreamInfo(profileIdxForBandwidthNotification)->bandwidthBitsPerSecond);
	}
	else
	{
		AAMPLOG_TRACE("%s:%d video track NULL\n", __FUNCTION__, __LINE__);
	}
	AAMPLOG_TRACE("%s:%d profileIdxForBandwidthNotification updated to %d \n", __FUNCTION__, __LINE__, profileIdxForBandwidthNotification);

	return desiredProfileIndex;
}

/**
 *   @brief Notify bitrate updates to application.
 *   Used internally by injection logic
 *
 *   @param[in]  profileIndex - profile index of last injected fragment.
 */
void StreamAbstractionAAMP::NotifyBitRateUpdate(int profileIndex)
{
	if (profileIndex != aamp->GetPersistedProfileIndex())
	{
		StreamInfo* streamInfo = GetStreamInfo(profileIndex);

		bool lGetBWIndex = false;
		/* START: Added As Part of DELIA-28363 and DELIA-28247 */
		if(aamp->IsTuneTypeNew &&
			streamInfo->bandwidthBitsPerSecond == (GetStreamInfo(GetMaxBWProfile())->bandwidthBitsPerSecond))
		{
			MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
			logprintf("NotifyBitRateUpdate: Max BitRate: %ld, timetotop: %f\n",
				streamInfo->bandwidthBitsPerSecond, video->GetTotalInjectedDuration());
			aamp->IsTuneTypeNew = false;
			lGetBWIndex = true;
		}
		/* END: Added As Part of DELIA-28363 and DELIA-28247 */

		// Send bitrate notification
		aamp->NotifyBitRateChangeEvent(streamInfo->bandwidthBitsPerSecond,
				"BitrateChanged - Network Adaptation", streamInfo->resolution.width,
				streamInfo->resolution.height, lGetBWIndex);
		// Store the profile , compare it before sending it . This avoids sending of event after trickplay if same bitrate
		aamp->SetPersistedProfileIndex(profileIndex);
	}
}



/**
 * @brief Update profile state based on bandwidth of fragments downloaded
 */
void StreamAbstractionAAMP::UpdateProfileBasedOnFragmentDownloaded(void)
{
	// This function checks for bandwidth change based on the fragment url from FOG
	int desiredProfileIndex = 0;
	if (mCurrentBandwidth != mTsbBandwidth)
	{
		// a) Check if network bandwidth changed from starting bw
		// b) Check if netwwork bandwidth is different from persisted bandwidth( needed for first time reporting)
		// find the profile for the newbandwidth
		desiredProfileIndex = mAbrManager.getBestMatchedProfileIndexByBandWidth(mTsbBandwidth);
		mCurrentBandwidth = mTsbBandwidth;
		profileIdxForBandwidthNotification = desiredProfileIndex;
		traceprintf("%s:%d profileIdxForBandwidthNotification updated to %d \n", __FUNCTION__, __LINE__, profileIdxForBandwidthNotification);
		GetMediaTrack(eTRACK_VIDEO)->SetCurrentBandWidth(GetStreamInfo(profileIdxForBandwidthNotification)->bandwidthBitsPerSecond);
	}
}


/**
 * @brief Get desired profile based on cached duration
 * @retval index of desired profile based on cached duration
 */
int StreamAbstractionAAMP::GetDesiredProfileBasedOnCache(void)
{
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	int desiredProfileIndex = currentProfileIndex;

	if (this->trickplayMode)
	{
		int tmpIframeProfile = GetIframeTrack();
		if(tmpIframeProfile != ABRManager::INVALID_PROFILE)
			desiredProfileIndex = tmpIframeProfile;
	}
	/*In live, fog takes care of ABR, and cache updating is not based only on bandwidth,
	 * but also depends on fragment availability in CDN*/
	else
	{
		long currentBandwidth = GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond;
		long networkBandwidth = aamp->GetCurrentlyAvailableBandwidth();
		int nwConsistencyCnt = (mNwConsistencyBypass)?1:gpGlobalConfig->abrNwConsistency;
		// Ramp up/down (do ABR)
		desiredProfileIndex = mAbrManager.getProfileIndexByBitrateRampUpOrDown(currentProfileIndex,
				currentBandwidth, networkBandwidth, nwConsistencyCnt);
		if(currentProfileIndex != desiredProfileIndex)
		{
			logprintf("aamp::GetDesiredProfileBasedOnCache---> currProf[%d] desiredProf[%d] vidCache[%d]\n",currentProfileIndex,desiredProfileIndex,video->numberOfFragmentsCached);
		}
	}
	// only for first call, consistency check is ignored
	mNwConsistencyBypass = false;

	return desiredProfileIndex;
}


/**
 * @brief Rampdown profile
 *
 * @param[in] http_error
 * @retval true on profile change
 */
bool StreamAbstractionAAMP::RampDownProfile(long http_error)
{
	bool ret = false;
	int desiredProfileIndex = currentProfileIndex;
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	if (this->trickplayMode)
	{
		//We use only second last and lowest profiles for iframes
		int lowestIframeProfile = mAbrManager.getLowestIframeProfile();
		if (desiredProfileIndex != lowestIframeProfile)
		{
			if (ABRManager::INVALID_PROFILE != lowestIframeProfile)
			{
				desiredProfileIndex = lowestIframeProfile;
			}
			else
			{
				logprintf("%s:%d lowestIframeProfile Invalid - Stream does not has an iframe track!! \n", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		desiredProfileIndex = mAbrManager.getRampedDownProfileIndex(currentProfileIndex);
	}
	if (desiredProfileIndex != currentProfileIndex)
	{
		AAMPAbrInfo stAbrInfo = {};

		stAbrInfo.abrCalledFor = AAMPAbrFragmentDownloadFailed;
		stAbrInfo.currentProfileIndex = currentProfileIndex;
		stAbrInfo.desiredProfileIndex = desiredProfileIndex;
		stAbrInfo.currentBandwidth = GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond;
		stAbrInfo.desiredBandwidth = GetStreamInfo(desiredProfileIndex)->bandwidthBitsPerSecond;
		stAbrInfo.networkBandwidth = aamp->GetCurrentlyAvailableBandwidth();
		stAbrInfo.errorType = AAMPNetworkErrorHttp;
		stAbrInfo.errorCode = (int)http_error;

		AAMP_LOG_ABR_INFO(&stAbrInfo);

		aamp->UpdateVideoEndMetrics(stAbrInfo);

		this->currentProfileIndex = desiredProfileIndex;
		profileIdxForBandwidthNotification = desiredProfileIndex;
		traceprintf("%s:%d profileIdxForBandwidthNotification updated to %d \n", __FUNCTION__, __LINE__, profileIdxForBandwidthNotification);
		ret = true;
		video->SetCurrentBandWidth(GetStreamInfo(profileIdxForBandwidthNotification)->bandwidthBitsPerSecond);

		// Send abr notification to XRE
		video->ABRProfileChanged();
	}

	return ret;
}

/**
 *   @brief Check for ramdown profile.
 *
 *   @param http_error
 *   @retval true if rampdown needed in the case of fragment not available in higher profile.
 */
bool StreamAbstractionAAMP::CheckForRampDownProfile(long http_error)
{
	bool retValue = false;

	if (!aamp->IsTSBSupported())
	{
		if (http_error == 404 || http_error == 500 || http_error == 503 || http_error == CURLE_PARTIAL_FILE)
		{
			if (RampDownProfile(http_error))
			{
				AAMPLOG_INFO("StreamAbstractionAAMP::%s:%d > Condition Rampdown Success\n", __FUNCTION__, __LINE__);
				retValue = true;
			}
		}
		//For timeout, rampdown in single steps might not be enough
		else if (http_error == CURLE_OPERATION_TIMEDOUT)
		{
			if(UpdateProfileBasedOnFragmentCache())
			{
				retValue = true;
			}
			else if (RampDownProfile(http_error))
			{
				retValue = true;
			}
		}
	}

	return retValue;
}


/**
 *   @brief Checks and update profile based on bandwidth.
 */
void StreamAbstractionAAMP::CheckForProfileChange(void)
{
	// FOG based
	if(aamp->IsTSBSupported())
	{
		// This is for FOG based download , where bandwidth is calculated based on downloaded fragment file name
		// No profile change will be done or manifest download triggered based on profilechange
		UpdateProfileBasedOnFragmentDownloaded();
	}
	else
	{
		MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
		bool checkProfileChange = true;
		//Avoid doing ABR during initial buffering which will affect tune times adversely
		if (video->GetTotalFetchedDuration() > 0 && video->GetTotalFetchedDuration() < gpGlobalConfig->abrSkipDuration)
		{
			//For initial fragment downloads, check available bw is less than default bw
			long availBW = aamp->GetCurrentlyAvailableBandwidth();
			long currBW = GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond;

			//If available BW is less than current selected one, we need ABR
			if (availBW > 0 && availBW < currBW)
			{
				logprintf("%s:%d Changing profile due to low available bandwidth(%ld) than default(%ld)!! \n", __FUNCTION__, __LINE__, availBW, currBW);
			}
			else
			{
				checkProfileChange = false;
			}
		}

		if (checkProfileChange)
		{
			UpdateProfileBasedOnFragmentCache();
		}
	}
}


/**
 *   @brief Get iframe track index.
 *   This shall be called only after UpdateIframeTracks() is done
 *
 *   @retval iframe track index.
 */
int StreamAbstractionAAMP::GetIframeTrack()
{
	return mAbrManager.getDesiredIframeProfile();
}


/**
 *   @brief Update iframe tracks.
 *   Subclasses shall invoke this after StreamInfo is populated .
 */
void StreamAbstractionAAMP::UpdateIframeTracks()
{
	mAbrManager.updateProfile();
}


/**
 *   @brief Function called when playback is paused to update related flags.
 *
 *   @param[in] paused - true if playback paused, otherwise false.
 */
void StreamAbstractionAAMP::NotifyPlaybackPaused(bool paused)
{
	mIsPaused = paused;
	if (paused)
	{
		mIsAtLivePoint = false;
		mLastPausedTimeStamp = aamp_GetCurrentTimeMS();
	}
	else
	{
		if(-1 != mLastPausedTimeStamp)
		{
			mTotalPausedDurationMS += (aamp_GetCurrentTimeMS() - mLastPausedTimeStamp);
			mLastPausedTimeStamp = -1;
		}
		else
		{
			logprintf("StreamAbstractionAAMP:%s() mLastPausedTimeStamp -1\n", __FUNCTION__);
		}
	}
}


/**
 *   @brief Check if player caches are running dry.
 *
 *   @return true if player caches are running dry, false otherwise.
 */
bool StreamAbstractionAAMP::CheckIfPlayerRunningDry()
{
	MediaTrack *videoTrack = GetMediaTrack(eTRACK_VIDEO);
	MediaTrack *audioTrack = GetMediaTrack(eTRACK_AUDIO);

	if (!audioTrack || !videoTrack)
	{
		return false;
	}
	bool videoBufferIsEmpty = videoTrack->numberOfFragmentsCached == 0 && aamp->IsSinkCacheEmpty(eMEDIATYPE_VIDEO);
	bool audioBufferIsEmpty = (audioTrack->Enabled() ? (audioTrack->numberOfFragmentsCached == 0) : true) && aamp->IsSinkCacheEmpty(eMEDIATYPE_AUDIO);
	if (videoBufferIsEmpty || audioBufferIsEmpty) /* Changed the condition from '&&' to '||', becasue if video getting stalled it doesn't need to wait until audio become dry */
	{
		logprintf("StreamAbstractionAAMP:%s() Stall detected. Buffer status is RED!\n", __FUNCTION__);
		return true;
	}
	return false;
}


/**
 * @brief Update profile state based on cached fragments
 *
 * @return true if profile was changed, false otherwise
 */
bool StreamAbstractionAAMP::UpdateProfileBasedOnFragmentCache()
{
	bool retVal = false;
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	int desiredProfileIndex = GetDesiredProfileBasedOnCache();
	if (desiredProfileIndex != currentProfileIndex)
	{
#if 0 /* Commented since the same is supported via AAMP_LOG_ABR_INFO */
		logprintf("\n\n**aamp changing profile: %d->%d [%ld->%ld]\n\n",
			currentProfileIndex, desiredProfileIndex,
			GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond,
			GetStreamInfo(desiredProfileIndex)->bandwidthBitsPerSecond);
#else
		AAMPAbrInfo stAbrInfo = {};

		stAbrInfo.abrCalledFor = AAMPAbrBandwidthUpdate;
		stAbrInfo.currentProfileIndex = currentProfileIndex;
		stAbrInfo.desiredProfileIndex = desiredProfileIndex;
		stAbrInfo.currentBandwidth = GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond;
		stAbrInfo.desiredBandwidth = GetStreamInfo(desiredProfileIndex)->bandwidthBitsPerSecond;
		stAbrInfo.networkBandwidth = aamp->GetCurrentlyAvailableBandwidth();
		stAbrInfo.errorType = AAMPNetworkErrorNone;

		AAMP_LOG_ABR_INFO(&stAbrInfo);
		aamp->UpdateVideoEndMetrics(stAbrInfo);
#endif /* 0 */

		this->currentProfileIndex = desiredProfileIndex;
		profileIdxForBandwidthNotification = desiredProfileIndex;
		traceprintf("%s:%d profileIdxForBandwidthNotification updated to %d \n", __FUNCTION__, __LINE__, profileIdxForBandwidthNotification);
		video->ABRProfileChanged();
		video->SetCurrentBandWidth(GetStreamInfo(profileIdxForBandwidthNotification)->bandwidthBitsPerSecond);
		retVal = true;
	}
	return retVal;
}


/**
 *   @brief Check if playback has stalled and update related flags.
 *
 *   @param[in] fragmentParsed - true if next fragment was parsed, otherwise false
 */
void StreamAbstractionAAMP::CheckForPlaybackStall(bool fragmentParsed)
{
	if (fragmentParsed)
	{
		mLastVideoFragParsedTimeMS = aamp_GetCurrentTimeMS();
		if (mIsPlaybackStalled)
		{
			mIsPlaybackStalled = false;
		}
	}
	else
	{
		/** Need to confirm if we are stalled here */
		double timeElapsedSinceLastFragment = (aamp_GetCurrentTimeMS() - mLastVideoFragParsedTimeMS);

		// We have not received a new fragment for a long time, check for cache empty required for dash
		if (!mNetworkDownDetected && (timeElapsedSinceLastFragment > gpGlobalConfig->stallTimeoutInMS) && GetMediaTrack(eTRACK_VIDEO)->numberOfFragmentsCached == 0)
		{
			AAMPLOG_INFO("StreamAbstractionAAMP::%s() Didn't download a new fragment for a long time(%f) and cache empty!\n", __FUNCTION__, timeElapsedSinceLastFragment);
			mIsPlaybackStalled = true;
			if (CheckIfPlayerRunningDry())
			{
				logprintf("StreamAbstractionAAMP::%s() Stall detected!. Time elapsed since fragment parsed(%f), caches are all empty!\n", __FUNCTION__, timeElapsedSinceLastFragment);
				aamp->SendStalledErrorEvent();
			}
		}
	}
}

/**
 *   @brief MediaTracks shall call this to notify first fragment is injected.
 */
void StreamAbstractionAAMP::NotifyFirstFragmentInjected()
{
	pthread_mutex_lock(&mLock);
	mIsPaused = false;
	mLastPausedTimeStamp = -1;
	mTotalPausedDurationMS = 0;
	mStartTimeStamp = aamp_GetCurrentTimeMS();
	pthread_mutex_unlock(&mLock);
}

/**
 *   @brief Get elapsed time of play-back.
 *
 *   @return elapsed time.
 */
double StreamAbstractionAAMP::GetElapsedTime()
{
	double elapsedTime;
	pthread_mutex_lock(&mLock);
	traceprintf("StreamAbstractionAAMP:%s() mStartTimeStamp %lld mTotalPausedDurationMS %lld mLastPausedTimeStamp %lld\n", __FUNCTION__, mStartTimeStamp, mTotalPausedDurationMS, mLastPausedTimeStamp);
	if (!mIsPaused)
	{
		elapsedTime = (double)(aamp_GetCurrentTimeMS() - mStartTimeStamp - mTotalPausedDurationMS) / 1000;
	}
	else
	{
		elapsedTime = (double)(mLastPausedTimeStamp - mStartTimeStamp - mTotalPausedDurationMS) / 1000;
	}
	pthread_mutex_unlock(&mLock);
	return elapsedTime;
}

/**
 *   @brief Get the bitrate of current video profile selected.
 *
 *   @return bitrate of current video profile.
 */
long StreamAbstractionAAMP::GetVideoBitrate(void)
{
	return (GetMediaTrack(eTRACK_VIDEO)->GetCurrentBandWidth() * 8);
}


/**
 *   @brief Get the bitrate of current audio profile selected.
 *
 *   @return bitrate of current audio profile.
 */
long StreamAbstractionAAMP::GetAudioBitrate(void)
{
	//TODO: This is a hardcoded value now, need to make it dynamically populated
	return (GetMediaTrack(eTRACK_AUDIO)->GetCurrentBandWidth() * 8);
}

/**
 *   @brief Check if a preferred bitrate is set and change profile accordingly.
 */
void StreamAbstractionAAMP::CheckUserProfileChangeReq(void)
{
	MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
	//Check if there is an actual change in bitrate
	long userRequestedBandwidth = aamp->GetVideoBitrate();
	if (userRequestedBandwidth != gpGlobalConfig->defaultBitrate)
	{
		int desiredProfileIndex = mAbrManager.getBestMatchedProfileIndexByBandWidth(userRequestedBandwidth);
		if (currentProfileIndex != desiredProfileIndex)
		{
#if 0 /* Commented since the same is supported via AAMP_LOG_ABR_INFO */
			logprintf("\n\n**aamp changing profile based on user request: %d->%d [%ld->%ld]\n\n",
				currentProfileIndex, desiredProfileIndex,
				GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond,
				GetStreamInfo(desiredProfileIndex)->bandwidthBitsPerSecond);
#else
			AAMPAbrInfo stAbrInfo = {};

			stAbrInfo.abrCalledFor = AAMPAbrUnifiedVideoEngine;
			stAbrInfo.currentProfileIndex = currentProfileIndex;
			stAbrInfo.desiredProfileIndex = desiredProfileIndex;
			stAbrInfo.currentBandwidth = GetStreamInfo(currentProfileIndex)->bandwidthBitsPerSecond;
			stAbrInfo.desiredBandwidth = GetStreamInfo(desiredProfileIndex)->bandwidthBitsPerSecond;
			stAbrInfo.networkBandwidth = aamp->GetCurrentlyAvailableBandwidth();
			stAbrInfo.errorType = AAMPNetworkErrorNone;

			AAMP_LOG_ABR_INFO(&stAbrInfo);
#endif /* 0 */
			this->currentProfileIndex = desiredProfileIndex;
			profileIdxForBandwidthNotification = desiredProfileIndex;
			traceprintf("%s:%d profileIdxForBandwidthNotification updated to %d \n", __FUNCTION__, __LINE__, profileIdxForBandwidthNotification);
			video->ABRProfileChanged();
			video->SetCurrentBandWidth(GetStreamInfo(profileIdxForBandwidthNotification)->bandwidthBitsPerSecond);
		}
	}
}

#ifdef USE_MAC_FOR_RANDOM_GEN

/**
 * @brief get EstbMac
 *
 * @param  mac[out] eSTB MAC address
 * @return true on success.
 */
static bool getEstbMac(char* mac)
{
	bool ret = false;
	char nwInterface[IFNAMSIZ] = { 'e', 't', 'h', '0', '\0' };
#ifdef READ_ESTB_IFACE_FROM_DEVICE_PROPERTIES
	FILE* fp = fopen("/etc/device.properties", "rb");
	if (fp)
	{
		logprintf("%s:%d - opened /etc/device.properties\n", __FUNCTION__, __LINE__);
		char buf[MAX_URI_LENGTH * 2];
		while (fgets(buf, sizeof(buf), fp))
		{
			if(strstr(buf, "ESTB_INTERFACE") != NULL)
			{
				const char * nwIfaceNameStart = buf + 15;
				int ifLen = 0;
				for (int i = 0; i < IFNAMSIZ-1; i++ )
				{
					if (!isspace(nwIfaceNameStart[i]))
					{
						nwInterface[i] = nwIfaceNameStart[i];
					}
					else
					{
						nwInterface[i] = '\0';
						break;
					}
				}
				nwInterface[IFNAMSIZ-1] = '\0';
				break;
			}
		}
		fclose(fp);
	}
	else
	{
		logprintf("%s:%d - failed to open /etc/device.properties\n", __FUNCTION__, __LINE__);
	}
#endif
	logprintf("%s:%d - use nwInterface %s\n", __FUNCTION__, __LINE__, nwInterface);
	int sockFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockFd == -1)
	{
		logprintf("%s:%d - Socket open failed\n", __FUNCTION__, __LINE__);
	}
	else
	{
		struct ifreq ifr;
		strcpy(ifr.ifr_name, nwInterface);
		if (ioctl(sockFd, SIOCGIFHWADDR, &ifr) == -1)
		{
			logprintf("%s:%d - Socket ioctl failed\n", __FUNCTION__, __LINE__);
		}
		else
		{
			char* macAddress = base16_Encode((unsigned char*) ifr.ifr_hwaddr.sa_data, 6);
			strcpy(mac, macAddress);
			free(macAddress);
			logprintf("%s:%d - Mac %s\n", __FUNCTION__, __LINE__, mac);
			ret = true;
		}
		close(sockFd);
	}
	return ret;
}
#endif

/**
 * @brief Get time to defer DRM acquisition
 *
 * @param  maxTimeSeconds Maximum time allowed for deferred license acquisition
 * @return Time in MS to defer DRM acquisition
 */
int MediaTrack::GetDeferTimeMs(long maxTimeSeconds)
{
	int ret = 0;
#ifdef USE_MAC_FOR_RANDOM_GEN
	static char randString[RAND_STRING_LEN+1];
	static bool estbMacAvalable = getEstbMac(randString);
	if (estbMacAvalable)
	{
		traceprintf ("%s:%d - estbMac %s\n", __FUNCTION__, __LINE__, randString);
		int randFD = open("/dev/urandom", O_RDONLY);
		if (randFD < 0)
		{
			logprintf("%s:%d - ERROR - opening /dev/urandom  failed\n", __FUNCTION__, __LINE__);
		}
		else
		{
			char* uRandString = &randString[MAC_STRING_LEN];
			int uRandStringLen = 0;
			unsigned char temp;
			for (int i = 0; i < URAND_STRING_LEN; i++)
			{
				ssize_t bytes = read(randFD, &temp, 1);
				if (bytes < 0)
				{
					logprintf("%s:%d - ERROR - reading /dev/urandom  failed\n", __FUNCTION__, __LINE__);
					break;
				}
				sprintf(uRandString + i * 2, "%02x", temp);
			}
			close(randFD);
			randString[RAND_STRING_LEN] = '\0';
			logprintf("%s:%d - randString %s\n", __FUNCTION__, __LINE__, randString);
			unsigned char hash[SHA_DIGEST_LENGTH];
			SHA1((unsigned char*) randString, RAND_STRING_LEN, hash);
			int divisor = maxTimeSeconds - DEFER_DRM_LIC_OFFSET_FROM_START - DEFER_DRM_LIC_OFFSET_TO_UPPER_BOUND;

			int mod = 0;
			for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
			{
				traceprintf ("mod %d hash[%d] %x\n", mod, i, hash[i]);
				mod = (mod * 10 + hash[i]) % divisor;
			}
			traceprintf ("%s:%d - divisor %d mod %d \n", __FUNCTION__, __LINE__, divisor, (int) mod);
			ret = (mod + DEFER_DRM_LIC_OFFSET_FROM_START) * 1000;
		}
	}
	else
	{
		logprintf("%s:%d - ERROR - estbMac not available\n", __FUNCTION__, __LINE__);
		ret = (DEFER_DRM_LIC_OFFSET_FROM_START + rand()%(maxTimeSeconds - DEFER_DRM_LIC_OFFSET_FROM_START - DEFER_DRM_LIC_OFFSET_TO_UPPER_BOUND))*1000;
	}
#else
	ret = (DEFER_DRM_LIC_OFFSET_FROM_START + rand()%(maxTimeSeconds - DEFER_DRM_LIC_OFFSET_FROM_START - DEFER_DRM_LIC_OFFSET_TO_UPPER_BOUND))*1000;
#endif
	logprintf("%s:%d - Added time for deferred license acquisition  %d \n", __FUNCTION__, __LINE__, (int)ret);
	return ret;
}

/**
 *   @brief Check if current stream is muxed
 *
 *   @return true if current stream is muxed
 */
bool StreamAbstractionAAMP::IsMuxedStream()
{
	bool ret = false;

	if ((!gpGlobalConfig->bAudioOnlyPlayback) && (AAMP_NORMAL_PLAY_RATE == aamp->rate))
	{
		MediaTrack *audio = GetMediaTrack(eTRACK_AUDIO);
		MediaTrack *video = GetMediaTrack(eTRACK_VIDEO);
		if (!audio || !video || !audio->enabled || !video->enabled)
		{
			ret = true;
		}
	}
	return ret;
}
