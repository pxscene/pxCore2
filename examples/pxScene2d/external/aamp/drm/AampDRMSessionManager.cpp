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
* @file AampDRMSessionManager.cpp
* @brief Source file for DrmSessionManager.
*/

#include "AampDRMSessionManager.h"
#include "priv_aamp.h"
#include <pthread.h>
#include "_base64.h"
#include <iostream>
//#define LOG_TRACE 1
#define COMCAST_LICENCE_REQUEST_HEADER_ACCEPT "Accept: application/vnd.xcal.mds.licenseResponse+json; version=1"
#define COMCAST_LICENCE_REQUEST_HEADER_CONTENT_TYPE "Content-Type: application/vnd.xcal.mds.licenseRequest+json; version=1"
#define LICENCE_RESPONSE_JSON_LICENCE_KEY "license\":\""
#ifdef USE_SECCLIENT
#define COMCAST_QA_DRM_LICENCE_SERVER_URL "mds-qa.ccp.xcal.tv"
#define COMCAST_DRM_LICENCE_SERVER_URL "mds.ccp.xcal.tv"
#define COMCAST_ROGERS_DRM_LICENCE_SERVER_URL "mds-rogers.ccp.xcal.tv"
#else
#define COMCAST_QA_DRM_LICENCE_SERVER_URL "https://mds-qa.ccp.xcal.tv/license"
#define COMCAST_DRM_LICENCE_SERVER_URL "https://mds.ccp.xcal.tv/license"
#define COMCAST_ROGERS_DRM_LICENCE_SERVER_URL "https://mds-rogers.ccp.xcal.tv/license"
#endif
#define COMCAST_DRM_METADATA_TAG_START "<ckm:policy xmlns:ckm=\"urn:ccp:ckm\">"
#define COMCAST_DRM_METADATA_TAG_END "</ckm:policy>"
#define SESSION_TOKEN_URL "http://localhost:50050/authService/getSessionToken"
#define MAX_LICENSE_REQUEST_ATTEMPTS 2

static const char *sessionTypeName[] = {"video", "audio"};

static pthread_mutex_t drmSessionMutex = PTHREAD_MUTEX_INITIALIZER;

KeyID::KeyID() : len(0), data(NULL), creationTime(0), isFailedKeyId(false), isPrimaryKeyId(false)
{
}


void *CreateDRMSession(void *arg);
int SpawnDRMLicenseAcquireThread(PrivateInstanceAAMP *aamp, DrmSessionDataInfo* drmData);
void ReleaseDRMLicenseAcquireThread(PrivateInstanceAAMP *aamp);

#ifdef USE_SECCLIENT
/**
 *  @brief Get formatted URL of license server
 *
 *  @param[in] url URL of license server
 *  @return		formatted url for secclient license acqusition.
 */
static string getFormattedLicenseServerURL(string url)
{
	size_t startpos = 0;
	size_t endpos, len;
	endpos = len = url.size();

	if (memcmp(url.data(), "https://", 8) == 0)
	{
		startpos += 8;
	}
	else if (memcmp(url.data(), "http://", 7) == 0)
	{
		startpos += 7;
	}

	if (startpos != 0)
	{
		endpos = url.find('/', startpos);
		if (endpos != string::npos)
		{
			len = endpos - startpos;
		}
	}

	return url.substr(startpos, len);
}
#endif

/**
 *  @brief      AampDRMSessionManager constructor.
 */
AampDRMSessionManager::AampDRMSessionManager() : drmSessionContexts(new DrmSessionContext[gpGlobalConfig->dash_MaxDRMSessions]),
		cachedKeyIDs(new KeyID[gpGlobalConfig->dash_MaxDRMSessions]), accessToken(NULL),
		accessTokenLen(0), sessionMgrState(SessionMgrState::eSESSIONMGR_ACTIVE), accessTokenMutex(PTHREAD_MUTEX_INITIALIZER),
		cachedKeyMutex(PTHREAD_MUTEX_INITIALIZER)
		,curlSessionAbort(false)
{
}

/**
 *  @brief      AampDRMSessionManager Destructor.
 */
AampDRMSessionManager::~AampDRMSessionManager()
{
	clearAccessToken();
	clearSessionData();
}

/**
 *  @brief		Clean up the memory used by session variables.
 *
 *  @return		void.
 */
void AampDRMSessionManager::clearSessionData()
{
	logprintf("%s:%d AampDRMSessionManager:: Clearing session data", __FUNCTION__, __LINE__);
	for(int i = 0 ; i < gpGlobalConfig->dash_MaxDRMSessions; i++)
	{
		if(drmSessionContexts != NULL && drmSessionContexts[i].drmSession != NULL)
		{
			delete drmSessionContexts[i].data;
			drmSessionContexts[i].data = NULL;
			drmSessionContexts[i].dataLength = 0;
			delete drmSessionContexts[i].drmSession;
			drmSessionContexts[i].drmSession = NULL;
		}
		if(cachedKeyIDs != NULL && cachedKeyIDs[i].data != NULL)
		{
			delete cachedKeyIDs[i].data;
			cachedKeyIDs[i].data = NULL;
			cachedKeyIDs[i].len = 0;
		}
	}

	if (drmSessionContexts != NULL)
	{
		delete[] drmSessionContexts;
		drmSessionContexts = NULL;
	}
	if (cachedKeyIDs != NULL)
	{
		delete[] cachedKeyIDs;
		cachedKeyIDs = NULL;
	}
}

/**
 * @brief	Set Session manager state
 * @param	state
 * @return	void.
 */
void AampDRMSessionManager::setSessionMgrState(SessionMgrState state)
{
	sessionMgrState = state;
}

/**
 * @brief	Set Session abort flag
 * @param	bool flag
 * @return	void.
 */
void AampDRMSessionManager::setCurlAbort(bool isAbort){
	curlSessionAbort = isAbort;
}

/**
 * @brief	Get Session abort flag
 * @param	void
 * @return	bool flag.
 */
bool AampDRMSessionManager::getCurlAbort(){
	return curlSessionAbort;
}
/**
 * @brief	Clean up the failed keyIds.
 *
 * @return	void.
 */
void AampDRMSessionManager::clearFailedKeyIds()
{
	pthread_mutex_lock(&cachedKeyMutex);
	for(int i = 0 ; i < gpGlobalConfig->dash_MaxDRMSessions; i++)
	{
		if(cachedKeyIDs[i].data != NULL && cachedKeyIDs[i].isFailedKeyId)
		{
			delete cachedKeyIDs[i].data;
			cachedKeyIDs[i].data = NULL;
			cachedKeyIDs[i].len = 0;
			cachedKeyIDs[i].isFailedKeyId = false;
			cachedKeyIDs[i].creationTime = 0;
		}
		cachedKeyIDs[i].isPrimaryKeyId = false;
	}
	pthread_mutex_unlock(&cachedKeyMutex);
}

/**
 *  @brief		Clean up the memory for accessToken.
 *
 *  @return		void.
 */
void AampDRMSessionManager::clearAccessToken()
{
	if(accessToken)
	{
		free(accessToken);
		accessToken = NULL;
		accessTokenLen = 0;
	}
}

/**
 * @brief
 * @param clientp app-specific as optionally set with CURLOPT_PROGRESSDATA
 * @param dltotal total bytes expected to download
 * @param dlnow downloaded bytes so far
 * @param ultotal total bytes expected to upload
 * @param ulnow uploaded bytes so far
 * @retval
 */
