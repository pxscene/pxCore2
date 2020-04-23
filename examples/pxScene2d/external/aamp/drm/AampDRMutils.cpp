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
* @file AampDRMutils.cpp
* @brief DataStructures and methods for DRM license acquisition
*/

#include "AampDRMutils.h"
#include "_base64.h"

#include <cjson/cJSON.h>
#include <uuid/uuid.h>

#define KEYID_TAG_START "<KID>"
#define KEYID_TAG_END "</KID>"

#define KEY_ID_SZE_INDICATOR 0x12

/**
 *  @brief		Default constructor for DrmData.
 *				NULL initialize data and dataLength.
 */
DrmData::DrmData() : data(NULL), dataLength(0)
{
}

/**
 *  @brief      Constructor for DrmData
 *              allocate memory and initialize data and
 *				dataLength with given params.
 *
 *  @param[in]	data - pointer to data to be copied.
 *  @param[in]	dataLength - length of data
 */
DrmData::DrmData(unsigned char *data, int dataLength) : data(NULL), dataLength(dataLength)
{
	this->data =(unsigned char*) malloc(dataLength + 1);
	memcpy(this->data,data,dataLength + 1);
}

/**
 *  @brief		Distructor for DrmData.
 *				Free memory (if any) allocated for data.
 */
DrmData::~DrmData()
{
	if(data != NULL)
	{
		free(data);
		data = NULL;
	}
}

/**
 *  @brief		Getter method for data.
 *
 *  @return		Returns pointer to data.
 */
unsigned char * DrmData::getData()
{
	return data;
}


/**
 *  @brief      Getter method for dataLength.
 *
 *  @return     Returns dataLength.
 */
int DrmData::getDataLength()
{
	return dataLength;
}

/**
 *  @brief		Updates DrmData with given data.
 *				Frees the existing data, before copying new data.
 *
 *  @param[in]	data - Pointer to data to be set.
 *  @param[in]	dataLength - length of data.
 *  @return		void.
 */
void DrmData::setData(unsigned char * data, int dataLength)
{
	if(this->data != NULL)
	{
		free(data);
	}
	this->data =  (unsigned char*) malloc(dataLength + 1);
	this->dataLength = dataLength;
	memcpy(this->data,data,dataLength + 1);
}

/**
 *  @brief      Appends DrmData with given data.
 *
 *  @param[in]  data - Pointer to data to be appended.
 *  @param[in]  dataLength - length of data.
 *  @return     void.
 */
void DrmData::addData(unsigned char * data, int dataLength)
{
	if(NULL == this->data)
	{
		this->setData(data,dataLength);
	}
	else
	{
		this->data = (unsigned char*) realloc(this->data, this->dataLength + dataLength + 1);
		assert(this->data);
		memcpy(&(this->data[this->dataLength]),data,dataLength + 1);
		this->dataLength = this->dataLength + dataLength;
	}
}

/**
 * @brief convert blob of binary data to ascii base64-URL-encoded equivalent
 * @param src pointer to first byte of binary data to be encoded
 * @param len number of bytes to encode
 * @retval pointer to malloc'd cstring containing base64 URL encoded version of string
 * @retval NULL if insufficient memory to allocate base64-URL-encoded copy
 * @note caller responsible for freeing returned cstring
 */
char *aamp_Base64_URL_Encode(const unsigned char *src, size_t len)
{
	char * b64Src = base64_Encode(src, len);
	char* urlEncodedSrc = (char*)malloc(sizeof(char) * strlen(b64Src));
	for (int iter = 0; iter < strlen(b64Src); iter++)
	{
		if (b64Src[iter] == '+')
		{
			urlEncodedSrc[iter] = '-';
		}
		else if (b64Src[iter] == '/')
		{
			urlEncodedSrc[iter] = '_';
		}
		else if (b64Src[iter] == '=')
		{
			urlEncodedSrc[iter] = '\0';
			break;
		}
		else
		{
			urlEncodedSrc[iter] = b64Src[iter];
		}
	}
	free(b64Src);
	return urlEncodedSrc;
}


/**
 * @brief decode base64 URL encoded data to binary equivalent
 * @param src pointer to cstring containing base64-URL-encoded data
 * @param len receives byte length of returned pointer, or zero upon failure
 * @retval pointer to malloc'd memory containing decoded binary data
 * @retval NULL if insufficient memory to allocate decoded data
 * @note caller responsible for freeing returned data
 */

