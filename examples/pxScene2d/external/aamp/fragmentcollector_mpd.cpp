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

#include "fragmentcollector_mpd.h"
#include "priv_aamp.h"
#include "AampDRMSessionManager.h"
#include <stdlib.h>
#include <string.h>
#include "_base64.h"
#include "libdash/IMPD.h"
#include "libdash/INode.h"
#include "libdash/IDASHManager.h"
#include "libdash/xml/Node.h"
#include "libdash/helpers/Time.h"
#include "libdash/xml/DOMParser.h"
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <set>
#include <iomanip>
#include <ctime>
#include <inttypes.h>
#include <libxml/xmlreader.h>
#include <math.h>
#include "AampCacheHandler.h"
#include <algorithm>
//#define DEBUG_TIMELINE
//#define AAMP_HARVEST_SUPPORT_ENABLED
//#define AAMP_DISABLE_INJECT
//#define HARVEST_MPD

using namespace dash;
using namespace std;
using namespace dash::mpd;
using namespace dash::xml;
using namespace dash::helpers;

/**
 * @addtogroup AAMP_COMMON_TYPES
 * @{
 */

/**
 * @brief Ad playback states
 */
enum class AdState
{
	OUTSIDE_ADBREAK,		//Not in adbreak, wait for period change
	IN_ADBREAK_AD_NOT_PLAYING,	//Base period in adbreak: But Ad not found/playing
	IN_ADBREAK_AD_PLAYING,		//Ad playing
	IN_ADBREAK_AD_READY2PLAY,	//Ready to play next Ad
	IN_ADBREAK_WAIT2CATCHUP		//Waiting for base period to catchup
};

static constexpr const char *ADSTATE_STR[] =
{
	(const char *)"OUTSIDE_ADBREAK",
	(const char *)"IN_ADBREAK_AD_NOT_PLAYING",
	(const char *)"IN_ADBREAK_AD_PLAYING",
	(const char *)"IN_ADBREAK_AD_READY2PLAY",
	(const char *)"IN_ADBREAK_WAIT2CATCHUP"
};

/**
 * @brief Ad event types
 */
enum class AdEvent
{
	INIT,
	BASE_OFFSET_CHANGE,
	AD_FINISHED,
	AD_FAILED,
	PERIOD_CHANGE,
	DEFAULT = PERIOD_CHANGE
};

#define OFFSET_ALIGN_FACTOR 2000 //Observed minor slacks in the ad durations. Align factor used to place the ads correctly.

/**
 * @struct AdNode
 * @brief Individual Ad's meta info
 */
struct AdNode {
	bool invalid;		//Failed to play first time.
	bool placed;  		//Ad completely placed on the period
	std::string adId;
	std::string url;
	uint64_t duration;
	std::string basePeriodId;
	int basePeriodOffset;
	MPD* mpd;

	AdNode() : invalid(false), placed(false), adId(), url(), duration(0), basePeriodId(), basePeriodOffset(0), mpd(nullptr)
	{
	
	}

	AdNode(bool invalid, bool placed, std::string adId, std::string url, uint64_t duration,
									std::string basePeriodId, int basePeriodOffset, MPD* mpd)
	: invalid(invalid), placed(placed), adId(adId), url(url), duration(duration), basePeriodId(basePeriodId),
		basePeriodOffset(basePeriodOffset), mpd(mpd)
	{

	}

	AdNode(const AdNode& adNode) : invalid(adNode.invalid), placed(adNode.placed), adId(adNode.adId),
									url(adNode.url), duration(adNode.duration), basePeriodId(adNode.basePeriodId),
									basePeriodOffset(adNode.basePeriodOffset), mpd(adNode.mpd)
	{
	}

	AdNode& operator=(const AdNode&) = delete;
};

/**
 * @struct AdBreakObject
 * @brief AdBreak's metadata
 */
struct AdBreakObject{
	uint32_t duration;
	std::shared_ptr<std::vector<AdNode>> ads;
	std::string endPeriodId;
	uint64_t endPeriodOffset;	//Last ad's end position stretches till here

	AdBreakObject() : duration(0), ads(), endPeriodId(), endPeriodOffset(0)
	{
	}

	AdBreakObject(uint32_t duration, std::shared_ptr<std::vector<AdNode>> ads, std::string endPeriodId, 
	uint64_t endPeriodOffset)
	: duration(duration), ads(ads), endPeriodId(endPeriodId), endPeriodOffset(endPeriodOffset)
	{
	}
};

/**
 * @struct AdOnPeriod
 * @brief Individual Ad's object falling in each period
 */
struct AdOnPeriod 
{
	int32_t adIdx;
	uint32_t adStartOffset;
};

/**
 * @struct Period2AdData
 * @brief Meta info corresponding to each period.
 */
struct Period2AdData {
	bool filled;				//Period filled with ads or not
	std::string adBreakId;		//Parent Adbreak
	uint64_t duration;			//Period Duration
	std::map<int, AdOnPeriod> offset2Ad;

	Period2AdData() : filled(false), adBreakId(), duration(0), offset2Ad()
	{
	}
};

/**
 * @struct AdFulfillObj
 * @brief Temporary object representing current fulfilling ad.
 */
struct AdFulfillObj {
	std::string periodId;
	std::string adId;
	std::string url;

	AdFulfillObj() : periodId(), adId(), url()
	{

	}
};

/**
 * @struct PlacementObj
 * @brief Current placing Ad's object
 */
struct PlacementObj {
	std::string pendingAdbrkId;			//Only one Adbreak will be pending for replacement
	std::string openPeriodId;			//The period in the adbreak that is progressing
	uint64_t 	curEndNumber;			//Current periods last fragment number
	int			curAdIdx;				//Currently placing ad, during MPD progression
	uint32_t 	adNextOffset;			//Current periods last fragment number

	PlacementObj(): pendingAdbrkId(), openPeriodId(), curEndNumber(0), curAdIdx(-1), adNextOffset(0)
	{

	}
};	//Keeping current placement status. Not used for playback

/**
 * @class PrivateCDAIObjectMPD
 * @brief Private Client Side DAI object for DASH
 */
class PrivateCDAIObjectMPD
{
public:
	PrivateInstanceAAMP* mAamp;
	std::mutex mDaiMtx;				/**< Mutex protecting DAI critical section */
	bool 	mIsFogTSB;				/**< Channel playing from TSB or not */
	std::unordered_map<std::string, AdBreakObject> mAdBreaks;
	std::unordered_map<std::string, Period2AdData> mPeriodMap;	//periodId -> (periodClosed, vector<<Ad, FragStartIdx, FragEndIdx>>)
	std::string mCurPlayingBreakId;
	pthread_t mAdObjThreadID;
	bool mAdFailed;
	std::shared_ptr<std::vector<AdNode>> mCurAds;
	int mCurAdIdx;
	AdFulfillObj mAdFulfillObj;
	PlacementObj mPlacementObj;
	double mContentSeekOffset;
	AdState mAdState;

	PrivateCDAIObjectMPD(PrivateInstanceAAMP* aamp);	//Ctor
	~PrivateCDAIObjectMPD();	//Dtor

	PrivateCDAIObjectMPD(const PrivateCDAIObjectMPD&) = delete;
	PrivateCDAIObjectMPD& operator= (const PrivateCDAIObjectMPD&) = delete;

	void SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS);
	void FulFillAdObject();
	MPD*  GetAdMPD(std::string &url, bool &finalManifest, bool tryFog = false);
	void InsertToPeriodMap(IPeriod *period);
	inline bool isPeriodExist(const std::string &periodId);
	inline bool isAdBreakObjectExist(const std::string &adBrkId);
	void PrunePeriodMaps(std::vector<std::string> &newPeriodIds);
	void ResetState();
	void ClearMaps();
	void  PlaceAds(dash::mpd::IMPD *mpd);
	int CheckForAdStart(bool continuePlay, const std::string &periodId, double offSet, std::string &breakId, double &adOffset);
	inline bool isPeriodInAdbreak(const std::string &periodId);
};

CDAIObjectMPD::CDAIObjectMPD(PrivateInstanceAAMP* aamp): CDAIObject(aamp), mPrivObj(new PrivateCDAIObjectMPD(aamp))
{

}

CDAIObjectMPD::~CDAIObjectMPD()
{
	delete mPrivObj;
}

#define MAX_ID_SIZE 1024
#define SEGMENT_COUNT_FOR_ABR_CHECK 5
#define PLAYREADY_SYSTEM_ID "9a04f079-9840-4286-ab92-e65be0885f95"
#define WIDEVINE_SYSTEM_ID "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"
#define DEFAULT_INTERVAL_BETWEEN_MPD_UPDATES_MS 3000
#define TIMELINE_START_RESET_DIFF 4000000000
#define MAX_DELAY_BETWEEN_MPD_UPDATE_MS (6000)
#define MIN_DELAY_BETWEEN_MPD_UPDATE_MS (500) // 500mSec

//Comcast DRM Agnostic CENC for Content Metadata
#define COMCAST_DRM_INFO_ID "afbcb50e-bf74-3d13-be8f-13930c783962"

/**
 * @struct FragmentDescriptor
 * @brief Stores information of dash fragment
 */