int AampDRMSessionManager::progress_callback(
	void *clientp, // app-specific as optionally set with CURLOPT_PROGRESSDATA
	double dltotal, // total bytes expected to download
	double dlnow, // downloaded bytes so far
	double ultotal, // total bytes expected to upload
	double ulnow // uploaded bytes so far
	)
{
	int returnCode = 0 ;
	AampDRMSessionManager *drmSessionManager = (AampDRMSessionManager *)clientp;
	if(drmSessionManager->getCurlAbort())
	{
		logprintf("Aborting DRM curl operation.. - CURLE_ABORTED_BY_CALLBACK");
		returnCode = -1; // Return non-zero for CURLE_ABORTED_BY_CALLBACK
		//Reset the abort variable
		drmSessionManager->setCurlAbort(false);
	}

	return returnCode;
}

/**
 *  @brief		Curl write callback, used to get the curl o/p
 *  			from DRM license, accessToken curl requests.
 *
 *  @param[in]	ptr - Pointer to received data.
 *  @param[in]	size, nmemb - Size of received data (size * nmemb).
 *  @param[out]	userdata - Pointer to buffer where the received data is copied.
 *  @return		returns the number of bytes processed.
 */
size_t AampDRMSessionManager::write_callback(char *ptr, size_t size,
		size_t nmemb, void *userdata)
{
	writeCallbackData *callbackData = (writeCallbackData*)userdata;
        DrmData *data = (DrmData *)callbackData->data;
        size_t numBytesForBlock = size * nmemb;
        if(callbackData->mDRMSessionManager->getCurlAbort())
        {
                logprintf("Aborting DRM curl operation.. - CURLE_ABORTED_BY_CALLBACK");
                numBytesForBlock = 0; // Will cause CURLE_WRITE_ERROR
		//Reset the abort variable
		callbackData->mDRMSessionManager->setCurlAbort(false);

        }
        else if (NULL == data->getData())        
        {
		data->setData((unsigned char *) ptr, numBytesForBlock);
	}
	else
	{
		data->addData((unsigned char *) ptr, numBytesForBlock);
	}
	if (gpGlobalConfig->logging.trace)
	{
		logprintf("%s:%d wrote %zu number of blocks", __FUNCTION__, __LINE__, numBytesForBlock);
	}
	return numBytesForBlock;
}

/**
 *  @brief		Extract substring between (excluding) two string delimiters.
 *
 *  @param[in]	parentStr - Parent string from which substring is extracted.
 *  @param[in]	startStr, endStr - String delimiters.
 *  @return		Returns the extracted substring; Empty string if delimiters not found.
 */
string _extractSubstring(string parentStr, string startStr, string endStr)
{
	string ret = "";
	int startPos = parentStr.find(startStr);
	if(string::npos != startPos)
	{
		int offset = strlen(startStr.c_str());
		int endPos = parentStr.find(endStr, startPos + offset + 1);
		if(string::npos != endPos)
		{
			ret = parentStr.substr(startPos + offset, endPos - (startPos + offset));
		}
	}
	return ret;
}

/**
 *  @brief		Get the accessToken from authService.
 *
 *  @param[out]	tokenLen - Gets updated with accessToken length.
 *  @return		Pointer to accessToken.
 *  @note		AccessToken memory is dynamically allocated, deallocation
 *				should be handled at the caller side.
 */
const char * AampDRMSessionManager::getAccessToken(int &tokenLen, long &error_code)
{
	if(accessToken == NULL)
	{
		DrmData * tokenReply = new DrmData();
		writeCallbackData *callbackData = new writeCallbackData();
		callbackData->data = tokenReply;
		callbackData->mDRMSessionManager = this;
		CURLcode res;
		long httpCode = -1;

		CURL *curl = curl_easy_init();;
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
		curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, DEFAULT_CURL_TIMEOUT);
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, callbackData);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_URL, SESSION_TOKEN_URL);

		res = curl_easy_perform(curl);

		if (res == CURLE_OK)
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
			if (httpCode == 200 || httpCode == 206)
			{
				string tokenReplyStr = string(reinterpret_cast<char*>(tokenReply->getData()));
				string tokenStatusCode = _extractSubstring(tokenReplyStr, "status\":", ",\"");
				if(tokenStatusCode.length() == 0)
				{
					//StatusCode could be last element in the json
					tokenStatusCode = _extractSubstring(tokenReplyStr, "status\":", "}");
				}
				if(tokenStatusCode.length() == 1 && tokenStatusCode.c_str()[0] == '0')
				{
					string token = _extractSubstring(tokenReplyStr, "token\":\"", "\"");
					if(token.length() != 0)
					{
						accessToken = (char*)calloc(token.length()+1, sizeof(char));
						accessTokenLen = token.length();
						strncpy(accessToken,token.c_str(),token.length());
						logprintf("%s:%d Received session token from auth service ", __FUNCTION__, __LINE__);
					}
					else
					{
						logprintf("%s:%d Could not get access token from session token reply", __FUNCTION__, __LINE__);
						error_code = (long)eAUTHTOKEN_TOKEN_PARSE_ERROR;
					}
				}
				else
				{
					logprintf("%s:%d Missing or invalid status code in session token reply", __FUNCTION__, __LINE__);
					error_code = (long)eAUTHTOKEN_INVALID_STATUS_CODE;
				}
			}
			else
			{
				logprintf("%s:%d Get Session token call failed with http error %d", __FUNCTION__, __LINE__, httpCode);
				error_code = httpCode;
			}
		}
		else
		{
			logprintf("%s:%d Get Session token call failed with curl error %d", __FUNCTION__, __LINE__, res);
			error_code = res;
		}
		delete tokenReply;
                delete callbackData;
		curl_easy_cleanup(curl);
	}

	tokenLen = accessTokenLen;
	return accessToken;
}

/**
 * @brief Sleep for given milliseconds
 * @param milliseconds Time to sleep
 */
static void mssleep(int milliseconds)
{
	struct timespec req, rem;
	if (milliseconds > 0)
	{
		req.tv_sec = milliseconds / 1000;
		req.tv_nsec = (milliseconds % 1000) * 1000000;
		nanosleep(&req, &rem);
	}
}

/**
 *  @brief		Get DRM license key from DRM server.
 *
 *  @param[in]	keyChallenge - Structure holding license request and it's length.
 *  @param[in]	destinationURL - Destination url to which request is send.
 *  @param[out]	httpCode - Gets updated with http error; default -1.
 *  @param[in]	isComcastStream - Flag to indicate whether Comcast specific headers
 *  			are to be used.
 *  @param[in]	licenseProxy - Proxy to use for license requests.
 *  @param[in]	headers - Custom headers from application for license request.
 *  @param[in]	drmSystem - DRM type.
 *  @return		Structure holding DRM license key and it's length; NULL and 0 if request fails
 *  @note		Memory for license key is dynamically allocated, deallocation
 *				should be handled at the caller side.
 *			customHeader ownership should be taken up by getLicense function
 *
 */
