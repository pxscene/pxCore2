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
 * @file ClearKeySession.h
 * @brief Header file for ClearKeySession
 */

#ifndef ClearKeySession_h
#define ClearKeySession_h

#include "AampDrmSession.h"
#include "openssl/evp.h"
#include "_base64.h"

#include <memory>
#include <pthread.h>
#include <cjson/cJSON.h>

using namespace std;

/**
 * @class AAMPOCDMSession
 * @brief Open CDM DRM session
 */
class ClearKeySession : public AampDrmSession
{

private:
	pthread_mutex_t decryptMutex;

	KeyState m_eKeyState;
	string m_sessionID;
	unsigned char* m_keyStr;
	size_t m_keyLen;
	unsigned char* m_keyId;
	size_t m_keyIdLen;
	void initAampDRMSession();
	EVP_CIPHER_CTX mOpensslCtx;

public:

	/**
	 * @brief ClearKeySession Constructor
	 */
	ClearKeySession();

	/**
	 * @brief ClearKeySession Destructor
	 */
	~ClearKeySession();

	ClearKeySession(const ClearKeySession&) = delete;

	ClearKeySession& operator=(const ClearKeySession&) = delete;

	/**
	 * @brief Create drm session with given init data
	 *        state will be KEY_INIT on success KEY_ERROR if failed
	 * @param f_pbInitData pointer to initdata
	 * @param f_cbInitData init data size
	 */
	void generateAampDRMSession(const uint8_t *f_pbInitData,
			uint32_t f_cbInitData);

	/**
	 * @brief Generate key request from DRM session
	 *        Caller function should free the returned memory.
	 * @param destinationURL : gets updated with license server url
	 * @retval Pointer to DrmData containing license request, NULL if failure.
	 */
	DrmData * aampGenerateKeyRequest(string& destinationURL);

	/**
	 * @brief Updates the received key to DRM session
	 * @param key : License key from license server.
	 * @retval 1 if no errors encountered
	 */
	int aampDRMProcessKey(DrmData* key);

	/**
	 * @brief SetKid for this session.
	 * @param keyId
	 * @param keyID Len
	 */
	void setKeyId(const char* keyId, int32_t keyIDLen);

	/**
	 * @brief Function to decrypt stream.
	 * @param f_pbIV : Initialization vector.
	 * @param f_cbIV : Initialization vector length.
	 * @param payloadData : Data to decrypt.
	 * @param payloadDataSize : Size of data.
	 * @param ppOpaqueData : pointer to opaque buffer in case of SVP.
	 * @retval Returns 0 on success.
	 */
	int decrypt(const uint8_t *f_pbIV, uint32_t f_cbIV,
			const uint8_t *payloadData, uint32_t payloadDataSize, uint8_t **ppOpaqueData);

	//If OCDM_ADAPTOR is in use below decrypt funtion wil be invoked from plugin
	/**
	 * @brief Function to decrypt stream  buffer.
	 * @param keyIDBuffer : keyID Buffer.
	 * @param ivBuffer : Initialization vector buffer.
	 * @param buffer : Data to decrypt.
	 * @param subSampleCount : subSampleCount in buffer
	 * @param subSamplesBuffer : sub Samples Buffer.
	 * @retval Returns 0 on success.
	 */
	int decrypt(GstBuffer* keyIDBuffer, GstBuffer* ivBuffer, GstBuffer* buffer, unsigned subSampleCount,
				GstBuffer* subSamplesBuffer);

	/**
	 * @brief Get the current state of DRM Session.
	 * @retval KeyState
	 */
	KeyState getState();

	/**
	 * @brief Clear the current session context
	 *        So that new init data can be bound.
	 */
	void clearDecryptContext();
};

#endif

