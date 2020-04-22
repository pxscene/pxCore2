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

#ifndef __AAMP_CACHE_HANDLER_H__
#define __AAMP_CACHE_HANDLER_H__

#include <iostream>
#include <memory>
#include <unordered_map>
#include "priv_aamp.h"

/**
 * @brief PlayListCachedData structure to store playlist data
 */
typedef struct playlistcacheddata{
	std::string mEffectiveUrl;
	GrowableBuffer* mCachedBuffer;
	MediaType mFileType;
	bool mDuplicateEntry;

	playlistcacheddata() : mEffectiveUrl(""), mCachedBuffer(NULL), mFileType(eMEDIATYPE_DEFAULT),mDuplicateEntry(false)
	{
	}

	playlistcacheddata(const playlistcacheddata& p) : mEffectiveUrl(p.mEffectiveUrl), mCachedBuffer(p.mCachedBuffer), mFileType(p.mFileType),mDuplicateEntry(p.mDuplicateEntry)
	{
		mCachedBuffer->ptr = p.mCachedBuffer->ptr;
		mCachedBuffer->len = p.mCachedBuffer->len;
		mCachedBuffer->avail = p.mCachedBuffer->avail;
	}

	playlistcacheddata& operator=(const playlistcacheddata &p)
	{
		mEffectiveUrl = p.mEffectiveUrl;
		mCachedBuffer = p.mCachedBuffer;
		mFileType = p.mFileType;
		mDuplicateEntry = p.mDuplicateEntry;
		return *this;
	}

}PlayListCachedData;


class AampCacheHandler
{

private:
	typedef std::unordered_map<std::string, PlayListCachedData *> PlaylistCache ;
	typedef std::unordered_map<std::string, PlayListCachedData *>::iterator PlaylistCacheIter;
	PlaylistCache mPlaylistCache;
	int mCacheStoredSize;
	bool mCacheActive;
	bool mAsyncCacheCleanUpThread;
	bool mAsyncThreadStartedFlag;
	int mMaxPlaylistCacheSize;
	pthread_mutex_t mMutex;
	pthread_mutex_t mCondVarMutex;
	pthread_cond_t mCondVar ;
	pthread_t mAsyncCleanUpTaskThreadId;
private:

	/**
	 *	 @brief Async Cache Cleanup task
	 *
	 *	 @return void
	 */
	void AsyncCacheCleanUpTask();
	/**
	 *	 @brief Thread entry function
	 *
	 *	 @return void
	 */
	static void * AampCacheThreadFunction(void * This) {((AampCacheHandler *)This)->AsyncCacheCleanUpTask(); return NULL;}
	/**
	 *	 @brief Clear playlist cache
	 *
	 *	 @return void
	 */
	void ClearPlaylistCache();
	/**
	 *   @brief AllocatePlaylistCacheSlot Freeup Playlist cache for new playlist caching
	 *   @param[in] fileType - Indicate the type of playlist to store/remove
	 *   @param[in] newLen  - Size required to store new playlist
	 *
	 *   @return bool Success or Failure
	 */
	bool AllocatePlaylistCacheSlot(MediaType fileType,size_t newLen);

public:

	/**
	 *	 @brief Default Constructor
	 *
	 *	 @return void
	 */
	AampCacheHandler();

	/**
	* @brief Destructor Function
	*/
	~AampCacheHandler();

	/**
	 *	 @brief Start playlist caching
	 *
	 *	 @return void
	 */
	void StartPlaylistCache();
	/**
	 *	 @brief Stop playlist caching
	 *
	 *	 @return void
	 */
	void StopPlaylistCache();

	/**
	 *   @brief Insert playlist into cache
	 *
	 *   @param[in] url - URL
	 *   @param[in] buffer - Pointer to growable buffer
	 *   @param[in] effectiveUrl - Final URL
	 *   @param[in] trackLiveStatus - Live Status of the track inserted
	 *   @param[in] fileType - Type of the file inserted
     *
	 *   @return void
	 */
	void InsertToPlaylistCache(const std::string url, const GrowableBuffer* buffer, std::string effectiveUrl,bool trackLiveStatus,MediaType fileType=eMEDIATYPE_DEFAULT);

	/**
	 *   @brief Retrieve playlist from cache
	 *
	 *   @param[in] url - URL
	 *   @param[out] buffer - Pointer to growable buffer
	 *   @param[out] effectiveUrl - Final URL
	 *   @return true: found, false: not found
	 */
	bool RetrieveFromPlaylistCache(const std::string url, GrowableBuffer* buffer, std::string& effectiveUrl);

	/**
	*   @brief SetMaxPlaylistCacheSize - Set Max Cache Size
	*
	*   @param[in] cacheSz- CacheSize
	*   @return None
	*/
	void SetMaxPlaylistCacheSize(int);
	/**
	*   @brief GetMaxPlaylistCacheSize - Get present CacheSize
	*
	*   @return int - maxCacheSize
	*/
	int  GetMaxPlaylistCacheSize() { return mMaxPlaylistCacheSize; }
	/**
	*   @brief IsUrlCached - Check if URL is already cached
	*
	*   @return bool - true if file found, else false
	*/
	bool IsUrlCached(std::string);

	// Copy constructor and Copy assignment disabled 
	AampCacheHandler(const AampCacheHandler&) = delete;
	AampCacheHandler& operator=(const AampCacheHandler&) = delete;
};


#endif