DrmData * AampDRMSessionManager::getLicense(DrmData * keyChallenge,
		string destinationURL, long *httpCode, bool isComcastStream, char* licenseProxy, struct curl_slist *customHeader, DRMSystems drmSystem)
{

	*httpCode = -1;
	CURL *curl;
	CURLcode res;
	double totalTime = 0;
	struct curl_slist *headers = NULL;
	DrmData * keyInfo = new DrmData();
	writeCallbackData *callbackData = new writeCallbackData();
	callbackData->data = keyInfo;
	callbackData->mDRMSessionManager = this;
	const long challegeLength = keyChallenge->getDataLength();
	char* destURL = new char[destinationURL.length() + 1];
	long long downloadTimeMS = 0;
	curl = curl_easy_init();
	if (customHeader != NULL)
	{
		headers = customHeader;
	}

	if(isComcastStream)
	{
		headers = curl_slist_append(headers, COMCAST_LICENCE_REQUEST_HEADER_ACCEPT);
		headers = curl_slist_append(headers, COMCAST_LICENCE_REQUEST_HEADER_CONTENT_TYPE);
		headers = curl_slist_append(headers, "Expect:");
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "AAMP/1.0.0");
	//	headers = curl_slist_append(headers, "X-MoneyTrace: trace-id=226c94fc4d-3535-4945-a173-61af53444a3d;parent-id=4557953636469444377;span-id=803972323171353973");
	}
	else if(customHeader == NULL)
	{
		if(drmSystem == eDRM_WideVine)
		{
			AAMPLOG_WARN("No custom header, setting default for Widevine");
			headers = curl_slist_append(headers,"Content-Type: application/octet-stream");
		}
		else if (drmSystem == eDRM_PlayReady)
		{
			AAMPLOG_WARN("No custom header, setting default for Playready");
			headers = curl_slist_append(headers,"Content-Type: text/xml; charset=utf-8");
		}
		else
		{
			AAMPLOG_WARN("!!! Custom header is missing and default is not processed.");
		}
	}

	strcpy((char*) destURL, destinationURL.c_str());

	//headers = curl_slist_append(headers, destURL);

	logprintf("%s:%d Sending license request to server : %s ", __FUNCTION__, __LINE__, destinationURL.c_str());
	//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progress_callback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
	curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, destURL);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, callbackData);
	curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, challegeLength);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,(uint8_t * )keyChallenge->getData());
	if (licenseProxy)
	{
		curl_easy_setopt(curl, CURLOPT_PROXY, licenseProxy);
		/* allow whatever auth the proxy speaks */
		curl_easy_setopt(curl, CURLOPT_PROXYAUTH, CURLAUTH_ANY);
	}
	unsigned int attemptCount = 0;
	bool requestFailed = true;
	while(attemptCount < MAX_LICENSE_REQUEST_ATTEMPTS)
	{
		bool loopAgain = false;
		attemptCount++;
		long long tStartTime = NOW_STEADY_TS_MS;
		res = curl_easy_perform(curl);
		long long tEndTime = NOW_STEADY_TS_MS;
		downloadTimeMS = tEndTime - tStartTime;
		if (res != CURLE_OK)
		{
			// To avoid scary logging
			if (res != CURLE_ABORTED_BY_CALLBACK && res != CURLE_WRITE_ERROR)
			{
				if (res == CURLE_OPERATION_TIMEDOUT || res == CURLE_COULDNT_CONNECT)
				{
					// Retry for curl 28 and curl 7 errors.
					loopAgain = true;

					delete keyInfo;
					delete callbackData;
					keyInfo = new DrmData();
					callbackData = new writeCallbackData();
					callbackData->data = keyInfo;
					callbackData->mDRMSessionManager = this;
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, callbackData);
				}
				logprintf("%s:%d curl_easy_perform() failed: %s", __FUNCTION__, __LINE__, curl_easy_strerror(res));
				logprintf("%s:%d acquireLicense FAILED! license request attempt : %d; response code : curl %d", __FUNCTION__, __LINE__, attemptCount, res);
			}
			*httpCode = res;
		}
		else
		{
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, httpCode);
			curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &totalTime);
			if (*httpCode != 200 && *httpCode != 206)
			{
				logprintf("%s:%d acquireLicense FAILED! license request attempt : %d; response code : http %d", __FUNCTION__, __LINE__, attemptCount, *httpCode);
				if(*httpCode >= 500 && *httpCode < 600
						&& attemptCount < MAX_LICENSE_REQUEST_ATTEMPTS && gpGlobalConfig->licenseRetryWaitTime > 0)
				{
					delete keyInfo;
					delete callbackData;
					keyInfo = new DrmData();
					callbackData = new writeCallbackData();
					callbackData->data = keyInfo;
					callbackData->mDRMSessionManager = this;
					curl_easy_setopt(curl, CURLOPT_WRITEDATA, callbackData);
					logprintf("%s:%d acquireLicense : Sleeping %d milliseconds before next retry.", __FUNCTION__, __LINE__, gpGlobalConfig->licenseRetryWaitTime);
					mssleep(gpGlobalConfig->licenseRetryWaitTime);
				}
			}
			else
			{
				logprintf("%s:%d DRM Session Manager Received license data from server; Curl total time  = %.1f", __FUNCTION__, __LINE__, totalTime);
				logprintf("%s:%d acquireLicense SUCCESS! license request attempt %d; response code : http %d",__FUNCTION__, __LINE__, attemptCount, *httpCode);
				requestFailed = false;
			}
		}

		double total, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize;
		long reqSize;
		double totalPerformRequest = (double)(downloadTimeMS)/1000;

		curl_easy_getinfo(curl, CURLINFO_NAMELOOKUP_TIME, &resolve);
		curl_easy_getinfo(curl, CURLINFO_CONNECT_TIME, &connect);
		curl_easy_getinfo(curl, CURLINFO_APPCONNECT_TIME, &appConnect);
		curl_easy_getinfo(curl, CURLINFO_PRETRANSFER_TIME, &preTransfer);
		curl_easy_getinfo(curl, CURLINFO_STARTTRANSFER_TIME, &startTransfer);
		curl_easy_getinfo(curl, CURLINFO_REDIRECT_TIME, &redirect);
		curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &dlSize);
		curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &reqSize);

		AAMPLOG(eLOGLEVEL_WARN, "HttpLicenseRequestEnd: {\"license_url\":\"%.500s\",\"curlTime\":%2.4f,\"times\":{\"total\":%2.4f,\"connect\":%2.4f,\"startTransfer\":%2.4f,\"resolve\":%2.4f,\"appConnect\":%2.4f,\"preTransfer\":%2.4f,\"redirect\":%2.4f,\"dlSz\":%g,\"ulSz\":%ld},\"responseCode\":%ld}",
				destinationURL.c_str(),
				totalPerformRequest,
				totalTime, connect, startTransfer, resolve, appConnect, preTransfer, redirect, dlSize, reqSize, *httpCode);

		if(!loopAgain)
			break;
	}

	if(requestFailed && keyInfo != NULL)
	{
		delete keyInfo;
		keyInfo = NULL;
	}

	delete destURL;
	delete callbackData;
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	return keyInfo;
}

