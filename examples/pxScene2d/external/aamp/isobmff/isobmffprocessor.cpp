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
* @file isobmffprocessor.cpp
* @brief Source file for ISO Base Media File Format Segment Processor
*/

#include "isobmffprocessor.h"
#include <pthread.h>
#include <assert.h>

static const char *IsoBmffProcessorTypeName[] =
{
    "video", "audio"
};

/**
 * @brief IsoBmffProcessor constructor
 *
 * @param[in] aamp - PrivateInstanceAAMP pointer
 * @param[in] trackType - track type (A/V)
 * @param[in] peerBmffProcessor - peer instance of IsoBmffProcessor
 */
IsoBmffProcessor::IsoBmffProcessor(class PrivateInstanceAAMP *aamp, IsoBmffProcessorType trackType, IsoBmffProcessor* peerBmffProcessor)
	: p_aamp(aamp), type(trackType), peerProcessor(peerBmffProcessor), basePTS(0),
	processPTSComplete(false), timeScale(0), initSegment(),
	playRate(1.0f), abortAll(false), m_mutex(), m_cond(),
	initSegmentProcessComplete(false)
{
	AAMPLOG_WARN("IsoBmffProcessor::%s() %d Created IsoBmffProcessor(%p) for type:%d and peerProcessor(%p)", __FUNCTION__, __LINE__, this, type, peerBmffProcessor);
	if (peerProcessor)
	{
		peerProcessor->setPeerProcessor(this);
	}
	pthread_mutex_init(&m_mutex, NULL);
	pthread_cond_init(&m_cond, NULL);

	// Sometimes AAMP pushes an encrypted init segment first to force decryptor plugin selection
	initSegment.reserve(2);
}

/**
 * @brief IsoBmffProcessor destructor
 */
IsoBmffProcessor::~IsoBmffProcessor()
{
	clearInitSegment();
}

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
bool IsoBmffProcessor::sendSegment(char *segment, size_t& size, double position, double duration, bool discontinuous, bool &ptsError)
{
	ptsError = false;
	bool ret = true;

	AAMPLOG_TRACE("IsoBmffProcessor::%s() %d [%s] sending segment at pos:%f dur:%f", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], position, duration);
	// Logic for Audio Track
	if (type == eBMFFPROCESSOR_TYPE_AUDIO)
	{
		if (!processPTSComplete)
		{
			IsoBmffBuffer buffer;
			buffer.setBuffer((uint8_t *)segment, size);
			buffer.parseBuffer();

			if (buffer.isInitSegment())
			{
				cacheInitSegment(segment, size);
				ret = false;
			}
			else
			{
				// Wait for video to parse PTS
				pthread_mutex_lock(&m_mutex);
				if (!processPTSComplete)
				{
					AAMPLOG_INFO("IsoBmffProcessor::%s() %d [%s] Going into wait for PTS processing to complete", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
					pthread_cond_wait(&m_cond, &m_mutex);
				}
				if (abortAll)
				{
					ret = false;
				}
				pthread_mutex_unlock(&m_mutex);
			}
		}
		if (ret && !initSegmentProcessComplete)
		{
			if (processPTSComplete)
			{
				double pos = ((double)basePTS / (double)timeScale);
				if (!initSegment.empty())
				{
					pushInitSegment(pos);
				}
				else
				{
					// We have no cached init fragment, maybe audio download was delayed very much
					// Push this fragment with calculated PTS
					p_aamp->SendStream((MediaType)type, segment, size, pos, pos, duration);
					ret = false;
				}
				initSegmentProcessComplete = true;
			}
		}
	}


	// Logic for Video Track
	// For trickplay, restamping is done in qtdemux. We can avoid
	// pts parsing logic
	if (ret && !processPTSComplete && playRate == AAMP_NORMAL_PLAY_RATE)
	{
		// We need to parse PTS from first buffer
		IsoBmffBuffer buffer;
		buffer.setBuffer((uint8_t *)segment, size);
		buffer.parseBuffer();

		if (buffer.isInitSegment())
		{
			uint32_t tScale = 0;
			if (buffer.getTimeScale(tScale))
			{
				timeScale = tScale;
				AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] TimeScale (%ld) set", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], timeScale);
			}

			cacheInitSegment(segment, size);
			ret = false;
		}
		else
		{
			// Init segment was parsed and stored previously. Find the base PTS now
			uint64_t fPts = 0;
			if (buffer.getFirstPTS(fPts))
			{
				basePTS = fPts;
				processPTSComplete = true;
				AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] Base PTS (%lld) set", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], basePTS);
			}
			else
			{
				AAMPLOG_ERR("IsoBmffProcessor::%s() %d [%s] Failed to process pts from buffer at pos:%f and dur:%f", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], position, duration);
			}

			pthread_mutex_lock(&m_mutex);
			if (abortAll)
			{
				ret = false;
			}
			pthread_mutex_unlock(&m_mutex);

			if (ret && processPTSComplete)
			{
				if (timeScale == 0)
				{
					if (initSegment.empty())
					{
						AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] Init segment missing during PTS processing!", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
						p_aamp->SendErrorEvent(AAMP_TUNE_MP4_INIT_FRAGMENT_MISSING);
						ret = false;
					}
					else
					{
						AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] MDHD/MVHD boxes are missing in init segment!", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
						uint32_t tScale = 0;
						if (buffer.getTimeScale(tScale))
						{
							timeScale = tScale;
							AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] TimeScale (%ld) set", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], timeScale);
						}
						if (timeScale == 0)
						{
							AAMPLOG_ERR("IsoBmffProcessor::%s() %d [%s] TimeScale value missing in init segment and mp4 fragment, setting to a default of 1!", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
							timeScale = 1; // to avoid div-by-zero errors later. MDHD and MVHD are mandatory boxes, but lets relax for now
						}

					}
				}

				if (ret)
				{
					double pos = ((double)basePTS / (double)timeScale);
					if (type == eBMFFPROCESSOR_TYPE_VIDEO)
					{
						// Send flushing seek to gstreamer pipeline.
						// For new tune, this will not work, so send pts as fragment position
						p_aamp->FlushStreamSink(pos, playRate);
					}

					if (peerProcessor)
					{
						peerProcessor->setBasePTS(basePTS, timeScale);
					}

					pushInitSegment(pos);
					initSegmentProcessComplete = true;
				}
			}
		}
	}

	if (ret)
	{
		p_aamp->SendStream((MediaType)type, segment, size, position, position, duration);
	}
	return true;
}

