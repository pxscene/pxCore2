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

#ifdef AAMP_CONTENT_METADATA_IPDVR_ENABLED 
#include <stddef.h> 
#include <string.h> 
#include "ZeroDrmAccessAdapter.h"
#ifdef TEST_CODE_ON
#include <iostream>
#include <fstream>
#endif

// static members 
bool ZeroDRMAccessAdapter::mInstanceAvailFlag = false;
ZeroDRMAccessAdapter * ZeroDRMAccessAdapter::mInstance = NULL;

// get current time in milliSec
long long ZeroDrmTimeCheck::drmGetCurrentTimeMS(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (long long)(t.tv_sec*1e3 + t.tv_usec*1e-3);
}

// Singleton getting instance 	
ZeroDRMAccessAdapter * ZeroDRMAccessAdapter::getInstance()
{
    if(!mInstanceAvailFlag)
	{
		mInstance = new ZeroDRMAccessAdapter();
		mInstanceAvailFlag = true;
	}
 	return mInstance;
}

// delete instance for graceful shutdown
void ZeroDRMAccessAdapter::deleteInstance()
{
    if(mInstanceAvailFlag)
	{
		delete mInstance ;
		mInstanceAvailFlag = false;
	}

}
// Constructor
ZeroDRMAccessAdapter::ZeroDRMAccessAdapter () 
{
	mCacheReUseFlag	=	true;
	mWorkerThreadId 	= 0;
	mWorkerThreadStarted = false;
	mWorkerThreadEndFlag	= false;
	mJobStatusCondVar 	= PTHREAD_COND_INITIALIZER;
	mJobPostingCondVar 	= PTHREAD_COND_INITIALIZER;
	mMutexJSCondVar	 	= PTHREAD_MUTEX_INITIALIZER;
	mMutexJPCondVar 	= PTHREAD_MUTEX_INITIALIZER;
	mMutexVar 		= PTHREAD_MUTEX_INITIALIZER;

#ifdef  PLAYBACK_ASYNC_SUPPORT
	// start the thread 
	if (0 == pthread_create(&mWorkerThreadId, NULL, &ThreadEntryFunction, this))
	{
		logprintf("[%s]->Created workerThread\n",__FUNCTION__);
	}
	else
	{
		logprintf("[%s]->Failed to create workerThread\n",__FUNCTION__);
	}
#endif
}
// Destro
ZeroDRMAccessAdapter::~ZeroDRMAccessAdapter ()
{
	if(mWorkerThreadStarted)
	{
		mWorkerThreadEndFlag = true ;
		//Send a termniation signal to thread 
		pthread_mutex_lock(&mMutexJPCondVar);
		pthread_cond_signal(&mJobPostingCondVar);
		pthread_mutex_unlock(&mMutexJPCondVar );
		void *ptr = NULL;
		int ret = pthread_join(mWorkerThreadId, &ptr);
		if (ret != 0)
		{
			logprintf("[%s]***pthread_join workerThreadId returned [%d][%s]\n",__FUNCTION__, ret, strerror(ret));
		}
		else
		{
			logprintf("[%s] Joined workerThreadId\n",__FUNCTION__);
		}
	}


	//logprintf("[%s]->Destro CtxlistSz[%d] HashTableSz[%d] WorkerQSz[%d]\n",__FUNCTION__,mContextList.size(),mKeyHashTable.size(),mZWorkerDataQue.size());
	// check and release memory incase its not released
	mCacheReUseFlag = false;
	// delete all user context if any pending 
	while (!mContextList.empty())
	{
		ContextListIter iter = mContextList.begin();
		zeroDrmFinalize(iter->first);
		mContextList.erase(iter);
	}
	// delete all cached metadata and keyinformation
	while(!mKeyHashTable.empty())
	{
		KeyHashTableIter keyiter = mKeyHashTable.begin();
        	zeroDrmDeleteMetadata(keyiter->second);
		mKeyHashTable.erase(keyiter);
	}

	// free all worker thread queue if any pending
	while(!mZWorkerDataQue.empty())
	{
		ZeroDrmWorkerData *qptr = (ZeroDrmWorkerData *)mZWorkerDataQue.front();
		zeroDrmDeleteKeyTag (&qptr->keyTag);
		mZWorkerDataQue.pop();
		delete qptr;
	}
}

////////////////////// Public API - Initialize ZeroDrm for a new context 
// For Async mode of operation , callback function and callback data is given in arguements ( default- NULL)
bool ZeroDRMAccessAdapter::zeroDrmInitialize(uint32_t &contextId , ZeroDrmStatusCallbackFnPtr fnPtr , void *callbackData)
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	bool retVal = true;
	pthread_mutex_lock(&mMutexVar);
	{
		ZeroDRMContextData *var 		= new(std::nothrow) ZeroDRMContextData();
		if(var == NULL)
		{
			logprintf("[%s]:Failed to allocate Memory \n",__FUNCTION__);
			retVal = false;
			contextId = 0;
		}
		else
		{
			var->mStatusCallbackFn	=	fnPtr ;
			var->mCallbackData	=	callbackData;
			contextId = var->getContextId();
			// store the contextId 
			mContextList[contextId] = var;	
		}
	}
	pthread_mutex_unlock( &mMutexVar );
	logprintf("[%s]:Created CtxId[%d] \n",__FUNCTION__,contextId);
	return retVal;
}


// Public API
//mContextList ( All Stream Context ) -->includes --> mCtxHashList ( all contentMetadata/receiptData hash vector)
//mKeyHashTable ( Hash vs its all keyrelated structure
void ZeroDRMAccessAdapter::zeroDrmFinalize(const uint32_t contextId)
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	pthread_mutex_lock(&mMutexVar);
	{
		logprintf("[%s]->Finalize for ContextId[%d]\n",__FUNCTION__,contextId);
		ContextListIter iter = mContextList.find(contextId);
		if(iter != mContextList.end())
		{
			ZeroDRMContextData *ctxvar = iter->second;
			// Now free all the hash keytable content associated with this stream. default is 1, for keyrotation multiple can exists 
			// If caching is enabled , need to hold it for its expiry time 
			uint32_t hashValue;
			//logprintf("[%s] Number of hashValues for this Stream before clean[%d]\n",__FUNCTION__,ctxvar->mCtxHashList.size());
			while (!ctxvar->mCtxHashList.empty())
			{
				// get all the hashvalues for the stream
				hashValue	=	ctxvar->mCtxHashList.back();
				// free those hash from KeyTable
				KeyHashTableIter keyiter = mKeyHashTable.find(hashValue);
				if(keyiter	!=	mKeyHashTable.end())
				{
					// Check if keyHashTable can be reused or its lifetime expiry happened
					ZeroDrmMetadata *metadata	=	keyiter->second;
					// if reuse flag is disabled , then free metadata and key acquired
					if(mCacheReUseFlag == false)
					{
						zeroDrmDeleteMetadata ( metadata );
						mKeyHashTable.erase(keyiter);
						zdebuglogprintf("[%s] Cache Not Enabled , delete the keyshash[%d] \n",__FUNCTION__,hashValue ) ;	
					}
					else
					{
						// dont delete , just update last accessed time 
						long long currTime = ZeroDrmTimeCheck::drmGetCurrentTimeMS();
						metadata->lastUpdateTime	=	currTime;
						zdebuglogprintf("[%s] Cache Enabled , not to delete the keys , updating the currTime for hash[%d] \n",__FUNCTION__,hashValue ) ;	
					}					
				}			
				ctxvar->mCtxHashList.pop_back();
			}
			// now that all hash value for this stream freed/reused , now remove the ctxStructure
			delete ctxvar;
			mContextList.erase(iter);
		}
		else
			logprintf("[%s]->Failed to find ContextId[%d]\n",__FUNCTION__,contextId);
	}
	pthread_mutex_unlock( &mMutexVar );
}

