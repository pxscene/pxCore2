/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// rtFileCache.cpp

#include <rtFileCache.h>
#include <pxOffscreen.h>
#include <pxUtil.h>
#include <string.h>
#include <sstream>
#include <dirent.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "rtSettings.h"

#define DEFAULT_MAX_CACHE_SIZE 20971520

using namespace std;

rtFileCache* rtFileCache::instance()
{
  if (NULL == mCache)
  {
    mCache = new  rtFileCache();
  }
  return mCache;
}

void rtFileCache::destroy()
{
  if (NULL != mCache)
  {
    delete mCache;
  }
  mCache = NULL;
}

rtFileCache* rtFileCache::mCache = NULL;
rtFileCache::rtFileCache():mMaxSize(DEFAULT_MAX_CACHE_SIZE),mCurrentSize(0),mDirectory("/tmp/cache"),mCacheMutex()
{
  char const *s = getenv("SPARK_CACHE_DIRECTORY");
  if (s)
  {
    if (strlen(s) > 0)
    {
      mDirectory = s;
    }
  }
  rtValue cacheDirectory;
  if (RT_OK == rtSettings::instance()->value("cacheDirectory", cacheDirectory))
  {
    rtLogInfo("using the rtSettings value for the cache");
    mDirectory = cacheDirectory.toString();
  }
  rtLogInfo("The cache directory is set to %s", mDirectory.cString());
  mFileSizeMap.clear();
  mFileTimeMap.clear();
  initCache();
}

rtFileCache::~rtFileCache()
{
  mMaxSize = 0;
  mCurrentSize = 0;
  mDirectory = "";
  mFileSizeMap.clear();
  mFileTimeMap.clear();
}

void  rtFileCache::initCache()
{
  int retVal = -1;
#ifdef RT_PLATFORM_WINDOWS
  retVal  = mkdir(mDirectory.cString());
#else
  retVal = mkdir(mDirectory.cString(), 0777);
#endif
  if (0 != retVal)
    rtLogWarn("creation of cache directory %s failed: %d", mDirectory.cString(), retVal);
  populateExistingFiles();
}

void rtFileCache::populateExistingFiles()
{
  mFileTimeMap.clear();
  mFileSizeMap.clear();
  DIR *directory;
  struct dirent *direntry;
  struct stat buf;
  int exists = 0;
  directory = opendir(mDirectory.cString());

  if (NULL == directory) {
    return;
  }

  for (direntry = readdir(directory); direntry != NULL; direntry = readdir(directory))
  {
    if ((strcmp(direntry->d_name,".") !=0 ) && (strcmp(direntry->d_name,"..") != 0))
    {
      rtString filename = mDirectory;
      filename.append("/");
      filename.append(direntry->d_name);
      exists = stat(filename.cString(), &buf);
      if (exists < 0)
      {
        rtLogWarn("Reading the cache directory is failed for file(%s)",filename.cString());
        continue;
      }
#if defined(PX_PLATFORM_MAC)
       mFileTimeMap.insert(make_pair(buf.st_atimespec.tv_sec,direntry->d_name));
#elif !(defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS))
       mFileTimeMap.insert(make_pair(buf.st_atim.tv_sec,direntry->d_name));
#else
       rtLogWarn("Platform not supported. Cache will not get cleared after cache limit is reached");
#endif
      mFileSizeMap[direntry->d_name] = buf.st_size;
      mCurrentSize += buf.st_size;
    }
  }
  closedir(directory);
}

rtError rtFileCache::setMaxCacheSize(int64_t bytes)
{
  mMaxSize = bytes;
  return RT_OK;
}

int64_t rtFileCache::maxCacheSize()
{
  return mMaxSize;
}

int64_t rtFileCache::cacheSize()
{
  return mCurrentSize;
}

rtError rtFileCache::setCacheDirectory(const char* directory)
{
  if ((NULL == directory) || (0 == strlen(directory)))
  {
    return RT_ERROR;
  }
  mDirectory = directory;

  int retVal = -1;
#ifdef RT_PLATFORM_WINDOWS
  retVal = mkdir(mDirectory.cString());
#else
  retVal = mkdir(mDirectory.cString(), 0777);
#endif //RT_PLATFORM_WINDOWS
  if (0 != retVal)
    rtLogWarn("creation of cache directory(%s) failed", mDirectory.cString());
  populateExistingFiles();
  return RT_OK;
}

