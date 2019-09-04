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
 * @file HlsDrmBase.h
 * @brief Declaration common to various HLS DRM implementations
 */


#ifndef _DRM_HLSDRMBASE_H_
#define _DRM_HLSDRMBASE_H_

#include "priv_aamp.h"

/**
 * @enum DrmReturn
 * @brief Return values of various functions
 */
enum DrmReturn
{
	eDRM_SUCCESS,
	eDRM_ERROR,
	eDRM_KEY_ACQUSITION_TIMEOUT
};

/**
 * @enum DRMState
 * @brief States of DRM object
 */
enum DRMState
{
	eDRM_INITIALIZED,
	eDRM_ACQUIRING_KEY,
	eDRM_KEY_ACQUIRED,
	eDRM_KEY_FAILED,
	eDRM_KEY_FLUSH
};

/**
 * @class HlsDrmBase
 * @brief Base class of HLS DRM implementations
 */
class HlsDrmBase
{
public:

	/**
	 * @brief Set DRM specific meta-data
	 *
	 * @param aamp AAMP instance to be associated with this decryptor
	 * @param metadata DRM specific metadata
	 * @retval 0 on success
	 */
	virtual DrmReturn SetMetaData( class PrivateInstanceAAMP *aamp, void* metadata,int trackType) = 0;

	/**
	 * @brief Set information required for decryption
	 *
	 * @param aamp AAMP instance to be associated with this decryptor
	 * @param drmInfo Drm information
	 * @retval eDRM_SUCCESS on success
	 */
	virtual DrmReturn SetDecryptInfo( PrivateInstanceAAMP *aamp, const struct DrmInfo *drmInfo) = 0;


	/**
	 * @brief Decrypts an encrypted buffer
	 * @param bucketType Type of bucket for profiling
	 * @param encryptedDataPtr pointer to encyrpted payload
	 * @param encryptedDataLen length in bytes of data pointed to by encryptedDataPtr
	 * @param timeInMs wait time
	 * @retval eDRM_SUCCESS on success
	 */
	virtual DrmReturn Decrypt(ProfilerBucketType bucketType, void *encryptedDataPtr, size_t encryptedDataLen, int timeInMs = 3000) = 0;

	/**
	 * @brief Release drm session
	 */
	virtual void Release() = 0;

	/**
	 * @brief Cancel timed_wait operation drm_Decrypt
	 */
	virtual void CancelKeyWait() = 0;

	/**
	 * @brief Restore key state post cleanup of
	 * audio/video TrackState in case DRM data is persisted
	 */
	virtual void RestoreKeyState() = 0;
	/**
	* @brief AcquireKey Function to get DRM Key
	*
	*/
	virtual void AcquireKey( class PrivateInstanceAAMP *aamp, void *metadata,int trackType) = 0;
	/**
	* @brief GetState Function to get current DRM state
	*
	*/
	virtual DRMState GetState() = 0;
	/**
	 * @brief HlsDrmBase Destructor
	 */
	virtual ~HlsDrmBase(){};

};
#endif /* _DRM_HLSDRMBASE_H_ */