// Public API 
// Stores ContentMetadata tag information 
bool ZeroDRMAccessAdapter::zeroDrmSetContentMetadata(const uint32_t contextId , const unsigned char * contentdata, size_t metadataSz)
{
	// Search for Metadata in already stored ContextList ,if already existing same contentMetadata, same can be reused 
	// This will save some time during retune 	
	ZeroDrmTimeCheck t(__FUNCTION__);
	bool retVal = true;	
	
	if (contentdata == NULL || metadataSz == 0)
	{
		logprintf("[%s] Invalid Inputs \n",__FUNCTION__);
		return false;
	}

	pthread_mutex_lock(&mMutexVar);	
	{
		// first check point , if input CtdId is valid or not 
		ContextListIter iter = mContextList.find(contextId);
		if(iter != mContextList.end())
		{
			// generate a simple hash for the data given 
			// TODO: replace this with sha based hash to support license rotation
			uint32_t hashValue = JSHash(contentdata,metadataSz);
			bool bReUseMetadata = false;	
			// Its a valid Ctx , now add the hash value  . It could be reused one or fresh
			ZeroDRMContextData *ctxvar = iter->second;
			// first check if same Metadata already exists for the stream and not duplicate
			if(std::find(ctxvar->mCtxHashList.begin(),ctxvar->mCtxHashList.end(),hashValue) == ctxvar->mCtxHashList.end()) {
			// Check if keyalready exists in keyHashTable from previous channel tunes
			KeyHashTableIter keyiter = mKeyHashTable.find(hashValue);
			if(keyiter	!=	mKeyHashTable.end())
			{
				// found same hash key in our db 
				// Check if keyHashTable can be reused or its lifetime expiry happened
				ZeroDrmMetadata *metadata	=	keyiter->second;
				long long currTime = ZeroDrmTimeCheck::drmGetCurrentTimeMS();
				if(((currTime - metadata->lastUpdateTime) > ZERO_DRM_KEY_CACHE_LIFETIME) || 
					(metadata->mZeroDrmState == eZERO_DRM_STATE_RECEIPT_FAILED) || 
					(metadata->mZeroDrmState == eZERO_DRM_STATE_KEY_FAILED))
				{
					// time expired of keyvalue / or was in failure state, remove it and get fresh state
					zdebuglogprintf("[%s]-> Lifetime over/in error state[%d] for KeyTable hash [%u]\n",__FUNCTION__,metadata->mZeroDrmState,hashValue);
					zeroDrmDeleteMetadata ( metadata );
					mKeyHashTable.erase(keyiter);
					bReUseMetadata  = false;	
				}
				else
				{
					// Already same contentMetadata based Hash is added in KeyTable , may be from last channel tune
					bReUseMetadata  = true;
					metadata->lastUpdateTime	=	currTime;					
					zdebuglogprintf("[%s]-> ReUse ContentMetadata KeyTable hash [%u]\n",__FUNCTION__,hashValue);		
				}					
			}	
			
			if(!bReUseMetadata)
			{
				zdebuglogprintf("[%s] New HashKey[%u] inserted in table Sz[%d][%s] \n",__FUNCTION__,hashValue,metadataSz,contentdata);
				ZeroDrmMetadata *metadata 	=	new(std::nothrow)  ZeroDrmMetadata;
				memset (metadata, 0 ,sizeof(ZeroDrmMetadata));
				// store ContentMetadata
				metadata->metadataPtr 	= new(std::nothrow) unsigned char[metadataSz+1];
				metadata->metadataSize	=  metadataSz;
				memcpy(metadata->metadataPtr , contentdata , metadataSz);
				metadata->metadataPtr[metadataSz] = '\0'; 	
				metadata->mZeroDrmState 	=	eZERO_DRM_STATE_ACQUIRING_RECEIPT	;
				// store current updated time , needed for cleanup
				metadata->lastUpdateTime	=	ZeroDrmTimeCheck::drmGetCurrentTimeMS();
				// insert the hash into KeyTable
				mKeyHashTable[hashValue]	=	metadata;
			}
				
			// add hash to Ctx Vector	
			ctxvar->mCtxHashList.push_back (hashValue);
			}
			logprintf("[%s] Number of hashValues for this Stream[%d]\n",__FUNCTION__,ctxvar->mCtxHashList.size());
		}
		else
		{
			logprintf("[%s]->Failed to find ContextId[%d] to storeMetadata\n",__FUNCTION__,contextId);
			retVal = false;
		}
	}
	pthread_mutex_unlock( &mMutexVar );
	return retVal;	
}

// Public API 
// Stores ReceiptData tag information
bool ZeroDRMAccessAdapter::zeroDrmSetReceiptMetadata(const uint32_t contextId , const unsigned char * receiptdata, size_t metadataSz)
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	bool retVal = true;	
	
	if (receiptdata == NULL || metadataSz == 0)
	{
		logprintf("[%s] Invalid Inputs \n",__FUNCTION__);
		return false;
	}

	pthread_mutex_lock(&mMutexVar);	
	{
		// first check point , if input CtdId is valid or not 
		ContextListIter iter = mContextList.find(contextId);
		if(iter != mContextList.end())
		{
			uint32_t hashValue = JSHash(receiptdata,metadataSz);
			bool bReUseMetadata = false;	
			// Its a valid Ctx , now add the hash value  . It could be reuse or create fresh
			ZeroDRMContextData *ctxvar = iter->second;
			// first check if same Metadata already exists for the stream and not duplicate
			if(std::find(ctxvar->mCtxHashList.begin(),ctxvar->mCtxHashList.end(),hashValue) == ctxvar->mCtxHashList.end()) {
			// Check if keyalready exists in keyHashTable from previous tunes
			KeyHashTableIter keyiter = mKeyHashTable.find(hashValue);
			if(keyiter	!=	mKeyHashTable.end())
			{
				// Check if keyHashTable can be reused or its lifetime expiry happened
				ZeroDrmMetadata *metadata	=	keyiter->second;
				long long currTime = ZeroDrmTimeCheck::drmGetCurrentTimeMS();
				if(((currTime - metadata->lastUpdateTime) > ZERO_DRM_KEY_CACHE_LIFETIME) || 
					(metadata->mZeroDrmState == eZERO_DRM_STATE_KEY_FAILED))
				{
					// time expired of keyvalue, remove it 
					zdebuglogprintf("[%s]-> Lifetime over for KeyTable hash [%u]\n",__FUNCTION__,hashValue); 
					zeroDrmDeleteMetadata ( metadata );
					mKeyHashTable.erase(keyiter);
					bReUseMetadata  = false;	
				}
				else
				{
					// Already same ReceiptData based Hash is added in KeyTable , may be from last channel tune
					bReUseMetadata  = true;
					metadata->lastUpdateTime	=	currTime;					
					zdebuglogprintf("[%s]-> ReUse Receipt based KeyTable hash [%u]\n",__FUNCTION__,hashValue);		
				}					
			}	
			
			if(!bReUseMetadata)
			{
				zdebuglogprintf("[%s] New HashKey[%u] inserted in table[%d][%s] \n",__FUNCTION__,hashValue,metadataSz,receiptdata);
				ZeroDrmMetadata *metadata 	=	new(std::nothrow)  ZeroDrmMetadata;
				memset (metadata, 0 ,sizeof(ZeroDrmMetadata));
				// store ReceiptData
				metadata->receiptdataPtr 	= new(std::nothrow) unsigned char[metadataSz+1];
				metadata->recieptdataSize	=  metadataSz;
				memcpy(metadata->receiptdataPtr , receiptdata , metadataSz);	
				metadata->receiptdataPtr[metadataSz]	=	'\0';
				metadata->mZeroDrmState 	=	eZERO_DRM_STATE_ACQUIRING_KEY	;
				// store current updated time , needed for cleanup
				metadata->lastUpdateTime	=	ZeroDrmTimeCheck::drmGetCurrentTimeMS();
				// insert the hash into KeyTable
				mKeyHashTable[hashValue]	=	metadata;
			}
				
			// add hash to Ctx Vector	
			ctxvar->mCtxHashList.push_back (hashValue);
			}
			logprintf("[%s] Number of hashValues for this Stream[%d]\n",__FUNCTION__,ctxvar->mCtxHashList.size());
		}
		else
		{
			logprintf("[%s]->Failed to find ContextId[%d] to storeMetadata\n",__FUNCTION__,contextId);
			retVal = false;
		}
	}
	pthread_mutex_unlock( &mMutexVar );
	return retVal;	
}

//Public API
// Get PLayback key in Sync mode 
bool ZeroDRMAccessAdapter::zeroDrmGetPlaybackKeySync(const uint32_t contextId , const char *sTagLine )
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	bool retVal = false;		
	
	if (sTagLine == NULL )
	{
		logprintf("[%s] Invalid Inputs \n",__FUNCTION__);
		return false;
	}
	pthread_mutex_lock(&mMutexVar);
	{
		ContextListIter iter = mContextList.find(contextId);
		if(iter != mContextList.end())
		{
			ZeroDRMContextData *ctxvar = iter->second;
			// KeyRequest will work only if there is atleast on Metadata/Receipt is available
			if(ctxvar->mCtxHashList.size())
			{	
				// Now decode the keytag line 
				ZeroDrmInfo		keyTag;
				memset(&keyTag , 0 , sizeof(ZeroDrmInfo));
				if(zeroDrmParseKeyTag(keyTag ,sTagLine)== false) 
				{	
					logprintf("[%s]->Failed to parse KeyTag, return failure \n",__FUNCTION__);
				}
				else
				{
					// Check if Hash is null , ideal scenario
					//if(keyTag.CMSha1Hash == NULL)
					{
						// No license rotation , only one Metadata/Receipt for this stream 
						uint32_t hashValue = ctxvar->mCtxHashList.front();
						ZeroDrmState    retState ;
						// Final way to get key
						ZeroDrmReturn   getKeyRetVal = zeroDrmGetKey(contextId,hashValue,keyTag,retState);
						if(getKeyRetVal == eZERO_DRM_STATUS_SUCCESS)
						{
							logprintf("[%s]->Success !!! Got key Ctx[%d]hash[%u]State[%d] \n",__FUNCTION__,contextId,hashValue,retState);
							retVal = true;
						}
						else
						{
							logprintf("[%s]->Failed to get key Ctx[%d]hash[%u]State[%d]Err[%d] \n",__FUNCTION__,contextId,hashValue,retState,getKeyRetVal);
						}
					}
					//else
					//{
						// TODO : License Rotation , not enabled now
						// Use CMSha1Hash and find the hash in mCtxHashList - If not found something wrong !!!				
					//}
				}
			}
			else
			{
				logprintf("[%s]->No Metadata/Receipt available for getting key in Ctx[%d]\n",__FUNCTION__,contextId);
			}
		}
		else
		{
			logprintf("[%s]->Failed to find ContextId[%d] \n",__FUNCTION__,contextId);
		}
	}
	pthread_mutex_unlock( &mMutexVar );
	return retVal;
}

