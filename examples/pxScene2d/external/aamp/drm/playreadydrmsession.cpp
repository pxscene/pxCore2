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
 * @file playreadydrmsession.cpp
 * @brief Comcast Playready Session management
 */

#include "config.h"
#include "playreadydrmsession.h"
#include <gst/gst.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <sys/utsname.h>
#include "priv_aamp.h"

#define NYI_KEYSYSTEM "keysystem-placeholder"
//#define TRACE_LOG 1

extern DRM_CONST_STRING g_dstrDrmPath;

// The default location of CDM DRM store.
// /opt/drm/playready/drmstore.dat
const DRM_WCHAR g_rgwchCDMDrmStoreName[] =
{ WCHAR_CAST('/'), WCHAR_CAST('o'), WCHAR_CAST('p'), WCHAR_CAST('t'),
		WCHAR_CAST('/'), WCHAR_CAST('d'), WCHAR_CAST('r'), WCHAR_CAST('m'),
		WCHAR_CAST('/'), WCHAR_CAST('p'), WCHAR_CAST('l'), WCHAR_CAST('a'),
		WCHAR_CAST('y'), WCHAR_CAST('r'), WCHAR_CAST('e'), WCHAR_CAST('a'),
		WCHAR_CAST('d'), WCHAR_CAST('y'), WCHAR_CAST('/'), WCHAR_CAST('d'),
		WCHAR_CAST('r'), WCHAR_CAST('m'), WCHAR_CAST('s'), WCHAR_CAST('t'),
		WCHAR_CAST('o'), WCHAR_CAST('r'), WCHAR_CAST('e'), WCHAR_CAST('.'),
		WCHAR_CAST('d'), WCHAR_CAST('a'), WCHAR_CAST('t'), WCHAR_CAST('\0') };
// default PR DRM path
// /opt/drm/playready
const DRM_WCHAR g_rgwchCDMDrmPath[] =
{ WCHAR_CAST('/'), WCHAR_CAST('o'), WCHAR_CAST('p'), WCHAR_CAST('t'),
		WCHAR_CAST('/'), WCHAR_CAST('d'), WCHAR_CAST('r'), WCHAR_CAST('m'),
		WCHAR_CAST('/'), WCHAR_CAST('p'), WCHAR_CAST('l'), WCHAR_CAST('a'),
		WCHAR_CAST('y'), WCHAR_CAST('r'), WCHAR_CAST('e'), WCHAR_CAST('a'),
		WCHAR_CAST('d'), WCHAR_CAST('y'), WCHAR_CAST('\0') };

const DRM_CONST_STRING g_dstrCDMDrmStoreName = CREATE_DRM_STRING(
		g_rgwchCDMDrmStoreName);
const DRM_CONST_STRING g_dstrCDMDrmPath = CREATE_DRM_STRING(g_rgwchCDMDrmPath);
const DRM_CONST_STRING *g_rgpdstrRights[1] =
{ &g_dstrWMDRM_RIGHT_PLAYBACK };


// The following function is missing from the official PK 2.5 release but
// will be available in the next PK release.
// It should be removed if the source is building with the next PK release.
/**
 * @brief Read UUID from init data
 * @param pbData : Pointer to initdata
 * @param cbData : size of init data
 * @param ibGuidOffset : offset to uuid
 * @param pDrmGuid : Gets updated with uuid
 * @retval DRM_SUCCESS if no errors encountered
 */

DRM_API DRM_RESULT DRM_CALL DRM_UTL_ReadNetworkBytesToNativeGUID(
		const DRM_BYTE *pbData, const DRM_DWORD cbData, DRM_DWORD ibGuidOffset,
		DRM_GUID *pDrmGuid)
{
	DRM_RESULT dr = DRM_SUCCESS;
	DRM_DWORD dwResult = 0;

	ChkArg(pbData != NULL);
	ChkArg(pDrmGuid != NULL);
	ChkOverflow(cbData, ibGuidOffset);
	ChkDR(DRM_DWordSub(cbData, ibGuidOffset, &dwResult));
	ChkBOOL(dwResult >= DRM_GUID_LEN, DRM_E_BUFFERTOOSMALL);

	// Convert field by field.
	NETWORKBYTES_TO_DWORD(pDrmGuid->Data1, pbData, ibGuidOffset);
	ChkDR(DRM_DWordAdd(ibGuidOffset, SIZEOF(DRM_DWORD), &ibGuidOffset));

	NETWORKBYTES_TO_WORD(pDrmGuid->Data2, pbData, ibGuidOffset);
	ChkDR(DRM_DWordAdd(ibGuidOffset, SIZEOF(DRM_WORD), &ibGuidOffset));

	NETWORKBYTES_TO_WORD(pDrmGuid->Data3, pbData, ibGuidOffset);
	ChkDR(DRM_DWordAdd(ibGuidOffset, SIZEOF(DRM_WORD), &ibGuidOffset));

	// Copy last 8 bytes.
	DRM_BYT_CopyBytes(pDrmGuid->Data4, 0, pbData, ibGuidOffset, 8);

	ErrorExit: return dr;
}

