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

//#define TEST_CODE_ON
// Enable following compile flag in future when all playback is turned on with 
// ContentMetadata method where in Async way of key retrieval is needed.
// For IPDVR this is disabled to avoid resouce utilization , IPDVR uses Sync way
// of key retrieval 
//#define PLAYBACK_ASYNC_SUPPORT
#ifdef AAMP_CONTENT_METADATA_IPDVR_ENABLED 
#ifndef ZERO_DRM_H
#define ZERO_DRM_H

#include <stddef.h> 
#include <string> 
#include "sec_security.h"
#include "sec_client.h"
#include <sys/time.h>
#include <uuid/uuid.h>
#include <vector>
#include <map>
#include <algorithm>
#include <queue>
#include <errno.h>

#ifdef TEST_CODE_ON
#define logprintf printf
#else
#include "priv_aamp.h"
#endif

//#define ZDRM_TRACE
#ifdef ZDRM_TRACE
#define zdebuglogprintf logprintf
#else
#define zdebuglogprintf (void)
#endif

#define ZERO_DRM_HOST_URL  "mds.ccp.xcal.tv"
#define ZERO_DRM_REQMETADATA_SZ 2
#define ZERO_DRM_KEY_CACHE_LIFETIME	1800000 // 30min of lifetime 

typedef enum
{
	eZERO_METHOD_NONE,
	eZERO_METHOD_AES_128, // encrypted using Advanced Encryption Standard 128-bit key and PKCS7 padding
} ZeroDrmMethod;


typedef enum
{
	eZERO_DRM_STATE_INITIALIZED,
	eZERO_DRM_STATE_ACQUIRING_RECEIPT,
	eZERO_DRM_STATE_RECEIPT_ACQUIRED,
	eZERO_DRM_STATE_RECEIPT_FAILED,
	eZERO_DRM_STATE_ACQUIRING_KEY,
	eZERO_DRM_STATE_KEY_ACQUIRED,
	eZERO_DRM_STATE_KEY_FAILED,
	eZERO_DRM_STATE_INVALID,
} ZeroDrmState;

typedef enum
{
	eZERO_DRM_STATUS_SUCCESS,
	eZERO_DRM_STATUS_GENERIC_ERROR,
	eZERO_DRM_STATUS_RECEIPT_FAILED,
	eZERO_DRM_STATUS_KEY_FAILED,
	eZERO_DRM_STATUS_KEY_TIMEOUT,
	eZERO_DRM_STATUS_DECRYPT_FAILED,
	eZERO_DRM_STATUS_INVALID_CTX,
	eZERO_DRM_STATUS_NO_METADATA,
	eZERO_DRM_STATUS_NO_KEY_AVAILABLE
	// map other errors from secclient
} ZeroDrmReturn;

typedef void (*ZeroDrmStatusCallbackFnPtr)(ZeroDrmState drmState , int errRet , void *callbackData);

typedef struct
{ 
	// from EXT-X-XCAL-CONTENTMETADATA
	ZeroDrmMethod method;
	bool useFirst16BytesAsIV;
	unsigned char iv[16];
	char *uri;
	unsigned char *CMSha1Hash;// [20]; // unused
	//	unsigned char *encryptedRotationKey;
}ZeroDrmInfo;

typedef struct 
{ 
	// from EXT-X-XCAL-CONTENTMETADATA
	unsigned char * metadataPtr;
	unsigned char * receiptdataPtr;
	size_t metadataSize;
	size_t recieptdataSize;
	unsigned char metadataHash[20];
	unsigned char receiptdataHash[20];
	Secure_Context 	secureContext;	
	ZeroDrmInfo	keyTagInfo;
	ZeroDrmState 	mZeroDrmState;
	long long 	lastUpdateTime;
	int	mDrmLastError;
	bool 	receiptAvailable;
	bool	keyAvailable;
	bool	keytagInfoAvailable;
	bool mbFirstCheck;
}ZeroDrmMetadata;

