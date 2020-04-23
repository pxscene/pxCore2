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
* @file isobmffbox.h
* @brief Header file for ISO Base Media File Format Boxes
*/

#ifndef __ISOBMFFBOX_H__
#define __ISOBMFFBOX_H__

#include <cstdint>
#include <vector>
#include <string.h>

#define READ_U32(buf) \
	(buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3]; buf+=4;

#define WRITE_U32(buf, val) \
	buf[0]= val>>24; buf[1]= val>>16; buf[2]= val>>8; buf[3]= val;

#define READ_U8(dst, src, sz) \
	memcpy(dst, src, sz); src+=sz;

#define READ_VERSION(buf) \
		buf[0]; buf++;

#define READ_FLAGS(buf) \
		(buf[0] << 16) | (buf[1] << 8) | buf[2]; buf+=3;

uint64_t ReadUint64(uint8_t *buf);

void WriteUint64(uint8_t *dst, uint64_t val);

#define READ_BMDT64(buf) \
		ReadUint64(buf); buf+=8;

#define IS_TYPE(value, type) \
		(value[0]==type[0] && value[1]==type[1] && value[2]==type[2] && value[3]==type[3])


/**
 * @brief Base Class for ISO BMFF Box
 */
class Box
{
private:
	uint32_t offset;	//Offset from the beginning of the segment
	uint32_t size;		//Box Size
	char type[5]; 		//Box Type Including \0

/*TODO: Handle special cases separately */
public:
	static constexpr const char *MOOV = "moov";
	static constexpr const char *MVHD = "mvhd";
	static constexpr const char *TRAK = "trak";
	static constexpr const char *MDIA = "mdia";
	static constexpr const char *MDHD = "mdhd";

	static constexpr const char *MOOF = "moof";
	static constexpr const char *TRAF = "traf";
	static constexpr const char *TFDT = "tfdt";
	static constexpr const char *FTYP = "ftyp";

	/**
	 * @brief Box constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] btype - box type
	 */
	Box(uint32_t sz, const char btype[4]);

	/**
	 * @brief Box destructor
	 */
	virtual ~Box()
	{

	}

	/**
	 * @brief Set box's offset from the beginning of the buffer
	 *
	 * @param[in] os - offset
	 * @return void
	 */
	void setOffset(uint32_t os);

	/**
	 * @brief Get box offset
	 *
	 * @return offset of box
	 */
	uint32_t getOffset();

	/**
	 * @brief To check if box has any child boxes
	 *
	 * @return true if this box has other boxes as children
	 */
	virtual bool hasChildren();

	/**
	 * @brief Get children of this box
	 *
	 * @return array of child boxes
	 */
	virtual const std::vector<Box*> *getChildren();

	/**
	 * @brief Get box size
	 *
	 * @return box size
	 */
	uint32_t getSize();

	/**
	 * @brief Get box type
	 *
	 * @return box type
	 */
	const char *getType();

	/**
	 * @brief Static function to construct a Box object
	 *
	 * @param[in] hdr - pointer to box
	 * @param[in] maxSz - box size
	 * @return newly constructed Box object
	 */
	static Box* constructBox(uint8_t *hdr, uint32_t maxSz);
};


/**
 * @brief Class for ISO BMFF Box container
 * Eg: MOOV, MOOF, TRAK, MDIA
 */
class GenericContainerBox : public Box
{
private:
	std::vector<Box*> children;	// array of child boxes

public:
	/**
	 * @brief GenericContainerBox constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] btype - box type
	 */
	GenericContainerBox(uint32_t sz, const char btype[4]);

	/**
	 * @brief GenericContainerBox destructor
	 */
	virtual ~GenericContainerBox();

	/**
	 * @brief Add a box as a child box
	 *
	 * @param[in] box - child box object
	 * @return void
	 */
	void addChildren(Box *box);

	/**
	 * @brief To check if box has any child boxes
	 *
	 * @return true if this box has other boxes as children
	 */
	bool hasChildren() override;

	/**
	 * @brief Get children of this box
	 *
	 * @return array of child boxes
	 */
	const std::vector<Box*> *getChildren() override;

	/**
	 * @brief Static function to construct a GenericContainerBox object
	 *
	 * @param[in] sz - box size
	 * @param[in] btype - box type
	 * @param[in] ptr - pointer to box
	 * @return newly constructed GenericContainerBox object
	 */
	static GenericContainerBox* constructContainer(uint32_t sz, const char btype[4], uint8_t *ptr);
};