/**
 * @brief PlayReadyDRMSession Constructor
 */
PlayReadyDRMSession::PlayReadyDRMSession() :
		AampDrmSession(PLAYREADY_KEY_SYSTEM_STRING), m_poAppContext(NULL), m_pbOpaqueBuffer(NULL),
		m_cbOpaqueBuffer(0), m_pbRevocationBuffer(NULL), m_eKeyState(KEY_INIT), m_fCommit(FALSE),
		m_pbChallenge(NULL), m_cbChallenge(0), m_pchSilentURL(NULL), m_pbPRO(NULL), m_cbPRO(0)
{
	pthread_mutex_init(&decryptMutex,NULL);
	initAampDRMSession();

	// Get output protection pointer
	m_pOutputProtection = AampOutputProtection::GetAampOutputProcectionInstance();
}


/**
 * @brief Initialize PR DRM session, state will be set as KEY_INIT
 *        on success KEY_ERROR if failure.
 */
void PlayReadyDRMSession::initAampDRMSession()
{

	DRM_RESULT dr = DRM_SUCCESS;
	DRM_ID oSessionID;
	DRM_DWORD cchEncodedSessionID = SIZEOF(m_rgchSessionID);

	char *envParseInitData = NULL;

	ChkMem(
			m_pbOpaqueBuffer = (DRM_BYTE *) Oem_MemAlloc(
					MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE));
	m_cbOpaqueBuffer = MINIMUM_APPCONTEXT_OPAQUE_BUFFER_SIZE;

	ChkMem(
			m_poAppContext = (DRM_APP_CONTEXT *) Oem_MemAlloc(
					SIZEOF(DRM_APP_CONTEXT)));

	g_dstrDrmPath = g_dstrCDMDrmPath;

	/* Initialize DRM app context.
	   Mutex lock is added as it runs in to deadlock sometimes
	   when initialization is called for both Audio and Video
	*/

	static pthread_mutex_t sessionMutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_lock(&sessionMutex);
	dr = Drm_Initialize(m_poAppContext,
	NULL, m_pbOpaqueBuffer, m_cbOpaqueBuffer, &g_dstrCDMDrmStoreName);
	pthread_mutex_unlock(&sessionMutex);

#ifdef TRACE_LOG
	logprintf("Printing initialization result : %08x \n", dr);
#endif

	ChkDR(dr);

	if (DRM_REVOCATION_IsRevocationSupported())
	{
		ChkMem(
				m_pbRevocationBuffer = (DRM_BYTE *) Oem_MemAlloc(
						REVOCATION_BUFFER_SIZE));

		ChkDR(
				Drm_Revocation_SetBuffer(m_poAppContext, m_pbRevocationBuffer,
						REVOCATION_BUFFER_SIZE));
	}

	// Generate a random media session ID.
	ChkDR(
			Oem_Random_GetBytes(NULL, (DRM_BYTE *) &oSessionID,
					SIZEOF(oSessionID)));

	ZEROMEM(m_rgchSessionID, SIZEOF(m_rgchSessionID));
	// Store the generated media session ID in base64 encoded form.
	ChkDR(
			DRM_B64_EncodeA((DRM_BYTE *) &oSessionID, SIZEOF(oSessionID),
					m_rgchSessionID, &cchEncodedSessionID, 0));

	logprintf("initAampDRMSession :: Playready initialized with session id : %s\n",m_rgchSessionID);
	// The current state MUST be KEY_INIT otherwise error out.
	ChkBOOL(m_eKeyState == KEY_INIT, DRM_E_INVALIDARG);
	return;
	ErrorExit:
	logprintf("Playready initialization failed code : %08x \n", dr);
	m_eKeyState = KEY_ERROR;
}

