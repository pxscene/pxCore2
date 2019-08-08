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

// NOTE - protex_scan is failing hence changed file name from LicenseStatistics.cpp LicnStatistics.cpp

#include "LicnStatistics.h"

#define TAG_TOTAL_ROTATIONS			"r"  	// Total License rotation or stream switches
#define TAG_TOTAL_ENCRYPTED_TO_CLEAR		"e"		// Total Encrypted to Clear Switch
#define TAG_TOTAL_CLEAR_TO_ENCRYPTED		"c"		// Total Clear  to Encrypted Switch


/**
 *   @brief  Converts class object data to Json object
 *
 *   @param[in]  NONE
 *
 *   @return cJSON pointer
 */
cJSON * CLicenseStatistics::ToJson() const
{
	cJSON *monitor = NULL;

	if(mTotalRotations !=0 ||
	   mTotalEncryptedToClear != 0 ||
	   mTotalClearToEncrypted != 0 	)
	{
		monitor = cJSON_CreateObject();
		if(monitor)
		{
			cJSON * jsonObj =  cJSON_CreateNumber(mTotalRotations);
			cJSON_AddItemToObject(monitor, TAG_TOTAL_ROTATIONS, jsonObj);

			jsonObj =  cJSON_CreateNumber(mTotalEncryptedToClear);
			cJSON_AddItemToObject(monitor, TAG_TOTAL_ENCRYPTED_TO_CLEAR, jsonObj);

			jsonObj =  cJSON_CreateNumber(mTotalClearToEncrypted);
			cJSON_AddItemToObject(monitor, TAG_TOTAL_CLEAR_TO_ENCRYPTED, jsonObj);

		}
	}
	return monitor;
}


/**
 *   @brief  Increments License stat count
 *   @param[in]  VideoStatCountType
 *
 *   @return None
 */
void CLicenseStatistics::IncrementCount(VideoStatCountType type)
{
    if( isInitialized ) 
    {
        switch (type) {
			case COUNT_LIC_TOTAL:
			{
				mTotalRotations++;
			}
				break;
			case COUNT_LIC_ENC_TO_CLR:
			{
				mTotalEncryptedToClear++;
			}
				break;
			case COUNT_LIC_CLR_TO_ENC:
			{
				mTotalClearToEncrypted++;
			}
				break;
			default:
				break;
		}
    }
}

/**
 *   @brief  Records license stat
 *   @param[in]  isEncypted indicates track or fragment is encrypted, based on this info clear to enc or enc to clear stats are incremented
 *   @param[in]  isKeyChanged indicates if keychanged for encrypted fragment
 *   @return None
 */
void CLicenseStatistics::Record_License_EncryptionStat(bool  isEncypted, bool isKeyChanged)
{
    if(isInitialized) 
    {
        // Encrypted to clear
        if(mbEncypted   && !isEncypted)
        {
            IncrementCount(VideoStatCountType::COUNT_LIC_ENC_TO_CLR);
        }

        // Clear  to Encrypted
        if(!mbEncypted   && isEncypted)
        {
            IncrementCount(VideoStatCountType::COUNT_LIC_CLR_TO_ENC);
        }
        
        if(isKeyChanged)
        {
            IncrementCount(VideoStatCountType::COUNT_LIC_TOTAL);
        }
    }

    if( !isInitialized )
    {
        isInitialized = true;
    }
    mbEncypted = isEncypted;
}
