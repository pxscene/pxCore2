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
 * @file VideoStat.cpp
 * @brief VideoStat data packing and storage
 */

#include "VideoStat.h"

#include <iostream>

#define TAG_VERSION					"vr" 	// version of video end event
//timetoTop
#define TAG_TIME_TO_TOP					"tt"  	// time to reach top profile
#define TAG_TIME_AT_TOP					"ta" 	//time for which video remain on top profile
#define TAG_TIME_PLAYBACK_DURATION			"d" 	// time for which playback was done, this is measured at the time of fragment download , hence play-back duration may be slightly less due to g-streamer and aamp buffers
#define TAG_ABR_NET_DROP				"dn"   	// Step down profile count happened due to Bad network bandwidth
#define TAG_ABR_ERR_DROP				"de"   	// Step down profile count happened due to Bad download errors/failures
#define TAG_TSB_AVAILIBLITY				"t"		// indicates if TSB used for playback,
// TAGs for Playback types
#define TAG_MAIN					"m"		// Main manifest
#define TAG_VIDEO					"v"		// Video Profile
#define TAG_IFRAME					"i"		// Iframe Profile
#define TAG_AUDIO_1					"a1"	// Audio track 1
#define TAG_AUDIO_2					"a2"	// Audio track 2
#define TAG_AUDIO_3					"a3"	// Audio track 3
#define TAG_AUDIO_4					"a4"	// Audio track 4
#define TAG_AUDIO_5					"a5"	// Audio track 5
#define TAG_UNKNOWN					"u"		// Unknown Profile or track type

#define TAG_SUPPORTED_LANG				"l"		// Supported language
#define TAG_PROFILES 					"p"		// Encapsulates Different Profile available in stream
#define TAG_LICENSE_STAT				"ls"	// License statistics






/**
 *   @brief Returns string of JSON object
 *
 *   @param[in]  None
 *
 *   @return char * - Note that caller is responsible for deleting memory allocated for string
 */
char * CVideoStat::ToJsonString() const
{
	char * strRet = NULL;
	cJSON *monitor = cJSON_CreateObject();
	if(monitor)
	{

		cJSON * jsonObj = NULL;

		jsonObj = cJSON_CreateString(VIDEO_END_DATA_VERSION);
		cJSON_AddItemToObject(monitor, TAG_VERSION, jsonObj);


		if(mTmeToTopProfile > 0 )
		{
			jsonObj =  cJSON_CreateNumber(mTmeToTopProfile);
			cJSON_AddItemToObject(monitor, TAG_TIME_TO_TOP, jsonObj);
		}

		if(mTimeAtTopProfile > 0)
		{
			jsonObj =  cJSON_CreateNumber(mTimeAtTopProfile);
			cJSON_AddItemToObject(monitor, TAG_TIME_AT_TOP, jsonObj);
		}

		if(mTotalVideoDuration > 0 )
		{
			jsonObj =  cJSON_CreateNumber(mTotalVideoDuration);
			cJSON_AddItemToObject(monitor, TAG_TIME_PLAYBACK_DURATION, jsonObj);
		}

		if(mTotalVideoDuration >0 )
		{
			jsonObj =  cJSON_CreateNumber(mAbrNetworkDropCount);
			cJSON_AddItemToObject(monitor, TAG_ABR_NET_DROP, jsonObj);
		}

		if(mAbrErrorDropCount > 0)
		{
			jsonObj =  cJSON_CreateNumber(mAbrErrorDropCount);
			cJSON_AddItemToObject(monitor, TAG_ABR_ERR_DROP, jsonObj);
		}

		if(mbTsb)
		{
			jsonObj =  cJSON_CreateNumber(1);
			cJSON_AddItemToObject(monitor, TAG_TSB_AVAILIBLITY, jsonObj);
		}

		bool isDataAdded = false;

		cJSON *langList = cJSON_CreateObject();
		for (auto const& langItem : mMapLang)
		{

			std::string value = TrackTypeToString(langItem.first);
			if(!value.empty() && !langItem.second.empty())
			{
				isDataAdded = true;
				cJSON *strObj = cJSON_CreateString(langItem.second.c_str());
				cJSON_AddItemToObject(langList, value.c_str(), strObj);
			}
		}

		if(isDataAdded)
		{
			cJSON_AddItemToObject(monitor, TAG_SUPPORTED_LANG, langList);
		}
		else
		{
			cJSON_Delete(langList);
		}



		for (auto const& mapProfileInfo : mMapStreamInfo)
		{
			cJSON *profiles = cJSON_CreateObject();
			jsonObj = NULL;

			isDataAdded = false;

			for (auto const& profileInfo : mapProfileInfo.second)
			{

				jsonObj = profileInfo.second.ToJson();
				if(jsonObj)
				{
					std::string profileIndiex = std::to_string(profileInfo.first);

					cJSON_AddItemToObject(profiles, profileIndiex.c_str(), jsonObj);
					isDataAdded = true;
				}

			}
			
			if(isDataAdded) // at least one profile added to profiles
			{

				cJSON * trackJson  = cJSON_CreateObject();

				cJSON_AddItemToObject(trackJson, TAG_PROFILES, profiles);

				cJSON * licenceJson = NULL;

				auto  it = mMapLicenseInfo.find(mapProfileInfo.first);
				if (it != mMapLicenseInfo.end())
				{
					licenceJson = (*it).second.ToJson();
				}

				if(licenceJson)
				{
					cJSON_AddItemToObject(trackJson, TAG_LICENSE_STAT, licenceJson);
				}

				cJSON_AddItemToObject(monitor, TrackTypeToString(mapProfileInfo.first).c_str(), trackJson);
			}
			else
			{
				cJSON_Delete(profiles);
			}
		}
		
		strRet = cJSON_PrintUnformatted(monitor);
		cJSON_Delete(monitor);
	}

	return strRet;

}


