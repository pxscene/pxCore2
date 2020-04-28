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
 * @file vttCue.h
 * 
 * @brief Provides data structure to hold a WebVTT cue data
 *
 */

#ifndef __VTT_CUE_H__
#define __VTT_CUE_H__

#include <string>


/**
* \struct      VTT cue data structure
* \brief       Data structure to hold a VTT cue
*
* This is the data structure to store parsed WebVTT cues in AAMP
*/
struct VTTCue
{
	VTTCue(double startTime, double duration, std::string text, std::string settings):
		mStart(startTime), mDuration(duration),
		mText(text), mSettings(settings)
	{

	}

	double mStart;
	double mDuration;
	std::string mText;
	std::string mSettings;
};

#endif /* __VTT_CUE_H__ */
