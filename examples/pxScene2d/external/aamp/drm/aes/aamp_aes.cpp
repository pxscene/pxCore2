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
 * @file aamp_aes.cpp
 * @brief HLS AES drm decryptor
 */


#include "aamp_aes.h"

#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>


#define AES_128_KEY_LEN_BYTES 16

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief key acquistion thread
 * @param arg AesDec pointer
 * @retval NULL
 */
static void * acquire_key(void* arg)
{
	AesDec *aesDec = (AesDec *)arg;
	aesDec->AcquireKey();
	return NULL;
}

/**
 * @brief Notify drm error
 * @param drmFailure drm error type
 */
void AesDec::NotifyDRMError(AAMPTuneFailure drmFailure)
{
	//If downloads are disabled, don't send error event upstream
	if (mpAamp->DownloadsAreEnabled())
	{
		mpAamp->DisableDownloads();
		if(AAMP_TUNE_UNTRACKED_DRM_ERROR == drmFailure)
		{
			char description[128] = {};
			sprintf(description, "AAMP: DRM Failure");
			mpAamp->SendErrorEvent(drmFailure, description);
		}
		else
		{
			mpAamp->SendErrorEvent(drmFailure);
		}
	}
	SignalDrmError();
	logprintf("AesDec::NotifyDRMError: drmState:%d\n", mDrmState );
}


/**
 * @brief Signal drm error
 */
void AesDec::SignalDrmError()
{
	pthread_mutex_lock(&mMutex);
	mDrmState = eDRM_KEY_FAILED;
	pthread_cond_broadcast(&mCond);
	pthread_mutex_unlock(&mMutex);
}


/**
 * @brief Signal key acquired event
 */
void AesDec::SignalKeyAcquired()
{
	logprintf("aamp:AesDRMListener %s drmState:%d moving to KeyAcquired\n", __FUNCTION__, mDrmState);
	pthread_mutex_lock(&mMutex);
	mDrmState = eDRM_KEY_ACQUIRED;
	pthread_cond_broadcast(&mCond);
	pthread_mutex_unlock(&mMutex);
	mpAamp->LogDrmInitComplete();
}


/**
 * @brief Acquire drm key from URI
 */
void AesDec::AcquireKey()
{
	char tempEffectiveUrl[MAX_URI_LENGTH];
	long http_error;
	bool keyAcquisitionStatus = false;
	AAMPTuneFailure failureReason = AAMP_TUNE_UNTRACKED_DRM_ERROR;

	if (aamp_pthread_setname(pthread_self(), "aampAesKey"))
	{
		logprintf("%s:%d: pthread_setname_np failed\n", __FUNCTION__, __LINE__);
	}
	logprintf("%s:%d: Key acquisition start uri = %s\n", __FUNCTION__, __LINE__, mDrmInfo.uri);
	bool fetched = mpAamp->GetFile(mDrmInfo.uri, &mAesKeyBuf, tempEffectiveUrl, &http_error, NULL, mCurlInstance, true, eMEDIATYPE_LICENCE);
	if (fetched)
	{
		if (AES_128_KEY_LEN_BYTES == mAesKeyBuf.len)
		{
			logprintf("%s:%d: Key fetch success len = %d\n", __FUNCTION__, __LINE__, (int)mAesKeyBuf.len);
			keyAcquisitionStatus = true;
		}
		else
		{
			logprintf("%s:%d: Error Key fetch - size %d\n", __FUNCTION__, __LINE__, (int)mAesKeyBuf.len);
			failureReason = AAMP_TUNE_INVALID_DRM_KEY;
		}
	}
	else
	{
		logprintf("%s:%d: Key fetch failed\n", __FUNCTION__, __LINE__);
		if (http_error == CURLE_OPERATION_TIMEDOUT)
		{
			failureReason = AAMP_TUNE_LICENCE_TIMEOUT;
		}
		else
		{
			failureReason = AAMP_TUNE_LICENCE_REQUEST_FAILED;
		}
	}

	if(keyAcquisitionStatus)
	{
		SignalKeyAcquired();
	}
	else
	{
		aamp_Free(&mAesKeyBuf.ptr); //To cleanup previous successful key if any
		NotifyDRMError(failureReason);
	}
}