#ifdef PLAYBACK_ASYNC_SUPPORT
//Public API
// Get Playback Key in Async mode - turn on this function when playback is turned On for linear
bool ZeroDRMAccessAdapter::zeroDrmGetPlaybackKeyAsync(const uint32_t contextId , const char *sTagLine )
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	bool retVal = true;		
	
	if (sTagLine == NULL )
	{
		logprintf("[%s] Invalid Inputs \n",__FUNCTION__);
		return false;
	}
	
	pthread_mutex_lock(&mMutexVar);
	{
		ContextListIter iter = mContextList.find(contextId);
		if(iter != mContextList.end())
		{
			ZeroDRMContextData *ctxvar = iter->second;
			// KeyRequest will work only if there is atleast on Metadata/Receipt is available
			if(ctxvar->mCtxHashList.size())
			{	
				// Now decode the keytag line 
				ZeroDrmInfo		keyTag;
				memset(&keyTag , 0 , sizeof(ZeroDrmInfo));
				if(!zeroDrmParseKeyTag(keyTag ,sTagLine))
				{
					logprintf("[%s]->Failed to parse KeyTag , return failure\n",__FUNCTION__);
					retVal = false;
				} 
				else
				{
					// Check if Hash is null , ideal scenario
					//if(keyTag.CMSha1Hash == NULL)
					{
						// No license rotation , only one Metadata/Receipt for this stream 
						uint32_t hashValue = ctxvar->mCtxHashList.front();			
						// post the work to queue and trigger cond var for Task to execute it 
						// No license rotation , only one Metadata/Receipt for this stream
						ZeroDrmWorkerData *ptr = new ZeroDrmWorkerData ; 
						ptr->hashValue	=	hashValue	;
						ptr->ctxId	=	contextId	;	
						ptr->keyTag.method	=	keyTag.method;
                				ptr->keyTag.useFirst16BytesAsIV	=	keyTag.useFirst16BytesAsIV;
						memcpy(ptr->keyTag.iv , keyTag.iv , sizeof(ptr->keyTag.iv));
                				//ptr->keyTag.iv	=       keyTag.iv;
                				ptr->keyTag.uri	=	keyTag.uri;
                				ptr->keyTag.CMSha1Hash	=	 keyTag.CMSha1Hash;
						mZWorkerDataQue.push(ptr);
						// Signal the thread 	
						pthread_mutex_lock(&mMutexJPCondVar);
                        			pthread_cond_signal(&mJobPostingCondVar);
                        			pthread_mutex_unlock(&mMutexJPCondVar );
						zdebuglogprintf("[%s]-> Posting Queue element to worker thread [%d]Ctx[%d]\n",__FUNCTION__,hashValue,contextId);
					}
					//else
					//{
						// TODO : License Rotation , not enabled now
						// Use CMSha1Hash and find the hash in mCtxHashList - If not found something wrong !!!				
					//}
				}
			}
			else
			{
				logprintf("[%s]->No Metadata/Receipt available for getting key in Ctx[%d]\n",__FUNCTION__,contextId);
				retVal = false;
			}
		}
		else
		{
			logprintf("[%s]->Failed to find ContextId[%d] \n",__FUNCTION__,contextId);
			retVal = false;
		}
	}
	pthread_mutex_unlock( &mMutexVar );
	return retVal;
}
#endif
// Public API
// Decrypt the data 
ZeroDrmReturn ZeroDRMAccessAdapter::zeroDrmDecrypt(const uint32_t contextId,void *encryptedDataPtr, size_t encryptedDataLen, int timeInMs , const uint32_t hashKey )
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	ZeroDrmReturn retVal = eZERO_DRM_STATUS_SUCCESS;
	ZeroDrmMetadata *metadata = NULL;
	pthread_mutex_lock(&mMutexVar);
	retVal        =       zeroDrmGetMetadata(contextId,&metadata,hashKey);; // use default 
	pthread_mutex_unlock( &mMutexVar );
			if(metadata != NULL) {
#ifdef PLAYBACK_ASYNC_SUPPORT 
				pthread_mutex_lock(&mMutexJSCondVar);
				//logprintf("zeroDrmDecrypt Ctx[%d] DrmState[%d]\n",contextId,metadata->mZeroDrmState);
				if ((metadata->mZeroDrmState == eZERO_DRM_STATE_ACQUIRING_RECEIPT) || (metadata->mZeroDrmState == eZERO_DRM_STATE_ACQUIRING_KEY)) 
					{
						logprintf("[%s]->waiting for key acquisition to complete Ctx[%d],wait time:%d->State[%d]\n",__FUNCTION__,contextId,timeInMs ,metadata->mZeroDrmState);
						struct timespec ts;
						struct timeval tv;
						gettimeofday(&tv, NULL);
						ts.tv_sec = time(NULL) + timeInMs / 1000;
						ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000));
						ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
						ts.tv_nsec %= (1000 * 1000 * 1000);
						if(ETIMEDOUT == pthread_cond_timedwait(&mJobStatusCondVar, &mMutexJSCondVar, &ts)) // block until drm ready
						{
							logprintf("[%s]-> Ctx[%d] wait for key acquisition timed out\n", __FUNCTION__, contextId);
							retVal = eZERO_DRM_STATUS_KEY_TIMEOUT;
						}
					}
					pthread_mutex_unlock(&mMutexJSCondVar);
#endif
					if (zeroDrmIsContextIdValid(contextId) && (metadata->mZeroDrmState == eZERO_DRM_STATE_KEY_ACQUIRED )) 
					{
						Sec_Result sec_result = SEC_RESULT_FAILURE;
						SEC_BYTE *out_buffer	=	(SEC_BYTE *)malloc(encryptedDataLen);
						SEC_SIZE bytesWritten = 0;
						sec_result = SecCipher_SingleInputId(metadata->secureContext.securityProcessorContext,SEC_CIPHERALGORITHM_AES_CBC_NO_PADDING, 
							SEC_CIPHERMODE_DECRYPT, metadata->secureContext.playbackKeyOid,
							&metadata->keyTagInfo.iv[0],
							(SEC_BYTE *)encryptedDataPtr,
							encryptedDataLen,
							(SEC_BYTE *)out_buffer,
							encryptedDataLen,
							&bytesWritten);
					
						if (SEC_RESULT_SUCCESS != sec_result) 
						{
							logprintf( "[%s] -> SecCipher_SingleInputId failed,ctx[%d] result[%d]",__FUNCTION__,contextId,sec_result);
							// should we release the secProc Instance / Key or try for another attempt // or cleaned from outside of this function
							retVal = eZERO_DRM_STATUS_DECRYPT_FAILED;
						}
						else
						{
							// copy decoded data back to same stringa
							zdebuglogprintf( "[%s] -> SecCipher_SingleInputId success of len[%d]\n",__FUNCTION__,encryptedDataLen);
							memcpy(encryptedDataPtr, out_buffer, encryptedDataLen);
						}
						free (out_buffer);
					}
	}
	return retVal;
}


// To check if Context is valid and active metadata is available 
bool ZeroDRMAccessAdapter::zeroDrmIsContextActive(const uint32_t contextId )
{
	ContextListIter iter = mContextList.find(contextId);
        if(iter != mContextList.end())
	{
		ZeroDRMContextData *ctxvar = iter->second;
		// check if any metadata is added for this context
		return (!ctxvar->mCtxHashList.empty());
	}
        else
                return false;
}

