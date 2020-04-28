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
* \file processProtection.cpp
*
* Process protection is for handling process protection of open cdm drm 
* for HSL Streaming  
* Functionalities are parse the file , get the drm type , PSSH data.
* Create DRM Session in thread 
* 
*/
#include "_base64.h"
#include "AampDRMSessionManager.h"
#include "AampDrmSession.h"
#include "fragmentcollector_hls.h"

#include <cstdlib>
#include <string>
using namespace std;

#ifdef AAMP_HLS_DRM

extern void *CreateDRMSession(void *arg);
extern void ReleaseDRMLicenseAcquireThread(PrivateInstanceAAMP *aamp);

/**
 * @brief vector pool of DrmSessionDataInfo
 */
vector <DrmSessionDataInfo *> drmSessionDataPool_g; 
static pthread_mutex_t drmProcessingMutex = PTHREAD_MUTEX_INITIALIZER;
/**
 * Global aamp config data 
 */
extern GlobalConfigAAMP *gpGlobalConfig;
DrmSessionDataInfo* ProcessContentProtection(PrivateInstanceAAMP *aamp, std::string attrName);

/**
 * Local APIs declarations
 */
static int GetFieldValue(string &attrName, string keyName, string &valuePtr);
static int getKeyId(string attrName, string &keyId);
static int getPsshData(string attrName, string &psshData);
static DRMSystems getDrmType(string attrName);
static uint8_t getPsshDataVersion(string attrName);

/**
 * @brief Return the string value, from the input KEY="value"
 * @param [in] attribute list to be searched 
 * @param [in] Key name to be checked to get the value
 * @param [out] value of the key
 * @return none
 */
static int GetFieldValue(string &attrName, string keyName, string &valuePtr){
	
	int valueStartPos = 0;
	int valueEndPos = attrName.length();
	int status = DRM_API_FAILED;

	AAMPLOG_TRACE("%s:%d Entring..", __FUNCTION__, __LINE__);

	if (attrName.find(keyName) != std::string::npos)
	{
		AAMPLOG_TRACE("%s:%d keyName = %s",
		 __FUNCTION__, __LINE__, keyName.c_str());

		valueStartPos = attrName.find(keyName) + keyName.length();
		if (attrName.at(valueStartPos) == '=')
		{
			string valueTempPtr = attrName.substr(valueStartPos);
			valueTempPtr = valueTempPtr.substr(1);

			AAMPLOG_TRACE("%s:%d valueTempPtr = %s",
			__FUNCTION__, __LINE__, valueTempPtr.c_str());

			/* update start position based on substring */
			valueStartPos = 0;
			if (valueTempPtr.at(0) == '"')
			{
				valueTempPtr = valueTempPtr.substr(1);
				valueEndPos = valueTempPtr.find('"');
			}
			else if (valueTempPtr.find(',') != std::string::npos)
			{
				valueEndPos = valueTempPtr.find(',');
			}
			else
			{
				/*look like end string*/
				valueEndPos = valueTempPtr.length();
			}

			valuePtr = valueTempPtr.substr(valueStartPos, valueEndPos);
			AAMPLOG_INFO("%s:%d Value found : %s for Key : %s",
			__FUNCTION__, __LINE__, valuePtr.c_str(), keyName.c_str());
			status = DRM_API_SUCCESS;
		}
		else
		{
			AAMPLOG_ERR("%s:%d Could not able to find %s= in %s",
			__FUNCTION__, __LINE__, keyName.c_str(), attrName.c_str());
			status = DRM_API_FAILED;
		}
	}
	else
	{
		AAMPLOG_ERR("%s:%d Could not able to find %s in %s",
		__FUNCTION__, __LINE__, keyName.c_str(), attrName.c_str());
		status = DRM_API_FAILED;
	}

	return status;
}