rtError rtFileCache::cacheDirectory(rtString& dir)
{
  if (mDirectory.isEmpty())
    return RT_ERROR;
  dir = mDirectory;
  return RT_OK;
}

void rtFileCache::eraseData(rtString& filename)
{
  if (! filename.isEmpty())
  {
    mCacheMutex.lock();
    mCurrentSize = mCurrentSize - mFileSizeMap[filename];
    mFileSizeMap.erase(filename);
    multimap<time_t,rtString>::iterator iter = mFileTimeMap.begin();
    while (iter != mFileTimeMap.end())
    {
      if (iter->second == filename.cString())
      {
        break;
      }
      iter++;
    }
    if (iter != mFileTimeMap.end())
      mFileTimeMap.erase(iter);
    mCacheMutex.unlock();
  }
}

rtError rtFileCache::removeData(const char* url)
{
  if (NULL == url)
    return RT_ERROR;

  rtString urlToRemove = url;
  rtString filename = hashedFileName(urlToRemove);
  if (! filename.isEmpty())
  {
    if (false == deleteFile(filename))
    {
      rtLogWarn("!!! deletion of cache failed for url(%s)",url);
      return RT_ERROR;
    }
    eraseData(filename);
  }
  else
  {
    rtLogWarn("!!! Got wrong filehash for url during removal of data (%s)",url);
    return RT_ERROR;
  }
  return RT_OK;
}

rtError rtFileCache::addToCache(const rtHttpCacheData& data)
{
  rtString url;
  rtError  err = data.url(url);
  if (RT_OK != err)
    return err;

  if  (url.isEmpty())
    return RT_ERROR;

  rtString filename =  hashedFileName(url);
  if (filename.isEmpty())
  {
    rtLogWarn("Problem in getting hash from the url(%s) while adding to cache ",url.cString());
    return RT_ERROR;
  }
  else
  {
    // If the file was already cached and got deleted manually, then we should clean up the corresponding data.
    if(mFileSizeMap[filename])
    {
      if ( !mFileTimeMap.empty() )
      {
        eraseData(filename);
      }
    }
  }

  bool ret = writeFile(filename,data);
  if (true != ret)
     return RT_ERROR;
  setFileSizeAndTime(filename);

  rtLogInfo("addToCache url(%s) filename(%s) size(%ld) Cache expiration(%s)", url.cString(), filename.cString(), (long) mFileSizeMap[filename], data.expirationDate().cString());

  mCacheMutex.lock();
  mCurrentSize += mFileSizeMap[filename];
  int64_t size = cleanup();
  mCacheMutex.unlock();
  rtLogInfo("current size after insertion and cleanup (%ld)",(long) size);
  return RT_OK;
}

rtError rtFileCache::httpCacheData(const char* url, rtHttpCacheData& cacheData)
{
  rtString urlToQuery = url;
  rtString filename =  hashedFileName(urlToQuery);
  if (filename.isEmpty())
  {
    rtLogWarn("Problem in getting hash from the url(%s) while read from cache",url);
    return RT_ERROR;
  }
  if (false == readFileHeader(filename,cacheData))
    return RT_ERROR;
  return RT_OK;
}

void rtFileCache::clearCache()
{
  if (! mDirectory.isEmpty())
  {
    stringstream buff;
    buff << "rm -rf " << mDirectory.cString() << "/*" ;
    int retVal = system(buff.str().c_str());

    if(retVal == -1)
    {
      // The system method failed
    }

    mFileSizeMap.clear();
    mCacheMutex.lock();
    mCurrentSize = 0;
    mCacheMutex.unlock();
  }
}

int64_t rtFileCache::cleanup()
{
  if ( (mCurrentSize > mMaxSize) && !(mFileTimeMap.empty()))
  {
    multimap<time_t,rtString>::iterator iter = mFileTimeMap.begin();
    vector <multimap<time_t,rtString>::iterator> timeMapIters;
    do
    {
      rtString filename = iter->second;
      if (! filename.isEmpty())
      {
          rtLogInfo("Storage capacity exceeded" );
          if(false == deleteFile(filename))
          {
            rtLogWarn("!!! deletion of cache failed during cleanup for file(%s)",filename.cString());
          }
          else
          {
            mCurrentSize = mCurrentSize - mFileSizeMap[filename];
            timeMapIters.push_back(iter);
            mFileSizeMap.erase(filename);
          }
      }
      iter++;
    } while ((mCurrentSize > mMaxSize) && (iter != mFileTimeMap.end()));

    for (unsigned int count =0; count < timeMapIters.size(); count++)
      mFileTimeMap.erase(timeMapIters[count]);
    timeMapIters.clear();
  }
  return mCurrentSize;
}