/**
 * @brief Create drm session with given init data
 *        state will be KEY_INIT on success KEY_ERROR if failed
 * @param f_pbInitData pointer to initdata
 * @param f_cbInitData init data size
 */
void PlayReadyDRMSession::generateAampDRMSession(const uint8_t *f_pbInitData,
		uint32_t f_cbInitData)
{
	DRM_RESULT dr = DRM_SUCCESS;

	if (f_pbInitData != NULL)
	{
#ifdef TRACE_LOG
		cout << "parseinitdata..." << endl;
#endif
		ChkDR(_ParseInitData(f_pbInitData, f_cbInitData));
	//	}
	}


	if (m_cbPRO > 0)
	{
		if (gpGlobalConfig->logging.debug)
		{
			logprintf("PRO found in initdata!\n");
		}
		// If PRO is supplied (via init data) then it is used
		// to create the content header inside of the app context.
		dr = Drm_Content_SetProperty(m_poAppContext, DRM_CSP_AUTODETECT_HEADER,
				m_pbPRO, m_cbPRO);
	} else
	{
		if (gpGlobalConfig->logging.debug)
		{
			logprintf("PRO not found in initdata!\n");
		}
		dr = Drm_Content_SetProperty(m_poAppContext, DRM_CSP_AUTODETECT_HEADER,
				f_pbInitData, f_cbInitData);
	}

#ifdef TRACE_LOG
	logprintf("initAampDRMSession :: Printing SetProperty result : %08x \n", dr);
#endif

	ChkDR(dr);

	// The current state MUST be KEY_INIT otherwise error out.
	ChkBOOL(m_eKeyState == KEY_INIT, DRM_E_INVALIDARG);
	return;

	ErrorExit:
	logprintf("Playready init data binding failed : Error code : %08x \n",dr);
	m_eKeyState = KEY_ERROR;
}


/**
 * @brief PlayReadyDRMSession Destructor
 */
PlayReadyDRMSession::~PlayReadyDRMSession()
{
	pthread_mutex_destroy(&decryptMutex);
	SAFE_OEM_FREE(m_pbChallenge);
	SAFE_OEM_FREE(m_pchSilentURL);
	if(m_pbPRO != NULL)
	{
		SAFE_OEM_FREE(m_pbPRO);
	}
	m_pbPRO = NULL;
	if (DRM_REVOCATION_IsRevocationSupported())
		SAFE_OEM_FREE(m_pbRevocationBuffer);

	SAFE_OEM_FREE(m_pbOpaqueBuffer);
	SAFE_OEM_FREE(m_poAppContext);
	m_eKeyState = KEY_CLOSED;

	if(m_pOutputProtection)
	{
	    m_pOutputProtection->Release();
	}
}

// The standard PlayReady protection system ID.
static DRM_ID CLSID_PlayReadyProtectionSystemID =
{ 0x79, 0xf0, 0x04, 0x9a, 0x40, 0x98, 0x86, 0x42, 0xab, 0x92, 0xe6, 0x5b, 0xe0,
		0x88, 0x5f, 0x95 };

// The standard ID of the PSSH box wrapped inside of a UUID box.
static DRM_ID PSSH_BOX_GUID =
{ 0x18, 0x4f, 0x8a, 0xd0, 0xf3, 0x10, 0x82, 0x4a, 0xb6, 0xc8, 0x32, 0xd8, 0xab,
		0xa1, 0x83, 0xd3 };

/**
 * @brief Retrieve PlayReady Object(PRO) from init data
 * @param f_pbInitData : Pointer to initdata
 * @param f_cbInitData : size of initdata
 * @param f_pibPRO : Gets updated with PRO
 * @param f_pcbPRO : size of PRO
 * @retval DRM_SUCCESS if no errors encountered
 */
