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
* @file isobmffbuffer.cpp
* @brief Source file for ISO Base Media File Format Buffer
*/

#include "isobmffbuffer.h"
#include "priv_aamp.h" //Required for AAMPLOG_WARN

/**
 * @brief IsoBmffBuffer destructor
 */
IsoBmffBuffer::~IsoBmffBuffer()
{
	for (size_t i = 0; i < boxes.size(); i++)
	{
		delete boxes[i];
	}
	boxes.clear();
}

/**
 * @brief Set buffer
 *
 * @param[in] buf - buffer pointer
 * @param[in] sz - buffer size
 * @return void
 */
void IsoBmffBuffer::setBuffer(uint8_t *buf, size_t sz)
{
	buffer = buf;
	bufSize = sz;
}

/**
 * @brief Parse ISOBMFF boxes from buffer
 *
 * @return true if parse was successful. false otherwise
 */
bool IsoBmffBuffer::parseBuffer()
{
	size_t curOffset = 0;
	while (curOffset < bufSize)
	{
		Box *box = Box::constructBox(buffer+curOffset, bufSize - curOffset);
		box->setOffset(curOffset);
		boxes.push_back(box);
		curOffset += box->getSize();
	}
	return !!(boxes.size());
}


/**
 * @brief Restamp PTS in a buffer
 *
 * @param[in] offset - pts offset
 * @param[in] basePts - base PTS
 * @param[in] segment - buffer pointer
 * @param[in] bufSz - buffer size
 * @return void
 */
void IsoBmffBuffer::restampPTS(uint64_t offset, uint64_t basePts, uint8_t *segment, uint32_t bufSz)
{
	// TODO: Untest code, not required for now
	uint32_t curOffset = 0;
	while (curOffset < bufSz)
	{
		uint8_t *buf = segment + curOffset;
		uint32_t size = READ_U32(buf);
		uint8_t type[5];
		READ_U8(type, buf, 4);
		type[4] = '\0';

		if (IS_TYPE(type, Box::MOOF) || IS_TYPE(type, Box::TRAF))
		{
			restampPTS(offset, basePts, buf, size);
		}
		else if (IS_TYPE(type, Box::TFDT))
		{
			uint8_t version = READ_VERSION(buf);
			uint32_t flags  = READ_FLAGS(buf);

			if (1 == version)
			{
				uint64_t pts = ReadUint64(buf);
				pts -= basePts;
				pts += offset;
				WriteUint64(buf, pts);
			}
			else
			{
				uint32_t pts = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
				pts -= (uint32_t)basePts;
				pts += (uint32_t)offset;
				WRITE_U32(buf, pts);
			}
		}
		curOffset += size;
	}
}

/**
 * @brief Release ISOBMFF boxes parsed
 *
 * @return void
 */
void IsoBmffBuffer::destroyBoxes()
{
	for (size_t i = 0; i < boxes.size(); i++)
	{
		delete boxes[i];
	}
	boxes.clear();
}

/**
 * @brief Get first PTS of buffer
 *
 * @param[in] boxes - ISOBMFF boxes
 * @param[out] pts - pts value
 * @return true if parse was successful. false otherwise
 */
bool IsoBmffBuffer::getFirstPTSInternal(const std::vector<Box*> *boxes, uint64_t &pts)
{
	bool ret = false;
	for (size_t i = 0; (false == ret) && i < boxes->size(); i++)
	{
		Box *box = boxes->at(i);
		if (IS_TYPE(box->getType(), Box::TFDT))
		{
			pts = dynamic_cast<TfdtBox *>(box)->getBaseMDT();
			ret = true;
			break;
		}
		if (box->hasChildren())
		{
			ret = getFirstPTSInternal(box->getChildren(), pts);
		}
	}
	return ret;
}

/**
 * @brief Get first PTS of buffer
 *
 * @param[out] pts - pts value
 * @return true if parse was successful. false otherwise
 */
bool IsoBmffBuffer::getFirstPTS(uint64_t &pts)
{
	return getFirstPTSInternal(&boxes, pts);
}

/**
 * @brief Get TimeScale value of buffer
 *
 * @param[in] boxes - ISOBMFF boxes
 * @param[out] timeScale - TimeScale value
 * @param[out] foundMdhd - flag indicates if MDHD box was seen
 * @return true if parse was successful. false otherwise
 */
bool IsoBmffBuffer::getTimeScaleInternal(const std::vector<Box*> *boxes, uint32_t &timeScale, bool &foundMdhd)
{
	bool ret = false;
	for (size_t i = 0; (false == foundMdhd) && i < boxes->size(); i++)
	{
		Box *box = boxes->at(i);
		if (IS_TYPE(box->getType(), Box::MVHD))
		{
			timeScale = dynamic_cast<MvhdBox *>(box)->getTimeScale();
			ret = true;
		}
		else if (IS_TYPE(box->getType(), Box::MDHD))
		{
			timeScale = dynamic_cast<MdhdBox *>(box)->getTimeScale();
			ret = true;
			foundMdhd = true;
		}
		if (box->hasChildren())
		{
			ret = getTimeScaleInternal(box->getChildren(), timeScale, foundMdhd);
		}
	}
	return ret;
}

/**
 * @brief Get TimeScale value of buffer
 *
 * @param[out] timeScale - TimeScale value
 * @return true if parse was successful. false otherwise
 */
bool IsoBmffBuffer::getTimeScale(uint32_t &timeScale)
{
	bool foundMdhd = false;
	return getTimeScaleInternal(&boxes, timeScale, foundMdhd);
}

/**
 * @brief Print ISOBMFF boxes
 *
 * @param[in] boxes - ISOBMFF boxes
 * @return void
 */
void IsoBmffBuffer::printBoxesInternal(const std::vector<Box*> *boxes)
{
	for (size_t i = 0; i < boxes->size(); i++)
	{
		Box *box = boxes->at(i);
		AAMPLOG_WARN("Offset[%u] Type[%s] Size[%u]\n", box->getOffset(), box->getType(), box->getSize());
		if (IS_TYPE(box->getType(), Box::TFDT))
		{
			AAMPLOG_WARN("****Base Media Decode Time: %lld \n", dynamic_cast<TfdtBox *>(box)->getBaseMDT());
		}
		else if (IS_TYPE(box->getType(), Box::MVHD))
		{
			AAMPLOG_WARN("**** TimeScale from MVHD: %u \n", dynamic_cast<MvhdBox *>(box)->getTimeScale());
		}
		else if (IS_TYPE(box->getType(), Box::MDHD))
		{
			AAMPLOG_WARN("**** TimeScale from MDHD: %u \n", dynamic_cast<MdhdBox *>(box)->getTimeScale());
		}

		if (box->hasChildren())
		{
			printBoxesInternal(box->getChildren());
		}
	}
}

/**
 * @brief Print ISOBMFF boxes
 *
 * @return void
 */
void IsoBmffBuffer::printBoxes()
{
	printBoxesInternal(&boxes);
}
 
/**
 * @brief Check if buffer is an initialization segment
 *
 * @return true if buffer is an initialization segment. false otherwise
 */
bool IsoBmffBuffer::isInitSegment()
{
	bool foundFtypBox = false;
	for (size_t i = 0; i < boxes.size(); i++)
	{
		Box *box = boxes.at(i);
		if (IS_TYPE(box->getType(), Box::FTYP))
		{
			foundFtypBox = true;
			break;
		}
	}
	return foundFtypBox;
}

