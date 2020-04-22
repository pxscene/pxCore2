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
 * @file fragmentcollector_mpd.cpp
 * @brief Fragment collector implementation of MPEG DASH
 */
#include "iso639map.h"
#include "fragmentcollector_mpd.h"
#include "priv_aamp.h"
#include "AampDRMSessionManager.h"
#include "admanager_mpd.h"

#include <stdlib.h>
#include <string.h>
#include "_base64.h"
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <fstream>
#include <set>
#include <iomanip>
#include <ctime>
#include <inttypes.h>
#include <libxml/xmlreader.h>
#include <math.h>
#include <cmath> // For double abs(double)
#include <algorithm>
#include <cctype>
#include "AampCacheHandler.h"
//#define DEBUG_TIMELINE
//#define AAMP_HARVEST_SUPPORT_ENABLED
//#define AAMP_DISABLE_INJECT
//#define HARVEST_MPD

/**
 * @addtogroup AAMP_COMMON_TYPES
 * @{
 */
#define SEGMENT_COUNT_FOR_ABR_CHECK 5
#define PLAYREADY_SYSTEM_ID "9a04f079-9840-4286-ab92-e65be0885f95"
#define WIDEVINE_SYSTEM_ID "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define CLEARKEY_SYSTEM_ID "1077efec-c0b2-4d02-ace3-3c1e52e2fb4b"
#define DEFAULT_INTERVAL_BETWEEN_MPD_UPDATES_MS 3000
#define TIMELINE_START_RESET_DIFF 4000000000
#define MAX_DELAY_BETWEEN_MPD_UPDATE_MS (6000)
#define MIN_DELAY_BETWEEN_MPD_UPDATE_MS (500) // 500mSec
#define MIN_TSB_BUFFER_DEPTH 6 //6 seconds from 4.3.3.2.2 in https://dashif.org/docs/DASH-IF-IOP-v4.2-clean.htm

//Comcast DRM Agnostic CENC for Content Metadata
#define COMCAST_DRM_INFO_ID "afbcb50e-bf74-3d13-be8f-13930c783962"

/**
 * Macros for extended audio codec check as per ETSI-TS-103-420-V1.2.1
 */
#define SUPPLEMENTAL_PROPERTY_TAG "SupplementalProperty"
#define SCHEME_ID_URI_EC3_EXT_CODEC "tag:dolby.com,2018:dash:EC3_ExtensionType:2018"
#define EC3_EXT_VALUE_AUDIO_ATMOS "JOC"
/**
 * @struct FragmentDescriptor
 * @brief Stores information of dash fragment
 */
struct FragmentDescriptor
{
	std::string manifestUrl;
	const std::vector<IBaseUrl *>*baseUrls;
	uint32_t Bandwidth;
	std::string RepresentationID; 
	uint64_t Number;
	double Time;

	FragmentDescriptor() : manifestUrl(""), baseUrls (NULL), Bandwidth(0), Number(0), Time(0), RepresentationID("")
	{
	}
    
	FragmentDescriptor(const FragmentDescriptor& p) : manifestUrl(p.manifestUrl), baseUrls(p.baseUrls), Bandwidth(p.Bandwidth), RepresentationID(p.RepresentationID), Number(p.Number), Time(p.Time)
	{
	}

	FragmentDescriptor& operator=(const FragmentDescriptor &p)
	{
		manifestUrl = p.manifestUrl;
		baseUrls = p.baseUrls;
		RepresentationID.assign(p.RepresentationID);
		Bandwidth = p.Bandwidth;
		Number = p.Number;
		Time = p.Time;
		return *this;
	}
};

/**
 * @struct PeriodInfo
 * @brief Stores details about available periods in mpd
 */

struct PeriodInfo {
	std::string periodId;
	uint64_t startTime;
	double duration;

	PeriodInfo() : periodId(""), startTime(0), duration(0.0)
	{
	}
};


static const char *mMediaTypeName[] = { "video", "audio", "text" };

#ifdef AAMP_HARVEST_SUPPORT_ENABLED
#ifdef USE_PLAYERSINKBIN
#define HARVEST_BASE_PATH "/media/tsb/aamp-harvest/" // SD card friendly path
#else
#define HARVEST_BASE_PATH "aamp-harvest/"
#endif
static void GetFilePath(std::string& filePath, const FragmentDescriptor *fragmentDescriptor, std::string media);
static void WriteFile(std::string fileName, const char* data, int len);
#endif // AAMP_HARVEST_SUPPORT_ENABLED


/**
 * @class MediaStreamContext
 * @brief MPD media track
 */
class MediaStreamContext : public MediaTrack
{
public:

	/**
	 * @brief MediaStreamContext Constructor
	 * @param type Type of track
	 * @param context  MPD collector context
	 * @param aamp Pointer to associated aamp instance
	 * @param name Name of the track
	 */
	MediaStreamContext(TrackType type, StreamAbstractionAAMP_MPD* context, PrivateInstanceAAMP* aamp, const char* name) :
			MediaTrack(type, aamp, name),
			mediaType((MediaType)type), adaptationSet(NULL), representation(NULL),
			fragmentIndex(0), timeLineIndex(0), fragmentRepeatCount(0), fragmentOffset(0),
			eos(false), fragmentTime(0), periodStartOffset(0), index_ptr(NULL), index_len(0),
			lastSegmentTime(0), lastSegmentNumber(0), adaptationSetIdx(0), representationIndex(0), profileChanged(true),
			adaptationSetId(0), fragmentDescriptor(), mContext(context), initialization(""), mDownloadedFragment(), discontinuity(false), mSkipSegmentOnError(true)
	{
		memset(&mDownloadedFragment, 0, sizeof(GrowableBuffer));
	}

	/**
	 * @brief MediaStreamContext Destructor
	 */
	~MediaStreamContext()
	{
		if(mDownloadedFragment.ptr)
		{
			aamp_Free(&mDownloadedFragment.ptr);
			mDownloadedFragment.ptr = NULL;
		}
	}

	/**
	 * @brief MediaStreamContext Copy Constructor
	 */
	 MediaStreamContext(const MediaStreamContext&) = delete;

	/**
	 * @brief MediaStreamContext Assignment operator overloading
	 */
	 MediaStreamContext& operator=(const MediaStreamContext&) = delete;


	/**
	 * @brief Get the context of media track. To be implemented by subclasses
	 * @retval Context of track.
	 */
	StreamAbstractionAAMP* GetContext()
	{
		return mContext;
	}

	/**
	 * @brief Receives cached fragment and injects to sink.
	 *
	 * @param[in] cachedFragment - contains fragment to be processed and injected
	 * @param[out] fragmentDiscarded - true if fragment is discarded.
	 */
	void InjectFragmentInternal(CachedFragment* cachedFragment, bool &fragmentDiscarded)
	{
#ifndef AAMP_DISABLE_INJECT
		aamp->SendStream((MediaType)type, &cachedFragment->fragment,
					cachedFragment->position, cachedFragment->position, cachedFragment->duration);
#endif
		fragmentDiscarded = false;
	} // InjectFragmentInternal

	/**
	 * @brief Fetch and cache a fragment
	 * @param fragmentUrl url of fragment
	 * @param curlInstance curl instance to be used to fetch
	 * @param position position of fragment in seconds
	 * @param duration duration of fragment in seconds
	 * @param range byte range
	 * @param initSegment true if fragment is init fragment
	 * @param discontinuity true if fragment is discontinuous
	 * @retval true on success
	 */
	bool CacheFragment(std::string fragmentUrl, unsigned int curlInstance, double position, double duration, const char *range = NULL, bool initSegment = false, bool discontinuity = false
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		, std::string media = 0
#endif
		, bool playingAd = false
	)
	{
		bool ret = false;

		fragmentDurationSeconds = duration;
		ProfilerBucketType bucketType = aamp->GetProfilerBucketForMedia(mediaType, initSegment);
		CachedFragment* cachedFragment = GetFetchBuffer(true);
		long http_code = 0;
		long bitrate = 0;
		MediaType actualType = (MediaType)(initSegment?(eMEDIATYPE_INIT_VIDEO+mediaType):mediaType); //Need to revisit the logic

		if(!initSegment && mDownloadedFragment.ptr)
		{
			ret = true;
			cachedFragment->fragment.ptr = mDownloadedFragment.ptr;
			cachedFragment->fragment.len = mDownloadedFragment.len;
			cachedFragment->fragment.avail = mDownloadedFragment.avail;
			memset(&mDownloadedFragment, 0, sizeof(GrowableBuffer));
		}
		else
		{
			std::string effectiveUrl;
			int iFogError = -1;
			int iCurrentRate = aamp->rate; //  Store it as back up, As sometimes by the time File is downloaded, rate might have changed due to user initiated Trick-Play
			ret = aamp->LoadFragment(bucketType, fragmentUrl,effectiveUrl, &cachedFragment->fragment, curlInstance,
						range, actualType, &http_code, &bitrate, &iFogError, fragmentDurationSeconds );

			if (iCurrentRate != AAMP_NORMAL_PLAY_RATE)
			{
				actualType = eMEDIATYPE_IFRAME;
				if(actualType == eMEDIATYPE_INIT_VIDEO)
				{
					actualType = eMEDIATYPE_INIT_IFRAME;
				}
			}

			//update videoend info
			aamp->UpdateVideoEndMetrics( actualType,
									bitrate? bitrate : fragmentDescriptor.Bandwidth,
									(iFogError > 0 ? iFogError : http_code),effectiveUrl,duration);
		}

		mContext->mCheckForRampdown = false;
		if(bitrate > 0 && bitrate != fragmentDescriptor.Bandwidth)
		{
			AAMPLOG_INFO("%s:%d Bitrate changed from %ld to %ld", __FUNCTION__, __LINE__, fragmentDescriptor.Bandwidth, bitrate);
			fragmentDescriptor.Bandwidth = bitrate;
			mDownloadedFragment.ptr = cachedFragment->fragment.ptr;
			mDownloadedFragment.avail = cachedFragment->fragment.avail;
			mDownloadedFragment.len = cachedFragment->fragment.len;
			memset(&cachedFragment->fragment, 0, sizeof(GrowableBuffer));
			ret = false;
		}
		else if (!ret)
		{
			aamp_Free(&cachedFragment->fragment.ptr);
			if( aamp->DownloadsAreEnabled())
			{
				logprintf("%s:%d LoadFragment failed", __FUNCTION__, __LINE__);

				if (initSegment)
				{
					logprintf("%s:%d Init fragment fetch failed. fragmentUrl %s", __FUNCTION__, __LINE__, fragmentUrl.c_str());
				}

				if (mSkipSegmentOnError)
				{
					// Skip segment on error, and increse fail count
					segDLFailCount += 1;
				}
				else
				{
					// Rampdown already attempted on same segment
					// Reset flag for next fetch
					mSkipSegmentOnError = true;
				}
				if (MAX_SEG_DOWNLOAD_FAIL_COUNT <= segDLFailCount)
				{
					if(!playingAd)	//If playingAd, we are invalidating the current Ad in onAdEvent().
					{
						if (!initSegment)
						{
							AAMPLOG_ERR("%s:%d Not able to download fragments; reached failure threshold sending tune failed event",__FUNCTION__, __LINE__);
							aamp->SendDownloadErrorEvent(AAMP_TUNE_FRAGMENT_DOWNLOAD_FAILURE, http_code);
						}
						else
						{
							// When rampdown limit is not specified, init segment will be ramped down, this wil
							AAMPLOG_ERR("%s:%d Not able to download init fragments; reached failure threshold sending tune failed event",__FUNCTION__, __LINE__);
							aamp->SendDownloadErrorEvent(AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, http_code);
						}
					}
				}
				// DELIA-32287 - Profile RampDown check and rampdown is needed only for Video . If audio fragment download fails
				// should continue with next fragment,no retry needed .
				else if ((eTRACK_VIDEO == type) && !(mContext->CheckForRampDownLimitReached()))
				{
					// Attempt rampdown
					if (mContext->CheckForRampDownProfile(http_code))
					{
						mContext->mCheckForRampdown = true;
						if (!initSegment)
						{
							// Rampdown attempt success, download same segment from lower profile.
							mSkipSegmentOnError = false;
						}
						AAMPLOG_WARN( "PrivateStreamAbstractionMPD::%s:%d > Error while fetching fragment:%s, failedCount:%d. decrementing profile",
								__FUNCTION__, __LINE__, fragmentUrl.c_str(), segDLFailCount);
					}
					else
					{
						if(!playingAd && initSegment)
						{
							// Already at lowest profile, send error event for init fragment.
							AAMPLOG_ERR("%s:%d Not able to download init fragments; reached failure threshold sending tune failed event",__FUNCTION__, __LINE__);
							aamp->SendDownloadErrorEvent(AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, http_code);
						}
						else
						{
							AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s:%d Already at the lowest profile, skipping segment", __FUNCTION__,__LINE__);
							mContext->mRampDownCount = 0;
						}
					}
				}
				else if (AAMP_IS_LOG_WORTHY_ERROR(http_code))
				{
					AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s:%d > Error on fetching %s fragment. failedCount:%d",
							__FUNCTION__, __LINE__, name, segDLFailCount);
					// For init fragment, rampdown limit is reached. Send error event.
					if(!playingAd && initSegment)
					{
						aamp->SendDownloadErrorEvent(AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, http_code);
					}
				}
			}
		}
		else
		{
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
			if (aamp->HarvestFragments())
			{
				std::string fileName;
				fileName.assign(fragmentUrl);
				GetFilePath(fileName, &fragmentDescriptor, media);
				logprintf("%s:%d filePath %s", __FUNCTION__, __LINE__, fileName.c_str());
				WriteFile(fileName, cachedFragment->fragment.ptr, cachedFragment->fragment.len);
			}
#endif
			cachedFragment->position = position;
			cachedFragment->duration = duration;
			cachedFragment->discontinuity = discontinuity;
#ifdef AAMP_DEBUG_INJECT
			if (discontinuity)
			{
				logprintf("%s:%d Discontinuous fragment", __FUNCTION__, __LINE__);
			}
			if ((1 << type) & AAMP_DEBUG_INJECT)
			{
				cachedFragment->uri.assign(fragmentUrl);
			}
#endif
			segDLFailCount = 0;
			if ((eTRACK_VIDEO == type) && (!initSegment))
			{
				// reset count on video fragment success
				mContext->mRampDownCount = 0;
			}
			UpdateTSAfterFetch();
			ret = true;
		}
		return ret;
	}


	/**
	 * @brief Listener to ABR profile change
	 */
	void ABRProfileChanged(void)
	{
		if (representationIndex != mContext->currentProfileIndex)
		{
			IRepresentation *pNewRepresentation = adaptationSet->GetRepresentation().at(mContext->currentProfileIndex);
			logprintf("PrivateStreamAbstractionMPD::%s:%d - ABR %dx%d[%d] -> %dx%d[%d]", __FUNCTION__, __LINE__,
					representation->GetWidth(), representation->GetHeight(), representation->GetBandwidth(),
					pNewRepresentation->GetWidth(), pNewRepresentation->GetHeight(), pNewRepresentation->GetBandwidth());
			representationIndex = mContext->currentProfileIndex;
			representation = adaptationSet->GetRepresentation().at(mContext->currentProfileIndex);
			const std::vector<IBaseUrl *>*baseUrls = &representation->GetBaseURLs();
			if (baseUrls->size() != 0)
			{
				fragmentDescriptor.baseUrls = &representation->GetBaseURLs();
			}
			fragmentDescriptor.Bandwidth = representation->GetBandwidth();
			fragmentDescriptor.RepresentationID.assign(representation->GetId());
			profileChanged = true;
		}
		else
		{
			traceprintf("PrivateStreamAbstractionMPD::%s:%d - Not switching ABR %dx%d[%d] ", __FUNCTION__, __LINE__,
					representation->GetWidth(), representation->GetHeight(), representation->GetBandwidth());
		}

	}

	double GetBufferedDuration()
	{
		return (fragmentTime - (aamp->GetPositionMs() / 1000));
	}


	/**
	 * @brief Notify discontinuity during trick-mode as PTS re-stamping is done in sink
	 */
	void SignalTrickModeDiscontinuity()
	{
		aamp->SignalTrickModeDiscontinuity();
	}

	/**
	 * @brief Returns if the end of track reached.
	 */
	bool IsAtEndOfTrack()
	{
		return eos;
	}

	MediaType mediaType;
	struct FragmentDescriptor fragmentDescriptor;
	IAdaptationSet *adaptationSet;
	IRepresentation *representation;
	int fragmentIndex;
	int timeLineIndex;
	int fragmentRepeatCount;
	int fragmentOffset;
	bool eos;
	bool profileChanged;
	bool discontinuity;
	GrowableBuffer mDownloadedFragment;

	double fragmentTime;
	double periodStartOffset;
	char *index_ptr;
	size_t index_len;
	uint64_t lastSegmentTime;
	uint64_t lastSegmentNumber;
	int adaptationSetIdx;
	int representationIndex;
	StreamAbstractionAAMP_MPD* mContext;
	std::string initialization;
	uint32_t adaptationSetId;
	bool mSkipSegmentOnError;
};

/**
 * @struct HeaderFetchParams
 * @brief Holds information regarding initialization fragment
 */
struct HeaderFetchParams
{
	HeaderFetchParams() : context(NULL), pMediaStreamContext(NULL), initialization(""), fragmentduration(0),
		isinitialization(false), discontinuity(false)
	{
	}
	HeaderFetchParams(const HeaderFetchParams&) = delete;
	HeaderFetchParams& operator=(const HeaderFetchParams&) = delete;
	class PrivateStreamAbstractionMPD *context;
	struct MediaStreamContext *pMediaStreamContext;
	string initialization;
	double fragmentduration;
	bool isinitialization;
	bool discontinuity;
};

/**
 * @struct FragmentDownloadParams
 * @brief Holds data of fragment to be downloaded
 */
struct FragmentDownloadParams
{
	class PrivateStreamAbstractionMPD *context;
	struct MediaStreamContext *pMediaStreamContext;
	bool playingLastPeriod;
	long long lastPlaylistUpdateMS;
};

static bool IsIframeTrack(IAdaptationSet *adaptationSet);

/**
 * @class PrivateStreamAbstractionMPD
 * @brief Private implementation of MPD fragment collector
 */
class PrivateStreamAbstractionMPD
{
public:
	PrivateStreamAbstractionMPD( StreamAbstractionAAMP_MPD* context, PrivateInstanceAAMP *aamp,double seekpos, float rate);
	~PrivateStreamAbstractionMPD();
	PrivateStreamAbstractionMPD(const PrivateStreamAbstractionMPD&) = delete;
	PrivateStreamAbstractionMPD& operator=(const PrivateStreamAbstractionMPD&) = delete;
	void SetEndPos(double endPosition);
	void Start();
	void Stop();
	AAMPStatusType Init(TuneType tuneType);
	void GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat);

	/**
	 * @brief Get current stream position.
	 *
	 * @retval current position of stream.
	 */
	double GetStreamPosition() { return seekPosition; }

	void FetcherLoop();
	bool PushNextFragment( MediaStreamContext *pMediaStreamContext, unsigned int curlInstance = 0);
	bool FetchFragment(MediaStreamContext *pMediaStreamContext, std::string media, double fragmentDuration, bool isInitializationSegment, unsigned int curlInstance = 0, bool discontinuity = false );
	double GetPeriodEndTime(IMPD *mpd, int periodIndex, uint64_t mpdRefreshTime);
	double GetPeriodStartTime(IMPD *mpd, int periodIndex);
	int GetProfileCount();
	StreamInfo* GetStreamInfo(int idx);
	MediaTrack* GetMediaTrack(TrackType type);
	double GetFirstPTS();
	int64_t GetMinUpdateDuration() { return mMinUpdateDurationMs;}
	PrivateInstanceAAMP *aamp;

	int GetBWIndex(long bitrate);
	std::vector<long> GetVideoBitrates(void);
	std::vector<long> GetAudioBitrates(void);
	void StopInjection();
	void StartInjection();
	void SetCDAIObject(CDAIObject *cdaiObj);
	bool isAdbreakStart(IPeriod *period, uint32_t &duration, uint64_t &startMS, std::string &scte35);
	bool onAdEvent(AdEvent evt);
	bool onAdEvent(AdEvent evt, double &adOffset);
	long GetMaxTSBBandwidth() { return mMaxTSBBandwidth; }
	bool IsTSBUsed() { return mIsFogTSB; }
private:
	AAMPStatusType UpdateMPD(bool init = false);
	void FindTimedMetadata(MPD* mpd, Node* root, bool init = false);
	void ProcessPeriodSupplementalProperty(Node* node, std::string& AdID, uint64_t startMS, uint64_t durationMS, bool isInit);
	void ProcessPeriodAssetIdentifier(Node* node, uint64_t startMS, uint64_t durationMS, std::string& assetID, std::string& providerID,bool isInit);
	bool ProcessEventStream(uint64_t startMS, IPeriod * period);
	void ProcessStreamRestrictionList(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessStreamRestriction(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessStreamRestrictionExt(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessTrickModeRestriction(Node* node, const std::string& AdID, uint64_t startMS);
	void FetchAndInjectInitialization(bool discontinuity = false);
	void StreamSelection(bool newTune = false);
	bool CheckForInitalClearPeriod();
	void PushEncryptedHeaders();
	AAMPStatusType UpdateTrackInfo(bool modifyDefaultBW, bool periodChanged, bool resetTimeLineIndex=false);
	double SkipFragments( MediaStreamContext *pMediaStreamContext, double skipTime, bool updateFirstPTS = false);
	void SkipToEnd( MediaStreamContext *pMediaStreamContext); //Added to support rewind in multiperiod assets
	void ProcessContentProtection(IAdaptationSet * adaptationSet,MediaType mediaType);
	void SeekInPeriod( double seekPositionSeconds);
	double GetCulledSeconds();
	void UpdateLanguageList();
	int GetBestAudioTrackByLanguage(int &desiredRepIdx,AudioType &selectedCodecType);
	int GetPreferredAudioTrackByLanguage();
	std::string GetLanguageForAdaptationSet( IAdaptationSet *adaptationSet );
	AAMPStatusType  GetMpdFromManfiest(const GrowableBuffer &manifest, MPD * &mpd, std::string manifestUrl, bool init = false);

	bool fragmentCollectorThreadStarted;
	std::set<std::string> mLangList;
	double seekPosition;
	float rate;
	pthread_t fragmentCollectorThreadID;
	pthread_t createDRMSessionThreadID;
	bool drmSessionThreadStarted;
	dash::mpd::IMPD *mpd;
	MediaStreamContext *mMediaStreamContext[AAMP_TRACK_COUNT];
	int mNumberOfTracks;
	int mCurrentPeriodIdx;
	double mEndPosition;
	bool mIsLiveStream;        //Stream is live or not; won't change during runtime.
	bool mIsLiveManifest;      //Current manifest is dynamic or static; may change during runtime. eg: Hot DVR.
	StreamAbstractionAAMP_MPD* mContext;
	StreamInfo* mStreamInfo;
	double mPrevStartTimeSeconds;
	std::string mPrevLastSegurlMedia;
	long mPrevLastSegurlOffset; //duration offset from beginning of TSB
	unsigned char *lastProcessedKeyId;
	int lastProcessedKeyIdLen;
	uint64_t mPeriodEndTime;
	uint64_t mPeriodStartTime;
	int64_t mMinUpdateDurationMs;
	double mTSBDepth;
	double mPresentationOffsetDelay;
	uint64_t mLastPlaylistDownloadTimeMs;
	double mFirstPTS;
	double mVideoPosRemainder;
	double mFirstFragPTS[AAMP_TRACK_COUNT];
	AudioType mAudioType;
	bool mPushEncInitFragment;
	int mPrevAdaptationSetCount;
	std::unordered_map<long, int> mBitrateIndexMap;
	bool mIsFogTSB;
	vector<PeriodInfo> mMPDPeriodsInfo;
	IPeriod *mCurrentPeriod;
	std::string mBasePeriodId;
	double mBasePeriodOffset;
	PrivateCDAIObjectMPD *mCdaiObject;

	// DASH does not use abr manager to store the supported bandwidth values,
	// hence storing max TSB bandwith in this variable which will be used for VideoEnd Metric data via
	// StreamAbstractionAAMP::GetMaxBitrate function,
	long mMaxTSBBandwidth;

	double mLiveEndPosition;
	double mCulledSeconds;
	bool mAdPlayingFromCDN;   /*Note: TRUE: Ad playing currently & from CDN. FALSE: Ad "maybe playing", but not from CDN.*/
	double mAvailabilityStartTime;
};


/**
 * @brief PrivateStreamAbstractionMPD Constructor
 * @param context MPD fragment collector context
 * @param aamp Pointer to associated aamp private object
 * @param seekpos Seek positon
 * @param rate playback rate
 */
PrivateStreamAbstractionMPD::PrivateStreamAbstractionMPD( StreamAbstractionAAMP_MPD* context, PrivateInstanceAAMP *aamp,double seekpos, float rate) : aamp(aamp),
	fragmentCollectorThreadStarted(false), mLangList(), seekPosition(seekpos), rate(rate), fragmentCollectorThreadID(0), createDRMSessionThreadID(0),
	drmSessionThreadStarted(false), mpd(NULL), mNumberOfTracks(0), mCurrentPeriodIdx(0), mEndPosition(0), mIsLiveStream(true), mIsLiveManifest(true), mContext(context),
	mStreamInfo(NULL), mPrevStartTimeSeconds(0), mPrevLastSegurlMedia(""), mPrevLastSegurlOffset(0), lastProcessedKeyId(NULL),
	lastProcessedKeyIdLen(0), mPeriodEndTime(0), mPeriodStartTime(0), mMinUpdateDurationMs(DEFAULT_INTERVAL_BETWEEN_MPD_UPDATES_MS),
	mLastPlaylistDownloadTimeMs(0), mFirstPTS(0), mAudioType(eAUDIO_UNKNOWN), mPushEncInitFragment(false),
	mPrevAdaptationSetCount(0), mBitrateIndexMap(), mIsFogTSB(false), mMPDPeriodsInfo(),
	mCurrentPeriod(NULL), mBasePeriodId(""), mBasePeriodOffset(0), mCdaiObject(NULL), mLiveEndPosition(0), mCulledSeconds(0)
	,mAdPlayingFromCDN(false)
	,mMaxTSBBandwidth(0), mTSBDepth(0)
	,mVideoPosRemainder(0)
	,mPresentationOffsetDelay(0)
	,mAvailabilityStartTime(0)
{
	this->aamp = aamp;
	memset(&mMediaStreamContext, 0, sizeof(mMediaStreamContext));
	for (int i=0; i<AAMP_TRACK_COUNT; i++) mFirstFragPTS[i] = 0.0;
	mContext->GetABRManager().clearProfiles();
	mLastPlaylistDownloadTimeMs = aamp_GetCurrentTimeMS();
};


/**
 * @brief Check if mime type is compatible with media type
 * @param mimeType mime type
 * @param mediaType media type
 * @retval true if compatible
 */
static bool IsCompatibleMimeType(const std::string& mimeType, MediaType mediaType)
{
	bool isCompatible = false;

	switch ( mediaType )
	{
		case eMEDIATYPE_VIDEO:
			if (mimeType == "video/mp4")
				isCompatible = true;
			break;

		case eMEDIATYPE_AUDIO:
			if ((mimeType == "audio/webm") ||
				(mimeType == "audio/mp4"))
				isCompatible = true;
			break;

		case eMEDIATYPE_SUBTITLE:
			if (mimeType == "text/vtt")
				isCompatible = true;
			break;

		default:
			break;
	}

	return isCompatible;
}

/**
 * @brief Get Additional tag property value from any child node of MPD
 * @param Pointer to MPD child node, Tage Name , Property Name, 
 * SchemeIdUri (if the propery mapped against scheme Id , default value is empty)
 * @retval return the property name if found, if not found return empty string 
 */
static bool IsAtmosAudio(const IMPDElement *nodePtr)
{
	bool isAtmos = false;

	if (!nodePtr){
		AAMPLOG_ERR("%s:%d > API Failed due to Invalid Arguments", __FUNCTION__, __LINE__);	
	}else{
		std::vector<INode*> childNodeList = nodePtr->GetAdditionalSubNodes();
		for (size_t j=0; j < childNodeList.size(); j++) {
			INode* childNode = childNodeList.at(j);
			const std::string& name = childNode->GetName();
			if (name == SUPPLEMENTAL_PROPERTY_TAG ) {
				if (childNode->HasAttribute("schemeIdUri")){
					const std::string& schemeIdUri = childNode->GetAttributeValue("schemeIdUri");
					if (schemeIdUri == SCHEME_ID_URI_EC3_EXT_CODEC ){
						if (childNode->HasAttribute("value")) {
							std::string value = childNode->GetAttributeValue("value");
							AAMPLOG_INFO("%s:%d > Recieved %s tag property value as %s ",
				 			__FUNCTION__, __LINE__, SUPPLEMENTAL_PROPERTY_TAG, value.c_str());
							if (value == EC3_EXT_VALUE_AUDIO_ATMOS){
								isAtmos = true;
								break;
							}

						}
					}
				}
			}
		}
	}

	return isAtmos;
}
/**
 * @brief Get representation index of desired codec
 * @param adaptationSet Adaptation set object
 * @param[out] selectedCodecType type of desired representation
 * @retval index of desired representation
 */
static int GetDesiredCodecIndex(IAdaptationSet *adaptationSet, AudioType &selectedCodecType, uint32_t &selectedRepBandwidth)
{
    const std::vector<IRepresentation *> representation = adaptationSet->GetRepresentation();
	int selectedRepIdx = -1;
	// check for codec defined in Adaptation Set
	const std::vector<string> adapCodecs = adaptationSet->GetCodecs();
	for (int representationIndex = 0; representationIndex < representation.size(); representationIndex++)
	{
		const dash::mpd::IRepresentation *rep = representation.at(representationIndex);
		uint32_t bandwidth = rep->GetBandwidth();
		const std::vector<string> codecs = rep->GetCodecs();
		AudioType audioType = eAUDIO_UNKNOWN;
		string codecValue="";
		// check if Representation includec codec
		if(codecs.size())
			codecValue=codecs.at(0);
		else if(adapCodecs.size()) // else check if Adaptation has codec defn
			codecValue = adapCodecs.at(0);
		// else no codec defined , go with unknown

		if (codecValue == "ec+3")
		{
#ifndef __APPLE__
			audioType = eAUDIO_ATMOS;
#endif
		}
		else if (codecValue == "ec-3")
		{
			audioType = eAUDIO_DDPLUS;
			/*
			* check whether ATMOS Flag is set as per ETSI TS 103 420
			*/
			if (IsAtmosAudio(rep)){
				AAMPLOG_INFO("%s:%d > Setting audio codec as eAUDIO_ATMOS as per ETSI TS 103 420",
					 __FUNCTION__, __LINE__);
				audioType = eAUDIO_ATMOS;
			}
		}
		else if( codecValue == "opus" || codecValue.find("vorbis") != std::string::npos )
		{
			audioType = eAUDIO_UNSUPPORTED;
		}
		else if( codecValue == "aac" || codecValue.find("mp4") != std::string::npos )
		{
			audioType = eAUDIO_AAC;
		}
		/*
		* By default the audio profile selection priority is set as ATMOS then DD+ then AAC
		* Note that this check comes after the check of selected language.
		* disableATMOS: avoid use of ATMOS track
		* disableEC3: avoid use of DDPLUS and ATMOS tracks
		*/
		if ((selectedCodecType == eAUDIO_UNKNOWN && (audioType != eAUDIO_UNSUPPORTED || selectedRepBandwidth == 0)) || // Select any profile for the first time, reject unsupported streams then
			(selectedCodecType == audioType && bandwidth>selectedRepBandwidth) || // same type but better quality
			(selectedCodecType < eAUDIO_ATMOS && audioType == eAUDIO_ATMOS && !gpGlobalConfig->disableATMOS && !gpGlobalConfig->disableEC3) || // promote to atmos
			(selectedCodecType < eAUDIO_DDPLUS && audioType == eAUDIO_DDPLUS && !gpGlobalConfig->disableEC3) || // promote to ddplus
			(selectedCodecType != eAUDIO_AAC && audioType == eAUDIO_AAC && gpGlobalConfig->disableEC3) || // force AAC
			(selectedCodecType == eAUDIO_UNSUPPORTED) // anything better than nothing
			)
		{
			selectedRepIdx = representationIndex;
			selectedCodecType = audioType;
			selectedRepBandwidth = bandwidth;
			AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d  > SelectedRepIndex : %d ,selectedCodecType : %d, selectedRepBandwidth: %d", __FUNCTION__, __LINE__, selectedRepIdx, selectedCodecType, selectedRepBandwidth);
		}
	}
	return selectedRepIdx;
}

/**
 * @brief Get representation index of desired video codec
 * @param adaptationSet Adaptation set object
 * @param[out] selectedRepIdx index of desired representation
 * @retval index of desired representation
 */
static int GetDesiredVideoCodecIndex(IAdaptationSet *adaptationSet)
{
	const std::vector<IRepresentation *> representation = adaptationSet->GetRepresentation();
	int selectedRepIdx = -1;
	for (int representationIndex = 0; representationIndex < representation.size(); representationIndex++)
	{
		const dash::mpd::IRepresentation *rep = representation.at(representationIndex);

		const std::vector<string> adapCodecs = adaptationSet->GetCodecs();
		const std::vector<string> codecs = rep->GetCodecs();
		string codecValue="";
		if(codecs.size())
			codecValue=codecs.at(0);
		else if(adapCodecs.size())
			codecValue = adapCodecs.at(0);

		//Ignore vp8 and vp9 codec video profiles(webm)
		if(codecValue.find("vp") == std::string::npos)
		{
			selectedRepIdx = representationIndex;
		}
	}
	return selectedRepIdx;
}

/**
 * @brief Check if adaptation set is of a given media type
 * @param adaptationSet adaptation set
 * @param mediaType media type
 * @retval true if adaptation set is of the given media type
 */
static bool IsContentType(IAdaptationSet *adaptationSet, MediaType mediaType )
{
	const char *name = mMediaTypeName[mediaType];

	if (adaptationSet->GetContentType() == name)
	{
		return true;
	}
	else if (adaptationSet->GetContentType() == "muxed")
	{
		logprintf("excluding muxed content");
	}
	else
	{
		if (IsCompatibleMimeType(adaptationSet->GetMimeType(), mediaType) )
		{
			return true;
		}
		const std::vector<IRepresentation *> &representation = adaptationSet->GetRepresentation();
		for (int i = 0; i < representation.size(); i++)
		{
			const IRepresentation * rep = representation.at(i);
			if (IsCompatibleMimeType(rep->GetMimeType(), mediaType) )
			{
				return true;
			}
		}

		const std::vector<IContentComponent *>contentComponent = adaptationSet->GetContentComponent();
		for( int i = 0; i < contentComponent.size(); i++)
		{
			if (contentComponent.at(i)->GetContentType() == name)
			{
				return true;
			}
		}
	}
	return false;
}

/**
 * @brief read unsigned 32 bit value and update buffer pointer
 * @param[in][out] pptr buffer
 * @retval 32 bit value
 */
static unsigned int Read32( const char **pptr)
{
	const char *ptr = *pptr;
	unsigned int rc = 0;
	for (int i = 0; i < 4; i++)
	{
		rc <<= 8;
		rc |= (unsigned char)*ptr++;
	}
	*pptr = ptr;
	return rc;
}


/**
 * @brief Parse segment index box
 * @note The SegmentBase indexRange attribute points to Segment Index Box location with segments and random access points.
 * @param start start of box
 * @param size size of box
 * @param segmentIndex segment index
 * @param[out] referenced_size referenced size
 * @param[out] referenced_duration referenced duration
 * @retval true on success
 */
static bool ParseSegmentIndexBox( const char *start, size_t size, int segmentIndex, unsigned int *referenced_size, float *referenced_duration )
{
	const char **f = &start;
	unsigned int len = Read32(f);
	assert(len == size);
	unsigned int type = Read32(f);
	assert(type == 'sidx');
	unsigned int version = Read32(f);
	unsigned int reference_ID = Read32(f);
	unsigned int timescale = Read32(f);
	unsigned int earliest_presentation_time = Read32(f);
	unsigned int first_offset = Read32(f);
	unsigned int count = Read32(f);
	for (unsigned int i = 0; i < count; i++)
	{
		*referenced_size = Read32(f);
		*referenced_duration = Read32(f)/(float)timescale;
		unsigned int flags = Read32(f);
		if (i == segmentIndex) return true;
	}
	return false;
}


/**
 * @brief Replace matching token with given number
 * @param str String in which operation to be performed
 * @param from token
 * @param toNumber number to replace token
 * @retval position
 */
static int replace(std::string& str, const std::string& from, uint64_t toNumber )
{
	int rc = 0;
	size_t tokenLength = from.length();

	for (;;)
	{
		bool done = true;
		size_t pos = 0;
		for (;;)
		{
			pos = str.find('$', pos);
			if (pos == std::string::npos)
			{
				break;
			}
			size_t next = str.find('$', pos + 1);
			if (str.substr(pos + 1, tokenLength) == from)
			{
				size_t formatLen = next - pos - tokenLength - 1;
				char buf[256];
				if (formatLen > 0)
				{
					std::string format = str.substr(pos + tokenLength + 1, formatLen);
					sprintf(buf, format.c_str(), toNumber);
					tokenLength += formatLen;
				}
				else
				{
					sprintf(buf, "%" PRIu64 "", toNumber);
				}
				str.replace(pos, tokenLength + 2, buf);
				done = false;
				rc++;
				break;
			}
			pos = next + 1;
		}
		if (done) break;
	}

	return rc;
}


/**
 * @brief Replace matching token with given string
 * @param str String in which operation to be performed
 * @param from token
 * @param toString string to replace token
 * @retval position
 */
static int replace(std::string& str, const std::string& from, const std::string& toString )
{
	int rc = 0;
	size_t tokenLength = from.length();

	for (;;)
	{
		bool done = true;
		size_t pos = 0;
		for (;;)
		{
			pos = str.find('$', pos);
			if (pos == std::string::npos)
			{
				break;
			}
			size_t next = str.find('$', pos + 1);
			if (str.substr(pos + 1, tokenLength) == from)
			{
				str.replace(pos, tokenLength + 2, toString);
				done = false;
				rc++;
				break;
			}
			pos = next + 1;
		}

		if (done) break;
	}

	return rc;
}


/**
 * @brief Generates fragment url from media information
 * @param[out] fragmentUrl fragment url
 * @param fragmentDescriptor descriptor
 * @param media media information string
 */
static void GetFragmentUrl( std::string& fragmentUrl, const FragmentDescriptor *fragmentDescriptor, std::string media)
{
	std::string constructedUri;
	if( media.compare(0, 7, "http://")==0 || media.compare(0, 8, "https://")==0 )
	{	// don't pre-pend baseurl if media starts with http:// or https://
	}
	else if (fragmentDescriptor->baseUrls->size() > 0)
	{
		constructedUri = fragmentDescriptor->baseUrls->at(0)->GetUrl();
		if(gpGlobalConfig->dashIgnoreBaseURLIfSlash)
		{
			if (constructedUri == "/")
			{
				logprintf("%s:%d ignoring baseurl /", __FUNCTION__, __LINE__);
				constructedUri.clear();
			}
		}

		//Add '/' to BaseURL if not already available.
		if( constructedUri.compare(0, 7, "http://")==0 || constructedUri.compare(0, 8, "https://")==0 )
		{
			if( constructedUri.back() != '/' )
			{
				constructedUri += '/';
			}
		}
	}
	else
	{
		AAMPLOG_TRACE("%s:%d BaseURL not available", __FUNCTION__, __LINE__);
	}
	constructedUri += media;

	replace(constructedUri, "Bandwidth", fragmentDescriptor->Bandwidth);
	replace(constructedUri, "RepresentationID", fragmentDescriptor->RepresentationID);
	replace(constructedUri, "Number", fragmentDescriptor->Number);
	replace(constructedUri, "Time", fragmentDescriptor->Time );

	aamp_ResolveURL(fragmentUrl, fragmentDescriptor->manifestUrl, constructedUri.c_str());
}

#ifdef AAMP_HARVEST_SUPPORT_ENABLED

#include <sys/stat.h>

/**
 * @brief Gets file path to havest
 * @param[out] filePath path of file
 * @param fragmentDescriptor fragment descriptor
 * @param media string containing media info
 */
static void GetFilePath(std::string& filePath, const FragmentDescriptor *fragmentDescriptor, std::string media)
{
	std::string constructedUri;
        if(gpGlobalConfig->harvestpath)
                constructedUri = gpGlobalConfig->harvestpath;
        else
                constructedUri = HARVEST_BASE_PATH;
	constructedUri += media;
	replace(constructedUri, "Bandwidth", fragmentDescriptor->Bandwidth);
	replace(constructedUri, "RepresentationID", fragmentDescriptor->RepresentationID);
	replace(constructedUri, "Number", fragmentDescriptor->Number);
	replace(constructedUri, "Time", fragmentDescriptor->Time);
	filePath = constructedUri;
}


/**
 * @brief Write file to storage
 * @param fileName out file name
 * @param data buffer
 * @param len length of buffer
 */
static void WriteFile(std::string fileName, const char* data, int len)
{
	std::size_t pos = fileName.rfind('/');
	std::string dirpath = fileName.substr(0, pos);
	DIR *d = opendir(dirpath.c_str());
	if (!d)
	{
		mkdir(dirpath.c_str(), 0777);
	}
	else
		closedir(d);

	std::ofstream f(fileName, std::ofstream::binary);
	if (f.good())
	{
		f.write(data, len);
		f.close();
	}
	else
	{
		logprintf("File open failed. outfile = %s ", fileName.c_str());
	}
}
#endif // AAMP_HARVEST_SUPPORT_ENABLED


/**
 * @brief Fetch and cache a fragment
 *
 * @param pMediaStreamContext Track object pointer
 * @param media media descriptor string
 * @param fragmentDuration duration of fragment in seconds
 * @param isInitializationSegment true if fragment is init fragment
 * @param curlInstance curl instance to be used to fetch
 * @param discontinuity true if fragment is discontinuous
 * @retval true on fetch success
 */
bool PrivateStreamAbstractionMPD::FetchFragment(MediaStreamContext *pMediaStreamContext, std::string media, double fragmentDuration, bool isInitializationSegment, unsigned int curlInstance, bool discontinuity)
{ // given url, synchronously download and transmit associated fragment
	bool retval = true;
	std::string fragmentUrl;
	GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, media);
	size_t len = 0;
	float position;
	if(isInitializationSegment)
	{
		if(!(pMediaStreamContext->initialization.empty()) && (0 == pMediaStreamContext->initialization.compare(fragmentUrl))&& !discontinuity)
		{
			AAMPLOG_TRACE("We have pushed the same initailization segment for %s skipping", mMediaTypeName[pMediaStreamContext->type]);
			return retval;
		}
		else
		{
			pMediaStreamContext->initialization = std::string(fragmentUrl);
		}
	}
	position = pMediaStreamContext->fragmentTime;

	float duration = fragmentDuration;
	if(rate > AAMP_NORMAL_PLAY_RATE)
	{
		position = position/rate;
		AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d rate %f pMediaStreamContext->fragmentTime %f updated position %f",
				__FUNCTION__, __LINE__, rate, pMediaStreamContext->fragmentTime, position);
		duration = duration/rate * gpGlobalConfig->vodTrickplayFPS;
		//aamp->disContinuity();
	}
//	logprintf("%s:%d [%s] mFirstFragPTS %f  position %f -> %f ", __FUNCTION__, __LINE__, pMediaStreamContext->name, mFirstFragPTS[pMediaStreamContext->mediaType], position, mFirstFragPTS[pMediaStreamContext->mediaType]+position);
	position += mFirstFragPTS[pMediaStreamContext->mediaType];
	bool fragmentCached = pMediaStreamContext->CacheFragment(fragmentUrl, curlInstance, position, duration, NULL, isInitializationSegment, discontinuity
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		, media
#endif
		,(mCdaiObject->mAdState == AdState::IN_ADBREAK_AD_PLAYING));
	// Check if we have downloaded the fragment and waiting for init fragment download on
	// bitrate switching before caching it.
	bool fragmentSaved = (NULL != pMediaStreamContext->mDownloadedFragment.ptr);

	if (!fragmentCached)
	{
		if(!fragmentSaved)
		{
		logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f", __FUNCTION__, __LINE__, fragmentUrl.c_str(), pMediaStreamContext->fragmentTime);
			if(mCdaiObject->mAdState == AdState::IN_ADBREAK_AD_PLAYING && (isInitializationSegment || pMediaStreamContext->segDLFailCount >= MAX_AD_SEG_DOWNLOAD_FAIL_COUNT))
			{
				logprintf("PrivateStreamAbstractionMPD::%s:%d [CDAI] Ad fragment not available. Playback failed.", __FUNCTION__, __LINE__);
				mCdaiObject->mAdFailed = true;
			}
		}
		retval = false;
	}

	/**In the case of ramp down same fragment will be retried
	 *Avoid fragmentTime update in such scenarios.
	 *In other cases if it's success or failure, AAMP will be going
	 *For next fragment so update fragmentTime with fragment duration
	 */
	if(!mContext->mCheckForRampdown && !fragmentSaved)
	{
		if(rate > 0)
		{
			pMediaStreamContext->fragmentTime += fragmentDuration;
			if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO) mBasePeriodOffset += fragmentDuration;
		}
		else
		{
			pMediaStreamContext->fragmentTime -= fragmentDuration;
			if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO) mBasePeriodOffset -= fragmentDuration;
			if(pMediaStreamContext->fragmentTime < 0)
			{
				pMediaStreamContext->fragmentTime = 0;
			}
		}
	}
	return retval;
}