int PlayReadyDRMSession::_GetPROFromInitData(const DRM_BYTE *f_pbInitData,
		DRM_DWORD f_cbInitData, DRM_DWORD *f_pibPRO, DRM_DWORD *f_pcbPRO)
{
	DRM_RESULT dr = DRM_SUCCESS;
	DRM_DWORD ibCur = 0;
	DRM_DWORD cbSize = 0;
	DRM_DWORD dwType = 0;
	DRM_WORD wVersion = 0;
	DRM_WORD wFlags = 0;
	DRM_GUID guidSystemID = EMPTY_DRM_GUID;
	DRM_GUID guidUUID = EMPTY_DRM_GUID;
	DRM_DWORD cbSystemSize = 0;
	DRM_BOOL fFound = FALSE;
	DRM_BOOL fUUIDBox = FALSE;
	DRM_DWORD cbMultiKid = 0;
	DRM_DWORD dwResult = 0;

	ChkArg(f_pbInitData != NULL);
	ChkArg(f_cbInitData > 0);
	ChkArg(f_pibPRO != NULL);
	ChkArg(f_pcbPRO != NULL);

	*f_pibPRO = 0;
	*f_pcbPRO = 0;

	while (ibCur < f_cbInitData && !fFound)
	{
		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);
		NETWORKBYTES_TO_DWORD(cbSize, f_pbInitData, ibCur);
		ibCur = dwResult;

		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);
		NETWORKBYTES_TO_DWORD(dwType, f_pbInitData, ibCur);
		ibCur = dwResult;

		// 0x64697575 in big endian stands for "uuid".
		if (dwType == 0x75756964)
		{
			ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_GUID), &dwResult));
			ChkBufferSize(dwResult, f_cbInitData);

			ChkDR(
					DRM_UTL_ReadNetworkBytesToNativeGUID(f_pbInitData,
							f_cbInitData, ibCur, &guidUUID));
			ibCur = dwResult;

			ChkBOOL(MEMCMP(&guidUUID, &PSSH_BOX_GUID, SIZEOF(DRM_ID)) == 0,
					DRM_E_FAIL);
			fUUIDBox = TRUE;
		} else
		{
			// 0x68737370 in big endian stands for "pssh".
			ChkBOOL(dwType == 0x70737368, DRM_E_FAIL);
		}

		// Read "version" of PSSH box.
		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_WORD), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);
		NETWORKBYTES_TO_WORD(wVersion, f_pbInitData, ibCur);
		ibCur = dwResult;

		// Read "flags" of PSSH box.
		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_WORD), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);
		NETWORKBYTES_TO_WORD(wFlags, f_pbInitData, ibCur);
		ibCur = dwResult;

		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_GUID), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);

		// Read "system ID" of PSSH box.
		ChkDR(
				DRM_UTL_ReadNetworkBytesToNativeGUID(f_pbInitData, f_cbInitData,
						ibCur, &guidSystemID));
		ibCur = dwResult;

		// Handle multi-KIDs pssh box.
		if (wVersion > 0)
		{
			DRM_DWORD cKids = 0;

			ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
			ChkBufferSize(dwResult, f_cbInitData);
			NETWORKBYTES_TO_DWORD(cKids, f_pbInitData, ibCur);
			ibCur = dwResult;

			// Ignore the KIDs.
			// ibCur + cKids * sizeof( GUID )
			ChkDR(DRM_DWordMult(cKids, SIZEOF(DRM_GUID), &dwResult));
			ChkDR(DRM_DWordAdd(ibCur, dwResult, &dwResult));
			ChkBufferSize(dwResult, f_cbInitData);
			ibCur = dwResult;

			cbMultiKid = SIZEOF(DRM_DWORD) + (cKids * SIZEOF(DRM_GUID));
		}

		ChkDR(DRM_DWordAdd(ibCur, SIZEOF(DRM_DWORD), &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);
		NETWORKBYTES_TO_DWORD(cbSystemSize, f_pbInitData, ibCur);
		ibCur = dwResult;

		// Make sure the payload is still within the limit.
		ChkDR(DRM_DWordAdd(ibCur, cbSystemSize, &dwResult));
		ChkBufferSize(dwResult, f_cbInitData);

		// Check whether the "system ID" just read is for PlayReady.
		if (MEMCMP(&guidSystemID, &CLSID_PlayReadyProtectionSystemID,
				SIZEOF(DRM_GUID)) == 0)
		{
			fFound = TRUE;
		} else
		{
			ibCur = dwResult;
		}
	}

	if (!fFound)
	{
		ChkDR (DRM_E_FAIL);
	}

	// Make sure the total size of all components
	// match the overall size.
	if (fUUIDBox)
	{
		ChkBOOL(
				cbSystemSize + SIZEOF(cbSize) + SIZEOF(dwType)
						+ SIZEOF(DRM_GUID) + SIZEOF(wVersion) + SIZEOF(wFlags)
						+ SIZEOF(DRM_GUID) + SIZEOF(cbSystemSize) == cbSize,
				DRM_E_FAIL);
	} else
	{
		ChkBOOL(
				cbSystemSize + SIZEOF(cbSize) + SIZEOF(dwType)
						+ SIZEOF(wVersion) + SIZEOF(wFlags) + SIZEOF(DRM_GUID)
						+ SIZEOF(cbSystemSize) + cbMultiKid == cbSize,
				DRM_E_FAIL);
	}

	*f_pibPRO = ibCur;
	*f_pcbPRO = cbSystemSize;

	ErrorExit: return dr;
}

