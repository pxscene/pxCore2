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
* @file isobmffbox.cpp
* @brief Source file for ISO Base Media File Format Boxes
*/

#include "isobmffbox.h"
#include "priv_aamp.h" //Required for AAMPLOG_WARN
#include <stddef.h>

/**
 * @brief Utility function to read 8 bytes from a buffer
 *
 * @param[in] buf - buffer pointer
 * @return bytes read from buffer
 */
uint64_t ReadUint64(uint8_t *buf)
{
	uint64_t val = READ_U32(buf);
	val = (val<<32) | (uint32_t)READ_U32(buf);
	return val;
}

/**
 * @brief Utility function to write 8 bytes to a buffer
 *
 * @param[in] dst - buffer pointer
 * @param[in] value - value to write
 * @return void
 */
void WriteUint64(uint8_t *dst, uint64_t val)
{
	uint32_t msw = (uint32_t)(val>>32);
	WRITE_U32(dst, msw); dst+=4;
	WRITE_U32(dst, val);
}

/**
 * @brief Box constructor
 *
 * @param[in] sz - box size
 * @param[in] btype - box type
 */
Box::Box(uint32_t sz, const char btype[4]) : offset(0), size(sz), type{}
{
	memcpy(type,btype,4);
}

/**
 * @brief Set box's offset from the beginning of the buffer
 *
 * @param[in] os - offset
 * @return void
 */
void Box::setOffset(uint32_t os)
{
	offset = os;
}

/**
 * @brief Get box offset
 *
 * @return offset of box
 */
uint32_t Box::getOffset()
{
	return offset;
}

/**
 * @brief To check if box has any child boxes
 *
 * @return true if this box has other boxes as children
 */
bool Box::hasChildren()
{
	return false;
}

/**
 * @brief Get children of this box
 *
 * @return array of child boxes
 */
const std::vector<Box*> *Box::getChildren()
{
	return NULL;
}

/**
 * @brief Get box size
 *
 * @return box size
 */
uint32_t Box::getSize()
{
	return size;
}

/**
 * @brief Get box type
 *
 * @return box type
 */
const char *Box::getType()
{
	return type;
}

/**
 * @brief Static function to construct a Box object
 *
 * @param[in] hdr - pointer to box
 * @param[in] maxSz - box size
 * @return newly constructed Box object
 */
Box* Box::constructBox(uint8_t *hdr, uint32_t maxSz)
{
	uint32_t size = READ_U32(hdr);
	uint8_t type[5];
	READ_U8(type, hdr, 4);
	type[4] = '\0';

	if (size > maxSz)
	{
		AAMPLOG_WARN("Box[%s] Size error:size[%u] > maxSz[%u]\n",type, size, maxSz);
	}
	else if (IS_TYPE(type, MOOV))
	{
		return GenericContainerBox::constructContainer(size, MOOV, hdr);
	}
	else if (IS_TYPE(type, TRAK))
	{
		return GenericContainerBox::constructContainer(size, TRAK, hdr);
	}
	else if (IS_TYPE(type, MDIA))
	{
		return GenericContainerBox::constructContainer(size, MDIA, hdr);
	}
	else if (IS_TYPE(type, MOOF))
	{
		return GenericContainerBox::constructContainer(size, MOOF, hdr);
	}
	else if (IS_TYPE(type, TRAF))
	{
		return GenericContainerBox::constructContainer(size, TRAF, hdr);
	}
	else if (IS_TYPE(type, TFDT))
	{
		return TfdtBox::constructTfdtBox(size,  hdr);
	}
	else if (IS_TYPE(type, MVHD))
	{
		return MvhdBox::constructMvhdBox(size,  hdr);
	}
	else if (IS_TYPE(type, MDHD))
	{
		return MdhdBox::constructMdhdBox(size,  hdr);
	}

	return new Box(size, (const char *)type);
}

/**
 * @brief GenericContainerBox constructor
 *
 * @param[in] sz - box size
 * @param[in] btype - box type
 */
GenericContainerBox::GenericContainerBox(uint32_t sz, const char btype[4]) : Box(sz, btype), children()
{

}

/**
 * @brief GenericContainerBox destructor
 */
GenericContainerBox::~GenericContainerBox()
{
	for (size_t i = 0; i < children.size(); i++)
	{
		delete children.at(i);
	}
	children.clear();
}

/**
 * @brief Add a box as a child box
 *
 * @param[in] box - child box object
 * @return void
 */
void GenericContainerBox::addChildren(Box *box)
{
	children.push_back(box);
}

/**
 * @brief To check if box has any child boxes
 *
 * @return true if this box has other boxes as children
 */
bool GenericContainerBox::hasChildren()
{
	return true;
}

/**
 * @brief Get children of this box
 *
 * @return array of child boxes
 */
const std::vector<Box*> *GenericContainerBox::getChildren()
{
	return &children;
}

/**
 * @brief Static function to construct a GenericContainerBox object
 *
 * @param[in] sz - box size
 * @param[in] btype - box type
 * @param[in] ptr - pointer to box
 * @return newly constructed GenericContainerBox object
 */
GenericContainerBox* GenericContainerBox::constructContainer(uint32_t sz, const char btype[4], uint8_t *ptr)
{
	GenericContainerBox *cbox = new GenericContainerBox(sz, btype);
	uint32_t curOffset = sizeof(uint32_t) + sizeof(uint32_t); //Sizes of size & type fields
	while (curOffset < sz)
	{
		Box *box = Box::constructBox(ptr, sz-curOffset);
		box->setOffset(curOffset);
		cbox->addChildren(box);
		curOffset += box->getSize();
		ptr += box->getSize();
	}
	return cbox;
}