/**
 * @brief Set DRM meta-data. Stub implementation
 *
 * @param aamp AAMP instance to be associated with this decryptor
 * @param metadata - Ignored
 *
 * @retval eDRM_SUCCESS
 */
DrmReturn AesDec::SetMetaData( PrivateInstanceAAMP *aamp, void* metadata,int trackType)
{
	return eDRM_SUCCESS;
}

/**
 * @brief AcquireKey Function to acquire key . Stub implementation
 *
 * @param[in] aamp       AAMP instance to be associated with this decryptor
 * @param[in] metadata   Ignored
 *
 * @retval None
 */
void AesDec::AcquireKey( class PrivateInstanceAAMP *aamp, void *metadata,int trackType)
{

}

/**
 * @brief GetState Function to get current DRM State
 *
 *
 * @retval DRMState
 */
DRMState AesDec::GetState()
{
	return mDrmState;
}

/**
 * @brief Set information required for decryption
 *
 * @param aamp AAMP instance to be associated with this decryptor
 * @param drmInfo Drm information
 * @retval eDRM_SUCCESS on success
 */
DrmReturn AesDec::SetDecryptInfo( PrivateInstanceAAMP *aamp, const struct DrmInfo *drmInfo)
{
	DrmReturn err = eDRM_ERROR;
	pthread_mutex_lock(&mMutex);
	if (mDrmState == eDRM_ACQUIRING_KEY)
	{
		logprintf("AesDec::%s:%d acquiring key in progress\n",__FUNCTION__, __LINE__);
		WaitForKeyAcquireCompleteUnlocked(20*1000, err);
	}
	mpAamp = aamp;
	mDrmInfo = *drmInfo;

	if (mDrmUrl)
	{
		if ((eDRM_KEY_ACQUIRED == mDrmState) && (0 == strcmp(mDrmUrl, drmInfo->uri)))
		{
			logprintf("AesDec::%s:%d same url:%s - not acquiring key\n",__FUNCTION__, __LINE__, mDrmUrl);
			pthread_mutex_unlock(&mMutex);
			return eDRM_SUCCESS;
		}
		free(mDrmUrl);
	}
	mDrmUrl = strdup(drmInfo->uri);
	mDrmState = eDRM_ACQUIRING_KEY;
	mPrevDrmState = eDRM_INITIALIZED;
	if (-1 == mCurlInstance)
	{
		mCurlInstance = AAMP_TRACK_COUNT;
		aamp->CurlInit(mCurlInstance, 1);
	}

	if (licenseAcquisitionThreadStarted)
	{
		int ret = pthread_join(licenseAcquisitionThreadId, NULL);
		if (ret != 0)
		{
			logprintf("AesDec::%s:%d pthread_join failed for license acquisition thread: %d\n", __FUNCTION__, __LINE__, licenseAcquisitionThreadId);
		}
		licenseAcquisitionThreadStarted = false;
	}

	int ret = pthread_create(&licenseAcquisitionThreadId, NULL, acquire_key, this);
	if(ret != 0)
	{
		logprintf("AesDec::%s:%d pthread_create failed for acquire_key with errno = %d, %s\n", __FUNCTION__, __LINE__, errno, strerror(errno));
		mDrmState = eDRM_KEY_FAILED;
		licenseAcquisitionThreadStarted = false;
	}
	else
	{
		err = eDRM_SUCCESS;
		licenseAcquisitionThreadStarted = true;
	}
	pthread_mutex_unlock(&mMutex);
	AAMPLOG_INFO("AesDec::%s:%d drmState:%d \n",__FUNCTION__, __LINE__, mDrmState);
	return err;
}

/**
 * @brief Wait for key acquisition completion
 * @param[in] timeInMs timeout
 * @param[out] err error on failure
 */