// Store the recent Metadata information for a quick access 
void ZeroDRMAccessAdapter::zeroDrmSetActiveMetadata(const uint32_t contextId , ZeroDrmMetadata *metadata)
{
	ContextListIter iter = mContextList.find(contextId);
	if(iter != mContextList.end())
	{
		ZeroDRMContextData *ctxvar = iter->second;
		ctxvar->setMetadata(metadata);
	}
}
#ifdef PLAYBACK_ASYNC_SUPPORT
// Worker thread for getting receipt and Key 
void ZeroDRMAccessAdapter::zeroDRMWorkerThreadTask()
{
	logprintf("[%s]->Starting zeroDRMWorkerThreadTask \n",__FUNCTION__);	
	mWorkerThreadStarted	=	true;
	ZeroDrmWorkerData *qptr;
	do	{
		qptr = NULL;
		// Check for any jobs to do
		pthread_mutex_lock(&mMutexJPCondVar);
		pthread_cond_wait(&mJobPostingCondVar, &mMutexJPCondVar);
		if((!mWorkerThreadEndFlag) && (!mZWorkerDataQue.empty()))
		{
			//logprintf("[%s]->Got a worker in Workerthread \n",__FUNCTION__);
			 // get worker data
                        qptr = (ZeroDrmWorkerData *)mZWorkerDataQue.front();
                        mZWorkerDataQue.pop();
		}
		pthread_mutex_unlock(&mMutexJPCondVar );		
		// Got some real work to do . Take out task from work queue
		
		if(qptr) {
			
			ZeroDrmState	retState ;
			ZeroDrmReturn	getKeyRetVal = zeroDrmGetKey(qptr->ctxId , qptr->hashValue,qptr->keyTag,retState);
			if( getKeyRetVal == eZERO_DRM_STATUS_SUCCESS)
			{
				logprintf("[%s]->Success !!! Got key Ctx[%d]hash[%u] \n",__FUNCTION__,qptr->ctxId,qptr->hashValue);
			}
			else
			{
				logprintf("[%s]->Failed to get key Ctx[%d]hash[%u]Err[%d] \n",__FUNCTION__,qptr->ctxId,qptr->hashValue,getKeyRetVal);
			}	
			if(zeroDrmIsContextIdValid(qptr->ctxId))
			{
				pthread_mutex_lock(&mMutexJSCondVar);
                               pthread_cond_signal(&mJobStatusCondVar);
                                pthread_mutex_unlock(&mMutexJSCondVar );

				ContextListIter iter = mContextList.find(qptr->ctxId);
				ZeroDRMContextData *ctxvar = iter->second;
				if(ctxvar->mStatusCallbackFn)
					ctxvar->mStatusCallbackFn(retState,getKeyRetVal, ctxvar->mCallbackData);
				else
					logprintf("[%s]->No callback function registered for ZDrm Status[%d] update\n",__FUNCTION__,getKeyRetVal);
            		}
			// delete the queue element
			delete qptr;	
			qptr = NULL;	
		}// end of if 
	}while(!mWorkerThreadEndFlag); // check if anyone fired you . 
	
	logprintf("!!!! Exiting ZeroDRM Worker Thread !!!!\n");
}
#endif

// Private function to get DrmKey - this func is called by Async and Sync functions 
ZeroDrmReturn ZeroDRMAccessAdapter::zeroDrmGetKey(const uint32_t contextId ,const  uint32_t hashValue,ZeroDrmInfo &keyTag , ZeroDrmState &retState)
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	ZeroDrmReturn retVal = eZERO_DRM_STATUS_SUCCESS;
	int32_t sec_client_result = SEC_CLIENT_RESULT_FAILURE;
	const char *requestMetadata[ZERO_DRM_REQMETADATA_SZ][2];
	char *moneytracebuf = NULL;
	
	// Now get the receipt for the ContentMetadata
	memset(requestMetadata, 0, sizeof(requestMetadata));
	requestMetadata[0][0] = "X-Custom-1";
	requestMetadata[0][1] = "One";
	requestMetadata[1][0] = "X-MoneyTrace";

	zdebuglogprintf("[%s]-> Getting key for hash[%d]\n",__FUNCTION__,hashValue);	
	// this cannot happen , but a safe check to see nothing happened to HashTable
	KeyHashTableIter keyiter = mKeyHashTable.find(hashValue);
	if(keyiter	!=	mKeyHashTable.end())
	{
		ZeroDrmMetadata *metadata		=	keyiter->second;
		// Key already available , no need to fetch again 
		if(metadata->mZeroDrmState == eZERO_DRM_STATE_KEY_ACQUIRED)
		{
			logprintf("[%s]->Keyready from Cache !!!hash[%d]\n",__FUNCTION__,hashValue);
			// As key already there , delete memory for keyTag created 
			zeroDrmDeleteKeyTag(&keyTag);
			long long currTime = ZeroDrmTimeCheck::drmGetCurrentTimeMS();
			metadata->lastUpdateTime	=	currTime;
			retState	=	eZERO_DRM_STATE_KEY_ACQUIRED;
		}
		else if((metadata->mZeroDrmState != eZERO_DRM_STATE_ACQUIRING_RECEIPT) && (metadata->mZeroDrmState != eZERO_DRM_STATE_ACQUIRING_KEY))
		{
			logprintf("[%s]->Metadata in Wrong state[%d] for key\n",__FUNCTION__,metadata->mZeroDrmState);
			retVal = eZERO_DRM_STATUS_GENERIC_ERROR;
			retState        =	eZERO_DRM_STATE_INVALID;	
		}
		else
		{
			 // storing KeyTagInfo
			metadata->keyTagInfo.method             =       keyTag.method;
			metadata->keyTagInfo.useFirst16BytesAsIV        =       keyTag.useFirst16BytesAsIV;
			memcpy(metadata->keyTagInfo.iv,keyTag.iv,sizeof(metadata->keyTagInfo.iv));
			//metadata->keyTagInfo.iv                 =       keyTag.iv;
			metadata->keyTagInfo.uri                =       keyTag.uri;
			metadata->keyTagInfo.CMSha1Hash         =       keyTag.CMSha1Hash;
			metadata->keytagInfoAvailable   		=       true;
			if(metadata->mZeroDrmState == eZERO_DRM_STATE_ACQUIRING_RECEIPT)
			{
				// ContentMetadata available ,send request for receipt
				// get MoneyTrace string
				moneytracebuf = zeroDrmGetTraceId();
				requestMetadata[1][1] = moneytracebuf;
				sec_client_result = acquireRecordingReceipt(ZERO_DRM_HOST_URL, ZERO_DRM_REQMETADATA_SZ, requestMetadata,
							(const char *)metadata->metadataPtr, 0, NULL,(char **) &metadata->receiptdataPtr);
				if (sec_client_result != SEC_CLIENT_RESULT_SUCCESS)
				{
					// Use the traceId to find the cause of failure
					logprintf( "[%s]->acquireRecordingReceipt failed, result[%d]Trace[%s]",__FUNCTION__, sec_client_result,moneytracebuf);
					metadata->mDrmLastError	=	(int)sec_client_result;
					metadata->mZeroDrmState	=	eZERO_DRM_STATE_RECEIPT_FAILED;
					metadata->receiptAvailable	=	false;
					delete[] moneytracebuf;
					moneytracebuf = NULL;
					retVal = eZERO_DRM_STATUS_RECEIPT_FAILED;
				}else {
					// Now that receipt is available , get the Key now 
					metadata->recieptdataSize = strlen((const char *)metadata->receiptdataPtr);
					metadata->mZeroDrmState 	=	eZERO_DRM_STATE_ACQUIRING_KEY	;
					metadata->receiptAvailable	=	true;
					logprintf( "[%s]->Successful in getting receipt of len [%d]\n",__FUNCTION__,metadata->recieptdataSize);
					// Not to delete moneytrace , same is used for key also 
				}
			}
			// Safe to check if the user ctx is still valid , incase quick channel change is done while waitin for receipt then next step can be avoided 
			// as following may run in separate thread .
			if((zeroDrmIsContextIdValid(contextId)) && (metadata->mZeroDrmState == eZERO_DRM_STATE_ACQUIRING_KEY))
			{
				// ReceiptData available , send request for key
				if(moneytracebuf == NULL)
					moneytracebuf = zeroDrmGetTraceId();
				requestMetadata[1][1] = moneytracebuf;
				zdebuglogprintf("KeyReqTrace[%s] Metadata[%d][%s]\n",moneytracebuf,strlen((const char *)metadata->receiptdataPtr),((const char *)metadata->receiptdataPtr));
				
				memset(&metadata->secureContext, 0, sizeof(metadata->secureContext));
				sec_client_result = acquirePlaybackKey(ZERO_DRM_HOST_URL, ZERO_DRM_REQMETADATA_SZ, requestMetadata, 
								(const char *)metadata->receiptdataPtr, &metadata->secureContext);
				if (sec_client_result != SEC_CLIENT_RESULT_SUCCESS)
				{
					// Use the traceId to find the cause of failure
					logprintf( "[%s]->zeroDrmSetReceiptData acquirePlaybackKey failed, result[%d]Trace[%s]",__FUNCTION__,sec_client_result,moneytracebuf);
					metadata->mDrmLastError	=       (int)sec_client_result;
					metadata->mZeroDrmState	=	eZERO_DRM_STATE_KEY_FAILED;
					metadata->keyAvailable	=	false;	
					retVal = eZERO_DRM_STATUS_KEY_FAILED;
				}
				else
				{
					// Get the decryption context
					metadata->mZeroDrmState	=	eZERO_DRM_STATE_KEY_ACQUIRED;
					metadata->keyAvailable	=	true;
					zdebuglogprintf( "zero DRM Key Acquired \n");

					// Test functions not needed - this is to validate the keyhandle //////////////////////
					/*
					logprintf("\n********* Sec Processor Info *********\n"); 
					SecProcessor_PrintInfo(metadata->secureContext.securityProcessorContext);
					Sec_KeyHandle *keyHandle;
					Sec_Result sec_result = SecKey_GetInstance(metadata->secureContext.securityProcessorContext, metadata->secureContext.playbackKeyOid, &keyHandle);
					if (sec_result == SEC_RESULT_SUCCESS)
					{
						// Print out Key Info.
						logprintf("\n********* Key Info *********\n");
						logprintf("SecKey_IsAES result is %s \n",
							SecKey_IsAES(SecKey_GetKeyType(keyHandle)) ? "TRUE" : "FALSE");
						logprintf("SecKey_GetKeyLen  result is %d \n", SecKey_GetKeyLen(keyHandle));
					}
					else
					{
						logprintf("SecKey_GetInstance failed, result is %d \n", sec_result); 
					}
					*/
					////////////////////////////////////////////////
				}
				delete [] moneytracebuf;
				moneytracebuf = NULL;			
			}
			retState = metadata->mZeroDrmState;
		}
		// set the activeMetadata for a quick access
		zeroDrmSetActiveMetadata(contextId , metadata );
	}
	else
	{
		logprintf("[%s]->No Metadata/Receipt available for getting key[%u] \n",__FUNCTION__,hashValue);
		retVal = eZERO_DRM_STATUS_GENERIC_ERROR;
		retState = eZERO_DRM_STATE_INVALID;
	}
	return retVal;
}

