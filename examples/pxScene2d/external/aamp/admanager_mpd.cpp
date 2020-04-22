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
 * @file admanager_mpd.cpp
 * @brief Client side DAI manger for MPEG DASH
 */

#include "admanager_mpd.h"
#include "fragmentcollector_mpd.h"
#include <inttypes.h>

#include <algorithm>

static void *AdFulfillThreadEntry(void *arg)
{
    PrivateCDAIObjectMPD *_this = (PrivateCDAIObjectMPD *)arg;
    if(aamp_pthread_setname(pthread_self(), "AdFulfillThread"))
    {
        logprintf("%s:%d: aamp_pthread_setname failed", __FUNCTION__, __LINE__);
    }
    _this->FulFillAdObject();
    return NULL;
}


CDAIObjectMPD::CDAIObjectMPD(PrivateInstanceAAMP* aamp): CDAIObject(aamp), mPrivObj(new PrivateCDAIObjectMPD(aamp))
{

}

CDAIObjectMPD::~CDAIObjectMPD()
{
	delete mPrivObj;
}

/**
 * @}
 */
void CDAIObjectMPD::SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS, uint32_t breakdur)
{
	mPrivObj->SetAlternateContents(periodId, adId, url, startMS, breakdur);
}



PrivateCDAIObjectMPD::PrivateCDAIObjectMPD(PrivateInstanceAAMP* aamp) : mAamp(aamp),mDaiMtx(), mIsFogTSB(false), mAdBreaks(), mPeriodMap(), mCurPlayingBreakId(), mAdObjThreadID(0), mAdFailed(false), mCurAds(nullptr),
					mCurAdIdx(-1), mContentSeekOffset(0), mAdState(AdState::OUTSIDE_ADBREAK),mPlacementObj(), mAdFulfillObj()
{
	mAamp->CurlInit(eCURLINSTANCE_DAI,1,mAamp->GetNetworkProxy());
}

PrivateCDAIObjectMPD::~PrivateCDAIObjectMPD()
{
	if(mAdObjThreadID)
	{
		int rc = pthread_join(mAdObjThreadID, NULL);
		if (rc != 0)
		{
			logprintf("%s:%d ***pthread_join failed, returned %d", __FUNCTION__, __LINE__, rc);
		}
		mAdObjThreadID = 0;
	}
	mAamp->CurlTerm(eCURLINSTANCE_DAI);
}

void PrivateCDAIObjectMPD::InsertToPeriodMap(IPeriod * period)
{
	const std::string &prdId = period->GetId();
	if(!isPeriodExist(prdId))
	{
		mPeriodMap[prdId] = Period2AdData();
	}
}