/**
 *  @brief		Overloaded version of createDrmSession where contentMetadataPtr is not there
 *  			Called from gstaampopencdmiplugins.
 *
 *  @param[in]	systemId - UUID of the DRM system.
 *  @param[in]	initDataPtr - Pointer to PSSH data.
 *  @param[in]	dataLength - Length of PSSH data.
 *  @param[in]	streamType - Whether audio or video.
 *  @param[in]	aamp - Pointer to PrivateInstanceAAMP, for DRM related profiling.
 *  @param[out]	error_code - Gets updated with proper error code, if session creation fails.
 *  			No NULL checks are done for error_code, caller should pass a valid pointer.
 *  @return		Pointer to DrmSession for the given PSSH data; NULL if session creation/mapping fails.
 */
AampDrmSession * AampDRMSessionManager::createDrmSession(
		const char* systemId, const unsigned char * initDataPtr,
		uint16_t dataLength, MediaType streamType, PrivateInstanceAAMP* aamp,  AAMPEvent *e)
{
	return createDrmSession(systemId, initDataPtr, dataLength, streamType, NULL, aamp, e);
}

/**
 *  @brief		Creates and/or returns the DRM session corresponding to keyId (Present in initDataPtr)
 *  			AampDRMSession manager has two static AampDrmSession objects.
 *  			This method will return the existing DRM session pointer if any one of these static
 *  			DRM session objects are created against requested keyId. Binds the oldest DRM Session
 *  			with new keyId if no matching keyId is found in existing sessions.
 *
 *  @param[in]	systemId - UUID of the DRM system.
 *  @param[in]	initDataPtr - Pointer to PSSH data.
 *  @param[in]	dataLength - Length of PSSH data.
 *  @param[in]	streamType - Whether audio or video.
 *  @param[in]	contentMetadataPtr - Pointer to content meta data, when content meta data
 *  			is already extracted during manifest parsing. Used when content meta data
 *  			is available as part of another PSSH header, like Comcast DRM Agnostic PSSH
 *  			header.
 *  @param[in]	aamp - Pointer to PrivateInstanceAAMP, for DRM related profiling.
 *  @param[out]	error_code - Gets updated with proper error code, if session creation fails.
 *  			No NULL checks are done for error_code, caller should pass a valid pointer.
 *  @return		Pointer to DrmSession for the given PSSH data; NULL if session creation/mapping fails.
 */
