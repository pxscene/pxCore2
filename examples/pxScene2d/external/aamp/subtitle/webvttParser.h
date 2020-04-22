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
 *  @file  webvttParser.h
 * 
 *  @brief WebVTT parser implementation for AAMP
 *
 */

#ifndef __WEBVTT_PARSER_H__
#define __WEBVTT_PARSER_H__

#include <queue>
#include <pthread.h>
#include "subtitleParser.h"
#include "vttCue.h"


/**
* \struct      CueTimeStamp
* \brief       Hold timestamps of a cue
*
* Structure to hold timestamps of previously parsed cues
* This is required since in playlists a single cue may be
* advertised twice
*/
typedef struct {
	double mStart;
	double mDuration;
} CueTimeStamp;


/**
* \class       WebVTTParser
* \brief       WebVTT parser class
*
* Class for WebVTT parser implementation in AAMP
*/
class WebVTTParser : public SubtitleParser
{

public:
	WebVTTParser(PrivateInstanceAAMP *aamp, SubtitleMimeType type);
	~WebVTTParser();

	bool init(double startPos, unsigned long long basePTS);
	bool processData(char *buffer, size_t bufferLen, double position, double duration);
	bool close();
	void reset();
	void setProgressEventOffset(double offset) { mProgressOffset = offset; }

	void addCueData(VTTCue *cue);
	void sendCueData();

private:
	unsigned long long mStartPTS;   //start/base PTS for current period
	unsigned long long mPtsOffset;  //offset between cue local time and MPEG time
	double mStartPos;               //position of first fragment in playlist
	double mCurrentPos;             //current fragment position in playlist
	bool mReset;                    //true if waiting for first fragment after processing a discontinuity or at start

	CueTimeStamp lastCue;           //holds timestamp of last parsed cue

	std::queue<VTTCue*> mVttQueue;  //queue for storing parsed cues
	guint mVttQueueIdleTaskId;      //task id for handler that sends cues upstream
	pthread_mutex_t mVttQueueMutex; //mutex for synchronising queue access
	double mProgressOffset;         //offset value in progress event compared to playlist position

};

#endif /* __WEBVTT_PARSER_H__ */