/**
 *   @brief Increment Normal Fragment stats
 *
 *   @param[in] VideoStatTrackType - Indicates track for which Increment required
 *    @param[in] VideoStatCountType - Type of count type
 *    bitrate : profile bitrate
 *
 *   @return None
 */
void CVideoStat::Increment_Fragment_Count(VideoStatTrackType eType, VideoStatCountType eCountType, long bitrate)
{
	if(eType != STAT_MAIN) // fragment stats are not applicable for main hls or dash manifest
	{
		//MapProfileInfo mapProfileInfo = mMapStreamInfo[eType];
		CProfileInfo * pinfo = &(mMapStreamInfo[eType][bitrate]);
		pinfo->GetFragementStat()->GetNormalFragmentStat()->IncrementCount(eCountType);
	}
}

/**
 *   @brief Increment Init Fragment stats ( used for dash case only )
 *
 *   @param[in] VideoStatTrackType - Indicates track for which Increment required
 *    @param[in] VideoStatCountType - Type of count type
 *    bitrate : profile bitrate
 *
 *   @return None
 */
void CVideoStat::Increment_Init_Fragment_Count(VideoStatTrackType eType, VideoStatCountType eCountType, long bitrate)
{
	if(eType != STAT_MAIN) // fragment stats are not applicable for main hls or dash manifest
	{
		CProfileInfo * pinfo = &(mMapStreamInfo[eType][bitrate]);
		pinfo->GetFragementStat()->GetInitFragmentStat()->IncrementCount(eCountType);
	}
}

/**
 *   @brief Increment Manifest stats
 *
 *   @param[in] VideoStatTrackType - Indicates track for which Increment required
 *    @param[in] VideoStatCountType - Type of count type
 *    bitrate : profile bitrate ( 0 means Main HLS Mainifest or DASH manifest )
 *
 *   @return None
 */
void CVideoStat::Increment_Manifest_Count(VideoStatTrackType eType, VideoStatCountType eCountType, long bitrate)
{
	CProfileInfo * pinfo = &(mMapStreamInfo[eType][bitrate]);
	pinfo->GetManifestStat()->IncrementCount(eCountType);
}


