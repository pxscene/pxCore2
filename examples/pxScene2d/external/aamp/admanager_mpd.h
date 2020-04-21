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
 * @file admanager_mpd.h
 * @brief Client side DAI manger for MPEG DASH
 */

#ifndef ADMANAGER_MPD_H_
#define ADMANAGER_MPD_H_

#include "AdManagerBase.h"
#include <string>
#include "libdash/INode.h"
#include "libdash/IDASHManager.h"
#include "libdash/xml/Node.h"
#include "libdash/IMPD.h"

using namespace dash;
using namespace std;
using namespace dash::mpd;

/**
 * @class CDAIObjectMPD
 *
 * @brief Client Side DAI object implementation for DASH
 */
class CDAIObjectMPD: public CDAIObject
{
	class PrivateCDAIObjectMPD* mPrivObj;    /**< Private instance of Client Side DAI object for DASH */
public:

	/**
	* @brief CDAIObjectMPD Constructor
	*
	 * @param[in] aamp - Pointer to PrivateInstanceAAMP
	*/
	CDAIObjectMPD(PrivateInstanceAAMP* aamp);

	/**
	 * @brief CDAIObjectMPD destructor.
	 */
	virtual ~CDAIObjectMPD();

	/**
	* @brief CDAIObject Copy Constructor
	*/
	CDAIObjectMPD(const CDAIObjectMPD&) = delete;

	/**
	* @brief CDAIObject assignment operator overloading
	*/
	CDAIObjectMPD& operator= (const CDAIObjectMPD&) = delete;

	/**
	 *   @brief Getter for the PrivateCDAIObjectMPD instance
	 *
	 *   @return PrivateCDAIObjectMPD object pointer
	 */
	PrivateCDAIObjectMPD* GetPrivateCDAIObjectMPD()
	{
		return mPrivObj;
	}

	/**
	 *   @brief Setting the alternate contents' (Ads/blackouts) URL
	 *
	 *   @param[in] periodId - Adbreak's unique identifier; the first period id
	 *   @param[in] adId - Individual Ad's id
	 *   @param[in] url - Ad URL
	 *   @param[in] startMS - Ad start time in milliseconds
	 *   @param[in] breakdur - Adbreak's duration in MS
	 */
	virtual void SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url, uint64_t startMS=0, uint32_t breakdur=0) override;
};


/**
 * @brief Events to the Ad manager's state machine
 */
enum class AdEvent
{
	INIT,                       /**< Playback initialization */
	BASE_OFFSET_CHANGE,         /**< Base period's offset change */
	AD_FINISHED,                /**< Ad playback finished */
	AD_FAILED,                  /**< Ad playback failed */
	PERIOD_CHANGE,              /**< Period changed */
	DEFAULT = PERIOD_CHANGE     /**< Default event */
};

#define OFFSET_ALIGN_FACTOR 2000 /**< Observed minor slacks in the ad durations. Align factor used to place the ads correctly. */

/**
 * @struct AdNode
 * @brief Individual Ad's meta info
 */
struct AdNode {
	bool         invalid;          /**< Failed to playback failed once, no need to attempt later */
	bool         placed;           /**< Ad completely placed on the period */
	std::string  adId;             /**< Ad's identifier */
	std::string  url;              /**< Ad's manifest URL */
	uint64_t     duration;         /**< Duration of the Ad */
	std::string  basePeriodId;     /**< Id of the base period at the beginning of the Ad */
	int          basePeriodOffset; /**< Offset of the base period at the beginning of the Ad */
	MPD*         mpd;              /**< Pointer to the MPD object */

	/**
	* @brief AdNode default constructor
	*/
	AdNode() : invalid(false), placed(false), adId(), url(), duration(0), basePeriodId(), basePeriodOffset(0), mpd(nullptr)
	{

	}

	/**
	* @brief AdNode constructor
	*
	* @param[in] invalid - Is Ad valid
	* @param[in] placed - Is Ad completed placed over underlying period
	* @param[in] adId - Ad identifier
	* @param[in] url - Ad's manifest URL
	* @param[in] duration - Duration of the Ad
	* @param[in] basePeriodId - Base period id of the Ad
	* @param[in] basePeriodOffset - Base period offset of the Ad
	* @param[in] mpd - Pointer to the MPD object
	*/
	AdNode(bool invalid, bool placed, std::string adId, std::string url, uint64_t duration,
									std::string basePeriodId, int basePeriodOffset, MPD* mpd)
	: invalid(invalid), placed(placed), adId(adId), url(url), duration(duration), basePeriodId(basePeriodId),
		basePeriodOffset(basePeriodOffset), mpd(mpd)
	{

	}

	/**
	* @brief AdNode Copy Constructor
	*
	* @param[in] adNode - Reference to the source AdNode
	*/
	AdNode(const AdNode& adNode) : invalid(adNode.invalid), placed(adNode.placed), adId(adNode.adId),
									url(adNode.url), duration(adNode.duration), basePeriodId(adNode.basePeriodId),
									basePeriodOffset(adNode.basePeriodOffset), mpd(adNode.mpd)
	{
	}