/* Widevine Example 
#EXT-X-KEY:METHOD=SAMPLE-AES-CTR,
KEYFORMAT="urn:uuid:edef8ba9-79d6-4ace-a3c8-27dcd51d21ed",
KEYFORMATVERSIONS="1",URI="data:text/plain;base64,AAAAW3Bzc2gAAAAA7e+LqXnWSs6jyCfc1R0h7QAAADsIARIgZTI1ZmFkYjQ4YmZiNDkyMjljZTBhNGFmZGZlMDUxOTcaB3NsaW5ndHYiBUhHVFZEKgVTRF9IRA==",
KEYID=0xe25fadb48bfb49229ce0a4afdfe05197
*/
/* PlayReady Example 
#EXT-X-KEY:METHOD=SAMPLE-AES-CTR,
URI="data:text/plain;charset=UTF-16;base64,BgIAAAEAAQD8ATwAVwBSAE0ASABFAEEARABFAFIAIAB4AG0AbABuAHMAPQAiAGgAdAB0AHAAOgAvAC8AcwBjAGgAZQBtAGEAcwAuAG0AaQBjAHIAbwBzAG8AZgB0AC4AYwBvAG0ALwBEAFIATQAvADIAMAAwADcALwAwADMALwBQAGwAYQB5AFIAZQBhAGQAeQBIAGUAYQBkAGUAcgAiACAAdgBlAHIAcwBpAG8AbgA9ACIANAAuADAALgAwAC4AMAAiAD4APABEAEEAVABBAD4APABQAFIATwBUAEUAQwBUAEkATgBGAE8APgA8AEsARQBZAEwARQBOAD4AMQA2ADwALwBLAEUAWQBMAEUATgA+ADwAQQBMAEcASQBEAD4AQQBFAFMAQwBUAFIAPAAvAEEATABHAEkARAA+ADwALwBQAFIATwBUAEUAQwBUAEkATgBGAE8APgA8AEsASQBEAD4AbgA0AEkARABBAEsATwAxAHMARwByAGcAegBpAHkAOAA4AFgAcgBqAGYAQQA9AD0APAAvAEsASQBEAD4APABDAEgARQBDAEsAUwBVAE0APgB1AGkAbwA4AFcAVQBwAFQANAA0ADAAPQA8AC8AQwBIAEUAQwBLAFMAVQBNAD4APAAvAEQAQQBUAEEAPgA8AC8AVwBSAE0ASABFAEEARABFAFIAPgA=",
KEYFORMAT="com.microsoft.playready",
KEYFORMATVERSIONS="1"
*/

/**
 * @brief API to get the Widevine PSSH Data from the manifest attribute list, getWVPsshData
 * @param [in] Attribute list
 * @param [out] keyId string as reference 
 * @return status of the API
 */
static int getKeyId(string attrName, string &keyId){

	int status = GetFieldValue(attrName, "KEYID", keyId );
	if(DRM_API_SUCCESS != status){
		AAMPLOG_INFO("%s:%d Could not able to get Key Id from manifest",
		__FUNCTION__, __LINE__);
		return status;
	}

	/* Remove 0x from begining */
	keyId = keyId.substr(2);

	/* Remove/make it to trace after Debugging */
	AAMPLOG_TRACE("%s:%d Key Id Received is %s",
	__FUNCTION__, __LINE__, keyId.c_str());
	return status;
}

/**
 * @brief API to get the PSSH Data from the manifest attribute list, getPsshData
 * @param [in] Attribute list
 * @param [out] pssData as reference 
 * @return status of the API
 */
static int getPsshData(string attrName, string &psshData){

	int status = GetFieldValue(attrName, "URI", psshData );
	if(DRM_API_SUCCESS != status){
		AAMPLOG_ERR("%s:%d Could not able to get psshData from manifest",
		__FUNCTION__, __LINE__);
		return status;
	}
	/* Split string based on , and get the PSSH Data */
	psshData = psshData.substr(psshData.find(',')+1);

	/* Remove/make it to trace after Debugging */
	AAMPLOG_TRACE("%s:%d PSSH Data Received is %s", 
	__FUNCTION__, __LINE__, psshData.c_str());
	return status;
}

/**
 * @brief API to get the DRM type from the manifest attribute list, getDrmType
 * @param [in] Attribute list
 * 
 * @return pssh Data version number, default is 0
 */
static uint8_t getPsshDataVersion(string attrName){

	uint8_t psshDataVer = 0;
	string psshDataVerStr = "";

	if(DRM_API_SUCCESS != GetFieldValue(attrName, "KEYFORMATVERSIONS", psshDataVerStr )){
		AAMPLOG_WARN("%s:%d Could not able to receive pssh data version from manifest"
		"returning default value as 0",
		__FUNCTION__, __LINE__);
	}else {
		psshDataVer = (uint8_t)std::atoi(psshDataVerStr.c_str());
	}

	/* Remove/make it to trace after Debugging */
	AAMPLOG_INFO("%s:%d PSSH Data Version Received is %d", 
	__FUNCTION__, __LINE__, psshDataVer);
	return psshDataVer;
}

/**
 * @brief API to get the DRM type from the manifest attribute list, getDrmType
 * @param [in] Attribute list
 * 
 * @return DRMSystems - DRM Type (eDRM_NONE in case of unexpected behaviour)
 */