unsigned char *aamp_Base64_URL_Decode(const char *src, size_t *len, size_t srcLen)
{
	//Calculate the size for corresponding Base64 Encoded string with padding
	int b64Len = (((srcLen / 4) + 1) * 4) + 1;
	char *b64Src = (char *) malloc(sizeof(char)* b64Len);
	b64Src[b64Len - 1] = '\0';
	b64Src[b64Len - 2] = '=';
	b64Src[b64Len - 3] = '=';
	for (int iter = 0; iter < strlen(src); iter++) {
		if (src[iter] == '-') {
			b64Src[iter] = '+';
		} else if (src[iter] == '_') {
			b64Src[iter] = '/';
		} else {
			b64Src[iter] = src[iter];
		}
	}
	*len = 0;
	unsigned char * decodedStr = base64_Decode(b64Src, len);
	free(b64Src);
	return decodedStr;
}


/**
 *  @brief		Find the position of substring in cleaned PSSH data.
 *  			Cleaned PSSH data, is PSSH data from which empty bytes are removed.
 *  			Used while extracting keyId or content meta data from PSSH.
 *
 *  @param[in]	psshData - Pointer to cleaned PSSH data.
 *  @param[in]	dataLength - Length of cleaned PSSH data.
 *  @param[in]	pos - Search start position.
 *  @param[in]	subStr - Pointer to substring to be searched.
 *  @param[out]	substrStartPos - Default NULL; If not NULL, gets updated with end
 *  			position of the substring in Cleaned PSSH; -1 if not found.
 *  @return		Start position of substring in cleaned PSSH; -1 if not found.
 */
static int aamp_FindSubstr(const char* psshData, int dataLength, int pos, const char* substr, int *substrStartPos = NULL)
{
	int subStrLen = strlen(substr);
	int psshIter = pos;
	int subStrIter = 0;
	bool strMatched = false;
	while (psshIter < dataLength - (subStrLen - 1))
	{
		if (psshData[psshIter] == substr[subStrIter])
		{
			if(substrStartPos && subStrIter == 0)
			{
				*substrStartPos = psshIter;
			}
			subStrIter++;
			if (subStrIter == subStrLen)
			{
				strMatched = true;
				break;
			}
		}
		else
		{
			subStrIter = 0;
		}
		psshIter++;
	}

	if(strMatched)
	{
		return psshIter;
	}
	else
	{
		if(substrStartPos)
		{
			*substrStartPos = -1;
		}
		return -1;
	}
}

/**
 *  @brief		Swap the bytes at given positions.
 *
 *  @param[in, out]	bytes - Pointer to byte block where swapping is done.
 *  @param[in]	pos1, pos2 - Swap positions.
 *  @return		void.
 */
static void aamp_SwapBytes(unsigned char *bytes, int pos1, int pos2)
{
	unsigned char temp = bytes[pos1];
	bytes[pos1] = bytes[pos2];
	bytes[pos2] = temp;
}

/**
 *  @brief		Convert endianness of 16 byte block.
 *
 *  @param[in]	original - Pointer to source byte block.
 *  @param[out]	guidBytes - Pointer to destination byte block.
 *  @return		void.
 */
static void aamp_ConvertEndianness(unsigned char *original, unsigned char *guidBytes)
{
	memcpy(guidBytes, original, 16);
	aamp_SwapBytes(guidBytes, 0, 3);
	aamp_SwapBytes(guidBytes, 1, 2);
	aamp_SwapBytes(guidBytes, 4, 5);
	aamp_SwapBytes(guidBytes, 6, 7);
}

/**
 *  @brief		Extract the keyId from PSSH data.
 *  			Different procedures are used for PlayReady and WideVine.
 *
 *  @param[in]	psshData - Pointer to PSSH data.
 *  @param[in]	dataLength - Length of PSSH data.
 *  @param[out]	len - Gets updated with length of keyId.
 *  @param[in]	isWidevine - Flag to indicate WV.
 *  @return		Pointer to extracted keyId.
 *  @note		Memory for keyId is dynamically allocated, deallocation
 *				should be handled at the caller side.
 */
