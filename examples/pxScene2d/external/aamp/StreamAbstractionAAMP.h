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
 * @file StreamAbstractionAAMP.h
 * @brief Base classes of HLS/MPD collectors. Implements common caching/injection logic.
 */

#ifndef STREAMABSTRACTIONAAMP_H
#define STREAMABSTRACTIONAAMP_H

#include "priv_aamp.h"
#include <map>
#include <iterator>
#include <vector>

#include <ABRManager.h>
#include <glib.h>


/**
 * @brief Media Track Types
 */
typedef enum
{
	eTRACK_VIDEO,   /**< Video track */
	eTRACK_AUDIO    /**< Audio track */
} TrackType;

/**
 * @brief Structure of cached fragment data
 *        Holds information about a cached fragment
 */
struct CachedFragment
{
	GrowableBuffer fragment;    /**< Buffer to keep fragment content */
	double position;            /**< Position in the playlist */
	double duration;            /**< Fragment duration */
	bool discontinuity;         /**< PTS discontinuity status */
	int profileIndex;           /**< Profile index; Updated internally */
#ifdef AAMP_DEBUG_INJECT
	char uri[MAX_URI_LENGTH];   /**< Fragment url */
#endif
};

/**
 * @brief Playlist Types
 */
typedef enum
{
	ePLAYLISTTYPE_UNDEFINED,    /**< Playlist type undefined */
	ePLAYLISTTYPE_EVENT,        /**< Playlist may grow via appended lines, but otherwise won't change */
	ePLAYLISTTYPE_VOD,          /**< Playlist will never change */
} PlaylistType;

/**
 * @brief Buffer health status
 */
enum BufferHealthStatus
{
	BUFFER_STATUS_GREEN,  /**< Healthy state, where buffering is close to being maxed out */
	BUFFER_STATUS_YELLOW, /**< Danger  state, where buffering is close to being exhausted */
	BUFFER_STATUS_RED     /**< Failed state, where buffers have run dry, and player experiences underrun/stalled video */
};

/**
 * @brief Base Class for Media Track
 */
class MediaTrack
{
public:

	/**
	 * @brief MediaTrack Constructor
	 *
	 * @param[in] type - Media track type
	 * @param[in] aamp - Pointer to PrivateInstanceAAMP
	 * @param[in] name - Media track name
	 */
	MediaTrack(TrackType type, PrivateInstanceAAMP* aamp, const char* name);

	/**
	 * @brief MediaTrack Destructor
	 */
	virtual ~MediaTrack();

	/**
	* @brief MediaTrack Copy Constructor
	*/
	MediaTrack(const MediaTrack&) = delete;

	/**
	* @brief MediaTrack assignment operator overloading
	*/
	MediaTrack& operator=(const MediaTrack&) = delete;

	/**
	 * @brief Start fragment injector loop
	 *
	 * @return void
	 */
	void StartInjectLoop();

	/**
	 * @brief Stop fragment injector loop
	 *
	 * @return void
	 */
	void StopInjectLoop();

	/**
	 * @brief Status of media track
	 *
	 * @return Enabled/Disabled
	 */
	bool Enabled();

	/**
	 * @brief Inject fragment into the gstreamer
	 *
	 * @return Success/Failure
	 */
	bool InjectFragment();

	/**
	 * @brief Get total fragment injected duration
	 *
	 * @return Total duration in seconds
	 */
	double GetTotalInjectedDuration() { return totalInjectedDuration; };

	/**
	 * @brief Run fragment injector loop.
	 *
	 * @return void
	 */
	void RunInjectLoop();

	/**
	 * @brief Update cache after fragment fetch
	 *
	 * @return void
	 */
	void UpdateTSAfterFetch();

	/**
	 * @brief Wait till fragments available
	 *
	 * @param[in] timeoutMs - Timeout in milliseconds. Default - infinite
	 * @return Fragment available or not.
	 */
	bool WaitForFreeFragmentAvailable( int timeoutMs = -1);

