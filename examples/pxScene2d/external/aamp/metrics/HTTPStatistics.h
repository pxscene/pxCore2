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

#ifndef __HTTP_STATISTICS_H__
#define __HTTP_STATISTICS_H__

#include <stdio.h>
#include <map>
#include <string>
#include <cjson/cJSON.h>

#define COUNT_NONE	0
#define VIDEO_END_DATA_VERSION		"1.0"

/*
 * Defines Video Stat count type
 */
typedef enum E_VideoStatCountType {
	COUNT_UNKNOWN,
	COUNT_LIC_TOTAL,
	COUNT_LIC_ENC_TO_CLR,
	COUNT_LIC_CLR_TO_ENC,
	COUNT_4XX,
	COUNT_5XX,
	COUNT_CURL, // all other curl errors except timeout
	COUNT_CURL_TIMEOUT,
	COUNT_SUCCESS
} VideoStatCountType;


/*
 *  Class to store all Video stats common to all download types
 */
class CHTTPStatistics
{
protected:
	int mHttp4xxCount;
	int http5xxCount;
	int mCurlCount; // other than timeout
	int mTimeOutCount;
	int mSuccessCount;
public:
	CHTTPStatistics() : mHttp4xxCount(COUNT_NONE) ,http5xxCount(COUNT_NONE) ,
						mCurlCount(COUNT_NONE),mSuccessCount(COUNT_NONE),mTimeOutCount(COUNT_NONE)
	{

	}


	/**
	 *   @brief  Increment stat count
	 *
	 *   @param[in]  VideoStatCountType
     *
	 *   @return None
	 */
	void IncrementCount(VideoStatCountType type);

	/**
	 *   @brief  Converts class object data to Json object
	 *
	 *   @param[in]  NONE
     *
	 *   @return cJSON pointer
	 */
	cJSON * ToJson() const;
};



#endif /* __HTTP_STATISTICS_H__ */