/**
 * @brief Fetch and push next fragment
 * @param pMediaStreamContext Track object
 * @param curlInstance instance of curl to be used to fetch
 * @retval true if push is done successfully
 */
bool PrivateStreamAbstractionMPD::PushNextFragment( struct MediaStreamContext *pMediaStreamContext, unsigned int curlInstance)
{
	ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
	bool retval=false;
	if (!segmentTemplate)
	{
		segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
	}

#ifdef DEBUG_TIMELINE
	logprintf("%s:%d Type[%d] timeLineIndex %d segmentTemplate %p fragmentRepeatCount %u", __FUNCTION__, __LINE__,pMediaStreamContext->type,
	        pMediaStreamContext->timeLineIndex, segmentTemplate, pMediaStreamContext->fragmentRepeatCount);
#endif
	if (segmentTemplate)
	{
		std::string media = segmentTemplate->Getmedia();
		const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
		if (segmentTimeline)
		{
			uint32_t timeScale = segmentTemplate->GetTimescale();
			std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
#ifdef DEBUG_TIMELINE
			logprintf("%s:%d Type[%d] timelineCnt=%d timeLineIndex:%d fragTime=%f L=%" PRIu64 " [fragmentTime = %f,  mLiveEndPosition = %f]", __FUNCTION__, __LINE__,
				pMediaStreamContext->type ,timelines.size(),pMediaStreamContext->timeLineIndex,pMediaStreamContext->fragmentDescriptor.Time,pMediaStreamContext->lastSegmentTime
				, pMediaStreamContext->fragmentTime, mLiveEndPosition);
#endif

			if ((pMediaStreamContext->timeLineIndex >= timelines.size()) || (pMediaStreamContext->timeLineIndex < 0)
					||(AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState &&
						((rate > AAMP_NORMAL_PLAY_RATE && pMediaStreamContext->fragmentTime >= mLiveEndPosition)
						 ||(rate < 0 && pMediaStreamContext->fragmentTime <= 0))))
			{
				AAMPLOG_INFO("%s:%d Type[%d] EOS. timeLineIndex[%d] size [%lu]",__FUNCTION__, __LINE__,pMediaStreamContext->type, pMediaStreamContext->timeLineIndex, timelines.size());
				pMediaStreamContext->eos = true;
			}
			else
			{
				if (pMediaStreamContext->fragmentRepeatCount == 0)
				{
					ITimeline *timeline = timelines.at(pMediaStreamContext->timeLineIndex);
					uint64_t startTime = 0;
					map<string, string> attributeMap = timeline->GetRawAttributes();
					if(attributeMap.find("t") != attributeMap.end())
					{
						startTime = timeline->GetStartTime();
					}
					else
					{ // DELIA-35059
						startTime = pMediaStreamContext->fragmentDescriptor.Time;
					}
					if(startTime && mIsLiveStream)
					{
						// After mpd refresh , Time will be 0. Need to traverse to the right fragment for playback
						if(0 == pMediaStreamContext->fragmentDescriptor.Time)
						{
							uint32_t duration =0;
							uint32_t repeatCount =0;
							uint64_t nextStartTime = 0;
							int index = pMediaStreamContext->timeLineIndex;
							// This for loop is to go to the right index based on LastSegmentTime
							for(;index<timelines.size();index++)
							{
								timeline = timelines.at(index);
								startTime = timeline->GetStartTime();
								duration = timeline->GetDuration();
								// For Dynamic segment timeline content
								if (0 == startTime && 0 != duration)
								{
									startTime = nextStartTime;
								}
								repeatCount = timeline->GetRepeatCount();
								nextStartTime = startTime+((repeatCount+1)*duration);
								if(pMediaStreamContext->lastSegmentTime < nextStartTime)
								{
									break;
								}
								pMediaStreamContext->fragmentDescriptor.Number += (repeatCount+1);
							}// end of for

							/*
							*  Boundary check added to handle the edge case leading to crash,
							*  reported in DELIA-30316.
							*/
							if(index == timelines.size())
							{
								logprintf("%s:%d Type[%d] Boundary Condition !!! Index(%d) reached Max.Start=%" PRIu64 " Last=%" PRIu64 " ",__FUNCTION__, __LINE__,
									pMediaStreamContext->type,index,startTime,pMediaStreamContext->lastSegmentTime);
								index--;
								startTime = pMediaStreamContext->lastSegmentTime;
								pMediaStreamContext->fragmentRepeatCount = repeatCount+1;
							}

#ifdef DEBUG_TIMELINE
							logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d Index=%d Num=%" PRIu64 " FTime=%f",__FUNCTION__, __LINE__, pMediaStreamContext->type,
							startTime,pMediaStreamContext->lastSegmentTime, duration, repeatCount,index,
							pMediaStreamContext->fragmentDescriptor.Number,pMediaStreamContext->fragmentTime);
#endif
							pMediaStreamContext->timeLineIndex = index;
							// Now we reached the right row , need to traverse the repeat index to reach right node
							while(startTime < pMediaStreamContext->lastSegmentTime &&
								pMediaStreamContext->fragmentRepeatCount < repeatCount )
							{
								startTime += duration;
								pMediaStreamContext->fragmentDescriptor.Number++;
								pMediaStreamContext->fragmentRepeatCount++;
							}
#ifdef DEBUG_TIMELINE
							logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d fragRep=%d Index=%d Num=%" PRIu64 " FTime=%f",__FUNCTION__, __LINE__, pMediaStreamContext->type,
							startTime,pMediaStreamContext->lastSegmentTime, duration, repeatCount,pMediaStreamContext->fragmentRepeatCount,pMediaStreamContext->timeLineIndex,
							pMediaStreamContext->fragmentDescriptor.Number,pMediaStreamContext->fragmentTime);
#endif
						}
					}// if starttime
					if(0 == pMediaStreamContext->timeLineIndex)
					{
						AAMPLOG_INFO("%s:%d Type[%d] update startTime to %" PRIu64 , __FUNCTION__, __LINE__,pMediaStreamContext->type, startTime);
					}
					pMediaStreamContext->fragmentDescriptor.Time = startTime;
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] Setting startTime to %" PRIu64 , __FUNCTION__, __LINE__,pMediaStreamContext->type, startTime);
#endif
				}// if fragRepeat == 0

				ITimeline *timeline = timelines.at(pMediaStreamContext->timeLineIndex);
				uint32_t repeatCount = timeline->GetRepeatCount();
				uint32_t duration = timeline->GetDuration();
#ifdef DEBUG_TIMELINE
				logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d fragrep=%d x=%d num=%lld",__FUNCTION__, __LINE__,
				pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time,
				pMediaStreamContext->lastSegmentTime, duration, repeatCount,pMediaStreamContext->fragmentRepeatCount,
				pMediaStreamContext->timeLineIndex,pMediaStreamContext->fragmentDescriptor.Number);
#endif
				if ((pMediaStreamContext->fragmentDescriptor.Time > pMediaStreamContext->lastSegmentTime) || (0 == pMediaStreamContext->lastSegmentTime))
				{
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] presenting %" PRIu64 " Number(%lld) Last=%" PRIu64 " Duration(%d) FTime(%f) ",__FUNCTION__, __LINE__,
					pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time,pMediaStreamContext->fragmentDescriptor.Number,pMediaStreamContext->lastSegmentTime,duration,pMediaStreamContext->fragmentTime);
#endif
					double fragmentDuration = (double)duration/(double)timeScale;
					retval = FetchFragment( pMediaStreamContext, media, fragmentDuration, false, curlInstance);
					if(retval)
					{
						pMediaStreamContext->lastSegmentTime = pMediaStreamContext->fragmentDescriptor.Time;
					}
					else if((mIsFogTSB && !mAdPlayingFromCDN) && pMediaStreamContext->mDownloadedFragment.ptr)
					{
						pMediaStreamContext->profileChanged = true;
						mContext->profileIdxForBandwidthNotification = mBitrateIndexMap[pMediaStreamContext->fragmentDescriptor.Bandwidth];
						FetchAndInjectInitialization();
						return false;
					}
					else if(mContext->mCheckForRampdown && pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO)
					{
						// DELIA-31780 - On audio fragment download failure (http500), rampdown was attempted .
						// rampdown is only needed for video fragments not for audio.
						// second issue : after rampdown lastSegmentTime was going into "0" . When this combined with mpd refresh immediately after rampdown ,
						// startTime is set to start of Period . This caused audio fragment download from "0" resulting in PTS mismatch and mute
						// Fix : Only do lastSegmentTime correction for video not for audio
						//	 lastSegmentTime to be corrected with duration of last segment attempted .
						return retval; /* Incase of fragment download fail, no need to increase the fragment number to download next fragment,
								 * instead check the same fragment in lower profile. */
					}
				}
				else if (rate < 0)
				{
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] presenting %f" , __FUNCTION__, __LINE__,pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time);
#endif
					pMediaStreamContext->lastSegmentTime = pMediaStreamContext->fragmentDescriptor.Time;
					double fragmentDuration = (double)duration/(double)timeScale;
					retval = FetchFragment( pMediaStreamContext, media, fragmentDuration, false, curlInstance);
					if (!retval && ((mIsFogTSB && !mAdPlayingFromCDN) && pMediaStreamContext->mDownloadedFragment.ptr))
					{
						pMediaStreamContext->profileChanged = true;
						mContext->profileIdxForBandwidthNotification = mBitrateIndexMap[pMediaStreamContext->fragmentDescriptor.Bandwidth];
						FetchAndInjectInitialization();
						return false;
					}
				}
				else if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO &&
						((pMediaStreamContext->lastSegmentTime - pMediaStreamContext->fragmentDescriptor.Time) > TIMELINE_START_RESET_DIFF))
				{
					if(!mIsLiveStream || !aamp->IsLiveAdjustRequired())
					{
						pMediaStreamContext->lastSegmentTime = pMediaStreamContext->fragmentDescriptor.Time - 1;
						return false;
					}
					logprintf("%s:%d Calling ScheduleRetune to handle start-time reset lastSegmentTime=%" PRIu64 " start-time=%f" , __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentTime, pMediaStreamContext->fragmentDescriptor.Time);
					aamp->ScheduleRetune(eDASH_ERROR_STARTTIME_RESET, pMediaStreamContext->mediaType);
				}
				else
				{
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] Before skipping. fragmentDescriptor.Time %f lastSegmentTime %" PRIu64 " Index=%d fragRep=%d,repMax=%d Number=%lld",__FUNCTION__, __LINE__,pMediaStreamContext->type,
						pMediaStreamContext->fragmentDescriptor.Time, pMediaStreamContext->lastSegmentTime,pMediaStreamContext->timeLineIndex,
						pMediaStreamContext->fragmentRepeatCount , repeatCount,pMediaStreamContext->fragmentDescriptor.Number);
#endif
					while(pMediaStreamContext->fragmentDescriptor.Time < pMediaStreamContext->lastSegmentTime &&
							pMediaStreamContext->fragmentRepeatCount < repeatCount )
						{
							if(rate > 0)
							{
								pMediaStreamContext->fragmentDescriptor.Time += duration;
								pMediaStreamContext->fragmentDescriptor.Number++;
								pMediaStreamContext->fragmentRepeatCount++;
							}
						}
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] After skipping. fragmentDescriptor.Time %f lastSegmentTime %" PRIu64 " Index=%d Number=%lld",__FUNCTION__, __LINE__,pMediaStreamContext->type,
							pMediaStreamContext->fragmentDescriptor.Time, pMediaStreamContext->lastSegmentTime,pMediaStreamContext->timeLineIndex,pMediaStreamContext->fragmentDescriptor.Number);
#endif
				}
				if(rate > 0)
				{
					pMediaStreamContext->fragmentDescriptor.Time += duration;
					pMediaStreamContext->fragmentDescriptor.Number++;
					pMediaStreamContext->fragmentRepeatCount++;
					if( pMediaStreamContext->fragmentRepeatCount > repeatCount)
					{
						pMediaStreamContext->fragmentRepeatCount = 0;
						pMediaStreamContext->timeLineIndex++;
					}
#ifdef DEBUG_TIMELINE
						logprintf("%s:%d Type[%d] After Incr. fragmentDescriptor.Time %f lastSegmentTime %" PRIu64 " Index=%d fragRep=%d,repMax=%d Number=%lld",__FUNCTION__, __LINE__,pMediaStreamContext->type,
						pMediaStreamContext->fragmentDescriptor.Time, pMediaStreamContext->lastSegmentTime,pMediaStreamContext->timeLineIndex,
						pMediaStreamContext->fragmentRepeatCount , repeatCount,pMediaStreamContext->fragmentDescriptor.Number);
#endif
				}
				else
				{
					pMediaStreamContext->fragmentDescriptor.Time -= duration;
					pMediaStreamContext->fragmentDescriptor.Number--;
					pMediaStreamContext->fragmentRepeatCount--;
					if( pMediaStreamContext->fragmentRepeatCount < 0)
					{
						pMediaStreamContext->timeLineIndex--;
						if(pMediaStreamContext->timeLineIndex >= 0)
						{
							pMediaStreamContext->fragmentRepeatCount = timelines.at(pMediaStreamContext->timeLineIndex)->GetRepeatCount();
						}
					}
				}
			}
		}
		else
		{
#ifdef DEBUG_TIMELINE
			logprintf("%s:%d segmentTimeline not available", __FUNCTION__, __LINE__);
#endif

			double currentTimeSeconds = (double)aamp_GetCurrentTimeMS() / 1000;
			double fragmentDuration = ((double)segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
			if (!fragmentDuration)
			{
				fragmentDuration = 2; // hack
			}
			if (0 == pMediaStreamContext->lastSegmentNumber)
			{
				if (mIsLiveStream)
				{
					double liveTime = currentTimeSeconds - aamp->mLiveOffset;
					if(liveTime < mPeriodStartTime)
					{
						// Not to go beyond the period , as that is checked in skipfragments 
						liveTime = mPeriodStartTime;
					}
					pMediaStreamContext->lastSegmentNumber = (long long)((liveTime - mPeriodStartTime) / fragmentDuration) + segmentTemplate->GetStartNumber();
					pMediaStreamContext->fragmentDescriptor.Time = liveTime;
					AAMPLOG_INFO("%s %d Printing fragmentDescriptor.Number %" PRIu64 " Time=%f  ", __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentNumber, pMediaStreamContext->fragmentDescriptor.Time);
				}
				else
				{
					if (rate < 0)
					{
						pMediaStreamContext->fragmentDescriptor.Time = mPeriodEndTime;
					}
					else
					{
						pMediaStreamContext->fragmentDescriptor.Time = mPeriodStartTime;
					}
				}
			}

			// Recalculate the fragmentDescriptor.Time after periodic manifest updates
			if (mIsLiveStream && 0 == pMediaStreamContext->fragmentDescriptor.Time)
			{
				pMediaStreamContext->fragmentDescriptor.Time = mPeriodStartTime;
				if(pMediaStreamContext->lastSegmentNumber > segmentTemplate->GetStartNumber())
				{
					pMediaStreamContext->fragmentDescriptor.Time += ((pMediaStreamContext->lastSegmentNumber - segmentTemplate->GetStartNumber()) * fragmentDuration);
				}
			}
			/**
			 *Find out if we reached end/beginning of period.
			 *First block in this 'if' is for VOD, where boundaries are 0 and PeriodEndTime
			 *Second block is for LIVE, where boundaries are
                         *  mPeriodStartTime and currentTime
			 */
			if ((!mIsLiveStream && ((mPeriodEndTime && (pMediaStreamContext->fragmentDescriptor.Time > mPeriodEndTime))
							|| (rate < 0 && pMediaStreamContext->fragmentDescriptor.Time < 0)))
					|| (mIsLiveStream && ((pMediaStreamContext->fragmentDescriptor.Time >= mPeriodEndTime)
							|| (pMediaStreamContext->fragmentDescriptor.Time < mPeriodStartTime))))
			{
				AAMPLOG_INFO("%s:%d EOS. fragmentDescriptor.Time=%f mPeriodEndTime=%lld mPeriodStartTime %lld  currentTimeSeconds %f FTime=%f",__FUNCTION__, __LINE__, pMediaStreamContext->fragmentDescriptor.Time, mPeriodEndTime, mPeriodStartTime, currentTimeSeconds, pMediaStreamContext->fragmentTime);
				pMediaStreamContext->lastSegmentNumber =0; // looks like change in period may happen now. hence reset lastSegmentNumber
				pMediaStreamContext->eos = true;
			}
			else if(mIsLiveStream && (pMediaStreamContext->fragmentDescriptor.Time + fragmentDuration) >= (currentTimeSeconds-mPresentationOffsetDelay))
			{
				int sleepTime = mMinUpdateDurationMs;
				sleepTime = (sleepTime > MAX_DELAY_BETWEEN_MPD_UPDATE_MS) ? MAX_DELAY_BETWEEN_MPD_UPDATE_MS : sleepTime;
				sleepTime = (sleepTime < 200) ? 200 : sleepTime;
				AAMPLOG_INFO("%s:%d Next fragment Not Available yet: fragmentDescriptor.Time %f currentTimeSeconds %f sleepTime %d ", __FUNCTION__, __LINE__, pMediaStreamContext->fragmentDescriptor.Time, currentTimeSeconds, sleepTime);
				aamp->InterruptableMsSleep(sleepTime);
				retval = false;
			}
			else
			{
				if (mIsLiveStream)
				{
					pMediaStreamContext->fragmentDescriptor.Number = pMediaStreamContext->lastSegmentNumber;
				}
				retval = FetchFragment(pMediaStreamContext, media, fragmentDuration, false, curlInstance);
				if (mContext->mCheckForRampdown)
				{
					/* NOTE : This case needs to be validated with the segmentTimeline not available stream */
					return retval;

				}

				if (rate > 0)
				{
					pMediaStreamContext->fragmentDescriptor.Number++;
					pMediaStreamContext->fragmentDescriptor.Time += fragmentDuration;
				}
				else
				{
					pMediaStreamContext->fragmentDescriptor.Number--;
					pMediaStreamContext->fragmentDescriptor.Time -= fragmentDuration;
				}
				pMediaStreamContext->lastSegmentNumber = pMediaStreamContext->fragmentDescriptor.Number;
			}
		}
	}
	else
	{
		ISegmentBase *segmentBase = pMediaStreamContext->representation->GetSegmentBase();
		if (segmentBase)
		{ // single-segment
			std::string fragmentUrl;
			GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
			if (!pMediaStreamContext->index_ptr)
			{ // lazily load index
				std::string range = segmentBase->GetIndexRange();
				int start;
				sscanf(range.c_str(), "%d-%d", &start, &pMediaStreamContext->fragmentOffset);

				ProfilerBucketType bucketType = aamp->GetProfilerBucketForMedia(pMediaStreamContext->mediaType, true);
				MediaType actualType = (MediaType)(eMEDIATYPE_INIT_VIDEO+pMediaStreamContext->mediaType);
				std::string effectiveUrl;
				long http_code;
				int iFogError = -1;
				int iCurrentRate = aamp->rate; //  Store it as back up, As sometimes by the time File is downloaded, rate might have changed due to user initiated Trick-Play
				pMediaStreamContext->index_ptr = aamp->LoadFragment(bucketType, fragmentUrl, effectiveUrl,&pMediaStreamContext->index_len, curlInstance, range.c_str(),&http_code,actualType,&iFogError);

				if (iCurrentRate != AAMP_NORMAL_PLAY_RATE)
				{
					actualType = eMEDIATYPE_IFRAME;
					if(actualType == eMEDIATYPE_INIT_VIDEO)
					{
						actualType = eMEDIATYPE_INIT_IFRAME;
					}
				}

				//update videoend info
				aamp->UpdateVideoEndMetrics( actualType,
										pMediaStreamContext->fragmentDescriptor.Bandwidth,
										(iFogError > 0 ? iFogError : http_code),effectiveUrl,pMediaStreamContext->fragmentDescriptor.Time);

				pMediaStreamContext->fragmentOffset++; // first byte following packed index

				if (pMediaStreamContext->fragmentIndex != 0)
				{
					unsigned int referenced_size;
					float fragmentDuration;
					AAMPLOG_INFO("%s:%d current fragmentIndex = %d", __FUNCTION__, __LINE__, pMediaStreamContext->fragmentIndex);
					//Find the offset of previous fragment in new representation
					for (int i = 0; i < pMediaStreamContext->fragmentIndex; i++)
					{
						if (ParseSegmentIndexBox(pMediaStreamContext->index_ptr, pMediaStreamContext->index_len, i,
							&referenced_size, &fragmentDuration))
						{
							pMediaStreamContext->fragmentOffset += referenced_size;
						}
					}
				}
			}
			if (pMediaStreamContext->index_ptr)
			{
				unsigned int referenced_size;
				float fragmentDuration;
				if (ParseSegmentIndexBox(pMediaStreamContext->index_ptr, pMediaStreamContext->index_len, pMediaStreamContext->fragmentIndex++, &referenced_size, &fragmentDuration))
				{
					char range[128];
					sprintf(range, "%d-%d", pMediaStreamContext->fragmentOffset, pMediaStreamContext->fragmentOffset + referenced_size - 1);
					AAMPLOG_INFO("%s:%d %s [%s]", __FUNCTION__, __LINE__,mMediaTypeName[pMediaStreamContext->mediaType], range);
					if(!pMediaStreamContext->CacheFragment(fragmentUrl, curlInstance, pMediaStreamContext->fragmentTime, 0.0, range ))
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f", __FUNCTION__, __LINE__, fragmentUrl.c_str(), pMediaStreamContext->fragmentTime);
					}
					pMediaStreamContext->fragmentTime += fragmentDuration;
					pMediaStreamContext->fragmentOffset += referenced_size;
				}
				else
				{ // done with index
					aamp_Free(&pMediaStreamContext->index_ptr);
					pMediaStreamContext->eos = true;
				}
			}
			else
			{
				pMediaStreamContext->eos = true;
			}
		}
		else
		{
			ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
			if (segmentList)
			{
				const std::vector<ISegmentURL*>segmentURLs = segmentList->GetSegmentURLs();
				if (pMediaStreamContext->fragmentIndex >= segmentURLs.size() ||  pMediaStreamContext->fragmentIndex < 0)
				{
					pMediaStreamContext->eos = true;
				}
				else if(!segmentURLs.empty())
				{
					ISegmentURL *segmentURL = segmentURLs.at(pMediaStreamContext->fragmentIndex);

					std::map<string,string> rawAttributes = segmentList->GetRawAttributes();
					if(rawAttributes.find("customlist") == rawAttributes.end()) //"CheckForFogSegmentList")
					{
						std::string fragmentUrl;
						GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor,  segmentURL->GetMediaURI());
						AAMPLOG_INFO("%s [%s]", mMediaTypeName[pMediaStreamContext->mediaType], segmentURL->GetMediaRange().c_str());
						if(!pMediaStreamContext->CacheFragment(fragmentUrl, curlInstance, pMediaStreamContext->fragmentTime, 0.0, segmentURL->GetMediaRange().c_str() ))
						{
							logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f", __FUNCTION__, __LINE__, fragmentUrl.c_str(), pMediaStreamContext->fragmentTime);
						}
					}
					else //We are procesing the custom segment list provided by Fog for DASH TSB
					{
						uint32_t timescale = segmentList->GetTimescale();
						string durationStr = segmentURL->GetRawAttributes().at("d");
						string startTimestr = segmentURL->GetRawAttributes().at("s");
						long long duration = stoll(durationStr);
						long long startTime = stoll(startTimestr);
						if(startTime > pMediaStreamContext->lastSegmentTime || 0 == pMediaStreamContext->lastSegmentTime || rate < 0 )
						{
							/*
								Added to inject appropriate initialization header in
								the case of fog custom mpd
							*/
							if(eMEDIATYPE_VIDEO == pMediaStreamContext->mediaType)
							{
								long long bitrate = 0;
								std::map<string,string> rawAttributes =  segmentURL->GetRawAttributes();
								if(rawAttributes.find("bitrate") == rawAttributes.end()){
									bitrate = pMediaStreamContext->fragmentDescriptor.Bandwidth;
								}else{
									string bitrateStr = rawAttributes["bitrate"];
									bitrate = stoll(bitrateStr);
								}
								if(pMediaStreamContext->fragmentDescriptor.Bandwidth != bitrate || pMediaStreamContext->profileChanged)
								{
									pMediaStreamContext->fragmentDescriptor.Bandwidth = bitrate;
									pMediaStreamContext->profileChanged = true;
									mContext->profileIdxForBandwidthNotification = mBitrateIndexMap[bitrate];
									FetchAndInjectInitialization();
									return false; //Since we need to check WaitForFreeFragmentCache
								}
							}
							double fragmentDuration = (double)duration / timescale;
							pMediaStreamContext->lastSegmentTime = startTime;
							retval = FetchFragment(pMediaStreamContext, segmentURL->GetMediaURI(), fragmentDuration, false, curlInstance);
							if(mContext->mCheckForRampdown)
							{
								/* This case needs to be validated with the segmentList available stream */

								return retval;
							}
						}
						else if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO && duration > 0 && ((pMediaStreamContext->lastSegmentTime - startTime) > TIMELINE_START_RESET_DIFF))
						{
							logprintf("%s:%d START-TIME RESET in TSB period, lastSegmentTime=%" PRIu64 " start-time=%lld duration=%lld", __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentTime, startTime, duration);
							pMediaStreamContext->lastSegmentTime = startTime - 1;
							return retval;
						}
						else
						{
							int index = pMediaStreamContext->fragmentIndex + 1;
							int listSize = segmentURLs.size();

							/*Added this block to reduce the skip overhead for custom mpd after
							 *MPD refresh
							*/
							int nextIndex = ((pMediaStreamContext->lastSegmentTime - startTime) / duration) - 5;
							while(nextIndex > 0 && nextIndex < listSize)
							{
								segmentURL = segmentURLs.at(nextIndex);
								string startTimestr = segmentURL->GetRawAttributes().at("s");
								startTime = stoll(startTimestr);
								if(startTime > pMediaStreamContext->lastSegmentTime)
								{
									nextIndex -= 5;
									continue;
								}
								else
								{
									index = nextIndex;
									break;
								}
							}

							while(startTime < pMediaStreamContext->lastSegmentTime && index < listSize)
							{
								segmentURL = segmentURLs.at(index);
								string startTimestr = segmentURL->GetRawAttributes().at("s");
								startTime = stoll(startTimestr);
								index++;
							}
							pMediaStreamContext->fragmentIndex = index - 1;
							AAMPLOG_TRACE("%s:%d PushNextFragment Exit : startTime %lld lastSegmentTime %lld index = %d", __FUNCTION__, __LINE__, startTime, pMediaStreamContext->lastSegmentTime, pMediaStreamContext->fragmentIndex);
						}
					}
					if(rate > 0)
					{
						pMediaStreamContext->fragmentIndex++;
					}
					else
					{
						pMediaStreamContext->fragmentIndex--;
					}
				}
				else
				{
					logprintf("PrivateStreamAbstractionMPD::%s:%d SegmentUrl is empty", __FUNCTION__, __LINE__);
				}
			}
			else
			{
				AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
			}
		}
	}
	return retval;
}