/**
 * @brief FullBox constructor
 *
 * @param[in] sz - box size
 * @param[in] btype - box type
 * @param[in] ver - version value
 * @param[in] f - flag value
 */
FullBox::FullBox(uint32_t sz, const char btype[4], uint8_t ver, uint32_t f) : Box(sz, btype), version(ver), flags(f)
{

}

/**
 * @brief MvhdBox constructor
 *
 * @param[in] sz - box size
 * @param[in] tScale - TimeScale value
 */
MvhdBox::MvhdBox(uint32_t sz, uint32_t tScale) : FullBox(sz, Box::MVHD, 0, 0), timeScale(tScale)
{

}

/**
 * @brief MvhdBox constructor
 *
 * @param[in] fbox - box object
 * @param[in] tScale - TimeScale value
 */
MvhdBox::MvhdBox(FullBox &fbox, uint32_t tScale) : FullBox(fbox), timeScale(tScale)
{

}

/**
 * @brief Set TimeScale value
 *
 * @param[in] tScale - TimeScale value
 * @return void
 */
void MvhdBox::setTimeScale(uint32_t tScale)
{
	timeScale = tScale;
}

/**
 * @brief Get TimeScale value
 *
 * @return TimeScale value
 */
uint32_t MvhdBox::getTimeScale()
{
	return timeScale;
}

/**
 * @brief Static function to construct a MvhdBox object
 *
 * @param[in] sz - box size
 * @param[in] ptr - pointer to box
 * @return newly constructed MvhdBox object
 */
MvhdBox* MvhdBox::constructMvhdBox(uint32_t sz, uint8_t *ptr)
{
	uint8_t version = READ_VERSION(ptr);
	uint32_t flags  = READ_FLAGS(ptr);
	uint64_t tScale;

	uint32_t skip = sizeof(uint32_t)*2;
	if (1 == version)
	{
		//Skipping creation_time &modification_time
		skip = sizeof(uint64_t)*2;
	}
	ptr += skip;

	tScale = READ_U32(ptr);

	FullBox fbox(sz, Box::MVHD, version, flags);
	return new MvhdBox(fbox, tScale);
}

/**
 * @brief MdhdBox constructor
 *
 * @param[in] sz - box size
 * @param[in] tScale - TimeScale value
 */
MdhdBox::MdhdBox(uint32_t sz, uint32_t tScale) : FullBox(sz, Box::MDHD, 0, 0), timeScale(tScale)
{

}

/**
 * @brief MdhdBox constructor
 *
 * @param[in] fbox - box object
 * @param[in] tScale - TimeScale value
 */
MdhdBox::MdhdBox(FullBox &fbox, uint32_t tScale) : FullBox(fbox), timeScale(tScale)
{

}

/**
 * @brief Set TimeScale value
 *
 * @param[in] tScale - TimeScale value
 * @return void
 */
void MdhdBox::setTimeScale(uint32_t tScale)
{
	timeScale = tScale;
}

/**
 * @brief Get TimeScale value
 *
 * @return TimeScale value
 */
uint32_t MdhdBox::getTimeScale()
{
	return timeScale;
}

/**
 * @brief Static function to construct a MdhdBox object
 *
 * @param[in] sz - box size
 * @param[in] ptr - pointer to box
 * @return newly constructed MdhdBox object
 */
MdhdBox* MdhdBox::constructMdhdBox(uint32_t sz, uint8_t *ptr)
{
	uint8_t version = READ_VERSION(ptr);
	uint32_t flags  = READ_FLAGS(ptr);
	uint64_t tScale;

	uint32_t skip = sizeof(uint32_t)*2;
	if (1 == version)
	{
		//Skipping creation_time &modification_time
		skip = sizeof(uint64_t)*2;
	}
	ptr += skip;

	tScale = READ_U32(ptr);

	FullBox fbox(sz, Box::MDHD, version, flags);
	return new MdhdBox(fbox, tScale);
}

/**
 * @brief TfdtBox constructor
 *
 * @param[in] sz - box size
 * @param[in] mdt - BaseMediaDecodeTime value
 */
TfdtBox::TfdtBox(uint32_t sz, uint64_t mdt) : FullBox(sz, Box::TFDT, 0, 0), baseMDT(mdt)
{

}

/**
 * @brief TfdtBox constructor
 *
 * @param[in] fbox - box object
 * @param[in] mdt - BaseMediaDecodeTime value
 */
TfdtBox::TfdtBox(FullBox &fbox, uint64_t mdt) : FullBox(fbox), baseMDT(mdt)
{

}

/**
 * @brief Set BaseMediaDecodeTime value
 *
 * @param[in] mdt - BaseMediaDecodeTime value
 * @return void
 */
void TfdtBox::setBaseMDT(uint64_t mdt)
{
	baseMDT = mdt;
}

/**
 * @brief Get BaseMediaDecodeTime value
 *
 * @return BaseMediaDecodeTime value
 */
uint64_t TfdtBox::getBaseMDT()
{
	return baseMDT;
}

/**
 * @brief Static function to construct a TfdtBox object
 *
 * @param[in] sz - box size
 * @param[in] ptr - pointer to box
 * @return newly constructed TfdtBox object
 */
TfdtBox* TfdtBox::constructTfdtBox(uint32_t sz, uint8_t *ptr)
{
	uint8_t version = READ_VERSION(ptr);
	uint32_t flags  = READ_FLAGS(ptr);
	uint64_t mdt;

	if (1 == version)
	{
		mdt = READ_BMDT64(ptr);
	}
	else
	{
		mdt = (uint32_t)READ_U32(ptr);
	}
	FullBox fbox(sz, Box::TFDT, version, flags);
	return new TfdtBox(fbox, mdt);
}