typedef struct
{
	uint32_t mZDrmContext;
	unsigned char *mZContentData;
	unsigned char *mZReceiptData;
	unsigned char *mKeyTagData;
	bool bZDrmPlayback;
	bool bZDrmMetadataRead;
}ZDrmContextData;

class ZeroDRMContextData
{
public :
	ZeroDRMContextData() 
	{
		mStatusCallbackFn = NULL;
		mCallbackData	=	NULL;
		mDrmLastError = 0;
		mActiveMetadata	=	NULL;
	}
	~ZeroDRMContextData()
	{
		// flush 
		if(!mCtxHashList.empty())
			mCtxHashList.erase(mCtxHashList.begin() , mCtxHashList.end());
	}
	uint32_t getContextId() { return reinterpret_cast<uint32_t> (this) ; }
	// store recent active metadata 
	void	setMetadata(ZeroDrmMetadata * metadata) { mActiveMetadata = metadata;}
	// get recent active metadata 
	ZeroDrmMetadata *getMetadata(){ return mActiveMetadata;}		
	// Per stream there can be multiple ContentData / Receipts . HashList stores all those per stream
	typedef std::vector <uint32_t> CtxHashList ; 
	typedef std::vector <uint32_t>::iterator CtxHashListIter;
	CtxHashList mCtxHashList;
	int			mDrmLastError;
	// user callback function and callback data
	ZeroDrmStatusCallbackFnPtr mStatusCallbackFn;
	void *mCallbackData;
	ZeroDrmMetadata *mActiveMetadata;
	
};

class ZeroDrmTimeCheck
{
public : 
	ZeroDrmTimeCheck(std::string functionName)
	{
		mFuncName = functionName;
		mStartTime = drmGetCurrentTimeMS();
	}
	
	~ZeroDrmTimeCheck()
	{
		extern void logprintf(const char *format, ...);

		long long timetaken = (drmGetCurrentTimeMS() - mStartTime);
		if(timetaken > 300)
			logprintf("!!!!! ZeroDRMPerf::Function[%s]->[%ld]\n", mFuncName.c_str(),timetaken);
	}

static long long drmGetCurrentTimeMS(void);
private:
	std::string mFuncName ; 
	long long mStartTime;
};

// data for interfacing with thread function 
typedef struct 
{
	uint32_t ctxId; 
	uint32_t hashValue;
	ZeroDrmInfo keyTag;
}ZeroDrmWorkerData;


class ZeroDRMAccessAdapter
{
public :
	static ZeroDRMAccessAdapter *getInstance();
	static void deleteInstance();
	