	/**
	 * @brief Abort the waiting for cached fragments and free fragment slot
	 *
	 * @param[in] immediate - Forced or lazy abort
	 * @return void
	 */
	void AbortWaitForCachedAndFreeFragment(bool immediate);

	/**
	 * @brief Notifies profile changes to subclasses
	 *
	 * @return void
	 */
	virtual void ABRProfileChanged(void) = 0;

	/**
	 * @brief Get number of fragments dpownloaded
	 *
	 * @return Number of downloaded fragments
	 */
	int GetTotalFragmentsFetched(){ return totalFragmentsDownloaded; }

	/**
	 * @brief Get buffer to store the downloaded fragment content
	 *
	 * @param[in] initialize - Buffer to to initialized or not
	 * @return Fragment cache buffer
	 */
	CachedFragment* GetFetchBuffer(bool initialize);

	/**
	 * @brief Set current bandwidth
	 *
	 * @param[in] bandwidthBps - Bandwidth in bps
	 * @return void
	 */
	void SetCurrentBandWidth(int bandwidthBps);

	/**
	 * @brief Get current bandwidth in bps
	 *
	 * @return Bandwidth in bps
	 */
	int GetCurrentBandWidth();

	/**
	 * @brief Get total duration of fetched fragments
	 *
	 * @return Total duration in seconds
	 */
	double GetTotalFetchedDuration() { return totalFetchedDuration; };

	/**
	 * @brief Check if discontinuity is being processed
	 *
	 * @return true if discontinuity is being processed
	 */
	bool IsDiscontinuityProcessed() { return discontinuityProcessed; }

	bool isFragmentInjectorThreadStarted( ) {  return fragmentInjectorThreadStarted;}
	void MonitorBufferHealth();

	void ScheduleBufferHealthMonitor();

	/**
	 * @brief Get buffer health status
	 *
	 * @return current buffer health status
	 */
	BufferHealthStatus GetBufferHealthStatus() { return bufferStatus; };

	/**
	 * @brief Abort the waiting for cached fragments immediately
	 *
	 * @return void
	 */
	void AbortWaitForCachedFragment();

	/**
	 * @brief Check whether track data injection is aborted
	 *
	 * @return true if injection is aborted, false otherwise
	 */
	bool IsInjectionAborted() { return (abort || abortInject); }
protected:

	/**
	 * @brief Update segment cache and inject buffer to gstreamer
	 *
	 * @return void
	 */
	void UpdateTSAfterInject();

	/**
	 * @brief Wait till cached fragment available
	 *
	 * @return TRUE if fragment available, FALSE if aborted/fragment not available.
	 */
	bool WaitForCachedFragmentAvailable();


	/**
	 * @brief Get the context of media track. To be implemented by subclasses
	 *
	 * @return Pointer to StreamAbstractionAAMP object
	 */
	virtual class StreamAbstractionAAMP* GetContext() = 0;

	/**
	 * @brief To be implemented by derived classes to receive cached fragment.
	 *
	 * @param[in] cachedFragment - contains fragment to be processed and injected
	 * @param[out] fragmentDiscarded - true if fragment is discarded.
	 * @return void
	 */
	virtual void InjectFragmentInternal(CachedFragment* cachedFragment, bool &fragmentDiscarded) = 0;


	static int GetDeferTimeMs(long maxTimeSeconds);


