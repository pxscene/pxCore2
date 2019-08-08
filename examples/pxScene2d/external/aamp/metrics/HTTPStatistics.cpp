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

#include "HTTPStatistics.h"

#define TAG_HTTP_4XX_ERRORS			"4"  	// Count of HTTP-4XX Errors
#define TAG_HTTP_5XX_ERRORS			"5"  	// Count of HTTP-5XX Errors
#define TAG_CURL_TIMEOUT_ERRORS			"t"  	// Count of Curl Timeout Errors
#define TAG_CURL_ERRORS				"c"  	// Count of Other Curl Errors
#define TAG_SUCCESS				"s"		// Count of Successful downloads


/**
 *   @brief  Increment stat count
 *
 *   @param[in]  VideoStatCountType
 *
 *   @return None
 */
void CHTTPStatistics::IncrementCount(VideoStatCountType type)
{
	switch (type) {

		case COUNT_4XX:
		{
			mHttp4xxCount++;
		}
			break;
		case COUNT_CURL:
		{
			mCurlCount++;
		}
			break;
		case COUNT_CURL_TIMEOUT:
		{
			mTimeOutCount++;
		}
			break;

		case COUNT_SUCCESS:
		{
			mSuccessCount++;
		}
			break;

		case COUNT_5XX:
		{
			http5xxCount++;
		}
			break;

		default:
			break;
	}
}


/**
 *   @brief  Converts class object data to Json object
 *
 *   @param[in]  NONE
 *
 *   @return cJSON pointer
 */
cJSON * CHTTPStatistics::ToJson() const
{

	bool bDataAdded = false;
	cJSON *monitor = cJSON_CreateObject();
	cJSON * jsonObj =  NULL;
	if(monitor)
	{


		if(mHttp4xxCount > 0 )
		{
			jsonObj = cJSON_CreateNumber(mHttp4xxCount);
			cJSON_AddItemToObject(monitor, TAG_HTTP_4XX_ERRORS, jsonObj);
		}

		if(mCurlCount > 0)
		{
			jsonObj =  cJSON_CreateNumber(mCurlCount);
			cJSON_AddItemToObject(monitor, TAG_CURL_ERRORS, jsonObj);
		}

		if(mTimeOutCount >0)
		{
			jsonObj =  cJSON_CreateNumber(mTimeOutCount);
			cJSON_AddItemToObject(monitor, TAG_CURL_TIMEOUT_ERRORS, jsonObj);
		}

		if(http5xxCount >0)
		{
			jsonObj =  cJSON_CreateNumber(http5xxCount);
			cJSON_AddItemToObject(monitor, TAG_HTTP_5XX_ERRORS, jsonObj);
		}

		if(mSuccessCount > 0 )
		{
			jsonObj =  cJSON_CreateNumber(mSuccessCount);
			cJSON_AddItemToObject(monitor, TAG_SUCCESS, jsonObj);
		}

	}

	if(jsonObj == NULL)
	{
		//None of the data got added so delete
		cJSON_Delete(monitor);
		monitor = NULL;
	}

	return monitor;
}