	//Initialize Context Info 
	bool zeroDrmInitialize(uint32_t &contextId , ZeroDrmStatusCallbackFnPtr fnPtr=NULL, void *callbackData=NULL);
	// Finalize Context Info
	void zeroDrmFinalize(const uint32_t contextId);
	// Set the ContentMetadata from Maninfest 
	bool zeroDrmSetContentMetadata(const uint32_t contextId , const unsigned char * metadata, size_t metadataSz);
	// Set the ReceiptData from Maninfest
	bool zeroDrmSetReceiptMetadata(const uint32_t contextId , const unsigned char * metadata, size_t metadataSz);
	// Decrypt the data 
	ZeroDrmReturn zeroDrmDecrypt(const uint32_t contextId,void *encryptedDataPtr, size_t encryptedDataLen,int timeInMs = 3000,const uint32_t hashKey=0);
	// get last error code
	int getZeroDRMLastError()  { return mDrmLastError;}
	bool zeroDrmIsContextActive(const uint32_t contextId );
	// parse X-KEY tag 
	// Sync mode - receipt/key is processed sync mode adding wait time to this API
	bool zeroDrmGetPlaybackKeySync(const uint32_t contextId , const char *sTagLine );
#ifdef PLAYBACK_ASYNC_SUPPORT
	// Async mode - receipt/key is processed async mode and callback is given to registered callback function
	bool zeroDrmGetPlaybackKeyAsync(const uint32_t contextId , const char *sTagLine);
#endif
	// Function to set the caching flag - This can be called to disable key caching in adapter
	void zeroDrmSetCacheReUseFlag(bool flag) { mCacheReUseFlag = flag; }	
private :
	int	mDrmLastError;	
	static bool mInstanceAvailFlag;
	static ZeroDRMAccessAdapter *mInstance;
	// thread related 
	pthread_cond_t mJobStatusCondVar ;
	pthread_cond_t mJobPostingCondVar ;	
	pthread_mutex_t mMutexJSCondVar ;
	pthread_mutex_t mMutexJPCondVar ;	
	pthread_mutex_t mMutexVar ;		
	pthread_t mWorkerThreadId;
	bool mWorkerThreadStarted;
	bool mWorkerThreadEndFlag;
	typedef std::queue<ZeroDrmWorkerData *> ZWorkerDataQ;
	ZWorkerDataQ mZWorkerDataQue;
	// if multiple context need to be supported by this DRM . Normally only one playback session will be there
	// <contextId , ZeroDRMContextData *>
	typedef std::map<uint32_t , ZeroDRMContextData *> ContextList;
	typedef std::map<uint32_t , ZeroDRMContextData *>::iterator ContextListIter;
	ContextList mContextList;
	// This is important keyhashtable , which maintains entire keyinfo / receipts / contentMetadata
	// < hashkey , ZeroDrmMetadata *>
	typedef std::map<uint32_t , ZeroDrmMetadata *> KeyHashTable;
	typedef std::map<uint32_t , ZeroDrmMetadata *>::iterator KeyHashTableIter;
	KeyHashTable mKeyHashTable;	
	bool mCacheReUseFlag;
private:	
	// private functions
	ZeroDRMAccessAdapter();
	~ZeroDRMAccessAdapter ();
	// Get traceId for drm request
	char *zeroDrmGetTraceId();
	// Generate Hash for each content/receipt data,this need to be replace with sha based hash for license rotation
	uint32_t JSHash(const unsigned char *str, size_t length);
	// Parse X-KEY tag from manifest
	bool zeroDrmParseKeyTag(ZeroDrmInfo	&keyTag , const char *sTagLine);
	// Worker Thread
	void zeroDRMWorkerThreadTask();
	// Release Metadata memory
	void zeroDrmDeleteMetadata(ZeroDrmMetadata *metadata);
	// Get key from remote server
	ZeroDrmReturn zeroDrmGetKey(const uint32_t contextId  , const uint32_t hashValue,ZeroDrmInfo &keyTag, ZeroDrmState   &retState);
	// free memory of keyTag information
	void zeroDrmDeleteKeyTag(ZeroDrmInfo *keyTagInfo);
	// When stream is having multiple metadata(key rotation), this function stores the latest metadata for quick access
	void zeroDrmSetActiveMetadata(const uint32_t contextId , ZeroDrmMetadata *metadata);
	// Get the latest stored metadata for quick decrypt
	ZeroDrmReturn zeroDrmGetMetadata(const uint32_t contextId,ZeroDrmMetadata **metadata,const uint32_t hashKey);
	// checks if user context is still active 
	bool zeroDrmIsContextIdValid(const uint32_t contextId );
#ifdef PLAYBACK_ASYNC_SUPPORT
	// Thread entry function for Async mode processing 
	static void * ThreadEntryFunction(void * This) {((ZeroDRMAccessAdapter *)This)->zeroDRMWorkerThreadTask(); return NULL;}
#endif
	inline uint8_t getHexFromChar(char c)
        {
                if (c >= '0' && c <= '9')
                {
                        return c - '0';
                }
                else if (c >= 'a' && c <= 'f')
                {
                        return c - 'a' + 0xa;
                }
                else if (c >= 'A' && c <= 'F')
                {
                        return c - 'A' + 0xa;
                }

                return 0;
        }

};


#endif  // ZERO_DRM_H
#endif