/**
 * @brief Seek current period by a given time
 * @param seekPositionSeconds seek positon in seconds
 */
void PrivateStreamAbstractionMPD::SeekInPeriod( double seekPositionSeconds)
{
	for (int i = 0; i < mNumberOfTracks; i++)
	{
		SkipFragments(mMediaStreamContext[i], seekPositionSeconds, true);
	}
}



/**
 * @brief Skip to end of track
 * @param pMediaStreamContext Track object pointer
 */
void PrivateStreamAbstractionMPD::SkipToEnd( MediaStreamContext *pMediaStreamContext)
{
	ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
	if (!segmentTemplate)
	{
		segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
	}
	if (segmentTemplate)
	{
		const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
		if (segmentTimeline)
		{
			std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
			uint32_t repeatCount = 0;
			for(int i = 0; i < timelines.size(); i++)
			{
				ITimeline *timeline = timelines.at(i);
				repeatCount += (timeline->GetRepeatCount() + 1);
			}
			pMediaStreamContext->fragmentDescriptor.Number = pMediaStreamContext->fragmentDescriptor.Number + repeatCount - 1;
			pMediaStreamContext->timeLineIndex = timelines.size() - 1;
			pMediaStreamContext->fragmentRepeatCount = timelines.at(pMediaStreamContext->timeLineIndex)->GetRepeatCount();
		}
		else
		{
			double segmentDuration = ((double)segmentTemplate->GetDuration())/segmentTemplate->GetTimescale();
			double startTime = mPeriodStartTime;
			int number = 0;
			while(startTime < mPeriodEndTime)
			{
				startTime += segmentDuration;
				number++;
			}
			pMediaStreamContext->fragmentDescriptor.Number = pMediaStreamContext->fragmentDescriptor.Number + number - 1;
		}
	}
	else
	{
		ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
		if (segmentList)
		{
			const std::vector<ISegmentURL*> segmentURLs = segmentList->GetSegmentURLs();
			pMediaStreamContext->fragmentIndex = segmentURLs.size() - 1;
		}
		else
		{
			AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
		}
	}
}


/**
 * @brief Skip fragments by given time
 * @param pMediaStreamContext Media track object
 * @param skipTime time to skip in seconds
 * @param updateFirstPTS true to update first pts state variable
 * @retval
 */
double PrivateStreamAbstractionMPD::SkipFragments( MediaStreamContext *pMediaStreamContext, double skipTime, bool updateFirstPTS)
{
	ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
	if (!segmentTemplate)
	{
		segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
	}
	if (segmentTemplate)
	{
		 AAMPLOG_INFO("%s:%d Enter : Type[%d] timeLineIndex %d fragmentRepeatCount %d fragmentTime %f skipTime %f segNumber %llu", __FUNCTION__, __LINE__,pMediaStreamContext->type,
                                pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentRepeatCount, pMediaStreamContext->fragmentTime, skipTime, pMediaStreamContext->fragmentDescriptor.Number);

		gboolean firstFrag = true;

		std::string media = segmentTemplate->Getmedia();
		const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
		do
		{
			if (segmentTimeline)
			{
				uint32_t timeScale = segmentTemplate->GetTimescale();
				std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
				if (pMediaStreamContext->timeLineIndex >= timelines.size())
				{
					AAMPLOG_INFO("%s:%d Type[%d] EOS. timeLineIndex[%d] size [%lu]",__FUNCTION__,__LINE__,pMediaStreamContext->type, pMediaStreamContext->timeLineIndex, timelines.size());
					pMediaStreamContext->eos = true;
					break;
				}
				else
				{
					ITimeline *timeline = timelines.at(pMediaStreamContext->timeLineIndex);
					uint32_t repeatCount = timeline->GetRepeatCount();
					if (pMediaStreamContext->fragmentRepeatCount == 0)
					{
						map<string, string> attributeMap = timeline->GetRawAttributes();
						if(attributeMap.find("t") != attributeMap.end())
						{
							uint64_t startTime = timeline->GetStartTime();
							pMediaStreamContext->fragmentDescriptor.Time = startTime;
						}
					}
					uint32_t duration = timeline->GetDuration();
					double fragmentDuration = ((double)duration)/(double)timeScale;
					double nextPTS = (double)(pMediaStreamContext->fragmentDescriptor.Time + duration)/timeScale;
					double firstPTS = (double)pMediaStreamContext->fragmentDescriptor.Time/timeScale;
//					AAMPLOG_TRACE("%s:%d [%s] firstPTS %f nextPTS %f duration %f skipTime %f", __FUNCTION__, __LINE__, pMediaStreamContext->name, firstPTS, nextPTS, fragmentDuration, skipTime);
					if (firstFrag && updateFirstPTS){
						if (pMediaStreamContext->type == eTRACK_AUDIO && (mFirstFragPTS[eTRACK_VIDEO] || mFirstPTS || mVideoPosRemainder)){
							/* need to adjust audio skipTime/seekPosition so 1st audio fragment sent matches start of 1st video fragment being sent */
							double newSkipTime = skipTime + (mFirstFragPTS[eTRACK_VIDEO] - firstPTS); /* adjust for audio/video frag start PTS differences */
							newSkipTime -= mVideoPosRemainder;   /* adjust for mVideoPosRemainder, which is (video seekposition/skipTime - mFirstPTS) */
							newSkipTime += fragmentDuration/4.0; /* adjust for case where video start is near end of current audio fragment by adding to the audio skipTime, pushing it into the next fragment, if close(within this adjustment) */
//							AAMPLOG_INFO("%s:%d newSkiptime %f, skipTime %f  mFirstFragPTS[vid] %f  firstPTS %f  mFirstFragPTS[vid]-firstPTS %f mVideoPosRemainder %f  fragmentDuration/4.0 %f", __FUNCTION__, __LINE__, newSkipTime, skipTime, mFirstFragPTS[eTRACK_VIDEO], firstPTS, mFirstFragPTS[eTRACK_VIDEO]-firstPTS, mVideoPosRemainder,  fragmentDuration/4.0);
							skipTime = newSkipTime;
						}
						firstFrag = false;
						mFirstFragPTS[pMediaStreamContext->mediaType] = firstPTS;
					}
					if (skipTime >= fragmentDuration)
					{
						skipTime -= fragmentDuration;
						pMediaStreamContext->fragmentTime += fragmentDuration;
						pMediaStreamContext->fragmentDescriptor.Time += duration;
						pMediaStreamContext->fragmentDescriptor.Number++;
						pMediaStreamContext->fragmentRepeatCount++;
						if( pMediaStreamContext->fragmentRepeatCount > repeatCount)
						{
							pMediaStreamContext->fragmentRepeatCount= 0;
							pMediaStreamContext->timeLineIndex++;
						}
						continue;  /* continue to next fragment */
					}
					else if (-(skipTime) >= fragmentDuration)
					{
						skipTime += fragmentDuration;
						pMediaStreamContext->fragmentTime -= fragmentDuration;
						pMediaStreamContext->fragmentDescriptor.Time -= duration;
						pMediaStreamContext->fragmentDescriptor.Number--;
						pMediaStreamContext->fragmentRepeatCount--;
						if( pMediaStreamContext->fragmentRepeatCount < 0)
						{
							pMediaStreamContext->timeLineIndex--;
							if(pMediaStreamContext->timeLineIndex >= 0)
							{
								pMediaStreamContext->fragmentRepeatCount = timelines.at(pMediaStreamContext->timeLineIndex)->GetRepeatCount();
							}
						}
						continue;
					}
					if (abs(skipTime) < fragmentDuration)
					{ // last iteration
						AAMPLOG_INFO("%s:%d [%s] firstPTS %f, nextPTS %f  skipTime %f  fragmentDuration %f ", __FUNCTION__, __LINE__, pMediaStreamContext->name, firstPTS, nextPTS, skipTime, fragmentDuration);
						if (updateFirstPTS)
						{
							/*Keep the lower PTS */
							if ( ((mFirstPTS == 0) || (firstPTS < mFirstPTS)) && (pMediaStreamContext->type == eTRACK_VIDEO))
							{
								AAMPLOG_INFO("%s:%d [%s] mFirstPTS %f -> %f ", __FUNCTION__, __LINE__, pMediaStreamContext->name, mFirstPTS, firstPTS);
								mFirstPTS = firstPTS;
								mVideoPosRemainder = skipTime;
								AAMPLOG_INFO("%s:%d [%s] mFirstPTS %f  mVideoPosRemainder %f", __FUNCTION__, __LINE__, pMediaStreamContext->name, mFirstPTS, mVideoPosRemainder);
							}
						}
						skipTime = 0;
						if (pMediaStreamContext->type == eTRACK_AUDIO){
//							AAMPLOG_TRACE("%s audio/video PTS offset %f  audio %f video %f", __FUNCTION__, firstPTS-mFirstPTS, firstPTS, mFirstPTS);
							if (abs(firstPTS - mFirstPTS)> 1.00){
								AAMPLOG_WARN("%s audio/video PTS offset Large %f  audio %f video %f", __FUNCTION__,  firstPTS-mFirstPTS, firstPTS, mFirstPTS);
							}
						}
						break;
					}
				}
			}
			else
			{
				if(0 == pMediaStreamContext->fragmentDescriptor.Time)
				{
					if (rate < 0)
					{
						pMediaStreamContext->fragmentDescriptor.Time = mPeriodEndTime;
					}
					else
					{
						pMediaStreamContext->fragmentDescriptor.Time = mPeriodStartTime;
					}
				}

				if(pMediaStreamContext->fragmentDescriptor.Time > mPeriodEndTime || (rate < 0 && pMediaStreamContext->fragmentDescriptor.Time <= 0))
				{
					AAMPLOG_INFO("%s:%d Type[%d] EOS. fragmentDescriptor.Time=%f",__FUNCTION__,__LINE__,pMediaStreamContext->type, pMediaStreamContext->fragmentDescriptor.Time);
					pMediaStreamContext->eos = true;
					break;
				}
				else
				{
					double segmentDuration = ((double)segmentTemplate->GetDuration())/segmentTemplate->GetTimescale();
					if(!segmentDuration)
					{
						segmentDuration = 2;
					}
					if (skipTime >= segmentDuration)
					{
						pMediaStreamContext->fragmentDescriptor.Number++;
						pMediaStreamContext->fragmentTime += segmentDuration;
						pMediaStreamContext->fragmentDescriptor.Time += segmentDuration;
						pMediaStreamContext->lastSegmentNumber = pMediaStreamContext->fragmentDescriptor.Number;
						skipTime -= segmentDuration;
					}
					else if (-(skipTime) >= segmentDuration)
					{
						pMediaStreamContext->fragmentDescriptor.Number--;
						pMediaStreamContext->fragmentTime -= segmentDuration;
						pMediaStreamContext->fragmentDescriptor.Time -= segmentDuration;
						pMediaStreamContext->lastSegmentNumber = pMediaStreamContext->fragmentDescriptor.Number;
						skipTime += segmentDuration;
					}
					else if(skipTime == 0)
					{
						// Linear or VOD in both the cases if offset is set to 0 then this code will execute.
						// if no offset set then there is back up code in PushNextFragment function
						// which will take care of setting fragmentDescriptor.Time
						// based on live offset(linear) or period start ( vod ) on (pMediaStreamContext->lastSegmentNumber ==0 ) condition
						pMediaStreamContext->lastSegmentNumber = pMediaStreamContext->fragmentDescriptor.Number;
						pMediaStreamContext->fragmentDescriptor.Time = mPeriodStartTime;
						break;
					}
					else if(abs(skipTime) < segmentDuration)
					{
						break;
					}					
				}
			}
		}while(skipTime != 0);

		AAMPLOG_INFO("%s:%d Exit :Type[%d] timeLineIndex %d fragmentRepeatCount %d fragmentDescriptor.Number %" PRIu64 " fragmentTime %f FTime:%f", __FUNCTION__, __LINE__,pMediaStreamContext->type,
				pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentRepeatCount, pMediaStreamContext->fragmentDescriptor.Number, pMediaStreamContext->fragmentTime,pMediaStreamContext->fragmentDescriptor.Time);
	}
	else
	{
		ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
		if (segmentList)
		{
			AAMPLOG_INFO("%s:%d Enter : fragmentIndex %d skipTime %f", __FUNCTION__, __LINE__,
					pMediaStreamContext->fragmentIndex, skipTime);
			const std::vector<ISegmentURL*> segmentURLs = segmentList->GetSegmentURLs();
			double segmentDuration = 0;
			if(!segmentURLs.empty())
			{
				std::map<string,string> rawAttributes = segmentList->GetRawAttributes();
				uint32_t timescale = segmentList->GetTimescale();
				bool isFogTsb = !(rawAttributes.find("customlist") == rawAttributes.end());
				if(!isFogTsb)
				{
					segmentDuration = segmentList->GetDuration() / timescale;
				}
				else if(pMediaStreamContext->type == eTRACK_AUDIO)
				{
					MediaStreamContext *videoContext = mMediaStreamContext[eMEDIATYPE_VIDEO];
					const std::vector<ISegmentURL*> vidSegmentURLs = videoContext->representation->GetSegmentList()->GetSegmentURLs();
					if(!vidSegmentURLs.empty())
					{
						string videoStartStr = vidSegmentURLs.at(0)->GetRawAttributes().at("s");
						string audioStartStr = segmentURLs.at(0)->GetRawAttributes().at("s");
						long long videoStart = stoll(videoStartStr);
						long long audioStart = stoll(audioStartStr);
						long long diff = audioStart - videoStart;
						logprintf("Printing diff value for adjusting %lld",diff);
						if(diff > 0)
						{
							double diffSeconds = double(diff) / timescale;
							skipTime -= diffSeconds;
						}
					}
					else
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d Video SegmentUrl is empty", __FUNCTION__, __LINE__);
					}
				}

				while (skipTime != 0)
				{

					if ((pMediaStreamContext->fragmentIndex >= segmentURLs.size()) || (pMediaStreamContext->fragmentIndex < 0))
					{
						pMediaStreamContext->eos = true;
						break;
					}
					else
					{
						//Calculate the individual segment duration for fog tsb
						if(isFogTsb)
						{
							ISegmentURL* segmentURL = segmentURLs.at(pMediaStreamContext->fragmentIndex);
							string durationStr = segmentURL->GetRawAttributes().at("d");
							long long duration = stoll(durationStr);
							segmentDuration = (double) duration / timescale;
						}
						if (skipTime >= segmentDuration)
						{
							pMediaStreamContext->fragmentIndex++;
							skipTime -= segmentDuration;
							pMediaStreamContext->fragmentTime += segmentDuration;
						}
						else if (-(skipTime) >= segmentDuration)
						{
							pMediaStreamContext->fragmentIndex--;
							skipTime += segmentDuration;
							pMediaStreamContext->fragmentTime -= segmentDuration;
						}
						else
						{
							skipTime = 0;
							break;
						}
					}
				}
			}
			else
			{
				logprintf("PrivateStreamAbstractionMPD::%s:%d SegmentUrl is empty", __FUNCTION__, __LINE__);
			}

			AAMPLOG_INFO("%s:%d Exit : fragmentIndex %d segmentDuration %f", __FUNCTION__, __LINE__,
					pMediaStreamContext->fragmentIndex, segmentDuration);
		}
		else
		{
			AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
		}
	}
	return skipTime;
}


/**
 * @brief Add attriblutes to xml node
 * @param reader xmlTextReaderPtr
 * @param node xml Node
 */
static void AddAttributesToNode(xmlTextReaderPtr *reader, Node *node)
{
	if (xmlTextReaderHasAttributes(*reader))
	{
		while (xmlTextReaderMoveToNextAttribute(*reader))
		{
			std::string key = (const char *)xmlTextReaderConstName(*reader);
			std::string value = (const char *)xmlTextReaderConstValue(*reader);
			node->AddAttribute(key, value);
		}
	}
}



/**
 * @brief Get xml node form reader
 * @param reader pointer to reader object
 * @param url manifest url
 * @retval xml node
*/
AAMPStatusType  PrivateStreamAbstractionMPD::GetMpdFromManfiest(const GrowableBuffer &manifest, MPD * &mpd, std::string manifestUrl, bool init)
{
	AAMPStatusType ret = eAAMPSTATUS_GENERIC_ERROR;
	xmlTextReaderPtr reader = xmlReaderForMemory(manifest.ptr, (int) manifest.len, NULL, NULL, 0);
	if (reader != NULL)
	{
		if (xmlTextReaderRead(reader))
		{
			Node *root = aamp_ProcessNode(&reader, manifestUrl);
			if(root != NULL)
			{
				uint32_t fetchTime = Time::GetCurrentUTCTimeInSec();
				mpd = root->ToMPD();
				if (mpd)
				{
					mpd->SetFetchTime(fetchTime);
#if 1
					FindTimedMetadata(mpd, root, init);
					ret = AAMPStatusType::eAAMPSTATUS_OK;
#else
					size_t prevPrdCnt = mCdaiObject->mAdBreaks.size();
					FindTimedMetadata(mpd, root, init);
					size_t newPrdCnt = mCdaiObject->mAdBreaks.size();
					if(prevPrdCnt < newPrdCnt)
					{
						static int myctr = 0;
						std::string filename = "/tmp/manifest.mpd_" + std::to_string(myctr++);
						WriteFile(filename.c_str(),manifest.ptr, manifest.len);
					}
#endif
				}
				else
				{
				    ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_CONTENT_ERROR;
				}
				delete root;
			}
			else if (root == NULL)
			{
				ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_PARSE_ERROR;
			}
		}
		else if (xmlTextReaderRead(reader) == -1)
		{
			ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_PARSE_ERROR;
		}
		xmlFreeTextReader(reader);
	}
	
	return ret;
}

/**
 * @brief Get xml node form reader
 *
 * @param[in] reader Pointer to reader object
 * @param[in] url    manifest url
 *
 * @retval xml node
 */
Node* aamp_ProcessNode(xmlTextReaderPtr *reader, std::string url, bool isAd)
{
	int type = xmlTextReaderNodeType(*reader);

	if (type != WhiteSpace && type != Text)
	{
		while (type == Comment || type == WhiteSpace)
		{
			xmlTextReaderRead(*reader);
			type = xmlTextReaderNodeType(*reader);
		}

		Node *node = new Node();
		node->SetType(type);
		node->SetMPDPath(Path::GetDirectoryPath(url));

		const char *name = (const char *)xmlTextReaderConstName(*reader);
		if (name == NULL)
		{
			delete node;
			return NULL;
		}

		int         isEmpty = xmlTextReaderIsEmptyElement(*reader);

		node->SetName(name);

		AddAttributesToNode(reader, node);

		if(isAd && !strcmp("Period", name))
		{
			//Making period ids unique. It needs for playing same ad back to back.
			static int UNIQ_PID = 0;
			std::string periodId = std::to_string(UNIQ_PID++) + "-";
			if(node->HasAttribute("id"))
			{
				periodId += node->GetAttributeValue("id");
			}
			node->AddAttribute("id", periodId);
		}

		if (isEmpty)
			return node;

		Node    *subnode = NULL;
		int     ret = xmlTextReaderRead(*reader);

		while (ret == 1)
		{
			if (!strcmp(name, (const char *)xmlTextReaderConstName(*reader)))
			{
				return node;
			}

			subnode = aamp_ProcessNode(reader, url, isAd);

			if (subnode != NULL)
				node->AddSubNode(subnode);

			ret = xmlTextReaderRead(*reader);
		}

		return node;
	}
	else if (type == Text)
	{
		xmlChar * text = xmlTextReaderReadString(*reader);

		if (text != NULL)
		{
			Node *node = new Node();
			node->SetType(type);
			node->SetText((const char*)text);
			xmlFree(text);
			return node;
		}
	}
	return NULL;
}


/**
 *   @brief  Initialize a newly created object.
 *   @note   To be implemented by sub classes
 *   @param  tuneType to set type of object.
 *   @retval true on success
 *   @retval false on failure
 */
AAMPStatusType StreamAbstractionAAMP_MPD::Init(TuneType tuneType)
{
	return mPriv->Init(tuneType);
}

/**
 * @brief Parse date time from ISO8601 string and return value in seconds
 * @param ptr ISO8601 string
 * @retval durationMs duration in milliseconds
 */
static time_t ISO8601DateTimeToUTCSeconds(const char *ptr)
{
	time_t timeSeconds = 0;
	if(ptr)
	{
		time_t offsetFromUTC = 0;
		std::tm timeObj = { 0 };

		//Find out offset from utc by convering epoch
		std::tm baseTimeObj = { 0 };
		strptime("1970-01-01T00:00:00Z", "%Y-%m-%dT%H:%M:%SZ", &baseTimeObj);
		offsetFromUTC = mktime(&baseTimeObj);

		//Convert input string to time
		strptime(ptr, "%Y-%m-%dT%H:%M:%SZ", &timeObj);
		timeSeconds = mktime(&timeObj) - offsetFromUTC;
	}
	return timeSeconds;
}


/**
 * @brief Parse duration from ISO8601 string
 * @param ptr ISO8601 string
 * @param[out] durationMs duration in milliseconds
 */
static void ParseISO8601Duration(const char *ptr, uint64_t &durationMs)
{
	durationMs = 0;
	int hour = 0;
	int minute = 0;
	double seconds = 0;
	if (ptr[0] == 'P' && ptr[1] == 'T')
	{
		ptr += 2;
		const char* temp = strchr(ptr, 'H');
		if (temp)
		{
			sscanf(ptr, "%dH", &hour);
			ptr = temp + 1;
		}
		temp = strchr(ptr, 'M');
		if (temp)
		{
			sscanf(ptr, "%dM", &minute);
			ptr = temp + 1;
		}
		temp = strchr(ptr, 'S');
		if (temp)
		{
			sscanf(ptr, "%lfS", &seconds);
			ptr = temp + 1;
		}
	}
	else
	{
		logprintf("%s:%d - Invalid input %s", __FUNCTION__, __LINE__, ptr);
	}
	durationMs = ((double)(((hour * 60) + minute) * 60 + seconds)) * 1000;
}


/**
 * @brief Parse XML NS
 * @param fullName full name of node
 * @param[out] ns namespace
 * @param[out] name name after :
 */
static void ParseXmlNS(const std::string& fullName, std::string& ns, std::string& name)
{
	size_t found = fullName.find(':');
	if (found != std::string::npos)
	{
		ns = fullName.substr(0, found);
		name = fullName.substr(found+1);
	}
	else
	{
		ns = "";
		name = fullName;
	}
}

#ifdef AAMP_MPD_DRM

/**
 * @brief thread function for create DRM session 
 * which defined in AampDrmSessionManager
 */
extern void *CreateDRMSession(void *arg);

/**
 * @brief Process content protection of adaptation
 * @param adaptationSet Adaptation set object
 * @param mediaType type of track
 */
void PrivateStreamAbstractionMPD::ProcessContentProtection(IAdaptationSet * adaptationSet, MediaType mediaType)
{
	const vector<IDescriptor*> contentProt = adaptationSet->GetContentProtection();
	unsigned char* data   = NULL;
	unsigned char* wvData = NULL;
	unsigned char* prData = NULL;
	unsigned char* ckData = NULL;
	size_t dataLength     = 0;
	size_t wvDataLength   = 0;
	size_t ckDataLength   = 0;
	size_t prDataLength   = 0;
	DRMSystems drmType    = eDRM_NONE;
	unsigned char* contentMetadata = NULL;

	AAMPLOG_TRACE("[HHH]contentProt.size=%d", contentProt.size());
	for (unsigned iContentProt = 0; iContentProt < contentProt.size(); iContentProt++)
	{
		std::string schemeIdUri = contentProt.at(iContentProt)->GetSchemeIdUri();
		if (schemeIdUri.empty())
		{
			AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s:%d type[%d], got schemeID empty at ContentProtection node-%d", __FUNCTION__, __LINE__, mediaType, iContentProt);
			continue;
		}
		//Convert UUID to all lowercase
		std::transform(schemeIdUri.begin(), schemeIdUri.end(), schemeIdUri.begin(), [](unsigned char ch){ return std::tolower(ch); });

		if (schemeIdUri.find(COMCAST_DRM_INFO_ID) != string::npos)
		{
			logprintf("[HHH]Comcast DRM Agnostic CENC system ID found!");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			if (!node.empty())
			{
				string psshData = node.at(0)->GetText();
				data = base64_Decode(psshData.c_str(), &dataLength);

				if(gpGlobalConfig->logging.trace)
				{
					logprintf("content metadata from manifest; length %d", dataLength);
					DumpBlob( data, dataLength );
				}
				if(dataLength != 0)
				{
					int contentMetadataLen = 0;
					contentMetadata = aamp_ExtractWVContentMetadataFromPssh((const char*)data, dataLength, &contentMetadataLen);
					if(gpGlobalConfig->logging.trace)
					{
						logprintf("content metadata from PSSH; length %d", contentMetadataLen);
						DumpBlob( contentMetadata, contentMetadataLen );
					}
				}
				if(data) free(data);
			}
		}
		else if (schemeIdUri.find(WIDEVINE_SYSTEM_ID) != string::npos)
		{
			logprintf("[HHH]Widevine system ID found!");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			if (!node.empty())
			{
				string psshData = node.at(0)->GetText();
				wvData = base64_Decode(psshData.c_str(), &wvDataLength);
				mContext->hasDrm = true;
				if(gpGlobalConfig->logging.trace)
				{
					logprintf("init data from manifest; length %d", wvDataLength);
					DumpBlob(wvData, wvDataLength);
				}
			}
		}
		else if (schemeIdUri.find(PLAYREADY_SYSTEM_ID) != string::npos)
		{
			logprintf("[HHH]Playready system ID found!");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			if (!node.empty())
			{
				string psshData = node.at(0)->GetText();
				prData = base64_Decode(psshData.c_str(), &prDataLength);
				mContext->hasDrm = true;
				if(gpGlobalConfig->logging.trace)
				{
					logprintf("init data from manifest; length %d", prDataLength);
					DumpBlob(prData, prDataLength);
				}
			}
		}
		else if (schemeIdUri.find(CLEARKEY_SYSTEM_ID) != string::npos)
		{
			logprintf("[HHH]ClearKey system ID found!");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			if (!node.empty())
			{
				string psshData = node.at(0)->GetText();
				ckData = base64_Decode(psshData.c_str(), &ckDataLength);
				mContext->hasDrm = true;
				if(gpGlobalConfig->logging.trace)
				{
					logprintf("init data from manifest; length %d", prDataLength);
					DumpBlob(prData, prDataLength);
				}
			}
		}
	}

	if(wvData != NULL && wvDataLength > 0 && ((DRMSystems)gpGlobalConfig->preferredDrm == eDRM_WideVine || prData == NULL))
	{
		drmType = eDRM_WideVine;
		data = wvData;
		dataLength = wvDataLength;

		if(prData)
		{
			free(prData);
		}
		if(ckData)
		{
			free(ckData);
		}
	}
	else if(prData != NULL && prDataLength > 0)
	{
		drmType = eDRM_PlayReady;
		data = prData;
		dataLength = prDataLength;
		if(wvData)
		{
			free(wvData);
		}
		if(ckData)
		{
			free(ckData);
		}	
	}
	else if(ckData != NULL && ckDataLength > 0)
	{
		drmType = eDRM_ClearKey;
		data = ckData;
		dataLength = ckDataLength;
	}

	if(dataLength != 0)
	{
		int keyIdLen = 0;
		unsigned char* keyId = NULL;
		aamp->licenceFromManifest = true;
		keyId = aamp_ExtractKeyIdFromPssh((const char*)data, dataLength, &keyIdLen, drmType);


		if (!(keyIdLen == lastProcessedKeyIdLen && 0 == memcmp(lastProcessedKeyId, keyId, keyIdLen)))
		{
			struct DrmSessionParams* sessionParams = (struct DrmSessionParams*)malloc(sizeof(struct DrmSessionParams));
			sessionParams->initData = data;
			sessionParams->initDataLen = dataLength;
			sessionParams->stream_type = mediaType;
			sessionParams->aamp = aamp;
			sessionParams->drmType = drmType;
			sessionParams->contentMetadata = contentMetadata;

			if(drmSessionThreadStarted) //In the case of license rotation
			{
				void *value_ptr = NULL;
				int rc = pthread_join(createDRMSessionThreadID, &value_ptr);
				if (rc != 0)
				{
					logprintf("pthread_join returned %d for createDRMSession Thread", rc);
				}
				drmSessionThreadStarted = false;
			}
			/*
			* Memory allocated for data via base64_Decode() and memory for sessionParams
			* is released in CreateDRMSession.
			* Memory for keyId allocated in _extractDataFromPssh() is released
			* a. In the else block of this 'if', if it's previously processed keyID
			* b. Assigned to lastProcessedKeyId which is released before new keyID is assigned
			*     or in the distructor of PrivateStreamAbstractionMPD
			*/
			if(0 == pthread_create(&createDRMSessionThreadID,NULL,CreateDRMSession,sessionParams))
			{
				drmSessionThreadStarted = true;
				if(lastProcessedKeyId)
				{
					free(lastProcessedKeyId);
				}
				lastProcessedKeyId =  keyId;
				lastProcessedKeyIdLen = keyIdLen;
				aamp->setCurrentDrm(drmType);
			}
			else
			{
				logprintf("%s %d pthread_create failed for CreateDRMSession : error code %d, %s", __FUNCTION__, __LINE__, errno, strerror(errno));
			}
		}
		else
		{
			if(keyId)
			{
				free(keyId);
			}
			free(data);
		}
	}

}