/**
 * @brief Parse init data to retrieve PRO from it
 * @param f_pbInitData : Pointer to initdata
 * @param f_cbInitData : size of init data
 * @retval DRM_SUCCESS if no errors encountered
 */
int PlayReadyDRMSession::_ParseInitData(const uint8_t *f_pbInitData,
		uint32_t f_cbInitData)
{
	DRM_RESULT dr = DRM_SUCCESS;
	DRM_DWORD ibPRO = 0;
	DRM_BYTE *pbPRO = NULL;
	DRM_DWORD cbPRO = 0;

	ChkArg(f_pbInitData != NULL && f_cbInitData > 0);

	// If key ID is already specified by CDM data then PRO is
	// not allowed to be specified in init data.
	// In the current implementation this should never happen
	// since init data is always processed before CDM data.
	//DRMASSERT(!m_fKeyIdSet);

	// Parse init data to retrieve PRO.
	ChkDR(_GetPROFromInitData(f_pbInitData, f_cbInitData, &ibPRO, &cbPRO));
	ChkBOOL(cbPRO > 0, DRM_E_FAIL);
	ChkMem(pbPRO = (DRM_BYTE *) Oem_MemAlloc(cbPRO));

	MEMCPY(pbPRO, f_pbInitData + ibPRO, cbPRO);

	m_cbPRO = cbPRO;
	m_pbPRO = pbPRO;

	ErrorExit:
	return dr;
}

/**
 * @brief Generate key request from DRM session
 *        Caller function should free the returned memory.
 * @param destinationURL : gets updated with license server url
 * @retval Pointer to DrmData containing license request, NULL if failure.
 */
DrmData * PlayReadyDRMSession::aampGenerateKeyRequest(string& destinationURL)
{

	DrmData * result = NULL;
	DRM_RESULT dr = DRM_SUCCESS;
	DRM_DWORD cchSilentURL = 0;
	DRM_ANSI_STRING dastrCustomData = EMPTY_DRM_STRING;


	// Try to figure out the size of the license acquisition
	// challenge to be returned.
	//cout << "aampGenerateKeyRequest :: going for playeady generate request" << endl;
	dr = Drm_LicenseAcq_GenerateChallenge(m_poAppContext, g_rgpdstrRights,
			sizeof(g_rgpdstrRights) / sizeof(DRM_CONST_STRING *),
			NULL,
			NULL,
			0,
			NULL, &cchSilentURL,
			NULL,
			NULL,
			NULL, &m_cbChallenge);

	if (dr == DRM_E_BUFFERTOOSMALL)
	{
		if (cchSilentURL > 0)
		{
			ChkMem(
					m_pchSilentURL = (DRM_CHAR *) Oem_MemAlloc(
							cchSilentURL + 1));
			ZEROMEM(m_pchSilentURL, cchSilentURL + 1);
		}

		// Allocate buffer that is sufficient to store the license acquisition
		// challenge.
		if (m_cbChallenge > 0)
			ChkMem(m_pbChallenge = (DRM_BYTE *) Oem_MemAlloc(m_cbChallenge));

		dr = DRM_SUCCESS;
	} else
	{
		ChkDR(dr);
#ifdef TRACE_LOG
		cout << "aampGenerateKeyRequest :: Playready challenge generated" << endl;
#endif
	}
	// Supply a buffer to receive the license acquisition challenge.
	ChkDR(
			Drm_LicenseAcq_GenerateChallenge(m_poAppContext, g_rgpdstrRights,
					sizeof(g_rgpdstrRights) / sizeof(DRM_CONST_STRING *),
					NULL,
					NULL,
					0,
					m_pchSilentURL, &cchSilentURL,
					NULL,
					NULL, m_pbChallenge, &m_cbChallenge));
#ifdef TRACE_LOG
	logprintf("aampGenerateKeyRequest :: Playready destination URL : %s \n\n", m_pchSilentURL);
#endif
	m_eKeyState = KEY_PENDING;

	result = new DrmData(m_pbChallenge, m_cbChallenge);
	destinationURL = static_cast<string>(m_pchSilentURL);
	return result;

	ErrorExit:
		if (DRM_FAILED(dr))
		{
			logprintf("aampGenerateKeyRequest :: Playread DRM key request generation failed error code : %08x \n", dr);
			m_eKeyState = KEY_ERROR;
		}
	return NULL;
}