bool PrivateCDAIObjectMPD::isPeriodExist(const std::string &periodId)
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
			AAMPLOG_INFO("%s:%d [CDAI] Removing the period[%s] from mAdBreaks.", __FUNCTION__, __LINE__, adBrkObj.first.c_str());
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
		AdBreakObject &abObj = mAdBreaks[mPlacementObj.pendingAdbrkId];
		vector<IPeriod *> periods = mpd->GetPeriods();
		if(!abObj.adjustEndPeriodOffset) // not all ads are placed
		{
			bool openPrdFound = false;

			for(int iter = 0; iter < periods.size(); iter++)
			{
				if(abObj.adjustEndPeriodOffset)
				{
					// placement done no need to run for loop now
					break;
				}

				auto period = periods.at(iter);
				const std::string &periodId = period->GetId();
				//We need to check, open period is available in the manifest. Else, something wrong
				if(mPlacementObj.openPeriodId == periodId)
				{
					openPrdFound = true;
				}
				else if(openPrdFound)
				{
					if(aamp_GetPeriodDuration(mpd, iter, 0) > 0)
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
					uint64_t periodDelta = aamp_GetPeriodNewContentDuration(period, mPlacementObj.curEndNumber);
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
						else if((mPlacementObj.curAdIdx < (abObj.ads->size()-1))    //If it is not the last Ad, we can start placement immediately.
								|| periodDelta >= OFFSET_ALIGN_FACTOR)              //Making sure that period has sufficient space to fallback
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
								//Place the end markers of adbreak
								abObj.endPeriodId = periodId;	//If it is the exact period boundary, end period will be the next one
								abObj.endPeriodOffset = p2AdData.duration - periodDelta;
								abObj.adjustEndPeriodOffset = true; // marked for later adjustment
								break;
							}
						}
						else
						{
							//No more ads to place & No sufficient space to finalize. Wait for next period/next mpd refresh.
							break;
						}
					}
				}
			}
		}
		if(abObj.adjustEndPeriodOffset) // make endPeriodOffset adjustment 
		{
			bool endPeriodFound = false;
			int iter =0;

			for(iter = 0; iter < periods.size(); iter++)
			{
				auto period = periods.at(iter);
				const std::string &periodId = period->GetId();
				//We need to check, end period is available in the manifest. Else, something wrong
				if(abObj.endPeriodId == periodId)
				{
					endPeriodFound = true;
					break;
				}
			}

			if(false == endPeriodFound) // something wrong keep the end-period positions same and proceed.
			{
				abObj.adjustEndPeriodOffset = false;
				AAMPLOG_WARN("%s:%d [CDAI] Couldn't adjust offset [endPeriodNotFound] ", __FUNCTION__, __LINE__);
			}
			else
			{
				//Inserted Ads finishes in < 4 seconds of new period (inside the adbreak) : Play-head goes to the period’s beginning.
				if(abObj.endPeriodOffset < 2*OFFSET_ALIGN_FACTOR)
				{
					abObj.adjustEndPeriodOffset = false; // done with Adjustment
					abObj.endPeriodOffset = 0;//Aligning the last period
					mPeriodMap[abObj.endPeriodId] = Period2AdData(); //Resetting the period with small out-lier.
					AAMPLOG_INFO("%s:%d [CDAI] Adjusted endperiodOffset", __FUNCTION__, __LINE__);
				}
				else
				{
					// get current period duration
					uint64_t currPeriodDuration = aamp_GetPeriodDuration(mpd, iter, 0);

					// Are we too close to current period end?
					//--> Inserted Ads finishes < 2 seconds behind new period : Channel play-back starts from new period.
					int diff = currPeriodDuration - abObj.endPeriodOffset;
					// if diff is negative or < OFFSET_ALIGN_FACTOR we have to wait for it to catch up
					// and either period will end with diff < OFFSET_ALIGN_FACTOR then adjust to next period start
					// or diff will be more than OFFSET_ALIGN_FACTOR then don't do any adjustment
					if(diff <  OFFSET_ALIGN_FACTOR)
					{
						//check if next period available
						iter++;
						if( iter < periods.size() )
						{
							auto nextPeriod = periods.at(iter);
							abObj.adjustEndPeriodOffset = false; // done with Adjustment
							abObj.endPeriodOffset = 0;//Aligning to next period start
							abObj.endPeriodId = nextPeriod->GetId();
							mPeriodMap[abObj.endPeriodId] = Period2AdData();
							AAMPLOG_INFO("%s:%d [CDAI] [%d] close to period end [%lld],Aligning to next-period:%s", __FUNCTION__, __LINE__,
														diff,currPeriodDuration,abObj.endPeriodId.c_str());
						}
						else
						{
							AAMPLOG_INFO("%s:%d [CDAI] [%d] close to period end [%lld],but next period not available,waiting", __FUNCTION__, __LINE__,
														diff,currPeriodDuration);
						}
					}// --> Inserted Ads finishes >= 2 seconds behind new period : Channel playback starts from that position in the current period.
					// OR //--> Inserted Ads finishes in >= 4 seconds of new period (inside the adbreak) : Channel playback starts from that position in the period.
					else
					{
						AAMPLOG_INFO("%s:%d [CDAI] [%d] NOT close to period end [%lld] Done", __FUNCTION__, __LINE__,diff);
						abObj.adjustEndPeriodOffset = false; // done with Adjustment
					}
				}
			}

			if(!abObj.adjustEndPeriodOffset) // placed all ads now print the placement data and set mPlacementObj.curAdIdx = -1;
			{
				mPlacementObj.curAdIdx = -1;
				//Printing the placement positions
				std::stringstream ss;
				ss<<"{AdbreakId: "<<mPlacementObj.pendingAdbrkId;
				ss<<", duration: "<<abObj.adsDuration;
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
				AAMPLOG_WARN("%s:%d [CDAI] Placement Done: %s.", __FUNCTION__, __LINE__, ss.str().c_str());

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

int PrivateCDAIObjectMPD::CheckForAdStart(const float &rate, bool init, const std::string &periodId, double offSet, std::string &breakId, double &adOffset)
{
	int adIdx = -1;
	auto pit = mPeriodMap.find(periodId);
	if(mPeriodMap.end() != pit && !(pit->second.adBreakId.empty()))
	{
		//mBasePeriodId belongs to an Adbreak. Now we need to see whether any Ad is placed in the offset.
		Period2AdData &curP2Ad = pit->second;
		if(isAdBreakObjectExist(curP2Ad.adBreakId))
		{
			breakId = curP2Ad.adBreakId;
			AdBreakObject &abObj = mAdBreaks[breakId];
			bool seamLess = init?false:(AAMP_NORMAL_PLAY_RATE == rate);
			if(seamLess)
			{
				int floorKey = (int)(offSet * 1000);
				floorKey = floorKey - (floorKey%OFFSET_ALIGN_FACTOR);
				auto adIt = curP2Ad.offset2Ad.find(floorKey);
				if(curP2Ad.offset2Ad.end() == adIt)
				{
					//Need in cases like the current offset=29.5sec, next adAdSart=30.0sec
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

			if(rate >= AAMP_NORMAL_PLAY_RATE && -1 == adIdx && abObj.endPeriodId == periodId && (uint64_t)(offSet*1000) >= abObj.endPeriodOffset)
			{
				breakId = "";	//AdState should not stick to IN_ADBREAK after Adbreak ends.
			}
		}
	}
	return adIdx;
}

bool PrivateCDAIObjectMPD::CheckForAdTerminate(double currOffset)
{
	uint64_t fragOffset = (uint64_t)(currOffset * 1000);
	if(fragOffset >= (mCurAds->at(mCurAdIdx).duration + OFFSET_ALIGN_FACTOR))
	{
		//Current Ad is playing beyond the AdBreak + OFFSET_ALIGN_FACTOR
		return true;
	}
	return false;
}

bool PrivateCDAIObjectMPD::isPeriodInAdbreak(const std::string &periodId)
{
	return !(mPeriodMap[periodId].adBreakId.empty());
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
	bool gotManifest = false;
	long http_error = 0;
	std::string effectiveUrl;
	memset(&manifest, 0, sizeof(manifest));
	gotManifest = mAamp->GetFile(manifestUrl, &manifest, effectiveUrl, &http_error, NULL, eCURLINSTANCE_DAI);
	if (gotManifest)
	{
		AAMPLOG_TRACE("PrivateCDAIObjectMPD::%s - manifest download success", __FUNCTION__);
	}
	else if (mAamp->DownloadsAreEnabled())
	{
		AAMPLOG_ERR("PrivateCDAIObjectMPD::%s - manifest download failed", __FUNCTION__);
	}

	if (gotManifest)
	{
		finalManifest = true;
		xmlTextReaderPtr reader = xmlReaderForMemory(manifest.ptr, (int) manifest.len, NULL, NULL, 0);
		if(tryFog && !(gpGlobalConfig->playAdFromCDN) && reader && mIsFogTSB)	//Main content from FOG. Ad is expected from FOG.
		{
			std::string channelUrl = mAamp->GetManifestUrl();	//TODO: Get FOG URL from channel URL
			std::string encodedUrl;
			UrlEncode(effectiveUrl, encodedUrl);
			int ipend = 0;
			for(int slashcnt=0; ipend < channelUrl.length(); ipend++)
			{
				if(channelUrl[ipend] == '/')
				{
					slashcnt++;
					if(slashcnt >= 3)
					{
						break;
					}
				}
			}

			effectiveUrl.assign(channelUrl.c_str(), 0, ipend);
			effectiveUrl.append("/adrec?clientId=FOG_AAMP&recordedUrl=");
			effectiveUrl.append(encodedUrl.c_str());
			GrowableBuffer fogManifest;
			memset(&fogManifest, 0, sizeof(manifest));
			http_error = 0;
			mAamp->GetFile(effectiveUrl, &fogManifest, effectiveUrl, &http_error, NULL, eCURLINSTANCE_DAI);
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
				Node* root = aamp_ProcessNode(&reader, manifestUrl, true);
				if (NULL != root)
				{
					std::vector<Node*> children = root->GetSubNodes();
					for (size_t i = 0; i < children.size(); i++)
					{
						Node* child = children.at(i);
						const std::string& name = child->GetName();
						AAMPLOG_INFO("PrivateCDAIObjectMPD::%s - child->name %s", __FUNCTION__, name.c_str());
						if (name == "Period")
						{
							AAMPLOG_INFO("PrivateCDAIObjectMPD::%s - found period", __FUNCTION__);
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
								AAMPLOG_INFO("PrivateCDAIObjectMPD::%s - manual adding BaseURL Node [%p] text %s",
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
					logprintf("%s:%d - Could not create root node", __FUNCTION__, __LINE__);
				}
			}
			else
			{
				logprintf("%s:%d - xmlTextReaderRead failed", __FUNCTION__, __LINE__);
			}
			xmlFreeTextReader(reader);
		}
		else
		{
			logprintf("%s:%d - xmlReaderForMemory failed", __FUNCTION__, __LINE__);
		}

		if (gpGlobalConfig->logging.trace)
		{
			aamp_AppendNulTerminator(&manifest); // make safe for cstring operations
			logprintf("%s:%d - Ad manifest: %s", __FUNCTION__, __LINE__, manifest.ptr);
		}
		aamp_Free(&manifest.ptr);
	}
	else
	{
		logprintf("%s:%d - aamp: error on manifest fetch", __FUNCTION__, __LINE__);
	}
	return adMpd;
}

void PrivateCDAIObjectMPD::FulFillAdObject()
{
	bool adStatus = false;
	uint64_t startMS = 0;
	uint32_t durationMs = 0;
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
			durationMs = aamp_GetDurationFromRepresentation(ad);

			startMS = adbreakObj.adsDuration;
			uint32_t availSpace = adbreakObj.brkDuration - startMS;
			if(availSpace < durationMs)
			{
				AAMPLOG_WARN("%s:%d: Adbreak's available space[%lu] < Ad's Duration[%lu]. Trimming the Ad.", __FUNCTION__, __LINE__, availSpace, durationMs);
				durationMs = availSpace;
			}
			adbreakObj.adsDuration += durationMs;

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
				AAMPLOG_INFO("%s:%d: Final manifest to be downloaded from the FOG later. Deleting the manifest got from CDN.", __FUNCTION__, __LINE__);
				delete ad;
				ad = NULL;
			}
			adBreakAssets->emplace_back(AdNode{false, false, mAdFulfillObj.adId, mAdFulfillObj.url, durationMs, bPeriodId, bOffset, ad});
			AAMPLOG_WARN("%s:%d: New Ad[Id=%s, url=%s] successfully added.", __FUNCTION__, __LINE__, mAdFulfillObj.adId.c_str(),mAdFulfillObj.url.c_str());

			adStatus = true;
		}
		else
		{
			logprintf("%s:%d: AdBreadkId[%s] not existing. Dropping the Ad.", __FUNCTION__, __LINE__, periodId.c_str());
			delete ad;
		}
	}
	else
	{
		logprintf("%s:%d: Failed to get Ad MPD[%s].", __FUNCTION__, __LINE__, mAdFulfillObj.url.c_str());
	}
	mAamp->SendAdResolvedEvent(mAdFulfillObj.adId, adStatus, startMS, durationMs);
}

void PrivateCDAIObjectMPD::SetAlternateContents(const std::string &periodId, const std::string &adId, const std::string &url,  uint64_t startMS, uint32_t breakdur)
{
	if("" == adId || "" == url)
	{
		std::lock_guard<std::mutex> lock(mDaiMtx);
		//Putting a place holder
		if(!(isAdBreakObjectExist(periodId)))
		{
			auto adBreakAssets = std::make_shared<std::vector<AdNode>>();
			mAdBreaks.emplace(periodId, AdBreakObject{breakdur, adBreakAssets, "", 0, 0});	//Fix the duration after getting the Ad
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
		if(isAdBreakObjectExist(periodId))
		{
			auto &adbreakObj = mAdBreaks[periodId];
			int ret = 0;
			if(adbreakObj.brkDuration <= adbreakObj.adsDuration)
			{
				AAMPLOG_WARN("%s:%d - No more space left in the Adbreak. Rejecting the promise.", __FUNCTION__, __LINE__);
				ret = -1;
			}
			else
			{
				mAdFulfillObj.periodId = periodId;
				mAdFulfillObj.adId = adId;
				mAdFulfillObj.url = url;
				int ret = pthread_create(&mAdObjThreadID, NULL, &AdFulfillThreadEntry, this);
				if(ret != 0)
				{
					logprintf("%s:%d pthread_create(FulFillAdObject) failed, errno = %d, %s. Rejecting promise.", __FUNCTION__, __LINE__, errno, strerror(errno));
				}
			}
			if(ret != 0)
			{
				mAamp->SendAdResolvedEvent(mAdFulfillObj.adId, false, 0, 0);
			}
		}
	}
}