#else

/**
 * @brief
 * @param adaptationSet
 * @param mediaType
 */
void PrivateStreamAbstractionMPD::ProcessContentProtection(IAdaptationSet * adaptationSet,MediaType mediaType)
{
	logprintf("MPD DRM not enabled");
}
#endif


/**
 *   @brief  GetFirstSegment start time from period
 *   @param  period
 *   @retval start time
 */
uint64_t GetFirstSegmentStartTime(IPeriod * period)
{
	uint64_t startTime = 0;
	const std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
	if (adaptationSets.size() > 0)
	{
		IAdaptationSet * firstAdaptation = adaptationSets.at(0);
		ISegmentTemplate *segmentTemplate = firstAdaptation->GetSegmentTemplate();
		if (!segmentTemplate)
		{
			const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
			if (representations.size() > 0)
			{
				segmentTemplate = representations.at(0)->GetSegmentTemplate();
			}
		}
		if (segmentTemplate)
		{
			const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
			if (segmentTimeline)
			{
				std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
				if(timelines.size() > 0)
				{
					startTime = timelines.at(0)->GetStartTime();
				}
			}
		}
	}
	return startTime;
}


uint64_t aamp_GetPeriodNewContentDuration(IPeriod * period, uint64_t &curEndNumber)
{
	uint64_t durationMs = 0;

	const std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
	if (adaptationSets.size() > 0)
	{
		IAdaptationSet * firstAdaptation = adaptationSets.at(0);
		ISegmentTemplate *segmentTemplate = firstAdaptation->GetSegmentTemplate();
		if (!segmentTemplate)
		{
			const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
			if (representations.size() > 0)
			{
				segmentTemplate = representations.at(0)->GetSegmentTemplate();
			}
		}
		if (segmentTemplate)
		{
			const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
			if (segmentTimeline)
			{
				uint32_t timeScale = segmentTemplate->GetTimescale();
				std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
				uint64_t startNumber = segmentTemplate->GetStartNumber();
				int timeLineIndex = 0;
				while (timeLineIndex < timelines.size())
				{
					ITimeline *timeline = timelines.at(timeLineIndex);
					uint32_t segmentCount = timeline->GetRepeatCount() + 1;
					uint32_t timelineDurationMs = timeline->GetDuration() * 1000 / timeScale;
					for(int i=0;i<segmentCount;i++)
					{
						if(startNumber > curEndNumber)
						{
							durationMs += timelineDurationMs;
							curEndNumber = startNumber;
						}
						startNumber++;
					}
					timeLineIndex++;
				}
			}
		}
	}
	return durationMs;
}

/**
 * @brief Get start time of current period
 * @param mpd : pointer manifest
 * @param periodIndex
 * @retval current period's start time
 */
double PrivateStreamAbstractionMPD::GetPeriodStartTime(IMPD *mpd, int periodIndex)
{
	double periodStart = 0;
	uint64_t  periodStartMs = 0;
	string startTimeStr = mpd->GetPeriods().at(periodIndex)->GetStart();
	if(!startTimeStr.empty())
	{
		ParseISO8601Duration(startTimeStr.c_str(), periodStartMs);
	}
	periodStart =  mAvailabilityStartTime + ((double)periodStartMs / (double)1000);
	AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d - MPD periodStart %lld", __FUNCTION__, __LINE__, periodStart);
	return periodStart;
}

/**
 * @brief Get end time of current period
 * @param mpd : pointer manifest
 * @param periodIndex
 * @param mpdRefreshTime : time when manifest was downloaded
 * @retval current period's end time
 */
double PrivateStreamAbstractionMPD::GetPeriodEndTime(IMPD *mpd, int periodIndex, uint64_t mpdRefreshTime)
{
	uint64_t periodStartMs = 0;
	uint64_t periodDurationMs = 0;
	double periodEndTime = 0;
	double availablilityStart = 0;
	IPeriod *period = mpd->GetPeriods().at(periodIndex);
	string startTimeStr = period->GetStart();
	periodDurationMs = aamp_GetPeriodDuration(mpd, periodIndex, mpdRefreshTime);

	if((0 == mAvailabilityStartTime) && !(mpd->GetType() == "static"))
	{
		AAMPLOG_WARN("%s:%d :  availabilityStartTime required to calculate period duration not present in MPD", __FUNCTION__, __LINE__);
	}
	else if(0 == periodDurationMs)
	{
		AAMPLOG_WARN("%s:%d : Could not get valid period duration to calculate end time", __FUNCTION__, __LINE__);
	}
	else
	{
		if(startTimeStr.empty())
		{
			AAMPLOG_WARN("%s:%d :  Period startTime required to calculate period duration not present in MPD", __FUNCTION__, __LINE__);
		}
		else
		{
			ParseISO8601Duration(startTimeStr.c_str(), periodStartMs);
		}
		periodEndTime = mAvailabilityStartTime + ((double)(periodStartMs + periodDurationMs) /1000);
	}
	AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d - MPD periodEndTime %lld", __FUNCTION__, __LINE__, periodEndTime);
	return periodEndTime;
}


/**
 *   @brief  Get Period Duration
 *   @param  mpd
 *   @param  periodIndex
 *   @retval period duration in milli seconds
  */
uint64_t aamp_GetPeriodDuration(dash::mpd::IMPD *mpd, int periodIndex, uint64_t mpdDownloadTime)
{
	uint64_t durationMs = 0;
	auto periods = mpd->GetPeriods();
	IPeriod * period = periods.at(periodIndex);

	std::string tempString = period->GetDuration();
	if(!tempString.empty())
	{
		ParseISO8601Duration( tempString.c_str(), durationMs);
	}

	if(0 == durationMs)
	{
		const std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
		if (adaptationSets.size() > 0)
		{
			IAdaptationSet * firstAdaptation = adaptationSets.at(0);
			ISegmentTemplate *segmentTemplate = firstAdaptation->GetSegmentTemplate();
			if (!segmentTemplate)
			{
				const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
				if (representations.size() > 0)
				{
					segmentTemplate = representations.at(0)->GetSegmentTemplate();
				}
			}
			if (segmentTemplate)
			{
				const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
				uint32_t timeScale = segmentTemplate->GetTimescale();
				//Calculate period duration by adding up the segment durations in timeline
				if (segmentTimeline)
				{
					std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
					int timeLineIndex = 0;
					while (timeLineIndex < timelines.size())
					{
						ITimeline *timeline = timelines.at(timeLineIndex);
						uint32_t repeatCount = timeline->GetRepeatCount();
						uint32_t timelineDurationMs = timeline->GetDuration() * 1000 / timeScale;
						durationMs += ((repeatCount + 1) * timelineDurationMs);
						AAMPLOG_TRACE("%s timeLineIndex[%d] size [%lu] updated durationMs[%" PRIu64 "]", __FUNCTION__, timeLineIndex, timelines.size(), durationMs);
						timeLineIndex++;
					}
				}
				else
				{
					std::string periodStartStr = period->GetStart();
					if(!periodStartStr.empty())
					{
						//If it's last period find period duration using mpd download time
						//and minimumUpdatePeriod
						std::string durationStr =  mpd->GetMediaPresentationDuration();
						if(!durationStr.empty() && mpd->GetType() == "static")
						{
							uint64_t periodStart = 0;
							uint64_t totalDuration = 0;
							ParseISO8601Duration( periodStartStr.c_str(), periodStart);
							ParseISO8601Duration( durationStr.c_str(), totalDuration);
							durationMs = totalDuration - periodStart;
						}
						else if(periodIndex == (periods.size() - 1))
						{
							std::string minimumUpdatePeriodStr = mpd->GetMinimumUpdatePeriod();
							std::string availabilityStartStr = mpd->GetAvailabilityStarttime();
							std::string publishTimeStr;
							auto attributesMap = mpd->GetRawAttributes();
							if(attributesMap.find("publishTime") != attributesMap.end())
							{
								publishTimeStr = attributesMap["publishTime"];
							}

							if(!publishTimeStr.empty())
							{
								mpdDownloadTime = (uint64_t)ISO8601DateTimeToUTCSeconds(publishTimeStr.c_str()) * 1000;
							}

							if(0 == mpdDownloadTime)
							{
								AAMPLOG_WARN("%s:%d :  mpdDownloadTime required to calculate period duration not provided", __FUNCTION__, __LINE__);
							}
							else if(minimumUpdatePeriodStr.empty())
							{
								AAMPLOG_WARN("%s:%d :  minimumUpdatePeriod required to calculate period duration not present in MPD", __FUNCTION__, __LINE__);
							}
							else if(availabilityStartStr.empty())
							{
								AAMPLOG_WARN("%s:%d :  availabilityStartTime required to calculate period duration not present in MPD", __FUNCTION__, __LINE__);
							}
							else
							{
								uint64_t periodStart = 0;
								uint64_t availablilityStart = 0;
								uint64_t minUpdatePeriod = 0;
								ParseISO8601Duration( periodStartStr.c_str(), periodStart);
								availablilityStart = (uint64_t)ISO8601DateTimeToUTCSeconds(availabilityStartStr.c_str()) * 1000;
								ParseISO8601Duration( minimumUpdatePeriodStr.c_str(), minUpdatePeriod);
								AAMPLOG_INFO("%s:%d : periodStart %llu availabilityStartTime %llu minUpdatePeriod %llu mpdDownloadTime %llu", __FUNCTION__, __LINE__, periodStart, availablilityStart, minUpdatePeriod, mpdDownloadTime);
								uint64_t periodEndTime = mpdDownloadTime + minUpdatePeriod;
								uint64_t periodStartTime = availablilityStart + periodStart;
								durationMs = periodEndTime - periodStartTime;
								if(durationMs <= 0)
								{
									AAMPLOG_WARN("%s:%d : Invalid period duration periodStartTime %llu periodEndTime %llu durationMs %llu", __FUNCTION__, __LINE__, periodStartTime, periodEndTime, durationMs);
									durationMs = 0;
								}
							}
						}
						//We can calculate period duration by subtracting startime from next period start time.
						else
						{
							std::string nextPeriodStartStr = periods.at(periodIndex + 1)->GetStart();
							if(!nextPeriodStartStr.empty())
							{
								uint64_t periodStart = 0;
								uint64_t nextPeriodStart = 0;
								ParseISO8601Duration( periodStartStr.c_str(), periodStart);
								ParseISO8601Duration( nextPeriodStartStr.c_str(), nextPeriodStart);
								durationMs = nextPeriodStart - periodStart;
								if(durationMs <= 0)
								{
									AAMPLOG_WARN("%s:%d : Invalid period duration periodStartTime %llu nextPeriodStart %llu durationMs %llu", __FUNCTION__, __LINE__, periodStart, nextPeriodStart, durationMs);
									durationMs = 0;
								}
							}
							else
							{
								AAMPLOG_WARN("%s:%d : Next period startTime missing periodIndex %d", __FUNCTION__, __LINE__, periodIndex);
							}
						}
					}
					else
					{
						AAMPLOG_WARN("%s:%d : Start time and duration missing in period %s", __FUNCTION__, __LINE__, period->GetId().c_str());
					}
				}
			}
			else
			{
				const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
				if (representations.size() > 0)
				{
					ISegmentList *segmentList = representations.at(0)->GetSegmentList();
					if (segmentList)
					{
						const std::vector<ISegmentURL*> segmentURLs = segmentList->GetSegmentURLs();
						durationMs += (double) segmentList->GetDuration() * 1000 / segmentList->GetTimescale();
					}
					else
					{
						AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
					}
				}
			}
		}
	}
	return durationMs;
}


/**
 *   @brief  Initialize a newly created object.
 *   @note   To be implemented by sub classes
 *   @param  tuneType to set type of object.
 *   @retval true on success
 *   @retval false on failure
 */
AAMPStatusType PrivateStreamAbstractionMPD::Init(TuneType tuneType)
{
	AAMPStatusType retval = eAAMPSTATUS_OK;
	aamp->CurlInit(eCURLINSTANCE_VIDEO, AAMP_TRACK_COUNT,aamp->GetNetworkProxy());
	mCdaiObject->ResetState();

	aamp->mStreamSink->ClearProtectionEvent();
  #ifdef AAMP_MPD_DRM
	AampDRMSessionManager *sessionMgr = aamp->mDRMSessionManager;
	sessionMgr->clearFailedKeyIds();
	sessionMgr->setSessionMgrState(SessionMgrState::eSESSIONMGR_ACTIVE);
  #endif
	aamp->licenceFromManifest = false;
	bool newTune = aamp->IsNewTune();

	aamp->IsTuneTypeNew = false;

#ifdef AAMP_MPD_DRM
	mPushEncInitFragment = newTune || (eTUNETYPE_RETUNE == tuneType);
#endif

	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		aamp->SetCurlTimeout(aamp->mNetworkTimeoutMs, (AampCurlInstance)i);
	}

	AAMPStatusType ret = UpdateMPD(true);
	if (ret == eAAMPSTATUS_OK)
	{
		std::string manifestUrl = aamp->GetManifestUrl();
		int numTracks = (rate == AAMP_NORMAL_PLAY_RATE)?AAMP_TRACK_COUNT:1;
		if (!aamp->IsSubtitleEnabled() && rate == AAMP_NORMAL_PLAY_RATE)
		{
			AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d - subtitles disabled by application", __FUNCTION__, __LINE__);
			numTracks--;
		}
		double offsetFromStart = seekPosition;
		uint64_t durationMs = 0;
		mNumberOfTracks = 0;
		bool mpdDurationAvailable = false;
		std::string tempString = mpd->GetMediaPresentationDuration();
		if(!tempString.empty())
		{
			ParseISO8601Duration( tempString.c_str(), durationMs);
			mpdDurationAvailable = true;
			logprintf("PrivateStreamAbstractionMPD::%s:%d - MPD duration str %s val %" PRIu64 " seconds", __FUNCTION__, __LINE__, tempString.c_str(), durationMs/1000);
		}

		mIsLiveStream = !(mpd->GetType() == "static");
		aamp->SetIsLive(mIsLiveStream);
		map<string, string> mpdAttributes = mpd->GetRawAttributes();
		if(mpdAttributes.find("fogtsb") != mpdAttributes.end())
		{
			mIsFogTSB = true;
			mCdaiObject->mIsFogTSB = true;
		}

		if(mIsLiveStream)
		{
			std::string tempStr = mpd->GetMinimumUpdatePeriod();
			if(!tempStr.empty())
			{
				ParseISO8601Duration( tempStr.c_str(), (uint64_t&)mMinUpdateDurationMs);
			}
			else
			{
				mMinUpdateDurationMs = DEFAULT_INTERVAL_BETWEEN_MPD_UPDATES_MS;
			}

			std::string availabilityStr = mpd->GetAvailabilityStarttime();
			if(!availabilityStr.empty())
			{
				mAvailabilityStartTime = (double)ISO8601DateTimeToUTCSeconds(availabilityStr.c_str());
			}
			logprintf("PrivateStreamAbstractionMPD::%s:%d - AvailabilityStartTime=%f", __FUNCTION__, __LINE__, mAvailabilityStartTime);

			tempStr = mpd->GetTimeShiftBufferDepth();
			uint64_t timeshiftBufferDepthMS = 0;
			if(!tempStr.empty())
			{
				ParseISO8601Duration( tempStr.c_str(), timeshiftBufferDepthMS);
			}
			if(timeshiftBufferDepthMS)
			{
				mTSBDepth = (double)timeshiftBufferDepthMS / 1000;
				// Add valid check for minimum size requirement here
				uint64_t segmentDuration = 0;	
				tempStr = mpd->GetMaxSegmentDuration();
				if(!tempStr.empty())
				{
					ParseISO8601Duration( tempStr.c_str(), (uint64_t&)segmentDuration);
				}
				if(mTSBDepth < ( 4 * (double)segmentDuration))
				{
					mTSBDepth = ( 4 * (double)segmentDuration); 
				}
			}

			tempStr = mpd->GetSuggestedPresentationDelay();
			uint64_t presentationDelay = 0;
			if(!tempStr.empty())
			{
				ParseISO8601Duration( tempStr.c_str(), presentationDelay);
			}
			if(presentationDelay)
			{
				mPresentationOffsetDelay = (double)presentationDelay / 1000;				
			}
			else
			{
				tempStr = mpd->GetMinBufferTime();
				uint64_t minimumBufferTime = 0;
				if(!tempStr.empty())
				{
					ParseISO8601Duration( tempStr.c_str(), minimumBufferTime);
				}
				if(minimumBufferTime)
				{
					mPresentationOffsetDelay = 	(double)minimumBufferTime / 1000;
				}
				else
				{
					mPresentationOffsetDelay = 2.0;
				}
			}

			AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s:%d - MPD minupdateduration val %" PRIu64 " seconds mTSBDepth %f mPresentationOffsetDelay :%f ", __FUNCTION__, __LINE__,  mMinUpdateDurationMs/1000, mTSBDepth,mPresentationOffsetDelay);
		}

		for (int i = 0; i < numTracks; i++)
		{
			mMediaStreamContext[i] = new MediaStreamContext((TrackType)i, mContext, aamp, mMediaTypeName[i]);
			mMediaStreamContext[i]->fragmentDescriptor.manifestUrl = manifestUrl;
			mMediaStreamContext[i]->mediaType = (MediaType)i;
			mMediaStreamContext[i]->representationIndex = -1;
		}

		unsigned int nextPeriodStart = 0;
		double currentPeriodStart = 0;
		double prevPeriodEndMs = 0; // used to find gaps between periods
		size_t numPeriods = mpd->GetPeriods().size();
		bool seekPeriods = true;
		for (unsigned iPeriod = 0; iPeriod < numPeriods; iPeriod++)
		{//TODO -  test with streams having multiple periods.
			IPeriod *period = mpd->GetPeriods().at(iPeriod);
			std::string tempString = period->GetDuration();
			uint64_t  periodStartMs = 0;
			uint64_t periodDurationMs = 0;
			if(!tempString.empty())
			{
				ParseISO8601Duration( tempString.c_str(), periodDurationMs);
				if(!mpdDurationAvailable)
				{
					durationMs += periodDurationMs;
					logprintf("%s:%d - Updated duration %" PRIu64 " seconds", __FUNCTION__, __LINE__, durationMs/1000);
				}
			}
			else if (mIsFogTSB)
			{
				periodDurationMs = aamp_GetPeriodDuration(mpd, iPeriod, mLastPlaylistDownloadTimeMs);
				durationMs += periodDurationMs;
				logprintf("%s:%d - Updated duration %" PRIu64 " seconds", __FUNCTION__, __LINE__, durationMs/1000);
			}

			if(offsetFromStart >= 0 && seekPeriods)
			{
				tempString = period->GetStart();
				if(!tempString.empty() && !mIsFogTSB)
				{
					ParseISO8601Duration( tempString.c_str(), periodStartMs);
				}
				else if (periodDurationMs)
				{
					periodStartMs = nextPeriodStart;
				}

				double periodStartSeconds = (double)periodStartMs/1000;
				double periodDurationSeconds = (double)periodDurationMs / 1000;
				if (periodDurationMs != 0)
				{
					nextPeriodStart += periodDurationMs; // set the value here, nextPeriodStart is used below to identify "Multi period assets with no period duration" if it is set to ZERO.
					double periodEnd = periodStartMs + periodDurationMs;

					// check for gaps between periods
					if(prevPeriodEndMs > 0)
					{
						double periodGap = (periodStartMs - prevPeriodEndMs)/ 1000; // current period start - prev period end will give us GAP between period
						if(periodGap > 0 ) // ohh we have GAP between last and current period
						{
							offsetFromStart -= periodGap; // reduce offset to accomodate gap
							if(offsetFromStart < 0 ) // this means offset is between gap, set to start of currentPeriod
							{
								offsetFromStart = 0;
							}
							AAMPLOG_WARN("%s:%d GAP betwen period found :GAP:%f  mCurrentPeriodIdx %d currentPeriodStart %f offsetFromStart %f", __FUNCTION__, __LINE__,
								periodGap, mCurrentPeriodIdx, periodStartSeconds, offsetFromStart);
						}
					}
					prevPeriodEndMs = periodEnd; // store for future use
					currentPeriodStart = periodStartSeconds;
					mCurrentPeriodIdx = iPeriod;
					if (periodDurationSeconds <= offsetFromStart && iPeriod < (numPeriods - 1))
					{
						offsetFromStart -= periodDurationSeconds;
						logprintf("Skipping period %d seekPosition %f periodEnd %f offsetFromStart %f", iPeriod, seekPosition, periodEnd, offsetFromStart);
						continue;
					}
					else
					{
						seekPeriods = false;
						logprintf("currentPeriodIdx %d/%d", iPeriod, (int)numPeriods);
					}
				}
				else if(periodStartSeconds <= offsetFromStart)
				{
					mCurrentPeriodIdx = iPeriod;
					currentPeriodStart = periodStartSeconds;
				}
			}
		}

		//Check added to update offsetFromStart for
		//Multi period assets with no period duration
		if(0 == nextPeriodStart)
		{
			offsetFromStart -= currentPeriodStart;
		}
		bool segmentTagsPresent = true;
		//The OR condition is added to see if segment info is available in live MPD
		//else we need to calculate fragments based on time
		if (0 == durationMs || (mpdDurationAvailable && mIsLiveStream && !mIsFogTSB))
		{
			durationMs = aamp_GetDurationFromRepresentation(mpd);
			logprintf("%s:%d - Duration after GetDurationFromRepresentation %" PRIu64 " seconds", __FUNCTION__, __LINE__, durationMs/1000);
		}

		if(0 == durationMs)
		{
			segmentTagsPresent = false;
			for(int iPeriod = 0; iPeriod < mpd->GetPeriods().size(); iPeriod++)
			{
				durationMs += aamp_GetPeriodDuration(mpd, iPeriod, mLastPlaylistDownloadTimeMs);
			}
			logprintf("%s:%d - Duration after adding up Period Duration %" PRIu64 " seconds", __FUNCTION__, __LINE__, durationMs/1000);
		}
		/*Do live adjust on live streams on 1. eTUNETYPE_NEW_NORMAL, 2. eTUNETYPE_SEEKTOLIVE,
		 * 3. Seek to a point beyond duration*/
		bool notifyEnteringLive = false;
		if (mIsLiveStream)
		{
			double duration = (double) durationMs / 1000;
			mLiveEndPosition = duration;
			bool liveAdjust = (eTUNETYPE_NEW_NORMAL == tuneType) && aamp->IsLiveAdjustRequired();
			
			if (eTUNETYPE_SEEKTOLIVE == tuneType)
			{
				logprintf("PrivateStreamAbstractionMPD::%s:%d eTUNETYPE_SEEKTOLIVE", __FUNCTION__, __LINE__);
				liveAdjust = true;
				notifyEnteringLive = true;
			}
			else if (((eTUNETYPE_SEEK == tuneType) || (eTUNETYPE_RETUNE == tuneType || eTUNETYPE_NEW_SEEK == tuneType)) && (rate > 0))
			{
				double seekWindowEnd = duration - aamp->mLiveOffset;
				// check if seek beyond live point
				if (seekPosition > seekWindowEnd)
				{
					logprintf( "PrivateStreamAbstractionMPD::%s:%d offSetFromStart[%f] seekWindowEnd[%f]",
							__FUNCTION__, __LINE__, seekPosition, seekWindowEnd);
					liveAdjust = true;
					if (eTUNETYPE_SEEK == tuneType)
					{
						notifyEnteringLive = true;
					}
				}
			}
			if (liveAdjust)
			{
				mCurrentPeriodIdx = mpd->GetPeriods().size() - 1;
				if(aamp->IsLiveAdjustRequired())
				{
					if(segmentTagsPresent)
					{
						duration = (double)(aamp_GetPeriodDuration(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs)) / 1000;
						currentPeriodStart = ((double)durationMs / 1000) - duration;
						offsetFromStart = duration - aamp->mLiveOffset;
						while(offsetFromStart < 0 && mCurrentPeriodIdx > 0)
						{
							AAMPLOG_INFO("%s:%d Adjusting to live offset offsetFromStart %f, mCurrentPeriodIdx %d", __FUNCTION__, __LINE__, offsetFromStart, mCurrentPeriodIdx);
							mCurrentPeriodIdx--;
							duration = (double)(aamp_GetPeriodDuration(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs)) / 1000;
							currentPeriodStart = currentPeriodStart - duration;
							offsetFromStart = offsetFromStart + duration;
						}
					}
					else
					{
						//Calculate live offset based on time elements in the mpd
						double currTime = (double)aamp_GetCurrentTimeMS() / 1000;
						double liveoffset = aamp->mLiveOffset;
						if(mTSBDepth && mTSBDepth < liveoffset)
						{
							liveoffset = mTSBDepth;
						}
						
						double startTime = currTime - liveoffset;
						if(startTime < 0) 
							startTime = 0; 
						currentPeriodStart = ((double)durationMs / 1000);
						while(mCurrentPeriodIdx >= 0)
						{
							mPeriodStartTime =  GetPeriodStartTime(mpd, mCurrentPeriodIdx);
							duration = (double)(aamp_GetPeriodDuration(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs)) / 1000;
							currentPeriodStart -= duration;
							if(mPeriodStartTime < startTime)
							{
								offsetFromStart = startTime - mPeriodStartTime;
								break;
							}
							mCurrentPeriodIdx--;
						}
					}
					AAMPLOG_WARN("%s:%d duration %f durationMs %f mCurrentPeriodIdx %d currentPeriodStart %f offsetFromStart %f", __FUNCTION__, __LINE__,
                                 duration, (double)durationMs / 1000, mCurrentPeriodIdx, currentPeriodStart, offsetFromStart);
				}
				else
				{
					uint64_t  periodStartMs = 0;
					IPeriod *period = mpd->GetPeriods().at(mCurrentPeriodIdx);
					std::string tempString = period->GetStart();
					ParseISO8601Duration( tempString.c_str(), periodStartMs);
					currentPeriodStart = (double)periodStartMs/1000;
					offsetFromStart = duration - aamp->mLiveOffset - currentPeriodStart;
				}

				if (offsetFromStart < 0)
				{
					offsetFromStart = 0;
				}
				mContext->mIsAtLivePoint = true;
				logprintf( "PrivateStreamAbstractionMPD::%s:%d - liveAdjust - Updated offSetFromStart[%f] duration [%f] currentPeriodStart[%f] MaxPeriodIdx[%d]",
						__FUNCTION__, __LINE__, offsetFromStart, duration, currentPeriodStart,mCurrentPeriodIdx);
			}

		}
		else
		{
			// Non-live - VOD/CDVR(Completed) - DELIA-30266
			double seekWindowEnd = (double) durationMs / 1000;
			if(seekPosition > seekWindowEnd)
			{
				for (int i = 0; i < mNumberOfTracks; i++)
				{
					mMediaStreamContext[i]->eosReached=true;
				}
				logprintf("%s:%d seek target out of range, mark EOS. playTarget:%f End:%f. ",
					__FUNCTION__,__LINE__,seekPosition, seekWindowEnd);
				return eAAMPSTATUS_SEEK_RANGE_ERROR;
			}
		}
		mPeriodStartTime =  GetPeriodStartTime(mpd, mCurrentPeriodIdx);
		mPeriodEndTime = GetPeriodEndTime(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs);
		mCurrentPeriod = mpd->GetPeriods().at(mCurrentPeriodIdx);
		mBasePeriodId = mCurrentPeriod->GetId();
		mBasePeriodOffset = offsetFromStart;

		onAdEvent(AdEvent::INIT, offsetFromStart);

		UpdateLanguageList();
		StreamSelection(true);

		if(mAudioType == eAUDIO_UNSUPPORTED)
		{
			retval = eAAMPSTATUS_MANIFEST_CONTENT_ERROR;
			aamp->SendErrorEvent(AAMP_TUNE_UNSUPPORTED_AUDIO_TYPE);
		}
		else if(mNumberOfTracks)
		{
			aamp->SendEventAsync(AAMP_EVENT_PLAYLIST_INDEXED);
			TunedEventConfig tunedEventConfig =  mIsLiveStream ?
					aamp->mTuneEventConfigLive : aamp->mTuneEventConfigVod;
			if (eTUNED_EVENT_ON_PLAYLIST_INDEXED == tunedEventConfig)
			{
				if (aamp->SendTunedEvent())
				{
					logprintf("aamp: mpd - sent tune event after indexing playlist");
				}
			}
			ret = UpdateTrackInfo(!newTune, true, true);

			if(eAAMPSTATUS_OK != ret)
			{
				if (ret == eAAMPSTATUS_MANIFEST_CONTENT_ERROR)
				{
					AAMPLOG_ERR("%s:%d ERROR: No playable profiles found", __FUNCTION__, __LINE__);
				}
				return ret;
			}

			if(notifyEnteringLive)
			{
				aamp->NotifyOnEnteringLive();
			}
			SeekInPeriod( offsetFromStart);
			seekPosition = mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentTime;
			if(0 != mCurrentPeriodIdx)
			{
				seekPosition += currentPeriodStart;
			}
			for (int i = 0; i < mNumberOfTracks; i++)
			{
				if(0 != mCurrentPeriodIdx)
				{
					mMediaStreamContext[i]->fragmentTime = seekPosition;
				}
				mMediaStreamContext[i]->periodStartOffset = currentPeriodStart;
			}
			AAMPLOG_INFO("%s:%d offsetFromStart(%f) seekPosition(%f) currentPeriodStart(%f)",__FUNCTION__,__LINE__, offsetFromStart,seekPosition, currentPeriodStart);
			if (newTune )
			{
				std::vector<long> bitrateList;
				bitrateList.reserve(GetProfileCount());

				for (int i = 0; i < GetProfileCount(); i++)
				{
					if (!mStreamInfo[i].isIframeTrack)
					{
						bitrateList.push_back(mStreamInfo[i].bandwidthBitsPerSecond);
					}
				}

				aamp->SetState(eSTATE_PREPARING);
				//For DASH, presence of iframe track depends on current period.
				aamp->SendMediaMetadataEvent(durationMs, mLangList, bitrateList, mContext->hasDrm, aamp->mIsIframeTrackPresent);

				aamp->UpdateDuration(((double)durationMs)/1000);
				GetCulledSeconds();
				aamp->UpdateRefreshPlaylistInterval((float)mMinUpdateDurationMs / 1000);
			}
		}
		else
		{
			logprintf("No adaptation sets could be selected");
			retval = eAAMPSTATUS_MANIFEST_CONTENT_ERROR;
		}
	}
	else
	{
		AAMPLOG_ERR("PrivateStreamAbstractionMPD::%s:%d corrupt/invalid manifest",__FUNCTION__,__LINE__);
		retval = eAAMPSTATUS_MANIFEST_PARSE_ERROR;
	}
	return retval;
}