static DRMSystems getDrmType(string attrName){

	string systemId = "";
	DRMSystems drmType = eDRM_NONE;

	if(DRM_API_SUCCESS != GetFieldValue(attrName, "KEYFORMAT", systemId )){
		AAMPLOG_ERR("%s:%d Could not able to receive key id from manifest",
		__FUNCTION__, __LINE__);
		return drmType;
	}

	/** Remove urn:uuid: from it */
	if (systemId.find("urn:uuid:") != std::string::npos){
		systemId = systemId.substr(strlen("urn:uuid:"));
	}
	
	if((systemId == WIDEVINE_PROTECTION_SYSTEM_ID) || (systemId == WIDEVINE_KEY_SYSTEM_STRING)){
		drmType = eDRM_WideVine;
	
	}else if ((systemId == PLAYREADY_PROTECTION_SYSTEM_ID) || (systemId == PLAYREADY_KEY_SYSTEM_STRING)){
		drmType = eDRM_PlayReady;
	
	}else if ((systemId == CLEARKEY_PROTECTION_SYSTEM_ID) || (systemId == CLEAR_KEY_SYSTEM_STRING)){
		drmType = eDRM_ClearKey;
	
	}else{
		AAMPLOG_ERR("%s:%d Unknown DRM Id received as %s",
		__FUNCTION__, __LINE__, systemId.c_str());
	}

	return drmType;
}

/**
 * @brief Process content protection of track
 * @param TrackState object 
 * @param attribute list from manifest
 * @return none
 */
DrmSessionDataInfo* ProcessContentProtection(PrivateInstanceAAMP *aamp, std::string attrName)
{
	/* StreamAbstractionAAMP_HLS* context; */
	/* Pseudo code for ProcessContentProtection in HLS is below
	 * Get Aamp instance as aamp
	 * 1. Get DRM type from manifest (KEYFORMAT uuid)
	 * 2. Get pssh data from manifest (extract URI value)
	 * 	  2.1 	Get KeyID using aamp_ExtractKeyIdFromPssh
	 * 3. Check whether keyID with last processed keyId
	 * 4. if not, Create DrmSessionParams instance and fill it 
	 *    4.1 Create a thread with CreateDRMSession and DrmSessionParams as parameter
	 *    4.2 Reuse the thread function CreateDRMSession which is used in MPD for HLS also
	 * 5. Else delete keyId and return
	 */
	DRMSystems drmType = eDRM_NONE;
	unsigned char* data = NULL;
	unsigned char* contentMetadata = NULL;
	size_t dataLength = 0;
	int status = DRM_API_FAILED;  
	string psshDataStr = "";
	char* psshData = NULL;
	unsigned char * keyId = NULL;
	int keyIdLen = 0;
	MediaType mediaType = eMEDIATYPE_VIDEO;
	DrmSessionDataInfo *drmSessioData = NULL;
	do{
		drmType = getDrmType(attrName);
		if (eDRM_NONE == drmType){
			AAMPLOG_ERR("%s:%d Failed to get DRM type from manifest!",
			__FUNCTION__, __LINE__);
			break;
		}

		AAMPLOG_INFO("%s:%d HSL DRM Type received as %s",
		__FUNCTION__, __LINE__,  GetDrmSystemName(drmType));

		/*Check for supported DRMs, TODO: Clear key support*/
		if ((drmType != eDRM_PlayReady) && (drmType != eDRM_WideVine)){
			AAMPLOG_ERR("%s:%d Unsupported DRM found : name: %s code: %d !", 
			__FUNCTION__, __LINE__, GetDrmSystemName(drmType), drmType );
			break;
		}

		status  = getPsshData(attrName, psshDataStr);
		if (DRM_API_SUCCESS != status){
			AAMPLOG_ERR("%s:%d Failed to get PSSH Data from manifest!",
			__FUNCTION__, __LINE__);
			break;
		}
		psshData = (char*) malloc(psshDataStr.length() + 1);
		memset(psshData, 0x00 , psshDataStr.length() + 1);
		strncpy(psshData, psshDataStr.c_str(), psshDataStr.length());

		data = base64_Decode(psshData, &dataLength);
		
		/* No more use */
		free(psshData);
		psshData = NULL;

		if (dataLength == 0)
		{
			AAMPLOG_ERR("%s:%d Could not able to retrive DRM data from PSSH",
						__FUNCTION__, __LINE__);
			break;
		}
		if (gpGlobalConfig->logging.trace)
		{
			AAMPLOG_TRACE("%s:%d content metadata from manifest; length %d",
			__FUNCTION__, __LINE__, dataLength);
			printf("*****************************************************************\n");
			for (int i = 0; i < dataLength; i++)
			{
				printf("%c", data[i]);
			}
			printf("\n*****************************************************************\n");
			for (int i = 0; i < dataLength; i++)
			{
				printf("%02x ", data[i]);
			}
			printf("\n*****************************************************************\n");

		}

		keyId = aamp_ExtractKeyIdFromPssh((const char*)data, dataLength, &keyIdLen, drmType);
		if (NULL == keyId){
			AAMPLOG_ERR("%s:%d Failed to get key Id from manifest",
			__FUNCTION__, __LINE__);
			
			if(data) {
				free(data);
				data = NULL;
			}
			break;
		}

		MediaType mediaType  = eMEDIATYPE_VIDEO;	
		bool alreadyPushed = false;
		pthread_mutex_lock(&drmProcessingMutex);
		for (auto iterator = drmSessionDataPool_g.begin();  iterator != drmSessionDataPool_g.end(); iterator++){
			if (keyIdLen == (*iterator)->processedKeyIdLen && 0 == memcmp((*iterator)->processedKeyId, keyId, keyIdLen)){
				alreadyPushed = true;
				AAMPLOG_WARN("%s:%d DRM Infor already available , not sending request again",__FUNCTION__, __LINE__);								
				break;
			}
		}
		if (!alreadyPushed){
			/** Push Drm Information for later use do not free the memory here*/
			//AAMPLOG_INFO("%s:%d - Storing DRM Info for DRM (%d) at keyId %s", 
			//__FUNCTION__, __LINE__, drmType, keyId);

			/** Populate session data **/
			struct DrmSessionParams* sessionParams = (struct DrmSessionParams*)malloc(sizeof(struct DrmSessionParams));
			sessionParams->initData = data;
			sessionParams->initDataLen = dataLength;
			sessionParams->stream_type = mediaType;
			sessionParams->aamp = aamp;
			sessionParams->drmType = drmType;
			sessionParams->contentMetadata = NULL;

			/** populate pool data **/
			drmSessioData = new DrmSessionDataInfo() ;
			drmSessioData->isProcessedLicenseAcquire = false;
			drmSessioData->drmType = drmType;
			drmSessioData->sessionData = sessionParams;
			drmSessioData->processedKeyIdLen = keyIdLen;
			drmSessioData->processedKeyId = (unsigned char *) malloc(keyIdLen + 1);
			memcpy(drmSessioData->processedKeyId, keyId, keyIdLen);
			drmSessionDataPool_g.push_back(drmSessioData);
		}
		pthread_mutex_unlock(&drmProcessingMutex);

		if (keyId) {
			free(keyId);
			keyId = NULL;
		}
		
	}while(0);
	return drmSessioData;
}