// Parse the KeyTag from Manifest and filled ZeroDRmInfo structure is returned.
bool ZeroDRMAccessAdapter::zeroDrmParseKeyTag(ZeroDrmInfo	&keyTagInfo , const char *sTagLine)
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	// Parse EXT-X-KEY m3u8 tag 
	std::string inpStr = sTagLine;
	std::string whitespaces (" \t\f\v\n\r");
	std::string tagDelim = ":";
	std::string attribDelim = ",";
	std::string keyValDelim = "=";

	// trim if any additional space at the end of string
	std::size_t found = inpStr.find_last_not_of(whitespaces);
  	if (found!=std::string::npos)
    		inpStr.resize(found+1);
	// remove the tag and delim
	size_t posn = inpStr.find(tagDelim);
	inpStr.erase(0, (posn+1));
	size_t prev = 0;
	bool retValue = true;
	// loop and extract all attributes
	do
	{
		posn = inpStr.find(attribDelim, prev);
		// check if it is last attrib
		if (posn == std::string::npos) posn = inpStr.length();
		std::string token = inpStr.substr(prev, posn-prev);
		
		if (!token.empty()) 
		{
			//logprintf("posn[%d] prev[%d] inpLen[%d] tokenlen[%d] token[%s]\n",posn,prev,inpStr.length(),token.length(),token.c_str());
			size_t subposn = token.find(keyValDelim,0);
			std::string key = token.substr(0,subposn);
			std::string value = token.substr(subposn+1);
			//logprintf("subposn[%d] key[%s] valuelen[%d] value[%s]\n",subposn,key.c_str(),value.length(),value.c_str());
			if(!key.compare("METHOD"))
			{
				if(!value.compare("AES-128"))
					keyTagInfo.method = eZERO_METHOD_AES_128;
				else
					keyTagInfo.method = eZERO_METHOD_NONE;				
			}else			
			if(!key.compare("URI") && (value.length() != 0))
			{
				keyTagInfo.uri = strdup(value.c_str());				
			}else
			if(!key.compare("IV") && (value.length() > 2))
			{
				memset(keyTagInfo.iv,0,sizeof(keyTagInfo.iv));
				if(value.length() > 34) // len + 0x
                		{
                   			// expectation is iv value is of length 32
					logprintf("[%s][%d] Invalid IV Value in KeyTag [%d][%s] \n",__FUNCTION__,__LINE__,value.length(),value.c_str());	 	
					retValue = false;
				}
				else
				{
					// Convert string to hex array 
					int i,j;
                    			for (i = (int)value.length() - 1, j = 15; i >= 2; i -= 2, j--)
                    			{
                        			if(i >= 3) {
                           	 			keyTagInfo.iv[j] = getHexFromChar(value[i-1]) << 4 | getHexFromChar(value[i]);
						}
                        			else {
                            				keyTagInfo.iv[j] = getHexFromChar(value[i]);
                        			}
                    			}
				}
			}else
			if(!key.compare("CMSha1Hash") && (value.length() != 0))
			{
				value.erase(0,2);
				keyTagInfo.CMSha1Hash = (unsigned char *)strdup(value.c_str());	
			}
			
		}
		prev = posn + 1; // incrementing by delim length
	}
	while (posn < inpStr.length() && prev < inpStr.length());
	return retValue;
}

// Gets the recent Metadata based on hashKey 
// When License rotation is enabled ,based on hashkey input Metadata is picked and returned 
// When no license rotation , hashkey = 0 and current Active Metadata is returned 
ZeroDrmReturn ZeroDRMAccessAdapter::zeroDrmGetMetadata(const uint32_t contextId,ZeroDrmMetadata **metadata,const uint32_t hashKey)
{

	ContextListIter iter = mContextList.find(contextId);
	ZeroDrmReturn ret = eZERO_DRM_STATUS_SUCCESS;
	if(iter != mContextList.end())
	{
		ZeroDRMContextData *ctxvar = iter->second;
		if(hashKey == 0)
		{
			*metadata = ctxvar->getMetadata();
		}else 
		if(ctxvar->mCtxHashList.size())
		{
			// if Application calls with hashkey != 0 ->No License rotation , pop only last key
			logprintf( "[%s]->License rotation not enabled , using default key\n",__FUNCTION__);
			uint32_t hashValue = ctxvar->mCtxHashList.front();

			KeyHashTableIter keyiter = mKeyHashTable.find(hashValue);
			if(keyiter      !=      mKeyHashTable.end())
			{
				// Check if keyHashTable can be reused or its lifetime expiry happened
				*metadata       =       keyiter->second;
			}
			else
			{
				*metadata = NULL;
				ret = eZERO_DRM_STATUS_NO_METADATA;
				logprintf("[%s]->zeroDrmDecrypt No Matching key found\n",__FUNCTION__);
            }
		}
	}
	else
	{
		*metadata = NULL;
		ret = eZERO_DRM_STATUS_INVALID_CTX;
	}
	return ret;
} 

// Deletes KeyTag Information 
void ZeroDRMAccessAdapter::zeroDrmDeleteKeyTag(ZeroDrmInfo *keyTagInfo)
{
	if(keyTagInfo != NULL)
	{
		if(keyTagInfo->uri != NULL)
		{
			free(keyTagInfo->uri);
			keyTagInfo->uri = NULL;
		}
		if(keyTagInfo->CMSha1Hash != NULL)
		{
			free(keyTagInfo->CMSha1Hash);
			keyTagInfo->CMSha1Hash = NULL;
		}
	}
}

// Deletes Metadata memory if allocated 
void ZeroDRMAccessAdapter::zeroDrmDeleteMetadata(ZeroDrmMetadata *metadata)
{
	zdebuglogprintf("[%s]-> deleting [%p]\n",__FUNCTION__,metadata);
	if(metadata != NULL)
	{
		if(metadata->secureContext.securityProcessorContext != NULL)
		{
			SecKey_Delete(metadata->secureContext.securityProcessorContext, metadata->secureContext.playbackKeyOid);
		}
			
		if(metadata->metadataPtr != NULL)
		{
			delete [] metadata->metadataPtr;
			metadata->metadataPtr	=	NULL;
		}
		
		if(metadata->receiptdataPtr != NULL)
		{
			delete [] metadata->receiptdataPtr;
			metadata->receiptdataPtr = NULL;
		}
	
		zeroDrmDeleteKeyTag(&metadata->keyTagInfo);	
		delete metadata;
	}
}

// Checks if ContextId is valid or not 
bool ZeroDRMAccessAdapter::zeroDrmIsContextIdValid(const uint32_t contextId )
{
	if(mContextList.end() != mContextList.find(contextId))
		return true;
	else
		return false;
}


// Temp test function , this need to be replaced with real Sha1 based hash code
// When hash based license rotation is enabled 
uint32_t ZeroDRMAccessAdapter::JSHash(const unsigned char *str, size_t length)
 {
      uint32_t hash = 1315423911;
      for(size_t i = 0; i < length; i++)
      {
          hash ^= ((hash << 5) + str[i] + (hash >> 2));
      }
      return (hash & 0x7FFFFFFF);
 }

// TODO : Now for each request new traceId is generated . TraceId need to be picked from Caller App  
char * ZeroDRMAccessAdapter::zeroDrmGetTraceId()
{
	ZeroDrmTimeCheck t(__FUNCTION__);
	uuid_t uuid;
	char uuid_str[37];
	char *moneytracebuf= (char *) new char[128];
	// generate unique traceId
	uuid_generate_time_safe(uuid);
	//uuid_generate_time(uuid);
	uuid_unparse_lower(uuid, uuid_str);
	uuid_clear(uuid);
	snprintf(moneytracebuf,128,"trace-id=%s;parent-id=1;span-id=2",uuid_str);
	zdebuglogprintf("[%s]->MoneyTrace :: [%s]\n",__FUNCTION__,moneytracebuf);
	return moneytracebuf;
}

#endif