/**
 * @brief Updates the received key to DRM session
 * @param key : License key from license server.
 * @retval DRM_SUCCESS if no errors encountered
 */
int PlayReadyDRMSession::aampDRMProcessKey(DrmData* key)
{
#ifdef TRACE_LOG
	cout << "aampDRMProcessKey :: Playready Update" << endl;
#endif
	DRM_RESULT dr = DRM_SUCCESS;
	DRM_LICENSE_RESPONSE oLicenseResponse =
	{ eUnknownProtocol, 0 };

	//cout << "aampDRMProcessKey :: Playready key state check if pending" << endl;
	// The current state MUST be KEY_PENDING otherwise error out.
	ChkBOOL(m_eKeyState == KEY_PENDING, DRM_E_INVALIDARG);
#ifdef TRACE_LOG
	cout << "aampDRMProcessKey :: Playready check if msg is null" << endl;
#endif
	ChkArg(key->getData() && key->getDataLength() > 0);

	//cout << "PlayreadyProcessResponse" << endl;
	dr = Drm_LicenseAcq_ProcessResponse(m_poAppContext,
					DRM_PROCESS_LIC_RESPONSE_SIGNATURE_NOT_REQUIRED,
					NULL,
					NULL, const_cast<DRM_BYTE *>(key->getData()),
					key->getDataLength(), &oLicenseResponse);
#ifdef TRACE_LOG
	printf("aampDRMProcessKey :: Drm_LicenseAcq_ProcessResponse result : %08x \n", dr);
#endif
	ChkDR(dr);


	dr = Drm_Reader_Bind(m_poAppContext, g_rgpdstrRights,
					NO_OF(g_rgpdstrRights), m_pOutputProtection->PR_OP_Callback,
					m_pOutputProtection->getPlayReadyLevels(), &m_oDecryptContext);
#ifdef TRACE_LOG
	logprintf("aampDRMProcessKey :: Printing bind result : %08x \n", dr);
#endif
    ChkDR(dr);
	m_eKeyState = KEY_READY;

	logprintf("aampDRMProcessKey :: Key processed, now ready for content decryption\n");

	return 1;

	ErrorExit: if (DRM_FAILED(dr))
	{
		logprintf("aampDRMProcessKey :: Playready failed processing license response : error code : %08x \n", dr);
		m_eKeyState = KEY_ERROR;
	}
	return 0;

}

/**
 * @brief Function to decrypt stream  buffer.
 * @param f_pbIV : Initialization vector.
 * @param f_cbIV : Initialization vector length.
 * @param payloadData : Data to decrypt.
 * @param payloadDataSize : Size of data.
 * @param ppOpaqueData : pointer to opaque buffer in case of SVP.
 * @retval Returns 1 on success 0 on failure.
 */