struct FragmentDescriptor
{
	const char *manifestUrl;
	const std::vector<IBaseUrl *>*baseUrls;
	uint32_t Bandwidth;
	char RepresentationID[MAX_ID_SIZE]; // todo: retrieve from representation instead of making a copy
	uint64_t Number;
	uint64_t Time;
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


static const char *mMediaTypeName[] = { "video", "audio" };

#ifdef AAMP_HARVEST_SUPPORT_ENABLED
#ifdef USE_PLAYERSINKBIN
#define HARVEST_BASE_PATH "/media/tsb/aamp-harvest/" // SD card friendly path
#else
#define HARVEST_BASE_PATH "aamp-harvest/"
#endif
static void GetFilePath(char filePath[MAX_URI_LENGTH], const FragmentDescriptor *fragmentDescriptor, std::string media);
static void WriteFile(char* fileName, const char* data, int len);
#endif // AAMP_HARVEST_SUPPORT_ENABLED

uint64_t GetPeriodNewContentDuration(IPeriod * period, uint64_t &curEndNumber);
uint64_t GetPeriodDuration(IPeriod * period);

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
			eos(false), fragmentTime(0),targetDnldPosition(0), index_ptr(NULL), index_len(0),
			lastSegmentTime(0), lastSegmentNumber(0), adaptationSetIdx(0), representationIndex(0), profileChanged(true),
			adaptationSetId(0), fragmentDescriptor(), mContext(context), initialization(""), mDownloadedFragment()
	{
		memset(&fragmentDescriptor, 0, sizeof(FragmentDescriptor));
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
	bool CacheFragment(const char *fragmentUrl, unsigned int curlInstance, double position, double duration, const char *range = NULL, bool initSegment = false, bool discontinuity = false
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
			char effectiveUrl[MAX_URI_LENGTH];
			effectiveUrl[0] = 0;
			int iFogError = -1;
			ret = aamp->LoadFragment(bucketType, fragmentUrl,effectiveUrl, &cachedFragment->fragment, curlInstance,
						range, actualType, &http_code, &bitrate, & iFogError);

			if (aamp->rate != AAMP_NORMAL_PLAY_RATE)
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
			AAMPLOG_INFO("%s:%d Bitrate changed from %ld to %ld\n", __FUNCTION__, __LINE__, fragmentDescriptor.Bandwidth, bitrate);
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
				logprintf("%s:%d LoadFragment failed\n", __FUNCTION__, __LINE__);

				if (initSegment)
				{
					logprintf("%s:%d Init fragment fetch failed. fragmentUrl %s\n", __FUNCTION__, __LINE__, fragmentUrl);
					if(!playingAd)
					{
						aamp->SendDownloadErrorEvent(AAMP_TUNE_INIT_FRAGMENT_DOWNLOAD_FAILURE, http_code);
					}
				}
				else
				{
					segDLFailCount += 1;
					if (MAX_SEG_DOWNLOAD_FAIL_COUNT <= segDLFailCount)
					{
						logprintf("%s:%d Not able to download fragments; reached failure threshold sending tune failed event\n",
								__FUNCTION__, __LINE__);
						if(!playingAd)	//If playingAd, we are invalidating the current Ad in onAdEvent().
						{
							aamp->SendDownloadErrorEvent(AAMP_TUNE_FRAGMENT_DOWNLOAD_FAILURE, http_code);
						}
					} 
					// DELIA-32287 - Profile RampDown check and rampdown is needed only for Video . If audio fragment download fails 
					// should continue with next fragment,no retry needed .
					else if ((eTRACK_VIDEO == type) && mContext->CheckForRampDownProfile(http_code))
					{
						mContext->mCheckForRampdown = true;
						logprintf( "PrivateStreamAbstractionMPD::%s:%d > Error while fetching fragment:%s, failedCount:%d. decrementing profile\n",
								__FUNCTION__, __LINE__, fragmentUrl, segDLFailCount);
					}
					else if (AAMP_IS_LOG_WORTHY_ERROR(http_code))
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d > Error on fetching %s fragment. failedCount:%d\n",
								__FUNCTION__, __LINE__, name, segDLFailCount);
					}
				}
			}
		}
		else
		{
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
			if (aamp->HarvestFragments())
			{
				char fileName[MAX_URI_LENGTH];
				strcpy(fileName, fragmentUrl);
				GetFilePath(fileName, &fragmentDescriptor, media);
				logprintf("%s:%d filePath %s\n", __FUNCTION__, __LINE__, fileName);
				WriteFile(fileName, cachedFragment->fragment.ptr, cachedFragment->fragment.len);
			}
#endif
			cachedFragment->position = position;
			cachedFragment->duration = duration;
			cachedFragment->discontinuity = discontinuity;
#ifdef AAMP_DEBUG_INJECT
			if (discontinuity)
			{
				logprintf("%s:%d Discontinuous fragment\n", __FUNCTION__, __LINE__);
			}
			if ((1 << type) & AAMP_DEBUG_INJECT)
			{
				strcpy(cachedFragment->uri, fragmentUrl);
			}
#endif
			segDLFailCount = 0;
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
			logprintf("PrivateStreamAbstractionMPD::%s:%d - ABR %dx%d[%d] -> %dx%d[%d]\n", __FUNCTION__, __LINE__,
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
			strcpy(fragmentDescriptor.RepresentationID, representation->GetId().c_str());
			profileChanged = true;
		}
		else
		{
			traceprintf("PrivateStreamAbstractionMPD::%s:%d - Not switching ABR %dx%d[%d] \n", __FUNCTION__, __LINE__,
					representation->GetWidth(), representation->GetHeight(), representation->GetBandwidth());
		}

	}

	/**
	 * @brief Notify discontinuity during trick-mode as PTS re-stamping is done in sink
	 */
	void SignalTrickModeDiscontinuity()
	{
		aamp->SignalTrickModeDiscontinuity();
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
	GrowableBuffer mDownloadedFragment;

	double fragmentTime;
	double targetDnldPosition;
	char *index_ptr;
	size_t index_len;
	uint64_t lastSegmentTime;
	uint64_t lastSegmentNumber;
	int adaptationSetIdx;
	int representationIndex;
	StreamAbstractionAAMP_MPD* mContext;
	std::string initialization;
	uint32_t adaptationSetId;
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

/**
 * @struct DrmSessionParams
 * @brief Holds data regarding drm session
 */
struct DrmSessionParams
{
	unsigned char *initData;
	int initDataLen;
	MediaType stream_type;
	PrivateInstanceAAMP *aamp;
	bool isWidevine;
	unsigned char *contentMetadata;
};

static bool IsIframeTrack(IAdaptationSet *adaptationSet);


PrivateCDAIObjectMPD::PrivateCDAIObjectMPD(PrivateInstanceAAMP* aamp) : mAamp(aamp),mDaiMtx(), mIsFogTSB(false), mAdBreaks(), mPeriodMap(), mCurPlayingBreakId(), mAdObjThreadID(0), mAdFailed(false), mCurAds(nullptr),
					mCurAdIdx(-1), mContentSeekOffset(0), mAdState(AdState::OUTSIDE_ADBREAK),mPlacementObj(), mAdFulfillObj() 
{
	mAamp->CurlInit(AAMP_DAI_CURL_IDX, 1);
}

PrivateCDAIObjectMPD::~PrivateCDAIObjectMPD()
{
	if(mAdObjThreadID)
	{
		int rc = pthread_join(mAdObjThreadID, NULL);
		if (rc != 0)
		{
			logprintf("%s:%d ***pthread_join failed, returned %d\n", __FUNCTION__, __LINE__, rc);
		}
		mAdObjThreadID = 0;
	}
	mAamp->CurlTerm(AAMP_DAI_CURL_IDX, 1);
}

void PrivateCDAIObjectMPD::InsertToPeriodMap(IPeriod * period)
{
	const std::string &prdId = period->GetId();
	if(!isPeriodExist(prdId))
	{
		mPeriodMap[prdId] = Period2AdData();
	}
}

inline bool PrivateCDAIObjectMPD::isPeriodExist(const std::string &periodId)
{
	return (mPeriodMap.end() != mPeriodMap.find(periodId))?true:false;
}

inline bool PrivateCDAIObjectMPD::isAdBreakObjectExist(const std::string &adBrkId)
{
	return (mAdBreaks.end() != mAdBreaks.find(adBrkId))?true:false;
}


void PrivateCDAIObjectMPD::PrunePeriodMaps(std::vector<std::string> &newPeriodIds)
{
	//Erase all adbreaks other than new adbreaks
	for (auto it = mAdBreaks.begin(); it != mAdBreaks.end();) {
		if ((mPlacementObj.pendingAdbrkId != it->first) && (mCurPlayingBreakId != it->first) &&//We should not remove the pending/playing adbreakObj
				(newPeriodIds.end() == std::find(newPeriodIds.begin(), newPeriodIds.end(), it->first))) {
			auto &adBrkObj = *it;
			logprintf("%s:%d [CDAI] Removing the period[%s] from mAdBreaks.\n", __FUNCTION__, __LINE__, adBrkObj.first.c_str());
			auto adNodes = adBrkObj.second.ads;
			for(AdNode &ad: *adNodes)
			{
				if(ad.mpd)
				{
					delete ad.mpd;
				}
			}
			it = mAdBreaks.erase(it);
		} else {
			++it;
		}
	}

	//Erase all periods other than new periods
	for (auto it = mPeriodMap.begin(); it != mPeriodMap.end();) {
		if (newPeriodIds.end() == std::find(newPeriodIds.begin(), newPeriodIds.end(), it->first)) {
			it = mPeriodMap.erase(it);
		} else {
			++it;
		}
	}
}

void PrivateCDAIObjectMPD::ResetState()
{
	 //TODO: Vinod, maybe we can move these playback state variables to PrivateStreamAbstractionMPD
	 mIsFogTSB = false;
	 mCurPlayingBreakId = "";
	 mAdFailed = false;
	 mCurAds = nullptr;
	 mCurAdIdx = -1;
	 mContentSeekOffset = 0;
	 mAdState = AdState::OUTSIDE_ADBREAK;
}

void PrivateCDAIObjectMPD::ClearMaps()
{
	std::unordered_map<std::string, AdBreakObject> tmpMap;
	std::swap(mAdBreaks,tmpMap);
	for(auto &adBrkObj: tmpMap)
	{
		auto adNodes = adBrkObj.second.ads;
		for(AdNode &ad: *adNodes)
		{
			if(ad.mpd)
			{
				delete ad.mpd;
			}
		}
	}

	mPeriodMap.clear();
}

void  PrivateCDAIObjectMPD::PlaceAds(dash::mpd::IMPD *mpd)
{
	bool placed = false;
	//Populate the map to specify the period boundaries
	if(mpd && (-1 != mPlacementObj.curAdIdx) && "" != mPlacementObj.pendingAdbrkId && isAdBreakObjectExist(mPlacementObj.pendingAdbrkId)) //Some Ad is still waiting for the placement
	{
		bool openPrdFound = false;

		AdBreakObject &abObj = mAdBreaks[mPlacementObj.pendingAdbrkId];
		vector<IPeriod *> periods = mpd->GetPeriods();
		for(auto period: periods)
		{
			const std::string &periodId = period->GetId();
			//We need to check, open period is available in the manifest. Else, something wrong
			if(mPlacementObj.openPeriodId == periodId)
			{
				openPrdFound = true;
			}
			else if(openPrdFound)
			{
				if(GetPeriodDuration(period) > 0)
				{
					//Previous openPeriod ended. New period in the adbreak will be the new open period
					mPeriodMap[mPlacementObj.openPeriodId].filled = true;
					mPlacementObj.openPeriodId = periodId;
					mPlacementObj.curEndNumber = 0;
				}
				else
				{
					continue;		//Empty period may come early; excluding them
				}
			}

			if(openPrdFound && -1 != mPlacementObj.curAdIdx)
			{
				uint64_t periodDelta = GetPeriodNewContentDuration(period, mPlacementObj.curEndNumber);
				Period2AdData& p2AdData = mPeriodMap[periodId];

				if("" == p2AdData.adBreakId)
				{
					//New period opened
					p2AdData.adBreakId = mPlacementObj.pendingAdbrkId;
					p2AdData.offset2Ad[0] = AdOnPeriod{mPlacementObj.curAdIdx,mPlacementObj.adNextOffset};
				}

				p2AdData.duration += periodDelta;

				while(periodDelta > 0)
				{
					AdNode &curAd = abObj.ads->at(mPlacementObj.curAdIdx);
					if("" == curAd.basePeriodId)
					{
						//Next ad started placing
						curAd.basePeriodId = periodId;
						curAd.basePeriodOffset = p2AdData.duration - periodDelta;
						int offsetKey = curAd.basePeriodOffset;
						offsetKey = offsetKey - (offsetKey%OFFSET_ALIGN_FACTOR);
						p2AdData.offset2Ad[offsetKey] = AdOnPeriod{mPlacementObj.curAdIdx,0};	//At offsetKey of the period, new Ad starts
					}
					if(periodDelta < (curAd.duration - mPlacementObj.adNextOffset))
					{
						mPlacementObj.adNextOffset += periodDelta;
						periodDelta = 0;
					}
					else
					{
						//Current Ad completely placed. But more space available in the current period for next Ad
						curAd.placed = true;
						periodDelta -= (curAd.duration - mPlacementObj.adNextOffset);
						mPlacementObj.curAdIdx++;
						if(mPlacementObj.curAdIdx < abObj.ads->size())
						{
							mPlacementObj.adNextOffset = 0; //New Ad's offset
						}
						else
						{
							mPlacementObj.curAdIdx = -1;
							//Place the end markers of adbreak
							abObj.endPeriodId = periodId;	//If it is the exact period boundary, end period will be the next one
							abObj.endPeriodOffset = p2AdData.duration - periodDelta;
							if(abObj.endPeriodOffset < 5000)
							{
								abObj.endPeriodOffset = 0;//Aligning the last period
								mPeriodMap[abObj.endPeriodId] = Period2AdData(); //Resetting the period with small outlier.
							}
							//TODO: else We need to calculate duration of the end period in the Adbreak

							//Printing the placement positions
							std::stringstream ss;
							ss<<"{AdbreakId: "<<mPlacementObj.pendingAdbrkId;
							ss<<", duration: "<<abObj.duration;
							ss<<", endPeriodId: "<<abObj.endPeriodId;
							ss<<", endPeriodOffset: "<<abObj.endPeriodOffset;
							ss<<", #Ads: "<<abObj.ads->size() << ",[";
							for(int k=0;k<abObj.ads->size();k++)
							{
								AdNode &ad = abObj.ads->at(k);
								ss<<"\n{AdIdx:"<<k <<",AdId:"<<ad.adId<<",duration:"<<ad.duration<<",basePeriodId:"<<ad.basePeriodId<<", basePeriodOffset:"<<ad.basePeriodOffset<<"},";
							}
							ss<<"],\nUnderlyingPeriods:[ ";
							for(auto it = mPeriodMap.begin();it != mPeriodMap.end();it++)
							{
								if(it->second.adBreakId == mPlacementObj.pendingAdbrkId)
								{
									ss<<"\n{PeriodId:"<<it->first<<", duration:"<<it->second.duration;
									for(auto pit = it->second.offset2Ad.begin(); pit != it->second.offset2Ad.end() ;pit++)
									{
										ss<<", offset["<<pit->first<<"]=> Ad["<<pit->second.adIdx<<"@"<<pit->second.adStartOffset<<"]";
									}
								}
							}
							ss<<"]}";
							logprintf("%s:%d [CDAI] Placement Done: %s.\n", __FUNCTION__, __LINE__, ss.str().c_str());
							break;
						}
					}
				}
			}
		}
		if(-1 == mPlacementObj.curAdIdx)
		{
			mPlacementObj.pendingAdbrkId = "";
			mPlacementObj.openPeriodId = "";
			mPlacementObj.curEndNumber = 0;
			mPlacementObj.adNextOffset = 0;
		}
	}
}

int PrivateCDAIObjectMPD::CheckForAdStart(bool continuePlay, const std::string &periodId, double offSet, std::string &breakId, double &adOffset)
{
	int adIdx = -1;
	auto pit = mPeriodMap.find(periodId);
	if(mPeriodMap.end() != pit && !(pit->second.adBreakId.empty()))
	{
		//mBasePeriodId belongs to an Adbreak. Now we need to any Ad is placed in the offset.
		Period2AdData &curP2Ad = pit->second;
		breakId = curP2Ad.adBreakId;
		if(isAdBreakObjectExist(breakId))
		{
			if(continuePlay)
			{
				int floorKey = (int)(offSet * 1000);
				floorKey = floorKey - (floorKey%OFFSET_ALIGN_FACTOR);
				auto adIt = curP2Ad.offset2Ad.find(floorKey);
				if(curP2Ad.offset2Ad.end() == adIt)
				{
					//Considering only Ad start
					int ceilKey = floorKey + OFFSET_ALIGN_FACTOR;
					adIt = curP2Ad.offset2Ad.find(ceilKey);
				}

				if((curP2Ad.offset2Ad.end() != adIt) && (0 == adIt->second.adStartOffset))
				{
					//Considering only Ad start
					adIdx = adIt->second.adIdx;
					adOffset = 0;
				}
			}
			else	//Discrete playback
			{
				AdBreakObject &abObj = mAdBreaks[breakId];
				uint64_t key = (uint64_t)(offSet * 1000);
				uint64_t start = 0;
				uint64_t end = curP2Ad.duration;
				if(periodId ==  abObj.endPeriodId)
				{
					end = abObj.endPeriodOffset;	//No need to look beyond the adbreakEnd
				}

				if(key >= start && key <= end)
				{
					//Yes, Key is in Adbreak. Find which Ad.
					for(auto it = curP2Ad.offset2Ad.begin(); it != curP2Ad.offset2Ad.end(); it++)
					{
						if(key >= it->first)
						{
							adIdx = it->second.adIdx;
							adOffset = (double)((key - it->first)/1000);
						}
						else
						{
							break;
						}
					}
				}
			}
		}
	}
	return adIdx;
}

bool PrivateCDAIObjectMPD::isPeriodInAdbreak(const std::string &periodId)
{
	return !(mPeriodMap[periodId].adBreakId.empty());
}

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
	uint64_t GetPeriodEndTime();
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
	static Node* ProcessNode(xmlTextReaderPtr *reader, const char *url, bool isAd = false);
	static uint64_t GetDurationFromRepresentation(dash::mpd::IMPD *mpd);
	void SetCDAIObject(CDAIObject *cdaiObj);
	bool isAdbreakStart(IPeriod *period, uint32_t &duration, uint64_t &startMS, std::string &scte35);
	bool onAdEvent(AdEvent evt);
	bool onAdEvent(AdEvent evt, double &adOffset);
private:
	AAMPStatusType UpdateMPD(bool init = false);
	void FindTimedMetadata(MPD* mpd, Node* root, bool init = false);
	void ProcessPeriodSupplementalProperty(Node* node, std::string& AdID, uint64_t startMS, uint64_t durationMS);
	void ProcessPeriodAssetIdentifier(Node* node, uint64_t startMS, uint64_t durationMS, std::string& assetID, std::string& providerID);
	bool ProcessEventStream(uint64_t startMS, IPeriod * period);
	void ProcessStreamRestrictionList(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessStreamRestriction(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessStreamRestrictionExt(Node* node, const std::string& AdID, uint64_t startMS);
	void ProcessTrickModeRestriction(Node* node, const std::string& AdID, uint64_t startMS);
	void FetchAndInjectInitialization(bool discontinuity = false);
	void StreamSelection(bool newTune = false);
	bool CheckForInitalClearPeriod();
	void PushEncryptedHeaders();
	void UpdateTrackInfo(bool modifyDefaultBW, bool periodChanged, bool resetTimeLineIndex=false);
	double SkipFragments( MediaStreamContext *pMediaStreamContext, double skipTime, bool updateFirstPTS = false);
	void SkipToEnd( MediaStreamContext *pMediaStreamContext); //Added to support rewind in multiperiod assets
	void ProcessContentProtection(IAdaptationSet * adaptationSet,MediaType mediaType);
	void SeekInPeriod( double seekPositionSeconds);
	double GetCulledSeconds();
	void UpdateLanguageList();
	AAMPStatusType  GetMpdFromManfiest(const GrowableBuffer &manifest, MPD * &mpd, const char* manifestUrl, bool init = false);

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
	bool mIsLive;
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
	uint64_t mLastPlaylistDownloadTimeMs;
	double mFirstPTS;
	AudioType mAudioType;
	bool mPushEncInitFragment;
	int mPrevAdaptationSetCount;
	std::unordered_map<long, int> mBitrateIndexMap;
	bool mIsFogTSB;
	bool mIsIframeTrackPresent;
	vector<PeriodInfo> mMPDPeriodsInfo;
	IPeriod *mCurrentPeriod;
	std::string mBasePeriodId;
	double mBasePeriodOffset;
	PrivateCDAIObjectMPD *mCdaiObject;

	double mLiveEndPosition;
	double mCulledSeconds;
	bool mAdPlayingFromCDN;   /*Note: TRUE: Ad playing currently & from CDN. FALSE: Ad "maybe playing", but not from CDN.*/
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
	drmSessionThreadStarted(false), mpd(NULL), mNumberOfTracks(0), mCurrentPeriodIdx(0), mEndPosition(0), mIsLive(true), mContext(context),
	mStreamInfo(NULL), mPrevStartTimeSeconds(0), mPrevLastSegurlMedia(""), mPrevLastSegurlOffset(0), lastProcessedKeyId(NULL),
	lastProcessedKeyIdLen(0), mPeriodEndTime(0), mPeriodStartTime(0), mMinUpdateDurationMs(DEFAULT_INTERVAL_BETWEEN_MPD_UPDATES_MS),
	mLastPlaylistDownloadTimeMs(0), mFirstPTS(0), mAudioType(eAUDIO_UNKNOWN), mPushEncInitFragment(false),
	mPrevAdaptationSetCount(0), mBitrateIndexMap(), mIsFogTSB(false), mIsIframeTrackPresent(false), mMPDPeriodsInfo(),
	mCurrentPeriod(NULL), mBasePeriodId(""), mBasePeriodOffset(0), mCdaiObject(NULL), mLiveEndPosition(0), mCulledSeconds(0)
	,mAdPlayingFromCDN(false)
{
	this->aamp = aamp;
	memset(&mMediaStreamContext, 0, sizeof(mMediaStreamContext));
	mContext->GetABRManager().clearProfiles();
	mLastPlaylistDownloadTimeMs = aamp_GetCurrentTimeMS();
};


/**
 * @brief Check if mime type is compatible with media type
 * @param mimeType mime type
 * @param mediaType media type
 * @retval true if compatible
 */
static bool IsCompatibleMimeType(std::string mimeType, MediaType mediaType)
{
	switch ( mediaType )
	{
	case eMEDIATYPE_VIDEO:
		if (mimeType == "video/mp4")
		{
			return true;
		}
		break;

	case eMEDIATYPE_AUDIO:
		if (mimeType == "audio/webm")
		{
			return true;
		}
		if (mimeType == "audio/mp4")
		{
			return true;
		}
		break;
	}
	return false;
}


/**
 * @brief Get representation index of desired codec
 * @param adaptationSet Adaptation set object
 * @param[out] selectedRepType type of desired representation
 * @retval index of desired representation
 */
static int GetDesiredCodecIndex(IAdaptationSet *adaptationSet, AudioType &selectedRepType, uint32_t &selectedRepBandwidth)
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
		if (selectedRepType == eAUDIO_UNKNOWN && (audioType != eAUDIO_UNSUPPORTED || selectedRepBandwidth == 0) || // Select any profile for the first time, reject unsupported streams then
			(selectedRepType == audioType && bandwidth>selectedRepBandwidth) || // same type but better quality
			(selectedRepType < eAUDIO_ATMOS && audioType == eAUDIO_ATMOS && !gpGlobalConfig->disableATMOS && !gpGlobalConfig->disableEC3) || // promote to atmos
			(selectedRepType < eAUDIO_DDPLUS && audioType == eAUDIO_DDPLUS && !gpGlobalConfig->disableEC3) || // promote to ddplus
			(selectedRepType != eAUDIO_AAC && audioType == eAUDIO_AAC && gpGlobalConfig->disableEC3) || // force AAC
			(selectedRepType == eAUDIO_UNSUPPORTED) // anything better than nothing
			)
		{
			selectedRepIdx = representationIndex;
			selectedRepType = audioType;
			selectedRepBandwidth = bandwidth;
			AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d  > SelectedRepIndex : %d ,selectedRepType : %d, selectedRepBandwidth: %d\n", __FUNCTION__, __LINE__, selectedRepIdx, selectedRepType, selectedRepBandwidth);
		}
	}
	return selectedRepIdx;
}

static int GetVideoAdaptaionMinResolution(IAdaptationSet *adaptationSet, uint32_t &minWidth, uint32_t &minHeight)
{
	const std::vector<IRepresentation *> representation = adaptationSet->GetRepresentation();
	int selectedRepIdx = -1;
	for (int representationIndex = 0; representationIndex < representation.size(); representationIndex++)
	{
		const dash::mpd::IRepresentation *rep = representation.at(representationIndex);
		uint32_t width = rep->GetWidth();
		uint32_t height = rep->GetHeight();
		if(width < minWidth || height < minHeight)
		{
			minWidth = width;
			minHeight = height;
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
		logprintf("excluding muxed content\n");
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
static void GetFragmentUrl( char fragmentUrl[MAX_URI_LENGTH], const FragmentDescriptor *fragmentDescriptor, std::string media)
{
	std::string constructedUri;
	if (fragmentDescriptor->baseUrls->size() > 0)
	{
		constructedUri = fragmentDescriptor->baseUrls->at(0)->GetUrl();
		if(gpGlobalConfig->dashIgnoreBaseURLIfSlash)
		{
			if (constructedUri == "/")
			{
				logprintf("%s:%d ignoring baseurl /\n", __FUNCTION__, __LINE__);
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
		logprintf("%s:%d BaseURL not available\n", __FUNCTION__, __LINE__);
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
static void GetFilePath(char filePath[MAX_URI_LENGTH], const FragmentDescriptor *fragmentDescriptor, std::string media)
{
	std::string constructedUri = HARVEST_BASE_PATH;
	constructedUri += media;
	replace(constructedUri, "Bandwidth", fragmentDescriptor->Bandwidth);
	replace(constructedUri, "RepresentationID", fragmentDescriptor->RepresentationID);
	replace(constructedUri, "Number", fragmentDescriptor->Number);
	replace(constructedUri, "Time", fragmentDescriptor->Time);
	strcpy(filePath, constructedUri.c_str());
}


/**
 * @brief Write file to storage
 * @param fileName out file name
 * @param data buffer
 * @param len length of buffer
 */
static void WriteFile(char* fileName, const char* data, int len)
{
	struct stat st = { 0 };
	for (unsigned int i = 0; i < strlen(fileName); i++)
	{
		if (fileName[i] == '/')
		{
			fileName[i] = '\0';
			if (-1 == stat(fileName, &st))
			{
				mkdir(fileName, 0777);
			}
			fileName[i] = '/';
		}
	}
	FILE *fp = fopen(fileName, "wb");
	if (NULL == fp)
	{
		logprintf("File open failed. outfile = %s \n", fileName);
		return;
	}
	fwrite(data, len, 1, fp);
	fclose(fp);
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
	char fragmentUrl[MAX_URI_LENGTH];
	GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, media);
	size_t len = 0;
	float position;
	if(isInitializationSegment)
	{
		if(!(pMediaStreamContext->initialization.empty()) && (0 == strcmp(pMediaStreamContext->initialization.c_str(),(const char*)fragmentUrl))&& !discontinuity)
		{
			AAMPLOG_TRACE("We have pushed the same initailization segment for %s skipping\n", mMediaTypeName[pMediaStreamContext->type]);
			return retval;
		}
		else
		{
			pMediaStreamContext->initialization = std::string(fragmentUrl);
		}
		position = mFirstPTS;
	}
	else
	{
		position = pMediaStreamContext->fragmentTime;
	}

	float duration = fragmentDuration;
	if(rate > AAMP_NORMAL_PLAY_RATE)
	{
		position = position/rate;
		AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d rate %f pMediaStreamContext->fragmentTime %f updated position %f\n",
				__FUNCTION__, __LINE__, rate, pMediaStreamContext->fragmentTime, position);
		duration = duration/rate * gpGlobalConfig->vodTrickplayFPS;
		//aamp->disContinuity();
	}
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
			logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f\n", __FUNCTION__, __LINE__, fragmentUrl, pMediaStreamContext->fragmentTime);
			if(mCdaiObject->mAdState == AdState::IN_ADBREAK_AD_PLAYING && (isInitializationSegment || pMediaStreamContext->segDLFailCount >= MAX_AD_SEG_DOWNLOAD_FAIL_COUNT))
			{
				logprintf("PrivateStreamAbstractionMPD::%s:%d [CDAI] Ad fragment not available. Playback failed.\n", __FUNCTION__, __LINE__);
				mCdaiObject->mAdFailed = true;
			}
		}
		retval = false;
	}
	else
	{
		if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO)
		{
			if(rate == 1.0)
			{
				mBasePeriodOffset += fragmentDuration;
			}
			else
			{
				mBasePeriodOffset += (fragmentDuration*rate/gpGlobalConfig->vodTrickplayFPS);
			}
		}
	}

	{
		if(rate > 0)
		{
			pMediaStreamContext->fragmentTime += fragmentDuration;
		}
		else
		{
			pMediaStreamContext->fragmentTime -= fragmentDuration;
			if(pMediaStreamContext->fragmentTime < 0)
			{
				pMediaStreamContext->fragmentTime = 0;
			}
//			if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO) mBasePeriodOffset -= fragmentDuration;
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
	logprintf("%s:%d Type[%d] timeLineIndex %d segmentTemplate %p fragmentRepeatCount %u\n", __FUNCTION__, __LINE__,pMediaStreamContext->type,
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
			logprintf("%s:%d Type[%d] timelineCnt=%d timeLineIndex:%d fragTime=%" PRIu64 " L=%" PRIu64 " [fragmentTime = %f,  mLiveEndPosition = %f]\n", __FUNCTION__, __LINE__,
				pMediaStreamContext->type ,timelines.size(),pMediaStreamContext->timeLineIndex,pMediaStreamContext->fragmentDescriptor.Time,pMediaStreamContext->lastSegmentTime
				, pMediaStreamContext->fragmentTime, mLiveEndPosition);
#endif

			if ((pMediaStreamContext->timeLineIndex >= timelines.size()) || (pMediaStreamContext->timeLineIndex < 0)
					||(AdState::IN_ADBREAK_AD_PLAYING == mCdaiObject->mAdState &&
						((rate > AAMP_NORMAL_PLAY_RATE && pMediaStreamContext->fragmentTime >= mLiveEndPosition)
						 ||(rate < 0 && pMediaStreamContext->fragmentTime <= 0))))
			{
				AAMPLOG_INFO("%s:%d Type[%d] EOS. timeLineIndex[%d] size [%lu]\n",__FUNCTION__, __LINE__,pMediaStreamContext->type, pMediaStreamContext->timeLineIndex, timelines.size());
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
					if(startTime && mIsLive)
					{
						// After mpd refresh , Time will be 0. Need to traverse to the right fragment for playback
						if(0 == pMediaStreamContext->fragmentDescriptor.Time)
						{
							uint32_t duration =0;
							uint32_t repeatCount =0;
							int index = pMediaStreamContext->timeLineIndex;
							// This for loop is to go to the right index based on LastSegmentTime
							for(;index<timelines.size();index++)
							{
								timeline = timelines.at(index);
								startTime = timeline->GetStartTime();
								duration = timeline->GetDuration();
								repeatCount = timeline->GetRepeatCount();
								if(pMediaStreamContext->lastSegmentTime < (startTime+((repeatCount+1)*duration)))
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
								logprintf("%s:%d Type[%d] Boundary Condition !!! Index(%d) reached Max.Start=%" PRIu64 " Last=%" PRIu64 " \n",__FUNCTION__, __LINE__,
									pMediaStreamContext->type,index,startTime,pMediaStreamContext->lastSegmentTime);
								index--;
								startTime = pMediaStreamContext->lastSegmentTime;
								pMediaStreamContext->fragmentRepeatCount = repeatCount+1;
							}

#ifdef DEBUG_TIMELINE
							logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d Index=%d Num=%" PRIu64 " FTime=%f\n",__FUNCTION__, __LINE__, pMediaStreamContext->type,
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
							logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d fragRep=%d Index=%d Num=%" PRIu64 " FTime=%f\n",__FUNCTION__, __LINE__, pMediaStreamContext->type,
							startTime,pMediaStreamContext->lastSegmentTime, duration, repeatCount,pMediaStreamContext->fragmentRepeatCount,pMediaStreamContext->timeLineIndex,
							pMediaStreamContext->fragmentDescriptor.Number,pMediaStreamContext->fragmentTime);
#endif
						}
					}// if starttime
					if(0 == pMediaStreamContext->timeLineIndex)
					{
						AAMPLOG_INFO("%s:%d Type[%d] update startTime to %" PRIu64 "\n", __FUNCTION__, __LINE__,pMediaStreamContext->type, startTime);
					}
					pMediaStreamContext->fragmentDescriptor.Time = startTime;
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] Setting startTime to %" PRIu64 "\n", __FUNCTION__, __LINE__,pMediaStreamContext->type, startTime);
#endif
				}// if fragRepeat == 0

				ITimeline *timeline = timelines.at(pMediaStreamContext->timeLineIndex);
				uint32_t repeatCount = timeline->GetRepeatCount();
				uint32_t duration = timeline->GetDuration();
#ifdef DEBUG_TIMELINE
				logprintf("%s:%d Type[%d] t=%" PRIu64 " L=%" PRIu64 " d=%d r=%d fragrep=%d x=%d num=%lld\n",__FUNCTION__, __LINE__,
				pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time,
				pMediaStreamContext->lastSegmentTime, duration, repeatCount,pMediaStreamContext->fragmentRepeatCount,
				pMediaStreamContext->timeLineIndex,pMediaStreamContext->fragmentDescriptor.Number);
#endif
				if ((pMediaStreamContext->fragmentDescriptor.Time > pMediaStreamContext->lastSegmentTime) || (0 == pMediaStreamContext->lastSegmentTime))
				{
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] presenting %" PRIu64 " Number(%lld) Last=%" PRIu64 " Duration(%d) FTime(%f) \n",__FUNCTION__, __LINE__,
					pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time,pMediaStreamContext->fragmentDescriptor.Number,pMediaStreamContext->lastSegmentTime,duration,pMediaStreamContext->fragmentTime);
#endif
					double fragmentDuration = (double)duration/(double)timeScale;
					retval = FetchFragment( pMediaStreamContext, media, fragmentDuration, false, curlInstance);
					if(retval)
					{
						pMediaStreamContext->lastSegmentTime = pMediaStreamContext->fragmentDescriptor.Time;
						//logprintf("VOD/CDVR Line:%d fragmentDuration:%f target:%f SegTime%f rate:%f\n",__LINE__,fragmentDuration,pMediaStreamContext->targetDnldPosition,pMediaStreamContext->fragmentTime,rate);
						if(rate > AAMP_NORMAL_PLAY_RATE)
						{
							pMediaStreamContext->targetDnldPosition = pMediaStreamContext->fragmentTime;
						}
						else
						{
							pMediaStreamContext->targetDnldPosition += fragmentDuration;
						}
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
					logprintf("%s:%d Type[%d] presenting %" PRIu64 "\n", __FUNCTION__, __LINE__,pMediaStreamContext->type,pMediaStreamContext->fragmentDescriptor.Time);
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
					if(!mIsLive || !aamp->IsLiveAdjustRequired())
					{
						pMediaStreamContext->lastSegmentTime = pMediaStreamContext->fragmentDescriptor.Time - 1;
						return false;
					}
					logprintf("%s:%d Calling ScheduleRetune to handle start-time reset lastSegmentTime=%" PRIu64 " start-time=%" PRIu64 "\n", __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentTime, pMediaStreamContext->fragmentDescriptor.Time);
					aamp->ScheduleRetune(eDASH_ERROR_STARTTIME_RESET, pMediaStreamContext->mediaType);
				}
				else
				{
#ifdef DEBUG_TIMELINE
					logprintf("%s:%d Type[%d] Before skipping. fragmentDescriptor.Time %" PRIu64 " lastSegmentTime %" PRIu64 " Index=%d fragRep=%d,repMax=%d Number=%lld\n",__FUNCTION__, __LINE__,pMediaStreamContext->type,
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
					logprintf("%s:%d Type[%d] After skipping. fragmentDescriptor.Time %" PRIu64 " lastSegmentTime %" PRIu64 " Index=%d Number=%lld\n",__FUNCTION__, __LINE__,pMediaStreamContext->type,
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
						logprintf("%s:%d Type[%d] After Incr. fragmentDescriptor.Time %" PRIu64 " lastSegmentTime %" PRIu64 " Index=%d fragRep=%d,repMax=%d Number=%lld\n",__FUNCTION__, __LINE__,pMediaStreamContext->type,
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
			logprintf("%s:%d segmentTimeline not available\n", __FUNCTION__, __LINE__);
#endif
			string startTimestr = mpd->GetAvailabilityStarttime();
			std::tm time = { 0 };
			strptime(startTimestr.c_str(), "%Y-%m-%dT%H:%M:%SZ", &time);
			double availabilityStartTime = (double)mktime(&time);
			double currentTimeSeconds = aamp_GetCurrentTimeMS() / 1000;
			double fragmentDuration = ((double)segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
			if (!fragmentDuration)
			{
				fragmentDuration = 2; // hack
			}
			if (0 == pMediaStreamContext->lastSegmentNumber)
			{
				if (mIsLive)
				{
					double liveTime = currentTimeSeconds - gpGlobalConfig->liveOffset;
					pMediaStreamContext->lastSegmentNumber = (long long)((liveTime - availabilityStartTime - mPeriodStartTime) / fragmentDuration) + segmentTemplate->GetStartNumber();
					pMediaStreamContext->fragmentDescriptor.Time = liveTime;
					AAMPLOG_INFO("%s %d Printing fragmentDescriptor.Number %" PRIu64 " Time=%" PRIu64 "  \n", __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentNumber, pMediaStreamContext->fragmentDescriptor.Time);
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
			if (mIsLive && 0 == pMediaStreamContext->fragmentDescriptor.Time)
			{
				pMediaStreamContext->fragmentDescriptor.Time = availabilityStartTime + mPeriodStartTime + ((pMediaStreamContext->lastSegmentNumber - segmentTemplate->GetStartNumber()) * fragmentDuration);
			}

			/**
			 *Find out if we reached end/beginning of period.
			 *First block in this 'if' is for VOD, where boundaries are 0 and PeriodEndTime
			 *Second block is for LIVE, where boundaries are
                         * (availabilityStartTime + mPeriodStartTime) and currentTime
			 */
			if ((!mIsLive && ((mPeriodEndTime && (pMediaStreamContext->fragmentDescriptor.Time > mPeriodEndTime))
							|| (rate < 0 && pMediaStreamContext->fragmentDescriptor.Time < 0)))
					|| (mIsLive && ((pMediaStreamContext->fragmentDescriptor.Time >= currentTimeSeconds)
							|| (pMediaStreamContext->fragmentDescriptor.Time < (availabilityStartTime + mPeriodStartTime)))))
			{
				AAMPLOG_INFO("%s:%d EOS. fragmentDescriptor.Time=%" PRIu64 " mPeriodEndTime=%f FTime=%f\n",__FUNCTION__, __LINE__, pMediaStreamContext->fragmentDescriptor.Time, mPeriodEndTime,pMediaStreamContext->fragmentTime);
				pMediaStreamContext->eos = true;
			}
			else
			{
				if (mIsLive)
				{
					pMediaStreamContext->fragmentDescriptor.Number = pMediaStreamContext->lastSegmentNumber;
				}
				FetchFragment(pMediaStreamContext, media, fragmentDuration, false, curlInstance);
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
			char fragmentUrl[MAX_URI_LENGTH];
			GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
			if (!pMediaStreamContext->index_ptr)
			{ // lazily load index
				std::string range = segmentBase->GetIndexRange();
				int start;
				sscanf(range.c_str(), "%d-%d", &start, &pMediaStreamContext->fragmentOffset);

				ProfilerBucketType bucketType = aamp->GetProfilerBucketForMedia(pMediaStreamContext->mediaType, true);
				MediaType actualType = (MediaType)(eMEDIATYPE_INIT_VIDEO+pMediaStreamContext->mediaType);
				char effectiveUrl[MAX_URI_LENGTH];
				effectiveUrl[0] =0;
				long http_code;
				int iFogError = -1;
				pMediaStreamContext->index_ptr = aamp->LoadFragment(bucketType, fragmentUrl, effectiveUrl,&pMediaStreamContext->index_len, curlInstance, range.c_str(),&http_code,actualType,&iFogError);

				if (aamp->rate != AAMP_NORMAL_PLAY_RATE)
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
					AAMPLOG_INFO("%s:%d current fragmentIndex = %d\n", __FUNCTION__, __LINE__, pMediaStreamContext->fragmentIndex);
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
					AAMPLOG_INFO("%s:%d %s [%s]\n", __FUNCTION__, __LINE__,mMediaTypeName[pMediaStreamContext->mediaType], range);
					if(!pMediaStreamContext->CacheFragment(fragmentUrl, curlInstance, pMediaStreamContext->fragmentTime, 0.0, range ))
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f\n", __FUNCTION__, __LINE__, fragmentUrl, pMediaStreamContext->fragmentTime);
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
						char fragmentUrl[MAX_URI_LENGTH];
						GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor,  segmentURL->GetMediaURI());
						AAMPLOG_INFO("%s [%s]\n", mMediaTypeName[pMediaStreamContext->mediaType], segmentURL->GetMediaRange().c_str());
						if(!pMediaStreamContext->CacheFragment(fragmentUrl, curlInstance, pMediaStreamContext->fragmentTime, 0.0, segmentURL->GetMediaRange().c_str() ))
						{
							logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f\n", __FUNCTION__, __LINE__, fragmentUrl, pMediaStreamContext->fragmentTime);
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
							if(retval && rate > 0)
							{
								//logprintf("Live update Line:%d fragmentDuration:%f target:%f FragTime%f rate:%f\n",__LINE__,fragmentDuration,pMediaStreamContext->targetDnldPosition,pMediaStreamContext->fragmentTime,rate);
								if(rate > AAMP_NORMAL_PLAY_RATE)
								{
									pMediaStreamContext->targetDnldPosition = pMediaStreamContext->fragmentTime;
								}
								else
								{
									pMediaStreamContext->targetDnldPosition += fragmentDuration;
								}
							}
							if(mContext->mCheckForRampdown)
							{
								/* This case needs to be validated with the segmentList available stream */

								return retval;
							}
						}
						else if(pMediaStreamContext->mediaType == eMEDIATYPE_VIDEO && duration > 0 && ((pMediaStreamContext->lastSegmentTime - startTime) > TIMELINE_START_RESET_DIFF))
						{
							logprintf("%s:%d START-TIME RESET in TSB period, lastSegmentTime=%" PRIu64 " start-time=%lld duration=%lld\n", __FUNCTION__, __LINE__, pMediaStreamContext->lastSegmentTime, startTime, duration);
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
							AAMPLOG_TRACE("%s:%d PushNextFragment Exit : startTime %lld lastSegmentTime %lld index = %d\n", __FUNCTION__, __LINE__, startTime, pMediaStreamContext->lastSegmentTime, pMediaStreamContext->fragmentIndex);
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
					logprintf("PrivateStreamAbstractionMPD::%s:%d SegmentUrl is empty\n", __FUNCTION__, __LINE__);
				}
			}
			else
			{
				aamp_Error("not-yet-supported mpd format");
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
			uint64_t startTime = mPeriodStartTime;
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
			aamp_Error("not-yet-supported mpd format");
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
		 AAMPLOG_INFO("%s:%d Enter : Type[%d] timeLineIndex %d fragmentRepeatCount %d fragmentTime %f skipTime %f\n", __FUNCTION__, __LINE__,pMediaStreamContext->type,
                                pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentRepeatCount, pMediaStreamContext->fragmentTime, skipTime);

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
					AAMPLOG_INFO("%s:%d Type[%d] EOS. timeLineIndex[%d] size [%lu]\n",__FUNCTION__,__LINE__,pMediaStreamContext->type, pMediaStreamContext->timeLineIndex, timelines.size());
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
					bool skipFlag = true;
					if ((pMediaStreamContext->type == eTRACK_AUDIO) && (nextPTS>mFirstPTS))
					{
						if ( ((nextPTS - mFirstPTS) >= ((fragmentDuration)/2.0)) &&
                             ((nextPTS - mFirstPTS) <= ((fragmentDuration * 3.0)/2.0)))
							skipFlag = false;
						AAMPLOG_INFO("%s:%d [%s] firstPTS %f, nextPTS %f, mFirstPTS %f skipFlag %d\n", __FUNCTION__, __LINE__, pMediaStreamContext->name, firstPTS, nextPTS, mFirstPTS, skipFlag);
					}
					if (skipTime >= fragmentDuration && skipFlag)
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
					}
					if (fabs(skipTime) < fragmentDuration || !skipFlag)
					{ // last iteration
						if (updateFirstPTS)
						{
							AAMPLOG_INFO("%s:%d [%s] newPTS %f, nextPTS %f \n", __FUNCTION__, __LINE__, pMediaStreamContext->name, firstPTS, nextPTS);
							/*Keep the lower PTS */
							if ( ((mFirstPTS == 0) || (firstPTS < mFirstPTS)) && (pMediaStreamContext->type == eTRACK_VIDEO))
							{
								AAMPLOG_INFO("%s:%d [%s] mFirstPTS %f -> %f \n", __FUNCTION__, __LINE__, pMediaStreamContext->name, mFirstPTS, firstPTS);
								mFirstPTS = firstPTS; 
								AAMPLOG_INFO("%s:%d [%s] mFirstPTS %f \n", __FUNCTION__, __LINE__, pMediaStreamContext->name, mFirstPTS);
							}
						}
						skipTime = 0;
						break;
					}
				}
			}
			else
			{
				if(0 == pMediaStreamContext->fragmentDescriptor.Time)
				{
					if(rate < 0)
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
					AAMPLOG_INFO("%s:%d Type[%d] EOS. fragmentDescriptor.Time=%" PRIu64 " \n",__FUNCTION__,__LINE__,pMediaStreamContext->type, pMediaStreamContext->fragmentDescriptor.Time);
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
						skipTime -= segmentDuration;
					}
					else if (-(skipTime) >= segmentDuration)
					{
						pMediaStreamContext->fragmentDescriptor.Number--;
						skipTime += segmentDuration;
					}
					else
					{
						break;
					}
				}
			}
		}while(skipTime != 0);
		AAMPLOG_INFO("%s:%d Exit :Type[%d] timeLineIndex %d fragmentRepeatCount %d fragmentDescriptor.Number %" PRIu64 " fragmentTime %f\n", __FUNCTION__, __LINE__,pMediaStreamContext->type,
				pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentRepeatCount, pMediaStreamContext->fragmentDescriptor.Number, pMediaStreamContext->fragmentTime);
	}
	else
	{
		ISegmentList *segmentList = pMediaStreamContext->representation->GetSegmentList();
		if (segmentList)
		{
			AAMPLOG_INFO("%s:%d Enter : fragmentIndex %d skipTime %f\n", __FUNCTION__, __LINE__,
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
						logprintf("Printing diff value for adjusting %lld\n",diff);
						if(diff > 0)
						{
							double diffSeconds = double(diff) / timescale;
							skipTime -= diffSeconds;
						}
					}
					else
					{
						logprintf("PrivateStreamAbstractionMPD::%s:%d Video SegmentUrl is empty\n", __FUNCTION__, __LINE__);
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
				logprintf("PrivateStreamAbstractionMPD::%s:%d SegmentUrl is empty\n", __FUNCTION__, __LINE__);
			}

			AAMPLOG_INFO("%s:%d Exit : fragmentIndex %d segmentDuration %f\n", __FUNCTION__, __LINE__,
					pMediaStreamContext->fragmentIndex, segmentDuration);
		}
		else
		{
			aamp_Error("not-yet-supported mpd format");
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
 * @brief Get libdash xml Node for Ad period
 * @param[in] manifestUrl url of the Ad
 * @retval libdash xml Node corresponding to Ad period
 */
MPD* PrivateCDAIObjectMPD::GetAdMPD(std::string &manifestUrl, bool &finalManifest, bool tryFog)
{
	MPD* adMpd = NULL;
	GrowableBuffer manifest;
	int downloadAttempt = 0;
	bool gotManifest = false;
	long http_error = 0;
	char effectiveUrl[MAX_URI_LENGTH];

	while (downloadAttempt < 2)
	{
		downloadAttempt++;
		memset(&manifest, 0, sizeof(manifest));
		gotManifest = mAamp->GetFile(manifestUrl.c_str(), &manifest, effectiveUrl, &http_error, NULL, AAMP_DAI_CURL_IDX);
		if (gotManifest)
		{
			break;
		}
		else if (mAamp->DownloadsAreEnabled())
		{
			if (downloadAttempt < 2 && 404 == http_error)
			{
				continue;
			}
			logprintf("PrivateStreamAbstractionMPD::%s - manifest download failed\n", __FUNCTION__);
			break;
		}
	}

	if (gotManifest)
	{
		finalManifest = true;
		xmlTextReaderPtr reader = xmlReaderForMemory(manifest.ptr, (int) manifest.len, NULL, NULL, 0);
		if(tryFog && !(gpGlobalConfig->playAdFromCDN) && reader && mIsFogTSB)	//Main content from FOG. Ad is expected from FOG.
		{
			const char *channelUrl = (char *)mAamp->GetManifestUrl();	//TODO: Get FOG URL from channel URL
			std::string encodedUrl;
			UrlEncode(effectiveUrl, encodedUrl);
			int ipend = 0;
			for(int slashcnt=0; slashcnt < 3 && channelUrl[ipend]; ipend++)
			{
				if(channelUrl[ipend] == '/') slashcnt++;
			}

			strncpy(effectiveUrl, channelUrl, ipend);
			effectiveUrl[ipend] = '\0';
			strcat(effectiveUrl, "adrec?clientId=FOG_AAMP&recordedUrl=");
			strcat(effectiveUrl, encodedUrl.c_str());
			GrowableBuffer fogManifest;
			memset(&fogManifest, 0, sizeof(manifest));
			http_error = 0;
			mAamp->GetFile(effectiveUrl, &fogManifest, effectiveUrl, &http_error, NULL, AAMP_DAI_CURL_IDX);
			if(200 == http_error || 204 == http_error)
			{
				manifestUrl = effectiveUrl;
				if(200 == http_error)
				{
					//FOG already has the manifest. Releasing the one from CDN and using FOG's
					xmlFreeTextReader(reader);
					reader = xmlReaderForMemory(fogManifest.ptr, (int) fogManifest.len, NULL, NULL, 0);
					aamp_Free(&manifest.ptr);
					manifest = fogManifest;
					fogManifest.ptr = NULL;
				}
				else
				{
					finalManifest = false;
				}
			}

			if(fogManifest.ptr)
			{
				aamp_Free(&fogManifest.ptr);
			}
		}
		if (reader != NULL)
		{
			if (xmlTextReaderRead(reader))
			{
				Node* root = PrivateStreamAbstractionMPD::ProcessNode(&reader, manifestUrl.c_str(), true);
				if (NULL != root)
				{
					std::vector<Node*> children = root->GetSubNodes();
					for (size_t i = 0; i < children.size(); i++)
					{
						Node* child = children.at(i);
						const std::string& name = child->GetName();
						logprintf("PrivateStreamAbstractionMPD::%s - child->name %s\n", __FUNCTION__, name.c_str());
						if (name == "Period")
						{
							logprintf("PrivateStreamAbstractionMPD::%s - found period\n", __FUNCTION__);
							std::vector<Node *> children = child->GetSubNodes();
							bool hasBaseUrl = false;
							for (size_t i = 0; i < children.size(); i++)
							{
								if (children.at(i)->GetName() == "BaseURL")
								{
									hasBaseUrl = true;
								}
							}
							if (!hasBaseUrl)
							{
								// BaseUrl not found in the period. Get it from the root and put it in the period
								children = root->GetSubNodes();
								for (size_t i = 0; i < children.size(); i++)
								{
									if (children.at(i)->GetName() == "BaseURL")
									{
										Node* baseUrl = new Node(*children.at(i));
										child->AddSubNode(baseUrl);
										hasBaseUrl = true;
										break;
									}
								}
							}
							if (!hasBaseUrl)
							{
								std::string baseUrlStr = Path::GetDirectoryPath(manifestUrl);
								Node* baseUrl = new Node();
								baseUrl->SetName("BaseURL");
								baseUrl->SetType(Text);
								baseUrl->SetText(baseUrlStr);
								logprintf("PrivateStreamAbstractionMPD::%s - manual adding BaseURL Node [%p] text %s\n",
								        __FUNCTION__, baseUrl, baseUrl->GetText().c_str());
								child->AddSubNode(baseUrl);
							}
							break;
						}
					}
					adMpd = root->ToMPD();
					delete root;
				}
				else
				{
					logprintf("%s:%d - Could not create root node\n", __FUNCTION__, __LINE__);
				}
			}
			else
			{
				logprintf("%s:%d - xmlTextReaderRead failed\n", __FUNCTION__, __LINE__);
			}
			xmlFreeTextReader(reader);
		}
		else
		{
			logprintf("%s:%d - xmlReaderForMemory failed\n", __FUNCTION__, __LINE__);
		}

		if (gpGlobalConfig->logging.trace)
		{
			aamp_AppendNulTerminator(&manifest); // make safe for cstring operations
			logprintf("%s:%d - Ad manifest: %s\n", __FUNCTION__, __LINE__, manifest.ptr);
		}
		aamp_Free(&manifest.ptr);
	}
	else
	{
		logprintf("%s:%d - aamp: error on manifest fetch\n", __FUNCTION__, __LINE__);
	}
	return adMpd;
}

/**
 * @brief Get xml node form reader
 * @param reader pointer to reader object
 * @param url manifest url
 * @retval xml node
*/
AAMPStatusType  PrivateStreamAbstractionMPD::GetMpdFromManfiest(const GrowableBuffer &manifest, MPD * &mpd, const char* manifestUrl, bool init)
{
	AAMPStatusType ret = eAAMPSTATUS_OK;
	xmlTextReaderPtr reader = xmlReaderForMemory(manifest.ptr, (int) manifest.len, NULL, NULL, 0);
	if (reader != NULL)
	{
		if (xmlTextReaderRead(reader))
		{
			Node *root = PrivateStreamAbstractionMPD::ProcessNode(&reader, manifestUrl);
			if(root != NULL)
			{
				uint32_t fetchTime = Time::GetCurrentUTCTimeInSec();
				mpd = root->ToMPD();
				if (mpd)
				{
					mpd->SetFetchTime(fetchTime);
#if 1
					FindTimedMetadata(mpd, root, init);
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
		}
		xmlFreeTextReader(reader);
	}
	else
	{
		ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_PARSE_ERROR;
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
Node* PrivateStreamAbstractionMPD::ProcessNode(xmlTextReaderPtr *reader, const char *url, bool isAd)
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

			subnode = PrivateStreamAbstractionMPD::ProcessNode(reader, url, isAd);

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
 * @brief Parse duration from ISO8601 string
 * @param ptr ISO8601 string
 * @param[out] durationMs duration in milliseconds
 */
static void ParseISO8601Duration(const char *ptr, uint64_t &durationMs)
{
	durationMs = 0;
	int hour = 0;
	int minute = 0;
	float seconds = 0;
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
			sscanf(ptr, "%fS", &seconds);
			ptr = temp + 1;
		}
	}
	else
	{
		logprintf("%s:%d - Invalid input %s\n", __FUNCTION__, __LINE__, ptr);
	}
	durationMs = (float(((hour * 60) + minute) * 60 + seconds)) * 1000;
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
 * @brief Create DRM Session
 * @param arg DrmSessionParams object pointer
 */
void *CreateDRMSession(void *arg)
{
	if(aamp_pthread_setname(pthread_self(), "aampDRM"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
	}
	struct DrmSessionParams* sessionParams = (struct DrmSessionParams*)arg;
	AampDRMSessionManager* sessionManger = AampDRMSessionManager::getInstance();
	sessionParams->aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_TOTAL);
	AAMPEvent e;
	e.type = AAMP_EVENT_DRM_METADATA;
	e.data.dash_drmmetadata.failure = AAMP_TUNE_FAILURE_UNKNOWN;
	e.data.dash_drmmetadata.responseCode = 0;
	unsigned char * data = sessionParams->initData;
	int dataLength = sessionParams->initDataLen;

	unsigned char *contentMetadata = sessionParams->contentMetadata;
	AampDrmSession *drmSession = NULL;
	const char * systemId = WIDEVINE_SYSTEM_ID;
	if (sessionParams->isWidevine)
	{
		logprintf("Found Widevine encryption from manifest\n");
	}
	else
	{
		logprintf("Found Playready encryption from manifest\n");
		systemId = PLAYREADY_SYSTEM_ID;
	}
	sessionParams->aamp->mStreamSink->QueueProtectionEvent(systemId, data, dataLength);
	//Hao Li: review changes for Widevine, contentMetadata is freed inside the following calls
	drmSession = sessionManger->createDrmSession(systemId, data, dataLength, sessionParams->stream_type,
					contentMetadata, sessionParams->aamp, &e);
	if(NULL == drmSession)
	{
		AAMPTuneFailure failure = e.data.dash_drmmetadata.failure;
		bool isRetryEnabled =      (failure != AAMP_TUNE_AUTHORISATION_FAILURE)
		                        && (failure != AAMP_TUNE_LICENCE_REQUEST_FAILED)
								&& (failure != AAMP_TUNE_LICENCE_TIMEOUT)
		                        && (failure != AAMP_TUNE_DEVICE_NOT_PROVISIONED);
		sessionParams->aamp->SendDrmErrorEvent(e.data.dash_drmmetadata.failure, e.data.dash_drmmetadata.responseCode, isRetryEnabled);
		sessionParams->aamp->profiler.SetDrmErrorCode((int)e.data.dash_drmmetadata.failure);
		sessionParams->aamp->profiler.ProfileError(PROFILE_BUCKET_LA_TOTAL, (int)e.data.dash_drmmetadata.failure);
	}
	else
	{
		if(e.data.dash_drmmetadata.accessStatus_value != 3)
		{
			AAMPLOG_INFO("Sending DRMMetaData\n");
			sessionParams->aamp->SendDRMMetaData(e);
		}
		sessionParams->aamp->profiler.ProfileEnd(PROFILE_BUCKET_LA_TOTAL);
	}
	free(data);
	if(contentMetadata != NULL)
		free(contentMetadata);
	free(sessionParams);
	return NULL;
}

/**
 * @brief Process content protection of adaptation
 * @param adaptationSet Adaptation set object
 * @param mediaType type of track
 */
void PrivateStreamAbstractionMPD::ProcessContentProtection(IAdaptationSet * adaptationSet,MediaType mediaType)
{
	const vector<IDescriptor*> contentProt = adaptationSet->GetContentProtection();
	unsigned char* data   = NULL;
	unsigned char* wvData = NULL;
	unsigned char* prData = NULL;
	size_t dataLength     = 0;
	size_t wvDataLength   = 0;
	size_t prDataLength   = 0;
	bool isWidevine       = false;
	unsigned char* contentMetadata = NULL;

	AAMPLOG_TRACE("[HHH]contentProt.size=%d\n", contentProt.size());
	for (unsigned iContentProt = 0; iContentProt < contentProt.size(); iContentProt++)
	{
		if (contentProt.at(iContentProt)->GetSchemeIdUri().find(COMCAST_DRM_INFO_ID) != string::npos)
		{
			logprintf("[HHH]Comcast DRM Agnostic CENC system ID found!\n");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			string psshData = node.at(0)->GetText();
			data = base64_Decode(psshData.c_str(), &dataLength);

			if(gpGlobalConfig->logging.trace)
			{
				logprintf("content metadata from manifest; length %d\n", dataLength);
				DumpBlob( data, dataLength );
			}
			if(dataLength != 0)
			{
				int contentMetadataLen = 0;
				contentMetadata = _extractWVContentMetadataFromPssh((const char*)data, dataLength, &contentMetadataLen);
				if(gpGlobalConfig->logging.trace)
				{
					logprintf("content metadata from PSSH; length %d\n", contentMetadataLen);
					DumpBlob( contentMetadata, contentMetadataLen );
				}
			}
			if(data) free(data);
			continue;
		}

		if (contentProt.at(iContentProt)->GetSchemeIdUri().find(WIDEVINE_SYSTEM_ID) != string::npos)
		{
			logprintf("[HHH]Widevine system ID found!\n");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			string psshData = node.at(0)->GetText();
			wvData = base64_Decode(psshData.c_str(), &wvDataLength);
			mContext->hasDrm = true;
			if(gpGlobalConfig->logging.trace)
			{
				logprintf("init data from manifest; length %d\n", wvDataLength);
				DumpBlob(wvData, wvDataLength);
			}
			continue;
		}

		if (contentProt.at(iContentProt)->GetSchemeIdUri().find(PLAYREADY_SYSTEM_ID) != string::npos)
		{
			logprintf("[HHH]Playready system ID found!\n");
			const vector<INode*> node = contentProt.at(iContentProt)->GetAdditionalSubNodes();
			string psshData = node.at(0)->GetText();
			prData = base64_Decode(psshData.c_str(), &prDataLength);
			mContext->hasDrm = true;
			if(gpGlobalConfig->logging.trace)
			{
				logprintf("init data from manifest; length %d\n", prDataLength);
				DumpBlob(prData, prDataLength);
			}
			continue;
		}
	}

	// Choose widevine if both widevine and playready contentprotectiondata sections are presenet.
	// TODO: We need to add more flexible selection logic here (using aamp.cfg etc)
	if(wvData != NULL && wvDataLength > 0 && ((DRMSystems)gpGlobalConfig->preferredDrm == eDRM_WideVine || prData == NULL))
	{
		isWidevine = true;
		data = wvData;
		dataLength = wvDataLength;

		if(prData){
			free(prData);
		}
	}else if(prData != NULL && prDataLength > 0)
	{
		isWidevine = false;
		data = prData;
		dataLength = prDataLength;
		if(wvData){
			free(wvData);
		}
	}

	if(dataLength != 0)
	{
		int keyIdLen = 0;
		unsigned char* keyId = NULL;
		aamp->licenceFromManifest = true;
		keyId = _extractKeyIdFromPssh((const char*)data, dataLength, &keyIdLen, isWidevine);


		if (!(keyIdLen == lastProcessedKeyIdLen && 0 == memcmp(lastProcessedKeyId, keyId, keyIdLen)))
		{
			struct DrmSessionParams* sessionParams = (struct DrmSessionParams*)malloc(sizeof(struct DrmSessionParams));
			sessionParams->initData = data;
			sessionParams->initDataLen = dataLength;
			sessionParams->stream_type = mediaType;
			sessionParams->aamp = aamp;
			sessionParams->isWidevine = isWidevine;
			sessionParams->contentMetadata = contentMetadata;

			if(drmSessionThreadStarted) //In the case of license rotation
			{
				void *value_ptr = NULL;
				int rc = pthread_join(createDRMSessionThreadID, &value_ptr);
				if (rc != 0)
				{
					logprintf("pthread_join returned %d for createDRMSession Thread\n", rc);
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
				aamp->setCurrentDrm(isWidevine?eDRM_WideVine:eDRM_PlayReady);
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
	logprintf("MPD DRM not enabled\n");
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

/**
 *   @brief  Get Period Duration
 *   @param  period
 *   @retval period duration
 */
uint64_t GetPeriodDuration(IPeriod * period)
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
			uint32_t timeScale = segmentTemplate->GetTimescale();
			durationMs = (segmentTemplate->GetDuration() / timeScale) * 1000;
			if (0 == durationMs && segmentTimeline)
			{
				std::vector<ITimeline *>&timelines = segmentTimeline->GetTimelines();
				int timeLineIndex = 0;
				while (timeLineIndex < timelines.size())
				{
					ITimeline *timeline = timelines.at(timeLineIndex);
					uint32_t repeatCount = timeline->GetRepeatCount();
					uint32_t timelineDurationMs = timeline->GetDuration() * 1000 / timeScale;
					durationMs += ((repeatCount + 1) * timelineDurationMs);
					traceprintf("%s timeLineIndex[%d] size [%lu] updated durationMs[%" PRIu64 "]\n", __FUNCTION__, timeLineIndex, timelines.size(), durationMs);
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
					aamp_Error("not-yet-supported mpd format");
				}
			}
		}
	}
	return durationMs;
}

uint64_t GetPeriodNewContentDuration(IPeriod * period, uint64_t &curEndNumber)
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
 * @brief Get end time of current period
 * @retval current period's end time
 */
uint64_t PrivateStreamAbstractionMPD::GetPeriodEndTime()
{
	uint64_t  periodStartMs = 0;
	uint64_t periodDurationMs = 0;
	uint64_t periodEndTime = 0;
	IPeriod *period = mCurrentPeriod;
	string statTimeStr = period->GetStart();
	string durationStr = period->GetDuration();
	if(!statTimeStr.empty())
	{
		ParseISO8601Duration(statTimeStr.c_str(), periodStartMs);
	}
	if(!durationStr.empty())
	{
		ParseISO8601Duration(durationStr.c_str(), periodDurationMs);
		/*uint32_t timeScale = 1000;
		ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
		if(segmentTemplate)
		{
			timeScale = segmentTemplate->GetTimescale();
		}
		*/
		periodEndTime =  (periodDurationMs + periodStartMs) / 1000;
	}
	traceprintf("PrivateStreamAbstractionMPD::%s:%d - MPD periodEndTime %lld\n", __FUNCTION__, __LINE__, periodEndTime);
	return periodEndTime;
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
	aamp->CurlInit(0, AAMP_TRACK_COUNT);
	mCdaiObject->ResetState();

	aamp->mStreamSink->ClearProtectionEvent();
  #ifdef AAMP_MPD_DRM
	AampDRMSessionManager::getInstance()->setSessionMgrState(SessionMgrState::eSESSIONMGR_ACTIVE);
  #endif
	aamp->licenceFromManifest = false;
	bool newTune = aamp->IsNewTune();

	aamp->IsTuneTypeNew = false;

#ifdef AAMP_MPD_DRM
	mPushEncInitFragment = newTune || (eTUNETYPE_RETUNE == tuneType);
#endif

	for (int i = 0; i < AAMP_TRACK_COUNT; i++)
	{
		aamp->SetCurlTimeout(gpGlobalConfig->networkTimeout, i);
	}

	AAMPStatusType ret = UpdateMPD(true);
	if (ret == eAAMPSTATUS_OK)
	{
		char *manifestUrl = (char *)aamp->GetManifestUrl();
		int numTracks = (rate == AAMP_NORMAL_PLAY_RATE)?AAMP_TRACK_COUNT:1;
		double offsetFromStart = seekPosition;
		uint64_t durationMs = 0;
		mNumberOfTracks = 0;
		bool mpdDurationAvailable = false;
		std::string tempString = mpd->GetMediaPresentationDuration();
		if(!tempString.empty())
		{
			ParseISO8601Duration( tempString.c_str(), durationMs);
			mpdDurationAvailable = true;
			logprintf("PrivateStreamAbstractionMPD::%s:%d - MPD duration str %s val %" PRIu64 " seconds\n", __FUNCTION__, __LINE__, tempString.c_str(), durationMs/1000);
		}

		mIsLive = !(mpd->GetType() == "static");
		map<string, string> mpdAttributes = mpd->GetRawAttributes();
		if(mpdAttributes.find("fogtsb") != mpdAttributes.end())
		{
			mIsFogTSB = true;
			mCdaiObject->mIsFogTSB = true;
		}

		if(mIsLive)
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
			logprintf("PrivateStreamAbstractionMPD::%s:%d - MPD minupdateduration val %" PRIu64 " seconds\n", __FUNCTION__, __LINE__,  mMinUpdateDurationMs/1000);
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
					logprintf("%s:%d - Updated duration %" PRIu64 " seconds\n", __FUNCTION__, __LINE__, durationMs/1000);
				}
			}
			else if (mIsFogTSB)
			{
				periodDurationMs = GetPeriodDuration(period);
				durationMs += periodDurationMs;
				logprintf("%s:%d - Updated duration %" PRIu64 " seconds\n", __FUNCTION__, __LINE__, durationMs/1000);
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
					nextPeriodStart += periodDurationMs;
				}
				if (periodDurationMs != 0)
				{
					double periodEnd = periodStartMs + periodDurationMs;
					currentPeriodStart = (double)periodStartMs/1000;
					mCurrentPeriodIdx = iPeriod;
					if (periodDurationMs/1000 <= offsetFromStart && iPeriod < (numPeriods - 1))
					{
						logprintf("Skipping period %d seekPosition %f periodEnd %f\n", iPeriod, seekPosition, periodEnd);
						offsetFromStart -= periodDurationMs/1000;
						continue;
					}
					else
					{
						seekPeriods = false;
						logprintf("currentPeriodIdx %d/%d\n", iPeriod, (int)numPeriods);
					}
				}
				else if(periodStartMs/1000 <= offsetFromStart)
				{
					mCurrentPeriodIdx = iPeriod;
					currentPeriodStart = (double)periodStartMs/1000;
				}
			}
		}

		//Check added to update offsetFromStart for
		//Multi period assets with no period duration
		if(0 == nextPeriodStart)
		{
			offsetFromStart -= currentPeriodStart;
		}

		if (0 == durationMs)
		{
			durationMs = PrivateStreamAbstractionMPD::GetDurationFromRepresentation(mpd);
			logprintf("%s:%d - Duration after GetDurationFromRepresentation %" PRIu64 " seconds\n", __FUNCTION__, __LINE__, durationMs/1000);
		}
		/*Do live adjust on live streams on 1. eTUNETYPE_NEW_NORMAL, 2. eTUNETYPE_SEEKTOLIVE,
		 * 3. Seek to a point beyond duration*/
		bool notifyEnteringLive = false;
		if (mIsLive)
		{
			double duration = (double) durationMs / 1000;
			mLiveEndPosition = duration;
			bool liveAdjust = (eTUNETYPE_NEW_NORMAL == tuneType) && aamp->IsLiveAdjustRequired();
			if (eTUNETYPE_SEEKTOLIVE == tuneType)
			{
				logprintf("PrivateStreamAbstractionMPD::%s:%d eTUNETYPE_SEEKTOLIVE\n", __FUNCTION__, __LINE__);
				liveAdjust = true;
				notifyEnteringLive = true;
			}
			else if (((eTUNETYPE_SEEK == tuneType) || (eTUNETYPE_RETUNE == tuneType || eTUNETYPE_NEW_SEEK == tuneType)) && (rate > 0))
			{
				double seekWindowEnd = duration - aamp->mLiveOffset;
				// check if seek beyond live point
				if (seekPosition > seekWindowEnd)
				{
					logprintf( "PrivateStreamAbstractionMPD::%s:%d offSetFromStart[%f] seekWindowEnd[%f]\n",
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
					duration = (double)GetPeriodDuration(mpd->GetPeriods().at(mCurrentPeriodIdx)) / 1000;
					currentPeriodStart = ((double)durationMs / 1000) - duration;
					offsetFromStart = duration - aamp->mLiveOffset;
					while(offsetFromStart < 0 && mCurrentPeriodIdx > 0)
					{
						logprintf("%s:%d Adjusting to live offset offsetFromStart %f, mCurrentPeriodIdx %d\n", __FUNCTION__, __LINE__, offsetFromStart, mCurrentPeriodIdx);
						mCurrentPeriodIdx--;
						duration = (double)GetPeriodDuration(mpd->GetPeriods().at(mCurrentPeriodIdx)) / 1000;
						currentPeriodStart = currentPeriodStart - duration;
						offsetFromStart = offsetFromStart + duration;
					}
					logprintf("%s:%d duration %f durationMs %f mCurrentPeriodIdx %d currentPeriodStart %f offsetFromStart %f \n", __FUNCTION__, __LINE__,
                                 duration, (double)durationMs / 1000, mCurrentPeriodIdx, currentPeriodStart, offsetFromStart);
				}
				if (offsetFromStart < 0)
				{
					offsetFromStart = 0;
				}
				mContext->mIsAtLivePoint = true;
				logprintf( "PrivateStreamAbstractionMPD::%s:%d - liveAdjust - Updated offSetFromStart[%f] duration [%f] currentPeriodStart[%f] MaxPeriodIdx[%d]\n",
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
				logprintf("%s:%d seek target out of range, mark EOS. playTarget:%f End:%f. \n",
					__FUNCTION__,__LINE__,seekPosition, seekWindowEnd);
				return eAAMPSTATUS_SEEK_RANGE_ERROR;
			}
		}
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
			TunedEventConfig tunedEventConfig =  mIsLive ?
					gpGlobalConfig->tunedEventConfigLive : gpGlobalConfig->tunedEventConfigVOD;
			if (eTUNED_EVENT_ON_PLAYLIST_INDEXED == tunedEventConfig)
			{
				if (aamp->SendTunedEvent())
				{
					logprintf("aamp: mpd - sent tune event after indexing playlist\n");
				}
			}
			UpdateTrackInfo(!newTune, true, true);

			if(notifyEnteringLive)
			{
				aamp->NotifyOnEnteringLive();
			}
			SeekInPeriod( offsetFromStart);
			seekPosition = mMediaStreamContext[eMEDIATYPE_VIDEO]->fragmentTime;
			if(0 != mCurrentPeriodIdx)
			{
				seekPosition += currentPeriodStart;
				for (int i = 0; i < mNumberOfTracks; i++)
				{
					mMediaStreamContext[i]->fragmentTime = seekPosition;
				}
			}
			AAMPLOG_INFO("%s:%d  offsetFromStart(%f) seekPosition(%f) \n",__FUNCTION__,__LINE__,offsetFromStart,seekPosition);
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
				aamp->SendMediaMetadataEvent(durationMs, mLangList, bitrateList, mContext->hasDrm, mIsIframeTrackPresent);

				aamp->UpdateDuration(((double)durationMs)/1000);
				GetCulledSeconds();
				aamp->UpdateRefreshPlaylistInterval((float)mMinUpdateDurationMs / 1000);
			}
		}
		else
		{
			logprintf("No adaptation sets could be selected\n");
			retval = eAAMPSTATUS_MANIFEST_CONTENT_ERROR;
		}
	}
	else
	{
		retval = eAAMPSTATUS_MANIFEST_PARSE_ERROR;
	}
	return retval;
}


/**
 * @brief Get duration though representation iteration
 * @retval duration in milliseconds
 */
uint64_t PrivateStreamAbstractionMPD::GetDurationFromRepresentation(dash::mpd::IMPD *mpd)
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
						traceprintf("%s period[%d] timeLineIndex[%d] size [%lu] updated durationMs[%" PRIu64 "]\n", __FUNCTION__, iPeriod, timeLineIndex, timelines.size(), durationMs);
						timeLineIndex++;
					}
				}
				else
				{
					uint32_t timeScale = segmentTemplate->GetTimescale();
					durationMs = (segmentTemplate->GetDuration() / timeScale) * 1000;
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
						aamp_Error("not-yet-supported mpd format");
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
	int downloadAttempt = 0;
	char *manifestUrl = aamp->GetManifestUrl();
	bool gotManifest = false;
	bool retrievedPlaylistFromCache = false;
	memset(&manifest, 0, sizeof(manifest));
	if (AampCacheHandler::GetInstance()->RetrieveFromPlaylistCache(manifestUrl, &manifest, manifestUrl))
	{
		logprintf("PrivateStreamAbstractionMPD::%s:%d manifest retrieved from cache\n", __FUNCTION__, __LINE__);
		retrievedPlaylistFromCache = true;
	}
	while( downloadAttempt < 2)
	{
		if (!retrievedPlaylistFromCache)
		{
			long http_error = 0;
			downloadAttempt++;
			memset(&manifest, 0, sizeof(manifest));
			aamp->profiler.ProfileBegin(PROFILE_BUCKET_MANIFEST);
			gotManifest = aamp->GetFile(manifestUrl, &manifest, manifestUrl, &http_error, NULL, 0, true, eMEDIATYPE_MANIFEST);

			//update videoend info
			aamp->UpdateVideoEndMetrics(eMEDIATYPE_MANIFEST,0,http_error,manifestUrl);

			if (gotManifest)
			{
				aamp->profiler.ProfileEnd(PROFILE_BUCKET_MANIFEST);
				if (mContext->mNetworkDownDetected)
				{
					mContext->mNetworkDownDetected = false;
				}
			}
			else if (aamp->DownloadsAreEnabled())
			{
				if(downloadAttempt < 2 && 404 == http_error)
				{
					continue;
				}
				aamp->profiler.ProfileError(PROFILE_BUCKET_MANIFEST);
				if (this->mpd != NULL && (CURLE_OPERATION_TIMEDOUT == http_error || CURLE_COULDNT_CONNECT == http_error))
				{
					//Skip this for first ever update mpd request
					mContext->mNetworkDownDetected = true;
					logprintf("PrivateStreamAbstractionMPD::%s Ignore curl timeout\n", __FUNCTION__);
					ret = AAMPStatusType::eAAMPSTATUS_OK;
					break;
				}
				aamp->SendDownloadErrorEvent(AAMP_TUNE_MANIFEST_REQ_FAILED, http_error);
				logprintf("PrivateStreamAbstractionMPD::%s - manifest download failed\n", __FUNCTION__);
				ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_DOWNLOAD_ERROR;
				break;
			}
		}
		else
		{
			gotManifest = true;
		}

		if (gotManifest)
		{
#ifdef AAMP_HARVEST_SUPPORT_ENABLED
			char fileName[1024] = {'\0'};
			strcat(fileName, HARVEST_BASE_PATH);
			strcat(fileName, "manifest.mpd");
			WriteFile( fileName, manifest.ptr, manifest.len);
#endif
			// parse xml
			xmlTextReaderPtr reader = xmlReaderForMemory(manifest.ptr, (int) manifest.len, NULL, NULL, 0);

//Enable to harvest MPD file
//Save the last 3 MPDs
#ifdef HARVEST_MPD
			static int counter = 0;
			string fileSuffix = to_string(counter % 3);
			counter++;
			string fullPath = "/opt/logs/ProcessNodeError.txt" + fileSuffix;
			logprintf("Saving manifest to %s\n",fullPath.c_str());
			FILE *outputFile = fopen(fullPath.c_str(), "w");
			fwrite(manifest.ptr, manifest.len, 1, outputFile);
			fprintf(outputFile,"\n\n\nEndofManifest\n\n\n");
			fclose(outputFile);
#endif

			MPD* mpd = nullptr;
			ret = GetMpdFromManfiest(manifest, mpd, manifestUrl, init);
			logprintf("%s:%d Created MPD[%p]\n", __FUNCTION__, __LINE__, mpd);
			if (eAAMPSTATUS_OK == ret)
			{
				if (this->mpd)
				{
					delete this->mpd;
				}
				this->mpd = mpd;
				mIsLive = !(mpd->GetType() == "static");
				aamp->SetIsLive(mIsLive);
				if (!retrievedPlaylistFromCache)
				{
					AampCacheHandler::GetInstance()->InsertToPlaylistCache(aamp->GetManifestUrl(), &manifest, aamp->GetManifestUrl(), mIsLive);
				}
			}
			else
			{
				logprintf("%s:%d Error while processing MPD, GetMpdFromManfiest returned %d\n", __FUNCTION__, __LINE__, ret);
				if(downloadAttempt < 2)
				{
					retrievedPlaylistFromCache = false;
					continue;
				}
			}

			aamp_Free(&manifest.ptr);
			mLastPlaylistDownloadTimeMs = aamp_GetCurrentTimeMS();
		}
		else
		{
			logprintf("aamp: error on manifest fetch\n");
			ret = AAMPStatusType::eAAMPSTATUS_MANIFEST_DOWNLOAD_ERROR;
		}
		break;
	}

	if( ret == eAAMPSTATUS_MANIFEST_PARSE_ERROR || ret == eAAMPSTATUS_MANIFEST_CONTENT_ERROR)
	{
	    if(NULL != manifest.ptr && NULL != manifestUrl)
	    {
            int tempDataLen = (MANIFEST_TEMP_DATA_LENGTH - 1);
            char temp[MANIFEST_TEMP_DATA_LENGTH];
            strncpy(temp, manifest.ptr, tempDataLen);
            temp[tempDataLen] = 0x00;
	        logprintf("ERROR: Invalid Playlist URL: %s ret:%d\n", manifestUrl,ret);
	        logprintf("ERROR: Invalid Playlist DATA: %s \n", temp);
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
					ProcessPeriodSupplementalProperty(child, AdID, periodStartMS, periodDurationMS);
					continue;
				}
				if (name == "AssetIdentifier") {
					ProcessPeriodAssetIdentifier(child, periodStartMS, periodDurationMS, AssetID, ProviderID);
					continue;
				}
				if(name == "EventStream" && "" != prdId && !(mCdaiObject->isPeriodExist(prdId))
				   && (!init || (1 < periodCnt && 0 == period->GetAdaptationSets().size())))	//Take last & empty period at the MPD init AND all new periods in the MPD refresh. (No empty periods will come the middle)
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
						const std::string& contentID = infoNode->GetAttributeValue("value");

						std::ostringstream s;
						s << "#EXT-X-CONTENT-IDENTIFIER:" << contentID;

						std::string content = s.str();
						AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", 0.0f, content.c_str());

						for (int i = 0; i < aamp->subscribedTags.size(); i++)
						{
							const std::string& tag = aamp->subscribedTags.at(i);
							if (tag == "#EXT-X-CONTENT-IDENTIFIER") {
								aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size());
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
					const std::string& identityADS = node->GetAttributeValue("value");

					std::ostringstream s;
					s << "#EXT-X-IDENTITY-ADS:" << identityADS;

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", 0.0f, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-IDENTITY-ADS") {
							aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size());
							break;
						}
					}
					continue;
				}
				if (ID == "messageRef" && node->HasAttribute("value")) {
					const std::string& messageRef = node->GetAttributeValue("value");

					std::ostringstream s;
					s << "#EXT-X-MESSAGE-REF:" << messageRef;

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", 0.0f, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-MESSAGE-REF") {
							aamp->ReportTimedMetadata(0, tag.c_str(), content.c_str(), content.size());
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
void PrivateStreamAbstractionMPD::ProcessPeriodSupplementalProperty(Node* node, std::string& AdID, uint64_t startMS, uint64_t durationMS)
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
					s << "#EXT-X-CUE:ID=" << AdID;
					s << ",DURATION=" << std::fixed << std::setprecision(3) << duration;
					s << ",PSN=true";

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", start, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-CUE") {
							aamp->ReportTimedMetadata(startMS, tag.c_str(), content.c_str(), content.size());
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
void PrivateStreamAbstractionMPD::ProcessPeriodAssetIdentifier(Node* node, uint64_t startMS, uint64_t durationMS, std::string& AssetID, std::string& ProviderID)
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
					s << "#EXT-X-ASSET-ID:ID=" << assetID;
					s << ",PROVIDER=" << providerID;
					s << ",DURATION=" << std::fixed << std::setprecision(3) << duration;

					std::string content = s.str();
					AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", start, content.c_str());

					for (int i = 0; i < aamp->subscribedTags.size(); i++)
					{
						const std::string& tag = aamp->subscribedTags.at(i);
						if (tag == "#EXT-X-ASSET-ID") {
							aamp->ReportTimedMetadata(startMS, tag.c_str(), content.c_str(), content.size());
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
		s << "#EXT-X-TRICKMODE-RESTRICTION"
		  << ":ADID=" << AdID
		  << ",MODE=" << trickMode
		  << ",LIMIT=" << text;

		if (!scale.empty()) {
			s << ",SCALE=" << scale;
		}

		std::string content = s.str();
		AAMPLOG_INFO("TimedMetadata: @%1.3f %s\n", start, content.c_str());

		for (int i = 0; i < aamp->subscribedTags.size(); i++)
		{
			const std::string& tag = aamp->subscribedTags.at(i);
			if (tag == "#EXT-X-TRICKMODE-RESTRICTION") {
				aamp->ReportTimedMetadata(startMS, tag.c_str(), content.c_str(), content.size());
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
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
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
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
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
					AAMPLOG_INFO("%s:%d %s EOS - Exit fetch loop\n", __FUNCTION__, __LINE__, downloadParams->pMediaStreamContext->name);
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
		logprintf("%s:%d NULL adaptationSet\n", __FUNCTION__, __LINE__);
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
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
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
					logprintf("%s:%d - skipping schemeUri %s\n", __FUNCTION__, __LINE__, schemeUri.c_str());
				}
			}
		}
		else
		{
			logprintf("%s:%d - skipping name %s\n", __FUNCTION__, __LINE__, xml->GetName().c_str());
		}
	}
	return false;
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
				mLangList.insert(adaptationSet->GetLang());
			}
		}
	}
}


/**
 * @brief Does stream selection
 * @param newTune true if this is a new tune
 */
void PrivateStreamAbstractionMPD::StreamSelection( bool newTune)
{
	int numTracks = (rate == AAMP_NORMAL_PLAY_RATE)?AAMP_TRACK_COUNT:1;
	mNumberOfTracks = 0;

	IPeriod *period = mCurrentPeriod;

	AAMPLOG_INFO("Selected Period index %d, id %s\n", mCurrentPeriodIdx, period->GetId().c_str());

	uint64_t  periodStartMs = 0;
	mPeriodEndTime = GetPeriodEndTime();
	string statTimeStr = period->GetStart();
	if(!statTimeStr.empty())
	{
		ParseISO8601Duration(statTimeStr.c_str(), periodStartMs);
	}
	mPeriodStartTime = periodStartMs / 1000;
	for (int i = 0; i < numTracks; i++)
	{
		struct MediaStreamContext *pMediaStreamContext = mMediaStreamContext[i];
		size_t numAdaptationSets = period->GetAdaptationSets().size();
		int  selAdaptationSetIndex = -1;
		int selRepresentationIndex = -1;
		AudioType selectedRepType = eAUDIO_UNKNOWN, internalSelRepType ;
		int desiredCodecIdx = -1;
		bool otherLanguageSelected = false;
		mMediaStreamContext[i]->enabled = false;
		std::string selectedLanguage;
		bool isIframeAdaptationAvailable = false;
		uint32_t selRepBandwidth = 0;
		uint32_t minVideoRepWidth;
		uint32_t minVideoRepHeight;
		int videoRepresentationIdx;
		for (unsigned iAdaptationSet = 0; iAdaptationSet < numAdaptationSets; iAdaptationSet++)
		{
			IAdaptationSet *adaptationSet = period->GetAdaptationSets().at(iAdaptationSet);
			AAMPLOG_TRACE("PrivateStreamAbstractionMPD::%s %d > Content type [%s] AdapSet [%d] \n",
					__FUNCTION__, __LINE__, adaptationSet->GetContentType().c_str(),iAdaptationSet);
			if (IsContentType(adaptationSet, (MediaType)i ))
			{
				if (AAMP_NORMAL_PLAY_RATE == rate)
				{
					if (eMEDIATYPE_AUDIO == i)
					{
						std::string lang = adaptationSet->GetLang();
						internalSelRepType = selectedRepType;
						// found my language configured
						if(strncmp(aamp->language, lang.c_str(), MAX_LANGUAGE_TAG_LENGTH) == 0)
						{
							// check if already other lang adap is selected, if so start fresh
							if (otherLanguageSelected)
							{
								internalSelRepType = eAUDIO_UNKNOWN;
							}
							desiredCodecIdx = GetDesiredCodecIndex(adaptationSet, internalSelRepType, selRepBandwidth);
							if(desiredCodecIdx != -1 )
							{
								otherLanguageSelected = false;
								selectedRepType	= internalSelRepType;
								selAdaptationSetIndex = iAdaptationSet;
								selRepresentationIndex = desiredCodecIdx;
								mAudioType = selectedRepType;
							}
							logprintf("PrivateStreamAbstractionMPD::%s %d > Got the matching lang[%s] AdapInx[%d] RepIndx[%d] AudioType[%d]\n",
								__FUNCTION__, __LINE__, lang.c_str(), selAdaptationSetIndex, selRepresentationIndex, selectedRepType);
						}
						else if(internalSelRepType == eAUDIO_UNKNOWN || otherLanguageSelected)
						{
							// Got first Adap with diff language , store it now until we find another matching lang adaptation
							desiredCodecIdx = GetDesiredCodecIndex(adaptationSet, internalSelRepType, selRepBandwidth);
							if(desiredCodecIdx != -1)
							{
								otherLanguageSelected = true;
								selectedLanguage = lang;
								selectedRepType	= internalSelRepType;
								selAdaptationSetIndex = iAdaptationSet;
								selRepresentationIndex = desiredCodecIdx;
								mAudioType = selectedRepType;
							}
							logprintf("PrivateStreamAbstractionMPD::%s %d > Got a non-matching lang[%s] AdapInx[%d] RepIndx[%d] AudioType[%d]\n",
								__FUNCTION__, __LINE__, lang.c_str(), selAdaptationSetIndex, selRepresentationIndex, selectedRepType);
						}
					}
					else if (!gpGlobalConfig->bAudioOnlyPlayback)
					{
						if (!isIframeAdaptationAvailable || selAdaptationSetIndex == -1)
						{
							if (!IsIframeTrack(adaptationSet))
							{
								// Got Video , confirmed its not iframe adaptation
								minVideoRepWidth = numeric_limits<uint32_t>::max();
								minVideoRepHeight = numeric_limits<uint32_t>::max();
								videoRepresentationIdx = GetVideoAdaptaionMinResolution(adaptationSet, minVideoRepWidth, minVideoRepHeight);
								AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s %d > Adaptation Set[%d] minVideoRepWidth : %d minVideoRepHeight : %d\n",__FUNCTION__, __LINE__, iAdaptationSet, minVideoRepWidth, minVideoRepHeight);
								if (videoRepresentationIdx != -1)
								{
#ifndef CONTENT_4K_SUPPORTED
									if (minVideoRepWidth <= 1920 && minVideoRepHeight <= 1080)
#endif
									selAdaptationSetIndex =	iAdaptationSet;
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
						logprintf("PrivateStreamAbstractionMPD::%s %d > Got TrickMode track\n", __FUNCTION__, __LINE__);
						pMediaStreamContext->enabled = true;
						pMediaStreamContext->profileChanged = true;
						pMediaStreamContext->adaptationSetIdx = iAdaptationSet;
						mNumberOfTracks = 1;
						isIframeAdaptationAvailable = true;
						break;
					}
				}
			}
		}

		if ((eAUDIO_UNKNOWN == mAudioType) && (AAMP_NORMAL_PLAY_RATE == rate) && (eMEDIATYPE_VIDEO != i) && selAdaptationSetIndex >= 0)
		{
			AAMPLOG_WARN("PrivateStreamAbstractionMPD::%s %d > Selected Audio Track codec is unknown\n", __FUNCTION__, __LINE__);
			mAudioType = eAUDIO_AAC; // assuming content is still playable
		}

		// end of adaptation loop
		if ((AAMP_NORMAL_PLAY_RATE == rate) && (pMediaStreamContext->enabled == false) && selAdaptationSetIndex >= 0)
		{
			pMediaStreamContext->enabled = true;
			pMediaStreamContext->adaptationSetIdx = selAdaptationSetIndex;
			pMediaStreamContext->representationIndex = selRepresentationIndex;
			pMediaStreamContext->profileChanged = true;
			//preferred audio language was not available, hence selected best audio format
			if (otherLanguageSelected)
			{
				if (mLangList.end() ==  mLangList.find(aamp->language))
				{
					logprintf("PrivateStreamAbstractionMPD::%s %d > update language [%s]->[%s]\n",
									__FUNCTION__, __LINE__, aamp->language, selectedLanguage.c_str());
					aamp->UpdateAudioLanguageSelection(selectedLanguage.c_str());
				}
				else
				{
					logprintf("PrivateStreamAbstractionMPD::%s %d > [%s] not available in period. Select [%s]\n",
									__FUNCTION__, __LINE__, aamp->language, selectedLanguage.c_str());
				}
			}

			/* To solve a no audio issue - Force configure gst audio pipeline/playbin in the case of multi period
			 * multi audio codec available for current decoding language on stream. For example, first period has AAC no EC3,
			 * so the player will choose AAC then start decoding, but in the forthcoming periods,
			 * if the stream has AAC and EC3 for the current decoding language then as per the EC3(default priority)
			 * the player will choose EC3 but the audio pipeline actually not configured in this case to affect this change.
			 */
			if ( aamp->previousAudioType != selectedRepType && eMEDIATYPE_AUDIO == i )
			{
				logprintf("PrivateStreamAbstractionMPD::%s %d > AudioType Changed %d -> %d\n",
						__FUNCTION__, __LINE__, aamp->previousAudioType, selectedRepType);
				aamp->previousAudioType = selectedRepType;
				mContext->SetESChangeStatus();
			}

			logprintf("PrivateStreamAbstractionMPD::%s %d > Media[%s] Adaptation set[%d] RepIdx[%d] TrackCnt[%d]\n",
				__FUNCTION__, __LINE__, mMediaTypeName[i],selAdaptationSetIndex,selRepresentationIndex,(mNumberOfTracks+1) );

			ProcessContentProtection(period->GetAdaptationSets().at(selAdaptationSetIndex),(MediaType)i);
			mNumberOfTracks++;
		}

		if(selAdaptationSetIndex < 0 && rate == 1)
		{
			logprintf("PrivateStreamAbstractionMPD::%s %d > No valid adaptation set found for Media[%s]\n",
				__FUNCTION__, __LINE__, mMediaTypeName[i]);
		}

		logprintf("PrivateStreamAbstractionMPD::%s %d > Media[%s] %s\n",
			__FUNCTION__, __LINE__, mMediaTypeName[i], pMediaStreamContext->enabled?"enabled":"disabled");

		//Store the iframe track status in current period if there is any change
		if (!gpGlobalConfig->bAudioOnlyPlayback && (i == eMEDIATYPE_VIDEO) && (mIsIframeTrackPresent != isIframeAdaptationAvailable))
		{
			mIsIframeTrackPresent = isIframeAdaptationAvailable;
			//Iframe tracks changed mid-stream, sent a playbackspeed changed event
			if (!newTune)
			{
				aamp->SendSupportedSpeedsChangedEvent(mIsIframeTrackPresent);
			}
		}
	}

	if(1 == mNumberOfTracks && !mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled)
	{
		if(newTune)
			logprintf("PrivateStreamAbstractionMPD::%s:%d Audio only period\n", __FUNCTION__, __LINE__);
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
void PrivateStreamAbstractionMPD::UpdateTrackInfo(bool modifyDefaultBW, bool periodChanged, bool resetTimeLineIndex)
{
	long defaultBitrate = gpGlobalConfig->defaultBitrate;
	long iframeBitrate = gpGlobalConfig->iframeBitrate;
	bool isFogTsb = mIsFogTSB && !mAdPlayingFromCDN;	/*Conveys whether the current playback from FOG or not.*/

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
					for (int idx = 0; idx < representationCount; idx++)
					{
						Representation* representation = representations.at(idx);
						mStreamInfo[idx].bandwidthBitsPerSecond = representation->GetBandwidth();
						mStreamInfo[idx].isIframeTrack = !(AAMP_NORMAL_PLAY_RATE == rate);
						mStreamInfo[idx].resolution.height = representation->GetHeight();
						mStreamInfo[idx].resolution.width = representation->GetWidth();
						mBitrateIndexMap[mStreamInfo[idx].bandwidthBitsPerSecond] = idx;
						delete representation;
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
					for (int idx = 0; idx < representationCount; idx++)
					{
						IRepresentation *representation = pMediaStreamContext->adaptationSet->GetRepresentation().at(idx);
						mStreamInfo[idx].bandwidthBitsPerSecond = representation->GetBandwidth();
						mStreamInfo[idx].isIframeTrack = !(AAMP_NORMAL_PLAY_RATE == rate);
						mStreamInfo[idx].resolution.height = representation->GetHeight();
						mStreamInfo[idx].resolution.width = representation->GetWidth();
#ifndef CONTENT_4K_SUPPORTED
						if ( ( mStreamInfo[idx].resolution.width <= 1920 ) && ( mStreamInfo[idx].resolution.height <= 1080) )
#endif
						mContext->GetABRManager().addProfile({
							mStreamInfo[idx].isIframeTrack,
							mStreamInfo[idx].bandwidthBitsPerSecond,
							mStreamInfo[idx].resolution.width,
							mStreamInfo[idx].resolution.height,
						});
#ifdef CONTENT_4K_SUPPORTED
						if(mStreamInfo[idx].resolution.height > 1080
								|| mStreamInfo[idx].resolution.width > 1920)
						{
							defaultBitrate = gpGlobalConfig->defaultBitrate4K;
							iframeBitrate = gpGlobalConfig->iframeBitrate4K;
						}
#endif
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
						if (defaultBitrate != DEFAULT_INIT_BITRATE)
						{
							mContext->currentProfileIndex = mContext->GetDesiredProfile(false);
						}
						else
						{
							mContext->currentProfileIndex = mContext->GetDesiredProfile(true);
						}
						pMediaStreamContext->representationIndex = mContext->currentProfileIndex;
						IRepresentation *selectedRepresentation = pMediaStreamContext->adaptationSet->GetRepresentation().at(pMediaStreamContext->representationIndex);
						// for the profile selected ,reset the abr values with default bandwidth values
						aamp->ResetCurrentlyAvailableBandwidth(selectedRepresentation->GetBandwidth(),mContext->trickplayMode,mContext->currentProfileIndex);
						aamp->profiler.SetBandwidthBitsPerSecondVideo(selectedRepresentation->GetBandwidth());
	//					aamp->NotifyBitRateChangeEvent(selectedRepresentation->GetBandwidth(),
	//							"BitrateChanged - Network Adaptation",
	//							selectedRepresentation->GetWidth(),
	//							selectedRepresentation->GetHeight());
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
					logprintf("%s:%d: [WARN] !! representationIndex is '-1' override with '0' since Custom MPD has single representation\n", __FUNCTION__, __LINE__);
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
			pMediaStreamContext->eos = false;
			if(0 == pMediaStreamContext->fragmentDescriptor.Bandwidth || !aamp->IsTSBSupported())
			{
				pMediaStreamContext->fragmentDescriptor.Bandwidth = pMediaStreamContext->representation->GetBandwidth();
			}
			strcpy(pMediaStreamContext->fragmentDescriptor.RepresentationID, pMediaStreamContext->representation->GetId().c_str());
			pMediaStreamContext->fragmentDescriptor.Time = 0;
			ISegmentTemplate *segmentTemplate = pMediaStreamContext->adaptationSet->GetSegmentTemplate();
			if(!segmentTemplate)
			{
				segmentTemplate = pMediaStreamContext->representation->GetSegmentTemplate();
			}
			if(segmentTemplate)
			{
				pMediaStreamContext->fragmentDescriptor.Number = segmentTemplate->GetStartNumber();
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d Track %d timeLineIndex %d fragmentDescriptor.Number %lu\n", __FUNCTION__, __LINE__, i, pMediaStreamContext->timeLineIndex, pMediaStreamContext->fragmentDescriptor.Number);
			}
		}
	}
}



/**
 * @brief Update culling state for live manifests
 */
double PrivateStreamAbstractionMPD::GetCulledSeconds()
{
	double newStartTimeSeconds = 0;
	double culled = 0;
	traceprintf("PrivateStreamAbstractionMPD::%s:%d Enter\n", __FUNCTION__, __LINE__);
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
				for (auto period : periods)
				{
					PeriodInfo periodInfo;
					periodInfo.periodId = period->GetId();
					periodInfo.duration = (double)GetPeriodDuration(period)/ 1000;
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
							logprintf("%s:%d PeriodId %s, prevStart %" PRIu64 " currStart %" PRIu64 " culled %f\n", __FUNCTION__, __LINE__,
												prevPeriodInfo.periodId.c_str(), prevPeriodInfo.startTime, currFirstPeriodInfo.startTime, culled);
						}
						break;
					}
					else
					{
						culled += prevPeriodInfo.duration;
						iter1++;
						logprintf("%s:%d PeriodId %s , with last known duration %f seems to have got culled\n", __FUNCTION__, __LINE__,
										prevPeriodInfo.periodId.c_str(), prevPeriodInfo.duration);
					}
				}
				mMPDPeriodsInfo = currMPDPeriodDetails;
			}
			else
			{
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d NULL segmentTimeline. Hence modifying culling logic based on MPD availabilityStartTime, periodStartTime, fragment number and current time\n", __FUNCTION__, __LINE__);
				string startTimestr = mpd->GetAvailabilityStarttime();
				std::tm time = { 0 };
				strptime(startTimestr.c_str(), "%Y-%m-%dT%H:%M:%SZ", &time);
				double availabilityStartTime = (double)mktime(&time);
				double currentTimeSeconds = aamp_GetCurrentTimeMS() / 1000;
				double fragmentDuration = ((double)segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
				double newStartTimeSeconds = 0;
				if (0 == pMediaStreamContext->lastSegmentNumber)
				{
					newStartTimeSeconds = availabilityStartTime;
				}

				// Recalculate the newStartTimeSeconds after periodic manifest updates
				if (mIsLive && 0 == newStartTimeSeconds)
				{
					newStartTimeSeconds = availabilityStartTime + mPeriodStartTime + ((pMediaStreamContext->lastSegmentNumber - segmentTemplate->GetStartNumber()) * fragmentDuration);
				}

				if (newStartTimeSeconds && mPrevStartTimeSeconds)
				{
					culled = newStartTimeSeconds - mPrevStartTimeSeconds;
					traceprintf("PrivateStreamAbstractionMPD::%s:%d post-refresh %fs before %f (%f)\n\n", __FUNCTION__, __LINE__, newStartTimeSeconds, mPrevStartTimeSeconds, culled);
				}
				else
				{
					logprintf("PrivateStreamAbstractionMPD::%s:%d newStartTimeSeconds %f mPrevStartTimeSeconds %F\n", __FUNCTION__, __LINE__, newStartTimeSeconds, mPrevStartTimeSeconds);
				}
				mPrevStartTimeSeconds = newStartTimeSeconds;
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
					AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d PrevOffset %ld CurrentOffset %ld culled (%f)\n", __FUNCTION__, __LINE__, mPrevLastSegurlOffset, currOffset, culled);
					mPrevLastSegurlOffset = duration - newOffset;
					mPrevLastSegurlMedia = newMedia;
				}
			}
			else
			{
				AAMPLOG_INFO("PrivateStreamAbstractionMPD::%s:%d NULL segmentTemplate and segmentList\n", __FUNCTION__, __LINE__);
			}
		}
	}
	else
	{
		logprintf("PrivateStreamAbstractionMPD::%s:%d NULL adaptationset\n", __FUNCTION__, __LINE__);
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
		if(pMediaStreamContext->enabled && (pMediaStreamContext->profileChanged || discontinuity))
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
							fetchParams->discontinuity = discontinuity;
							int ret = pthread_create(&trackDownloadThreadID, NULL, TrackDownloader, fetchParams);
							if(ret != 0)
							{
								logprintf("PrivateStreamAbstractionMPD::%s:%d pthread_create failed for TrackDownloader with errno = %d, %s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
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
								FetchFragment(pMediaStreamContext, initialization, fragmentDuration,true, (eMEDIATYPE_AUDIO == i), discontinuity);
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
							logprintf("init %s %d..%d\n", mMediaTypeName[pMediaStreamContext->mediaType], start, fin);
#endif
							char fragmentUrl[MAX_URI_LENGTH];
							GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
							if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
							{
								pMediaStreamContext->profileChanged = false;
								if(!pMediaStreamContext->CacheFragment(fragmentUrl, 0, pMediaStreamContext->fragmentTime, 0, range.c_str(), true ))
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f\n", __FUNCTION__, __LINE__, fragmentUrl, pMediaStreamContext->fragmentTime);
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
										logprintf("PrivateStreamAbstractionMPD::%s:%d pthread_create failed for TrackDownloader with errno = %d, %s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
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
									AAMPLOG_INFO("firstSegmentRange %s [%s]\n",
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
											logprintf("PrivateStreamAbstractionMPD::%s:%d segmentList - cannot determine range for Initialization - first segment start %d\n",
													__FUNCTION__, __LINE__, start);
										}
									}
								}
#endif
								if (!range.empty())
								{
									char fragmentUrl[MAX_URI_LENGTH];
									GetFragmentUrl(fragmentUrl, &pMediaStreamContext->fragmentDescriptor, "");
									AAMPLOG_INFO("%s [%s]\n", mMediaTypeName[pMediaStreamContext->mediaType],
											range.c_str());
									if(pMediaStreamContext->WaitForFreeFragmentAvailable(0))
									{
										pMediaStreamContext->profileChanged = false;
										if(!pMediaStreamContext->CacheFragment(fragmentUrl, 0, pMediaStreamContext->fragmentTime, 0.0, range.c_str(), true ))
										{
											logprintf("PrivateStreamAbstractionMPD::%s:%d failed. fragmentUrl %s fragmentTime %f\n", __FUNCTION__, __LINE__, fragmentUrl, pMediaStreamContext->fragmentTime);
										}
									}
								}
								else
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d segmentList - empty range string for Initialization\n",
											__FUNCTION__, __LINE__);
								}
							}
						}
						else
						{
							aamp_Error("not-yet-supported mpd format");
						}
					}
				}
			}
		}
	}

	if(dlThreadCreated)
	{
		AAMPLOG_TRACE("Waiting for pthread_join trackDownloadThread\n");
		pthread_join(trackDownloadThreadID, NULL);
		AAMPLOG_TRACE("Joined trackDownloadThread\n");
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
		logprintf("%s %d Initial period is clear period, trying work around\n",__FUNCTION__,__LINE__);
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
								char fragmentUrl[MAX_URI_LENGTH];
								struct FragmentDescriptor * fragmentDescriptor = (struct FragmentDescriptor *) malloc(sizeof(struct FragmentDescriptor));
								memset(fragmentDescriptor, 0, sizeof(FragmentDescriptor));
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
									logprintf("%s %d Audio type %d\n", __FUNCTION__, __LINE__, selectedAudioType);
								}
								else
								{
									logprintf("%s %d Audio type eAUDIO_UNKNOWN\n", __FUNCTION__, __LINE__);
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
								strcpy(fragmentDescriptor->RepresentationID, representation->GetId().c_str());
								GetFragmentUrl(fragmentUrl,fragmentDescriptor , initialization);
								if (mMediaStreamContext[i]->WaitForFreeFragmentAvailable())
								{
									logprintf("%s %d Pushing encrypted header for %s\n", __FUNCTION__, __LINE__, mMediaTypeName[i]);
									mMediaStreamContext[i]->CacheFragment(fragmentUrl, i, mMediaStreamContext[i]->fragmentTime, 0.0, NULL, true);
								}
								free(fragmentDescriptor);
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

	logprintf("PrivateStreamAbstractionMPD::%s:%d - fetch initialization fragments\n", __FUNCTION__, __LINE__);
	FetchAndInjectInitialization();
	IPeriod *currPeriod = mCurrentPeriod;
	std::string currentPeriodId = currPeriod->GetId();
	mPrevAdaptationSetCount = currPeriod->GetAdaptationSets().size();
	logprintf("aamp: ready to collect fragments. mpd %p\n", mpd);
	do
	{
		bool liveMPDRefresh = false;
		if (mpd)
		{
			size_t numPeriods = mpd->GetPeriods().size();
			unsigned iPeriod = mCurrentPeriodIdx;
			logprintf("MPD has %d periods current period index %d\n", numPeriods, mCurrentPeriodIdx);
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
						IPeriod *oldPeriod = mpd->GetPeriods().at(mCurrentPeriodIdx);
						if(AdState::OUTSIDE_ADBREAK == mCdaiObject->mAdState)
						{
							mBasePeriodOffset = 0;		//Not considering the delta from previous period's duration.
						}
						if(rate > 0)
						{
							mBasePeriodOffset -= ((double)mCdaiObject->mPeriodMap[mBasePeriodId].duration)/1000.00;
						}
						else
						{
							mBasePeriodOffset += ((double)GetPeriodDuration(mpd->GetPeriods().at(iPeriod)))/1000.00;	//Already reached -ve. Subtracting from current period duration
						}

						mCurrentPeriodIdx = iPeriod;
						IPeriod *newPeriod = mpd->GetPeriods().at(iPeriod);
						mBasePeriodId = newPeriod->GetId();

						//We are moving to new period so reset the lastsegment time
						//for VOD and cDVR
						logprintf("%s:%d Period(%s - %d/%d) Offset[%lf] IsLive(%d) IsCdvr(%d) \n",__FUNCTION__,__LINE__,
							mBasePeriodId.c_str(), mCurrentPeriodIdx,numPeriods, mBasePeriodOffset, mIsLive,aamp->IsInProgressCDVR());

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
						periodChanged = false; //If the playing period changes, it will be detected below [if(currentPeriodId != mCurrentPeriod->GetId())]
					}

					adStateChange = onAdEvent(AdEvent::DEFAULT);		//TODO: Vinod, We can optimize here.

					if(AdState::IN_ADBREAK_WAIT2CATCHUP == mCdaiObject->mAdState)
					{
						goto NEEDFRAGMENTS;
					}
					if(adStateChange && AdState::OUTSIDE_ADBREAK == mCdaiObject->mAdState)
					{
						//Just came out from the Adbreak. Need to search the right period
						for(iPeriod=0;iPeriod < numPeriods;  iPeriod++)
						{
							if(mBasePeriodId == mpd->GetPeriods().at(iPeriod)->GetId())
							{
								mCurrentPeriodIdx = iPeriod;
								logprintf("%s:%d [CDAI] Landed at the periodId[%d] \n",__FUNCTION__,__LINE__,mCurrentPeriodIdx);
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
						logprintf("Period ID changed from \'%s\' to \'%s\' [BasePeriodId=\'%s\']\n", currentPeriodId.c_str(),mCurrentPeriod->GetId().c_str(), mBasePeriodId.c_str());
						currentPeriodId = mCurrentPeriod->GetId();
						mPrevAdaptationSetCount = adaptationSetCount;
						logprintf("playing period %d/%d\n", iPeriod, (int)numPeriods);
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
						logprintf("Change in AdaptationSet count; adaptationSetCount %d  mPrevAdaptationSetCount %d,updating stream selection\n", adaptationSetCount, mPrevAdaptationSetCount);
						mPrevAdaptationSetCount = adaptationSetCount;
						requireStreamSelection = true;
					}
					else
					{
						for (int i = 0; i < mNumberOfTracks; i++)
						{
							if(mMediaStreamContext[i]->adaptationSetId != adapatationSets.at(mMediaStreamContext[i]->adaptationSetIdx)->GetId())
							{
								logprintf("AdaptationSet index changed; updating stream selection\n");
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
						bool resetTimeLineIndex = (mIsLive || lastLiveFlag|| periodChanged);
						UpdateTrackInfo(true, periodChanged, resetTimeLineIndex);
					}


					if(mIsLive || lastLiveFlag)
					{
						double culled = 0;
						if(aamp->IsLiveAdjustRequired() && mMediaStreamContext[eMEDIATYPE_VIDEO]->enabled)
						{
							culled = GetCulledSeconds();
						}
						if(culled > 0)
						{
							logprintf("%s:%d Culled seconds = %f\n", __FUNCTION__, __LINE__, culled);
							aamp->UpdateCullingState(culled);
							mCulledSeconds += culled;
						}
						double duration = ((double)PrivateStreamAbstractionMPD::GetDurationFromRepresentation(mpd)) / 1000;
						aamp->UpdateDuration(duration);
						mLiveEndPosition = duration + mCulledSeconds;
						if(mCdaiObject->mContentSeekOffset)
						{
							logprintf("CDAI: Resuming channel playback at PeriodID[%s] at Position[%lf]\n", currentPeriodId.c_str(), mCdaiObject->mContentSeekOffset);
							SeekInPeriod(mCdaiObject->mContentSeekOffset);
							mCdaiObject->mContentSeekOffset = 0;
						}
					}

					lastLiveFlag = mIsLive;
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
							logprintf("%s:%d Error! Audio or Video track missing in period, ignoring discontinuity\n",	__FUNCTION__, __LINE__);
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
								if (nextSegmentTime != segmentStartTime)
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d discontinuity detected nextSegmentTime %" PRIu64 " FirstSegmentStartTime %" PRIu64 " \n", __FUNCTION__, __LINE__, nextSegmentTime, segmentStartTime);
									discontinuity = true;
									mFirstPTS = (double)segmentStartTime/segmentTemplate->GetTimescale();
								}
								else
								{
									logprintf("PrivateStreamAbstractionMPD::%s:%d No discontinuity detected nextSegmentTime %" PRIu64 " FirstSegmentStartTime %" PRIu64 " \n", __FUNCTION__, __LINE__, nextSegmentTime, segmentStartTime);
								}
							}
							else
							{
								traceprintf("PrivateStreamAbstractionMPD::%s:%d Segment template not available\n", __FUNCTION__, __LINE__);
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
										delta = SkipFragments(pMediaStreamContext, delta);
									}

									if(PushNextFragment(pMediaStreamContext,i))
									{
										if (mIsLive)
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
									else if (pMediaStreamContext->eos == true && mIsLive && i == eMEDIATYPE_VIDEO)
									{
										mContext->CheckForPlaybackStall(false);
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
						if((!mIsLive || (rate != AAMP_NORMAL_PLAY_RATE))
							&& ((rate > 0 && mCurrentPeriodIdx >= (numPeriods -1)) || (rate < 0 && 0 == mCurrentPeriodIdx)))
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
							AAMPLOG_INFO("%s:%d EOS - Exit fetch loop \n", __FUNCTION__, __LINE__);
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
							logprintf("%s:%d CDAI: BasePeriod[%s] completed @%lf. Changing to next \n",__FUNCTION__,__LINE__, mBasePeriodId.c_str(),mBasePeriodOffset);
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
					if(timeoutMs <= 0 && mIsLive && rate > 0)
					{
						liveMPDRefresh = true;
						break;
					}
					else if(bCacheFullState)
					{
						// play cache is full , wait until cache is available to inject next, max wait of 1sec
						int timeoutMs = 200;
						AAMPLOG_TRACE("%s:%d Cache full state,no download until(%d) Time(%lld)\n",__FUNCTION__, __LINE__,timeoutMs,aamp_GetCurrentTimeMS());
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

			if (exitFetchLoop || (rate < AAMP_NORMAL_PLAY_RATE && iPeriod < 0) || (rate > 1 && iPeriod >= numPeriods) || mpd->GetType() == "static")
			{
				break;
			}
			else
			{
				traceprintf("PrivateStreamAbstractionMPD::%s:%d Refresh playlist\n", __FUNCTION__, __LINE__);
			}
		}
		else
		{
			logprintf("PrivateStreamAbstractionMPD::%s:%d - null mpd\n", __FUNCTION__, __LINE__);
		}

NEEDFRAGMENTS:
		// If it comes here , two reason a) Reached eos b) Need livempdUpdate
		// If liveMPDRefresh is true , that means it already reached 6 sec timeout .
		// 		No more further delay required for mpd update .
		// If liveMPDRefresh is false, then it hit eos . Here the timeout is calculated based
		// on the buffer availability.
		if (!liveMPDRefresh && mLastPlaylistDownloadTimeMs)
		{
			int minDelayBetweenPlaylistUpdates = (int)mMinUpdateDurationMs;
			int timeSinceLastPlaylistDownload = (int)(aamp_GetCurrentTimeMS() - mLastPlaylistDownloadTimeMs);
			//long bufferAvailable = ((long)(mMediaStreamContext[eMEDIATYPE_VIDEO]->targetDnldPosition*1000) - (long)aamp->GetPositionMs());
			long long currentPlayPosition = aamp->GetPositionMs();
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
						logprintf("Buffer is running low(%ld).Refreshing playlist(%d).PlayPosition(%lld) End(%lld)\n",
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

			AAMPLOG_INFO("aamp playlist end refresh bufferMs(%ld) delay(%d) delta(%d) End(%lld) PlayPosition(%lld)\n",
				bufferAvailable,minDelayBetweenPlaylistUpdates,timeSinceLastPlaylistDownload,endPositionAvailable,currentPlayPosition);

			// sleep before next manifest update
			aamp->InterruptableMsSleep(minDelayBetweenPlaylistUpdates);
		}
		if (!aamp->DownloadsAreEnabled() || UpdateMPD() != eAAMPSTATUS_OK)
		{
			break;
		}

		mCdaiObject->PlaceAds(mpd);

		if(mIsFogTSB)
		{
			//Periods could be added or removed, So select period based on periodID
			//If period ID not found in MPD that means got culled, in that case select
			// first period
			logprintf("Updataing period index after mpd refresh\n");
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
				logprintf("MPD Fragment Collector detected reset in Period(New Size:%d)(currentIdx:%d->%d)\n",
					newPeriods,mCurrentPeriodIdx,newPeriods - 1);
				mCurrentPeriodIdx = newPeriods - 1;
			}
		}
		mpdChanged = true;
	}		//Loop 1
	while (!exitFetchLoop);
	logprintf("MPD fragment collector done\n");
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
	AampDRMSessionManager::getInstance()->setSessionMgrState(SessionMgrState::eSESSIONMGR_ACTIVE);
#endif
	pthread_create(&fragmentCollectorThreadID, NULL, &FragmentCollector, this);
	fragmentCollectorThreadStarted = true;
	for (int i=0; i< mNumberOfTracks; i++)
	{
		// DELIA-30608 - Right place to update targetDnldPosition.
		// Here GetPosition will give updated seek position (for live)
		mMediaStreamContext[i]->targetDnldPosition= aamp->GetPositionMs()/1000;
		mMediaStreamContext[i]->StartInjectLoop();
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
		AAMPLOG_INFO("Waiting to join CreateDRMSession thread\n");
		int rc = pthread_join(createDRMSessionThreadID, NULL);
		if (rc != 0)
		{
			logprintf("pthread_join returned %d for createDRMSession Thread\n", rc);
		}
		AAMPLOG_INFO("Joined CreateDRMSession thread\n");
		drmSessionThreadStarted = false;
	}
	if(fragmentCollectorThreadStarted)
	{
		int rc = pthread_join(fragmentCollectorThreadID, NULL);
		if (rc != 0)
		{
			logprintf("%s:%d ***pthread_join failed, returned %d\n", __FUNCTION__, __LINE__, rc);
		}
		fragmentCollectorThreadStarted = false;
	}
	aamp->mStreamSink->ClearProtectionEvent();
 #ifdef AAMP_MPD_DRM
	AampDRMSessionManager *sessionMgr = AampDRMSessionManager::getInstance();
	sessionMgr->setSessionMgrState(SessionMgrState::eSESSIONMGR_INACTIVE);
	sessionMgr->clearFailedKeyIds();
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

	aamp->CurlTerm(0, AAMP_TRACK_COUNT);

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

/**
 * @}
 */
void CDAIObjectMPD::SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS)
{
	mPrivObj->SetAlternateContents(periodId, adId, url, startMS);
}

static void *AdFulfillThreadEntry(void *arg)
{
	PrivateCDAIObjectMPD *_this = (PrivateCDAIObjectMPD *)arg;
	if(aamp_pthread_setname(pthread_self(), "AdFulfillThread"))
	{
		logprintf("%s:%d: aamp_pthread_setname failed\n", __FUNCTION__, __LINE__);
	}
	_this->FulFillAdObject();
	return NULL;
}

void PrivateCDAIObjectMPD::FulFillAdObject()
{
	bool adStatus = false;
	uint64_t startMS = 0;
	uint64_t durationMs = 0;
	bool finalManifest = false;
	MPD *ad = GetAdMPD(mAdFulfillObj.url, finalManifest, true);
	if(ad)
	{
		std::lock_guard<std::mutex> lock(mDaiMtx);
		auto periodId = mAdFulfillObj.periodId;
		if(ad->GetPeriods().size() && isAdBreakObjectExist(periodId))	// Ad has periods && ensuring that the adbreak still exists
		{
			auto &adbreakObj = mAdBreaks[periodId];
			std::shared_ptr<std::vector<AdNode>> adBreakAssets = adbreakObj.ads;
			durationMs = PrivateStreamAbstractionMPD::GetDurationFromRepresentation(ad);

			startMS = adbreakObj.duration;
			adbreakObj.duration += durationMs;

			std::string bPeriodId = "";		//BasePeriodId will be filled on placement
			int bOffset = -1;				//BaseOffset will be filled on placement
			if(0 == adBreakAssets->size())
			{
				//First Ad placement is doing now.
				if(isPeriodExist(periodId))
				{
					mPeriodMap[periodId].offset2Ad[0] = AdOnPeriod{0,0};
				}

				mPlacementObj.pendingAdbrkId = periodId;
				mPlacementObj.openPeriodId = periodId;	//May not be available Now.
				mPlacementObj.curEndNumber = 0;
				mPlacementObj.curAdIdx = 0;
				mPlacementObj.adNextOffset = 0;
				bPeriodId = periodId;
				bOffset = 0;
			}
			if(!finalManifest)
			{
				logprintf("%s:%d: Final manifest to be downloaded from the FOG later. Deleting the manifest got from CDN.\n", __FUNCTION__, __LINE__);
				delete ad;
				ad = NULL;
			}
			adBreakAssets->emplace_back(AdNode{false, false, mAdFulfillObj.adId, mAdFulfillObj.url, durationMs, bPeriodId, bOffset, ad});
			logprintf("%s:%d: New Ad[Id=%s, url=%s] successfully added.\n", __FUNCTION__, __LINE__, mAdFulfillObj.adId.c_str(),mAdFulfillObj.url.c_str());

			adStatus = true;
		}
		else
		{
			logprintf("%s:%d: AdBreadkId[%s] not existing. Dropping the Ad.\n", __FUNCTION__, __LINE__, periodId.c_str());
			delete ad;
		}
	}
	else
	{
		logprintf("%s:%d: Failed to get Ad MPD[%s].\n", __FUNCTION__, __LINE__, mAdFulfillObj.url.c_str());
	}
	mAamp->SendAdResolvedEvent(mAdFulfillObj.adId, adStatus, startMS, durationMs);
}

void PrivateCDAIObjectMPD::SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS)
{
	if("" == adId || "" == url)
	{
		std::lock_guard<std::mutex> lock(mDaiMtx);
		//Putting a place holder
		if(!(isAdBreakObjectExist(periodId)))
		{
			auto adBreakAssets = std::make_shared<std::vector<AdNode>>();
			mAdBreaks.emplace(periodId, AdBreakObject{0, adBreakAssets, "", 0});	//Fix the duration after getting the Ad
			Period2AdData &pData = mPeriodMap[periodId];
			pData.adBreakId = periodId;
		}
	}
	else
	{
		if(mAdObjThreadID)
		{
			//Clearing the previous thread
			int rc = pthread_join(mAdObjThreadID, NULL);
			mAdObjThreadID = 0;
		}
		mAdFulfillObj.periodId = periodId;
		mAdFulfillObj.adId = adId;
		mAdFulfillObj.url = url;
		int ret = pthread_create(&mAdObjThreadID, NULL, &AdFulfillThreadEntry, this);
		if(ret != 0)
		{
			logprintf("%s:%d pthread_create(FulFillAdObject) failed, errno = %d, %s. Rejecting promise.\n", __FUNCTION__, __LINE__, errno, strerror(errno));
			mAamp->SendAdResolvedEvent(mAdFulfillObj.adId, false, 0, 0);
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
	bool ret = false;
	std::vector<IEventStream *> eventStreams = period->GetEventStreams();
	for(int i=0; i<eventStreams.size(); i++)
	{
		std::vector<IEvent *> events =  eventStreams.at(i)->GetEvents();
		for(int j=0; j<events.size(); j++)
		{
			IEvent * event = events.at(j);
			if(event && event->GetDuration())
			{
				const vector<INode*> nodes = event->GetAdditionalSubNodes();
				if(nodes.size() && "scte35:Signal" == nodes[0]->GetName())
				{
					uint32_t timeScale = 1;
					if(eventStreams.at(i)->GetTimescale() > 1)
					{
						timeScale = eventStreams.at(i)->GetTimescale();
					}
					duration = (event->GetDuration()/timeScale)*1000; //milliseconds
					scte35 = nodes[0]->GetText();
					ret = true;
					break;
				}
			}
		}
	}
	return ret;
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
				bool seamLess = (AdEvent::INIT == evt)?false:(AAMP_NORMAL_PLAY_RATE == rate);
				int adIdx = mCdaiObject->CheckForAdStart(seamLess, mBasePeriodId, mBasePeriodOffset, brkId, adOffset);
				if(!brkId.empty())
				{
					mCdaiObject->mCurPlayingBreakId = brkId;
					if(-1 != adIdx && mCdaiObject->mAdBreaks[brkId].ads)
					{
						if(!(mCdaiObject->mAdBreaks[brkId].ads->at(adIdx).invalid))
						{
							logprintf("%s:%d CDAI: ADBREAK STARTING AdIdx[%d] Found at Period[%s].\n",__FUNCTION__,__LINE__, adIdx, mBasePeriodId.c_str());
							mCdaiObject->mCurAds = mCdaiObject->mAdBreaks[brkId].ads;

							mCdaiObject->mCurAdIdx = adIdx;
							mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_PLAYING;

							for(int i=0; i<adIdx; i++)
								adPos2Send += mCdaiObject->mCurAds->at(i).duration;
						}
						else
						{
							logprintf("%s:%d CDAI: AdIdx[%d] in the AdBreak[%s] is invalid. Skipping.\n",__FUNCTION__,__LINE__, adIdx, brkId.c_str());
						}
						reservationEvt2Send = AAMP_EVENT_AD_RESERVATION_START;
						adbreakId2Send = brkId;
						if(AdEvent::INIT == evt) sendImmediate = true;
					}

					if(AdState::IN_ADBREAK_AD_PLAYING != mCdaiObject->mAdState)
					{
						logprintf("%s:%d CDAI: BasePeriodId[%s] in Adbreak. But Ad not available.\n",__FUNCTION__,__LINE__, mBasePeriodId.c_str());
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
				int adIdx = mCdaiObject->CheckForAdStart((AAMP_NORMAL_PLAY_RATE == rate), mBasePeriodId, mBasePeriodOffset, brkId, adOffset);
				if(-1 != adIdx && mCdaiObject->mAdBreaks[brkId].ads)
				{
					if(!(mCdaiObject->mAdBreaks[brkId].ads->at(adIdx).invalid))
					{
						logprintf("%s:%d CDAI: AdIdx[%d] Found at Period[%s].\n",__FUNCTION__,__LINE__, adIdx, mBasePeriodId.c_str());
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
					logprintf("%s:%d CDAI: Adbreak[%s] ENDED. Playing the basePeriod[%s].\n",__FUNCTION__,__LINE__, mCdaiObject->mCurPlayingBreakId.c_str(), mBasePeriodId.c_str());
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
				logprintf("%s:%d CDAI: Ad[idx=%d] finished at Period[%s]. Waiting to catchup the base offset..\n",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str());
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
				logprintf("%s:%d CDAI: Ad[idx=%d] Playback failed. Going to the base period[%s] at offset[%lf].\n",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str(), mBasePeriodOffset);
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
				logprintf("%s:%d CDAI: AdIdx[-1]. BUG! BUG!! BUG!!! We should not come here.\n",__FUNCTION__,__LINE__);
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
				logprintf("%s:%d CDAI: Current Ad placement Completed. Ready to play next Ad.\n",__FUNCTION__,__LINE__);
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
						logprintf("%s:%d CDAI: AdIdx[%d] in invalid. Skipping!!.\n",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx);
						mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING;
					}
					else
					{
						logprintf("%s:%d CDAI: Next AdIdx[%d] Found at Period[%s].\n",__FUNCTION__,__LINE__, mCdaiObject->mCurAdIdx, mBasePeriodId.c_str());
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
						mCdaiObject->mContentSeekOffset = mCdaiObject->mAdBreaks[mCdaiObject->mCurPlayingBreakId].endPeriodOffset;
					}
					else
					{
						//TODO: Vinod, Need to make sure that the mBasePeriodId reaches just previous period of the adbreak.
						 mCdaiObject->mContentSeekOffset = 0; //Should continue tricking from the end of the previous period.
//						mCdaiObject->mContentSeekOffset = mBasePeriodOffset;	//mBasePeriodOffset offset should be the endPeriodOffset. If not, check why?
					}
					logprintf("%s:%d CDAI: All Ads in the Adbreak[%s] FINISHED. Playing the basePeriod[%s] at Offset[%lf].\n",__FUNCTION__,__LINE__, mCdaiObject->mCurPlayingBreakId.c_str(), mBasePeriodId.c_str(), mCdaiObject->mContentSeekOffset);
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
		logprintf("%s:%d [CDAI]: State changed from [%s] => [%s].\n",__FUNCTION__,__LINE__, ADSTATE_STR[static_cast<int>(oldState)],ADSTATE_STR[static_cast<int>(mCdaiObject->mAdState)]);

		mAdPlayingFromCDN = false;
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
					//TODO: Vinod, if mpd is still NULL we need to revert everything. This is a temporary fix
					mCdaiObject->mAdState = AdState::IN_ADBREAK_AD_NOT_PLAYING;
					return stateChanged;
				}
			}
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

		if(AAMP_NORMAL_PLAY_RATE == rate)
		{
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
			}

			if(AAMP_EVENT_AD_PLACEMENT_START == placementEvt2Send || AAMP_EVENT_AD_PLACEMENT_END == placementEvt2Send || AAMP_EVENT_AD_PLACEMENT_ERROR == placementEvt2Send)
			{
				uint32_t adDuration = 30000;
				if(AAMP_EVENT_AD_PLACEMENT_START == placementEvt2Send)
				{
					adDuration = mCdaiObject->mCurAds->at(mCdaiObject->mCurAdIdx).duration;
					adPos2Send += adOffset;
				}
				aamp->SendAdPlacementEvent(placementEvt2Send,adId2Send, adPos2Send, adOffset, adDuration, sendImmediate);
				if(AAMP_EVENT_AD_PLACEMENT_ERROR == placementEvt2Send)
				{
					aamp->SendAdPlacementEvent(AAMP_EVENT_AD_PLACEMENT_END,adId2Send, adPos2Send, adOffset, adDuration, true);	//Ad ended with error
				}
			}

			if(AAMP_EVENT_AD_RESERVATION_END == reservationEvt2Send)
			{
				aamp->SendAdReservationEvent(reservationEvt2Send,adbreakId2Send, resPosMS, sendImmediate);
			}
		}
	}
	return stateChanged;
}