#ifdef TEST_CODE_ON
#define EXTTAGLINE "#EXT-X-KEY:METHOD=AES-128,URI=\"https://bet.ccp.xcal.tv\",IV=0x000102030405060708090a0b0c0d0e0f"
#define CONTENTMETADATA_1 "ZXlKNE5YUWpVekkxTmlJNklsbFNTMUpCWkZsUVYwUTJUMjV6U0ZwQ1dFMDBUMG90VEVsVFgwNUxjVGd4UmxoNldEY3RaVGRQY1djaUxDSmhiR2NpT2lKU1V6STFOaUo5LmV5SmphMjFOWlhSaFpHRjBZU0k2SWxGcVEwTkJWVWxOUVZSSldFUlVSVE5OVkVWNFRtcEZOVTVVU1hkT1ZtOU5RVlJSUlVWRVl6UmZNRGxIYWxaSE4yd3dWWEJ3YTJsQk5GWlZUVUZVVlVWR1NXNXNkUzFQVmtaVFkwTnhXbkF0WkhadE5rMU1UbDgwUzBSTFJFRkZNa0pKU0ZaTlNVaFRSRUZ3YW1FeU1EWmpSemx6WVZkT05VUkNSbXBpTWpGcVdWaE9NRXhYVG14aWJVMTBaRWhhY0dKQmQwMVpNblIwVDI1Q2RtSkhiR3BsVld4clJFSkdhbUl5TVdwWldFNHdURmRPYkdKdFRYUmtTRnB3WWtGM1Mxa3lPWFZrUjFaMVpFUndjRnBCZDFST1ZFVjZUVlJWZWs1VVVUSlBSR3N5VGtSQk0wNTZSVEpOZDNkS1draEtkRTl0ZEd4bFZXeHJSRU5SZWs5SFdUUk9WRTE2VGtNd01VNUhXVE5NVkdoc1RWUlpkRnBIUlhwTmVUQjRXVlJCTlU1RVJYbE9ha1V5VGxSblRVZHRUblppYmxKc1ltNVJObUV5VmpWU1IxWjVZVmhhYUdSSGJIWmlhM1JzWlZWc2EwUkRTbXBpTWpGcVdWaE9NRXhZUVhkTmFURnFZVEl3ZEdOSFRteGliVTR3Wkcxc2MweFVRWGROVXpGcVlUSlNja1JCUlRORVEwcHFZakl4YWxsWVRqQk1XRUYzVFdreGFtRXlNSFJqUjA1c1ltMU9NR1J0YkhOTVZFRjNUV2t4ZEZwSE1YSWlMQ0prY20xVWVYQmxJam9pWTJWdVl5SXNJbVJ5YlU1aGJXVWlPaUpqWlc1aklpd2laSEp0VUhKdlptbHNaU0k2SWtOUFRVTkJVMVF0UTBWT1F5MVVWa2xNSWl3aVkyOXVkR1Z1ZEZSNWNHVWlPaUowZG1sc0lpd2lZMjl1ZEdWdWRFTnNZWE56YVdacFkyRjBhVzl1SWpvaWREWk1hVzVsWVhJaUxDSmtjbTFMWlhsSlpDSTZJak00WmpnMU16TTBMVFUwWmpjdE9HVXhOaTFrWVRNekxURmhNRGswTVRJMk1UWTFPQ0lzSW1OcmJWQnZiR2xqZVNJNklrTlBUVU5CVTFRdFEwVk9ReTFVVmtsTUlpd2lZMjl1ZEdWdWRFbGtJam9pTlRFek1UVXpOVFEyT0RrMk5EQTNOekUyTXlJc0ltbHpjeUk2SWtOT1BWQXdNakF3TURBd01Ea3dMQ0JQVlQxMWNtNDZZMjl0WTJGemREcGpZM0E2Y0d0cExXTnpMV2x1ZERwamEyMHNJRTg5UTI5dFkyRnpkQ0JEYjI1MlpYSm5aV1FnVUhKdlpIVmpkSE1nVEV4RExDQk1QVkJvYVd4aFpHVnNjR2hwWVN3Z1UxUTlVRUVzSUVNOVZWTWlMQ0p1WW1ZaU9qRTFNVEE0TmpFNU1qVXNJbWxoZENJNk1UVXhNRGcyTVRreU5Td2lkbVZ5YzJsdmJpSTZJakVpTENKdFpYTnpZV2RsVkhsd1pTSTZJbU52Ym5SbGJuUk5aWFJoWkdGMFlTSXNJbUYxWkNJNkltUnNjeUlzSW1wMGFTSTZJbGROZGt0VFNGcFdWMUk0UVhoMlJYaGFTblZ5U1VFOVBTSjkuYmlDeE1kWUpSS3o4bE83RE8wT0ZuZUNMOHVSVW02M3N2OW5ha2laZUJfNTlYS1NVZFRfeVhCS0wxTkluU1pRZFlyNVB5bjdLZVFsS1I5Z0I2UnhxNWFLS2FKenpwMWFfZkRLSXk5ck8wR1U3TEh0VW14SGZ6RjVsRkVWVDk1ZEdFcGVsOU5FcG16RlZkRlRrZXdqYjJXMXdjVjBEYjRleVdiTm92YzJSdmxveEc0bTNlbjJva004cmpvTnYzdkZrTmZ0OHZER19RS1c0Z2tGbjJtZ2w4cEgxSlRnS3JPUzgtaV9hVlo5TTZRTkExYnhzTXZScHV2bE1OUGo0SkZJRnpFWVJpaElhSGFENWUxRGZoU2hMa1BQUVYwaW9jZkVUTEliMTU2cm0tQ3JjWE5vSU1BSVcxZ1dJOWRuWUh2a0NUNzhYaFFWUmkwQ1BTZGRkQktGTm9nDQo="