	/**
	* @brief AdNode assignment operator overloading
	*/
	AdNode& operator=(const AdNode&) = delete;
};

/**
 * @struct AdBreakObject
 *
 * @brief AdBreak's metadata object
 */
struct AdBreakObject{
	uint32_t                             brkDuration;     /**< Adbreak's duration */
	std::shared_ptr<std::vector<AdNode>> ads;             /**< Ads in the Adbreak in sequential order */
	std::string                          endPeriodId;     /**< Base period's id after the adbreak playback */
	uint64_t                             endPeriodOffset; /**< Base period's offset after the adbreak playback */
	uint32_t                             adsDuration;     /**< Ads' duration in the Adbreak */
	bool                        	     adjustEndPeriodOffset;     /**< endPeriodOffset needs be re-adjusted or not */

	/**
	* @brief AdBreakObject default constructor
	*/
	AdBreakObject() : brkDuration(0), ads(), endPeriodId(), endPeriodOffset(0), adsDuration(0), adjustEndPeriodOffset(false)
	{
	}

	/**
	* @brief AdBreakObject constructor
	*
	* @param[in] _duration - Adbreak's duration
	* @param[in] _ads - Ads in the Adbreak
	* @param[in] _endPeriodId - Base period's id after the adbreak playback
	* @param[in] _endPeriodOffset - Base period's offset after the adbreak playback
	* @param[in] _adsDuration - Ads' duration in the Adbreak
	*/
	AdBreakObject(uint32_t _duration, std::shared_ptr<std::vector<AdNode>> _ads, std::string _endPeriodId,
	uint64_t _endPeriodOffset, uint32_t _adsDuration)
	: brkDuration(_duration), ads(_ads), endPeriodId(_endPeriodId), endPeriodOffset(_endPeriodOffset), adsDuration(_adsDuration), adjustEndPeriodOffset(false)
	{
	}
};

/**
 * @struct AdOnPeriod
 *
 * @brief Individual Ad's object placed over the period
 */
struct AdOnPeriod
{
	int32_t  adIdx;           /**< Ad's idx (of vector) */
	uint32_t adStartOffset;   /**< Ad's start offset */
};

/**
 * @struct Period2AdData
 * @brief Meta info corresponding to each period.
 */
struct Period2AdData {
	bool                        filled;      /**< Period filled with ads or not */
	std::string                 adBreakId;   /**< Parent Adbreak */
	uint64_t                    duration;    /**< Period's Duration */
	std::map<int, AdOnPeriod>   offset2Ad;   /**< Mapping period's offset to individual Ads */

	/**
	* @brief Period2AdData constructor
	*/
	Period2AdData() : filled(false), adBreakId(), duration(0), offset2Ad()
	{
	}
};

/**
 * @struct AdFulfillObj
 *
 * @brief Temporary object representing currently fulfilling ad (given by setAlternateContent).
 */
struct AdFulfillObj {
	std::string periodId;      /**< Currently fulfilling adbreak id */
	std::string adId;          /**< Currently placing Ad id */
	std::string url;           /**< Current Ad's URL */

	/**
	* @brief AdFulfillObj constructor
	*/
	AdFulfillObj() : periodId(), adId(), url()
	{

	}
};

/**
 * @struct PlacementObj
 *
 * @brief Currently placing Ad's object
 */
struct PlacementObj {
	std::string pendingAdbrkId;         /**< Only one Adbreak will be pending for replacement */
	std::string openPeriodId;           /**< The period in the adbreak that is progressing */
	uint64_t    curEndNumber;           /**< Current periods last fragment number */
	int         curAdIdx;               /**< Currently placing ad, during MPD progression */
	uint32_t    adNextOffset;           /**< Current Ad's offset to be placed in the next iteration of PlaceAds */

	/**
	* @brief PlacementObj constructor
	*/
	PlacementObj(): pendingAdbrkId(), openPeriodId(), curEndNumber(0), curAdIdx(-1), adNextOffset(0)
	{

	}
};


/**
 * @class PrivateCDAIObjectMPD
 *
 * @brief Private Client Side DAI object for DASH
 */
