#include <pxFileCache.h>
#include <pxOffscreen.h>
#include <pxUtil.h> 
#include <string.h>
#include <sstream>
#include <dirent.h>
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
  struct stat st;
  memset(&st,0,sizeof(struct stat));
  if (stat(mDirectory.cString(), &st) == -1) {
    mkdir(mDirectory.cString(), 0777);
  }
  else
  {
    populateExistingFiles();
  }
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
      mFileTimeMap[buf.st_atim.tv_sec] = direntry->d_name;
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
  if (NULL == directory)
  {
    return RT_ERROR;
  }
  mDirectory = directory;

  struct stat st;
  memset(&st,0,sizeof(struct stat));
  if (stat(mDirectory.cString(), &st) == -1)
  {
    mkdir(mDirectory.cString(), 0777);
  }
  else
  {
    populateExistingFiles();
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
    mCurrentSize = mCurrentSize - mFileSizeMap[filename];
    mFileSizeMap.erase(filename);
    map<time_t,rtString>::iterator iter = mFileTimeMap.begin();
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
  setFileSizeAndTime(filename);
  mCurrentSize += mFileSizeMap[filename];
  int64_t size = cleanup();
  rtLogWarn("current size after insertion and cleanup (%ld)",size);
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
  if ( (mCurrentSize > mMaxSize) && !(mFileTimeMap.empty()))
  {
    map<time_t,rtString>::iterator iter = mFileTimeMap.begin();
    vector <time_t> timeMapIters;
    do
    {
      rtString filename = iter->second;
      if (! filename.isEmpty())
      {
          if(false == deleteFile(filename))
          {
            rtLogWarn("!!! deletion of cache failed during cleanup for file(%s)",filename.cString());
          }
          else
          {
            mCurrentSize = mCurrentSize - mFileSizeMap[filename];
            timeMapIters.push_back(iter->first);
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

rtString rtFileCache::getHashedFileName(const rtString& url)
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
    rtString absPath  = getAbsPath(filename);
    if (stat(absPath.cString(), &statbuf) == 0)
    {
      mFileSizeMap[filename] = statbuf.st_size;
      mFileTimeMap[statbuf.st_atim.tv_sec] = filename;
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
  data.init(cacheData->getHeaderData().length() + date.length() + 1 + cacheData->getContentsData().length() + 1);
  memcpy(data.data(),cacheData->getHeaderData().data(),cacheData->getHeaderData().length());
  memset(data.data()+cacheData->getHeaderData().length(),'|',1);
  memcpy(data.data()+cacheData->getHeaderData().length()+1,date.c_str(), date.length());
  memset(data.data()+cacheData->getHeaderData().length() + date.length() + 1,'|',1);
  memcpy(data.data()+cacheData->getHeaderData().length()+1+ date.length() + 1,cacheData->getContentsData().data(),cacheData->getContentsData().length());
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
    cacheData.setAttributes((char *)headerData.c_str());
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