	/**
	 * @brief To be implemented by derived classes if discontinuity on trick-play is to be notified.
	 *
	 */
	virtual void SignalTrickModeDiscontinuity(){};

private:
	static const char* GetBufferHealthStatusString(BufferHealthStatus status);

public:
	bool eosReached;                    /**< set to true when a vod asset has been played to completion */
	bool enabled;                       /**< set to true if track is enabled */
	int numberOfFragmentsCached;        /**< Number of fragments cached in this track*/
	const char* name;                   /**< Track name used for debugging*/
	double fragmentDurationSeconds;     /**< duration in seconds for current fragment-of-interest */
	int segDLFailCount;                 /**< Segment download fail count*/
	int segDrmDecryptFailCount;         /**< Segment decryption failure count*/
	int mSegInjectFailCount;            /**< Segment Inject/Decode fail count */
	TrackType type;                     /**< Media type of the track*/
protected:
	PrivateInstanceAAMP* aamp;          /**< Pointer to the PrivateInstanceAAMP*/
	CachedFragment *cachedFragment;     /**< storage for currently-downloaded fragment */
	bool abort;                         /**< Abort all operations if flag is set*/
	pthread_mutex_t mutex;              /**< protection of track variables accessed from multiple threads */
	bool ptsError;                      /**< flag to indicate if last injected fragment has ptsError */
	bool abortInject;                   /**< Abort inject operations if flag is set*/
private:
	pthread_cond_t fragmentFetched;     /**< Signaled after a fragment is fetched*/
	pthread_cond_t fragmentInjected;    /**< Signaled after a fragment is injected*/
	pthread_t fragmentInjectorThreadID; /**< Fragment injector thread id*/
	pthread_t bufferMonitorThreadID;    /**< Buffer Monitor thread id */
	int totalFragmentsDownloaded;       /**< Total fragments downloaded since start by track*/
	bool fragmentInjectorThreadStarted; /**< Fragment injector's thread started or not*/
	bool bufferMonitorThreadStarted;    /**< Buffer Monitor thread started or not */
	double totalInjectedDuration;       /**< Total fragment injected duration*/
	int cacheDurationSeconds;           /**< Total fragment cache duration*/
	bool notifiedCachingComplete;       /**< Fragment caching completed or not*/
	int fragmentIdxToInject;            /**< Write position */
	int fragmentIdxToFetch;             /**< Read position */
	int bandwidthBytesPerSecond;        /**< Bandwidth of last selected profile*/
	double totalFetchedDuration;        /**< Total fragment fetched duration*/
	bool discontinuityProcessed;

	BufferHealthStatus bufferStatus;     /**< Buffer status of the track*/
	BufferHealthStatus prevBufferStatus; /**< Previous buffer status of the track*/
};


/**
 * @brief Structure holding the resolution of stream
 */
struct StreamResolution
{
	int width;      /**< Width in pixels*/
	int height;     /**< Height in pixels*/
};

/**
 * @brief Structure holding the information of a stream.
 */
struct StreamInfo
{
	bool isIframeTrack;             /**< indicates if the stream is iframe stream*/
	long bandwidthBitsPerSecond;    /**< Bandwidth of the stream bps*/
	StreamResolution resolution;    /**< Resolution of the stream*/
};

/**
 * @brief Base class for the client side DAI object
 */
class CDAIObject
{
public:
	/**
	 * @brief CDAIObject constructor.
	 */
	CDAIObject(PrivateInstanceAAMP* aamp)
	{

	}

	/**
	 * @brief CDAIObject destructor.
	 */
	virtual ~CDAIObject()
	{

	}

	/**
	 *   @brief Setting the alternate contents' (Ads/blackouts) URL
	 *
	 *   @param[in] adBreakId - Adbreak's unique identifier.
	 *   @param[in] adId - Individual Ad's id
	 *   @param[in] url - Ad URL
	 *   @param[in] startMS - Ad start time in milliseconds
	 */
	virtual void SetAlternateContents(const std::string &adBreakId, const std::string &adId, const std::string &url, uint64_t startMS=0){}
};

/**
 * @brief StreamAbstraction class of AAMP
 */
class StreamAbstractionAAMP
{
public:
	/**
	 * @brief StreamAbstractionAAMP constructor.
	 */
	StreamAbstractionAAMP(PrivateInstanceAAMP* aamp);

	/**
	 * @brief StreamAbstractionAAMP destructor.
	 */
	virtual ~StreamAbstractionAAMP();

	/**
	* @brief StreamAbstractionAAMP Copy Constructor
	*/
	StreamAbstractionAAMP(const StreamAbstractionAAMP&) = delete;

	/**
	* @brief StreamAbstractionAAMP assignment operator overloading
	*/
	StreamAbstractionAAMP& operator=(const StreamAbstractionAAMP&) = delete;