void AesDec::WaitForKeyAcquireCompleteUnlocked(int timeInMs, DrmReturn &err )
{
	struct timespec ts;
	struct timeval tv;
	AAMPLOG_INFO( "aamp:waiting for key acquisition to complete,wait time:%d\n",timeInMs );
	gettimeofday(&tv, NULL);
	ts.tv_sec = time(NULL) + timeInMs / 1000;
	ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000));
	ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
	ts.tv_nsec %= (1000 * 1000 * 1000);

	if(0 != pthread_cond_timedwait(&mCond, &mMutex, &ts)) // block until drm ready
	{
		logprintf("AesDec::%s:%d wait for key acquisition timed out\n", __FUNCTION__, __LINE__);
		err = eDRM_KEY_ACQUSITION_TIMEOUT;
	}
}


/**
 * @brief Decrypts an encrypted buffer
 * @param bucketType Type of bucket for profiling
 * @param encryptedDataPtr pointer to encyrpted payload
 * @param encryptedDataLen length in bytes of data pointed to by encryptedDataPtr
 * @param timeInMs wait time
 */
DrmReturn AesDec::Decrypt( ProfilerBucketType bucketType, void *encryptedDataPtr, size_t encryptedDataLen,int timeInMs)
{
	DrmReturn err = eDRM_ERROR;

	pthread_mutex_lock(&mMutex);
	if (mDrmState == eDRM_ACQUIRING_KEY)
	{
		WaitForKeyAcquireCompleteUnlocked(timeInMs, err);
	}
	if (mDrmState == eDRM_KEY_ACQUIRED)
	{
		AAMPLOG_INFO("AesDec::%s:%d Starting decrypt\n", __FUNCTION__, __LINE__);
		unsigned char *decryptedDataBuf = (unsigned char *)malloc(encryptedDataLen);
		int decryptedDataLen = 0;
		if (decryptedDataBuf)
		{
			int decLen = encryptedDataLen;
			memset(decryptedDataBuf, 0, encryptedDataLen);
			mpAamp->LogDrmDecryptBegin(bucketType);
			if(!EVP_DecryptInit_ex(&mOpensslCtx, EVP_aes_128_cbc(), NULL, (unsigned char*)mAesKeyBuf.ptr, mDrmInfo.iv))
			{
				logprintf( "AesDec::%s:%d: EVP_DecryptInit_ex failed mDrmState = %d\n",  __FUNCTION__, __LINE__, (int)mDrmState);
			}
			else
			{
				if (!EVP_DecryptUpdate(&mOpensslCtx, decryptedDataBuf, &decLen, (const unsigned char*) encryptedDataPtr,
				        encryptedDataLen))
				{
					logprintf("AesDec::%s:%d: EVP_DecryptUpdate failed mDrmState = %d\n", __FUNCTION__, __LINE__, (int) mDrmState);
				}
				else
				{
					decryptedDataLen = decLen;
					decLen = 0;
					AAMPLOG_INFO("AesDec::%s:%d: EVP_DecryptUpdate success decryptedDataLen = %d encryptedDataLen %d\n", __FUNCTION__, __LINE__, (int) decryptedDataLen, (int)encryptedDataLen);
					if (!EVP_DecryptFinal_ex(&mOpensslCtx, decryptedDataBuf + decryptedDataLen, &decLen))
					{
						logprintf("AesDec::%s:%d: EVP_DecryptFinal_ex failed mDrmState = %d\n", __FUNCTION__, __LINE__,
						        (int) mDrmState);
					}
					else
					{
						decryptedDataLen += decLen;
						AAMPLOG_INFO("AesDec::%s:%d decrypt success\n", __FUNCTION__, __LINE__);
						err = eDRM_SUCCESS;
					}
				}
			}
			mpAamp->LogDrmDecryptEnd(bucketType);

			memcpy(encryptedDataPtr, decryptedDataBuf, encryptedDataLen);
			free(decryptedDataBuf);
		}
	}
	else
	{
		logprintf( "AesDec::%s:%d:key acquisition failure! mDrmState = %d\n",  __FUNCTION__, __LINE__, (int)mDrmState);
	}
	pthread_mutex_unlock(&mMutex);
	return err;
}


