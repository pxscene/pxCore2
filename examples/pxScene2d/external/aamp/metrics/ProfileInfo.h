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




#ifndef AAMP_SRC_PROFILEINFO_H
#define AAMP_SRC_PROFILEINFO_H

#include "FragmentStatistics.h"
#include "LicnStatistics.h"

/*
 *  Stores Track types, With respect to Video Stat
 *  Main Manifest download is also considared as seperate track as Stats are same.
 *  Max of 5 Audio tracks data collection supported and seperate IFRAME stats are collected in single track type STAT_IFRAME
 */
typedef enum E_VideoStatTrackType {
	STAT_UNKNOWN,
	STAT_MAIN, // HLS Main manifest or DASH Manifest
	STAT_VIDEO,
	STAT_IFRAME,
	STAT_AUDIO_1,
	STAT_AUDIO_2,
	STAT_AUDIO_3,
	STAT_AUDIO_4,
	STAT_AUDIO_5

} VideoStatTrackType;

typedef enum E_VideoStatDataType{
	VE_DATA_UNKNOWN,
	VE_DATA_MANIFEST,
	VE_DATA_FRAGMENT,
	VE_DATA_INIT_FRAGMENT,
	VE_DATA_LICENSE
} VideoStatDataType;

/*
 *  Profile is consist of Manifest , Fragment and License stat,
 *  Here there are two types of fragments Init and Normal and last failed URL is stored
 *  for only fragments hence separate class is created for fragment CFragmentStatistics
 */
class CProfileInfo
{
private:
	CHTTPStatistics * mpManifestStat;
	CFragmentStatistics * mpFragmentStat;
public:

	/**
	 *   @brief Default constructor
	 *
	 *   @param[in]  NONE
         *
	 *   @return None
	 */
	CProfileInfo() : mpManifestStat(NULL),mpFragmentStat(NULL)
	{

	}

	CProfileInfo(const CProfileInfo & newObj) :CProfileInfo()
	{
		if(newObj.mpManifestStat)
		{
			mpManifestStat = new CHTTPStatistics(*newObj.mpManifestStat);
		}

		if(newObj.mpFragmentStat)
		{
			this->mpFragmentStat = new CFragmentStatistics(*newObj.mpFragmentStat);
		}
	}

	CProfileInfo& operator=(const CProfileInfo& newObj)
	{
		if(newObj.mpManifestStat)
		{
			mpManifestStat = GetManifestStat(); // Allocate if required

			*mpManifestStat = *newObj.mpManifestStat;
		}
		else
		{
			if(mpManifestStat)
			{
				delete mpManifestStat;
				mpManifestStat = NULL;
			}
		}

		if(newObj.mpFragmentStat)
		{
			mpFragmentStat = GetFragementStat(); // Allocate if required

			*mpFragmentStat = *newObj.mpFragmentStat;
		}
		else
		{
			if(mpFragmentStat)
			{
				delete mpFragmentStat;
				mpFragmentStat = NULL;
			}
		}

		return *this;
	}

	/**
	 *   @brief Default Destructor
	 *
	 *   @param[in]  NONE
         *
	 *   @return None
	 */
	~CProfileInfo()
	{
		if(mpManifestStat)
		{
			delete mpManifestStat;
		}
		if(mpFragmentStat)
		{
			delete mpFragmentStat;
		}
	}

	/**
	 *   @brief Returns Manifest stat and allocates if not allocated.
	 *
	 *   @param[in]  NONE
         *
	 *   @return CHTTPStatistics pointer
	 */
	CHTTPStatistics * GetManifestStat()
	{
		if(!mpManifestStat)
		{
			mpManifestStat = new CHTTPStatistics();
		}
		return mpManifestStat;
	}

	/**
	 *   @brief Returns Fragment stat and allocates if not allocated.
	 *
	 *   @param[in]  NONE
         *
	 *   @return CFragmentStatistics pointer
	 */
	CFragmentStatistics * GetFragementStat()
	{
		if(!mpFragmentStat)
		{
			mpFragmentStat = new CFragmentStatistics();
		}
		return mpFragmentStat;
	}

	/**
	 *   @brief  Converts class object data to Json object
	 *
	 *   @param[in]  NONE
     *
	 *   @return cJSON pointer
	 */
	cJSON * ToJson() const;


};

#endif //AAMP_SRC_PROFILEINFO_H