/**
 * @brief Abort all operations
 *
 * @return void
 */
void IsoBmffProcessor::abort()
{
	pthread_mutex_lock(&m_mutex);
	abortAll = true;
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Reset all variables
 *
 * @return void
 */
void IsoBmffProcessor::reset()
{
	clearInitSegment();

	pthread_mutex_lock(&m_mutex);
	basePTS = 0;
	timeScale = 0;
	processPTSComplete = false;
	abortAll = false;
	initSegmentProcessComplete = false;
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Set playback rate
 *
 * @param[in] rate - playback rate
 * @param[in] mode - playback mode
 * @return void
 */
void IsoBmffProcessor::setRate(double rate, PlayMode mode)
{
	playRate = rate;
}

/**
 * @brief Set base PTS and TimeScale value
 *
 * @param[in] pts - base PTS value
 * @param[in] tScale - TimeScale value
 * @return void
 */
void IsoBmffProcessor::setBasePTS(uint64_t pts, uint32_t tScale)
{
	AAMPLOG_WARN("%s:%d [%s] Base PTS (%lld) and TimeScale (%ld) set", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type], pts, tScale);
	pthread_mutex_lock(&m_mutex);
	basePTS = pts;
	timeScale = tScale;
	processPTSComplete = true;
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

/**
 * @brief Cache init fragment internally
 *
 * @param[in] segment - buffer pointer
 * @param[in] size - buffer size
 * @return void
 */
void IsoBmffProcessor::cacheInitSegment(char *segment, size_t size)
{
	// Save init segment for later. Init segment will be pushed once basePTS is calculated
	AAMPLOG_INFO("IsoBmffProcessor::%s() %d [%s] Caching init fragment", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
	GrowableBuffer *buffer = new GrowableBuffer();
	memset(buffer, 0x00, sizeof(GrowableBuffer));
	aamp_AppendBytes(buffer, segment, size);
	initSegment.push_back(buffer);
}

/**
 * @brief Push init fragment cached earlier
 *
 * @param[in] position - position value
 * @return void
 */
void IsoBmffProcessor::pushInitSegment(double position)
{
	// Push init segment now, duration = 0
	AAMPLOG_WARN("IsoBmffProcessor::%s() %d [%s] Push init fragment", __FUNCTION__, __LINE__, IsoBmffProcessorTypeName[type]);
	if (initSegment.size() > 0)
	{
		for (auto it = initSegment.begin(); it != initSegment.end();)
		{
			GrowableBuffer *buf = *it;
			p_aamp->SendStream((MediaType)type, buf, position, position, 0);
			aamp_Free(&(buf->ptr));
			delete buf;
			it = initSegment.erase(it);
		}
	}
}

/**
 * @brief Clear init fragment cached earlier
 *
 * @return void
 */
void IsoBmffProcessor::clearInitSegment()
{
	if (initSegment.size() > 0)
	{
		for (auto it = initSegment.begin(); it != initSegment.end();)
		{
			GrowableBuffer *buf = *it;
			aamp_Free(&(buf->ptr));
			delete buf;
			it = initSegment.erase(it);
		}
	}
}