	/**
	 * @brief  Dump profiles for debugging.
	 *         To be implemented by sub classes
	 *
	 * @return void
	 */
	virtual void DumpProfiles(void) = 0;

	/**
	 *   @brief  Initialize a newly created object.
	 *           To be implemented by sub classes
	 *
	 *   @param[in]  tuneType - to set type of playback.
	 *   @return true on success, false failure
	 */
	virtual AAMPStatusType Init(TuneType tuneType) = 0;

	/**
	 *   @brief  Start streaming.
	 *
 	 *   @return void
	 */
	virtual void Start() = 0;

	/**
	*   @brief  Stops streaming.
	*
	*   @param[in]  clearChannelData - clear channel /drm data on stop.
	*   @return void
	*/
	virtual void Stop(bool clearChannelData) = 0;

	/**
	 *   @brief Get output format of stream.
	 *
	 *   @param[out]  primaryOutputFormat - format of primary track
	 *   @param[out]  audioOutputFormat - format of audio track
	 *   @return void
	 */
	virtual void GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat) = 0;

	/**
	 *   @brief Get current stream position.
	 *
	 *   @return current position of stream.
	 */
	virtual double GetStreamPosition() = 0;

	/**
	 *   @brief  Get PTS of first sample.
	 *
	 *   @return PTS of first sample
	 */
	virtual double GetFirstPTS() = 0;

	/**
	 *   @brief Return MediaTrack of requested type
	 *
	 *   @param[in]  type - track type
	 *   @return MediaTrack pointer.
	 */
	virtual MediaTrack* GetMediaTrack(TrackType type) = 0;

	/**
	 *   @brief Waits track injection until caught up with video track.
	 *          Used internally by injection logic
	 *
	 *   @param None
	 *   @return void
	 */
	void WaitForVideoTrackCatchup();

	/**
	 *   @brief Unblock track if caught up with video or downloads are stopped
	 *
	 *   @return void
	 */
	void ReassessAndResumeAudioTrack(bool abort);

	/**
	 *   @brief When TSB is involved, use this to set bandwidth to be reported.
	 *
	 *   @param[in]  tsbBandwidth - Bandwidth of the track.
	 *   @return void
	 */
	void SetTsbBandwidth(long tsbBandwidth){ mTsbBandwidth = tsbBandwidth;}

	/**
	 *   @brief When TSB is involved, use this to get bandwidth to be reported.
	 *
	 *   @return Bandwidth of the track.
	 */
	long GetTsbBandwidth() { return mTsbBandwidth ;}

	/**
	 *   @brief Set elementary stream type change status for reconfigure the pipeline.
	 *
	 *   @param[in]  None
	 *   @return void
	 */
	void SetESChangeStatus(void){ mESChangeStatus = true;}

	/**
	 *   @brief Reset elementary stream type change status once the pipeline reconfigured.
	 *
	 *   @param[in]  None
	 *   @return void
	 */
	void ResetESChangeStatus(void){ mESChangeStatus = false;}

	/**
	 *   @brief Get elementary stream type change status for reconfigure the pipeline..
	 *
	 *   @param[in]  None
	 *   @retval mESChangeStatus flag value ( true or false )
	 */
	bool GetESChangeStatus(void){ return mESChangeStatus;}

	PrivateInstanceAAMP* aamp;  /**< Pointer to PrivateInstanceAAMP object associated with stream*/

	/**
	 * @brief Rampdown profile
	 *
	 * @param[in] http_error
	 * @return True, if ramp down successful. Else false
	 */
	bool RampDownProfile(long http_error);

	/**
	 *   @brief Check for ramdown profile.
	 *
	 *   @param http_error
	 *   @return true if rampdown needed in the case of fragment not available in higher profile.
	 */
	bool CheckForRampDownProfile(long http_error);

	/**
	 *   @brief Checks and update profile based on bandwidth.
	 *
	 *   @param None
	 *   @return void
	 */
	void CheckForProfileChange(void);

	/**
	 *   @brief Get iframe track index.
	 *   This shall be called only after UpdateIframeTracks() is done
	 *
	 *   @param None
	 *   @return iframe track index.
	 */
	int GetIframeTrack();

	/**
	 *   @brief Update iframe tracks.
	 *   Subclasses shall invoke this after StreamInfo is populated .
	 *
	 *   @param None
	 *   @return void
	 */
	void UpdateIframeTracks();

	/**
	 *   @brief Get the last video fragment parsed time.
	 *
	 *   @param None
	 *   @return Last video fragment parsed time.
	 */
	double LastVideoFragParsedTimeMS(void);

	/**
	 *   @brief Get the desired profile to start fetching.
	 *
	 *   @param getMidProfile
	 *   @return profile index to be used for the track.
	 */
	int GetDesiredProfile(bool getMidProfile);

	/**
	 *   @brief Notify bitrate updates to application.
	 *   Used internally by injection logic
	 *
	 *   @param[in]  profileIndex - profile index of last injected fragment.
	 *   @return void
	 */
	void NotifyBitRateUpdate(int profileIndex);

	/**
	 *   @brief Fragment Buffering is required before playing.
	 *
	 *   @return true if buffering is required.
	 */
	bool IsFragmentBufferingRequired() { return false; }

	/**
	 *   @brief Whether we are playing at live point or not.
	 *
	 *   @return true if we are at live point.
	 */
	bool IsStreamerAtLivePoint() { return mIsAtLivePoint; }

	/**
	 *   @brief Informs streamer that playback was paused.
	 *
	 *   @param[in] paused - true, if playback was paused
	 *   @return void
	 */
	virtual void NotifyPlaybackPaused(bool paused);

	/**
	 *   @brief Check if player caches are running dry.
	 *
	 *   @return true if player caches are dry, false otherwise.
	 */
	bool CheckIfPlayerRunningDry(void);

	/**
	 *   @brief Check if playback has stalled and update related flags.
	 *
	 *   @param[in] fragmentParsed - true if next fragment was parsed, otherwise false
	 */
	void CheckForPlaybackStall(bool fragmentParsed);

	void NotifyFirstFragmentInjected(void);

	double GetElapsedTime();

	bool trickplayMode;                     /**< trick play flag to be updated by subclasses*/
	int currentProfileIndex;                /**< current profile index of the track*/
	int profileIdxForBandwidthNotification; /**< internal - profile index for bandwidth change notification*/
	bool hasDrm;                            /**< denotes if the current asset is DRM protected*/

	bool mIsAtLivePoint;                    /**< flag that denotes if playback is at live point*/

	bool mIsPlaybackStalled;                /**< flag that denotes if playback was stalled or not*/
	bool mIsFirstBuffer;                    /** <flag that denotes if the first buffer was processed or not*/
	bool mNetworkDownDetected;              /**< Network down status indicator */
	bool mCheckForRampdown;			/**< flag to indicate if rampdown is attempted or not */
	TuneType mTuneType;                     /**< Tune type of current playback, initialize by derived classes on Init()*/


	/**
	 *   @brief Get profile index of highest bandwidth
	 *
	 *   @return Profile index
	 */
	int GetMaxBWProfile() { return mAbrManager.getMaxBandwidthProfile(); } /* Return the Top Profile Index*/

	/**
	 *   @brief Get profile index of given bandwidth.
	 *
	 *   @param[in]  bandwidth - Bandwidth
	 *   @return Profile index
	 */
	virtual int GetBWIndex(long bandwidth) = 0;

	/**
	 *    @brief Get the ABRManager reference.
	 *
	 *    @return The ABRManager reference.
	 */
	ABRManager& GetABRManager() {
		return mAbrManager;
	}

	/**
	 *   @brief Get number of profiles/ representations from subclass.
	 *
	 *   @return number of profiles.
	 */
	int GetProfileCount() {
		return mAbrManager.getProfileCount();
	}

	long GetCurProfIdxBW(){
		return mAbrManager.getBandwidthOfProfile(this->currentProfileIndex);
	}


	/**
	 *   @brief Gets Max bitrate supported
	 *
	 *   @return max bandwidth
	 */
	long GetMaxBitrate(){
		return mAbrManager.getBandwidthOfProfile(mAbrManager.getMaxBandwidthProfile());
	}


	/**
	 *   @brief Get the bitrate of current video profile selected.
	 *
	 *   @return bitrate of current video profile.
	 */
	long GetVideoBitrate(void);

	/**
	 *   @brief Get the bitrate of current audio profile selected.
	 *
	 *   @return bitrate of current audio profile.
	 */
	long GetAudioBitrate(void);

	/**
	 *   @brief Set a preferred bitrate for video.
	 *
	 *   @param[in] preferred bitrate.
	 */
	void SetVideoBitrate(long bitrate);

	/**
	 *   @brief Check if a preferred bitrate is set and change profile accordingly.
	 */
	void CheckUserProfileChangeReq(void);

	/**
	 *   @brief Get available video bitrates.
	 *
	 *   @return available video bitrates.
	 */
	virtual std::vector<long> GetVideoBitrates(void) = 0;

	/**
	 *   @brief Get available audio bitrates.
	 *
	 *   @return available audio bitrates.
	 */
	virtual std::vector<long> GetAudioBitrates(void) = 0;

	/**
	 *   @brief Check if playback stalled in fragment collector side.
	 *
	 *   @return true if stalled, false otherwise.
	 */
	bool IsStreamerStalled(void) { return mIsPlaybackStalled; }

	/**
	 *   @brief Stop injection of fragments.
	 */
	virtual void StopInjection(void) = 0;

	/**
	 *   @brief Start injection of fragments.
	 */
	virtual void StartInjection(void) = 0;

	/**
	 *   @brief Check if current stream is muxed
	 *
	 *   @return true if current stream is muxed
	 */
	bool IsMuxedStream();

	/**
	 *   @brief Set Client Side DAI object instance
	 *
	 *   @param[in] cdaiObj - Pointer to Client Side DAI object.
	 */
	virtual void SetCDAIObject(CDAIObject *cdaiObj) {};
