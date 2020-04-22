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
 * @file webvttParser.cpp
 *
 * @brief Parser impl for WebVTT subtitle fragments
 *
 */

#include <string.h>
#include <assert.h>
#include <cctype>
#include <algorithm>
#include "webvttParser.h"

//Macros
#define CHAR_CARRIAGE_RETURN    '\r'
#define CHAR_LINE_FEED          '\n'
#define CHAR_SPACE              ' '

#define VTT_QUEUE_TIMER_INTERVAL 250 //milliseconds

/// Variable initialization for supported cue line alignment values
std::vector<std::string> allowedCueLineAligns = { "start", "center", "end" };

/// Variable initialization for supported cue position alignment values
std::vector<std::string> allowedCuePosAligns = { "line-left", "center", "line-right" };

/// Variable initialization for supported cue text alignment values
std::vector<std::string> allowedCueTextAligns = { "start", "center", "end", "left", "right" };


/***************************************************************************
* @fn parsePercentageValueSetting
* @brief Function to parse the value in percentage format
* 
* @param settingValue[in] value to be parsed
* @return char* pointer to the parsed value
***************************************************************************/
static char * parsePercentageValueSetting(char *settingValue)
{
	char *ret = NULL;
	char *percentageSym = strchr(settingValue, '%');
	if ((std::isdigit( static_cast<unsigned char>(settingValue[0]) ) != 0) && (percentageSym != NULL))
	{
		*percentageSym = '\0';
		ret = settingValue;
	}
	return ret;
}


/***************************************************************************
* @fn findWebVTTLineBreak
* @brief Function to extract a line from a VTT fragment
* 
* @param buffer[in] VTT data to extract from
* @return char* pointer to extracted line
***************************************************************************/
static char * findWebVTTLineBreak(char *buffer)
{
	//VTT has CR and LF as line terminators or both
	char *lineBreak = strpbrk(buffer, "\r\n");
	char *next = NULL;
	if (lineBreak)
	{
		next = lineBreak + 1;
		//For CR, LF pair cases
		if (*lineBreak == CHAR_CARRIAGE_RETURN && *next == CHAR_LINE_FEED)
		{
			next += 1;
		}
		*lineBreak = '\0';
	}
	return next;
}


/***************************************************************************
* @fn convertHHMMSSToTime
* @brief Function to convert time in HH:MM:SS.MS format to milliseconds
* 
* @param str[in] time in HH:MM:SS.MS format
* @return long long equivalent time in milliseconds
***************************************************************************/
static long long convertHHMMSSToTime(char *str)
{
	long long timeValueMs = 0;
	//HH:MM:SS.MS
	char *args[4] = { str, NULL, NULL, NULL };
	int argCount = 1;
	while(*(++str) != '\0' && argCount < 4)
	{
		if(*str == ':' || *str == '.')
		{
			args[argCount++] = (str + 1);
			*str = '\0';
		}
	}

	if (argCount == 1)
	{
		logprintf("%s:%d Unsupported value received!", __FUNCTION__, __LINE__);
	}
	//HH:MM:SS.MS
	else
	{
		timeValueMs = atoll(args[--argCount]);
		int multiplier = 1;
		while (argCount > 0)
		{
			timeValueMs += (atoll(args[--argCount]) * multiplier * 1000);
			if (argCount > 0)
			{
				multiplier *= 60;
			}
		}
	}
	return timeValueMs;
}


/***************************************************************************
 * @fn SendVttCueToExt
 * @brief Timer's callback to send WebVTT cues to external app
 *
 * @param[in] user_data pointer to WebVTTParser instance
 * @retval G_SOURCE_REMOVE, if the source should be removed
***************************************************************************/
static gboolean SendVttCueToExt(gpointer user_data)
{
	WebVTTParser *parser = (WebVTTParser *) user_data;
	parser->sendCueData();
	return G_SOURCE_CONTINUE;
}


