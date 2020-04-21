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
* @file isobmffbuffer.h
* @brief Header file for ISO Base Media File Format Buffer
*/

#ifndef __ISOBMFFBUFFER_H__
#define __ISOBMFFBUFFER_H__

#include "isobmffbox.h"
#include <stddef.h>
#include <vector>
#include <cstdint>

/**
 * @brief Class for ISO BMFF Buffer
 */
class IsoBmffBuffer
{
private:
	std::vector<Box*> boxes;	//ISOBMFF boxes of associated buffer
	uint8_t *buffer;
	size_t bufSize;

	/**
	 * @brief Get first PTS of buffer
	 *
	 * @param[in] boxes - ISOBMFF boxes
	 * @param[out] pts - pts value
	 * @return true if parse was successful. false otherwise
	 */
	bool getFirstPTSInternal(const std::vector<Box*> *boxes, uint64_t &pts);

	/**
	 * @brief Get TimeScale value of buffer
	 *
	 * @param[in] boxes - ISOBMFF boxes
	 * @param[out] timeScale - TimeScale value
	 * @param[out] foundMdhd - flag indicates if MDHD box was seen
	 * @return true if parse was successful. false otherwise
	 */
	bool getTimeScaleInternal(const std::vector<Box*> *boxes, uint32_t &timeScale, bool &foundMdhd);

	/**
	 * @brief Print ISOBMFF boxes
	 *
	 * @param[in] boxes - ISOBMFF boxes
	 * @return void
	 */
	void printBoxesInternal(const std::vector<Box*> *boxes);

public:
	/**
	 * @brief IsoBmffBuffer constructor
	 */
	IsoBmffBuffer(): boxes(), buffer(NULL), bufSize(0)
	{

	}

	/**
	 * @brief IsoBmffBuffer destructor
	 */
	~IsoBmffBuffer();

	IsoBmffBuffer(const IsoBmffBuffer&) = delete;
	IsoBmffBuffer& operator=(const IsoBmffBuffer&) = delete;

	/**
	 * @brief Set buffer
	 *
	 * @param[in] buf - buffer pointer
	 * @param[in] sz - buffer size
	 * @return void
	 */
	void setBuffer(uint8_t *buf, size_t sz);

	/**
	 * @brief Parse ISOBMFF boxes from buffer
	 *
	 * @return true if parse was successful. false otherwise
	 */
	bool parseBuffer();

	/**
	 * @brief Restamp PTS in a buffer
	 *
	 * @param[in] offset - pts offset
	 * @param[in] basePts - base pts
	 * @param[in] segment - buffer pointer
	 * @param[in] bufSz - buffer size
	 * @return void
	 */
	void restampPTS(uint64_t offset, uint64_t basePts, uint8_t *segment, uint32_t bufSz);

	/**
	 * @brief Get first PTS of buffer
	 *
	 * @param[out] pts - pts value
	 * @return true if parse was successful. false otherwise
	 */
	bool getFirstPTS(uint64_t &pts);

	/**
	 * @brief Get TimeScale value of buffer
	 *
	 * @param[out] timeScale - TimeScale value
	 * @return true if parse was successful. false otherwise
	 */
	bool getTimeScale(uint32_t &timeScale);

	/**
	 * @brief Release ISOBMFF boxes parsed
	 *
	 * @return void
	 */
	void destroyBoxes();

	/**
	 * @brief Print ISOBMFF boxes
	 *
	 * @return void
	 */
	void printBoxes();

	/**
	 * @brief Check if buffer is an initialization segment
	 *
	 * @return true if buffer is an initialization segment. false otherwise
	 */
	bool isInitSegment();
};


#endif /* __ISOBMFFBUFFER_H__ */