/**
 * @brief Get duration though representation iteration
 * @retval duration in milliseconds
 */
uint64_t aamp_GetDurationFromRepresentation(dash::mpd::IMPD *mpd)
{
	uint64_t durationMs = 0;
	size_t numPeriods = mpd->GetPeriods().size();

	for (unsigned iPeriod = 0; iPeriod < numPeriods; iPeriod++)
	{
		IPeriod *period = mpd->GetPeriods().at(iPeriod);
		const std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
		if (adaptationSets.size() > 0)
		{
			IAdaptationSet * firstAdaptation = adaptationSets.at(0);
			ISegmentTemplate *segmentTemplate = firstAdaptation->GetSegmentTemplate();
			if (!segmentTemplate)
			{
				const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
				if (representations.size() > 0)
				{
					segmentTemplate = representations.at(0)->GetSegmentTemplate();
				}
			}
			if (segmentTemplate)
			{
				std::string media = segmentTemplate->Getmedia();
				const ISegmentTimeline *segmentTimeline = segmentTemplate->GetSegmentTimeline();
				if (segmentTimeline)
				{
					std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
					uint32_t timeScale = segmentTemplate->GetTimescale();
					int timeLineIndex = 0;
					while (timeLineIndex < timelines.size())
					{
						ITimeline *timeline = timelines.at(timeLineIndex);
						uint32_t repeatCount = timeline->GetRepeatCount();
						uint32_t timelineDurationMs = timeline->GetDuration() * 1000 / timeScale;
						durationMs += ((repeatCount + 1) * timelineDurationMs);
						traceprintf("%s period[%d] timeLineIndex[%d] size [%lu] updated durationMs[%" PRIu64 "]", __FUNCTION__, iPeriod, timeLineIndex, timelines.size(), durationMs);
						timeLineIndex++;
					}
				}
			}
			else
			{
				const std::vector<IRepresentation *> representations = firstAdaptation->GetRepresentation();
				if (representations.size() > 0)
				{
					ISegmentList *segmentList = representations.at(0)->GetSegmentList();
					if (segmentList)
					{
						const std::vector<ISegmentURL*> segmentURLs = segmentList->GetSegmentURLs();
						durationMs += (double) segmentList->GetDuration() * 1000 / segmentList->GetTimescale();
					}
					else
					{
						AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
					}
				}
			}
		}
	}
	return durationMs;
}


/**
 * @brief Update MPD manifest
 * @param retrievePlaylistFromCache true to try to get from cache
 * @retval true on success
 */
AAMPStatusType PrivateStreamAbstractionMPD::UpdateMPD(bool init)
{
	GrowableBuffer manifest;
	AAMPStatusType ret = AAMPStatusType::eAAMPSTATUS_OK;
	std::string manifestUrl = aamp->GetManifestUrl();
	
	// take the original url before it gets changed in GetFile
	std::string origManifestUrl = manifestUrl;
	bool gotManifest = false;
	bool retrievedPlaylistFromCache = false;
	memset(&manifest, 0, sizeof(manifest));
	if (aamp->getAampCacheHandler()->RetrieveFromPlaylistCache(manifestUrl, &manifest, manifestUrl))
	{
		logprintf("PrivateStreamAbstractionMPD::%s:%d manifest retrieved from cache", __FUNCTION__, __LINE__);
		retrievedPlaylistFromCache = true;
	}
	if (!retrievedPlaylistFromCache)
	{
		long http_error = 0;
		memset(&manifest, 0, sizeof(manifest));
		aamp->profiler.ProfileBegin(PROFILE_BUCKET_MANIFEST);
		aamp->SetCurlTimeout(aamp->mManifestTimeoutMs,eCURLINSTANCE_VIDEO);
		gotManifest = aamp->GetFile(manifestUrl, &manifest, manifestUrl, &http_error, NULL, eCURLINSTANCE_VIDEO, true, eMEDIATYPE_MANIFEST);
		aamp->SetCurlTimeout(aamp->mNetworkTimeoutMs,eCURLINSTANCE_VIDEO);

		//update videoend info
		aamp->UpdateVideoEndMetrics(eMEDIATYPE_MANIFEST,0,http_error,manifestUrl);

		if (gotManifest)
		{
			aamp->mManifestUrl = manifestUrl;
			aamp->profiler.ProfileEnd(PROFILE_BUCKET_MANIFEST);
			if (mContext->mNetworkDownDetected)
			{
				mContext->mNetworkDownDetected = false;
			}
		}
		else if (aamp->DownloadsAreEnabled())
		{
			aamp->profiler.ProfileError(PROFILE_BUCKET_MANIFEST, http_error);
			if (this->mpd != NULL && (CURLE_OPERATION_TIMEDOUT == http_error || CURLE_COULDNT_CONNECT == http_error))
			{
				//Skip this for first ever update mpd request
				mContext->mNetworkDownDetected = true;
				logprintf("PrivateStreamAbstractionMPD::%s Ignore curl timeout", __FUNCTION__);
				ret = AAMPStatusType::eAAMPSTATUS_OK;
			}
			else
			{
				aamp->SendDownloadErrorEvent(AAMP_TUNE_MANIFEST_REQ_FAILED, http_error);
				logprintf("PrivateStreamAbstractionMPD::%s - manifest download failed", __FUNCTION__);
				ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_DOWNLOAD_ERROR;
			}
		}
	}
	else
	{
		gotManifest = true;
	}

	if (gotManifest)
	{
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
		std::string fileName;
                if(gpGlobalConfig->harvestpath)
                        fileName = gpGlobalConfig->harvestpath;
                else
                        fileName = HARVEST_BASE_PATH;

		fileName.append("manifest.mpd");
		WriteFile( fileName, manifest.ptr, manifest.len);
#endif

//Enable to harvest MPD file
//Save the last 3 MPDs
#ifdef HARVEST_MPD
		static int counter = 0;
		string fileSuffix = to_string(counter % 3);
		counter++;
		string fullPath = "/opt/logs/ProcessNodeError.txt" + fileSuffix;
		logprintf("Saving manifest to %s",fullPath.c_str());
		FILE *outputFile = fopen(fullPath.c_str(), "w");
		fwrite(manifest.ptr, manifest.len, 1, outputFile);
		fprintf(outputFile,"EndofManifest");
		fclose(outputFile);
#endif

		MPD* mpd = nullptr;
		ret = GetMpdFromManfiest(manifest, mpd, manifestUrl, init);
		AAMPLOG_INFO("%s:%d Created MPD[%p]", __FUNCTION__, __LINE__, mpd);
		if (eAAMPSTATUS_OK == ret)
		{
			if (this->mpd)
			{
				delete this->mpd;
			}
			this->mpd = mpd;
			mIsLiveManifest = !(mpd->GetType() == "static");
			if (!retrievedPlaylistFromCache && !mIsLiveManifest)
			{
				aamp->getAampCacheHandler()->InsertToPlaylistCache(origManifestUrl, &manifest, aamp->GetManifestUrl(), mIsLiveStream,eMEDIATYPE_MANIFEST);
			}
		}
		else
		{
			logprintf("%s:%d Error while processing MPD, GetMpdFromManfiest returned %d", __FUNCTION__, __LINE__, ret);
			retrievedPlaylistFromCache = false;
		}
		aamp_Free(&manifest.ptr);
		mLastPlaylistDownloadTimeMs = aamp_GetCurrentTimeMS();
		if(mIsLiveStream && gpGlobalConfig->enableClientDai)
		{
			mCdaiObject->PlaceAds(mpd);
		}
	}
	else if (AAMPStatusType::eAAMPSTATUS_OK != ret)
	{
		logprintf("aamp: error on manifest fetch");
	}

	if( ret == eAAMPSTATUS_MANIFEST_PARSE_ERROR || ret == eAAMPSTATUS_MANIFEST_CONTENT_ERROR)
	{
	    if(NULL != manifest.ptr && !manifestUrl.empty())
	    {
            int tempDataLen = (MANIFEST_TEMP_DATA_LENGTH - 1);
            char temp[MANIFEST_TEMP_DATA_LENGTH];
            strncpy(temp, manifest.ptr, tempDataLen);
            temp[tempDataLen] = 0x00;
	        logprintf("ERROR: Invalid Playlist URL: %s ret:%d", manifestUrl.c_str(),ret);
	        logprintf("ERROR: Invalid Playlist DATA: %s ", temp);
	    }
        aamp->SendErrorEvent(AAMP_TUNE_INVALID_MANIFEST_FAILURE);
	}

	return ret;
}


/**
 * @brief Find timed metadata from mainifest
 * @param mpd MPD top level element
 * @param root XML root node
 */
void PrivateStreamAbstractionMPD::FindTimedMetadata(MPD* mpd, Node* root, bool init)
{
	std::vector<Node*> subNodes = root->GetSubNodes();

	uint64_t periodStartMS = 0;
	uint64_t periodDurationMS = 0;

	std::vector<std::string> newPeriods;
	// Iterate through each of the MPD's Period nodes, and ProgrameInformation.
	int periodCnt = 0;
	for (size_t i=0; i < subNodes.size(); i++) {
		Node* node = subNodes.at(i);
		const std::string& name = node->GetName();
		if (name == "Period") {
			std::string AdID;
			std::string AssetID;
			std::string ProviderID;
			periodCnt++;

			// Calculate period start time and duration
			periodStartMS += periodDurationMS;
			if (node->HasAttribute("start")) {
				const std::string& value = node->GetAttributeValue("start");
				uint64_t valueMS = 0;
				if (!value.empty())
					ParseISO8601Duration(value.c_str(), valueMS);
				if (periodStartMS < valueMS)
					periodStartMS = valueMS;
			}
			periodDurationMS = 0;
			if (node->HasAttribute("duration")) {
				const std::string& value = node->GetAttributeValue("duration");
				uint64_t valueMS = 0;
				if (!value.empty())
					ParseISO8601Duration(value.c_str(), valueMS);
				periodDurationMS = valueMS;
			}

			IPeriod * period = mpd->GetPeriods().at(periodCnt-1);
			std::string adBreakId("");
			const std::string &prdId = period->GetId();
			// Iterate through children looking for SupplementProperty nodes
			std::vector<Node*> children = node->GetSubNodes();
			for (size_t j=0; j < children.size(); j++) {
				Node* child = children.at(j);
				const std::string& name = child->GetName();
				if (name == "SupplementalProperty") {
					ProcessPeriodSupplementalProperty(child, AdID, periodStartMS, periodDurationMS , init);
					continue;
				}
				if (name == "AssetIdentifier") {
					ProcessPeriodAssetIdentifier(child, periodStartMS, periodDurationMS, AssetID, ProviderID,init);
					continue;
				}
				if(name == "EventStream" && "" != prdId && !(mCdaiObject->isPeriodExist(prdId))
				   && (!init || (1 < periodCnt && 0 == period->GetAdaptationSets().size())))    //Take last & empty period at the MPD init AND all new periods in the MPD refresh. (No empty periods will come the middle)
				{
					mCdaiObject->InsertToPeriodMap(period);	//Need to do it. Because the FulFill may finish quickly
					ProcessEventStream(periodStartMS, period);
					continue;
				}
			}
			if("" != prdId)
			{
				mCdaiObject->InsertToPeriodMap(period);
				newPeriods.emplace_back(prdId);
			}
			continue;
		}
		if (name == "ProgramInformation") {
			std::vector<Node*> infoNodes = node->GetSubNodes();
			for (size_t i=0; i < infoNodes.size(); i++) {
				Node* infoNode = infoNodes.at(i);
				std::string name;
				std::string ns;
				ParseXmlNS(infoNode->GetName(), ns, name);
				const std::string& infoNodeType = infoNode->GetAttributeValue("type");
				if (name == "ContentIdentifier" && (infoNodeType == "URI" || infoNodeType == "URN")) {
					if (infoNode->HasAttribute("value")) {
						const std::string& content = infoNode->GetAttributeValue("value");

						AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-CONTENT-IDENTIFIER:%s", 0.0f, content.c_str());

						for (int i = 0; i < aamp->subscribedTags.size(); i++)
						{
							const std::string& tag = aamp->subscribedTags.at(i);
							if (tag == "#EXT-X-CONTENT-IDENTIFIER") {
								aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size(),init);
								break;
							}
						}
					}
					continue;
				}
			}
			continue;
		}
		if (name == "SupplementalProperty" && node->HasAttribute("schemeIdUri")) {
			const std::string& schemeIdUri = node->GetAttributeValue("schemeIdUri");
			if (schemeIdUri == "urn:comcast:dai:2018" && node->HasAttribute("id")) {
				const std::string& ID = node->GetAttributeValue("id");
				if (ID == "identityADS" && node->HasAttribute("value")) {
					const std::string& content = node->GetAttributeValue("value");

					AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-IDENTITY-ADS:%s", 0.0f, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-IDENTITY-ADS") {
							aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size(),init);
							break;
						}
					}
					continue;
				}
				if (ID == "messageRef" && node->HasAttribute("value")) {
					const std::string& content = node->GetAttributeValue("value");

					AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-MESSAGE-REF:%s", 0.0f, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-MESSAGE-REF") {
							aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size(),init);
							break;
						}
					}
					continue;
				}
			}
			continue;
		}
	}
	mCdaiObject->PrunePeriodMaps(newPeriods);
}


/**
 * @brief Process supplemental property of a period
 * @param node SupplementalProperty node
 * @param[out] AdID AD Id
 * @param startMS start time in MS
 * @param durationMS duration in MS
 */
void PrivateStreamAbstractionMPD::ProcessPeriodSupplementalProperty(Node* node, std::string& AdID, uint64_t startMS, uint64_t durationMS, bool isInit)
{
	if (node->HasAttribute("schemeIdUri")) {
		const std::string& schemeIdUri = node->GetAttributeValue("schemeIdUri");
		if ((schemeIdUri == "urn:comcast:dai:2018") && node->HasAttribute("id")) {
			const std::string& ID = node->GetAttributeValue("id");
			if ((ID == "Tracking") && node->HasAttribute("value")) {
				const std::string& value = node->GetAttributeValue("value");
				if (!value.empty()) {
					AdID = value;

					// Check if we need to make AdID a quoted-string
					if (AdID.find(',') != std::string::npos) {
						AdID="\"" + AdID + "\"";
					}

					double duration = durationMS / 1000.0f;
					double start = startMS / 1000.0f;

					std::ostringstream s;
					s << "ID=" << AdID;
					s << ",DURATION=" << std::fixed << std::setprecision(3) << duration;
					s << ",PSN=true";

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-CUE:%s", start, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-CUE") {
							aamp->ReportTimedMetadata((long long)startMS, tag.c_str(), content.c_str(), content.size() , isInit);
							break;
						}
					}
				}
			}
		}
		else if (!AdID.empty() && (schemeIdUri == "urn:scte:scte130-10:2014")) {
			std::vector<Node*> children = node->GetSubNodes();
			for (size_t i=0; i < children.size(); i++) {
				Node* child = children.at(i);
				std::string name;
				std::string ns;
				ParseXmlNS(child->GetName(), ns, name);
				if (name == "StreamRestrictionListType") {
					ProcessStreamRestrictionList(child, AdID, startMS);
					continue;
				}
				if (name == "StreamRestrictionList") {
					ProcessStreamRestrictionList(child, AdID, startMS);
					continue;
				}
				if (name == "StreamRestriction") {
					ProcessStreamRestriction(child, AdID, startMS);
					continue;
				}
			}
		}
	}
}


/**
 * @brief Process Period AssetIdentifier
 * @param node AssetIdentifier node
 * @param startMS start time MS
 * @param durationMS duration MS
 * @param AssetID Asset Id
 * @param ProviderID Provider Id
 */
void PrivateStreamAbstractionMPD::ProcessPeriodAssetIdentifier(Node* node, uint64_t startMS, uint64_t durationMS, std::string& AssetID, std::string& ProviderID,bool isInit)
{
	if (node->HasAttribute("schemeIdUri")) {
		const std::string& schemeIdUri = node->GetAttributeValue("schemeIdUri");
		if ((schemeIdUri == "urn:cablelabs:md:xsd:content:3.0") && node->HasAttribute("value")) {
			const std::string& value = node->GetAttributeValue("value");
			if (!value.empty()) {
				size_t pos = value.find("/Asset/");
				if (pos != std::string::npos) {
					std::string assetID = value.substr(pos+7);
					std::string providerID = value.substr(0, pos);
					double duration = durationMS / 1000.0f;
					double start = startMS / 1000.0f;

					AssetID = assetID;
					ProviderID = providerID;

					// Check if we need to make assetID a quoted-string
					if (assetID.find(',') != std::string::npos) {
						assetID="\"" + assetID + "\"";
					}

					// Check if we need to make providerID a quoted-string
					if (providerID.find(',') != std::string::npos) {
						providerID="\"" + providerID + "\"";
					}

					std::ostringstream s;
					s << "ID=" << assetID;
					s << ",PROVIDER=" << providerID;
					s << ",DURATION=" << std::fixed << std::setprecision(3) << duration;

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-ASSET-ID:%s", start, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-ASSET-ID") {
							aamp->ReportTimedMetadata((long long)startMS, tag.c_str(), content.c_str(), content.size(),isInit);
							break;
						}
					}
				}
			}
		}
	}
}


bool PrivateStreamAbstractionMPD::ProcessEventStream(uint64_t startMS, IPeriod * period)
{
	bool ret = false;
	if(!(gpGlobalConfig->enableClientDai))
	{
		return ret;
	}
	const std::string &prdId = period->GetId();
	if(!prdId.empty())
	{
		uint32_t breakdur = 0;
		uint64_t startMS1 = 0;
		std::string scte35;
		if(isAdbreakStart(period, breakdur, startMS1, scte35))
		{
			//Found an Ad break.
			aamp->FoundSCTE35(prdId, startMS, breakdur, scte35);
			ret = true;
		}
	}
	return ret;
}

/**
 * @brief Process Stream restriction list
 * @param node StreamRestrictionListType node
 * @param AdID Ad Id
 * @param startMS start time MS
 */
void PrivateStreamAbstractionMPD::ProcessStreamRestrictionList(Node* node, const std::string& AdID, uint64_t startMS)
{
	std::vector<Node*> children = node->GetSubNodes();
	for (size_t i=0; i < children.size(); i++) {
		Node* child = children.at(i);
		std::string name;
		std::string ns;
		ParseXmlNS(child->GetName(), ns, name);
		if (name == "StreamRestriction") {
			ProcessStreamRestriction(child, AdID, startMS);
			continue;
		}
	}
}


/**
 * @brief Process stream restriction
 * @param node StreamRestriction xml node
 * @param AdID Ad ID
 * @param startMS Start time in MS
 */
void PrivateStreamAbstractionMPD::ProcessStreamRestriction(Node* node, const std::string& AdID, uint64_t startMS)
{
	std::vector<Node*> children = node->GetSubNodes();
	for (size_t i=0; i < children.size(); i++) {
		Node* child = children.at(i);
		std::string name;
		std::string ns;
		ParseXmlNS(child->GetName(), ns, name);
		if (name == "Ext") {
			ProcessStreamRestrictionExt(child, AdID, startMS);
			continue;
		}
	}
}


/**
 * @brief Process stream restriction extension
 * @param node Ext child of StreamRestriction xml node
 * @param AdID Ad ID
 * @param startMS start time in ms
 */
void PrivateStreamAbstractionMPD::ProcessStreamRestrictionExt(Node* node, const std::string& AdID, uint64_t startMS)
{
	std::vector<Node*> children = node->GetSubNodes();
	for (size_t i=0; i < children.size(); i++) {
		Node* child = children.at(i);
		std::string name;
		std::string ns;
		ParseXmlNS(child->GetName(), ns, name);
		if (name == "TrickModeRestriction") {
			ProcessTrickModeRestriction(child, AdID, startMS);
			continue;
		}
	}
}


/**
 * @brief Process trick mode restriction
 * @param node TrickModeRestriction xml node
 * @param AdID Ad ID
 * @param startMS start time in ms
 */
void PrivateStreamAbstractionMPD::ProcessTrickModeRestriction(Node* node, const std::string& AdID, uint64_t startMS)
{
	double start = startMS / 1000.0f;

	std::string trickMode;
	if (node->HasAttribute("trickMode")) {
		trickMode = node->GetAttributeValue("trickMode");
	}

	std::string scale;
	if (node->HasAttribute("scale")) {
		scale = node->GetAttributeValue("scale");
	}

	std::string text = node->GetText();
	if (!trickMode.empty() && !text.empty()) {
		std::ostringstream s;
		s << "ADID=" << AdID
		  << ",MODE=" << trickMode
		  << ",LIMIT=" << text;

		if (!scale.empty()) {
			s << ",SCALE=" << scale;
		}

		std::string content = s.str();
		AAMPLOG_INFO("TimedMetadata: @%1.3f #EXT-X-TRICKMODE-RESTRICTION:%s", start, content.c_str());

		for (int i = 0; i < aamp->subscribedTags.size(); i++)
		{
			const std::string& tag = aamp->subscribedTags.at(i);
			if (tag == "#EXT-X-TRICKMODE-RESTRICTION") {
				aamp->ReportTimedMetadata((long long)startMS, tag.c_str(), content.c_str(), content.size());
				break;
			}
		}
	}
}


/**
 * @brief Fragment downloader thread
 * @param arg HeaderFetchParams pointer
 */