AampDrmSession * AampDRMSessionManager::createDrmSession(
		const char* systemId, const unsigned char * initDataPtr,
		uint16_t dataLength, MediaType streamType,
		const unsigned char* contentMetadataPtr, PrivateInstanceAAMP* aamp, AAMPEvent *e, bool isPrimarySession)
{
	KeyState code = KEY_CLOSED;
	long responseCode = -1;
	unsigned char * contentMetaData = NULL;
	int contentMetaDataLen = 0;
	unsigned char *keyId = NULL;
	int keyIdLen = 0;
	string destinationURL;
	DrmData * key = NULL;
	bool keySlotFound = false;
	bool isCachedKeyId = false;
	int sessionSlot = 0;
	int processKeyRetValue = -1;
	if(gpGlobalConfig->logging.debug)
	{
		logprintf("%s:%d Received DRM Session request, Init Data length  : %d", __FUNCTION__, __LINE__,dataLength);
		logprintf("%s:%d Printing InitData from %s stream ", __FUNCTION__, __LINE__,systemId,sessionTypeName[streamType]);
		for (int i = 0; i < dataLength; ++i)
			cout <<(char)initDataPtr[i];
		cout << endl;
	}

	e->data.dash_drmmetadata.accessStatus = "accessAttributeStatus";
	e->data.dash_drmmetadata.accessStatus_value = 3;
	e->data.dash_drmmetadata.responseCode = 0;

	if(!(eMEDIATYPE_VIDEO == streamType || eMEDIATYPE_AUDIO == streamType))
	{
		e->data.dash_drmmetadata.failure = AAMP_TUNE_UNSUPPORTED_STREAM_TYPE;
		return NULL;
	}

	const char *keySystem = NULL;
	DRMSystems drmType = eDRM_NONE;
	if (!strncmp(systemId, PLAYREADY_PROTECTION_SYSTEM_ID, sizeof(PLAYREADY_PROTECTION_SYSTEM_ID)))
	{
		AAMPLOG_INFO("%s:%d [HHH]systemId is PLAYREADY", __FUNCTION__, __LINE__);
		drmType = eDRM_PlayReady;
#ifdef USE_SECCLIENT
		keySystem = SEC_CLIENT_PLAYREADY_KEYSYSTEMID;
#else
		keySystem = PLAYREADY_KEY_SYSTEM_STRING;
#endif
	}
	else if (!strncmp(systemId, WIDEVINE_PROTECTION_SYSTEM_ID, sizeof(WIDEVINE_PROTECTION_SYSTEM_ID)))
	{
#ifdef USE_SECCLIENT
		keySystem = SEC_CLIENT_WIDEVINE_KEYSYSTEMID;
#else
		keySystem = WIDEVINE_KEY_SYSTEM_STRING;
#endif
		AAMPLOG_INFO("%s:%d [HHH]systemId is Widevine", __FUNCTION__, __LINE__);
		drmType = eDRM_WideVine;
	}
	else if (!strncmp(systemId, CLEARKEY_PROTECTION_SYSTEM_ID, sizeof(CLEARKEY_PROTECTION_SYSTEM_ID)))
	{
		keySystem = CLEAR_KEY_SYSTEM_STRING;
		drmType = eDRM_ClearKey;
	}
	else
	{
		logprintf("Unsupported systemid: %s !", systemId);
	}
	logprintf("keysystem is %s", keySystem);

	keyId = aamp_ExtractKeyIdFromPssh(reinterpret_cast<const char*>(initDataPtr),dataLength, &keyIdLen, drmType);

	if (keyId == NULL)
	{
		logprintf("%s:%d Key Id not found in initdata", __FUNCTION__, __LINE__);
		e->data.dash_drmmetadata.failure = AAMP_TUNE_FAILED_TO_GET_KEYID;
		return NULL;
	}

	pthread_mutex_lock(&cachedKeyMutex);

	/*Find drmSession slot by going through cached keyIds */

	/* Check if requested keyId is already cached*/
	for (; sessionSlot < gpGlobalConfig->dash_MaxDRMSessions; sessionSlot++)
	{
		if (keyIdLen == cachedKeyIDs[sessionSlot].len && 0 == memcmp(cachedKeyIDs[sessionSlot].data, keyId, keyIdLen))
		{
			if(gpGlobalConfig->logging.debug)
			{
				logprintf("%s:%d  Session created/inprogress with same keyID %s at slot %d, can reuse same for %s",
							__FUNCTION__, __LINE__, keyId, sessionSlot, sessionTypeName[streamType]);
			}
			keySlotFound = true;
			isCachedKeyId = true;
			break;
		}
	}

	if(!keySlotFound)
	{
		/*Key Id not in cached list so we need to find out oldest slot to use;
		 * Oldest slot may be used by current playback which is marked primary
		 * Avoid selecting that slot*/

		/*First select the first slot that is not primary*/
		for (int iter = 0; iter < gpGlobalConfig->dash_MaxDRMSessions; iter++)
		{
			if(!cachedKeyIDs[iter].isPrimaryKeyId)
			{
				keySlotFound = true;
				sessionSlot = iter;
				break;
			}
		}

		/*Check if there's an older slot */
		for (int iter = sessionSlot + 1; iter < gpGlobalConfig->dash_MaxDRMSessions; iter++)
		{
			if(cachedKeyIDs[iter].creationTime < cachedKeyIDs[sessionSlot].creationTime)
			{
				sessionSlot = iter;
			}
		}

		if(keySlotFound)
		{
			if(gpGlobalConfig->logging.debug)
			{
				logprintf("%s:%d  Selected slot %d for keyId %s sessionType %s",
							__FUNCTION__, __LINE__, sessionSlot, keyId, sessionTypeName[streamType]);
			}
		}
		else
		{
			//@TODO It should not come to this unless license rotation is involved
			//add new error code for this
			logprintf("%s:%d  Unable to find keySlot for keyId %s sessionType %s, max sessions supported %d",
							__FUNCTION__, __LINE__, keyId, sessionTypeName[streamType], gpGlobalConfig->dash_MaxDRMSessions);
			free(keyId);
			keyId = NULL;
			pthread_mutex_unlock(&cachedKeyMutex);
			return NULL;
		}
	}

	if(!isCachedKeyId)
	{
		if(cachedKeyIDs[sessionSlot].data != NULL)
		{
			delete cachedKeyIDs[sessionSlot].data;
		}

		cachedKeyIDs[sessionSlot].len = keyIdLen;
		cachedKeyIDs[sessionSlot].isFailedKeyId = false;
		cachedKeyIDs[sessionSlot].data = new unsigned char[keyIdLen];
		memcpy(reinterpret_cast<void*>(cachedKeyIDs[sessionSlot].data),
									reinterpret_cast<const void*>(keyId), keyIdLen);
	}
	cachedKeyIDs[sessionSlot].creationTime = aamp_GetCurrentTimeMS();
	cachedKeyIDs[sessionSlot].isPrimaryKeyId = isPrimarySession;

	pthread_mutex_unlock(&cachedKeyMutex);


	pthread_mutex_lock(&(drmSessionContexts[sessionSlot].sessionMutex));
	aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_PREPROC);
	//logprintf("%s:%d Locked session mutex for %s", __FUNCTION__, __LINE__, sessionTypeName[sessionType]);
	if(drmSessionContexts[sessionSlot].drmSession == NULL)
	{
		drmSessionContexts[sessionSlot].drmSession = AampDrmSessionFactory::GetDrmSession(systemId);
	}
	else if(drmSessionContexts[sessionSlot].drmSession->getKeySystem() != string(keySystem))
	{
		AAMPLOG_WARN("%s:%d Switching DRM from %s to %s", __FUNCTION__, __LINE__, drmSessionContexts[sessionSlot].drmSession->getKeySystem().c_str(), keySystem);
		delete drmSessionContexts[sessionSlot].drmSession;
		drmSessionContexts[sessionSlot].drmSession = AampDrmSessionFactory::GetDrmSession(systemId);
	}
	else
	{
			if(keyIdLen == drmSessionContexts[sessionSlot].dataLength)
			{
				if ((0 == memcmp(drmSessionContexts[sessionSlot].data, keyId, keyIdLen))
						&& (drmSessionContexts[sessionSlot].drmSession->getState()
								== KEY_READY))
				{
					AAMPLOG_INFO("%s:%d Found drm session READY with same keyID %s - Reusing drm session for %s",
								__FUNCTION__, __LINE__, keyId, sessionTypeName[streamType]);
					pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
#if defined(USE_OPENCDM_ADAPTER)
					drmSessionContexts[sessionSlot].drmSession->setKeyId(reinterpret_cast<const char*>(keyId), keyIdLen);
#endif
					free(keyId);
					keyId = NULL;
					return drmSessionContexts[sessionSlot].drmSession;
				}
			}

			bool abortSessionRequest = false;
			if(SessionMgrState::eSESSIONMGR_INACTIVE == sessionMgrState && isCachedKeyId)
			{
				//This means that the previous session request for the same keyId already failed
				abortSessionRequest = true;
				AAMPLOG_INFO("%s:%d SessionManager state inactive, aborting request", __FUNCTION__, __LINE__);
			}

			if(!abortSessionRequest && isCachedKeyId && NULL == contentMetadataPtr)
			{
				AAMPLOG_INFO("%s:%d Aborting session creation for keyId %s: StreamType %s, since previous try failed",
								__FUNCTION__, __LINE__, keyId, sessionTypeName[streamType]);
				cachedKeyIDs[sessionSlot].isFailedKeyId = true;
				abortSessionRequest = true;
			}

			if(abortSessionRequest)
			{
				pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
				free(keyId);
				keyId = NULL;
				return NULL;
			}

			drmSessionContexts[sessionSlot].drmSession->clearDecryptContext();
	}

	if(drmSessionContexts[sessionSlot].drmSession)
	{
		code = drmSessionContexts[sessionSlot].drmSession->getState();
	}
	if (code != KEY_INIT)
	{
		logprintf("%s:%d DRM initialization failed : Key State %d ", __FUNCTION__, __LINE__, code);
		pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
		free(keyId);
		keyId = NULL;
		e->data.dash_drmmetadata.failure = AAMP_TUNE_DRM_INIT_FAILED;
		return NULL;
	}


	drmSessionContexts[sessionSlot].drmSession->generateAampDRMSession(initDataPtr, dataLength);
	code = drmSessionContexts[sessionSlot].drmSession->getState();
	if(code == KEY_ERROR_EMPTY_SESSION_ID)
	{
		AAMPLOG_ERR("%s:%d DRM session ID is empty: Key State %d ", __FUNCTION__, __LINE__, code);
		pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
		free(keyId);
		keyId = NULL;
		e->data.dash_drmmetadata.failure = AAMP_TUNE_DRM_SESSIONID_EMPTY;
		return NULL;
	}
	if (code != KEY_INIT)
	{
		logprintf("%s:%d DRM init data binding failed: Key State %d ", __FUNCTION__, __LINE__, code);
		pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
		free(keyId);
		keyId = NULL;
		e->data.dash_drmmetadata.failure = AAMP_TUNE_DRM_DATA_BIND_FAILED;
		return NULL;
	}

	DrmData * licenceChallenge = drmSessionContexts[sessionSlot].drmSession->aampGenerateKeyRequest(
			destinationURL);
	code = drmSessionContexts[sessionSlot].drmSession->getState();
	if (code == KEY_PENDING)
	{
		aamp->profiler.ProfileEnd(PROFILE_BUCKET_LA_PREPROC);
		//license request logic here
		if (gpGlobalConfig->logging.debug)
		{
			logprintf("%s:%d Licence challenge from DRM  : length = %d ",
						__FUNCTION__, __LINE__, licenceChallenge->getDataLength());
		}

#ifdef LOG_TRACE
		logprintf("\n\n%s:%d Licence challenge = \n", __FUNCTION__, __LINE__);
		unsigned char * data = licenceChallenge->getData();
		DumpBlob( data, licenceChallenge->getDataLength() );
		cout << endl;
#endif

		if (contentMetadataPtr)
		{
			contentMetaDataLen = strlen((const char*)contentMetadataPtr);
			contentMetaData = (unsigned char *)malloc(contentMetaDataLen + 1);
			memset(contentMetaData, 0, contentMetaDataLen + 1);
			strncpy(reinterpret_cast<char*>(contentMetaData), reinterpret_cast<const char*>(contentMetadataPtr), contentMetaDataLen);
			logprintf("%s:%d [HHH]contentMetaData length=%d", __FUNCTION__, __LINE__, contentMetaDataLen);
		}
		//For WV _extractWVContentMetadataFromPssh() won't work at this point
		//Since the content meta data is with Agnostic DRM PSSH.
		else if (drmType == eDRM_PlayReady)
		{
				contentMetaData = aamp_ExtractDataFromPssh(reinterpret_cast<const char*>(initDataPtr),dataLength,COMCAST_DRM_METADATA_TAG_START, COMCAST_DRM_METADATA_TAG_END, &contentMetaDataLen);
		}

		bool isComcastStream = false;

		char *externLicenseServerURL = NULL;
		if (gpGlobalConfig->prLicenseServerURL && drmType == eDRM_PlayReady)
		{
			externLicenseServerURL = gpGlobalConfig->prLicenseServerURL;
		}
		else if (gpGlobalConfig->wvLicenseServerURL && drmType == eDRM_WideVine)
		{
			externLicenseServerURL = gpGlobalConfig->wvLicenseServerURL;
		}
		else if (gpGlobalConfig->ckLicenseServerURL && drmType == eDRM_ClearKey)
		{
			externLicenseServerURL = gpGlobalConfig->ckLicenseServerURL;
		}
		else if (gpGlobalConfig->licenseServerURL)
		{
			externLicenseServerURL = gpGlobalConfig->licenseServerURL;
		}


		if(contentMetaData)
		{
			/*
				Constuct the licence challenge in the form of JSON message which can be parsed by MDS server
				For the time keySystem and mediaUsage are constants
				licenceChallenge from drm and contentMetadata are to be b64 encoded in the JSON
			*/
			logprintf("%s:%d MDS server spcific conent metadata found in initdata", __FUNCTION__, __LINE__);

#ifdef LOG_TRACE
			logprintf("\n\n%s:%d ContentMetaData = \n", __FUNCTION__, __LINE__);
			DumpBlob( contentMetaData, contentMetaDataLen );
			cout<<endl;
#endif
			GrowableBuffer comChallenge = {0,0,0};
			const char * availableFields = "{\"keySystem\":\"playReady\",\"mediaUsage\":\"stream\",\"licenseRequest\":\"";
			aamp_AppendBytes(&comChallenge, availableFields, strlen(availableFields));

			char *licenseRequest = base64_Encode(licenceChallenge->getData(),licenceChallenge->getDataLength());
			delete licenceChallenge;
			aamp_AppendBytes(&comChallenge, licenseRequest, strlen(licenseRequest));
			aamp_AppendBytes(&comChallenge,"\",\"contentMetadata\":\"", strlen("\",\"contentMetadata\":\""));
			char * encodedData = base64_Encode(contentMetaData,contentMetaDataLen);
			free(contentMetaData);
			aamp_AppendBytes(&comChallenge, encodedData,strlen(encodedData));

			int tokenLen = 0;
			long tokenError = 0;
			const char * sessionToken = getAccessToken(tokenLen, tokenError);
			const char * secclientSessionToken = NULL;
			if(sessionToken != NULL && !gpGlobalConfig->licenseAnonymousRequest)
			{
				logprintf("%s:%d access token is available", __FUNCTION__, __LINE__);
				aamp_AppendBytes(&comChallenge,"\",\"accessToken\":\"", strlen("\",\"accessToken\":\""));
				aamp_AppendBytes(&comChallenge, sessionToken, tokenLen);
				secclientSessionToken = sessionToken;
			}
			else
			{
				if(NULL == sessionToken)
				{
					e->data.dash_drmmetadata.failure = AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN;
					e->data.dash_drmmetadata.responseCode = tokenError;
				}
				logprintf("%s:%d Trying to get license without token", __FUNCTION__, __LINE__);
			}
			aamp_AppendBytes(&comChallenge, "\"}",strlen("\"}"));

#ifdef LOG_TRACE
			cout << systemId << endl << "Inside Session manager; printing Challenge : ";
			DumpBlob( (const unsigned char *)comChallenge.ptr, comChallenge.len );
			cout << endl;
#endif
			licenceChallenge = new DrmData(reinterpret_cast<unsigned char*>(comChallenge.ptr),comChallenge.len);
			aamp_Free(&comChallenge.ptr);

			if (externLicenseServerURL)
			{
#ifdef USE_SECCLIENT
				destinationURL = getFormattedLicenseServerURL(string(externLicenseServerURL));
#else
				destinationURL = string(externLicenseServerURL);
#endif
			}
			else
			{
				if (string::npos != destinationURL.find("rogers.ccp.xcal.tv"))
				{
					destinationURL = string(COMCAST_ROGERS_DRM_LICENCE_SERVER_URL);
				}
				else if (string::npos != destinationURL.find("qa.ccp.xcal.tv"))
				{
					destinationURL = string(COMCAST_QA_DRM_LICENCE_SERVER_URL);
				}
				else if (string::npos != destinationURL.find("ccp.xcal.tv"))
				{
					destinationURL = string(COMCAST_DRM_LICENCE_SERVER_URL);
				}
			}
			isComcastStream = true;
			aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_NETWORK);
#ifdef USE_SECCLIENT
			const char *mediaUsage = "stream";

			int32_t sec_client_result = SEC_CLIENT_RESULT_FAILURE;
			char *licenseResponse = NULL;
			size_t licenseResponseLength = 2;
			uint32_t refreshDuration = 3;
			SecClient_ExtendedStatus statusInfo;
			const char *requestMetadata[1][2];
			std::string moneytracestr;
			requestMetadata[0][0] = "X-MoneyTrace";
			aamp->GetMoneyTraceString(moneytracestr);
			requestMetadata[0][1] = moneytracestr.c_str();			

			logprintf("[HHH] Before calling SecClient_AcquireLicense-----------");
			logprintf("destinationURL is %s", destinationURL.c_str());
			logprintf("MoneyTrace[%s]", requestMetadata[0][1]);
			//logprintf("encodedData is %s, length=%d", encodedData, strlen(encodedData));
			//logprintf("licenseRequest is %s", licenseRequest);
			logprintf("keySystem is %s", keySystem);
			//logprintf("mediaUsage is %s", mediaUsage);
			//logprintf("sessionToken is %s", sessionToken);
			unsigned int attemptCount = 0;
			int sleepTime = gpGlobalConfig->licenseRetryWaitTime;
                        if(sleepTime<=0) sleepTime = 100;
			while(attemptCount < MAX_LICENSE_REQUEST_ATTEMPTS)
			{
				attemptCount++;
				sec_client_result = SecClient_AcquireLicense(destinationURL.c_str(), 1,
									requestMetadata, 0, NULL,
									encodedData,
									strlen(encodedData),
									licenseRequest, strlen(licenseRequest), keySystem, mediaUsage,
									secclientSessionToken,
									&licenseResponse, &licenseResponseLength, &refreshDuration, &statusInfo);

				if (((sec_client_result >= 500 && sec_client_result < 600)||
					(sec_client_result >= SEC_CLIENT_RESULT_HTTP_RESULT_FAILURE_TLS  && sec_client_result <= SEC_CLIENT_RESULT_HTTP_RESULT_FAILURE_GENERIC ))
					&& attemptCount < MAX_LICENSE_REQUEST_ATTEMPTS)
				{
					logprintf("%s:%d acquireLicense FAILED! license request attempt : %d; response code : sec_client %d", __FUNCTION__, __LINE__, attemptCount, sec_client_result);
					if (licenseResponse) SecClient_FreeResource(licenseResponse);
					logprintf("%s:%d acquireLicense : Sleeping %d milliseconds before next retry.", __FUNCTION__, __LINE__, gpGlobalConfig->licenseRetryWaitTime);
					mssleep(sleepTime);
				}
				else
				{
					break;
				}
			}

			if (gpGlobalConfig->logging.debug)
			{
				logprintf("licenseResponse is %s", licenseResponse);
				logprintf("licenseResponse len is %zd", licenseResponseLength);
				logprintf("accessAttributesStatus is %d", statusInfo.accessAttributeStatus);
				logprintf("refreshDuration is %d", refreshDuration);
			}

			if (sec_client_result != SEC_CLIENT_RESULT_SUCCESS)
			{
				logprintf("%s:%d acquireLicense FAILED! license request attempt : %d; response code : sec_client %d", __FUNCTION__, __LINE__, attemptCount, sec_client_result);
				responseCode = sec_client_result;
			}
			else
			{
				logprintf("%s:%d acquireLicense SUCCESS! license request attempt %d; response code : sec_client %d",__FUNCTION__, __LINE__, attemptCount, sec_client_result);
				e->type = AAMP_EVENT_DRM_METADATA;
                                e->data.dash_drmmetadata.accessStatus_value = statusInfo.accessAttributeStatus;
				key = new DrmData((unsigned char *)licenseResponse, licenseResponseLength);
			}
			if (licenseResponse) SecClient_FreeResource(licenseResponse);
#else
			struct curl_slist *headers = NULL;
			aamp->GetCustomLicenseHeaders(&headers); //headers are freed in getLicense call
			logprintf("%s:%d License request ready for %s stream", __FUNCTION__, __LINE__, sessionTypeName[streamType]);
			char *licenseProxy = aamp->GetLicenseReqProxy();
			key = getLicense(licenceChallenge, destinationURL, &responseCode, isComcastStream, licenseProxy, headers, drmType);
#endif
			free(licenseRequest);
			free(encodedData);

		}
		else 
		{
			if (externLicenseServerURL)
			{
				destinationURL = string(externLicenseServerURL);
			}
			logprintf("%s:%d License request ready for %s stream", __FUNCTION__, __LINE__, sessionTypeName[streamType]);
			struct curl_slist *headers = NULL;
			aamp->GetCustomLicenseHeaders(&headers); //headers are freed in getLicense call
			aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_NETWORK);
			char *licenseProxy = aamp->GetLicenseReqProxy();
			key = getLicense(licenceChallenge, destinationURL, &responseCode , isComcastStream, licenseProxy, headers, drmType);
		}

		if(key != NULL && key->getDataLength() != 0)
		{
			aamp->profiler.ProfileEnd(PROFILE_BUCKET_LA_NETWORK);
			if(isComcastStream)
			{
#ifndef USE_SECCLIENT
				/*
					Licence response from MDS server is in JSON form
					Licence to decrypt the data can be found by extracting the contents for JSON key licence
					Format : {"licence":"b64encoded licence","accessAttributes":"0"}
				*/
				size_t keylen = 0;
				string jsonStr = string(reinterpret_cast<char*>(key->getData()));
				string keyStr = _extractSubstring(jsonStr, LICENCE_RESPONSE_JSON_LICENCE_KEY, "\"");
				if(keyStr.length() != 0)
				{
					delete key;
					unsigned char* keydata = base64_Decode(keyStr.c_str(),&keylen);
					key = new DrmData(keydata, keylen);
					free(keydata);
				}
#endif
#ifdef LOG_TRACE
				cout << "Printing key data  from server \n";
				unsigned char * data1 = key->getData();
				DumpBlob( data1, key->getDataLength() );
				cout << endl;
#endif
			}
			aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_POSTPROC);
			processKeyRetValue = drmSessionContexts[sessionSlot].drmSession->aampDRMProcessKey(key);
			aamp->profiler.ProfileEnd(PROFILE_BUCKET_LA_POSTPROC);
		}
		else
		{
			aamp->profiler.ProfileError(PROFILE_BUCKET_LA_NETWORK, responseCode);
			logprintf("%s:%d Could not get license from server for %s stream", __FUNCTION__, __LINE__, sessionTypeName[streamType]);

			if(412 == responseCode)
			{
				if(e->data.dash_drmmetadata.failure != AAMP_TUNE_FAILED_TO_GET_ACCESS_TOKEN)
				{
					e->data.dash_drmmetadata.failure = AAMP_TUNE_AUTHORISATION_FAILURE;
				}
			}
#ifdef USE_SECCLIENT
			else if(SEC_CLIENT_RESULT_HTTP_RESULT_FAILURE_TIMEOUT == responseCode)
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_LICENCE_TIMEOUT;
			}
			else if(SEC_CLIENT_RESULT_MAC_AUTH_NOT_PROVISIONED == responseCode)
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_DEVICE_NOT_PROVISIONED;
				e->data.dash_drmmetadata.responseCode = responseCode;
			}