class PrivateCDAIObjectMPD
{
public:
	PrivateInstanceAAMP*                           mAamp;               /**< AAMP player's private instance */
	std::mutex                                     mDaiMtx;             /**< Mutex protecting DAI critical section */
	bool                                           mIsFogTSB;           /**< Channel playing from TSB or not */
	std::unordered_map<std::string, AdBreakObject> mAdBreaks;           /**< Periodid to adbreakobject map*/
	std::unordered_map<std::string, Period2AdData> mPeriodMap;          /**< periodId to Ad map */
	std::string                                    mCurPlayingBreakId;  /**< Currently playing Ad */
	pthread_t                                      mAdObjThreadID;      /**< ThreadId of Ad fulfillment */
	bool                                           mAdFailed;           /**< Current Ad playback failed flag */
	std::shared_ptr<std::vector<AdNode>>           mCurAds;             /**< Vector of ads from the current Adbreak */
	int                                            mCurAdIdx;           /**< Currently playing Ad index */
	AdFulfillObj                                   mAdFulfillObj;       /**< Temporary object for Ad fulfillment (to pass to the fulfillment thread) */
	PlacementObj                                   mPlacementObj;       /**< Temporary object for Ad placement over period */
	double                                         mContentSeekOffset;  /**< Seek offset after the Ad playback */
	AdState                                        mAdState;            /**< Current state of the CDAI state machine */

	/**
	* @brief PrivateCDAIObjectMPD constructor
	*
	* @param[in] aamp - Pointer to PrivateInstanceAAMP
	*/
	PrivateCDAIObjectMPD(PrivateInstanceAAMP* aamp);

	/**
	* @brief PrivateCDAIObjectMPD destructor
	*/
	~PrivateCDAIObjectMPD();

	/**
	* @brief PrivateCDAIObjectMPD copy constructor
	*/
	PrivateCDAIObjectMPD(const PrivateCDAIObjectMPD&) = delete;

	/**
	* @brief PrivateCDAIObjectMPD assignment operator
	*/
	PrivateCDAIObjectMPD& operator= (const PrivateCDAIObjectMPD&) = delete;

	/**
	 * @brief Setting the alternate contents' (Ads/blackouts) URL
	 *
	 * @param[in] adBreakId - Adbreak's unique identifier.
	 * @param[in] adId - Individual Ad's id
	 * @param[in] url - Ad URL
	 * @param[in] startMS - Ad start time in milliseconds
	 * @param[in] breakdur - Adbreak's duration in MS
	 */
	void SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS, uint32_t breakdur=0);

	/**
	 *   @brief Method for fullfilling the Ad
	 */
	void FulFillAdObject();

	/**
	 * @brief Method for downloading and parsing Ad's MPD
	 *
	 * @param[in]  url - Ad manifest's URL
	 * @param[out] finalManifest - Is final MPD or the final MPD should be downloaded later
	 * @param[in]  tryFog - Attempt to download from FOG or not
	 *
	 * @return Pointer to the MPD object
	 */
	MPD*  GetAdMPD(std::string &url, bool &finalManifest, bool tryFog = false);

	/**
	 * @brief Method to insert period into period map
	 *
	 * @param[in]  period - Pointer of the period to be inserted
	 */
	void InsertToPeriodMap(IPeriod *period);

	/**
	 * @brief Method to check the existence of period in the period map
	 *
	 * @param[in]  periodId - Period id to be checked.
	 * @return true or false
	 */
	bool isPeriodExist(const std::string &periodId);

	/**
	 * @brief Method to check the existence of Adbreak object in the AdbreakObject map
	 *
	 * @param[in]  adBrkId - Adbreak id to be checked.
	 * @return true or false
	 */
	inline bool isAdBreakObjectExist(const std::string &adBrkId);

	/**
	 * @brief Method to remove expired periods from the period map
	 *
	 * @param[in]  newPeriodIds - Period ids from the latest manifest
	 */
	void PrunePeriodMaps(std::vector<std::string> &newPeriodIds);

	/**
	 * @brief Method to reset the state of the CDAI state machine
	 */
	void ResetState();

	/**
	 * @brief Method to clear the maps in the CDAI object
	 */
	void ClearMaps();

	/**
	 * @brief Method to create a bidirectional between the ads and the underlying content periods
	 */
	void  PlaceAds(dash::mpd::IMPD *mpd);

	/**
	 * @brief Checking to see if a period is the begining of the Adbreak or not.
	 *
	 * @param[in]  rate - Playback rate
	 * @param[in]  periodId - Period id to be checked
	 * @param[in]  offSet - Period offset to be checked
	 * @param[out] breakId - Id of the Adbreak, if the period & offset falls in an Adbreak
	 * @param[out] adOffset - Offset of the Ad for that point of the period
	 *
	 * @return Ad index, if the period has an ad over it. Else -1
	 */
	int CheckForAdStart(const float &rate, bool init, const std::string &periodId, double offSet, std::string &breakId, double &adOffset);

	/**
	 * @brief Checking to see if the position in a period corresponds to an end of Ad playback or not
	 *
	 * @param[in]  fragmentTime - Current offset in the period
	 *
	 * @return True or false
	 */
	bool CheckForAdTerminate(double fragmentTime);

	/**
	 * @brief Checking to see if a period has Adbreak
	 *
	 * @param[in]  periodId - Period id
	 *
	 * @return True or false
	 */
	inline bool isPeriodInAdbreak(const std::string &periodId);
};


#endif /* ADMANAGER_MPD_H_ */
