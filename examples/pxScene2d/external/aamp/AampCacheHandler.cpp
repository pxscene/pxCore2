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

#include "AampCacheHandler.h"

/**
 * @brief Insert playlist to playlist cache
 * @param url URL corresponding to playlist
 * @param buffer Contains the playlist
 * @param effectiveUrl Effective URL of playlist
 */
void AampCacheHandler::InsertToPlaylistCache(const std::string url, const GrowableBuffer* buffer, std::string effectiveUrl,bool trackLiveStatus,MediaType fileType)
{
	PlayListCachedData *tmpData,*newtmpData;
	pthread_mutex_lock(&mMutex);

	// First check point , Caching is allowed only if its VOD and for Main Manifest(HLS) for both VOD/Live
	// For Main manifest , fileType will bypass storing for live content
	if(trackLiveStatus==false || fileType==eMEDIATYPE_MANIFEST)
	{

		PlaylistCacheIter it = mPlaylistCache.find(url);
		if (it != mPlaylistCache.end())
		{
			logprintf("%s:%d : playlist %s already present in cache", __FUNCTION__, __LINE__, url.c_str());
		}
		// insert only if buffer size is less than Max size
		else
		{
			if(fileType==eMEDIATYPE_MANIFEST && mPlaylistCache.size())
			{
				// If new Manifest is inserted which is not present in the cache , flush out other playlist files related with old manifest,
				ClearPlaylistCache();
			}
			if(buffer->len < mMaxPlaylistCacheSize)
			{
				// Before inserting into cache, need to check if max cache size will exceed or not on adding new data
				// if more , need to pop out some from same type of playlist
				bool cacheStoreReady = true;
				if(mCacheStoredSize + buffer->len > mMaxPlaylistCacheSize)
				{
					AAMPLOG_WARN("[%s][%d] Count[%d]Avail[%d]Needed[%d] Reached max cache size ",__FUNCTION__,__LINE__,mPlaylistCache.size(),mCacheStoredSize,buffer->len);
					cacheStoreReady = AllocatePlaylistCacheSlot(fileType,buffer->len);
				}
				if(cacheStoreReady)
				{
					tmpData = new PlayListCachedData();
					tmpData->mCachedBuffer = new GrowableBuffer();
					memset (tmpData->mCachedBuffer, 0, sizeof(GrowableBuffer));
					aamp_AppendBytes(tmpData->mCachedBuffer, buffer->ptr, buffer->len );

					tmpData->mEffectiveUrl = effectiveUrl;
					tmpData->mFileType = fileType;
					mPlaylistCache[url] = tmpData;
					mCacheStoredSize += buffer->len;
					AAMPLOG_INFO("[%s][%d]  Inserted. url %s", __FUNCTION__, __LINE__, url.c_str());
					// There are cases where Main url and effective url will be different ( for Main manifest)
					// Need to store both the entries with same content data 
					// When retune happens within aamp due to failure , effective url wll be asked to read from cached manifest
					// When retune happens from JS , regular Main url will be asked to read from cached manifest. 
					// So need to have two entries in cache table but both pointing to same CachedBuffer (no space is consumed for storage)
					{						
						// if n only there is diff in url , need to store both
						if(url != effectiveUrl)
						{
							newtmpData = new PlayListCachedData();
							// Not to allocate for Cachebuffer again , use the same buffer as above
							newtmpData->mCachedBuffer = tmpData->mCachedBuffer;
							newtmpData->mEffectiveUrl = effectiveUrl;
							// This is a duplicate entry
							newtmpData->mDuplicateEntry = true;
							newtmpData->mFileType = fileType;
							mPlaylistCache[effectiveUrl] = newtmpData;
							AAMPLOG_INFO("[%s][%d]Added an effective url entry %s", __FUNCTION__, __LINE__, effectiveUrl.c_str());
						}
					}
				}
			}
		}
	}
	pthread_mutex_unlock(&mMutex);
}


/**
 * @brief Retrieve playlist from playlist cache
 * @param url URL corresponding to playlist
 * @param[out] buffer Output buffer containing playlist
 * @param[out] effectiveUrl effective URL of retrieved playlist
 * @retval true if playlist is successfully retrieved.
 */