#endif
			else if(CURLE_OPERATION_TIMEDOUT == responseCode)
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_LICENCE_TIMEOUT;
			}
			else
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_LICENCE_REQUEST_FAILED;
				e->data.dash_drmmetadata.responseCode = responseCode;
			}
		}

		if (key != NULL)
		{
			delete key;
		}
	}
	else
	{
		logprintf("%s:%d Error in getting license challenge for %s stream : Key State %d ",
					__FUNCTION__, __LINE__, sessionTypeName[streamType], code);
		aamp->profiler.ProfileError(PROFILE_BUCKET_LA_PREPROC, AAMP_TUNE_DRM_CHALLENGE_FAILED);
		e->data.dash_drmmetadata.failure = AAMP_TUNE_DRM_CHALLENGE_FAILED;
	}

	delete licenceChallenge;
	code = drmSessionContexts[sessionSlot].drmSession->getState();

	if (code == KEY_READY)
	{
		logprintf("%s:%d Key Ready for %s stream", __FUNCTION__, __LINE__, sessionTypeName[streamType]);
		if(drmSessionContexts[sessionSlot].data != NULL)
		{
			delete drmSessionContexts[sessionSlot].data;
		}
		drmSessionContexts[sessionSlot].dataLength = keyIdLen;
		drmSessionContexts[sessionSlot].data = new unsigned char[keyIdLen];
		memcpy(reinterpret_cast<void*>(drmSessionContexts[sessionSlot].data),
		reinterpret_cast<const void*>(keyId),keyIdLen);
		pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
		free(keyId);
		keyId = NULL;
		return drmSessionContexts[sessionSlot].drmSession;
	}
	else if (code == KEY_ERROR)
	{
		if(AAMP_TUNE_FAILURE_UNKNOWN == e->data.dash_drmmetadata.failure)
		{
			// check if key failure is due to HDCP , if so report it appropriately instead of Failed to get keys 
			if(processKeyRetValue == HDCP_OUTPUT_PROTECTION_FAILURE || processKeyRetValue == HDCP_COMPLIANCE_CHECK_FAILURE)
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_HDCP_COMPLIANCE_ERROR;
			}
			else
			{
				e->data.dash_drmmetadata.failure = AAMP_TUNE_DRM_KEY_UPDATE_FAILED;
			}
		}
	}
	else if (code == KEY_PENDING)
	{
		logprintf("%s:%d Failed to get %s DRM keys for %s stream",
					__FUNCTION__, __LINE__, systemId ,sessionTypeName[streamType]);
		if(AAMP_TUNE_FAILURE_UNKNOWN == e->data.dash_drmmetadata.failure)
		{
			e->data.dash_drmmetadata.failure = AAMP_TUNE_INVALID_DRM_KEY;
		}
	}

	pthread_mutex_unlock(&(drmSessionContexts[sessionSlot].sessionMutex));
	free(keyId);
	keyId = NULL;
	return NULL;
}