protected:
	/**
	 *   @brief Get stream information of a profile from subclass.
	 *
	 *   @param[in]  idx - profile index.
	 *   @return stream information corresponding to index.
	 */
	virtual StreamInfo* GetStreamInfo(int idx) = 0;

private:

	/**
	 * @brief Get desired profile based on cache
	 *
	 * @return Profile index
	 */
	int GetDesiredProfileBasedOnCache(void);

	/**
	 * @brief Update profile based on fragments downloaded.
	 *
	 * @return void
	 */
	void UpdateProfileBasedOnFragmentDownloaded(void);

	/**
	 * @brief Update profile based on fragment cache.
	 *
	 * @return bool
	 */
	bool UpdateProfileBasedOnFragmentCache(void);

	pthread_mutex_t mLock;              /**< lock for A/V track catchup logic*/
	pthread_cond_t mCond;               /**< condition for A/V track catchup logic*/

	// abr variables
	long mCurrentBandwidth;             /**< stores current bandwidth*/
	int mLastVideoFragCheckedforABR;    /**< Last video fragment for which ABR is checked*/
	long mTsbBandwidth;                 /**< stores bandwidth when TSB is involved*/
	long mNwConsistencyBypass;          /**< Network consistency bypass**/
	bool mESChangeStatus;               /**< flag value which is used to call pipeline configuration if the audio type changed in mid stream */
	double mLastVideoFragParsedTimeMS;  /**< timestamp when last video fragment was parsed */

	bool mIsPaused;                     /**< paused state or not */
	long long mTotalPausedDurationMS;   /**< Total duration for which stream is paused */
	long long mStartTimeStamp;          /**< stores timestamp at which injection starts */
	long long mLastPausedTimeStamp;     /**< stores timestamp of last pause operation */
protected:
	ABRManager mAbrManager;             /**< Pointer to abr manager*/
};

#endif // STREAMABSTRACTIONAAMP_H