#define RECEIPTDATA "ZXlKNE5YUWpVekkxTmlJNklqbDRZbEZKWkRZdE5tVlBXSEZPTTJwMUxYQldjREJTUTJwWWNHeFJkM280UlhBemQyMU9RblZXZUc4aUxDSmhiR2NpT2lKU1V6STFOaUo5LmV5SmhZMk52ZFc1MFNXUWlPaUk0TURNd01URXdNRFU1TnpnNU9USXlNemczSWl3aVpHVjJhV05sU1dRaU9pSXdNREJDTURFd1F6QXpNREF5UmpORUlpd2laR1YyYVdObFRXOWtaV3dpT2lJd01EQkNNREV3UXpBek1EQXlSak5FSWl3aVpHVjJhV05sUTJ4aGMzTWlPaUl3TURCQ01ERXdRekF6TURBeVJqTkVJaXdpWTI5dWRHVnVkRTFsZEdGa1lYUmhJam9pV2xoc1MwNUZOVmxWVjNCV1pXdHJlRlJ0YkVwT2EyeHpZa1pPVkUxVmNFTlhhMXB6VlZaWmQxVlVTbFZOYWxZMlZUQmFkMUV4WkVaTlJFSlZUVWM1TUZaRlZuTldSbWQzVGxWNGFsWkhaRFJWYlhodlRteGtSVmt6VW1GV1IxSlJXVEZrYW1GVmVFUlRiV2hwVWpKT2NGUXliRXRWTVZZMlUxUkdUMkZWYnpWTWJWWTFVMjF3YUUxcVJrOVhiR2hUWVVad1NGSnFRbHBWTUdzeVUxZDRSMk5XUlhkVWEwcFhWa1pLVDFWV1dsTlRiR1JHVld4V1UxWkZOVTlXYTFaWFRteFNWbFZyV2s1TlJFWjRWV3hvZDFSdFNraFBWVFZTVm14S1UxVnNWbGRWVms1RlUyNW9hbUpXU2tSYVJFNVBUbGRXU0ZSVVNsZE5ia0kwV2tST2ExTkdXa1phUlZwVlZsVmFWbFpzVmxkU01VcFlUbFJhVDFaRlNrZFZWbHBMWlVVeE5sRnFTazlTUmxsM1ZsZDRZVTB3TlVaT1ZsSldZbGRTWVZsVlpEUmpNVXBHVW10YVRtRXdjRXRWTUdSdlZHeE9WbUZIZUZOU1ZWb3pXVmN4Um1WVk1VVlhiWEJUWlcxNE5sZFdXbXRVTURWV1ZXdE9ZVkl6UW5CVVYzQkhZMVprVjJGRk9VNVNXR2haVmtjeE5HRlhTbFpOVkVKaFVsZG9hRmt3WkV0U1JURlpZMGQwVTAxR2NERldNbmhIVFRGU1YyRXpiR3RUUmtwUldXMTBTMDFzYkhKYVNFNW9ZbFphVjFsclpEQlNWa1owVlc1R1dtRnJiRFJaVjNoelYxWlNjVkZyTVZkTlJGWjZWMWN3ZUZSdFVraFZhMnhZWW10S2NGVlljRWRPYkhCR1drVmthV0pJUWxOYVJFSXdWMnN4Y1dKRVJtRlNWMUpZV2taa1UxSlhUa2xSYlVaU1YwZFNWbFpIZUZOUmJWWldUMVpXV0ZaRmNFOVdhMVpHWlZaU1dXTkdTazVXVkVaV1ZteFNTMVF4V2taV1dHeFZWMGRSZWxVeWVIZFRWazUxVld4Q2FWZEdTbnBYYkZwWFl6SkZkMVZyVWxaWFIzaFBWV3RXZDJOV1VYaGFSbHBPVFVSc1ZWUldaRFJaVjBwV1RVaGtWVkp0VWxkVVYzTjRVMFpPV1ZWc2NHeGlSWEI2VmpGb2NtUXdNVlpPVm1oWVZrVndVRlpyVmt0amJGSlpZMGMxVGxKVVJrbFdSbFY0VTBkS1ZrNVVTbHBpVkZaVVdXdGtTMlJXVmxWWGJXaE9Za1pyZUZaWGRHdFdNbFpZVW14c1dHSlhhSEpWYWtvMFRXeHNkR1JFUW1sU01WcFhXV3RrTUZKV1JYZGpTRVphWVd0c05GbFhlSE5YVmxKeFVXc3hXRkpWV1hwV1JtUnlaVWRHZEZKWWJFNVRSa3B4VkZkek1XTXhiSFJOVlRsT1VqRktNRmxyYUU5VVZscEdVbXBPVlZack1UUlpWekZHWlZaV2RWTnJWbEpXVmxZMlZXdFdUMU15Um5SVFdHeE9Wak5DWVZZd1ZUQmtNVkpIWVVWS2EwMUVSbmRVVm1SM1lVVXhjVkZxUWxwbGEzQlFXV3RrUzJSR1VuRlJiWFJwVmpOb05sWkZXbE5SYlZGM1RWaENUbGRHU21oVmJuQkhaVlZzY0dReWJHRlRSWEF3Vm10b2MyUXhjRlJUVkZwS1lsVTFjMWx0TVU1aFZYaEVVMjEwYW1KVVJsQlhWbU40WWtWc2NXSXliRnBOYkZveFYxaHNTbU13YkhSVmJteHBWbXRLTlZscVNtRmpSMHBJVmxkc1VHRlZjRVZXUkVGNFVrWkdWMVJzVmsxV1ZUVkhWa2QwVG1SR1drZFhhM0JWVVhwR1ZWWnJWa2RUUmtwVVUxaE9TbUpWTlRKWmJUVlRZa2RLZFZWc1ZteFhSVXB6VTFkd2RtRlhVa2xYYmtKcFVUQnNlbE5YTVU5a2JVcDFWVzE0YVdKc1NrVlphMlJIWlcxTmVXSkhNV2hXTURWdldrVmtjMlJ0U25CVFZGcEtZbXhGZVZaRlpITmtWbkJZVW01c1NtRllaSEJYYTJoTFpFWk5lVlpxVmxSV01VWndWREpzU21WVk1VVlRiWEJRVmpGVmVsUXhUWGhpUm5CMFZGUkNUVll4VlhsVVZXUktaRVpzTmxWdGVGcGxWRUY0Vkd4a1drMXJOVlZSYlhST1pXMWpkMVJWWkU1aFZYaEVVMjF3YUUxcVJsSlpha28wWTBacmVtRXliRkJoVlhCRlZrUkJlRkpHUmxkVWJGWk5WbFUxUjFaSGRFNWtSbHBIVjJ0d1ZWRjZSbFZXYTFaSFUwWktWRk5ZVGtwaVZUVXlXVzAxVTJKSFNuVlZhM0JoVVRCck1sTlhjRlprTURGeFlYcEtUMkZyVmpOVWJYQk9UVVUxVlZKVVJrOWhiRlkwVkcxd1RtRlZlRVJUYmtKcVRUQXhjRlF5YkV0U1JsSnhUVlpHVGxKRmJETlVWVkpDWkRBeFJWRlVWazVSTTJSdVZrUkdWazlYVWxsVGJsWlFZbFUxTWxsc1pFOWhSMDE2VlZSYVdrMXJOVE5VTWpWRFkyMUdWRTFYY0dwbFZFWjNXVzAxVWs1c2EzbGtTRkpOVVRCS1VWVkdWazlrYlVwWVZHMW9hazB4Um01VlZFazFaRmRTZEZadWJHRk5iRnB5VTFWYVEyVlhTWGxWYWtaYVRURktObE5WVmpSVVZrWTFaREprVlZKRVJsSlpWV1J6WXpGc1dGVnRlR2xUUlVwMldWWmtSbU13YkVkVWJGWlJWbXRLUTFSRlRrTlNSa0pYVm14U1NtRllaSEJaYlRGTFlsVnNjV0l6YUU5V1JWWTFWRlZTV21WVk5VVmhNMnhOVVRCd2QxZFdhRkpoVlRseFVsUkdUbFpGYkROVWJYQktUVVU1VlZOWVRrcGliSEJ6V1RJMVQyTkhTWGxPUjJ4UVlWVnNORk5YYkROaFYwcFlWbTV3YWsxclduVlhiRnBUVGxkT1NGWlhiRkJoVlhCeFdXcEpNVTFHY0ZoT1ZFSlZWakZaZDFkV1pGTmhSMUpJVWxkc1RWRXdjRzlhUm1SU1lWVTVjRk50ZEdsVFJURndWRVZPUzJOWFVraGhNbXhRWVZWc05sVXdXbUZpUjBwSFZHMXNXRll6YUhsWFZFSkhWa1pLYzFGclZreE5NWEJ2Vm14U1YwMHhRbFZOUjJ4dFZWTTFibGRXYURGV00xWTBXVEU1YkdOSWNHcFdNbFpDVlc1S1NHVnJSbWxhVm14Q1lWVXdNR0pYY0ZwVk0wWjJaVWRLUjFKVmRITlhSbEpwVGtSV1dWWkVWWFJTVlZKSFkwWk9NbUZzYXpCbFZWWllVbFphUzJKcmFFcE5Wa0Y2WTIxU2FreFdWbUZrYWtKcFZUSk9WRTFZVm5wV1ZFSnFZV3MxUmxWc1RuWk5lbVF4VkcweFFrNUlTVE5PZWtZeVZXeHNhVkZ0Um10YVJrcElZbnBPVDJWc1VsSlZNVVpYVjFSQmVsTnJWalZrVkVwWlpVUkdTbFpzT1V0TlIwWlhXVzE0VUZNeVRYUlZSM1J2Vkd0T1RGa3dTazVVUlhOM1pFWTVZVm95ZUVWWU1UaDBaVmMxVkdGRVZtRmlNR3hRV1ZSb1dWTlZTazVSYlZKbVZWUk9iazVyVGtSUk1tUXpZV3Q0UTFNemFIWmxia0V5VW5wUk1XRXpSbEZoV0d4b1UxVjBVbU5GTUhSVFJYZ3hXWGt4ZVV4WE1WSlRWR3hFVkZka1NWSXdiSEphTWxwSllVUk9RazVHU20xTlYxcFpaRU14V1dKVlJqRldXRXBWV1hwU1NsUkVRbXBrTVZwMFZXNXdURTlHUWtoWk0zQlJZMnR3UkdJelVsbGpNbXg2WTBoYU5FMUZkRTFqUlZWNlYyNXJOR1J0ZHpSTlJURkNXVlJrZWxGcVdsRlNiV2Q2WTFac1MxVnJUbHBQVkZKdFpVZHZkMWRxUm5Wa1ZVNDJZVEIzTW1SWFRtOWpNRlpYV214RlBTSXNJbXhwWTJWdWMyVkRiR0Z6Y3lJNklsTkVJaXdpYVhOeklqb2lRMDQ5VURBeU1UQXdNREE0TlVJc0lFOVZQWFZ5YmpwamIyMWpZWE4wT21OamNEcHdhMmt0WTNNdGFXNTBPbVJzY3pwdFpITXNJRTg5UTI5dFkyRnpkQ0JEYjI1MlpYSm5aV1FnVUhKdlpIVmpkSE1nVEV4RExDQk1QVkJvYVd4aFpHVnNjR2hwWVN3Z1UxUTlVRUVzSUVNOVZWTWlMQ0pwWVhRaU9qRTFNVE13TURVNU56SXNJbTVpWmlJNk1UVXhNekF3TWpNM01pd2laWGh3SWpveE5UUTBOVFF4T1RjeUxDSjJaWEp6YVc5dUlqb2lNU0lzSW0xbGMzTmhaMlZVZVhCbElqb2ljbVZqYjNKa2FXNW5VbVZqWldsd2RDSXNJbUYxWkNJNkltUnNjeUlzSW1wMGFTSTZJbEJZWldoT1VHMUZlbkpHVEVScVNpOVdVa3AyYzJjOVBTSjkuRkZWT0xqWFFnNm9xMlVzb2hNdVU5T2Q2NWkyaUEyUWRhdGhBUHNTNG5jV1N3djhGV0dZczhNa2pRSnFsRzR3aTNYSXFwLXBIems5VTRqR0Raa3lDTXN0dWRlQnhkZm5mcW84azR3SDFsMjhibXlZVTVNVU9aY29BV0V5VkE0clhrYmt2UG1WUlRBYzlqSXVQci1sakUya3hLNGJXMDkwMHhObC1KTEhpOUxIUVBsOF9zalZZZEQ1aUhWTUsxbHV6TjBaMDREWHVXUHh5SmFzYWNEUS1xWXRhRUh2OUdLLUtOQUVJcHpDZ1ZHVm9JQlV5RFRKV2o5SkY4LVVoUGNHR2RNbFN5ZWNUemZMcWxYWTV1WVFZdnNiRnhEQTVub0JWTXBENHRoUUlNSWVNcXJxU1JVdmJ6eVZPWUZRdS1MZlQwWTB0SGxlaXI2VmdzQWhTQTYwLTVR"