/**
 * @brief Process content protection of track
 * @param TrackState object 
 * @param attribute list from manifest
 * @return none
 */
void ReleaseContentProtectionCache(PrivateInstanceAAMP *aamp)
{	
	aamp->mDRMSessionManager->setCurlAbort(true);
	ReleaseDRMLicenseAcquireThread(aamp);
	DrmSessionDataInfo *drmSessioData = NULL;
	pthread_mutex_lock(&drmProcessingMutex);
	while (!drmSessionDataPool_g.empty())
	{
		drmSessioData = drmSessionDataPool_g.back();
		drmSessionDataPool_g.pop_back();
		// check if session Data is not NULL . This is not freed any other place
		if(drmSessioData->sessionData)
		{
			if(drmSessioData->sessionData->initData)
				free(drmSessioData->sessionData->initData);		
			if(drmSessioData->sessionData->contentMetadata)
				free(drmSessioData->sessionData->contentMetadata);
			free(drmSessioData->sessionData);
		}
		if(drmSessioData->processedKeyId)
		{
			free(drmSessioData->processedKeyId);
		}
		// clear the session
		if(drmSessioData)
		{
			delete drmSessioData;
		}
	}
	pthread_mutex_unlock(&drmProcessingMutex);
}


#else

void* ProcessContentProtection(PrivateInstanceAAMP *aamp, std::string attrName){
	AAMPLOG_INFO("%s:%d AAMP_HLS_DRM not enabled", 
				__FUNCTION__, __LINE__);
	return NULL;
}
#endif /** AAMP_HLS_DRM */

/**
 * EOF
 */