bool AampCacheHandler::RetrieveFromPlaylistCache(const std::string url, GrowableBuffer* buffer, std::string& effectiveUrl)
{
	GrowableBuffer* buf = NULL;
	bool ret;
	std::string eUrl;
	pthread_mutex_lock(&mMutex);
	PlaylistCacheIter it = mPlaylistCache.find(url);
	if (it != mPlaylistCache.end())
	{
		PlayListCachedData *tmpData = it->second;
		buf = tmpData->mCachedBuffer;
		eUrl = tmpData->mEffectiveUrl;
		buffer->len = 0;
		aamp_AppendBytes(buffer, buf->ptr, buf->len );
		effectiveUrl = eUrl;
		traceprintf("%s:%d : url %s found", __FUNCTION__, __LINE__, url.c_str());
		ret = true;
	}
	else
	{
		traceprintf("%s:%d : url %s not found", __FUNCTION__, __LINE__, url.c_str());
		ret = false;
	}
	pthread_mutex_unlock(&mMutex);
	return ret;
}


/**
 * @brief Clear playlist cache
 */
void AampCacheHandler::ClearPlaylistCache()
{
	AAMPLOG_INFO("%s:%d : cache size %d", __FUNCTION__, __LINE__, (int)mPlaylistCache.size());
	PlaylistCacheIter it = mPlaylistCache.begin();
	for (;it != mPlaylistCache.end(); it++)
	{
		PlayListCachedData *tmpData = it->second;
		if(!tmpData->mDuplicateEntry)
		{
			aamp_Free(&tmpData->mCachedBuffer->ptr);
			delete tmpData->mCachedBuffer;
			tmpData->mCachedBuffer = NULL;
		}
		delete tmpData;
	}
	mCacheStoredSize = 0;
	mPlaylistCache.clear();
}

/**
 * @brief AllocatePlaylistCacheSlot Allocate Slot for adding new playlist
 */
bool AampCacheHandler::AllocatePlaylistCacheSlot(MediaType fileType,size_t newLen)
{
	bool retVal = true;
	size_t freedSize=0;
	if(mPlaylistCache.size())
	{
		if(fileType == eMEDIATYPE_MANIFEST)
		{
			// This case cannot happen, but for safety need to handle.
			// If for any reason Main Manifest is pushed after cache is full , better clear all the playlist cached .
			// As per new Main Manifest  ,new  playlist files need to be downloaded and cached
			ClearPlaylistCache();
		}
		else // for non main manifest
		{
		PlaylistCacheIter Iter = mPlaylistCache.begin();
		// Two pass to remove the item from cache to create space for caching
		// First pass : Search for same file type to clean, If Video need to be inserted , free another Video type
		// 				if audio type to be inserted , remove older audio type . Same for iframe .
		// Second pass : Even after removing same file type entry ,still not enough space to add new item then remove from other file type ( rare scenario)
		while(Iter != mPlaylistCache.end())
		{
			PlayListCachedData *tmpData = Iter->second;
			if(tmpData->mFileType == eMEDIATYPE_MANIFEST || tmpData->mFileType != fileType)
			{ 	// Not to remove main manifest file and filetype which are different
				Iter++;
				continue;
			}
			if(!tmpData->mDuplicateEntry)
			{
				freedSize += tmpData->mCachedBuffer->len;
				aamp_Free(&tmpData->mCachedBuffer->ptr);
				delete tmpData->mCachedBuffer;
				tmpData->mCachedBuffer = NULL;
			}
			delete tmpData;
			Iter = mPlaylistCache.erase(Iter);
		}
		//Second Pass - if still more cleanup required for space, remove  from other playlist types
		if(freedSize < newLen)
		{
			Iter = mPlaylistCache.begin();
			while(Iter != mPlaylistCache.end())
			{
				PlayListCachedData *tmpData = Iter->second;
				if(tmpData->mFileType == eMEDIATYPE_MANIFEST)
				{ 	// Not to remove main manifest file
					Iter++;
					continue;
				}
				if(!tmpData->mDuplicateEntry)
				{
					freedSize += tmpData->mCachedBuffer->len;
					aamp_Free(&tmpData->mCachedBuffer->ptr);
					delete tmpData->mCachedBuffer;
					tmpData->mCachedBuffer = NULL;                                
				}
				delete tmpData;
				Iter = mPlaylistCache.erase(Iter);
			}
		}
		mCacheStoredSize -= freedSize;
		// After all freeing still size is not enough to insert , better not allow to insert such huge file
		if(freedSize < newLen)
			retVal = false;
		}
	}
	return retVal;
}

