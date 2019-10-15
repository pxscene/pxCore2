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
 * @file playreadydrmsession.h
 * @brief Comcast Playready Session management
 */

#ifndef PlayReadyDrmSession_h
#define PlayReadyDrmSession_h

#include "AampDrmSession.h"
#include "aampoutputprotection.h"
#include <drmbuild_oem.h>
#include <drmcommon.h>
#include <drmmanager.h>
#include <drmmathsafe.h>
#include <drmtypes.h>
#include <drmerr.h>
#undef __in
#undef __out
using namespace std;

#define ChkBufferSize(a,b) do { \
    ChkBOOL((a) <= (b), DRM_E_FAIL); \
} while(FALSE)

/**
 * @class PlayReadyDRMSession
 * @brief Class for PlayReady DRM operations
 */
class PlayReadyDRMSession : public AampDrmSession
{

private:
	DRM_APP_CONTEXT *m_poAppContext;
	DRM_DECRYPT_CONTEXT m_oDecryptContext;

	DRM_BYTE *m_pbOpaqueBuffer;
	DRM_DWORD m_cbOpaqueBuffer;

	DRM_BYTE *m_pbRevocationBuffer;
	KeyState m_eKeyState;
	DRM_CHAR m_rgchSessionID[CCH_BASE64_EQUIV(SIZEOF(DRM_ID)) + 1];
	DRM_BOOL m_fCommit;

	DRM_BYTE *m_pbPRO;
	DRM_DWORD m_cbPRO;

	DRM_BYTE *m_pbChallenge;
	DRM_DWORD m_cbChallenge;
	DRM_CHAR *m_pchSilentURL;
	pthread_mutex_t decryptMutex;

	AampOutputProtection* m_pOutputProtection;


	void initAampDRMSession();

	int _GetPROFromInitData(const DRM_BYTE *f_pbInitData,
			DRM_DWORD f_cbInitData, DRM_DWORD *f_pibPRO, DRM_DWORD *f_pcbPRO);

	int _ParseInitData(const uint8_t *f_pbInitData, uint32_t f_cbInitData);


public:

	PlayReadyDRMSession();

	~PlayReadyDRMSession();

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

