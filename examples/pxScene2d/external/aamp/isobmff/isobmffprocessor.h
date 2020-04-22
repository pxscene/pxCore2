/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2019 RDK Management
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
* @file isobmffprocessor.h
* @brief Header file for ISO Base Media File Format Fragment Processor
*/

#ifndef __ISOBMFFPROCESSOR_H__
#define __ISOBMFFPROCESSOR_H__

#include "isobmffbuffer.h"
#include "mediaprocessor.h"
#include "priv_aamp.h"
#include <pthread.h>

/**
 * @brief ISOBMFF Processor types
 */
enum IsoBmffProcessorType
{
	eBMFFPROCESSOR_TYPE_VIDEO = 0,
	eBMFFPROCESSOR_TYPE_AUDIO = 1
};

/**
 * @brief Class for ISO BMFF Fragment Processor
 */
class IsoBmffProcessor : public MediaProcessor
{

public:
	/**
	 * @brief IsoBmffProcessor constructor
	 *
	 * @param[in] aamp - PrivateInstanceAAMP pointer
	 * @param[in] trackType - track type (A/V)
	 * @param[in] peerBmffProcessor - peer instance of IsoBmffProcessor
	 */
	IsoBmffProcessor(class PrivateInstanceAAMP *aamp, IsoBmffProcessorType trackType = eBMFFPROCESSOR_TYPE_VIDEO, IsoBmffProcessor* peerBmffProcessor = NULL);

	/**
	 * @brief IsoBmffProcessor destructor
	 */
	~IsoBmffProcessor();

	IsoBmffProcessor(const IsoBmffProcessor&) = delete;
	IsoBmffProcessor& operator=(const IsoBmffProcessor&) = delete;

	/**
	 * @brief Enable or disable throttle
	 *
	 * @param[in] enable - throttle enable/disable
	 * @return void
	 */
	void setThrottleEnable(bool enable) override { };

	/**
	 * @brief Set frame rate for trickmode
	 *
	 * @param[in] frameRate - rate per second
	 * @return void
	 */
	void setFrameRateForTM (int frameRate) override { };

	/**
	 * @brief Process and send ISOBMFF fragment
	 *
	 * @param[in] segment - fragment buffer pointer
	 * @param[in] size - fragment buffer size
	 * @param[in] position - position of fragment
	 * @param[in] duration - duration of fragment
	 * @param[in] discontinuous - true if discontinuous fragment
	 * @param[out] ptsError - flag indicates if any PTS error occurred
	 * @return true if fragment was sent, false otherwise
	 */
	bool sendSegment(char *segment, size_t& size, double position, double duration, bool discontinuous, bool &ptsError) override;

	/**
	 * @brief Abort all operations
	 *
	 * @return void
	 */
	void abort() override;

	/**
	 * @brief Reset all variables
	 *
	 * @return void
	 */
	void reset() override;

	/**
	 * @brief Set playback rate
	 *
	 * @param[in] rate - playback rate
	 * @param[in] mode - playback mode
	 * @return void
	 */
	void setRate(double rate, PlayMode mode) override;

private:

	/**
	 * @brief Set peer instance of IsoBmffProcessor
	 *
	 * @param[in] processor - peer instance
	 * @return void
	 */
	void setPeerProcessor(IsoBmffProcessor *processor) { peerProcessor = processor; }

	/**
	 * @brief Set base PTS and TimeScale value
	 *
	 * @param[in] pts - base PTS value
	 * @param[in] tScale - TimeScale value
	 * @return void
	 */
	void setBasePTS(uint64_t pts, uint32_t tScale);

	/**
	 * @brief Cache init fragment internally
	 *
	 * @param[in] segment - buffer pointer
	 * @param[in] size - buffer size
	 * @return void
	 */
	void cacheInitSegment(char *segment, size_t size);

	/**
	 * @brief Push init fragment cached earlier
	 *
	 * @param[in] position - position value
	 * @return void
	 */
	void pushInitSegment(double position);

	/**
	 * @brief Clear init fragment cached earlier
	 *
	 * @return void
	 */
	void clearInitSegment();

	PrivateInstanceAAMP *p_aamp;
	uint32_t timeScale;
	uint64_t basePTS;
	IsoBmffProcessor *peerProcessor;
	IsoBmffProcessorType type;
	bool processPTSComplete;
	bool initSegmentProcessComplete;
	double playRate;

	std::vector<GrowableBuffer *> initSegment;

	bool abortAll;

	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;
};

#endif /* __ISOBMFFPROCESSOR_H__ */

