#include <pxFileCache.h>
#include <pxOffscreen.h>
#include <pxUtil.h> 
#include <string.h>
#define DEFAULT_MAX_CACHE_SIZE 20971520

rtFileCache* rtFileCache::getInstance()
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
rtFileCache::rtFileCache():mMaxSize(DEFAULT_MAX_CACHE_SIZE),mCurrentSize(0),mDirectory("/tmp/cache")
{
  mFileSizeMap.clear();
  struct stat st = {0};

  if (stat(mDirectory.cString(), &st) == -1) {
    mkdir(mDirectory.cString(), 0777);
  }
  else
    clearCache();
}

rtFileCache::~rtFileCache()
{
  mMaxSize = 0;
  mCurrentSize = 0;
  mDirectory = "";
  mFileSizeMap.clear();
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
  if (NULL == directory)
  {
    return RT_ERROR;
  }
  mDirectory = directory;

  struct stat st = {0};
  if (stat(mDirectory.cString(), &st) == -1) {
    mkdir(mDirectory.cString(), 0777);
  }

  return RT_OK;
}

rtError rtFileCache::cacheDirectory(rtString& dir)
{
  if (mDirectory.isEmpty())
    return RT_ERROR;
  dir = mDirectory;
  return RT_OK;
}

rtError rtFileCache::removeData(const char* url)
{
  if (NULL == url)
    return RT_ERROR;
 
  rtString urlToRemove = url; 
  rtString filename = getHashedFileName(urlToRemove);
  if (! filename.isEmpty())
  {
    if (false == deleteFile(filename))
    {
      rtLogWarn("!!! deletion of cache failed for url(%s)",url);
      return RT_ERROR;
    }
    mCurrentSize = mCurrentSize - mFileSizeMap[url];
    mFileSizeMap.erase(url);
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

  rtString filename =  getHashedFileName(url);
  if (filename.isEmpty())
  {
    rtLogWarn("Problem in getting hash from the url(%s) while adding to cache ",url.cString());
    return RT_ERROR;
  }

  bool ret = writeFile(filename,data);
  if (true != ret)
     return RT_ERROR;
  int64_t fileSize = getFileSize(filename);
  mCurrentSize += fileSize;
  mFileSizeMap[url] = fileSize;
  int64_t size = cleanup();
  rtLogWarn("current size after insertion and cleanup (%d)",size);
  return RT_OK;
}

rtError rtFileCache::getHttpCacheData(const char* url, rtHttpCacheData& cacheData)
{
  rtString urlToQuery = url;
  rtString filename =  getHashedFileName(urlToQuery);
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
  string cmd = "rm -rf ";
  cmd.append(mDirectory.cString());
  cmd.append("/*");
  system(cmd.c_str());
  mFileSizeMap.clear();
  mCurrentSize = 0;
}

int64_t rtFileCache::cleanup()
{
  if ( (mCurrentSize > mMaxSize) && !(mFileSizeMap.empty()))
  {
    map<rtString,int64_t>::iterator iter = mFileSizeMap.begin();
   
    do
    {
      rtString filename = getHashedFileName(iter->first);
      if (! filename.isEmpty())
      {
        if(false == deleteFile(filename))
        {
          rtLogWarn("!!! deletion of cache failed during cleanup for url(%s)",iter->first.cString());
        }
        else
        {
	  mCurrentSize = mCurrentSize - iter->second;
          mFileSizeMap.erase(iter);
        }
      }
      iter++;
    } while ((mCurrentSize > mMaxSize) && (iter != mFileSizeMap.end()));
  }
  return mCurrentSize;
}

rtString rtFileCache::getHashedFileName(const rtString& url)
{
  long int hash = hashFn(url.cString());
  return to_string(hash).c_str();
}

int64_t rtFileCache::getFileSize(rtString& filename)
{
  if (mDirectory.isEmpty())
    return 0;
  struct stat statbuf;
  int64_t size = 0;
  rtString absPath  = getAbsPath(filename);
  if (stat(absPath.cString(), &statbuf) == 0)
  {
    size = statbuf.st_size;
  }
  return size;
}

bool rtFileCache::writeFile(rtString& filename,const rtHttpCacheData& cacheData)
{
  rtData data;
  data.init(cacheData.getHeaderData().length() + cacheData.getContentsData().length() + 1);
  memcpy(data.data(),cacheData.getHeaderData().data(),cacheData.getHeaderData().length());
  memset(data.data()+cacheData.getHeaderData().length(),'|',1);
  memcpy(data.data()+cacheData.getHeaderData().length()+1,cacheData.getContentsData().data(),cacheData.getContentsData().length());
  rtString absPath  = getAbsPath(filename);
  if (RT_OK != rtStoreFile(absPath.cString(),data))
    return false;
  return true;
}

bool rtFileCache::deleteFile(rtString& filename)
{
  rtString cmd = "rm -rf ";
  rtString absPath  = getAbsPath(filename);
  cmd.append(absPath);
  if (0 != system(cmd.cString()))
  {
    rtLogWarn("removal of file failed");
    return false;
  }
  return true;
}

bool rtFileCache::readFileHeader(rtString& filename,rtHttpCacheData& cacheData)
{
  rtString absPath  = getAbsPath(filename);
  FILE* fp  = fopen(absPath.cString(), "r");

  if (NULL == fp)
  {
    rtLogWarn("Reading the file failed ");
    return false;
  }  

  bool reachedHeaderEnd = false;
  char buffer;
  string headerData;
  while ( !feof(fp) && (reachedHeaderEnd == false))
  {
    buffer = fgetc(fp);
    if (buffer == '|')
    {
      reachedHeaderEnd = true;
      break;
    }
    headerData.append(1,buffer);
  }
  if (true == reachedHeaderEnd)
  {
    cacheData.setAttributes(headerData.c_str());
  }
  else
  {
    rtLogWarn("Logfile is not proper");
    return false;
  }
  cacheData.setFilePointer(fp);
  return true;
}

rtString rtFileCache::getAbsPath(rtString& filename)
{
  rtString absPath = mDirectory;
  absPath.append("/");
  absPath.append(filename);
  return absPath;
}