/**
 * @brief Class for single ISO BMFF Box
 * Eg: FTYP, MDHD, MVHD, TFDT
 */
class FullBox : public Box
{
protected:
	uint8_t version;
	uint32_t flags;

public:
	/**
	 * @brief FullBox constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] btype - box type
	 * @param[in] ver - version value
	 * @param[in] f - flag value
	 */
	FullBox(uint32_t sz, const char btype[4], uint8_t ver, uint32_t f);
};


/**
 * @brief Class for ISO BMFF MVHD Box
 */
class MvhdBox : public FullBox
{
private:
	uint32_t timeScale;

public:
	/**
	 * @brief MvhdBox constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] tScale - TimeScale value
	 */
	MvhdBox(uint32_t sz, uint32_t tScale);

	/**
	 * @brief MvhdBox constructor
	 *
	 * @param[in] fbox - box object
	 * @param[in] tScale - TimeScale value
	 */
	MvhdBox(FullBox &fbox, uint32_t tScale);

	/**
	 * @brief Set TimeScale value
	 *
	 * @param[in] tScale - TimeScale value
	 * @return void
	 */
	void setTimeScale(uint32_t tScale);

	/**
	 * @brief Get TimeScale value
	 *
	 * @return TimeScale value
	 */
	uint32_t getTimeScale();

	/**
	 * @brief Static function to construct a MvhdBox object
	 *
	 * @param[in] sz - box size
	 * @param[in] ptr - pointer to box
	 * @return newly constructed MvhdBox object
	 */
	static MvhdBox* constructMvhdBox(uint32_t sz, uint8_t *ptr);
};


/**
 * @brief Class for ISO BMFF MDHD Box
 */
class MdhdBox : public FullBox
{
private:
	uint32_t timeScale;

public:
	/**
	 * @brief MdhdBox constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] tScale - TimeScale value
	 */
	MdhdBox(uint32_t sz, uint32_t tScale);

	/**
	 * @brief MdhdBox constructor
	 *
	 * @param[in] fbox - box object
	 * @param[in] tScale - TimeScale value
	 */
	MdhdBox(FullBox &fbox, uint32_t tScale);

	/**
	 * @brief Set TimeScale value
	 *
	 * @param[in] tScale - TimeScale value
	 * @return void
	 */
	void setTimeScale(uint32_t tScale);

	/**
	 * @brief Get TimeScale value
	 *
	 * @return TimeScale value
	 */
	uint32_t getTimeScale();

	/**
	 * @brief Static function to construct a MdhdBox object
	 *
	 * @param[in] sz - box size
	 * @param[in] ptr - pointer to box
	 * @return newly constructed MdhdBox object
	 */
	static MdhdBox* constructMdhdBox(uint32_t sz, uint8_t *ptr);
};


/**
 * @brief Class for ISO BMFF TFDT Box
 */
class TfdtBox : public FullBox
{
private:
	uint64_t baseMDT;	//BaseMediaDecodeTime value

public:
	/**
	 * @brief TfdtBox constructor
	 *
	 * @param[in] sz - box size
	 * @param[in] mdt - BaseMediaDecodeTime value
	 */
	TfdtBox(uint32_t sz, uint64_t mdt);

	/**
	 * @brief TfdtBox constructor
	 *
	 * @param[in] fbox - box object
	 * @param[in] mdt - BaseMediaDecodeTime value
	 */
	TfdtBox(FullBox &fbox, uint64_t mdt);

	/**
	 * @brief Set BaseMediaDecodeTime value
	 *
	 * @param[in] mdt - BaseMediaDecodeTime value
	 * @return void
	 */
	void setBaseMDT(uint64_t mdt);

	/**
	 * @brief Get BaseMediaDecodeTime value
	 *
	 * @return BaseMediaDecodeTime value
	 */
	uint64_t getBaseMDT();

	/**
	 * @brief Static function to construct a TfdtBox object
	 *
	 * @param[in] sz - box size
	 * @param[in] ptr - pointer to box
	 * @return newly constructed TfdtBox object
	 */
	static TfdtBox* constructTfdtBox(uint32_t sz, uint8_t *ptr);
};

#endif /* __ISOBMFFBOX_H__ */
