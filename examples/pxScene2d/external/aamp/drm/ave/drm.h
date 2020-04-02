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
 * @file drm.h
 * @brief AVE DRM helper declarations
 */

#ifndef DRM_H
#define DRM_H

#include <stddef.h> // for size_t
#include "HlsDrmBase.h"
#include <memory>

#define MAX_DRM_CONTEXT 6
#define DRM_SHA1_HASH_LEN 40
#define DRM_IV_LEN 16
#ifdef AVE_DRM
#include "ave-adapter/MyFlashAccessAdapter.h"
#else
/**
 * @enum DrmMethod
 * @brief AVE drm method
 */
typedef enum
{
	eMETHOD_NONE,
	eMETHOD_AES_128, /// encrypted using Advanced Encryption Standard 128-bit key and PKCS7 padding
} DrmMethod;

/**
 * @struct DrmMetadata
 * @brief AVE drm metadata extracted from EXT-X-FAXS-CM
 */
struct DrmMetadata
{ // from EXT-X-FAXS-CM
	unsigned char * metadataPtr;
	size_t metadataSize;
};

/**
 * @struct DrmInfo
 * @brief DRM information required to decrypt
 */
struct DrmInfo
{ // from EXT-X-KEY
	DrmMethod method;
	bool useFirst16BytesAsIV;
	unsigned char *iv; // [16]
	char *uri;
//	unsigned char *CMSha1Hash;// [20]; // unused
//	unsigned char *encryptedRotationKey;
};
#endif /*AVE_DRM*/


/**
 * @class AveDrm
 * @brief Adobe AVE DRM management
 */
class AveDrm : public HlsDrmBase
{
public:
	AveDrm();
	AveDrm(const AveDrm&) = delete;
	AveDrm& operator=(const AveDrm&) = delete;
	~AveDrm();
	DrmReturn SetMetaData(class PrivateInstanceAAMP *aamp, void* metadata,int trackType);
	DrmReturn SetDecryptInfo(PrivateInstanceAAMP *aamp, const struct DrmInfo *drmInfo);
	DrmReturn Decrypt(ProfilerBucketType bucketType, void *encryptedDataPtr, size_t encryptedDataLen, int timeInMs);
	void Release();
	void CancelKeyWait();
	void RestoreKeyState();
	void SetState(DRMState state);
	DRMState GetState();
	void AcquireKey( class PrivateInstanceAAMP *aamp, void *metadata,int trackType);
	DRMState mDrmState;
private:
	PrivateInstanceAAMP *mpAamp;
	class MyFlashAccessAdapter *m_pDrmAdapter;
	class TheDRMListener *m_pDrmListner;
	DRMState mPrevDrmState;
	DrmMetadata mMetaData;
	DrmInfo mDrmInfo;
	pthread_cond_t cond;
	pthread_mutex_t mutex;
	// Function to store the new DecrypytInfo 
	bool StoreDecryptInfoIfChanged( const DrmInfo *drmInfo);
};


/**
* @struct       data
* @brief        data structure for poplulating values from DRM listner to senderrorevent.
*/
typedef struct DRMErrorData
{
        void *ptr;
        char description[MAX_ERROR_DESCRIPTION_LENGTH];
        AAMPTuneFailure drmFailure;
        bool isRetryEnabled;
}DRMErrorData;


/**
* @struct	DrmMetadataNode
* @brief	DrmMetadataNode structure for DRM Metadata/Hash storage
*/
struct DrmMetadataNode
{
	DrmMetadata metaData;
	int deferredInterval ;
	long long drmKeyReqTime;
	char* sha1Hash;
};

/**
* @class	AveDrmManager
* @brief	Manages AveDrm instances and provide functions for license acquisition and rotation.
* 			Methods are not multi-thread safe. Caller is responsible for synchronization.
*/
class AveDrmManager
{
public:
	static void ResetAll();
	static void CancelKeyWaitAll();
	static void ReleaseAll();
	static void RestoreKeyStateAll();
	static void SetMetadata(PrivateInstanceAAMP *aamp, DrmMetadataNode *metaDataNode,int trackType);
	static void PrintSha1Hash( char* sha1Hash);
	static void DumpCachedLicenses();
	static void FlushAfterIndexList(const char* trackname,int trackType);
	static void UpdateBeforeIndexList(const char* trackname,int trackType);
	static int IsMetadataAvailable(char* sha1Hash);
	static std::shared_ptr<AveDrm> GetAveDrm(char* sha1Hash,int trackType);
	static bool AcquireKey(PrivateInstanceAAMP *aamp, DrmMetadataNode *metaDataNode,int trackType,bool overrideDeferring=false);
	static int GetNewMetadataIndex(DrmMetadataNode* drmMetadataIdx, int drmMetadataCount);
private:
	AveDrmManager();
	void Reset();
	char mSha1Hash[DRM_SHA1_HASH_LEN];
	std::shared_ptr<AveDrm> mDrm;
	bool mDrmContexSet;
	bool mHasBeenUsed;
	int mUserCount;
	int mTrackType;
	long long mDeferredTime;
	static std::vector<AveDrmManager*> sAveDrmManager;
};

#endif // DRM_H