/**
 *  @brief		Function to release the DrmSession if it running
 *  @param[out]	private aamp instance
 *  @return		None.
 */
void ReleaseDRMLicenseAcquireThread(PrivateInstanceAAMP *aamp){
		
	if(aamp->drmSessionThreadStarted) //In the case of license rotation
	{
		void *value_ptr = NULL;
		int rc = pthread_join(aamp->createDRMSessionThreadID, &value_ptr);
		if (rc != 0)
		{
			AAMPLOG_WARN("%s:%d pthread_join returned %d for createDRMSession Thread", 
			__FUNCTION__, __LINE__, rc);
		}
		aamp->drmSessionThreadStarted = false;
	}
}

/**
 *  @brief		Function to spawn the DrmSession Thread based on the
 *              preferred drm set.  
 *  @param[out]	private aamp instance
 *  @return		None.
 */
int SpawnDRMLicenseAcquireThread(PrivateInstanceAAMP *aamp, DrmSessionDataInfo* drmData)
{
	int iState = DRM_API_FAILED;
	do{

		/** API protection added **/
		if (NULL == drmData){
			AAMPLOG_ERR("%s:%d Could not able to process with the NULL Drm data", 
				__FUNCTION__, __LINE__);
			break;
		}
		/** Achieve single thread logic for DRM Session Creation **/
		ReleaseDRMLicenseAcquireThread(aamp);
		AAMPLOG_INFO("%s:%d Creating thread with sessionData = 0x%08x",
					__FUNCTION__, __LINE__, drmData->sessionData );
        if(0 == pthread_create(&aamp->createDRMSessionThreadID, NULL,\
		 CreateDRMSession, drmData->sessionData))
		{
			drmData->isProcessedLicenseAcquire = true;
			aamp->drmSessionThreadStarted = true;
			aamp->setCurrentDrm(drmData->drmType);
			iState = DRM_API_SUCCESS;
		}
		else
		{
			AAMPLOG_ERR("%s:%d pthread_create failed for CreateDRMSession : error code %d, %s", 
			__FUNCTION__, __LINE__, errno, strerror(errno));
		}
	}while(0);

	return iState;
}