unsigned char * aamp_ExtractKeyIdFromPssh(const char* psshData, int dataLength, int *len, DRMSystems drmSystem)
{
	unsigned char* key_id = NULL;
	if(drmSystem == eDRM_WideVine)
	{
                uint8_t psshDataVer = psshData[8];
                AAMPLOG_INFO("%s:%d wv pssh data version - %d ",
                         __FUNCTION__, __LINE__, psshDataVer);
		if (psshDataVer == 0){
			//The following 2 are for Widevine
			//PSSH version 0
			//4+4+4+16(system id)+4(data size)+2(keyId size indicator + keyid size)+ keyId +
			//2 (unknown byte + content id size) + content id
			uint32_t header = 0;
			if (psshData[32] == KEY_ID_SZE_INDICATOR){
				header = 33; //pssh data in comcast format
			}else if(psshData[34] == KEY_ID_SZE_INDICATOR){
				header = 35; //pssh data in sling format
			}else{
				AAMPLOG_WARN("%s:%d wv version %d keyid indicator"
				" byte not found using default logic",
				__FUNCTION__, __LINE__);
				header = 33; //pssh data in comcast format
			}
			uint8_t  key_id_size = (uint8_t)psshData[header];
			key_id = (unsigned char*)malloc(key_id_size + 1);
			memset(key_id, 0, key_id_size + 1);
			memcpy(reinterpret_cast<char*>(key_id), psshData + header + 1, key_id_size);
			*len = (int)key_id_size;
			AAMPLOG_INFO("%s:%d wv version %d keyid: %s keyIdlen: %d",
			__FUNCTION__, __LINE__, psshDataVer, key_id, key_id_size);
			if(gpGlobalConfig->logging.trace)
			{
				DumpBlob(key_id, key_id_size);
			}
		}else if (psshDataVer == 1){
			//PSSH version 1
			//8 byte BMFF box header + 4 byte Full box header + 16 (system id) +
			// 4(KID Count) + 16 byte KID 1 + .. + 4 byte Data Size
			/** TODO : Handle multiple key Id logic ,
			 * right now we are choosing only first one if have multiple key Id **/
			uint32_t header = 32;
			uint8_t  key_id_size = 16;
			key_id = (unsigned char*)malloc(key_id_size + 1 );
			memset(key_id, 0, key_id_size + 1);
		        memcpy(reinterpret_cast<char*>(key_id), psshData + header, key_id_size);
			*len = (int)key_id_size;
			AAMPLOG_INFO("%s:%d wv version %d keyid: %s keyIdlen: %d",
			__FUNCTION__, __LINE__, psshDataVer, key_id, key_id_size);
			if(gpGlobalConfig->logging.trace)
			{
				DumpBlob(key_id, key_id_size);
			}
		}

	}
	else if (drmSystem == eDRM_PlayReady) {

		int keyIdLen = 0;
		unsigned char *keydata = aamp_ExtractDataFromPssh(psshData, dataLength, KEYID_TAG_START, KEYID_TAG_END, &keyIdLen);

		AAMPLOG_INFO("%s:%d pr keyid: %s keyIdlen: %d",__FUNCTION__, __LINE__, keydata, keyIdLen);

		size_t decodedDataLen = 0;
		unsigned char* decodedKeydata = base64_Decode((const char *) keydata, &decodedDataLen);
		if(decodedDataLen != 16)
		{
			AAMPLOG_ERR("invalid key size found while extracting PR KeyID: %d", decodedDataLen);
			free (keydata);
			free (decodedKeydata);
			return NULL;
		}

		unsigned char *swappedKeydata = (unsigned char*)malloc(16);

		aamp_ConvertEndianness(decodedKeydata, swappedKeydata);

		key_id = (unsigned char *)calloc(37, sizeof(char));
		uuid_t *keyiduuid = (uuid_t *) swappedKeydata;
		uuid_unparse_lower(*keyiduuid, reinterpret_cast<char*>(key_id));

		*len = 37;

		free (keydata);
		free (decodedKeydata);
		free (swappedKeydata);
	}
	else if (drmSystem == eDRM_ClearKey) {
		/*
			00 00 00 34 70 73 73 68 01 00 00 00 10 77 ef ec
			c0 b2 4d 02 ac e3 3c 1e 52 e2 fb 4b 00 00 00 01

			fe ed f0 0d ee de ad be ef f0 ba ad f0 0d d0 0d
			00 00 00 00
		*/
		uint32_t header = 32;
		uint8_t  key_id_count = (uint8_t)psshData[header];
		key_id = (unsigned char*)malloc(16 + 1);
		memset(key_id, 0, 16 + 1);
		strncpy(reinterpret_cast<char*>(key_id), psshData + header, 16);
		*len = (int)16;
		AAMPLOG_INFO("%s:%d ck keyid: %s keyIdlen: %d",__FUNCTION__, __LINE__, key_id, 16);
		if(gpGlobalConfig->logging.trace)
		{
			DumpBlob(key_id, 16);
		}
	}

	return key_id;
}

/**
 *  @brief		Extract WideVine content meta data from Comcast DRM
 *  			Agnostic PSSH header. Might not work with WideVine PSSH header
 *
 *  @param[in]	Pointer to PSSH data.
 *  @param[in]	dataLength - Length of PSSH data.
 *  @param[out]	len - Gets updated with length of content meta data.
 *  @return		Extracted ContentMetaData.
 *  @note		Memory for ContentMetaData is dynamically allocated, deallocation
 *				should be handled at the caller side.
 */