/**
 *   @brief   Records License stat based on isEncypted
 *
 *   @param[in] VideoStatTrackType - Indicates track
 *   @param[in] isEncypted - Indicates clear(false) or encrypted ( true)
 *   @param[in] isKeyChanged - indicates if key is changed for encrypted fragment
 *   @return None
 */
void CVideoStat::Record_License_EncryptionStat(VideoStatTrackType eType, bool isEncypted, bool isKeyChanged)
{
    
	mMapLicenseInfo[eType].Record_License_EncryptionStat(isEncypted,isKeyChanged);
}

/**
 *   @brief Sets URL for failed download fragments
 *
 *   @param[in]  long long time
     *
 *   @return None
 */
void CVideoStat::SetFailedFragmentUrl(VideoStatTrackType eType, long bitrate, std::string url)
{
	if(eType != STAT_MAIN) // fragment stats are not applicable for main hls or dash manifest
	{
		CProfileInfo * pinfo = &(mMapStreamInfo[eType][bitrate]);
		pinfo->GetFragementStat()->SetUrl(url);
	}
}

/**
 *   @brief sets Lang associated with Audio Tracks
 *
 *   @param[in]  VideoStatTrackType - Audio Track
 *   @param[in]  std::string lang string
     *
 *   @return None
 */
void CVideoStat::Setlanguage(VideoStatTrackType eType, std::string strLang)
{
	switch(eType)
	{
		case VideoStatTrackType::STAT_AUDIO_1:
		case VideoStatTrackType::STAT_AUDIO_2:
		case VideoStatTrackType::STAT_AUDIO_3:
		case VideoStatTrackType::STAT_AUDIO_4:
		case VideoStatTrackType::STAT_AUDIO_5:
		{
			this->mMapLang[eType] = strLang;
		}
			break;


		default:
			break;
	}

}

/**
 *   @brief Converts Tracktype enum to string
 *
 *   @param[in]  VideoStatTrackType - track type
 *
 *   @return std::string - track type in string format
 */
std::string CVideoStat::TrackTypeToString(VideoStatTrackType type) const
{
	std::string strRetVal;
	switch (type) {
		case STAT_MAIN:
		{
			strRetVal = TAG_MAIN;
		}
			break;
			
		case STAT_VIDEO:
		{
			strRetVal =TAG_VIDEO;
		}
			break;
		case STAT_IFRAME:
		{
			strRetVal = TAG_IFRAME;
		}
			break;
			
		case STAT_AUDIO_1:
		{
			strRetVal = TAG_AUDIO_1;
		}
			break;

		case STAT_AUDIO_2:
		{
			strRetVal = TAG_AUDIO_2;
		}
			break;
			
		case STAT_AUDIO_3:
		{
			strRetVal = TAG_AUDIO_3;
		}
			break;
		case STAT_AUDIO_4:
		{
			strRetVal = TAG_AUDIO_4;
		}
			break;
			
		case STAT_AUDIO_5:
		{
			strRetVal =TAG_AUDIO_5;
		}
			break;
		default:
			strRetVal = TAG_UNKNOWN;
			break;
	}
	
	return strRetVal;
}



/**
 *   @brief Increment stats ,
 *
 *   @param[in] VideoStatDataType - indicates type of Data ( e.g manifest/fragment/license etc )
 *   @param[in] VideoStatTrackType - Indicates track for which Increment required
 *    @param[in] VideoStatCountType - Type of count type
 *    bitrate : profile bitrate
 *
 *   @return None
 */
void CVideoStat::Increment_Data(VideoStatDataType dataType,VideoStatTrackType eType, VideoStatCountType eCountType, long bitrate )
{
	switch(dataType)
	{
		case VE_DATA_MANIFEST:
			{
				Increment_Manifest_Count(eType,eCountType,bitrate );
			}
				break;
		case VE_DATA_FRAGMENT:
			{
				Increment_Fragment_Count(eType,eCountType,bitrate );
			}
				break;
		case VE_DATA_INIT_FRAGMENT:
			{
				Increment_Init_Fragment_Count(eType,eCountType,bitrate );
			}
				break;
			default:

				break;
	}
}