void * TrackDownloader(void *arg)
{
	struct HeaderFetchParams* fetchParms = (struct HeaderFetchParams*)arg;
	if(aamp_pthread_setname(pthread_self(), "aampFetchInit"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed", __FUNCTION__, __LINE__);
	}
	//Calling WaitForFreeFragmentAvailable timeout as 0 since waiting for one tracks
	//init header fetch can slow down fragment downloads for other track
	if(fetchParms->pMediaStreamContext->WaitForFreeFragmentAvailable(0))
	{
		fetchParms->pMediaStreamContext->profileChanged = false;
		fetchParms->context->FetchFragment(fetchParms->pMediaStreamContext,
				fetchParms->initialization,
				fetchParms->fragmentduration,
				fetchParms->isinitialization, (eMEDIATYPE_AUDIO == fetchParms->pMediaStreamContext->mediaType), //CurlContext 0=Video, 1=Audio)
				fetchParms->discontinuity);
		fetchParms->pMediaStreamContext->discontinuity = false;
	}
	return NULL;
}


/**
 * @brief Fragment downloader thread
 * @param arg Pointer to FragmentDownloadParams  object
 * @retval NULL
 */
void * FragmentDownloader(void *arg)
{
	struct FragmentDownloadParams* downloadParams = (struct FragmentDownloadParams*) arg;
	if(aamp_pthread_setname(pthread_self(), "aampFragDown"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed", __FUNCTION__, __LINE__);
	}
	if (downloadParams->pMediaStreamContext->adaptationSet)
	{
		while (downloadParams->context->aamp->DownloadsAreEnabled() && !downloadParams->pMediaStreamContext->profileChanged)
		{
			int timeoutMs = downloadParams->context->GetMinUpdateDuration() - (int)(aamp_GetCurrentTimeMS() - downloadParams->lastPlaylistUpdateMS);
			if(downloadParams->pMediaStreamContext->WaitForFreeFragmentAvailable(timeoutMs))
			{
				downloadParams->context->PushNextFragment(downloadParams->pMediaStreamContext, 1);
				if (downloadParams->pMediaStreamContext->eos)
				{
					if(!downloadParams->context->aamp->IsLive() && downloadParams->playingLastPeriod)
					{
						downloadParams->pMediaStreamContext->eosReached = true;
						downloadParams->pMediaStreamContext->AbortWaitForCachedAndFreeFragment(false);
					}
					AAMPLOG_INFO("%s:%d %s EOS - Exit fetch loop", __FUNCTION__, __LINE__, downloadParams->pMediaStreamContext->name);
					break;
				}
			}
			timeoutMs = downloadParams->context->GetMinUpdateDuration() - (int)(aamp_GetCurrentTimeMS() - downloadParams->lastPlaylistUpdateMS);
			if(timeoutMs <= 0 && downloadParams->context->aamp->IsLive())
			{
				break;
			}
		}
	}
	else
	{
		logprintf("%s:%d NULL adaptationSet", __FUNCTION__, __LINE__);
	}
	return NULL;
}

/**
 * @brief Fragment collector thread
 * @param arg Pointer to PrivateStreamAbstractionMPD object
 * @retval NULL
 */
static void * FragmentCollector(void *arg)
{
	PrivateStreamAbstractionMPD *context = (PrivateStreamAbstractionMPD *)arg;
	if(aamp_pthread_setname(pthread_self(), "aampMPDFetch"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed", __FUNCTION__, __LINE__);
	}
	context->FetcherLoop();
	return NULL;
}


/**
 * @brief Check if adaptation set is iframe track
 * @param adaptationSet Pointer to adaptainSet
 * @retval true if iframe track
 */
static bool IsIframeTrack(IAdaptationSet *adaptationSet)
{
	const std::vector<INode *> subnodes = adaptationSet->GetAdditionalSubNodes();
	for (unsigned i = 0; i < subnodes.size(); i++)
	{
		INode *xml = subnodes[i];
		if (xml->GetName() == "EssentialProperty")
		{
			if (xml->HasAttribute("schemeIdUri"))
			{
				const std::string& schemeUri = xml->GetAttributeValue("schemeIdUri");
				if (schemeUri == "http://dashif.org/guidelines/trickmode")
				{
					return true;
				}
				else
				{
					logprintf("%s:%d - skipping schemeUri %s", __FUNCTION__, __LINE__, schemeUri.c_str());
				}
			}
		}
		else
		{
			logprintf("%s:%d - skipping name %s", __FUNCTION__, __LINE__, xml->GetName().c_str());
		}
	}
	return false;
}


/**
 * @brief Get the language for an adaptation set
 * @param adaptationSet Pointer to adaptation set
 * @retval language of adaptation set
 */
std::string PrivateStreamAbstractionMPD::GetLanguageForAdaptationSet( IAdaptationSet *adaptationSet )
{
	std::string lang = adaptationSet->GetLang();

	if( (GetLangCodePreference()!=ISO639_NO_LANGCODE_PREFERENCE ))
	{
		char lang2[MAX_LANGUAGE_TAG_LENGTH];
		strcpy( lang2, lang.c_str() );
		iso639map_NormalizeLanguageCode( lang2, GetLangCodePreference() );
		lang = lang2;
	}
 
	if( gpGlobalConfig->bDescriptiveAudioTrack )
	{
		std::vector<IDescriptor *> role = adaptationSet->GetRole();
		for (unsigned iRole = 0; iRole < role.size(); iRole++)
		{
			if (role.at(iRole)->GetSchemeIdUri().find("urn:mpeg:dash:role:2011") != string::npos)
			{
				lang += "-" + role.at(iRole)->GetValue();
			}
		}
	}
	return lang;
}


/**
 * @brief Update language list state variables
 */
void PrivateStreamAbstractionMPD::UpdateLanguageList()
{
	size_t numPeriods = mpd->GetPeriods().size();
	for (unsigned iPeriod = 0; iPeriod < numPeriods; iPeriod++)
	{
		IPeriod *period = mpd->GetPeriods().at(iPeriod);
		size_t numAdaptationSets = period->GetAdaptationSets().size();
		for (int iAdaptationSet = 0; iAdaptationSet < numAdaptationSets; iAdaptationSet++)
		{
			IAdaptationSet *adaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
			if (IsContentType(adaptationSet, eMEDIATYPE_AUDIO ))
			{
				mLangList.insert( GetLanguageForAdaptationSet(adaptationSet) );
			}
		}
	}
}

int PrivateStreamAbstractionMPD::GetBestAudioTrackByLanguage( int &desiredRepIdx,AudioType &CodecType)
{
	// Priority in choosing best audio track:
	// 1. Language selected by User, 1. exact match 2.language match  (aamp->language and aamp->noExplicitUserLanguageSelection=false)
	// 2. Preferred language, language match (aamp->preferredLanguages and aamp->noExplicitUserLanguageSelection=true)
	// 3. Initial value of aamp->language (aamp->noExplicitUserLanguageSelection=true)
	// 4. First audio track
	int retAdapSetValue = -1;
	int first_audio_track = -1;
	int first_audio_track_matching_language = -1;
	int iAdaptationSet_codec_cmp = -1;
	int not_explicit_user_lang_track = -1;
	int preferred_audio_track = -1;
	int current_preferred_lang_index = aamp->preferredLanguagesList.size();
	std::string lang;
	const char *delim = strchr(aamp->language,'-');
	size_t aamp_language_length = delim?(delim - aamp->language):strlen(aamp->language);
	struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[eMEDIATYPE_AUDIO];
	IPeriod *period = mCurrentPeriod;
	size_t numAdaptationSets = period->GetAdaptationSets().size();
	logprintf("%s: aamp->language %s, aamp->noExplicitUserLanguageSelection %s, aamp->preferredLanguages \"%s\"",
					__func__, aamp->language, aamp->noExplicitUserLanguageSelection? "true" : "false", aamp->preferredLanguagesString.c_str());
	for( int iAdaptationSet = 0; iAdaptationSet < numAdaptationSets; iAdaptationSet++)
	{
		IAdaptationSet *adaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
		AAMPLOG_TRACE("PrivateStreamAbstractionMPD::%s %d > Content type [%s] AdapSet [%d] ",
				__FUNCTION__, __LINE__, adaptationSet->GetContentType().c_str(),iAdaptationSet);
		if( IsContentType(adaptationSet, eMEDIATYPE_AUDIO) )
		{
			lang = GetLanguageForAdaptationSet(adaptationSet);
			const char *track_language = lang.c_str();
			if(strncmp(aamp->language, track_language, MAX_LANGUAGE_TAG_LENGTH) == 0)
			{ // exact match, i.e. to eng-commentary, great - we're done!
				AudioType selectedCodecType = eAUDIO_UNKNOWN;
				IAdaptationSet *audioAdaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
				if( audioAdaptationSet )
				{
					uint32_t selRepBandwidth = 0;
					int audioRepresentationIndex = GetDesiredCodecIndex(audioAdaptationSet, selectedCodecType, selRepBandwidth);
					if (iAdaptationSet_codec_cmp < selectedCodecType)
					{
						desiredRepIdx = audioRepresentationIndex;
						iAdaptationSet_codec_cmp = selectedCodecType;
						first_audio_track = iAdaptationSet;
						first_audio_track_matching_language = iAdaptationSet;
						not_explicit_user_lang_track = iAdaptationSet;
						CodecType = selectedCodecType;
					}
				}
			}
			if(current_preferred_lang_index > 0)
			{
				// find language part in preferred language list
				// but not further than current index
				std::string langPart = std::string(lang, 0, lang.find_first_of('-'));
				auto iter = std::find(aamp->preferredLanguagesList.begin(),
						(aamp->preferredLanguagesList.begin() + current_preferred_lang_index), langPart);
				if(iter != (aamp->preferredLanguagesList.begin() + current_preferred_lang_index) )
				{
					current_preferred_lang_index = std::distance(aamp->preferredLanguagesList.begin(),
							iter);
					preferred_audio_track = iAdaptationSet;
				}
			}
			if( first_audio_track < 0 )
			{ // remember first track as lowest-priority fallback
				first_audio_track = iAdaptationSet;
			}
			if(first_audio_track_matching_language < 0 )
			{
				int len = 0;
				const char *delim = strchr(track_language,'-');
				len = delim? (delim - track_language):strlen(track_language);
				if( len && len == aamp_language_length && memcmp(aamp->language,track_language,len)==0 )
				{ // remember matching language (but not role) as next-best fallback
					first_audio_track_matching_language = iAdaptationSet;
				}
			}
		}
	} // next iAdaptationSet

	if( !( aamp->noExplicitUserLanguageSelection && preferred_audio_track>=0 )
			&& first_audio_track_matching_language>=0 )
	{
		retAdapSetValue = first_audio_track_matching_language;
	}
	else if ( preferred_audio_track>=0 )
	{
		retAdapSetValue = preferred_audio_track;
		// if preferred one is different than
		// first_audio_track_matching_language,then clear codec info,
		// since current values do not refer to selected track
		if(preferred_audio_track != first_audio_track_matching_language)
		{
			iAdaptationSet_codec_cmp = -1;
			desiredRepIdx = -1;
			CodecType = eAUDIO_UNKNOWN;
		}
	}
	else if ( not_explicit_user_lang_track>=0 )
	{
		retAdapSetValue = not_explicit_user_lang_track;
	}
	else
	{
		retAdapSetValue = first_audio_track;
	}

	if (retAdapSetValue >= 0) //only if adptationset found
	{
		if(iAdaptationSet_codec_cmp == -1) // if nothing set 
		{
			IAdaptationSet *audioAdaptationSet = period->GetAdaptationSets().at(retAdapSetValue);
			if( audioAdaptationSet )
			{
				uint32_t selRepBandwidth = 0;
				desiredRepIdx = GetDesiredCodecIndex(audioAdaptationSet,  CodecType, selRepBandwidth);
			}
		}
	}
	return retAdapSetValue;
}

/**
 * @brief Does stream selection
 * @param newTune true if this is a new tune
 */
void PrivateStreamAbstractionMPD::StreamSelection( bool newTune)
{
	int numTracks = (rate == AAMP_NORMAL_PLAY_RATE)?AAMP_TRACK_COUNT:1;
	mNumberOfTracks = 0;
	if (!aamp->IsSubtitleEnabled() && rate == AAMP_NORMAL_PLAY_RATE)
	{
		AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d - subtitles disabled by application", __FUNCTION__, __LINE__);
		numTracks--;
	}
	IPeriod *period = mCurrentPeriod;
	AAMPLOG_INFO("Selected Period index %d, id %s", mCurrentPeriodIdx, period->GetId().c_str());
	for( int i=0; i<numTracks; i++ )
	{
		mMediaStreamContext[i]->enabled = false;
	}
	AudioType selectedCodecType = eAUDIO_UNKNOWN;
	int audioRepresentationIndex = -1;
 	int desiredRepIdx = -1;	
	int audioAdaptationSetIndex = GetBestAudioTrackByLanguage(desiredRepIdx,selectedCodecType);
	IAdaptationSet *audioAdaptationSet = NULL;
	if ( audioAdaptationSetIndex >= 0 )
	{
		audioAdaptationSet = period->GetAdaptationSets().at(audioAdaptationSetIndex);
	}

	if( audioAdaptationSet )
	{
		std::string lang = GetLanguageForAdaptationSet(audioAdaptationSet);
		aamp->UpdateAudioLanguageSelection( lang.c_str() );
		if(desiredRepIdx != -1 )
		{
			audioRepresentationIndex = desiredRepIdx;
			mAudioType = selectedCodecType;
		}
		logprintf("PrivateStreamAbstractionMPD::%s %d > lang[%s] AudioType[%d]", __FUNCTION__, __LINE__, lang.c_str(), selectedCodecType);
	}
	else
	{
		logprintf("PrivateStreamAbstractionMPD::%s %d Unable to get audioAdaptationSet.", __FUNCTION__, __LINE__);
	}

	for (int i = 0; i < numTracks; i++)
	{
		struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[i];
		size_t numAdaptationSets = period->GetAdaptationSets().size();
		int  selAdaptationSetIndex = -1;
		int selRepresentationIndex = -1;
		bool isIframeAdaptationAvailable = false;
		uint32_t selRepBandwidth = 0;
		int videoRepresentationIdx;
		for (unsigned iAdaptationSet = 0; iAdaptationSet < numAdaptationSets; iAdaptationSet++)
		{
			IAdaptationSet *adaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
			AAMPLOG_TRACE("PrivateStreamAbstractionMPD::%s %d > Content type [%s] AdapSet [%d] ",
					__FUNCTION__, __LINE__, adaptationSet->GetContentType().c_str(),iAdaptationSet);
			if (IsContentType(adaptationSet, (MediaType)i ))
			{
				if (AAMP_NORMAL_PLAY_RATE == rate)
				{
					if (eMEDIATYPE_SUBTITLE == i)
					{
						// map 3 character language code to 2 character
						char lang2[MAX_LANGUAGE_TAG_LENGTH];
						strcpy( lang2, adaptationSet->GetLang().c_str() ); // should use GetLanguageForAdaptationSet?
						iso639map_NormalizeLanguageCode( lang2, ISO639_PREFER_2_CHAR_LANGCODE  );
						std::string lang = lang2;
						if (lang == aamp->mSubLanguage)
						{
							//We support only plain text vtt for now
							const char* supportedMimeType = "text/vtt";
							std::string adaptationMimeType = adaptationSet->GetMimeType();
							if (!adaptationMimeType.empty())
							{
								if (adaptationMimeType == supportedMimeType)
								{
									selAdaptationSetIndex = iAdaptationSet;
									selRepresentationIndex = 0;
								}
							}
							else
							{
								const std::vector<IRepresentation *> representation = adaptationSet->GetRepresentation();
								for (int representationIndex = 0; representationIndex < representation.size(); representationIndex++)
								{
									const dash::mpd::IRepresentation *rep = representation.at(representationIndex);
									std::string mimeType = rep->GetMimeType();
									if (!mimeType.empty() && (mimeType == supportedMimeType))
									{
										selAdaptationSetIndex = iAdaptationSet;
										selRepresentationIndex = representationIndex;
									}
								}
							}
							if (selAdaptationSetIndex != iAdaptationSet)
							{
								//Even though language matched, mimeType is missing or not supported right now. Log for now
								AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s %d > Found matching subtitle language:%s but not supported mimeType and thus disabled!!", __FUNCTION__, __LINE__, lang.c_str());
							}
						}
					}
					else if (eMEDIATYPE_AUDIO == i)
					{
						selAdaptationSetIndex = audioAdaptationSetIndex;
						selRepresentationIndex = audioRepresentationIndex;
					}
					else if (!gpGlobalConfig->bAudioOnlyPlayback)
					{
						if (!isIframeAdaptationAvailable || selAdaptationSetIndex == -1)
						{
							if (!IsIframeTrack(adaptationSet))
							{
								// Got Video , confirmed its not iframe adaptation
								videoRepresentationIdx = GetDesiredVideoCodecIndex(adaptationSet);
								if (videoRepresentationIdx != -1)
								{
									selAdaptationSetIndex = iAdaptationSet;
									AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d > Got video Adaptation Set[%d] Representation[%d]",__FUNCTION__, __LINE__, iAdaptationSet, videoRepresentationIdx);
								}
								if(!newTune)
								{
									if(GetProfileCount() == adaptationSet->GetRepresentation().size())
									{
										selRepresentationIndex = pMediaStreamContext->representationIndex;
									}
									else
									{
										selRepresentationIndex = -1; // this will be set based on profile selection
									}
								}
							}
							else
							{
								isIframeAdaptationAvailable = true;
							}
						}
						else
						{
							break;
						}
					}
				}
				else if ((!gpGlobalConfig->bAudioOnlyPlayback) && (eMEDIATYPE_VIDEO == i))
				{
					//iframe track
					if ( IsIframeTrack(adaptationSet) )
					{
						logprintf("PrivateStreamAbstractionMPD::%s %d > Got TrickMode track", __FUNCTION__, __LINE__);
						pMediaStreamContext->enabled = true;
						pMediaStreamContext->profileChanged = true;
						pMediaStreamContext->adaptationSetIdx = iAdaptationSet;
						mNumberOfTracks = 1;
						isIframeAdaptationAvailable = true;
						break;
					}
				}
			}
		} // next iAdaptationSet

		if ((eAUDIO_UNKNOWN == mAudioType) && (AAMP_NORMAL_PLAY_RATE == rate) && (eMEDIATYPE_VIDEO != i) && selAdaptationSetIndex >= 0)
		{
            AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s %d > Selected Audio Track codec is unknown", __FUNCTION__, __LINE__);
			mAudioType = eAUDIO_AAC; // assuming content is still playable
		}

		// end of adaptation loop
		if ((AAMP_NORMAL_PLAY_RATE == rate) && (pMediaStreamContext->enabled == false) && selAdaptationSetIndex >= 0)
		{
			pMediaStreamContext->enabled = true;
			pMediaStreamContext->adaptationSetIdx = selAdaptationSetIndex;
			pMediaStreamContext->representationIndex = selRepresentationIndex;
			pMediaStreamContext->profileChanged = true;
			
            /* To solve a no audio issue - Force configure gst audio pipeline/playbin in the case of multi period
			 * multi audio codec available for current decoding language on stream. For example, first period has AAC no EC3,
			 * so the player will choose AAC then start decoding, but in the forthcoming periods,
			 * if the stream has AAC and EC3 for the current decoding language then as per the EC3(default priority)
			 * the player will choose EC3 but the audio pipeline actually not configured in this case to affect this change.
			 */
			if ( aamp->previousAudioType != selectedCodecType && eMEDIATYPE_AUDIO == i )
			{
				logprintf("PrivateStreamAbstractionMPD::%s %d > AudioType Changed %d -> %d",
						__FUNCTION__, __LINE__, aamp->previousAudioType, selectedCodecType);
				aamp->previousAudioType = selectedCodecType;
				mContext->SetESChangeStatus();
			}
			logprintf("PrivateStreamAbstractionMPD::%s %d > Media[%s] Adaptation set[%d] RepIdx[%d] TrackCnt[%d]",
				__FUNCTION__, __LINE__, mMediaTypeName[i],selAdaptationSetIndex,selRepresentationIndex,(mNumberOfTracks+1) );

			ProcessContentProtection(period->GetAdaptationSets().at(selAdaptationSetIndex),(MediaType)i);
			mNumberOfTracks++;
		} // AAMP_NORMAL_PLAY_RATE

		if(selAdaptationSetIndex < 0 && rate == 1)
		{
			logprintf("PrivateStreamAbstractionMPD::%s %d > No valid adaptation set found for Media[%s]",
				__FUNCTION__, __LINE__, mMediaTypeName[i]);
		}

		logprintf("PrivateStreamAbstractionMPD::%s %d > Media[%s] %s",
			__FUNCTION__, __LINE__, mMediaTypeName[i], pMediaStreamContext->enabled?"enabled":"disabled");

		//Store the iframe track status in current period if there is any change
		if (!gpGlobalConfig->bAudioOnlyPlayback && (i == eMEDIATYPE_VIDEO) && (aamp->mIsIframeTrackPresent != isIframeAdaptationAvailable))
		{
			aamp->mIsIframeTrackPresent = isIframeAdaptationAvailable;
			//Iframe tracks changed mid-stream, sent a playbackspeed changed event
			if (!newTune)
			{
				aamp->SendSupportedSpeedsChangedEvent(aamp->mIsIframeTrackPresent);
			}
		}
	} // next track

	if(1 == mNumberOfTracks && !mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled)
	{ // what about audio+subtitles?
		if(newTune)
			logprintf("PrivateStreamAbstractionMPD::%s:%d Audio only period", __FUNCTION__, __LINE__);
		mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled = mMediaStreamContext[eMEDIATYPE_AUDIO]->enabled;
		mMediaStreamContext[eMEDIATYPE_VIDEO]->adaptationSetIdx = mMediaStreamContext[eMEDIATYPE_AUDIO]->adaptationSetIdx;
		mMediaStreamContext[eMEDIATYPE_VIDEO]->representationIndex = mMediaStreamContext[eMEDIATYPE_AUDIO]->representationIndex;
		mMediaStreamContext[eMEDIATYPE_VIDEO]->mediaType = eMEDIATYPE_VIDEO;
		mMediaStreamContext[eMEDIATYPE_VIDEO]->type = eTRACK_VIDEO;
		mMediaStreamContext[eMEDIATYPE_VIDEO]->profileChanged = true;
		mMediaStreamContext[eMEDIATYPE_AUDIO]->enabled = false;
	}
}

/**
 * @brief Extract bitrate info from custom mpd
 * @note Caller function should delete the vector elements after use.
 * @param adaptationSet : AdaptaionSet from which bitrate info is to be extracted
 * @param[out] representations : Representation vector gets updated with Available bit rates.
 */
static void GetBitrateInfoFromCustomMpd(IAdaptationSet *adaptationSet, std::vector<Representation *>& representations )
{
	std::vector<xml::INode *> subNodes = adaptationSet->GetAdditionalSubNodes();
	for(int i = 0; i < subNodes.size(); i ++)
	{
		xml::INode * node = subNodes.at(i);
		if(node->GetName() == "AvailableBitrates")
		{
			std::vector<xml::INode *> reprNodes = node->GetNodes();
			for(int reprIter = 0; reprIter < reprNodes.size(); reprIter++)
			{
				xml::INode * reprNode = reprNodes.at(reprIter);
				if(reprNode->GetName() == "Representation")
				{
					dash::mpd::Representation * repr = new dash::mpd::Representation();
					if(reprNode->HasAttribute("bandwidth"))
					{
						repr->SetBandwidth(stol(reprNode->GetAttributeValue("bandwidth")));
					}
					if(reprNode->HasAttribute("height"))
					{
						repr->SetHeight(stol(reprNode->GetAttributeValue("height")));
					}
					if(reprNode->HasAttribute("width"))
					{
						repr->SetWidth(stol(reprNode->GetAttributeValue("width")));
					}
					representations.push_back(repr);
				}
			}
			break;
		}
	}
}


/**
 * @brief Updates track information based on current state
 */
AAMPStatusType PrivateStreamAbstractionMPD::UpdateTrackInfo(bool modifyDefaultBW, bool periodChanged, bool resetTimeLineIndex)
{
	AAMPStatusType ret = eAAMPSTATUS_OK;
	long defaultBitrate = gpGlobalConfig->defaultBitrate;
	long iframeBitrate = gpGlobalConfig->iframeBitrate;
	bool isFogTsb = mIsFogTSB && !mAdPlayingFromCDN;	/*Conveys whether the current playback from FOG or not.*/
	long minBitrate = aamp->GetMinimumBitrate();
	long maxBitrate = aamp->GetMaximumBitrate();

	for (int i = 0; i < mNumberOfTracks; i++)
	{
		struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[i];
		if(mCdaiObject->mAdState == AdState::IN_ADBREAK_AD_PLAYING)
		{
			AdNode &adNode = mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx);
			if(adNode.mpd != NULL)
			{
				pMediaStreamContext->fragmentDescriptor.manifestUrl = adNode.url.c_str();
			}
		}
		if(pMediaStreamContext->enabled)
		{
			IPeriod *period = mCurrentPeriod;
			pMediaStreamContext->adaptationSet = period->GetAdaptationSets().at(pMediaStreamContext->adaptationSetIdx);
			pMediaStreamContext->adaptationSetId = pMediaStreamContext->adaptationSet->GetId();
			std::string adapFrameRate = pMediaStreamContext->adaptationSet->GetFrameRate();
			/*Populate StreamInfo for ABR Processing*/
			if (i == eMEDIATYPE_VIDEO)
			{
				if(isFogTsb && periodChanged)
				{
					vector<Representation *> representations;
					GetBitrateInfoFromCustomMpd(pMediaStreamContext->adaptationSet, representations);
					int representationCount = representations.size();
					if ((representationCount != mBitrateIndexMap.size()) && mStreamInfo)
					{
						delete[] mStreamInfo;
						mStreamInfo = NULL;
					}
					if (!mStreamInfo)
					{
						mStreamInfo = new StreamInfo[representationCount];
					}
					mContext->GetABRManager().clearProfiles();
					mBitrateIndexMap.clear();
					mMaxTSBBandwidth = 0;
					for (int idx = 0; idx < representationCount; idx++)
					{
						Representation* representation = representations.at(idx);
						mStreamInfo[idx].bandwidthBitsPerSecond = representation->GetBandwidth();
						mStreamInfo[idx].isIframeTrack = !(AAMP_NORMAL_PLAY_RATE == rate);
						mStreamInfo[idx].resolution.height = representation->GetHeight();
						mStreamInfo[idx].resolution.width = representation->GetWidth();
						mStreamInfo[idx].resolution.framerate = 0;
						//Update profile resolution with VideoEnd Metrics object.
						aamp->UpdateVideoEndProfileResolution((mStreamInfo[idx].isIframeTrack ? eMEDIATYPE_IFRAME : eMEDIATYPE_VIDEO ),
												mStreamInfo[idx].bandwidthBitsPerSecond,
												mStreamInfo[idx].resolution.width,
												mStreamInfo[idx].resolution.height);

						std::string repFrameRate = representation->GetFrameRate();
						if(repFrameRate.empty())
							repFrameRate = adapFrameRate;
						if(!repFrameRate.empty())
						{
							int val1, val2;
							sscanf(repFrameRate.c_str(),"%d/%d",&val1,&val2);
							double frate = val2? ((double)val1/val2):val1;
							mStreamInfo[idx].resolution.framerate = frate;
						}
						
						mBitrateIndexMap[mStreamInfo[idx].bandwidthBitsPerSecond] = idx;
						delete representation;

						if(mStreamInfo[idx].bandwidthBitsPerSecond > mMaxTSBBandwidth)
						{
							mMaxTSBBandwidth = mStreamInfo[idx].bandwidthBitsPerSecond;
						}
					}
					pMediaStreamContext->representationIndex = 0; //Fog custom mpd has a single representation
					IRepresentation* representation = pMediaStreamContext->adaptationSet->GetRepresentation().at(0);
					pMediaStreamContext->fragmentDescriptor.Bandwidth = representation->GetBandwidth();
					aamp->profiler.SetBandwidthBitsPerSecondVideo(pMediaStreamContext->fragmentDescriptor.Bandwidth);
					mContext->profileIdxForBandwidthNotification = mBitrateIndexMap[pMediaStreamContext->fragmentDescriptor.Bandwidth];
				}
				else if(!isFogTsb && periodChanged)
				{
					int representationCount = pMediaStreamContext->adaptationSet->GetRepresentation().size();
					if ((representationCount != GetProfileCount()) && mStreamInfo)
					{
						delete[] mStreamInfo;
						mStreamInfo = NULL;
					}
					if (!mStreamInfo)
					{
						mStreamInfo = new StreamInfo[representationCount];
					}
					mContext->GetABRManager().clearProfiles();
					mBitrateIndexMap.clear();
					int addedProfiles = 0;
					for (int idx = 0; idx < representationCount; idx++)
					{
						IRepresentation *representation = pMediaStreamContext->adaptationSet->GetRepresentation().at(idx);
						mStreamInfo[idx].bandwidthBitsPerSecond = representation->GetBandwidth();
						mStreamInfo[idx].isIframeTrack = !(AAMP_NORMAL_PLAY_RATE == rate);
						mStreamInfo[idx].resolution.height = representation->GetHeight();
						mStreamInfo[idx].resolution.width = representation->GetWidth();
						mStreamInfo[idx].resolution.framerate = 0;
						std::string repFrameRate = representation->GetFrameRate();
						if(repFrameRate.empty())
							repFrameRate = adapFrameRate;
						if(!repFrameRate.empty())
						{
							int val1, val2;
							sscanf(repFrameRate.c_str(),"%d/%d",&val1,&val2);
							double frate = val2? ((double)val1/val2):val1;
							mStreamInfo[idx].resolution.framerate = frate;
						}
						if ((mStreamInfo[idx].bandwidthBitsPerSecond > minBitrate) && (mStreamInfo[idx].bandwidthBitsPerSecond < maxBitrate))
						{
							mContext->GetABRManager().addProfile({
								mStreamInfo[idx].isIframeTrack,
								mStreamInfo[idx].bandwidthBitsPerSecond,
								mStreamInfo[idx].resolution.width,
								mStreamInfo[idx].resolution.height,
							});
							addedProfiles++;

							//Update profile resolution with VideoEnd Metrics object.
							aamp->UpdateVideoEndProfileResolution((mStreamInfo[idx].isIframeTrack ? eMEDIATYPE_IFRAME : eMEDIATYPE_VIDEO ),
													mStreamInfo[idx].bandwidthBitsPerSecond,
													mStreamInfo[idx].resolution.width,
													mStreamInfo[idx].resolution.height);

							if(mStreamInfo[idx].resolution.height > 1080
									|| mStreamInfo[idx].resolution.width > 1920)
							{
								defaultBitrate = gpGlobalConfig->defaultBitrate4K;
								iframeBitrate = gpGlobalConfig->iframeBitrate4K;
							}
						}
					}

					if (0 == addedProfiles)
					{
						ret = eAAMPSTATUS_MANIFEST_CONTENT_ERROR;
						AAMPLOG_WARN("%s:%d No video profiles found, minBitrate : %ld maxBitrate: %ld", __FUNCTION__, __LINE__, minBitrate, maxBitrate);
						return ret;
					}
					if (modifyDefaultBW)
					{
						long persistedBandwidth = aamp->GetPersistedBandwidth();
						if(persistedBandwidth > 0 && (persistedBandwidth < defaultBitrate))
						{
							defaultBitrate = persistedBandwidth;
						}
					}
				}

				if (defaultBitrate != gpGlobalConfig->defaultBitrate)
				{
					mContext->GetABRManager().setDefaultInitBitrate(defaultBitrate);
				}
			}

			if(-1 == pMediaStreamContext->representationIndex)
			{
				if(!isFogTsb)
				{
					if(i == eMEDIATYPE_VIDEO)
					{
						if(mContext->trickplayMode)
						{
							if (iframeBitrate > 0)
							{
								mContext->GetABRManager().setDefaultIframeBitrate(iframeBitrate);
							}
							mContext->UpdateIframeTracks();
						}
						mContext->currentProfileIndex = mContext->GetDesiredProfile(false);
						pMediaStreamContext->representationIndex = mContext->currentProfileIndex;
						IRepresentation *selectedRepresentation = pMediaStreamContext->adaptationSet->GetRepresentation().at(pMediaStreamContext->representationIndex);
						// for the profile selected ,reset the abr values with default bandwidth values
						aamp->ResetCurrentlyAvailableBandwidth(selectedRepresentation->GetBandwidth(),mContext->trickplayMode,mContext->currentProfileIndex);
						aamp->profiler.SetBandwidthBitsPerSecondVideo(selectedRepresentation->GetBandwidth());
					}
					else
					{
						pMediaStreamContext->representationIndex = pMediaStreamContext->adaptationSet->GetRepresentation().size() / 2; //Select the medium profile on start
						if(i == eMEDIATYPE_AUDIO)
						{
							IRepresentation *selectedRepresentation = pMediaStreamContext->adaptationSet->GetRepresentation().at(pMediaStreamContext->representationIndex);
							aamp->profiler.SetBandwidthBitsPerSecondAudio(selectedRepresentation->GetBandwidth());
						}
					}
				}
				else
				{
					logprintf("%s:%d: [WARN] !! representationIndex is '-1' override with '0' since Custom MPD has single representation", __FUNCTION__, __LINE__);
					pMediaStreamContext->representationIndex = 0; //Fog custom mpd has single representation
				}
			}
			pMediaStreamContext->representation = pMediaStreamContext->adaptationSet->GetRepresentation().at(pMediaStreamContext->representationIndex);

			pMediaStreamContext->fragmentDescriptor.baseUrls = &pMediaStreamContext->representation->GetBaseURLs();
			if (pMediaStreamContext->fragmentDescriptor.baseUrls->size() == 0)
			{
				pMediaStreamContext->fragmentDescriptor.baseUrls = &pMediaStreamContext->adaptationSet->GetBaseURLs();
				if (pMediaStreamContext->fragmentDescriptor.baseUrls->size() == 0)
				{
					pMediaStreamContext->fragmentDescriptor.baseUrls = &mCurrentPeriod->GetBaseURLs();
					if (pMediaStreamContext->fragmentDescriptor.baseUrls->size() == 0)
					{
						pMediaStreamContext->fragmentDescriptor.baseUrls = &mpd->GetBaseUrls();
					}
				}
			}
			pMediaStreamContext->fragmentIndex = 0;
			if(resetTimeLineIndex)
				pMediaStreamContext->timeLineIndex = 0;
			pMediaStreamContext->fragmentRepeatCount = 0;
			pMediaStreamContext->fragmentOffset = 0;
			pMediaStreamContext->periodStartOffset = pMediaStreamContext->fragmentTime;
			pMediaStreamContext->eos = false;
			if(0 == pMediaStreamContext->fragmentDescriptor.Bandwidth || !aamp->IsTSBSupported())
			{
				pMediaStreamContext->fragmentDescriptor.Bandwidth = pMediaStreamContext->representation->GetBandwidth();
			}
			pMediaStreamContext->fragmentDescriptor.RepresentationID.assign(pMediaStreamContext->representation->GetId());
			pMediaStreamContext->fragmentDescriptor.Time = 0;
			if(periodChanged)
			{
				//update period start and endtimes as period has changed. 
				mPeriodEndTime = GetPeriodEndTime(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs);
				mPeriodStartTime = GetPeriodStartTime(mpd, mCurrentPeriodIdx);
			}
			ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
			if(!segmentTemplate)
			{
				segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
			}
			if(segmentTemplate)
			{
				pMediaStreamContext->fragmentDescriptor.Number = segmentTemplate->GetStartNumber();
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d Track %d timeLineIndex %d fragmentDescriptor.Number %llu", __FUNCTION__, __LINE__, i, pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentDescriptor.Number);
			}
		}
	}
	return ret;
}



/**
 * @brief Update culling state for live manifests
 */
double PrivateStreamAbstractionMPD::GetCulledSeconds()
{
	double newStartTimeSeconds = 0;
	double culled = 0;
	traceprintf("PrivateStreamAbstractionMPD::%s:%d Enter", __FUNCTION__, __LINE__);
	MediaStreamContext *pMediaStreamContext = mMediaStreamContext[eMEDIATYPE_VIDEO];
	if (pMediaStreamContext->adaptationSet)
	{
		ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
		const ISegmentTimeline *segmentTimeline = NULL;
		if (!segmentTemplate && pMediaStreamContext->representation)
		{
			segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
		}

		if (segmentTemplate)
		{
			segmentTimeline = segmentTemplate->GetSegmentTimeline();
			if (segmentTimeline)
			{
				auto periods = mpd->GetPeriods();
				vector<PeriodInfo> currMPDPeriodDetails;
				uint32_t timescale = segmentTemplate->GetTimescale();
				for (int iter = 0; iter < periods.size(); iter++)
				{
					auto period = periods.at(iter);
					PeriodInfo periodInfo;
					periodInfo.periodId = period->GetId();
					periodInfo.duration = (double)aamp_GetPeriodDuration(mpd, iter, mLastPlaylistDownloadTimeMs)/ 1000;
					periodInfo.startTime = GetFirstSegmentStartTime(period);
					currMPDPeriodDetails.push_back(periodInfo);
				}

				int iter1 = 0;
				PeriodInfo currFirstPeriodInfo = currMPDPeriodDetails.at(0);
				while (iter1 < mMPDPeriodsInfo.size())
				{
					PeriodInfo prevPeriodInfo = mMPDPeriodsInfo.at(iter1);
					if(prevPeriodInfo.periodId == currFirstPeriodInfo.periodId)
					{
						if(prevPeriodInfo.startTime && currFirstPeriodInfo.startTime)
						{
							uint64_t timeDiff = currFirstPeriodInfo.startTime - prevPeriodInfo.startTime;
							culled += ((double)timeDiff / (double)timescale);
							AAMPLOG_INFO("%s:%d PeriodId %s, prevStart %" PRIu64 " currStart %" PRIu64 " culled %f", __FUNCTION__, __LINE__,
												prevPeriodInfo.periodId.c_str(), prevPeriodInfo.startTime, currFirstPeriodInfo.startTime, culled);
						}
						break;
					}
					else
					{
						culled += prevPeriodInfo.duration;
						iter1++;
						logprintf("%s:%d PeriodId %s , with last known duration %f seems to have got culled", __FUNCTION__, __LINE__,
										prevPeriodInfo.periodId.c_str(), prevPeriodInfo.duration);
					}
				}
				mMPDPeriodsInfo = currMPDPeriodDetails;
			}
			else
			{
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d NULL segmentTimeline. Hence modifying culling logic based on MPD availabilityStartTime, periodStartTime, fragment number and current time", __FUNCTION__, __LINE__);
				double newStartSegment = 0;
				ISegmentTemplate *firstSegTempate = NULL;

				// Recalculate the new start fragment after periodic manifest updates
				auto periods = mpd->GetPeriods();
				for (auto period : periods)
				{
					auto adaptationSets = period->GetAdaptationSets();
					for(auto adaptation : adaptationSets)
					{
						auto segTemplate = adaptation->GetSegmentTemplate();
						if(!segTemplate && adaptation->GetRepresentation().size() > 0)
						{
							segTemplate = adaptation->GetRepresentation().at(0)->GetSegmentTemplate();
						}

						if(segTemplate)
						{
							firstSegTempate = segTemplate;
							break;
						}
					}
					if(firstSegTempate)
					{
						break;
					}
				}

				if(firstSegTempate)
				{
					newStartSegment = (double)firstSegTempate->GetStartNumber();
					double fragmentDuration = ((double)segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
					if (newStartSegment && mPrevStartTimeSeconds)
					{
						culled = (newStartSegment - mPrevStartTimeSeconds) * fragmentDuration;
						traceprintf("PrivateStreamAbstractionMPD::%s:%d post-refresh %fs before %f (%f)", __FUNCTION__, __LINE__, newStartTimeSeconds, mPrevStartTimeSeconds, culled);
					}
					else
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d newStartTimeSeconds %f mPrevStartTimeSeconds %F", __FUNCTION__, __LINE__, newStartSegment, mPrevStartTimeSeconds);
					}
					mPrevStartTimeSeconds = newStartSegment;
				}
			}
		}
		else
		{
			ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
			if (segmentList)
			{
				std::map<string,string> rawAttributes = segmentList->GetRawAttributes();
				if(rawAttributes.find("customlist") == rawAttributes.end())
				{
					segmentTimeline = segmentList->GetSegmentTimeline();
				}
				else
				{
					//Updated logic for culling,
					vector<IPeriod*> periods =  mpd->GetPeriods();
					long duration = 0;
					long prevLastSegUrlOffset = 0;
					long newOffset = 0;
					bool offsetFound = false;
					std::string newMedia;
					for(int iPeriod = periods.size() - 1 ; iPeriod >= 0; iPeriod--)
					{
						IPeriod* period = periods.at(iPeriod);
						vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
						if (adaptationSets.empty())
						{
							continue;
						}
						IAdaptationSet * adaptationSet = adaptationSets.at(0);

						vector<IRepresentation *> representations = adaptationSet->GetRepresentation();

						if(representations.empty())
						{
							continue;
						}

						IRepresentation* representation = representations.at(0);
						ISegmentList *segmentList = representation->GetSegmentList();

						if(!segmentList)
						{
							continue;
						}

						duration += segmentList->GetDuration();
						vector<ISegmentURL*> segUrls = segmentList->GetSegmentURLs();
						if(!segUrls.empty())
						{
							for(int iSegurl = segUrls.size() - 1; iSegurl >= 0 && !offsetFound; iSegurl--)
							{
								std::string media = segUrls.at(iSegurl)->GetMediaURI();
								std::string offsetStr = segUrls.at(iSegurl)->GetRawAttributes().at("d");
								uint32_t offset = stol(offsetStr);
								if(0 == newOffset)
								{
									newOffset = offset;
									newMedia = media;
								}
								if(0 == mPrevLastSegurlOffset && !offsetFound)
								{
									offsetFound = true;
									break;
								}
								else if(mPrevLastSegurlMedia == media)
								{
									offsetFound = true;
									prevLastSegUrlOffset += offset;
									break;
								}
								else
								{
									prevLastSegUrlOffset += offset;
								}
							}//End of segurl for loop
						}
					} //End of Period for loop
					long offsetDiff = 0;
					long currOffset = duration - prevLastSegUrlOffset;
					if(mPrevLastSegurlOffset)
					{
						long timescale = segmentList->GetTimescale();
						offsetDiff = mPrevLastSegurlOffset - currOffset;
						if(offsetDiff > 0)
						{
							culled = (double)offsetDiff / timescale;
						}
					}
					AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d PrevOffset %ld CurrentOffset %ld culled (%f)", __FUNCTION__, __LINE__, mPrevLastSegurlOffset, currOffset, culled);
					mPrevLastSegurlOffset = duration - newOffset;
					mPrevLastSegurlMedia = newMedia;
				}
			}
			else
			{
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d NULL segmentTemplate and segmentList", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		logprintf("PrivateStreamAbstractionMPD::%s:%d NULL adaptationset", __FUNCTION__, __LINE__);
	}
	return culled;
}

/**
 * @brief Fetch and inject initialization fragment
 * @param discontinuity true if discontinuous fragment
 */
void PrivateStreamAbstractionMPD::FetchAndInjectInitialization(bool discontinuity)
{
	pthread_t trackDownloadThreadID;
	HeaderFetchParams *fetchParams = NULL;
	bool dlThreadCreated = false;
	int numberOfTracks = mNumberOfTracks;
	for (int i = 0; i < numberOfTracks; i++)
	{
		struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[i];
		if(discontinuity && pMediaStreamContext->enabled)
		{
			pMediaStreamContext->discontinuity = discontinuity;
		}
		if(pMediaStreamContext->enabled && (pMediaStreamContext->profileChanged || pMediaStreamContext->discontinuity))
		{
			if (pMediaStreamContext->adaptationSet)
			{
				ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();

				//XRE-12249: SegmentTemplate can be a sub-node of Representation
				if(!segmentTemplate)
				{
					segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
				}
				//XRE-12249: End of Fix

				if (segmentTemplate)
				{
					std::string initialization = segmentTemplate->Getinitialization();
					if (!initialization.empty())
					{
						double fragmentDuration = 0.0;
						/*
						 * This block is added to download the initialization tracks in parallel
						 * to reduce the tune time, especially when using DRM.
						 * Moving the fragment download of first AAMPTRACK to separate thread
						 */
						if(!dlThreadCreated)
						{
							fetchParams = new HeaderFetchParams();
							fetchParams->context = this;
							fetchParams->fragmentduration = fragmentDuration;
							fetchParams->initialization = initialization;
							fetchParams->isinitialization = true;
							fetchParams->pMediaStreamContext = pMediaStreamContext;
							fetchParams->discontinuity = pMediaStreamContext->discontinuity;
							int ret = pthread_create(&trackDownloadThreadID, NULL, TrackDownloader, fetchParams);
							if(ret != 0)
							{
								logprintf("PrivateStreamAbstractionMPD::%s:%d pthread_create failed for TrackDownloader with errno = %d, %s", __FUNCTION__, __LINE__, errno, strerror(errno));
								delete fetchParams;
							}
							else
							{
								dlThreadCreated = true;
							}
						}
						else
						{
							if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
							{
								pMediaStreamContext->profileChanged = false;
								FetchFragment(pMediaStreamContext, initialization, fragmentDuration,true, (eMEDIATYPE_AUDIO == i), pMediaStreamContext->discontinuity);
								pMediaStreamContext->discontinuity = false;
							}
						}
					}
				}
				else
				{
					ISegmentBase *segmentBase = pMediaStreamContext->representation->GetSegmentBase();
					if (segmentBase)
					{
						pMediaStreamContext->fragmentOffset = 0;
						if (pMediaStreamContext->index_ptr)
						{
							aamp_Free(&pMediaStreamContext->index_ptr);
						}
						const IURLType *urlType = segmentBase->GetInitialization();
						if (urlType)
						{
							std::string range = urlType->GetRange();
							int start, fin;
							sscanf(range.c_str(), "%d-%d", &start, &fin);
#ifdef DEBUG_TIMELINE
							logprintf("init %s %d..%d", mMediaTypeName[pMediaStreamContext->mediaType], start, fin);
#endif
							std::string fragmentUrl;
							GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
							if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
							{
								pMediaStreamContext->profileChanged = false;
								if(!pMediaStreamContext->CacheFragment(fragmentUrl, 0, pMediaStreamContext->fragmentTime, 0, range.c_str(), true ))
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f", __FUNCTION__, __LINE__, fragmentUrl.c_str(), pMediaStreamContext->fragmentTime);
								}
							}
						}
					}
					else
					{
						ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
						if (segmentList)
						{
							std::string initialization = segmentList->GetInitialization()->GetSourceURL();
							if (!initialization.empty())
							{
								double fragmentDuration = 0.0;
								/*
								 * This block is added to download the initialization tracks in parallel
								 * to reduce the tune time, especially when using DRM.
								 * Moving the fragment download of first AAMPTRACK to separate thread
								 */
								if(!dlThreadCreated)
								{
									fetchParams = new HeaderFetchParams();
									fetchParams->context = this;
									fetchParams->fragmentduration = fragmentDuration;
									fetchParams->initialization = initialization;
									fetchParams->isinitialization = true;
									fetchParams->pMediaStreamContext = pMediaStreamContext;
									int ret = pthread_create(&trackDownloadThreadID, NULL, TrackDownloader, fetchParams);
									if(ret != 0)
									{
										logprintf("PrivateStreamAbstractionMPD::%s:%d pthread_create failed for TrackDownloader with errno = %d, %s", __FUNCTION__, __LINE__, errno, strerror(errno));
										delete fetchParams;
									}
									else
									{
										dlThreadCreated = true;
									}
								}
								else
								{
									if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
									{
										pMediaStreamContext->profileChanged = false;
										FetchFragment(pMediaStreamContext, initialization, fragmentDuration,true, (eMEDIATYPE_AUDIO == i));
									}
								}
							}
							else
							{
								string range;
#ifdef LIBDASH_SEGMENTLIST_GET_INIT_SUPPORT
								const ISegmentURL *segmentURL = NULL;
								segmentURL = segmentList->Getinitialization();

								if (segmentURL)
								{
									range = segmentURL->GetMediaRange();
								}
#else
								const std::vector<ISegmentURL*> segmentURLs = segmentList->GetSegmentURLs();
								if (segmentURLs.size() > 0)
								{
									ISegmentURL *firstSegmentURL = segmentURLs.at(0);
									int start, fin;
									const char *firstSegmentRange = firstSegmentURL->GetMediaRange().c_str();
									AAMPLOG_INFO("firstSegmentRange %s [%s]",
											mMediaTypeName[pMediaStreamContext->mediaType], firstSegmentRange);
									if (sscanf(firstSegmentRange, "%d-%d", &start, &fin) == 2)
									{
										if (start > 1)
										{
											char range_c[64];
											sprintf(range_c, "%d-%d", 0, start - 1);
											range = range_c;
										}
										else
										{
											logprintf("PrivateStreamAbstractionMPD::%s:%d segmentList - cannot determine range for Initialization - first segment start %d",
													__FUNCTION__, __LINE__, start);
										}
									}
								}
#endif
								if (!range.empty())
								{
									std::string fragmentUrl;
									GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
									AAMPLOG_INFO("%s [%s]", mMediaTypeName[pMediaStreamContext->mediaType],
											range.c_str());
									if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
									{
										pMediaStreamContext->profileChanged = false;
										if(!pMediaStreamContext->CacheFragment(fragmentUrl, 0, pMediaStreamContext->fragmentTime, 0.0, range.c_str(), true ))
										{
											logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f", __FUNCTION__, __LINE__, fragmentUrl.c_str(), pMediaStreamContext->fragmentTime);
										}
									}
								}
								else
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d segmentList - empty range string for Initialization",
											__FUNCTION__, __LINE__);
								}
							}
						}
						else
						{
							AAMPLOG_ERR("%s:%d not-yet-supported mpd format",__FUNCTION__,__LINE__);
						}
					}
				}
			}
		}
	}

	if(dlThreadCreated)
	{
		AAMPLOG_TRACE("Waiting for pthread_join trackDownloadThread");
		pthread_join(trackDownloadThreadID, NULL);
		AAMPLOG_TRACE("Joined trackDownloadThread");
		delete fetchParams;
	}
}


/**
 * @brief Check if current period is clear
 * @retval true if clear period
 */
bool PrivateStreamAbstractionMPD::CheckForInitalClearPeriod()
{
	bool ret = true;
	vector<IDescriptor*> contentProt;
	for(int i = 0; i < mNumberOfTracks; i++)
	{
		contentProt = mMediaStreamContext[i]->adaptationSet->GetContentProtection();
		if(0 != contentProt.size())
		{
			ret = false;
			break;
		}
	}
	if(ret)
	{
		logprintf("%s %d Initial period is clear period, trying work around",__FUNCTION__,__LINE__);
	}
	return ret;
}

/**
 * @brief Push encrypted headers if available
 */
void PrivateStreamAbstractionMPD::PushEncryptedHeaders()
{
	//Find the first period with contentProtection
	size_t numPeriods = mpd->GetPeriods().size();
	int headerCount = 0;
	for(int i = mNumberOfTracks - 1; i >= 0; i--)
	{
		bool encryptionFound = false;
		unsigned iPeriod = 0;
		while(iPeriod < numPeriods && !encryptionFound)
		{
			IPeriod *period = mpd->GetPeriods().at(iPeriod);
			size_t numAdaptationSets = period->GetAdaptationSets().size();
			for(unsigned iAdaptationSet = 0; iAdaptationSet < numAdaptationSets && !encryptionFound; iAdaptationSet++)
			{
				IAdaptationSet *adaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
				if (IsContentType(adaptationSet, (MediaType)i ))
				{
					vector<IDescriptor*> contentProt;
					contentProt = adaptationSet->GetContentProtection();
					if(0 == contentProt.size())
					{
						continue;
					}
					else
					{
						ISegmentTemplate *segmentTemplate = adaptationSet->GetSegmentTemplate();
						if (segmentTemplate)
						{
							std::string initialization = segmentTemplate->Getinitialization();
							if (!initialization.empty())
							{
								std::string fragmentUrl;
								FragmentDescriptor *fragmentDescriptor = new FragmentDescriptor();
								fragmentDescriptor->manifestUrl = mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentDescriptor.manifestUrl;
								IRepresentation *representation = NULL;
								size_t representionIndex = 0;
								if(MediaType(i) == eMEDIATYPE_VIDEO)
								{
									size_t representationCount = adaptationSet->GetRepresentation().size();
									if(adaptationSet->GetRepresentation().at(representionIndex)->GetBandwidth() > adaptationSet->GetRepresentation().at(representationCount - 1)->GetBandwidth())
									{
										representionIndex = representationCount - 1;
									}
								}
								else if (mAudioType != eAUDIO_UNKNOWN)
								{
									AudioType selectedAudioType = eAUDIO_UNKNOWN;
									uint32_t selectedRepBandwidth = 0;
									representionIndex = GetDesiredCodecIndex(adaptationSet, selectedAudioType, selectedRepBandwidth);
									if(selectedAudioType != mAudioType)
									{
										continue;
									}
									logprintf("%s %d Audio type %d", __FUNCTION__, __LINE__, selectedAudioType);
								}
								else
								{
									logprintf("%s %d Audio type eAUDIO_UNKNOWN", __FUNCTION__, __LINE__);
								}
								ProcessContentProtection(adaptationSet, (MediaType)i);
								representation = adaptationSet->GetRepresentation().at(representionIndex);
								fragmentDescriptor->Bandwidth = representation->GetBandwidth();
								fragmentDescriptor->baseUrls = &representation->GetBaseURLs();
								if (fragmentDescriptor->baseUrls->size() == 0)
								{
									fragmentDescriptor->baseUrls = &adaptationSet->GetBaseURLs();
									if (fragmentDescriptor->baseUrls->size() == 0)
									{
										fragmentDescriptor->baseUrls = &period->GetBaseURLs();
										if (fragmentDescriptor->baseUrls->size() == 0)
										{
											fragmentDescriptor->baseUrls = &mpd->GetBaseUrls();
										}
									}
								}
								fragmentDescriptor->RepresentationID.assign(representation->GetId());
								GetFragmentUrl(fragmentUrl,fragmentDescriptor , initialization);
								if (mMediaStreamContext[i]->WaitForFreeFragmentAvailable())
								{
									logprintf("%s %d Pushing encrypted header for %s", __FUNCTION__, __LINE__, mMediaTypeName[i]);
									mMediaStreamContext[i]->CacheFragment(fragmentUrl, i, mMediaStreamContext[i]->fragmentTime, 0.0, NULL, true);
								}
								delete fragmentDescriptor;
								encryptionFound = true;
							}
						}
					}
				}
			}
			iPeriod++;
		}
	}
}

/**
 * @brief Fetches and caches fragments in a loop
 */
void PrivateStreamAbstractionMPD::FetcherLoop()
{
	bool exitFetchLoop = false;
	bool trickPlay = (AAMP_NORMAL_PLAY_RATE != rate);
	bool mpdChanged = false;
	double delta = 0;
	bool lastLiveFlag = false;
	bool placeNextAd = false;
	int direction = 1;
  
	if(rate < 0)
		direction = -1;
	bool adStateChange = false;
#ifdef AAMP_MPD_DRM
	if (mPushEncInitFragment && CheckForInitalClearPeriod())
	{
		PushEncryptedHeaders();
	}
	mPushEncInitFragment = false;
#endif

	logprintf("PrivateStreamAbstractionMPD::%s:%d - fetch initialization fragments", __FUNCTION__, __LINE__);
	FetchAndInjectInitialization();
	IPeriod *currPeriod = mCurrentPeriod;
	std::string currentPeriodId = currPeriod->GetId();
	mPrevAdaptationSetCount = currPeriod->GetAdaptationSets().size();
	logprintf("aamp: ready to collect fragments. mpd %p", mpd);
	do
	{
		bool liveMPDRefresh = false;
          	bool waitForAdBreakCatchup= false;
		if (mpd)
		{
			size_t numPeriods = mpd->GetPeriods().size();
			unsigned iPeriod = mCurrentPeriodIdx;
			AAMPLOG_INFO("MPD has %d periods current period index %d", numPeriods, mCurrentPeriodIdx);
			while(iPeriod < numPeriods && iPeriod >= 0 && !exitFetchLoop)
			{
				bool periodChanged = (iPeriod != mCurrentPeriodIdx) | (mBasePeriodId != mpd->GetPeriods().at(mCurrentPeriodIdx)->GetId());
				if (periodChanged || mpdChanged || adStateChange)
				{
					bool discontinuity = false;
					bool requireStreamSelection = false;
					uint64_t nextSegmentTime = mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentDescriptor.Time;
					mpdChanged = false;
					if (periodChanged)
					{
						IPeriod *newPeriod = mpd->GetPeriods().at(iPeriod);

						//for VOD and cDVR
						logprintf("%s:%d Period(%s - %d/%d) Offset[%lf] IsLive(%d) IsCdvr(%d) ",__FUNCTION__,__LINE__,
							mBasePeriodId.c_str(), mCurrentPeriodIdx,numPeriods, mBasePeriodOffset, mIsLiveStream,aamp->IsInProgressCDVR());

						vector <IAdaptationSet*> adapatationSets = newPeriod->GetAdaptationSets();
						int adaptationSetCount = adapatationSets.size();
						if(0 == adaptationSetCount)
						{
							/*To Handle non fog scenarios where empty periods are
							* present after mpd update causing issues (DELIA-29879)
							*/
							iPeriod += direction;
							continue;
						}

						if(mBasePeriodId != newPeriod->GetId() && AdState::OUTSIDE_ADBREAK == mCdaiObject->mAdState)
						{
							mBasePeriodOffset = 0;		//Not considering the delta from previous period's duration.
						}
						if(rate > 0)
						{
							if(AdState::OUTSIDE_ADBREAK != mCdaiObject->mAdState)	//If Adbreak (somehow) goes beyond the designated periods, period outside adbreak will have +ve duration. Avoiding catastrophic cases.
							{
								mBasePeriodOffset -= ((double)mCdaiObject->mPeriodMap[mBasePeriodId].duration)/1000.00;
							}
						}
						else
						{
							mBasePeriodOffset += ((double)aamp_GetPeriodDuration(mpd, iPeriod, mLastPlaylistDownloadTimeMs))/1000.00;	//Already reached -ve. Subtracting from current period duration
						}
						mCurrentPeriodIdx = iPeriod;
						mBasePeriodId = newPeriod->GetId();
						periodChanged = false; //If the playing period changes, it will be detected below [if(currentPeriodId != mCurrentPeriod->GetId())]
					}
					adStateChange = onAdEvent(AdEvent::DEFAULT);		//TODO: Vinod, We can optimize here.

					if(AdState::IN_ADBREAK_WAIT2CATCHUP == mCdaiObject->mAdState)
					{
						waitForAdBreakCatchup= true;
						break;
					}
					if(adStateChange && AdState::OUTSIDE_ADBREAK == mCdaiObject->mAdState)
					{
						//Just came out from the Adbreak. Need to search the right period
						for(iPeriod=0;iPeriod < numPeriods;  iPeriod++)
						{
							if(mBasePeriodId == mpd->GetPeriods().at(iPeriod)->GetId())
							{
								mCurrentPeriodIdx = iPeriod;
								AAMPLOG_INFO("%s:%d [CDAI] Landed at the periodId[%d] ",__FUNCTION__,__LINE__,mCurrentPeriodIdx);
								break;
							}
						}
					}
					if(AdState::IN_ADBREAK_AD_PLAYING != mCdaiObject->mAdState)
					{
						mCurrentPeriod = mpd->GetPeriods().at(mCurrentPeriodIdx);
					}

					vector <IAdaptationSet*> adapatationSets = mCurrentPeriod->GetAdaptationSets();
					int adaptationSetCount = adapatationSets.size();
					if(currentPeriodId != mCurrentPeriod->GetId())
					{
						logprintf("Period ID changed from \'%s\' to \'%s\' [BasePeriodId=\'%s\']", currentPeriodId.c_str(),mCurrentPeriod->GetId().c_str(), mBasePeriodId.c_str());
						currentPeriodId = mCurrentPeriod->GetId();
						mPrevAdaptationSetCount = adaptationSetCount;
						logprintf("playing period %d/%d", iPeriod, (int)numPeriods);
						//We are moving to new period so reset the lastsegment time
						for (int i = 0; i < mNumberOfTracks; i++)
						{
							mMediaStreamContext[i]->lastSegmentTime = 0;
						}
						requireStreamSelection = true;
						periodChanged = true;
					}
					else if(mPrevAdaptationSetCount != adaptationSetCount)
					{
						logprintf("Change in AdaptationSet count; adaptationSetCount %d  mPrevAdaptationSetCount %d,updating stream selection", adaptationSetCount, mPrevAdaptationSetCount);
						mPrevAdaptationSetCount = adaptationSetCount;
						requireStreamSelection = true;
					}
					else
					{
						for (int i = 0; i < mNumberOfTracks; i++)
						{
							if(mMediaStreamContext[i]->adaptationSetId != adapatationSets.at(mMediaStreamContext[i]->adaptationSetIdx)->GetId())
							{
								logprintf("AdaptationSet index changed; updating stream selection");
								requireStreamSelection = true;
							}
						}
					}
					adStateChange = false;

					if(requireStreamSelection)
					{
						StreamSelection();
					}

					// IsLive = 1 , resetTimeLineIndex = 1
					// InProgressCdvr (IsLive=1) , resetTimeLineIndex = 1
					// Vod/CDVR for PeriodChange , resetTimeLineIndex = 1
					if(AdState::IN_ADBREAK_AD_PLAYING != mCdaiObject->mAdState || (AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState && periodChanged))
					{
						bool resetTimeLineIndex = (mIsLiveStream || lastLiveFlag|| periodChanged);
						UpdateTrackInfo(true, periodChanged, resetTimeLineIndex);
					}


					if(mIsLiveStream || lastLiveFlag)
					{
						double culled = 0;
						if(mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled)
						{
							culled = GetCulledSeconds();
						}
						if(culled > 0)
						{
							AAMPLOG_INFO("%s:%d Culled seconds = %f", __FUNCTION__, __LINE__, culled);
							aamp->UpdateCullingState(culled);
							mCulledSeconds += culled;
						}
						auto durMs = aamp_GetDurationFromRepresentation(mpd);
						if(0 == durMs)
						{
						    mPeriodEndTime = GetPeriodEndTime(mpd, mCurrentPeriodIdx, mLastPlaylistDownloadTimeMs);
						    mPeriodStartTime = GetPeriodStartTime(mpd, mCurrentPeriodIdx);

							for(int periodIter = 0; periodIter < mpd->GetPeriods().size(); periodIter++)
							{
								durMs += aamp_GetPeriodDuration(mpd, periodIter, mLastPlaylistDownloadTimeMs);
							}
						}
						double duration = (double)durMs / 1000;
						aamp->UpdateDuration(duration);
						mLiveEndPosition = duration + mCulledSeconds;
						if(mCdaiObject->mContentSeekOffset)
						{
							AAMPLOG_INFO("%s:%d [CDAI]: Resuming channel playback at PeriodID[%s] at Position[%lf]",	__FUNCTION__, __LINE__, currentPeriodId.c_str(), mCdaiObject->mContentSeekOffset);
							//This seek should not be reflected in the fragmentTime, since we have already cached
							//same duration of ad content; So keep a copy of current fragmentTime so that it can be
							//updated back when seek is done
							double fragmentTime[AAMP_TRACK_COUNT];
							for (int i = 0; i < mNumberOfTracks; i++)
							{
								fragmentTime[i] = mMediaStreamContext[i]->fragmentTime;
							}
							SeekInPeriod(mCdaiObject->mContentSeekOffset);
							mCdaiObject->mContentSeekOffset = 0;
							for (int i = 0; i < mNumberOfTracks; i++)
							{
								mMediaStreamContext[i]->fragmentTime = fragmentTime[i];
							}
						}
					}

					lastLiveFlag = mIsLiveStream;
					/*Discontinuity handling on period change*/
					if (periodChanged && gpGlobalConfig->mpdDiscontinuityHandling && mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled &&
							(gpGlobalConfig->mpdDiscontinuityHandlingCdvr || (!aamp->IsInProgressCDVR())))
					{
						MediaStreamContext *pMediaStreamContext = mMediaStreamContext[eMEDIATYPE_VIDEO];
						ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
						bool ignoreDiscontinuity = false;

						if (!trickPlay)
						{
							ignoreDiscontinuity = (mMediaStreamContext[eMEDIATYPE_AUDIO] && !mMediaStreamContext[eMEDIATYPE_AUDIO]->enabled && mMediaStreamContext[eMEDIATYPE_AUDIO]->isFragmentInjectorThreadStarted());
						}

						if(ignoreDiscontinuity)
						{
							logprintf("%s:%d Error! Audio or Video track missing in period, ignoring discontinuity",	__FUNCTION__, __LINE__);
						}
						else
						{
							if (!segmentTemplate)
							{
								segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
							}
							if (segmentTemplate)
							{
								uint64_t segmentStartTime = GetFirstSegmentStartTime(mCurrentPeriod);
								if (segmentTemplate->GetSegmentTimeline() != NULL && nextSegmentTime != segmentStartTime)
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d discontinuity detected nextSegmentTime %" PRIu64 " FirstSegmentStartTime %" PRIu64 " ", __FUNCTION__, __LINE__, nextSegmentTime, segmentStartTime);
									discontinuity = true;
									mFirstPTS = (double)segmentStartTime/segmentTemplate->GetTimescale();
								}
								else
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d No discontinuity detected nextSegmentTime %" PRIu64 " FirstSegmentStartTime %" PRIu64 " ", __FUNCTION__, __LINE__, nextSegmentTime, segmentStartTime);
								}
							}
							else
							{
								traceprintf("PrivateStreamAbstractionMPD::%s:%d Segment template not available", __FUNCTION__, __LINE__);
							}
						}
					}
					FetchAndInjectInitialization(discontinuity);
					if(mCdaiObject->mAdFailed)
					{
						adStateChange = onAdEvent(AdEvent::AD_FAILED);
						mCdaiObject->mAdFailed = false;
						continue;
					}
					if(rate < 0 && periodChanged)
					{
						SkipToEnd(mMediaStreamContext[eMEDIATYPE_VIDEO]);
					}
				}

				double lastPrdOffset = mBasePeriodOffset;
				// playback
				while (!exitFetchLoop && !liveMPDRefresh)
				{
					bool bCacheFullState = true;
					for (int i = mNumberOfTracks-1; i >= 0; i--)
					{
						struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[i];
						if (pMediaStreamContext->adaptationSet )
						{
							if((pMediaStreamContext->numberOfFragmentsCached != gpGlobalConfig->maxCachedFragmentsPerTrack) && !(pMediaStreamContext->profileChanged))
							{	// profile not changed and Cache not full scenario
								if (!pMediaStreamContext->eos)
								{
									if(trickPlay && pMediaStreamContext->mDownloadedFragment.ptr == NULL)
									{
										if((rate > 0 && delta <= 0) || (rate < 0 && delta >= 0))
										{
											delta = rate / gpGlobalConfig->vodTrickplayFPS;
										}
										double currFragTime = pMediaStreamContext->fragmentTime;
										delta = SkipFragments(pMediaStreamContext, delta);
										mBasePeriodOffset += (pMediaStreamContext->fragmentTime - currFragTime);
									}

									if(PushNextFragment(pMediaStreamContext,i))
									{
										if (mIsLiveManifest)
										{
											mContext->CheckForPlaybackStall(true);
										}
										if((!pMediaStreamContext->mContext->trickplayMode) && (eMEDIATYPE_VIDEO == i) && (!aamp->IsTSBSupported()))
										{
											if (aamp->CheckABREnabled())
											{
												pMediaStreamContext->mContext->CheckForProfileChange();
											}
											else
											{
												pMediaStreamContext->mContext->CheckUserProfileChangeReq();
											}
										}
									}
									else if (pMediaStreamContext->eos == true && mIsLiveManifest && i == eMEDIATYPE_VIDEO)
									{
										mContext->CheckForPlaybackStall(false);
									}

									if (AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState && rate > 0 && !(pMediaStreamContext->eos)
											&& mCdaiObject->CheckForAdTerminate(pMediaStreamContext->fragmentTime - pMediaStreamContext->periodStartOffset))
									{
										//Ensuring that Ad playback doesn't go beyond Adbreak
										AAMPLOG_WARN("%s:%d: [CDAI] Adbreak ended early. Terminating Ad playback. fragmentTime[%lf] periodStartOffset[%lf]",
															__FUNCTION__, __LINE__, pMediaStreamContext->fragmentTime, pMediaStreamContext->periodStartOffset);
										pMediaStreamContext->eos = true;
									}
								}
							}
							// Fetch init header for both audio and video ,after mpd refresh(stream selection) , profileChanged = true for both tracks .
							// Need to reset profileChanged flag which is done inside FetchAndInjectInitialization
							// Without resetting profileChanged flag , fetch of audio was stopped causing audio drop
							// DELIA-32017
							else if(pMediaStreamContext->profileChanged)
							{	// Profile changed case
								FetchAndInjectInitialization();
							}

							if(pMediaStreamContext->numberOfFragmentsCached != gpGlobalConfig->maxCachedFragmentsPerTrack)
							{
								bCacheFullState = false;
							}

						}
						if (!aamp->DownloadsAreEnabled())
						{
							exitFetchLoop = true;
							bCacheFullState = false;
							break;
						}
					}// end of for loop
					// BCOM-2959  -- Exit from fetch loop for period to be done only after audio and video fetch
					// While playing CDVR with EAC3 audio , durations doesnt match and only video downloads are seen leaving audio behind
					// Audio cache is always full and need for data is not received for more fetch.
					// So after video downloads loop was exiting without audio fetch causing audio drop .
					// Now wait for both video and audio to reach EOS before moving to next period or exit.
					bool vEos = mMediaStreamContext[eMEDIATYPE_VIDEO]->eos;
					bool audioEnabled = (mMediaStreamContext[eMEDIATYPE_AUDIO] && mMediaStreamContext[eMEDIATYPE_AUDIO]->enabled);
					bool aEos = (audioEnabled && mMediaStreamContext[eMEDIATYPE_AUDIO]->eos);
					if (vEos || aEos)
					{
						bool eosOutSideAd = (AdState::IN_ADBREAK_AD_PLAYING != mCdaiObject->mAdState &&
								((rate > 0 && mCurrentPeriodIdx >= (numPeriods -1)) || (rate < 0 && 0 == mCurrentPeriodIdx)));

						bool eosAdPlayback = (AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState &&
								((rate > 0 && mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentTime >= mLiveEndPosition)
								||(rate < 0 && mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentTime <= 0)));

						if((!mIsLiveStream || (rate != AAMP_NORMAL_PLAY_RATE))
							&& (eosOutSideAd || eosAdPlayback))
						{
							if(vEos)
							{
								mMediaStreamContext[eMEDIATYPE_VIDEO]->eosReached = true;
								mMediaStreamContext[eMEDIATYPE_VIDEO]->AbortWaitForCachedAndFreeFragment(false);
							}
							if(audioEnabled)
							{
								if(mMediaStreamContext[eMEDIATYPE_AUDIO]->eos)
								{
									mMediaStreamContext[eMEDIATYPE_AUDIO]->eosReached = true;
									mMediaStreamContext[eMEDIATYPE_AUDIO]->AbortWaitForCachedAndFreeFragment(false);
								}
							}
							else
							{	// No Audio enabled , fake the flag to true
								aEos = true;
							}
						}
						else
						{
							if(!audioEnabled)
							{
								aEos = true;
							}
						}
						// If audio and video reached EOS then only break the fetch loop .
						if(vEos && aEos)
						{
							AAMPLOG_INFO("%s:%d EOS - Exit fetch loop ", __FUNCTION__, __LINE__);
							if(AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState)
							{
								adStateChange = onAdEvent(AdEvent::AD_FINISHED);
							}
							break;
						}
					}
					if (AdState::OUTSIDE_ADBREAK != mCdaiObject->mAdState)
					{
						Period2AdData &curPeriod = mCdaiObject->mPeriodMap[mBasePeriodId];
						if((rate < 0 && mBasePeriodOffset <= 0 ) ||
								(rate > 0 && curPeriod.filled && curPeriod.duration <= (uint64_t)(mBasePeriodOffset * 1000)))
						{
							AAMPLOG_INFO("%s:%d [CDAI]: BasePeriod[%s] completed @%lf. Changing to next ",__FUNCTION__,__LINE__, mBasePeriodId.c_str(),mBasePeriodOffset);
							break;
						}
						else if(lastPrdOffset != mBasePeriodOffset && AdState::IN_ADBREAK_AD_NOT_PLAYING == mCdaiObject->mAdState)
						{
							//In adbreak, but somehow Ad is not playing. Need to check whether the position reached the next Ad start.
							adStateChange = onAdEvent(AdEvent::BASE_OFFSET_CHANGE);
							if(adStateChange)
								break;
						}
						lastPrdOffset = mBasePeriodOffset;
					}
					int timeoutMs =  MAX_DELAY_BETWEEN_MPD_UPDATE_MS - (int)(aamp_GetCurrentTimeMS() - mLastPlaylistDownloadTimeMs);
					if(timeoutMs <= 0 && mIsLiveManifest && rate > 0)
					{
						liveMPDRefresh = true;
						break;
					}
					else if(bCacheFullState)
					{
						// play cache is full , wait until cache is available to inject next, max wait of 1sec
						int timeoutMs = 200;
						AAMPLOG_TRACE("%s:%d Cache full state,no download until(%d) Time(%lld)",__FUNCTION__, __LINE__,timeoutMs,aamp_GetCurrentTimeMS());
						mMediaStreamContext[eMEDIATYPE_VIDEO]->WaitForFreeFragmentAvailable(timeoutMs);
					}
					else
					{	// This sleep will hit when there is no content to download and cache is not full
						// and refresh interval timeout not reached . To Avoid tight loop adding a min delay
						aamp->InterruptableMsSleep(50);
					}
				} // Loop 3: end of while loop (!exitFetchLoop && !liveMPDRefresh)
				if(liveMPDRefresh)
				{
					break;
				}
				if(AdState::IN_ADBREAK_WAIT2CATCHUP == mCdaiObject->mAdState)
				{
					continue; //Need to finish all the ads in current before period change
				}
				if(rate > 0)
				{
					iPeriod++;
				}
				else
				{
					iPeriod--;
				}
			} //Loop 2: End of Period while loop
			if (exitFetchLoop || (rate < AAMP_NORMAL_PLAY_RATE && iPeriod < 0) || (rate > 1 && iPeriod >= numPeriods) || (!mIsLiveManifest && waitForAdBreakCatchup != true))
			{
				break;
			}
		}
		else
		{
			logprintf("PrivateStreamAbstractionMPD::%s:%d - null mpd", __FUNCTION__, __LINE__);
		}


		// If it comes here , two reason a) Reached eos b) Need livempdUpdate
		// If liveMPDRefresh is true , that means it already reached 6 sec timeout .
		// 		No more further delay required for mpd update .
		// If liveMPDRefresh is false, then it hit eos . Here the timeout is calculated based
		// on the buffer availability.
		if (!liveMPDRefresh && mLastPlaylistDownloadTimeMs)
		{
			int minDelayBetweenPlaylistUpdates = (int)mMinUpdateDurationMs;
			int timeSinceLastPlaylistDownload = (int)(aamp_GetCurrentTimeMS() - mLastPlaylistDownloadTimeMs);
			long long currentPlayPosition = aamp->GetPositionMilliseconds();
			long long endPositionAvailable = (aamp->culledSeconds + aamp->durationSeconds)*1000;
			// playTarget value will vary if TSB is full and trickplay is attempted. Cant use for buffer calculation
			// So using the endposition in playlist - Current playing position to get the buffer availability
			long bufferAvailable = (endPositionAvailable - currentPlayPosition);

			// If buffer Available is > 2*mMinUpdateDurationMs
			if(bufferAvailable  > (mMinUpdateDurationMs*2) )
			{
				// may be 1.0 times also can be set ???
				minDelayBetweenPlaylistUpdates = (int)(1.5 * mMinUpdateDurationMs);
			}
			// if buffer is between 2*target & mMinUpdateDurationMs
			else if(bufferAvailable  > mMinUpdateDurationMs)
			{
				minDelayBetweenPlaylistUpdates = (int)(0.5 * mMinUpdateDurationMs);
			}
			// This is to handle the case where target duration is high value(>Max delay)  but buffer is available just above the max update inteval
			else if(bufferAvailable > (2*MAX_DELAY_BETWEEN_MPD_UPDATE_MS))
			{
				minDelayBetweenPlaylistUpdates = MAX_DELAY_BETWEEN_MPD_UPDATE_MS;
			}
			// if buffer < targetDuration && buffer < MaxDelayInterval
			else
			{
				// if bufferAvailable is less than targetDuration ,its in RED alert . Close to freeze
				// need to refresh soon ..
				if(bufferAvailable)
				{
					minDelayBetweenPlaylistUpdates = (int)(bufferAvailable / 3) ;
				}
				else
				{
					minDelayBetweenPlaylistUpdates = MIN_DELAY_BETWEEN_MPD_UPDATE_MS; // 500mSec
				}
				// limit the logs when buffer is low
				{
					static int bufferlowCnt;
					if((bufferlowCnt++ & 5) == 0)
					{
						logprintf("Buffer is running low(%ld).Refreshing playlist(%d).PlayPosition(%lld) End(%lld)",
							bufferAvailable,minDelayBetweenPlaylistUpdates,currentPlayPosition,endPositionAvailable);
					}
				}

			}

			// First cap max limit ..
			// remove already consumed time from last update
			// if time interval goes negative, limit to min value

			// restrict to Max delay interval
			if (minDelayBetweenPlaylistUpdates > MAX_DELAY_BETWEEN_MPD_UPDATE_MS)
			{
				minDelayBetweenPlaylistUpdates = MAX_DELAY_BETWEEN_MPD_UPDATE_MS;
			}

			// adjust with last refreshed time interval
			minDelayBetweenPlaylistUpdates -= timeSinceLastPlaylistDownload;

			if(minDelayBetweenPlaylistUpdates < MIN_DELAY_BETWEEN_MPD_UPDATE_MS)
			{
				// minimum of 500 mSec needed to avoid too frequent download.
				minDelayBetweenPlaylistUpdates = MIN_DELAY_BETWEEN_MPD_UPDATE_MS;
			}

			AAMPLOG_INFO("aamp playlist end refresh bufferMs(%ld) delay(%d) delta(%d) End(%lld) PlayPosition(%lld)",
				bufferAvailable,minDelayBetweenPlaylistUpdates,timeSinceLastPlaylistDownload,endPositionAvailable,currentPlayPosition);

			// sleep before next manifest update
			aamp->InterruptableMsSleep(minDelayBetweenPlaylistUpdates);
		}
		if (!aamp->DownloadsAreEnabled() || UpdateMPD() != eAAMPSTATUS_OK)
		{
			break;
		}

		if(mIsFogTSB)
		{
			//Periods could be added or removed, So select period based on periodID
			//If period ID not found in MPD that means it got culled, in that case select
			// first period
			AAMPLOG_INFO("Updating period index after mpd refresh");
			vector<IPeriod *> periods = mpd->GetPeriods();
			int iter = periods.size() - 1;
			mCurrentPeriodIdx = 0;
			while(iter > 0)
			{
				if(mBasePeriodId == periods.at(iter)->GetId())
				{
					mCurrentPeriodIdx = iter;
					break;
				}
				iter--;
			}
		}
		else
		{
			// DELIA-31750 - looping of cdvr video - Issue happens with multiperiod content only
			// When playback is near live position (last period) or after eos in period
			// mCurrentPeriodIdx was resetted to 0 . This caused fetch loop to continue from Period 0/fragement 1
			// Reset of mCurrentPeriodIdx to be done to max period if Period count changes after mpd refresh
			size_t newPeriods = mpd->GetPeriods().size();
			if(mCurrentPeriodIdx > (newPeriods - 1))
			{
				logprintf("MPD Fragment Collector detected reset in Period(New Size:%d)(currentIdx:%d->%d)",
					newPeriods,mCurrentPeriodIdx,newPeriods - 1);
				mCurrentPeriodIdx = newPeriods - 1;
			}
		}
		mpdChanged = true;
	}		//Loop 1
	while (!exitFetchLoop);
	logprintf("MPD fragment collector done");
}


