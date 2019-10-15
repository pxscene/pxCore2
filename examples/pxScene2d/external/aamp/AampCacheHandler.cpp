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

AampCacheHandler * AampCacheHandler::mInstance = NULL;

/**
 * @brief Insert playlist to playlist cache
 * @param url URL corresponding to playlist
 * @param buffer Contains the playlist
 * @param effectiveUrl Effective URL of playlist
 */
void AampCacheHandler::InsertToPlaylistCache(const std::string url, const GrowableBuffer* buffer, const char* effectiveUrl,bool trackLiveStatus,MediaType fileType)
{
	PlayListCachedData *tmpData;
	pthread_mutex_lock(&mMutex);

	// First check point , Caching is allowed only if its VOD and for Main Manifest(HLS) for both VOD/Live
	// For Main manifest , fileType will bypass storing for live content
	if(trackLiveStatus==false || fileType==eMEDIATYPE_MANIFEST)
	{

		PlaylistCacheIter it = mPlaylistCache.find(url);
		if (it != mPlaylistCache.end())
		{
			logprintf("%s:%d : playlist %s already present in cache\n", __FUNCTION__, __LINE__, url.c_str());
		}
		// insert only if buffer size is less than Max size
		else
		{
			if(fileType==eMEDIATYPE_MANIFEST && mPlaylistCache.size())
			{
				// If new Manifest is inserted which is not present in the cache , flush out other playlist files related with old manifest,
				ClearPlaylistCache();
			}
			if(buffer->len < gpGlobalConfig->gMaxPlaylistCacheSize)
			{
				// Before inserting into cache, need to check if max cache size will exceed or not on adding new data
				// if more , need to pop out some from same type of playlist
				bool cacheStoreReady = true;
				if(mCacheStoredSize + buffer->len > gpGlobalConfig->gMaxPlaylistCacheSize)
				{
					AAMPLOG_WARN("[%s][%d] Count[%d]Avail[%d]Needed[%d] Reached max cache size \n",__FUNCTION__,__LINE__,mPlaylistCache.size(),mCacheStoredSize,buffer->len);
					cacheStoreReady = AllocatePlaylistCacheSlot(fileType,buffer->len);
				}
				if(cacheStoreReady)
				{
					tmpData = new PlayListCachedData();
					tmpData->mCachedBuffer = new GrowableBuffer();
					memset (tmpData->mCachedBuffer, 0, sizeof(GrowableBuffer));
					aamp_AppendBytes(tmpData->mCachedBuffer, buffer->ptr, buffer->len );
					tmpData->mEffectiveUrl= new char[MAX_URI_LENGTH];
					strncpy(tmpData->mEffectiveUrl, effectiveUrl, MAX_URI_LENGTH);
					tmpData->mFileType = fileType;
					mPlaylistCache[url] = tmpData;
					mCacheStoredSize += buffer->len;
					AAMPLOG_INFO("[%s][%d]  Inserted. url %s %s\n", __FUNCTION__, __LINE__, url.c_str(),effectiveUrl);
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
bool AampCacheHandler::RetrieveFromPlaylistCache(const std::string url, GrowableBuffer* buffer, char effectiveUrl[])
{
	GrowableBuffer* buf = NULL;
	bool ret;
	char* eUrl;
	pthread_mutex_lock(&mMutex);
	PlaylistCacheIter it = mPlaylistCache.find(url);
	if (it != mPlaylistCache.end())
	{
		PlayListCachedData *tmpData = it->second;
		buf = tmpData->mCachedBuffer;
		eUrl = tmpData->mEffectiveUrl;
		buffer->len = 0;
		aamp_AppendBytes(buffer, buf->ptr, buf->len );
		strncpy(effectiveUrl,eUrl, MAX_URI_LENGTH);
		traceprintf("%s:%d : url %s found\n", __FUNCTION__, __LINE__, url.c_str());
		ret = true;
	}
	else
	{
		traceprintf("%s:%d : url %s not found\n", __FUNCTION__, __LINE__, url.c_str());
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
	AAMPLOG_INFO("%s:%d : cache size %d\n", __FUNCTION__, __LINE__, (int)mPlaylistCache.size());
	PlaylistCacheIter it = mPlaylistCache.begin();
	for (;it != mPlaylistCache.end(); it++)
	{
		PlayListCachedData *tmpData = it->second;
		aamp_Free(&tmpData->mCachedBuffer->ptr);
		delete[] tmpData->mEffectiveUrl;
		delete tmpData->mCachedBuffer;
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
		while(Iter != mPlaylistCache.end()  && (freedSize < newLen))
		{
			PlayListCachedData *tmpData = Iter->second;
			if(tmpData->mFileType == eMEDIATYPE_MANIFEST || tmpData->mFileType != fileType)
			{ 	// Not to remove main manifest file and filetype which are different
				Iter++;
				continue;
			}
			freedSize += tmpData->mCachedBuffer->len;
			aamp_Free(&tmpData->mCachedBuffer->ptr);
			delete[] tmpData->mEffectiveUrl;
			delete tmpData->mCachedBuffer;
			delete tmpData;
			Iter = mPlaylistCache.erase(Iter);
		}
		//Second Pass - if still more cleanup required for space, remove  from other playlist types
		Iter = mPlaylistCache.begin();
		while(Iter != mPlaylistCache.end()  && (freedSize < newLen))
		{
			PlayListCachedData *tmpData = Iter->second;
			if(tmpData->mFileType == eMEDIATYPE_MANIFEST)
			{ 	// Not to remove main manifest file
				Iter++;
				continue;
			}
			freedSize += tmpData->mCachedBuffer->len;
			aamp_Free(&tmpData->mCachedBuffer->ptr);
			delete[] tmpData->mEffectiveUrl;
			delete tmpData->mCachedBuffer;
			delete tmpData;
			Iter = mPlaylistCache.erase(Iter);
		}
		mCacheStoredSize -= freedSize;
		// After all freeing still size is not enough to insert , better not allow to insert such huge file
		if(freedSize < newLen)
			retVal = false;
		}
	}
	return retVal;
}


/**
 * @brief Create Singleton Instance of AampCacheHandler
 */
AampCacheHandler * AampCacheHandler::GetInstance()
{
	if(mInstance == NULL)
	{
		mInstance = new AampCacheHandler();
	}
	return mInstance;
}


AampCacheHandler::AampCacheHandler():
	mCacheStoredSize(0),mAsyncThreadStartedFlag(false),mAsyncCleanUpTaskThreadId(0),mCacheActive(false),
	mAsyncCacheCleanUpThread(false),mMutex(),mCondVarMutex(),mCondVar(),mPlaylistCache()
{
	pthread_mutex_init(&mMutex, NULL);
	pthread_mutex_init(&mCondVarMutex, NULL);
	pthread_cond_init(&mCondVar, NULL);
	if(0 != pthread_create(&mAsyncCleanUpTaskThreadId, NULL, &AampCacheThreadFunction, this))
	{
		AAMPLOG_ERR("Failed to create AampCacheHandler thread errno = %d, %s\n", errno, strerror(errno));
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
			AAMPLOG_ERR("***pthread_join AsyncCacheCleanUpTask returned %d(%s)\n", rc, strerror(rc));
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
				AAMPLOG_INFO("%s:%d[%p] Cacheflush timed out\n", __FUNCTION__, __LINE__, this);
				ClearPlaylistCache();
			}
		}
		pthread_mutex_unlock( &mCondVarMutex );
	}while(mAsyncCacheCleanUpThread);
}
