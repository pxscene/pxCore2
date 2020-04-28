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
* @file AampDRMSessionManager.h
* @brief Header file for DRM session manager
*/
#ifndef AampDRMSessionManager_h
#define AampDRMSessionManager_h

#include "aampdrmsessionfactory.h"
#include "AampDrmSession.h"
#include "AampDRMutils.h"
#include "main_aamp.h"
#include <string>
#include <curl/curl.h>

#ifdef USE_SECCLIENT
#include "sec_client.h"
#endif

#define VIDEO_SESSION 0
#define AUDIO_SESSION 1

/**
 *  @struct	DrmSessionDataInfo
 *  @brief	Drm Session Data Information 
 * for storing in a pool from parser.
 */
typedef struct DrmSessionDataInfo{
	struct DrmSessionParams* sessionData; /**< DRM Session Data */
	bool isProcessedLicenseAcquire; /**< Flag to avoid multiple acquire for a key */
	DRMSystems drmType; /**< DRM Type */
	unsigned char *processedKeyId; /**< Pointer to store last processed key Id */
	int processedKeyIdLen; /**< Last processed key Id size */
}DrmSessionDataInfo;

/**
 *  @struct	DrmSessionContext
 *  @brief	To store drmSession and keyId data.
 */
struct DrmSessionContext
{
	size_t dataLength;
	unsigned char* data;
	pthread_mutex_t sessionMutex;
	AampDrmSession * drmSession;

	DrmSessionContext() : dataLength(0), data(NULL), sessionMutex(PTHREAD_MUTEX_INITIALIZER), drmSession(NULL)
	{
	}
};

/**
 * @struct DrmSessionParams
 * @brief Holds data regarding drm session
 */
struct DrmSessionParams
{
	unsigned char *initData;
	int initDataLen;
	MediaType stream_type;
	PrivateInstanceAAMP *aamp;
	DRMSystems drmType;
	unsigned char *contentMetadata;
};

/**
 *  @struct	KeyID
 *  @brief	Structure to hold, keyId and session creation time for
 *  		keyId
 */
struct KeyID
{
	size_t len;
	unsigned char* data;
	long long creationTime;
	bool isFailedKeyId;
	bool isPrimaryKeyId;

	KeyID();
};

/**
 *  @brief	Enum to represent session manager state.
 *  		Session manager would abort any createDrmSession
 *  		request if in eSESSIONMGR_INACTIVE state.
 */
typedef enum{
	eSESSIONMGR_INACTIVE,
	eSESSIONMGR_ACTIVE
}SessionMgrState;

/**
 *  @class	AampDRMSessionManager
 *  @brief	Controller for managing DRM sessions.
 */
class AampDRMSessionManager
{

private:
	DrmSessionContext *drmSessionContexts;
	KeyID *cachedKeyIDs;
	char* accessToken;
	int accessTokenLen;
	SessionMgrState sessionMgrState;
	pthread_mutex_t accessTokenMutex;
	pthread_mutex_t cachedKeyMutex;
	bool curlSessionAbort;

	AampDRMSessionManager(const AampDRMSessionManager &) = delete;
	AampDRMSessionManager& operator=(const AampDRMSessionManager &) = delete;

	static size_t write_callback(char *ptr, size_t size, size_t nmemb,
			void *userdata);
	static int progress_callback(void *clientp,	double dltotal, 
			double dlnow, double ultotal, double ulnow );
public:

	AampDRMSessionManager();

	void initializeDrmSessions();

	~AampDRMSessionManager();

	AampDrmSession * createDrmSession(const char* systemId,
			const unsigned char * initDataPtr, uint16_t dataLength, MediaType streamType, PrivateInstanceAAMP* aamp, AAMPEvent *e);

	AampDrmSession * createDrmSession(const char* systemId,
			const unsigned char * initDataPtr, uint16_t dataLength, MediaType streamType,
			const unsigned char *contentMetadata, PrivateInstanceAAMP* aamp, AAMPEvent *e,bool isPrimarySession = false);

	DrmData * getLicense(DrmData * keyChallenge, string destinationURL, long *httpError, bool isComcastStream = false, char* licenseProxy = NULL, struct curl_slist *customHeader = NULL, DRMSystems drmSystem = eDRM_NONE);

	void clearSessionData();

	void clearAccessToken();

	void clearFailedKeyIds();

	void setSessionMgrState(SessionMgrState state);
	
	void setCurlAbort(bool isAbort);

	bool getCurlAbort(void);

	const char* getAccessToken(int &tokenLength, long &error_code);
};

typedef struct writeCallbackData{
	DrmData *data ;
	AampDRMSessionManager* mDRMSessionManager;
}writeCallbackData;

#endif