/**
 * @brief StreamAbstractionAAMP_MPD Constructor
 * @param aamp pointer to PrivateInstanceAAMP object associated with player
 * @param seek_pos Seek position
 * @param rate playback rate
 */
StreamAbstractionAAMP_MPD::StreamAbstractionAAMP_MPD(class PrivateInstanceAAMP *aamp,double seek_pos, float rate): StreamAbstractionAAMP(aamp), mPriv(NULL)
{
	mPriv = new PrivateStreamAbstractionMPD( this, aamp, seek_pos, rate);
	trickplayMode = (rate != AAMP_NORMAL_PLAY_RATE);
}


/**
 * @brief StreamAbstractionAAMP_MPD Destructor
 */
StreamAbstractionAAMP_MPD::~StreamAbstractionAAMP_MPD()
{
	delete mPriv;
}

/**
 *   @brief  Starts streaming.
 */
void StreamAbstractionAAMP_MPD::Start(void)
{
	mPriv->Start();
}


/**
 *   @brief  Starts streaming.
 */
void PrivateStreamAbstractionMPD::Start(void)
{
#ifdef AAMP_MPD_DRM
	aamp->mDRMSessionManager->setSessionMgrState(SessionMgrState::eSESSIONMGR_ACTIVE);
#endif
	pthread_create(&fragmentCollectorThreadID, NULL, &FragmentCollector, this);
	fragmentCollectorThreadStarted = true;
	for (int i=0; i< mNumberOfTracks; i++)
	{
		if(aamp->IsPlayEnabled())
		{
			mMediaStreamContext[i]->StartInjectLoop();
		}
	}
}