int PlayReadyDRMSession::decrypt(const uint8_t *f_pbIV, uint32_t f_cbIV,
		const uint8_t *payloadData, uint32_t payloadDataSize, uint8_t **ppOpaqueData)
{

	int status = 1;
	DRM_AES_COUNTER_MODE_CONTEXT oAESContext =
	{ 0 };
	DRM_RESULT dr = DRM_SUCCESS;
    DRM_RESULT err = DRM_SUCCESS;
#ifdef TRACE_LOG
	cout << "PR decrypt :: Playready Decrypt invoked" << endl;
#endif
	uint8_t *ivData = (uint8_t *) f_pbIV;
	uint8_t temp;

    // Verify output protection parameters
    if(m_pOutputProtection->IsSourceUHD()) {
        // Source material is UHD
        if(!m_pOutputProtection->isHDCPConnection2_2()) {
            // UHD and not HDCP 2.2
            logprintf("%s : UHD source but not HDCP 2.2. FAILING decrypt\n", __FUNCTION__);
            return HDCP_AUTHENTICATION_FAILURE;
        }
    }

	pthread_mutex_lock(&decryptMutex);
	ChkDR(Drm_Reader_InitDecrypt(&m_oDecryptContext, NULL, 0));
	//cout << "PR decrypt :: Playready Decrypt swap IV" << endl;
	// FIXME: IV bytes need to be swapped ???
	for (uint32_t i = 0; i < f_cbIV / 2; i++)
	{
		temp = ivData[i];
		ivData[i] = ivData[f_cbIV - i - 1];
		ivData[f_cbIV - i - 1] = temp;
	}

	MEMCPY(&oAESContext.qwInitializationVector, ivData, f_cbIV);

#ifdef USE_SAGE_SVP
    // Make this a run time decision.
	{
#ifdef TRACE_LOG
	    printf("%s data len = %d\n", __FUNCTION__, payloadDataSize);
#endif
	    OEM_OPAQUE_BUFFER_HANDLE hOpaqueBufferIn;
        OEM_OPAQUE_BUFFER_HANDLE hOpaqueBufferOut;
        OEM_HAL_OpaqueBufferCreateWithPointer((DRM_BYTE *)payloadData, payloadDataSize, &hOpaqueBufferIn);
        OEM_HAL_OpaqueBufferCreate(&hOpaqueBufferOut);

        err = Drm_Reader_DecryptOpaque(&m_oDecryptContext,
                                       &oAESContext,
                                       hOpaqueBufferIn,
                                       hOpaqueBufferOut,
                                       payloadDataSize);
        // Assign the opaque pointer from the decryptor.
        DRM_RESULT errHandle = OEM_HAL_OpaqueBufferGetDataHandle(hOpaqueBufferOut, ppOpaqueData);
        if (DRM_FAILED(errHandle)) {
            logprintf("AampDRMSession::decrypt --> OEM_HAL_OpaqueBufferGetDataHandle FAILED errHandle = %X, opaqueData = %p\n",
                    errHandle, *ppOpaqueData);
        }

        OEM_HAL_OpaqueBufferDestroy(&hOpaqueBufferIn);
        OEM_HAL_OpaqueBufferDestroy(&hOpaqueBufferOut);     // Will not delete the data handle
    }
#else
	{
	    err = Drm_Reader_Decrypt(&m_oDecryptContext, &oAESContext,
                (DRM_BYTE *) payloadData, payloadDataSize);
	    *ppOpaqueData = NULL;
    }
#endif
	ChkDR(err);

	//cout << "PR decrypt :: Playready Decrypt commit" << endl;
	// Call commit during the decryption of the first sample.
	if (!m_fCommit)
	{
		ChkDR(Drm_Reader_Commit(m_poAppContext, m_pOutputProtection->PR_OP_Callback, m_pOutputProtection->getPlayReadyLevels() ));
		m_fCommit = TRUE;
	}
	pthread_mutex_unlock(&decryptMutex);
#ifdef TRACE_LOG
	cout << "Playready Decrypt return clear content" << endl;
#endif
	// Return clear content.
	status = 0;

	return status;

	ErrorExit:
		pthread_mutex_unlock(&decryptMutex);
		logprintf("PR decrypt :: Play ready session : Decrypt Error\n");
		return status;

}


/**
 * @brief Get the current state of DRM Session.
 * @retval KeyState
 */
KeyState PlayReadyDRMSession::getState()
{
	return m_eKeyState;
}

/**
 * @brief Clear the current session context
 *        So that new init data can be bound.
 */
void PlayReadyDRMSession:: clearDecryptContext()
{
	Drm_Reader_Close(&m_oDecryptContext);
	Drm_Reinitialize(m_poAppContext);
	SAFE_OEM_FREE(m_pbChallenge);
	SAFE_OEM_FREE(m_pchSilentURL);
	m_pbChallenge = NULL;
	m_pchSilentURL = NULL;
	m_fCommit = false;
	m_cbChallenge = 0;
	if(m_pbPRO != NULL)
	{
		SAFE_OEM_FREE(m_pbPRO);
	}
	m_pbPRO = NULL;
	m_cbPRO = 0;
	m_eKeyState = KEY_INIT;
}