#define CONTENTMETADATA "ZXlKNE5YUWpVekkxTmlJNklsbFNTMUpCWkZsUVYwUTJUMjV6U0ZwQ1dFMDBUMG90VEVsVFgwNUxjVGd4UmxoNldEY3RaVGRQY1djaUxDSmhiR2NpT2lKU1V6STFOaUo5LmV5SmphMjFOWlhSaFpHRjBZU0k2SWxGcVEwTkJWVFJOUVZSSldFUlVSVE5OVkVWNlRVUkZNMDFxUlhwTmJHOU5RVlJSUlVWUVNESnhjbVJDZDNONWVHTTJWMnB4ZDNkSFZFZEZUVUZVVlVWR1JXNTZOVEJGUVZKeE16QjJORFYwVWxaM05FNVRVbWRaYUd4c1JFRkZNa0pKU0doTlNVaGxSRUZ3YW1FeU1EWmpSemx6WVZkT05VUkNaR3BpTWpGcVdWaE9NRXhYVG14aWJVMTBaRWhhY0dKRE1YcGtSMFp1V2xGM1RWa3lkSFJQYmtKMllrZHNhbVZWYkd0RVFtUnFZakl4YWxsWVRqQk1WMDVzWW0xTmRHUklXbkJpUXpGNlpFZEdibHBSZDB0Wk1qbDFaRWRXZFdSRWNIQmFRWGRVVGxSQmVVOVVXVEpOVkVFeVRYcFJNVTFVVlRKT1ZFVXlUWGQzU2xwSVNuUlBiWFJzWlZWc2EwUkRVWGxOUkVwcVQxZFZNMDlUTVd4YWJVMHdURmRWTWsxSFNYUlplbEpzV1hrd01VNVhXVEpPVkVKclRYcG5NRTFIVFUxSGJVNTJZbTVTYkdKdVVUWmhNbFkxVWtkV2VXRllXbWhrUjJ4MlltdDBiR1ZWYkd0RVEwcHFZakl4YWxsWVRqQk1XRUYzVFdreGFtRXlNSFJqTWs1c1ltMU9NR1J0YkhOTVZFRjNUVk14YW1FeVVuSkVRVVV6UkVOS2FtSXlNV3BaV0U0d1RGaEJkMDFwTVdwaE1qQjBZekpPYkdKdFRqQmtiV3h6VEZSQmQwMXBNWFJhUnpGeUlpd2laSEp0Vkhsd1pTSTZJbU5sYm1NaUxDSmtjbTFPWVcxbElqb2lZMlZ1WXlJc0ltUnliVkJ5YjJacGJHVWlPaUpEVDAxRFFWTlVMVU5GVGtNdFZGWkpUQzFUVkVGSFJTSXNJbU52Ym5SbGJuUlVlWEJsSWpvaWRIWnBiQ0lzSW1OdmJuUmxiblJEYkdGemMybG1hV05oZEdsdmJpSTZJblEyVEdsdVpXRnlJaXdpWkhKdFMyVjVTV1FpT2lJeU1ESmpPV1UzT1MxbFptTTBMV1UyTUdJdFl6UmxZeTAxTldZMk5UQmtNemcwTUdNaUxDSmphMjFRYjJ4cFkza2lPaUpEVDAxRFFWTlVMVU5GVGtNdFZGWkpUQzFUVkVGSFJTSXNJbU52Ym5SbGJuUkpaQ0k2SWpVd01qazJOakV3TmpNME5URTFOalV4TmpNaUxDSnBjM01pT2lKRFRqMVFNREl3TURBd01EQTVNQ3dnVDFVOWRYSnVPbU52YldOaGMzUTZZMk53T25CcmFTMWpjeTFwYm5RNlkydHRMQ0JQUFVOdmJXTmhjM1FnUTI5dWRtVnlaMlZrSUZCeWIyUjFZM1J6SUV4TVF5d2dURDFRYUdsc1lXUmxiSEJvYVdFc0lGTlVQVkJCTENCRFBWVlRJaXdpYm1KbUlqb3hOVEV5TURZeU5Ea3lMQ0pwWVhRaU9qRTFNVEl3TmpJME9USXNJblpsY25OcGIyNGlPaUl4SWl3aWJXVnpjMkZuWlZSNWNHVWlPaUpqYjI1MFpXNTBUV1YwWVdSaGRHRWlMQ0poZFdRaU9pSmtiSE1pTENKcWRHa2lPaUl6U0ZabGJGTmlXV3hyWTBGVFJsQkVLM1poVlRWM1BUMGlmUS5nWVh1V3V4Y19lcHpjV2VBUnJHekFiZVlBaU00bWpZU3FveGJGRUtsWFRiNDVYVDUtRURGcFN2alk0eUVXRVZKbkhJMVAzcmRjLVVadjBiU2NTMXVzVTBjak5FUlNvMzd1Tm1BNHI3NzF2UlliQmFkZFJHbzNOelRRU1FWWTAzSkV5dTJYeDFJVl9KMGFWYmxPS2MtUGtoTkNLY0JNTEswdF9aZ2xEX18teW5TaDVab0lPYThYSUJNQmRfUTNnNkNDQ2d3akxCS3hvenA2RzQ1a3FQaXlhSUtRcE0tSEx1Yy1yLW1RSTlDTWdIR0lrZ2ZIaDNBNFJmMWZYdC1YbUF1VXJUYzRJTDBjd1ZtUnpLOFBHY3pQckpDb3RYc2lzcHZ4MEtMcEUzWnk4dmw4ME1BYTdzQjZQRmgzcVlKUkNZOTRmeGowWjFudUN6a0w2dWNoc0VWZlE="


//1. For every channel tune , call Initialize and get a Ctx for interaction .
//2. At the end of channel tune , call ReleaseCtx to free up all memory related with Ctx 
//3. One Manifest can have multiple ContentMetadata and Multiple Keys if license rotation is supported 
//4. Call InitializeContentMetadata(for each ContentMetadata with Ctx as input ) -> This API stores the ContentMetadata 
//5. For recording Scenario , InitializeReceiptMetadata will store with Ctx as input ) 
//6. GetKey (with Ctx and KeyTag String) -> this will request for Key fo

//Per ContentMetadata : 
   
void ZeroDRMCallbackFunction(ZeroDrmState drmState , int errRet,void *callbackData)
{
	logprintf("!!! Received callback from ZeroDRMAccessAdapter state[%d] error[%d]\n",drmState,errRet);
}

int main(int argc, char** argv)
{
	ZeroDRMAccessAdapter *inst = ZeroDRMAccessAdapter::getInstance();
	sleep(3);
	uint32_t myCtx = 0;
	// playback scenario 
#if 1
	////// Sync Calls ///////////
	inst->zeroDrmInitialize(myCtx); // get Context from Init call 
	// Store all the ContentMetadata in manifest file (No request is made here) for key
	inst->zeroDrmSetContentMetadata(myCtx,(const unsigned char *)CONTENTMETADATA , strlen(CONTENTMETADATA));
	// When KeyTag is parsed , Key is requested .
	inst->zeroDrmGetPlaybackKeySync(myCtx , EXTTAGLINE );	
	///// Decrypt call here 
	inst->zeroDrmSetContentMetadata(myCtx,(const unsigned char *)CONTENTMETADATA , strlen(CONTENTMETADATA));
	inst->zeroDrmFinalize(myCtx);
	logprintf("------------------->>> End of Ctx[%d] \n",myCtx);
	sleep(10);
#endif 
	// for recording case // with direct receipt data
#if 0
	// Sync call 
	inst->zeroDrmInitialize(myCtx); // get Context from Init call
	// Store all the ContentMetadata in manifest file (No request is made here) for key
	inst->zeroDrmSetReceiptMetadata(myCtx,(const unsigned char *)RECEIPTDATA , strlen(RECEIPTDATA));
	inst->zeroDrmGetPlaybackKeySync(myCtx , EXTTAGLINE );
	sleep(10);
	inst->zeroDrmFinalize(myCtx);
	logprintf("------------------->>> End of Ctx[%d] \n",myCtx);
#endif
	
    //////

#if 1	
	///////////// Async Calls ////////////////////
	inst->zeroDrmInitialize(myCtx ,ZeroDRMCallbackFunction , NULL ); // get Context from Init call 
	// Store all the ContentMetadata in manifest file (No request is made here) for key
	inst->zeroDrmSetContentMetadata(myCtx,(const unsigned char *)CONTENTMETADATA , strlen(CONTENTMETADATA));
	inst->zeroDrmGetPlaybackKeyAsync(myCtx , EXTTAGLINE );
	sleep(10);	
#if 1	
	size_t size;
  	char * memblock;

	logprintf("\n !!! Opening file [%s] for decoding \n",argv[1]);
  	std::ifstream file (argv[1], std::ios::in|std::ios::binary|std::ios::ate);
  	if (file.is_open())
  	{
		size = file.tellg();
		memblock = new char [size];
		file.seekg (0, std::ios::beg);
		file.read (memblock, size);
		file.close();

		ZeroDrmReturn retval = inst->zeroDrmDecrypt( myCtx , (void *)memblock , size,3000 );
		if(retval != eZERO_DRM_STATUS_SUCCESS)
		{
			logprintf("Failed in zeroDrmDecrypt , error [ %d]  \n",retval);
		}
		else
		{
			std::ofstream opfile("op.ts",std::ios::binary);
			if (opfile.is_open())	
			{
				opfile.write(memblock,size);
				opfile.close();
			}
		}
		file.close();
		delete[] memblock;
	}
#endif
	inst->zeroDrmFinalize(myCtx);
#endif
	inst->deleteInstance();
	logprintf("\nEnd of test program \n");
}

#endif 