/**
*   @brief  Stops streaming.
*
*   @param  clearChannelData - ignored.
*/
void StreamAbstractionAAMP_MPD::Stop(bool clearChannelData)
{
	aamp->DisableDownloads();
	ReassessAndResumeAudioTrack(true);
	AbortWaitForAudioTrackCatchup();
	mPriv->Stop();
	aamp->EnableDownloads();
}


/**
*   @brief  Stops streaming.
*/
void PrivateStreamAbstractionMPD::Stop()
{
	for (int iTrack = 0; iTrack < mNumberOfTracks; iTrack++)
	{
		MediaStreamContext *track = mMediaStreamContext[iTrack];
		if(track && track->Enabled())
		{
			track->AbortWaitForCachedAndFreeFragment(true);
			track->StopInjectLoop();
		}
	}
	if(drmSessionThreadStarted)
	{
		AAMPLOG_INFO("Waiting to join CreateDRMSession thread");
		int rc = pthread_join(createDRMSessionThreadID, NULL);
		if (rc != 0)
		{
			logprintf("pthread_join returned %d for createDRMSession Thread", rc);
		}
		AAMPLOG_INFO("Joined CreateDRMSession thread");
		drmSessionThreadStarted = false;
	}
	if(fragmentCollectorThreadStarted)
	{
		int rc = pthread_join(fragmentCollectorThreadID, NULL);
		if (rc != 0)
		{
			logprintf("%s:%d ***pthread_join failed, returned %d", __FUNCTION__, __LINE__, rc);
		}
		fragmentCollectorThreadStarted = false;
	}
	aamp->mStreamSink->ClearProtectionEvent();
 #ifdef AAMP_MPD_DRM
	aamp->mDRMSessionManager->setSessionMgrState(SessionMgrState::eSESSIONMGR_INACTIVE);
  #endif
}

/**
 * @brief PrivateStreamAbstractionMPD Destructor
 */
PrivateStreamAbstractionMPD::~PrivateStreamAbstractionMPD(void)
{
	for (int iTrack = 0; iTrack < mNumberOfTracks; iTrack++)
	{
		MediaStreamContext *track = mMediaStreamContext[iTrack];
		if(track )
		{
			delete track;
		}
	}

	if(lastProcessedKeyId)
	{
		free(lastProcessedKeyId);
	}

	aamp->SyncBegin();
	if (mpd)
	{
		delete mpd;
		mpd = NULL;
	}

	if(mStreamInfo)
	{
		delete[] mStreamInfo;
	}

	aamp->CurlTerm(eCURLINSTANCE_VIDEO, AAMP_TRACK_COUNT);

	aamp->SyncEnd();
}


/**
 * @brief Stub implementation
 */
void StreamAbstractionAAMP_MPD::DumpProfiles(void)
{ // STUB
}


/**
 *   @brief Get output format of stream.
 *
 *   @param[out]  primaryOutputFormat - format of primary track
 *   @param[out]  audioOutputFormat - format of audio track
 */
void PrivateStreamAbstractionMPD::GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat)
{
	if(mMediaStreamContext[eMEDIATYPE_VIDEO] && mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled )
	{
		primaryOutputFormat = FORMAT_ISO_BMFF;
	}
	else
	{
		primaryOutputFormat = FORMAT_NONE;
	}
	if(mMediaStreamContext[eMEDIATYPE_AUDIO] && mMediaStreamContext[eMEDIATYPE_AUDIO]->enabled )
	{
		audioOutputFormat = FORMAT_ISO_BMFF;
	}
	else
	{
		audioOutputFormat = FORMAT_NONE;
	}
}


/**
 * @brief Get output format of stream.
 *
 * @param[out]  primaryOutputFormat - format of primary track
 * @param[out]  audioOutputFormat - format of audio track
 */
void StreamAbstractionAAMP_MPD::GetStreamFormat(StreamOutputFormat &primaryOutputFormat, StreamOutputFormat &audioOutputFormat)
{
	mPriv->GetStreamFormat(primaryOutputFormat, audioOutputFormat);
}


/**
 *   @brief Return MediaTrack of requested type
 *
 *   @param[in]  type - track type
 *   @retval MediaTrack pointer.
 */
MediaTrack* StreamAbstractionAAMP_MPD::GetMediaTrack(TrackType type)
{
	return mPriv->GetMediaTrack(type);
}


double StreamAbstractionAAMP_MPD::GetBufferedDuration()
{
	MediaTrack *video = mPriv->GetMediaTrack(eTRACK_VIDEO);
	double retval = -1.0; 
	if (video && video->enabled)
	{
		retval = video->GetBufferedDuration();
	}
	return retval;
}


/**
 *   @brief Return MediaTrack of requested type
 *
 *   @param[in]  type - track type
 *   @retval MediaTrack pointer.
 */
MediaTrack* PrivateStreamAbstractionMPD::GetMediaTrack(TrackType type)
{
	return mMediaStreamContext[type];
}


/**
 * @brief Get current stream position.
 *
 * @retval current position of stream.
 */
double StreamAbstractionAAMP_MPD::GetStreamPosition()
{
	return mPriv->GetStreamPosition();
}

/**
 * @brief Gets number of profiles
 * @retval number of profiles
 */
int PrivateStreamAbstractionMPD::GetProfileCount()
{
	int ret = 0;
	bool isFogTsb = mIsFogTSB && !mAdPlayingFromCDN;

	if(isFogTsb)
	{
		ret = mBitrateIndexMap.size();
	}
	else
	{
		ret = mContext->GetProfileCount();
	}
	return ret;
}

/**
 *   @brief Get stream information of a profile from subclass.
 *
 *   @param[in]  idx - profile index.
 *   @retval stream information corresponding to index.
 */
StreamInfo* PrivateStreamAbstractionMPD::GetStreamInfo(int idx)
{
	assert(idx < GetProfileCount());
	return &mStreamInfo[idx];
}


/**
 *   @brief Get stream information of a profile from subclass.
 *
 *   @param[in]  idx - profile index.
 *   @retval stream information corresponding to index.
 */
StreamInfo* StreamAbstractionAAMP_MPD::GetStreamInfo(int idx)
{
	return mPriv->GetStreamInfo(idx);
}


/**
 *   @brief  Get PTS of first sample.
 *
 *   @retval PTS of first sample
 */
double StreamAbstractionAAMP_MPD::GetFirstPTS()
{
	return mPriv->GetFirstPTS();
}


/**
 *   @brief  Get PTS of first sample.
 *
 *   @retval PTS of first sample
 */
double PrivateStreamAbstractionMPD::GetFirstPTS()
{
	return mFirstPTS;
}


/**
 * @brief Get index corresponding to bitrate
 * @param bitrate Stream's bitrate
 * @retval Bandwidth index
 */
int PrivateStreamAbstractionMPD::GetBWIndex(long bitrate)
{
	int topBWIndex = 0;
	int profileCount = GetProfileCount();
	if (profileCount)
	{
		for (int i = 0; i < profileCount; i++)
		{
			StreamInfo *streamInfo = &mStreamInfo[i];
			if (!streamInfo->isIframeTrack && streamInfo->bandwidthBitsPerSecond > bitrate)
			{
				--topBWIndex;
			}
		}
	}
	return topBWIndex;
}


/**
 * @brief Get index of profile corresponds to bandwidth
 * @param[in] bitrate Bitrate to lookup profile
 * @retval profile index
 */
int StreamAbstractionAAMP_MPD::GetBWIndex(long bitrate)
{
	return mPriv->GetBWIndex(bitrate);
}


/**
 * @brief To get the available video bitrates.
 * @ret available video bitrates
 */
std::vector<long> PrivateStreamAbstractionMPD::GetVideoBitrates(void)
{
	std::vector<long> bitrates;
	int profileCount = GetProfileCount();
	bitrates.reserve(profileCount);
	if (profileCount)
	{
		for (int i = 0; i < profileCount; i++)
		{
			StreamInfo *streamInfo = &mStreamInfo[i];
			if (!streamInfo->isIframeTrack)
			{
				bitrates.push_back(streamInfo->bandwidthBitsPerSecond);
			}
		}
	}
	return bitrates;
}


/**
 * @brief To get the available audio bitrates.
 * @ret available audio bitrates
 */
std::vector<long> PrivateStreamAbstractionMPD::GetAudioBitrates(void)
{
	//TODO: Impl getter for audio bitrates
	return std::vector<long>();
}


/**
 * @brief To get the available video bitrates.
 * @ret available video bitrates
 */
std::vector<long> StreamAbstractionAAMP_MPD::GetVideoBitrates(void)
{
	return mPriv->GetVideoBitrates();
}

/*
* @brief Gets Max Bitrate avialable for current playback.
* @ret long MAX video bitrates
*/
long StreamAbstractionAAMP_MPD::GetMaxBitrate()
{
	long maxBitrate = 0;
	if(mPriv->IsTSBUsed())
	{
		maxBitrate = mPriv->GetMaxTSBBandwidth();
	}
	else
	{

		maxBitrate = StreamAbstractionAAMP::GetMaxBitrate();
	}

	return maxBitrate;
}


/**
 * @brief To get the available audio bitrates.
 * @ret available audio bitrates
 */
std::vector<long> StreamAbstractionAAMP_MPD::GetAudioBitrates(void)
{
	return mPriv->GetAudioBitrates();
}


/**
*   @brief  Stops injecting fragments to StreamSink.
*/
void StreamAbstractionAAMP_MPD::StopInjection(void)
{
	//invoked at times of discontinuity. Audio injection loop might have already exited here
	ReassessAndResumeAudioTrack(true);
	mPriv->StopInjection();
}


/**
*   @brief  Stops injection.
*/
void PrivateStreamAbstractionMPD::StopInjection(void)
{
	for (int iTrack = 0; iTrack < mNumberOfTracks; iTrack++)
	{
		MediaStreamContext *track = mMediaStreamContext[iTrack];
		if(track && track->Enabled())
		{
			track->AbortWaitForCachedFragment();
			aamp->StopTrackInjection((MediaType) iTrack);
			track->StopInjectLoop();
		}
	}
}


/**
*   @brief  Start injecting fragments to StreamSink.
*/
void StreamAbstractionAAMP_MPD::StartInjection(void)
{
	mTrackState = eDISCONTIUITY_FREE;
	mPriv->StartInjection();
}


/**
*   @brief  Start injection.
*/
void PrivateStreamAbstractionMPD::StartInjection(void)
{
	for (int iTrack = 0; iTrack < mNumberOfTracks; iTrack++)
	{
		MediaStreamContext *track = mMediaStreamContext[iTrack];
		if(track && track->Enabled())
		{
			aamp->ResumeTrackInjection((MediaType) iTrack);
			track->StartInjectLoop();
		}
	}
}


void StreamAbstractionAAMP_MPD::SetCDAIObject(CDAIObject *cdaiObj)
{
	mPriv->SetCDAIObject(cdaiObj);
}

void PrivateStreamAbstractionMPD::SetCDAIObject(CDAIObject *cdaiObj)
{
	if(cdaiObj)
	{
		CDAIObjectMPD *cdaiObjMpd = static_cast<CDAIObjectMPD *>(cdaiObj);
		mCdaiObject = cdaiObjMpd->GetPrivateCDAIObjectMPD();
	}
}

bool PrivateStreamAbstractionMPD::isAdbreakStart(IPeriod *period, uint32_t &duration, uint64_t &startMS, std::string &scte35)
{
	const std::vector<IEventStream *> &eventStreams = period->GetEventStreams();
	for(auto &eventStream: eventStreams)
	{
		for(auto &event: eventStream->GetEvents())
		{
			if(event && event->GetDuration())
			{
				for(auto &evtChild: event->GetAdditionalSubNodes())
				{
					std::string prefix = "scte35:";

					if(evtChild->HasAttribute("xmlns") && "http://www.scte.org/schemas/35/2016" == evtChild->GetAttributeValue("xmlns"))
					{
						//scte35 namespace defined here. Hence, this & children don't need the prefix 'scte35'
						prefix = "";
					}

					if(prefix+"Signal" == evtChild->GetName())
					{
						for(auto &signalChild: evtChild->GetNodes())
						{
							if(signalChild && prefix+"Binary" == signalChild->GetName())
							{
								uint32_t timeScale = 1;
								if(eventStream->GetTimescale() > 1)
								{
									timeScale = eventStream->GetTimescale();
								}
								//first multiply then divide to avoid magnifying rounding off issues.
								duration = ((uint64_t)event->GetDuration()*1000)/timeScale; //milliseconds
								scte35 = signalChild->GetText();
								if(scte35.length())
								{
									return true;
								}
							}
						}
						AAMPLOG_WARN("%s:%d [CDAI]: Found a scte35:Signal in manifest without scte35:Binary!!",__FUNCTION__,__LINE__);
					}
				}
			}
		}
	}
	return false;
}
bool PrivateStreamAbstractionMPD::onAdEvent(AdEvent evt)
{
	double adOffset;
	return onAdEvent(evt, adOffset);
}

bool PrivateStreamAbstractionMPD::onAdEvent(AdEvent evt, double &adOffset)
{
	if(!(gpGlobalConfig->enableClientDai))
	{
		return false;
	}
	std::lock_guard<std::mutex> lock(mCdaiObject->mDaiMtx);
	bool stateChanged = false;
	AdState oldState = mCdaiObject->mAdState;
	AAMPEventType reservationEvt2Send = AAMP_MAX_NUM_EVENTS; //None
	std::string adbreakId2Send("");
	AAMPEventType placementEvt2Send = AAMP_MAX_NUM_EVENTS; //None
	std::string adId2Send("");
	uint32_t adPos2Send = 0;
	bool sendImmediate = false;
	switch(mCdaiObject->mAdState)
	{
		case AdState::OUTSIDE_ADBREAK:
			if(AdEvent::DEFAULT == evt || AdEvent::INIT == evt)
			{
				std::string brkId = "";
				int adIdx = mCdaiObject->CheckForAdStart(rate, (AdEvent::INIT == evt), mBasePeriodId, mBasePeriodOffset, brkId, adOffset);
				if(!brkId.empty())
				{
					AAMPLOG_INFO("%s:%d [CDAI] CheckForAdStart found Adbreak. adIdx[%d] mBasePeriodOffset[%lf] adOffset[%lf].",__FUNCTION__,__LINE__, adIdx, mBasePeriodOffset, adOffset);

					mCdaiObject->mCurPlayingBreakId = brkId;
					if(-1 != adIdx && mCdaiObject->mAdBreaks[brkId].ads)
					{
						if(!(mCdaiObject->mAdBreaks[brkId].ads->at(adIdx).invalid))
						{
							AAMPLOG_WARN("%s:%d [CDAI]: STARTING ADBREAK[%s] AdIdx[%d] Found at Period[%s].",__FUNCTION__,__LINE__, brkId.c_str(), adIdx, mBasePeriodId.c_str());
							mCdaiObject->mCurAds = mCdaiObject->mAdBreaks[brkId].ads;

							mCdaiObject->mCurAdIdx = adIdx;
							mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_PLAYING;

							for(int i=0; i<adIdx; i++)
								adPos2Send += mCdaiObject->mCurAds->at(i).duration;
						}
						else
						{
							AAMPLOG_WARN("%s:%d [CDAI]: AdIdx[%d] in the AdBreak[%s] is invalid. Skipping.",__FUNCTION__,__LINE__, adIdx, brkId.c_str());
						}
						reservationEvt2Send = AAMP_EVENT_AD_RESERVATION_START;
						adbreakId2Send = brkId;
						if(AdEvent::INIT == evt) sendImmediate = true;
					}

					if(AdState::IN_ADBREAK_AD_PLAYING != mCdaiObject->mAdState)
					{
						AAMPLOG_WARN("%s:%d [CDAI]: BasePeriodId[%s] in Adbreak[%s]. But Ad not available.",__FUNCTION__,__LINE__, mBasePeriodId.c_str(), brkId.c_str());
						mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING;
					}
					stateChanged = true;
				}
			}
			break;
		case AdState::IN_ADBREAK_AD_NOT_PLAYING:
			if(AdEvent::BASE_OFFSET_CHANGE == evt || AdEvent::PERIOD_CHANGE == evt)
			{
				std::string brkId = "";
				int adIdx = mCdaiObject->CheckForAdStart(rate, false, mBasePeriodId, mBasePeriodOffset, brkId, adOffset);
				if(-1 != adIdx && mCdaiObject->mAdBreaks[brkId].ads)
				{
					if(0 == adIdx && 0 != mBasePeriodOffset)
					{
						//Ad is ready; but it is late. Invalidate.
						mCdaiObject->mAdBreaks[brkId].ads->at(0).invalid = true;
					}
					if(!(mCdaiObject->mAdBreaks[brkId].ads->at(adIdx).invalid))
					{
						AAMPLOG_WARN("%s:%d [CDAI]: AdIdx[%d] Found at Period[%s].",__FUNCTION__,__LINE__, adIdx, mBasePeriodId.c_str());
						mCdaiObject->mCurAds = mCdaiObject->mAdBreaks[brkId].ads;

						mCdaiObject->mCurAdIdx = adIdx;
						mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_PLAYING;

						for(int i=0; i<adIdx; i++)
							adPos2Send += mCdaiObject->mCurAds->at(i).duration;
						stateChanged = true;
					}
					if(adIdx == (mCdaiObject->mAdBreaks[brkId].ads->size() -1))	//Rewind case only.
					{
						reservationEvt2Send = AAMP_EVENT_AD_RESERVATION_START;
						adbreakId2Send = brkId;
					}
				}
				else if(brkId.empty())
				{
					AAMPLOG_WARN("%s:%d [CDAI]: ADBREAK[%s] ENDED. Playing the basePeriod[%s].",__FUNCTION__,__LINE__, mCdaiObject->mCurPlayingBreakId.c_str(), mBasePeriodId.c_str());
					mCdaiObject->mCurPlayingBreakId = "";
					mCdaiObject->mCurAds = nullptr;
					mCdaiObject->mCurAdIdx = -1;
					//Base content playing already. No need to jump to offset again.
					mCdaiObject->mAdState = AdState::OUTSIDE_ADBREAK;
					stateChanged = true;
				}
			}
			break;
		case AdState::IN_ADBREAK_AD_PLAYING:
			if(AdEvent::AD_FINISHED == evt)
			{
				AAMPLOG_WARN("%s:%d [CDAI]: Ad[idx=%d] finished at Period[%s]. Waiting to catchup the base offset..",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str());
				mCdaiObject->mAdState = AdState::IN_ADBREAK_WAIT2CATCHUP;

				placementEvt2Send = AAMP_EVENT_AD_PLACEMENT_END;
				AdNode &adNode =  mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx);
				adId2Send = adNode.adId;
				for(int i=0; i <= mCdaiObject->mCurAdIdx; i++)
					adPos2Send += mCdaiObject->mCurAds->at(i).duration;
				stateChanged = true;
			}
			else if(AdEvent::AD_FAILED == evt)
			{
				mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).invalid = true;
				logprintf("%s:%d [CDAI]: Ad[idx=%d] Playback failed. Going to the base period[%s] at offset[%lf].",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str(), mBasePeriodOffset);
				mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING; //TODO: Vinod, It should be IN_ADBREAK_WAIT2CATCHUP, But you need to fix the catchup check logic.

				placementEvt2Send = AAMP_EVENT_AD_PLACEMENT_ERROR;	//Followed by AAMP_EVENT_AD_PLACEMENT_END
				AdNode &adNode =  mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx);
				adId2Send = adNode.adId;
				sendImmediate = true;
				adPos2Send = 0; //TODO: Vinod, Fix it
				stateChanged = true;
			}

			if(stateChanged)
			{
				for (int i = 0; i < mNumberOfTracks; i++)
				{
					//Resetting the manifest Url in track contexts
					mMediaStreamContext[i]->fragmentDescriptor.manifestUrl = aamp->GetManifestUrl();
				}
			}
			break;
		case AdState::IN_ADBREAK_WAIT2CATCHUP:
			if(-1 == mCdaiObject->mCurAdIdx)
			{
				logprintf("%s:%d [CDAI]: AdIdx[-1]. BUG! BUG!! BUG!!! We should not come here.",__FUNCTION__,__LINE__);
				mCdaiObject->mCurPlayingBreakId = "";
				mCdaiObject->mCurAds = nullptr;
				mCdaiObject->mCurAdIdx = -1;
				mCdaiObject->mContentSeekOffset = mBasePeriodOffset;
				mCdaiObject->mAdState = AdState::OUTSIDE_ADBREAK;
				stateChanged = true;
				break;
			}
			//In every event, we need to check this.But do it only on the begining of the fetcher loop. Hence it is the default event
			if(AdEvent::DEFAULT == evt)
			{
				if(!(mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).placed)) //TODO: Vinod, Need to wait till the base period offset is available. 'placed' won't help in case of rewind.
				{
					break;
				}
				//Wait till placement of current ad is completed
				AAMPLOG_WARN("%s:%d [CDAI]: Current Ad placement Completed. Ready to play next Ad.",__FUNCTION__,__LINE__);
				mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_READY2PLAY;
			}
		case AdState::IN_ADBREAK_AD_READY2PLAY:
			if(AdEvent::DEFAULT == evt)
			{
				bool curAdFailed = mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).invalid;	//TODO: Vinod, may need to check boundary.

				if(rate >= AAMP_NORMAL_PLAY_RATE)
					mCdaiObject->mCurAdIdx++;
				else
					mCdaiObject->mCurAdIdx--;
				if(mCdaiObject->mCurAdIdx >= 0 && mCdaiObject->mCurAdIdx < mCdaiObject->mCurAds->size())
				{
					if(mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).invalid)
					{
						AAMPLOG_WARN("%s:%d [CDAI]: AdIdx[%d] in invalid. Skipping!!.",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx);
						mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING;
					}
					else
					{
						AAMPLOG_WARN("%s:%d [CDAI]: Next AdIdx[%d] Found at Period[%s].",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str());
						mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_PLAYING;

						for(int i=0; i<mCdaiObject->mCurAdIdx; i++)
							adPos2Send += mCdaiObject->mCurAds->at(i).duration;
					}
					stateChanged = true;
				}
				else
				{
					if(rate > 0)
					{
						mBasePeriodId =	mCdaiObject->mAdBreaks[mCdaiObject->mCurPlayingBreakId].endPeriodId;
						mCdaiObject->mContentSeekOffset = (double)(mCdaiObject->mAdBreaks[mCdaiObject->mCurPlayingBreakId].endPeriodOffset)/ 1000;
					}
					else
					{
						// mCdaiObject->mCurPlayingBreakId is the first period in the Adbreak. Set the previous period as mBasePeriodId to play
						std::string prevPId = "";
						size_t numPeriods = mpd->GetPeriods().size();
						for(size_t iPeriod=0;iPeriod < numPeriods;  iPeriod++)
						{
							const std::string &pId = mpd->GetPeriods().at(iPeriod)->GetId();
							if(mCdaiObject->mCurPlayingBreakId == pId)
							{
								break;
							}
							prevPId = pId;
						}
						if(!prevPId.empty())
						{
							mBasePeriodId = prevPId;
						} //else, it should play the mBasePeriodId
						mCdaiObject->mContentSeekOffset = 0; //Should continue tricking from the end of the previous period.
					}
					AAMPLOG_WARN("%s:%d [CDAI]: All Ads in the ADBREAK[%s] FINISHED. Playing the basePeriod[%s] at Offset[%lf].",__FUNCTION__,__LINE__, mCdaiObject->mCurPlayingBreakId.c_str(), mBasePeriodId.c_str(), mCdaiObject->mContentSeekOffset);
					reservationEvt2Send = AAMP_EVENT_AD_RESERVATION_END;
					adbreakId2Send = mCdaiObject->mCurPlayingBreakId;
					sendImmediate = curAdFailed;	//Current Ad failed. Hence may not get discontinuity from gstreamer.
					mCdaiObject->mCurPlayingBreakId = "";
					mCdaiObject->mCurAds = nullptr;
					mCdaiObject->mCurAdIdx = -1;
					mCdaiObject->mAdState = AdState::OUTSIDE_ADBREAK;	//No more offset check needed. Hence, changing to OUTSIDE_ADBREAK
					stateChanged = true;
				}
			}
			break;
		default:
			break;
	}
	if(stateChanged)
	{
		mAdPlayingFromCDN = false;
		bool fogManifestFailed = false;
		if(AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState)
		{
			AdNode &adNode = mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx);
			if(NULL == adNode.mpd)
			{
				//Need to ensure that mpd is available, if not available, download it (mostly from FOG)
				bool finalManifest = false;
				adNode.mpd = mCdaiObject->GetAdMPD(adNode.url, finalManifest, false);

				if(NULL == adNode.mpd)
				{
					AAMPLOG_WARN("%s:%d [CDAI]: Ad playback failed. Not able to download Ad manifest from FOG.",__FUNCTION__,__LINE__);
					mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING;
					fogManifestFailed = true;
					if(AdState::IN_ADBREAK_AD_NOT_PLAYING == oldState)
					{
						stateChanged = false;
					}
				}
			}
			if(adNode.mpd)
			{
				mCurrentPeriod = adNode.mpd->GetPeriods().at(0);
				/* TODO: Fix redundancy from UpdateTrackInfo */
				for (int i = 0; i < mNumberOfTracks; i++)
				{
					mMediaStreamContext[i]->fragmentDescriptor.manifestUrl = adNode.url.c_str();
				}

				placementEvt2Send = AAMP_EVENT_AD_PLACEMENT_START;
				adId2Send = adNode.adId;

				map<string, string> mpdAttributes = adNode.mpd->GetRawAttributes();
				if(mpdAttributes.find("fogtsb") == mpdAttributes.end())
				{
					//No attribute 'fogtsb' in MPD. Hence, current ad is from CDN
					mAdPlayingFromCDN = true;
				}
			}
		}

		if(stateChanged)
		{
			AAMPLOG_WARN("%s:%d [CDAI]: State changed from [%s] => [%s].",__FUNCTION__,__LINE__, ADSTATE_STR[static_cast<int>(oldState)],ADSTATE_STR[static_cast<int>(mCdaiObject->mAdState)]);
		}

		if(AAMP_NORMAL_PLAY_RATE == rate)
		{
			//Sending Ad events
			uint64_t resPosMS = 0;
			if(AAMP_EVENT_AD_RESERVATION_START == reservationEvt2Send || AAMP_EVENT_AD_RESERVATION_END == reservationEvt2Send)
			{
				const std::string &startStr = mpd->GetPeriods().at(mCurrentPeriodIdx)->GetStart();
				if(!startStr.empty())
				{
					ParseISO8601Duration(startStr.c_str(), resPosMS);
				}
				resPosMS += (uint64_t)(mBasePeriodOffset * 1000);
			}

			if(AAMP_EVENT_AD_RESERVATION_START == reservationEvt2Send)
			{
				aamp->SendAdReservationEvent(reservationEvt2Send,adbreakId2Send, resPosMS, sendImmediate);
				aamp->SendAnomalyEvent(ANOMALY_TRACE, "[CDAI] Adbreak of duration=%u sec starts.", (mCdaiObject->mAdBreaks[mCdaiObject->mCurPlayingBreakId].brkDuration)/1000);
			}

			if(AAMP_EVENT_AD_PLACEMENT_START == placementEvt2Send || AAMP_EVENT_AD_PLACEMENT_END == placementEvt2Send || AAMP_EVENT_AD_PLACEMENT_ERROR == placementEvt2Send)
			{
				uint32_t adDuration = 30000;
				if(AAMP_EVENT_AD_PLACEMENT_START == placementEvt2Send)
				{
					adDuration = mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).duration;
					adPos2Send += adOffset;
					aamp->SendAnomalyEvent(ANOMALY_TRACE, "[CDAI] AdId=%s starts. Duration=%u sec URL=%s",
							adId2Send.c_str(),(adDuration/1000), mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).url.c_str());
				}

				aamp->SendAdPlacementEvent(placementEvt2Send,adId2Send, adPos2Send, adOffset, adDuration, sendImmediate);
				if(fogManifestFailed)
				{
					aamp->SendAdPlacementEvent(AAMP_EVENT_AD_PLACEMENT_ERROR,adId2Send, adPos2Send, adOffset, adDuration, true);
				}
				if(AAMP_EVENT_AD_PLACEMENT_ERROR == placementEvt2Send || fogManifestFailed)
				{
					aamp->SendAdPlacementEvent(AAMP_EVENT_AD_PLACEMENT_END,adId2Send, adPos2Send, adOffset, adDuration, true);	//Ad ended with error
					aamp->SendAnomalyEvent(ANOMALY_ERROR, "[CDAI] AdId=%s encountered error.", adId2Send.c_str());
				}
			}

			if(AAMP_EVENT_AD_RESERVATION_END == reservationEvt2Send)
			{
				aamp->SendAdReservationEvent(reservationEvt2Send,adbreakId2Send, resPosMS, sendImmediate);
				aamp->SendAnomalyEvent(ANOMALY_TRACE, "%s", "[CDAI] Adbreak ends.");
			}
		}
	}
	return stateChanged;
}



