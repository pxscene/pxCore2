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
* @file ClearKeyDrmSession.cpp
* @brief Source file for ClearKey DRM Session.
*/

#include "config.h"
#include "ClearKeyDrmSession.h"
#include "AampDRMutils.h"
#include <gst/gst.h>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include "priv_aamp.h"

#include <openssl/err.h>
#include <sys/time.h>
#include <gst/base/gstbytereader.h>

#define AES_CTR_KID_LEN 16
#define AES_CTR_IV_LEN 16
#define AES_CTR_KEY_LEN 16
/**
 * @brief ClearKeySession Constructor
 */
ClearKeySession::ClearKeySession() :
		AampDrmSession(CLEAR_KEY_SYSTEM_STRING),
		m_sessionID(),
		m_eKeyState(KEY_INIT),
		decryptMutex(),
		m_keyId(NULL),
		mOpensslCtx(),
		m_keyStr(NULL),
		m_keyLen(0),
		m_keyIdLen(0)
{
	pthread_mutex_init(&decryptMutex,NULL);
	initAampDRMSession();
}


/**
 * @brief Initialize CK DRM session, Initializes EVP context.
 */
void ClearKeySession::initAampDRMSession()
{
	EVP_CIPHER_CTX_init(&mOpensslCtx);
	AAMPLOG_ERR("ClearKeySession:: %s:%d :: enter ", __FUNCTION__,__LINE__);
}

/**
 * @brief SetKid for this session.
 * @param keyId
 * @param keyID Len
 */

void ClearKeySession::setKeyId(const char* keyId, int32_t keyIDLen)
{
	if (m_keyId != NULL)
	{
		free(m_keyId);
	}
	m_keyId = (unsigned char*) malloc(sizeof(unsigned char) * keyIDLen);
	memcpy(m_keyId, keyId, keyIDLen);
	m_keyIdLen = keyIDLen;
}

/**
 * @brief Create drm session with given init data
 *        state will be KEY_INIT on success KEY_ERROR if failed
 * @param f_pbInitData pointer to initdata
 * @param f_cbInitData init data size
 */
void ClearKeySession::generateAampDRMSession(const uint8_t *f_pbInitData,
		uint32_t f_cbInitData)
{
	unsigned char *keyId = NULL;
	if(f_pbInitData != NULL && f_cbInitData > 0)
	{
		int keyIDLen = 0;
		keyId = aamp_ExtractKeyIdFromPssh(reinterpret_cast<const char*>(f_pbInitData),f_cbInitData, &keyIDLen, eDRM_ClearKey);
		if (keyId == NULL)
		{
			AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR: Key Id not found in initdata", __FUNCTION__, __LINE__);
			m_eKeyState = KEY_ERROR;
		}
		else
		{
			if(keyIDLen != AES_CTR_KID_LEN)
			{
				AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR: Invalid keyID length %d", __FUNCTION__, __LINE__, keyIDLen);
				m_eKeyState = KEY_ERROR;
			}
			else
			{
				setKeyId(reinterpret_cast<const char*>(keyId), keyIDLen);
				AAMPLOG_ERR("ClearKeySession:: %s:%d Session generated for clearkey",__FUNCTION__, __LINE__);
			}
			free(keyId);
		}
	}
	else
	{
		AAMPLOG_ERR("%s:%d Invalid init data",__FUNCTION__,__LINE__);
		m_eKeyState = KEY_ERROR;
	}
	return;
}


/**
 * @brief ClearKeySession Destructor
 */

ClearKeySession::~ClearKeySession()
{
	pthread_mutex_destroy(&decryptMutex);
	EVP_CIPHER_CTX_cleanup(&mOpensslCtx);
    if(m_keyId != NULL)
    {
        free(m_keyId);
    }
    if(m_keyStr != NULL)
    {
        free(m_keyStr);
    }

	m_eKeyState = KEY_CLOSED;
}