/**
 *  @brief		Thread function to create DRM Session which would be invoked in thread from
 *              HLS , PlayReady or from pipeline  
 *
 *  @param[out]	arg - DrmSessionParams structure with filled data
 *  @return		None.
 */
void *CreateDRMSession(void *arg)
{
	AAMPLOG_INFO("%s:%d Entered arg - 0x%08x", 
	 __FUNCTION__, __LINE__, arg );

	if(aamp_pthread_setname(pthread_self(), "aampDRM"))
	{
		AAMPLOG_ERR("%s:%d: aamp_pthread_setname failed", __FUNCTION__, __LINE__);
	}
	struct DrmSessionParams* sessionParams = (struct DrmSessionParams*)arg;
	AampDRMSessionManager* sessionManger = sessionParams->aamp->mDRMSessionManager;
	sessionParams->aamp->profiler.ProfileBegin(PROFILE_BUCKET_LA_TOTAL);
	AAMPEvent e;
	e.type = AAMP_EVENT_DRM_METADATA;
	e.data.dash_drmmetadata.failure = AAMP_TUNE_FAILURE_UNKNOWN;
	e.data.dash_drmmetadata.responseCode = 0;
	unsigned char * data = sessionParams->initData;
	int dataLength = sessionParams->initDataLen;

	unsigned char *contentMetadata = sessionParams->contentMetadata;
	AampDrmSession *drmSession = NULL;
	const char * systemId = WIDEVINE_PROTECTION_SYSTEM_ID;
	if (sessionParams->drmType == eDRM_WideVine)
	{
		AAMPLOG_INFO("Found Widevine encryption from manifest");
	}
	else if(sessionParams->drmType == eDRM_PlayReady)
	{
		AAMPLOG_INFO("Found Playready encryption from manifest");
		systemId = PLAYREADY_PROTECTION_SYSTEM_ID;
	}
	else if(sessionParams->drmType == eDRM_ClearKey)
	{
		AAMPLOG_INFO("Found ClearKey encryption from manifest");
		systemId = CLEARKEY_PROTECTION_SYSTEM_ID;
	}
	sessionParams->aamp->mStreamSink->QueueProtectionEvent(systemId, data, dataLength, sessionParams->stream_type);
	//Hao Li: review changes for Widevine, contentMetadata is freed inside the following calls
	drmSession = sessionManger->createDrmSession(systemId, data, dataLength, sessionParams->stream_type,
					contentMetadata, sessionParams->aamp, &e);
	if(NULL == drmSession)
	{
		AAMPLOG_ERR("%s:%d Failed DRM Session Creation for systemId = %s", __FUNCTION__, __LINE__, systemId);
		AAMPTuneFailure failure = e.data.dash_drmmetadata.failure;
		long responseCode = e.data.dash_drmmetadata.responseCode;
		bool selfAbort = (failure == AAMP_TUNE_LICENCE_REQUEST_FAILED && (responseCode == CURLE_ABORTED_BY_CALLBACK || responseCode == CURLE_WRITE_ERROR));
		if (!selfAbort)
		{
			bool isRetryEnabled =      (failure != AAMP_TUNE_AUTHORISATION_FAILURE)
						&& (failure != AAMP_TUNE_LICENCE_REQUEST_FAILED)
						&& (failure != AAMP_TUNE_LICENCE_TIMEOUT)
						&& (failure != AAMP_TUNE_DEVICE_NOT_PROVISIONED)
						&& (failure != AAMP_TUNE_HDCP_COMPLIANCE_ERROR);
			sessionParams->aamp->SendDrmErrorEvent(e.data.dash_drmmetadata.failure, e.data.dash_drmmetadata.responseCode, isRetryEnabled);
		}
		sessionParams->aamp->profiler.SetDrmErrorCode((int)e.data.dash_drmmetadata.failure);
		sessionParams->aamp->profiler.ProfileError(PROFILE_BUCKET_LA_TOTAL, (int)e.data.dash_drmmetadata.failure);
	}
	else
	{
		if(e.data.dash_drmmetadata.accessStatus_value != 3)
		{
			AAMPLOG_INFO("Sending DRMMetaData");
			sessionParams->aamp->SendDRMMetaData(e);
		}
		sessionParams->aamp->profiler.ProfileEnd(PROFILE_BUCKET_LA_TOTAL);
	}

	//AAMPLOG_INFO("%s:%d Exited",  __FUNCTION__, __LINE__ );
	return NULL;
}