AampCacheHandler::AampCacheHandler():
	mCacheStoredSize(0),mAsyncThreadStartedFlag(false),mAsyncCleanUpTaskThreadId(0),mCacheActive(false),
	mAsyncCacheCleanUpThread(false),mMutex(),mCondVarMutex(),mCondVar(),mPlaylistCache()
	,mMaxPlaylistCacheSize(MAX_PLAYLIST_CACHE_SIZE)
{
	pthread_mutex_init(&mMutex, NULL);
	pthread_mutex_init(&mCondVarMutex, NULL);
	pthread_cond_init(&mCondVar, NULL);
	if(0 != pthread_create(&mAsyncCleanUpTaskThreadId, NULL, &AampCacheThreadFunction, this))
	{
		AAMPLOG_ERR("Failed to create AampCacheHandler thread errno = %d, %s", errno, strerror(errno));
	}
	else
	{
		mAsyncThreadStartedFlag = true;
	}
}

/**
 * @brief Destructor Function
 */
AampCacheHandler::~AampCacheHandler()
{
	mCacheActive = true;
	mAsyncCacheCleanUpThread = false;
	pthread_mutex_lock(&mCondVarMutex);
	pthread_cond_signal(&mCondVar);
	pthread_mutex_unlock(&mCondVarMutex );
	if(mAsyncThreadStartedFlag)
	{
		void *ptr = NULL;
		int rc = pthread_join(mAsyncCleanUpTaskThreadId, &ptr);
		if (rc != 0)
		{
			AAMPLOG_ERR("***pthread_join AsyncCacheCleanUpTask returned %d(%s)", rc, strerror(rc));
		}
	}
	ClearPlaylistCache();
	pthread_mutex_destroy(&mMutex);
	pthread_mutex_destroy(&mCondVarMutex);
	pthread_cond_destroy(&mCondVar);

}

/**
 *	 @brief Start playlist caching
 *
 *	 @return void
 */
void AampCacheHandler::StartPlaylistCache()
{
	mCacheActive = true;
	pthread_mutex_lock(&mCondVarMutex);
	pthread_cond_signal(&mCondVar);
	pthread_mutex_unlock(&mCondVarMutex );
}
/**
 *	 @brief Stop playlist caching
 *
 *	 @return void
 */
void AampCacheHandler::StopPlaylistCache()
{
	mCacheActive = false;
	pthread_mutex_lock(&mCondVarMutex);
	pthread_cond_signal(&mCondVar);
	pthread_mutex_unlock(&mCondVarMutex );
}

/**
 *	 @brief Thread function for Async Cache clean
 *
 *	 @return void
 */
void AampCacheHandler::AsyncCacheCleanUpTask()
{
	mAsyncCacheCleanUpThread	=	true;
	do{
		pthread_mutex_lock( &mCondVarMutex );
		pthread_cond_wait( &mCondVar, &mCondVarMutex );
		if(!mCacheActive)
		{
			int timeInMs = 10000;
			struct timespec ts;
			struct timeval tv;
			gettimeofday(&tv, NULL);
			ts.tv_sec = time(NULL) + timeInMs / 1000;
			ts.tv_nsec = (long)(tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000));
			ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
			ts.tv_nsec %= (1000 * 1000 * 1000);

			if(ETIMEDOUT == pthread_cond_timedwait(&mCondVar, &mCondVarMutex, &ts))
			{
				AAMPLOG_INFO("%s:%d[%p] Cacheflush timed out", __FUNCTION__, __LINE__, this);
				ClearPlaylistCache();
			}
		}
		pthread_mutex_unlock( &mCondVarMutex );
	}while(mAsyncCacheCleanUpThread);
}


/**
*   @brief SetMaxPlaylistCacheSize - Set Max Cache Size
*
*   @param[in] cacheSz- CacheSize
*   @return None
*/
void AampCacheHandler::SetMaxPlaylistCacheSize(int maxPlaylistCacheSz)
{
	pthread_mutex_lock(&mMutex);
	mMaxPlaylistCacheSize = maxPlaylistCacheSz;
	AAMPLOG_WARN("%s Setting mMaxPlaylistCacheSize to :%d",__FUNCTION__,maxPlaylistCacheSz);
	pthread_mutex_unlock(&mMutex);	
}
/**
*   @brief IsUrlCached - Check if URL is already cached
*
*   @return bool - true if file found, else false
*/
bool AampCacheHandler::IsUrlCached(std::string url)
{
	bool retval = false;
	pthread_mutex_lock(&mMutex);
	PlaylistCacheIter it = mPlaylistCache.find(url);
	if (it != mPlaylistCache.end())
		retval = true;

	pthread_mutex_unlock(&mMutex);
	return retval;
}
