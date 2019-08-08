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

/* Comcast DRM Session management for Aamp
 *
 */

#ifndef OpenCDMSession_h
#define OpenCDMSession_h

#include "AampDrmSession.h"
#include "aampoutputprotection.h"
#include <open_cdm.h>
using namespace std;

/**
 * @class AAMPOCDMSession
 * @brief Open CDM DRM session
 */
class AAMPOCDMSession : public AampDrmSession
{

private:
	pthread_mutex_t decryptMutex;

	AampOutputProtection* m_pOutputProtection;
	KeyState m_eKeyState;
	media::OpenCdm* m_pOpencdm;
	media::OpenCdm* m_pOpencdmDecrypt;
	string m_sessionID;
	void initAampDRMSession();

public:
    AAMPOCDMSession(string& keySystem);
	~AAMPOCDMSession();
	AAMPOCDMSession(const AAMPOCDMSession&) = delete;
	AAMPOCDMSession& operator=(const AAMPOCDMSession&) = delete;
	void generateAampDRMSession(const uint8_t *f_pbInitData,
			uint32_t f_cbInitData);
	DrmData * aampGenerateKeyRequest(string& destinationURL);
	int aampDRMProcessKey(DrmData* key);
	int decrypt(const uint8_t *f_pbIV, uint32_t f_cbIV,
			const uint8_t *payloadData, uint32_t payloadDataSize, uint8_t **ppOpaqueData);
	KeyState getState();
	void clearDecryptContext();
};

#endif