/***************************************************************************
* @fn WebVTTParser
* @brief Constructor function
* 
* @param aamp[in] PrivateInstanceAAMP pointer
* @param type[in] VTT data type
* @return void
***************************************************************************/
WebVTTParser::WebVTTParser(PrivateInstanceAAMP *aamp, SubtitleMimeType type) : SubtitleParser(aamp, type),
	mStartPTS(0), mCurrentPos(0), mStartPos(0), mPtsOffset(0),
	mReset(true), mVttQueue(), mVttQueueIdleTaskId(0), mVttQueueMutex(), lastCue(),
	mProgressOffset(0)
{
	pthread_mutex_init(&mVttQueueMutex, NULL);
	lastCue = { 0, 0 };
}


/***************************************************************************
* @fn ~WebVTTParser
* @brief Destructor function
* 
* @return void
***************************************************************************/
WebVTTParser::~WebVTTParser()
{
	close();
	pthread_mutex_destroy(&mVttQueueMutex);
}


/***************************************************************************
* @fn init
* @brief Initializes the parser instance
* 
* @param startPos[in] playlist start position in milliseconds
* @param basePTS[in] base PTS value
* @return bool true if successful, false otherwise
***************************************************************************/
bool WebVTTParser::init(double startPos, unsigned long long basePTS)
{
	bool ret = true;
	mStartPTS = basePTS;

	if (mVttQueueIdleTaskId == 0)
	{
		mVttQueueIdleTaskId = g_timeout_add(VTT_QUEUE_TIMER_INTERVAL, SendVttCueToExt, this);
	}

	logprintf("WebVTTParser::%s %d startPos:%.3f and mStartPTS:%lld", __FUNCTION__, __LINE__, startPos, mStartPTS);
	//We are ready to receive data, unblock in PrivateInstanceAAMP
	mAamp->ResumeTrackDownloads(eMEDIATYPE_SUBTITLE);
	return ret;
}