/**
 * @brief Release drm session
 */
void AesDec::Release()
{
	DrmReturn err = eDRM_ERROR;
	pthread_mutex_lock(&mMutex);
	//We wait for license acquisition to complete. Once license acquisition is complete
	//the appropriate state will be set to mDrmState and hence RestoreKeyState will be a no-op.
	if (mDrmState == eDRM_ACQUIRING_KEY || mPrevDrmState == eDRM_ACQUIRING_KEY)
	{
		WaitForKeyAcquireCompleteUnlocked(20*1000, err);
	}
	if (licenseAcquisitionThreadStarted)
	{
		int ret = pthread_join(licenseAcquisitionThreadId, NULL);
		if (ret != 0)
		{
			logprintf("AesDec::%s:%d pthread_join failed for license acquisition thread: %d\n", __FUNCTION__, __LINE__, licenseAcquisitionThreadId);
		}
		licenseAcquisitionThreadStarted = false;
	}
	pthread_cond_broadcast(&mCond);
	if (-1 != mCurlInstance)
	{
		if (mpAamp)
		{
			mpAamp->SyncBegin();
			mpAamp->CurlTerm(mCurlInstance, 1);
			mpAamp->SyncEnd();
		}
		mCurlInstance = -1;
	}
	pthread_mutex_unlock(&mMutex);
}


/**
 * @brief Cancel timed_wait operation drm_Decrypt
 *
 */
void AesDec::CancelKeyWait()
{
	pthread_mutex_lock(&mMutex);
	//save the current state in case required to restore later.
	if (mDrmState != eDRM_KEY_FLUSH)
	{
		mPrevDrmState = mDrmState;
	}
	//required for demuxed assets where the other track might be waiting on mMutex lock.
	mDrmState = eDRM_KEY_FLUSH;
	pthread_cond_broadcast(&mCond);
	pthread_mutex_unlock(&mMutex);
}


/**
 * @brief Restore key state post cleanup of
 * audio/video TrackState in case DRM data is persisted
 */
void AesDec::RestoreKeyState()
{
	pthread_mutex_lock(&mMutex);
	//In case somebody overwritten mDrmState before restore operation, keep that state
	if (mDrmState == eDRM_KEY_FLUSH)
	{
		mDrmState = mPrevDrmState;
	}
	pthread_mutex_unlock(&mMutex);
}

std::shared_ptr<AesDec> AesDec::mInstance = nullptr;

/**
 * @brief Get singleton instance
 */
std::shared_ptr<AesDec> AesDec::GetInstance()
{
	pthread_mutex_lock(&mutex);
	if (nullptr == mInstance)
	{
		mInstance = std::make_shared<AesDec>();
	}
	pthread_mutex_unlock(&mutex);
	return mInstance;
}

/**
 * @brief AesDec Constructor
 * @retval
 */
AesDec::AesDec() : mpAamp(nullptr), mDrmState(eDRM_INITIALIZED),
		mPrevDrmState(eDRM_INITIALIZED), mDrmUrl(nullptr),
		mCond(), mMutex(), mOpensslCtx(),
		mDrmInfo(), mAesKeyBuf(), mCurlInstance(-1),
		licenseAcquisitionThreadId(),
		licenseAcquisitionThreadStarted(false)
{
	pthread_cond_init(&mCond, NULL);
	pthread_mutex_init(&mMutex, NULL);
	EVP_CIPHER_CTX_init(&mOpensslCtx);
	memset( &mDrmInfo, 0 , sizeof(DrmInfo));
}


/**
 * @brief AesDec Destructor
 */
AesDec::~AesDec()
{
	CancelKeyWait();
	Release();
	pthread_mutex_destroy(&mMutex);
	pthread_cond_destroy(&mCond);
	EVP_CIPHER_CTX_cleanup(&mOpensslCtx);
}