/**
 * @brief Generate key request from DRM session
 *        Caller function should free the returned memory.
 * @param destinationURL : gets updated with license server url
 * @retval Pointer to DrmData containing license request, NULL if failure.
 */
DrmData * ClearKeySession::aampGenerateKeyRequest(string& destinationURL)
{
	DrmData *licenseChallenge = NULL;
	if(NULL == m_keyId || m_keyIdLen <= 0)
	{
		AAMPLOG_ERR("ClearKeySession:: %s:%d :: Error generating license request ", __FUNCTION__, __LINE__);
	}
	else
	{
		char* urlEncodedkeyId = aamp_Base64_URL_Encode(m_keyId, m_keyIdLen);
		if(urlEncodedkeyId)
		{
			cJSON *licenseRequest = cJSON_CreateObject();
			if(licenseRequest)
			{
				cJSON *keyIds = cJSON_CreateArray();
				if(keyIds)
				{
					cJSON_AddItemToArray(keyIds, cJSON_CreateString(urlEncodedkeyId));
					cJSON_AddItemToObject(licenseRequest, "kids", keyIds);
					cJSON_AddItemToObject(licenseRequest, "type",cJSON_CreateString("temporary"));
					char* requestBody = cJSON_PrintUnformatted(licenseRequest);
					if(requestBody)
					{
						AAMPLOG_INFO("%s:%d:: Generated license request : %s", __FUNCTION__, __LINE__, requestBody);
						licenseChallenge = new DrmData(reinterpret_cast<unsigned char*>(requestBody), strlen(requestBody));
						m_eKeyState = KEY_PENDING;
						free(requestBody);
					}
				}
				cJSON_Delete(licenseRequest);
			}
			free(urlEncodedkeyId);
		}
	}
	return licenseChallenge;
}


/**
 * @brief Updates the received key to DRM session
 * @param key : License key from license server.
 * @retval DRM_SUCCESS if no errors encountered
 */
int ClearKeySession::aampDRMProcessKey(DrmData* key)
{
	int ret = 0;
	AAMPLOG_INFO("ClearKeySession:: %s:%d:: Processing license response %s", __FUNCTION__, __LINE__, key->getData());
	if (m_eKeyState == KEY_PENDING)
	{
		cJSON *licenseResponse = cJSON_Parse(reinterpret_cast<const char*>(key->getData()));
		if (licenseResponse)
		{
			if (cJSON_HasObjectItem(licenseResponse, "keys"))
			{
				cJSON * keyEntry = NULL;
				cJSON *keyJsonObj = NULL;
				cJSON *keyIdJsonObj = NULL;

				cJSON *keysArray = cJSON_GetObjectItem(licenseResponse, "keys");
				if(keysArray)
				{
					keyEntry = keysArray->child;
					if(keyEntry)
					{
						keyJsonObj = cJSON_GetObjectItem(keyEntry, "k");
						keyIdJsonObj = cJSON_GetObjectItem(keyEntry, "kid");
					}
				}

				if (keyJsonObj && keyIdJsonObj)
				{
					char *keyJsonStr = cJSON_GetStringValue(keyJsonObj);
					char *keyIdStr = cJSON_GetStringValue(keyIdJsonObj);
					size_t resKeyIdLen = 0;
					size_t resKeyLen = 0;
					unsigned char * resKeyId = aamp_Base64_URL_Decode(keyIdStr,	&resKeyIdLen, strlen(keyIdStr));
					if (resKeyIdLen == m_keyIdLen && 0 == memcmp(m_keyId, resKeyId, m_keyIdLen))
					{
						if (m_keyStr != NULL)
						{
							free (m_keyStr);
						}
						m_keyStr = aamp_Base64_URL_Decode(keyJsonStr, &resKeyLen, strlen(keyJsonStr));
						if (resKeyLen == AES_CTR_KEY_LEN)
						{
							m_keyLen = resKeyLen;
							m_eKeyState = KEY_READY;
							AAMPLOG_INFO("ClearKeySession:: %s:%d:: Got key from license response keyLength %d", __FUNCTION__, __LINE__, m_keyLen);
							ret = 1;
						}
						else
						{
							AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed parse Key from response", __FUNCTION__, __LINE__);
							if (m_keyStr)
							{
								free (m_keyStr);
								m_keyStr = NULL;
							}
							m_eKeyState = KEY_ERROR;
						}
					}
					else
					{
						AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed parse KeyID/invalid keyID, from response", __FUNCTION__, __LINE__);
						m_eKeyState = KEY_ERROR;
					}
					if (resKeyId)
					{
						free(resKeyId);
					}
				}
				else
				{
					AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed parse Key info, from response", __FUNCTION__, __LINE__);
					m_eKeyState = KEY_ERROR;
				}
			}
			cJSON_Delete(licenseResponse);
		}
		else
		{
			AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed parse response JSON", __FUNCTION__, __LINE__);
			m_eKeyState = KEY_ERROR;
		}
	}
	else
	{
		AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Invalid DRM state : DRMState %d",__FUNCTION__, __LINE__, m_eKeyState);
	}
	return ret;
}