/***************************************************************************
* @fn processData
* @brief Parse incoming VTT data
* 
* @param buffer[in] input VTT data
* @param bufferLen[in] data length
* @param position[in] position of buffer
* @param duration[in] duration of buffer
* @return bool true if successful, false otherwise
***************************************************************************/
bool WebVTTParser::processData(char* buffer, size_t bufferLen, double position, double duration)
{
	bool ret = false;

	traceprintf("WebVTTParser::%s %d Enter with position:%.3f and duration:%.3f ", __FUNCTION__, __LINE__, position, duration);

	if (mReset)
	{
		mStartPos = (position * 1000) - mProgressOffset;
		mReset = false;
		logprintf("WebVTTParser::%s %d Received first buffer after reset with mStartPos:%.3f", __FUNCTION__, __LINE__, mStartPos);
	}

	//Check for VTT signature at the start of buffer
	if (bufferLen > 6)
	{
		char *next = findWebVTTLineBreak(buffer);
		if (next && strlen(buffer) >= 6)
		{
			char *token = strtok(buffer, " \t\n\r");
			//VTT is UTF-8 encoded and BOM is 0xEF,0xBB,0xBF
			if ((unsigned char) token[0] == 0xEF && (unsigned char) token[1] == 0xBB && (unsigned char) token[2] == 0xBF)
			{
				//skip BOM
				token += 3;
			}
			if (strlen(token) == 6 && strncmp(token, "WEBVTT", 6) == 0)
			{
				buffer = next;
				ret = true;
			}
		}
	}

	if (ret)
	{
		while (buffer)
		{
			char *nextLine = findWebVTTLineBreak(buffer);
			//TODO: Parse CUE ID

			if (strstr(buffer, "X-TIMESTAMP-MAP") != NULL && mPtsOffset == 0)
			{
				unsigned long long mpegTime = 0;
				unsigned long long localTime = 0;
				//Found X-TIMESTAMP-MAP=LOCAL:<cue time>,MPEGTS:<MPEG-2 time>
				char* token = strtok(buffer, "=,");
				while (token)
				{
					if (token[0] == 'L')
					{
						localTime = convertHHMMSSToTime(token + 6);
					}
					else if (token[0] == 'M')
					{
						mpegTime = atoll(token + 7);
					}
					token = strtok(NULL, "=,");
				}
				mPtsOffset = (mpegTime / 90) - localTime; //in milliseconds
				logprintf("Parsed local time:%lld and PTS:%lld and cuePTSOffset:%lld", localTime, mpegTime, mPtsOffset);
			}
			else if (strstr(buffer, " --> ") != NULL)
			{
				AAMPLOG_INFO("Found cue:%s", buffer);
				long long start = -1;
				long long end = -1;
				char *text = NULL;
				//cue settings
				char *cueLine = NULL; //default "auto"
				char *cueLineAlign = NULL; //default "start"
				char *cuePosition = NULL; //default "auto"
				char *cuePosAlign = NULL; //default "auto"
				char *cueSize = NULL; //default 100
				char *cueTextAlign = NULL; //default "center"

				char *token = strtok(buffer, " -->\t");
				while (token != NULL)
				{
					traceprintf("Inside tokenizer, token:%s", token);
					if (std::isdigit( static_cast<unsigned char>(token[0]) ) != 0)
					{
						if (start == -1)
						{
							start = convertHHMMSSToTime(token);
						}
						else if (end == -1)
						{
							end = convertHHMMSSToTime(token);
						}
					}
					//TODO:parse cue settings
					else
					{
						//for setting ':' is the delimiter
						char *key = token;
						char *value = strchr(token, ':');
						if (value != NULL)
						{
							*value = '\0';
							value++;
							if (strncmp(key, "line", 4) == 0)
							{
								char *lineAlign = strchr(value, ',');
								if (lineAlign != NULL)
								{
									*lineAlign = '\0';
									lineAlign++;
									if (std::find(allowedCueLineAligns.begin(), allowedCueLineAligns.end(), lineAlign) != allowedCueLineAligns.end())
									{
										cueLineAlign = lineAlign;
									}
								}
								cueLine = parsePercentageValueSetting(value);
							}
							else if (strncmp(key, "position", 8) == 0)
							{
								char *posAlign = strchr(value, ',');
								if (posAlign != NULL)
								{
									*posAlign = '\0';
									posAlign++;
									if (std::find(allowedCuePosAligns.begin(), allowedCuePosAligns.end(), posAlign) != allowedCuePosAligns.end())
									{
										cuePosAlign = posAlign;
									}
								}
								cuePosition = parsePercentageValueSetting(value);
							}
							else if (strncmp(key, "size", 4) == 0)
							{
								cueSize = parsePercentageValueSetting(value);
							}
							else if (strncmp(key, "align", 5) == 0)
							{
								if (std::find(allowedCueTextAligns.begin(), allowedCueTextAligns.end(), value) != allowedCueTextAligns.end())
								{
									cueTextAlign = value;
								}
							}
						}
					}
					token = strtok(NULL, " -->\t");
				}

				text = nextLine;
				nextLine = findWebVTTLineBreak(nextLine);

				while(nextLine && (*nextLine != CHAR_LINE_FEED && *nextLine != CHAR_CARRIAGE_RETURN && *nextLine != '\0'))
				{
					traceprintf("Found nextLine:%s", nextLine);
					if (nextLine[-1] == '\0')
					{
						nextLine[-1] = '\n';
					}
					else if (nextLine[-2] == '\0')
					{
						nextLine[-1] = '\r';
					}
					nextLine = findWebVTTLineBreak(nextLine);
				}
				double cueStartInMpegTime = (start + mPtsOffset);
				double duration = (end - start);
				double mpegTimeOffset = cueStartInMpegTime - (mStartPTS / 90);
				double relativeStartPos = mStartPos + mpegTimeOffset; //w.r.t to position in reportProgress
				AAMPLOG_INFO("So found cue with startPTS:%.3f and duration:%.3f, and mpegTimeOffset:%.3f and relative time being:%.3f\n", cueStartInMpegTime/1000.0, duration/1000.0, mpegTimeOffset/1000.0, relativeStartPos/1000.0);
				addCueData(new VTTCue(relativeStartPos, duration, std::string(text), std::string()));
			}

			buffer = nextLine;
		}
	}
	mCurrentPos = (position + duration) * 1000.0;
	traceprintf("%s:%d ################# Exit sub PTS:%.3f", __FUNCTION__, __LINE__, mCurrentPos);
	return ret;
}