rtString rtFileCache::hashedFileName(const rtString& url)
{
  long int hash = hashFn(url.cString());
  stringstream stream;
  stream << hash;
  return stream.str().c_str();
}

void rtFileCache::setFileSizeAndTime(rtString& filename)
{
  if (!mDirectory.isEmpty())
  {
    struct stat statbuf;
    rtString absPathString  = absPath(filename);
    if (stat(absPathString.cString(), &statbuf) == 0)
    {
      mCacheMutex.lock();
      mFileSizeMap[filename] = statbuf.st_size;
#if defined(PX_PLATFORM_MAC)
      mFileTimeMap.insert(make_pair(statbuf.st_atimespec.tv_sec,filename));
#elif !(defined(WIN32) || defined(_WIN32) || defined (WINDOWS) || defined (_WINDOWS))
      mFileTimeMap.insert(make_pair(statbuf.st_atim.tv_sec,filename));
#else
       rtLogWarn("Platform not supported. Cache will not get cleared after cache limit is reached");
#endif
      mCacheMutex.unlock();
    }
  }
}

bool rtFileCache::writeFile(rtString& filename,const rtHttpCacheData& constCacheData)
{
  rtHttpCacheData* cacheData = const_cast<rtHttpCacheData*>(&constCacheData);
  rtData data;
  stringstream stream;
  stream << cacheData->expirationDateUnix();
  string date = stream.str().c_str();
  data.init(cacheData->headerData().length() + date.length() + 1 + cacheData->contentsData().length() + 1);
  memcpy(data.data(),cacheData->headerData().data(),cacheData->headerData().length());
  memset(data.data()+cacheData->headerData().length(),'|',1);
  memcpy(data.data()+cacheData->headerData().length()+1,date.c_str(), date.length());
  memset(data.data()+cacheData->headerData().length() + date.length() + 1,'|',1);
  memcpy(data.data()+cacheData->headerData().length()+1+ date.length() + 1,cacheData->contentsData().data(),cacheData->contentsData().length());
  rtString absPathString  = absPath(filename);
  if (RT_OK != rtStoreFile(absPathString.cString(),data))
    return false;
  return true;
}

bool rtFileCache::deleteFile(rtString& filename)
{
  rtString cmd = "rm -rf ";
  rtString absPathString  = absPath(filename);
  cmd.append(absPathString);
  rtLogInfo("Deleting the file (%s)", absPathString.cString());
  if (0 != system(cmd.cString()))
  {
    rtLogWarn("removal of file failed");
    return false;
  }
  return true;
}

bool rtFileCache::readFileHeader(rtString& filename,rtHttpCacheData& cacheData)
{
  rtString absPathString  = absPath(filename);
  FILE* fp  = fopen(absPathString.cString(), "r");

  if (NULL == fp)
  {
    rtLogDebug("Reading the cache file \"%s\" Failed - does not EXIST or OPEN already", filename.cString());
    return false;
  }

  bool reachedHeaderEnd = false;
  int buffer;
  string headerData;
  while ( !feof(fp) && (reachedHeaderEnd == false))
  {
    buffer = fgetc(fp);
    if (buffer == '|')
    {
      reachedHeaderEnd = true;
      break;
    }
    headerData.append(1,(char)buffer);
  }
  if (true == reachedHeaderEnd)
  {
    cacheData.setAttributes((char *)headerData.c_str());
  }
  else
  {
    rtLogWarn("Logfile is not proper");
    int closeret = fclose(fp);
    if (0 != closeret)
      rtLogWarn("improper logfile close failed");
    fp  =  NULL;
    return false;
  }
  cacheData.setFilePointer(fp);
  cacheData.setFileName(filename);
  return true;
}

rtString rtFileCache::absPath(rtString& filename)
{
  rtString absPathString = mDirectory;
  absPathString.append("/");
  absPathString.append(filename);
  return absPathString;
}