/**
 * @brief Function to decrypt stream  buffer.
 * @param keyIDBuffer : keyID Buffer.
 * @param ivBuffer : Initialization vector buffer.
 * @param buffer : Data to decrypt.
 * @param subSampleCount : subSampleCount in buffer
 * @param subSamplesBuffer : sub Samples Buffer.
 * @retval Returns 0 on success.
 */
int ClearKeySession::decrypt(GstBuffer* keyIDBuffer, GstBuffer* ivBuffer, GstBuffer* buffer, unsigned subSampleCount,
                GstBuffer* subSamplesBuffer)
{
	int retVal = 1;
	uint8_t *pbData = NULL;
	uint32_t cbData = 0;
	uint16_t nBytesClear = 0;
	uint32_t nBytesEncrypted = 0;

	GstMapInfo ivMap;
	GstMapInfo subsampleMap;
	GstMapInfo bufferMap;
	GstByteReader* reader = NULL;

	bool ivMapped = false;
	bool subSampleMapped = false;
	bool bufferMapped = false;

	if(!(ivBuffer && buffer && (subSampleCount == 0 || subSamplesBuffer)))
	{
		AAMPLOG_ERR(
		"ClearKeySession:: %s:%d ERROR : Ivalid buffer ivBuffer(%p), buffer(%p), subSamplesBuffer(%p) ",
		__FUNCTION__, __LINE__, ivBuffer, buffer, subSamplesBuffer);
	}
	else
	{
		bufferMapped = gst_buffer_map(buffer, &bufferMap,static_cast<GstMapFlags>(GST_MAP_READWRITE));
		if (!bufferMapped)
		{
			AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to map buffer",	__FUNCTION__, __LINE__);
		}

		ivMapped = gst_buffer_map(ivBuffer, &ivMap, GST_MAP_READ);
		if (!ivMapped)
		{
			AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to map ivBuffer", __FUNCTION__, __LINE__);
		}

		if(subSampleCount > 0)
		{
			subSampleMapped = gst_buffer_map(subSamplesBuffer, &subsampleMap, GST_MAP_READ);
			if (!subSampleMapped)
			{
				AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to map subSamplesBuffer", __FUNCTION__, __LINE__);
			}
		}
	}

	if(bufferMapped && ivMapped && (subSampleCount ==0 || subSampleMapped))
	{
		reader = gst_byte_reader_new(subsampleMap.data, subsampleMap.size);
		if(reader)
		{
			// collect all the encrypted bytes into one contiguous buffer
			// we need to call decrypt once for all encrypted bytes.
			if (subSampleCount > 0)
			{
				pbData = (uint8_t *) malloc(sizeof(uint8_t) * bufferMap.size);
				uint8_t *pbCurrTarget = (uint8_t *) pbData;
				uint32_t iCurrSource = 0;

		        for (int i = 0; i < subSampleCount; i++)
		        {
					if (!gst_byte_reader_get_uint16_be(reader, &nBytesClear)
							|| !gst_byte_reader_get_uint32_be(reader, &nBytesEncrypted))
					{
						AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to read from subsamples reader", __FUNCTION__, __LINE__);
						cbData = 0;
						break;
					}
					// Skip the clear byte range from source buffer.
					iCurrSource += nBytesClear;

					// Copy cipher bytes from f_pbData to target buffer.
					memcpy(pbCurrTarget, (uint8_t*) bufferMap.data + iCurrSource, nBytesEncrypted);

					// Adjust current pointer of target buffer.
					pbCurrTarget += nBytesEncrypted;

					// Adjust current offset of source buffer.
					iCurrSource += nBytesEncrypted;
					cbData += nBytesEncrypted;
				}
			}
	        else
	        {
				pbData = bufferMap.data;
				cbData = bufferMap.size;
	        }

			if(cbData != 0)
			{
				retVal = decrypt(static_cast<uint8_t *>(ivMap.data), static_cast<uint32_t>(ivMap.size),
			            pbData, cbData, NULL);
			}

			if(retVal == 0)
			{
				if(subSampleCount > 0)
				{
					// If subsample mapping is used, copy decrypted bytes back
					// to the original buffer.
					gst_byte_reader_set_pos(reader, 0);
					uint8_t *pbCurrTarget = bufferMap.data;
					uint32_t iCurrSource = 0;

			        for (int i = 0; i < subSampleCount; i++)
			        {
						if (!gst_byte_reader_get_uint16_be(reader, &nBytesClear)
								|| !gst_byte_reader_get_uint32_be(reader, &nBytesEncrypted))
						{
							AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to read from subsamples reader", __FUNCTION__, __LINE__);
							retVal = 1;
							break;
						}
						// Skip the clear byte range from target buffer.
						pbCurrTarget += nBytesClear;
						memcpy(pbCurrTarget, pbData + iCurrSource, nBytesEncrypted);

						// Adjust current pointer of target buffer.
						pbCurrTarget += nBytesEncrypted;

						// Adjust current offset of source buffer.
						iCurrSource += nBytesEncrypted;
					}
				}//SubSample end if
			} //retVal end if
		}//reader end if
		else
		{
			AAMPLOG_ERR("ClearKeySession:: %s:%d ERROR : Failed to allocate subsample reader", __FUNCTION__, __LINE__);
		}
	}

	if(subSampleCount > 0 && pbData)
	{
		free(pbData);
	}
	if(bufferMapped)
	{
		gst_buffer_unmap(buffer, &bufferMap);
	}
	if(ivMapped)
	{
		gst_buffer_unmap(ivBuffer, &ivMap);
	}
	if(subSampleMapped)
	{
		gst_buffer_unmap(subSamplesBuffer, &subsampleMap);
	}

	return retVal;
}