/***************************************************************************
* @fn close
* @brief Close and release all resources
* 
 @return bool true if successful, false otherwise
***************************************************************************/
bool WebVTTParser::close()
{
	bool ret = true;
	if (mVttQueueIdleTaskId != 0)
	{
                logprintf("WebVTTParser::%s %d Remove mVttQueueIdleTaskId %d", __FUNCTION__, __LINE__, mVttQueueIdleTaskId);
                g_source_remove(mVttQueueIdleTaskId);
                mVttQueueIdleTaskId = 0;
	}

	pthread_mutex_lock(&mVttQueueMutex);
	if (!mVttQueue.empty())
	{
		while(mVttQueue.size() > 0)
		{
			VTTCue *cue = mVttQueue.front();
			mVttQueue.pop();
			delete cue;
		}
	}
	pthread_mutex_unlock(&mVttQueueMutex);

	lastCue.mStart = 0;
	lastCue.mDuration = 0;
	mProgressOffset = 0;

	return ret;
}


/***************************************************************************
* @fn reset
* @brief Reset the parser
* 
* @return void
***************************************************************************/
void WebVTTParser::reset()
{
	//Called on discontinuity, blocks further VTT processing
	//Blocked until we get new basePTS
	logprintf("WebVTTParser::%s %d Reset subtitle parser at position:%.3f", __FUNCTION__, __LINE__, mCurrentPos);
	//Avoid calling stop injection if the first buffer is discontinuous
	if (!mReset)
	{
		mAamp->StopTrackDownloads(eMEDIATYPE_SUBTITLE);
	}
	mPtsOffset = 0;
	mStartPTS = 0;
	mStartPos = 0;
	mReset = true;
}


/***************************************************************************
* @fn addCueData
* @brief Add cue to queue
* 
* @param cue[in] pointer to cue to store
* @return void
***************************************************************************/
void WebVTTParser::addCueData(VTTCue *cue)
{
	if (lastCue.mStart != cue->mStart || lastCue.mDuration != cue->mDuration)
	{
		pthread_mutex_lock(&mVttQueueMutex);
		mVttQueue.push(cue);
		pthread_mutex_unlock(&mVttQueueMutex);
	}
	lastCue.mStart = cue->mStart;
	lastCue.mDuration = cue->mDuration;
}


/***************************************************************************
* @fn sendCueData
* @brief Send cues stored in queue to AAMP
* 
* @return void
***************************************************************************/
void WebVTTParser::sendCueData()
{
	pthread_mutex_lock(&mVttQueueMutex);
	if (!mVttQueue.empty())
	{
		while(mVttQueue.size() > 0)
		{
			VTTCue *cue = mVttQueue.front();
			mVttQueue.pop();
			if (cue->mStart > 0)
			{
				mAamp->SendVTTCueDataAsEvent(cue);
			}
			else
			{
				logprintf("Discarding cue with start:%.3f and text:%s", cue->mStart/1000.0, cue->mText.c_str());
			}
			delete cue;
		}
	}
	pthread_mutex_unlock(&mVttQueueMutex);
}

/**
 * @}
 */
