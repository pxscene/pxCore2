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
 * @file AampDrmSession.h
 * @brief Header file for AampDrmSession
 */


#ifndef AampDrmSession_h
#define AampDrmSession_h
#include "AampDRMutils.h"
#include <string>
#include <stdint.h>
#include <gst/gst.h>
using namespace std;

#define PLAYREADY_PROTECTION_SYSTEM_ID "9a04f079-9840-4286-ab92-e65be0885f95"
#define WIDEVINE_PROTECTION_SYSTEM_ID "edef8ba9-79d6-4ace-a3c8-27dcd51d21ed"

#define PLAYREADY_KEY_SYSTEM_STRING "com.microsoft.playready"
#define WIDEVINE_KEY_SYSTEM_STRING "com.widevine.alpha"

#define HDCP_AUTHENTICATION_FAILURE 4327
/**
 * @enum KeyState 
 * @brief DRM session states
 */
typedef	enum
{
	// Has been initialized.
	KEY_INIT = 0,
	// Has a key message pending to be processed.
	KEY_PENDING = 1,
	// Has a usable key.
	KEY_READY = 2,
	// Has an error.
	KEY_ERROR = 3,
	// Has been closed.
	KEY_CLOSED = 4
} KeyState;

/**
 * @class AampDrmSession
 * @brief Base class for DRM sessions
 */
class AampDrmSession
{
protected:
	std::string m_keySystem;
public:

	/**
	 * @brief Create drm session with given init data
	 * @param f_pbInitData : pointer to initdata
	 * @param f_cbInitData : init data size
	 */
	virtual void generateAampDRMSession(const uint8_t *f_pbInitData,uint32_t f_cbInitData) = 0;

	/**
	 * @brief Generate key request from DRM session
	 *	      Caller function should free the returned memory.
	 * @param destinationURL : gets updated with license server url
	 * @retval Pointer to DrmData containing license request.
	 */
	virtual DrmData* aampGenerateKeyRequest(string& destinationURL) = 0;

	/**
	 * @brief Updates the received key to DRM session
	 * @param key : License key from license server.
	 * @retval returns status of update request
	 */
	virtual int aampDRMProcessKey(DrmData* key) = 0;

	/**
	 * @brief Function to decrypt stream  buffer.
	 * @param f_pbIV : Initialization vector.
	 * @param f_cbIV : Initialization vector length.
	 * @param payloadData : Data to decrypt.
	 * @param payloadDataSize : Size of data.
	 * @param ppOpaqueData : pointer to opaque buffer in case of SVP.
	 * @retval Returns status of decrypt request.
	 */
#if defined(USE_OPENCDM_ADAPTER)
	virtual int decrypt(GstBuffer* keyIDBuffer, GstBuffer* ivBuffer, GstBuffer* buffer, unsigned subSampleCount, GstBuffer* subSamplesBuffer) = 0;
#else
	virtual int decrypt(const uint8_t *f_pbIV, uint32_t f_cbIV,const uint8_t *payloadData, uint32_t payloadDataSize, uint8_t **ppOpaqueData) = 0;
#endif

	/**
	 * @brief Get the current state of DRM Session.
	 * @retval KeyState
	 */
	virtual KeyState getState() = 0;

	/**
	 * @brief Clear the current session context
	 *        So that new init data can be bound.
	 */
	virtual void clearDecryptContext() = 0;

	/**
	 * @brief Constructor for AampDrmSession.
	 * @param keySystem : DRM key system uuid
	 */
	AampDrmSession(const string &keySystem);

	/**
	 * @brief Destructor for AampDrmSession..
	 */
	virtual ~AampDrmSession();

	/**
	 * @brief Get the DRM System, ie, UUID for PlayReady WideVine etc..
	 * @retval DRM system uuid
	 */
	string getKeySystem();

#if defined(USE_OPENCDM_ADAPTER)
	virtual void setKeyId(const char* keyId, int32_t keyLen) = 0;
#endif
};
#endif