unsigned char * aamp_ExtractWVContentMetadataFromPssh(const char* psshData, int dataLength, int *len)
{

	//WV PSSH format 4+4+4+16(system id)+4(data size)
	uint32_t header = 28;
	unsigned char* content_id = NULL;
	uint32_t  content_id_size =
                    (uint32_t)((psshData[header] & 0x000000FFu) << 24 |
                               (psshData[header+1] & 0x000000FFu) << 16 |
                               (psshData[header+2] & 0x000000FFu) << 8 |
                               (psshData[header+3] & 0x000000FFu));

	AAMPLOG_INFO("%s:%d content meta data length  : %d", __FUNCTION__, __LINE__,content_id_size);

	content_id = (unsigned char*)malloc(content_id_size + 1);
	memset(content_id, 0, content_id_size + 1);
	strncpy(reinterpret_cast<char*>(content_id), psshData + header + 4, content_id_size);
//	logprintf("%s:%d content meta data : %s", __FUNCTION__, __LINE__,content_id);

	*len = (int)content_id_size;
	return content_id;
}
//End of special for Widevine

/**
 *  @brief		Extract content meta data or keyID from given PSSH data.
 *  			For example for content meta data,
 *  			When strings are given as "ckm:policy xmlns:ckm="urn:ccp:ckm"" and "ckm:policy"
 *  			<ckm:policy xmlns:ckm="urn:ccp:ckm">we need the contents from here</ckm:policy>
 *
 *  			PSSH is cleaned of empty bytes before extraction steps, since from manifest the
 *  			PSSH data is in 2 bytes. Data dump looking like below, so direct string comparison
 *  			would strstr fail.

 *				000003c0 (0x14d3c0): 3c 00 63 00 6b 00 6d 00 3a 00 70 00 6f 00 6c 00  <.c.k.m.:.p.o.l.
 *				000003d0 (0x14d3d0): 69 00 63 00 79 00 20 00 78 00 6d 00 6c 00 6e 00  i.c.y. .x.m.l.n.
 *				000003e0 (0x14d3e0): 73 00 3a 00 63 00 6b 00 6d 00 3d 00 22 00 75 00  s.:.c.k.m.=.".u.
 *				000003f0 (0x14d3f0): 72 00 6e 00 3a 00 63 00 63 00 70 00 3a 00 63 00  r.n.:.c.c.p.:.c.
 *				00000400 (0x14d400): 6b 00 6d 00 22 00 3e 00 65 00 79 00 4a 00 34 00  k.m.".>.e.y.J.4.
 *				00000410 (0x14d410): 4e 00 58 00 51 00 6a 00 55 00 7a 00 49 00 31 00  N.X.Q.j.U.z.I.1.
 *				00000420 (0x14d420): 4e 00 69 00 49 00 36 00 49 00 6c 00 64 00 51 00  N.i.I.6.I.l.d.Q.
 *
 *  @param[in]	Pointer to PSSH data.
 *  @param[in]	dataLength - Length of PSSH data.
 *  @param[in]	startStr, endStr - Pointer to delimiter strings.
 *  @param[out]	len - Gets updated with length of content meta data.
 *  @return		Extracted data between delimiters; NULL if not found.
 *  @note		Memory of returned data is dynamically allocated, deallocation
 *				should be handled at the caller side.
 */
unsigned char * aamp_ExtractDataFromPssh(const char* psshData, int dataLength,
										const char* startStr, const char* endStr, int *len) {
	int endPos = -1;
	int startPos = -1;
	unsigned char* contentMetaData = NULL;

	//Clear the 00  bytes
	char* cleanedPssh = (char*) malloc(dataLength);
	int cleanedPsshLen = 0;
	for(int itr = 0; itr < dataLength; itr++)
	{
		if(psshData[itr] != 0)
		{
			//cout<<psshData[itr];
			cleanedPssh[cleanedPsshLen++] = psshData[itr];
		}
	}

	startPos = aamp_FindSubstr(cleanedPssh, cleanedPsshLen, 0, startStr);

	if(startPos >= 0)
	{
		aamp_FindSubstr(cleanedPssh, cleanedPsshLen, startPos, endStr, &endPos);
		if(endPos > 0 && startPos < endPos)
		{
			*len = endPos - startPos - 1;
			contentMetaData = (unsigned char*)malloc(*len + 1);
			memset(contentMetaData, 0, *len + 1);
			strncpy(reinterpret_cast<char*>(contentMetaData),reinterpret_cast<char*>(cleanedPssh + startPos + 1), *len);
			//logprintf("%s:%d Content Meta data length  : %d", __FUNCTION__, __LINE__,*len);
		}
	}
	free(cleanedPssh);
	return contentMetaData;
}