/**
 * @brief Function to decrypt stream  buffer.
 * @param f_pbIV : Initialization vector.
 * @param f_cbIV : Initialization vector length.
 * @param payloadData : Data to decrypt.
 * @param payloadDataSize : Size of data.
 * @param ppOpaqueData : pointer to opaque buffer in case of SVP.
 * @retval Returns 0 on success.
 */
int ClearKeySession::decrypt(const uint8_t *f_pbIV, uint32_t f_cbIV,
		const uint8_t *payloadData, uint32_t payloadDataSize, uint8_t **ppOpaqueData=NULL)
{
	int status = 1;
	pthread_mutex_lock(&decryptMutex);

	if (m_eKeyState == KEY_READY)
	{
		uint8_t *decryptedDataBuf = (uint8_t *)malloc(payloadDataSize);
		uint32_t decryptedDataLen = 0;
		uint8_t *ivBuff = NULL;
		memset(decryptedDataBuf, 0, payloadDataSize);
		if(f_cbIV == 8)//8 byte IV need to pad with 0 before decrypt
		{
			ivBuff = (uint8_t *)malloc(sizeof(uint8_t) * AES_CTR_IV_LEN);
			memset(ivBuff + 8, 0, 8);
			memcpy(ivBuff, f_pbIV, 8);
		}
		else if(f_cbIV == AES_CTR_IV_LEN)
		{
			ivBuff = (uint8_t *)malloc(sizeof(uint8_t) * AES_CTR_IV_LEN);
			memcpy(ivBuff, f_pbIV, AES_CTR_IV_LEN);
		}
		else
		{
			AAMPLOG_TRACE("ClearKeySession::%s:%d: invalid IV size %u",  __FUNCTION__, __LINE__, f_cbIV);
		}

		if (decryptedDataBuf && ivBuff)
		{
			uint32_t decLen = payloadDataSize;

			if(!EVP_DecryptInit_ex(&mOpensslCtx, EVP_aes_128_ctr(), NULL, m_keyStr, ivBuff))
			{
				AAMPLOG_TRACE( "ClearKeySession::%s:%d: EVP_DecryptInit_ex failed",  __FUNCTION__, __LINE__);
			}
			else
			{
				if (!EVP_DecryptUpdate(&mOpensslCtx,(unsigned char*) decryptedDataBuf, (int*) &decLen, (const unsigned char*) payloadData,
					payloadDataSize))
				{
					AAMPLOG_TRACE("ClearKeySession::%s:%d: EVP_DecryptUpdate failed", __FUNCTION__, __LINE__);
				}
				else
				{
					decryptedDataLen = decLen;
					decLen = 0;
					AAMPLOG_TRACE("ClearKeySession::%s:%d: EVP_DecryptUpdate success decryptedDataLen = %d payload Data length = %d", __FUNCTION__, __LINE__, (int) decryptedDataLen, (int)payloadDataSize);
					if (!EVP_DecryptFinal_ex(&mOpensslCtx, (unsigned char*) (decryptedDataBuf + decryptedDataLen), (int*) &decLen))
					{
						AAMPLOG_TRACE("ClearKeySession::%s:%d: EVP_DecryptFinal_ex failed mDrmState = %d", __FUNCTION__, __LINE__, (int) m_eKeyState);
					}
					else
					{
						decryptedDataLen += decLen;
						AAMPLOG_TRACE("ClearKeySession::%s:%d decrypt success", __FUNCTION__, __LINE__);
						status = 0;
					}
				}
			}

			memcpy((void *)payloadData, decryptedDataBuf, payloadDataSize);
			free(decryptedDataBuf);
			free(ivBuff);
		}
	}
	else
	{
		AAMPLOG_ERR( "ClearKeySession:: %s:%d:key not ready! mDrmState = %d",  __FUNCTION__, __LINE__, m_eKeyState);
	}
	pthread_mutex_unlock(&decryptMutex);
	return status;
}


/**
 * @brief Get the current state of DRM Session.
 * @retval KeyState
 */
KeyState ClearKeySession::getState()
{
	return m_eKeyState;
}

/**
 * @brief Clear the current session context
 *        So that new init data can be bound.
 */
void ClearKeySession:: clearDecryptContext()
{
	if(m_keyId != NULL)
	{
		free(m_keyId);
		m_keyId = NULL;
		m_keyIdLen = 0;
	}
	if(m_keyStr != NULL)
	{
		free(m_keyStr);
		m_keyStr = NULL;
		m_keyLen = 0;
	}
	EVP_CIPHER_CTX_cleanup(&mOpensslCtx);
	m_eKeyState = KEY_INIT;
}


