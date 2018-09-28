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

#include <list>
#include <sstream>

#define private public
#define protected public

#include "rtFileCache.h"
#include "rtHttpCache.h"
#include "pxResource.h"
#include "pxArchive.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include "pxScene2d.h"
#include <string.h>

#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <semaphore.h>

#include "test_includes.h" // Needs to be included last

#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void (x))
#endif

using namespace std;
bool defaultCallbackExecuted = false;
extern void startFileDownloadInBackground(void* data);
extern bool continueDownloadHandleCheck;

// disabled as it causes crash
// please note that realloc is also extensively
// used by various glibc functions/threads
// also on linux even if the malloc/realloc returns non-null
// it doesn't guarantee that the returned memory will be available.
// #define UNAVAILABLE_MEMORY_TEST_ENABLED

#ifdef UNAVAILABLE_MEMORY_TEST_ENABLED // {
bool failRealloc = false;
typedef void* (*realloc_t)(void*, size_t);

void* realloc(void *ptr, size_t size)
{
  static realloc_t reallocp = (realloc_t) dlsym(RTLD_NEXT, "realloc");
  if (true == failRealloc)
  {
    return NULL;
  }
  else
  {
    if (NULL != reallocp)
    {
      return reallocp(ptr,size);
    }
  }
  return NULL;
}
#endif // } UNAVAILABLE_MEMORY_TEST_ENABLED

class commonTestFns
{
  public:
    commonTestFns() {
      memset(mNonExpireDate, 0, sizeof(mNonExpireDate));
      memset(mNonExpireDateVal, 0, sizeof(mNonExpireDateVal));
      memset(mExpireDate, 0, sizeof(mExpireDate));
      strcpy(mExpireDate,"Expires: Sun, 02 Oct 2016 22:33:33 UTC\n");
      time_t t = time(NULL);
      tm* timePtr = localtime(&t);
      sprintf(mNonExpireDateVal,"Sun, 02 Oct %d 22:33:33 UTC",(timePtr->tm_year+1+1900));
      sprintf(mNonExpireDate,"Expires:%s\n",mNonExpireDateVal);
    }

  protected:
     rtError addDataToCache(const char* url, const char* headerMetaData, const char* data, int size, bool isExpired=false)
     {
       rtString headerData(headerMetaData);
       if (isExpired)
       {
         headerData.append(mExpireDate);
       }
       else
       {
         headerData.append(mNonExpireDate);
       }
       rtHttpCacheData cacheData(url,headerData,data,size);
       return rtFileCache::instance()->addToCache(cacheData);
     }

  private:
     char mNonExpireDate[1000];
     char mNonExpireDateVal[1000];
     char mExpireDate[1000];
};

class pxFileCacheTest : public testing::Test, public commonTestFns
{
  public:
    virtual void SetUp()
    {

    }

    virtual void TearDown()
    {
      rtFileCache::instance()->clearCache();
      rtFileCache::destroy();
    }

    void fileCacheCreateNewTest ()
    {
      rtFileCache::destroy();
      bool sysret = system("rm -rf /tmp/cache");
      UNUSED_PARAM(sysret);
      rtFileCache::instance()->clearCache();
      struct stat st;
      memset(&st,0,sizeof(struct stat));
      EXPECT_TRUE (stat("/tmp/cache", &st) != -1);
    }

    void fileCacheSetNullCacheDirectoryTest()
    {
      EXPECT_TRUE (RT_ERROR == rtFileCache::instance()->setCacheDirectory(NULL));
    }

    void fileCacheSetEmptyCacheDirectoryTest()
    {
      EXPECT_TRUE (RT_ERROR == rtFileCache::instance()->setCacheDirectory(""));
    }

    void fileCachePopulateExistingFilesWoCacheTest()
    {
      rtFileCache::instance()->clearCache();
      bool sysret = system("rm -rf /tmp/cache");
      UNUSED_PARAM(sysret);
      rtFileCache::instance()->populateExistingFiles();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }

    void fileCachePopulateExistingFilesWithCacheTest()
    {
      rtFileCache::instance()->initCache();
      bool sysret = system("touch /tmp/cache/a.txt");
      UNUSED_PARAM(sysret);
      sysret = system("echo \"Hello\" >  /tmp/cache/a.txt");
      rtFileCache::instance()->populateExistingFiles();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() > 0);
    }

    void fileCacheSetMaxCacheSizeTest ()
    {
       rtFileCache::instance()->setMaxCacheSize(200);
       int64_t cacheSize;
       cacheSize  = rtFileCache::instance()->maxCacheSize();
       EXPECT_TRUE (cacheSize == 200);
    }

    void fileCacheSetDirectoryTest()
    {
      rtFileCache::instance()->setCacheDirectory("/tmp/cache");
      rtString directory;
      rtFileCache::instance()->cacheDirectory(directory);
      EXPECT_TRUE (directory == "/tmp/cache");
    }

    void fileCacheRemoveDataUrlNullTest()
    {
      EXPECT_TRUE (rtFileCache::instance()->removeData(NULL) == RT_ERROR);
    }

    void fileCacheRemoveDataUrlEmptyTest()
    {
      EXPECT_TRUE (rtFileCache::instance()->removeData("") == RT_OK);
    }

    void fileCacheRemoveDataUrlUnavailableTest()
    {
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://fileserver/a.jpeg") == RT_OK);
    }

    void fileCacheRemoveDataUrlAvailableTest()
    {
      resetAndAddCacheData();
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://fileserver/a.jpeg") == RT_OK);
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }

    void fileCacheRemoveDataSingleUrlAvailableTest()
    {
      resetAndAddCacheData();
      addDataToCache("http://fileserver/b.jpeg","","abcde",5);
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://fileserver/a.jpeg") == RT_OK);
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://fileserver/b.jpeg",data);
      stringstream stream;
      stream << data.expirationDateUnix();
      string date = stream.str().c_str();
      int expectedSize = strlen(mNonExpireDate) + 1 + strlen("abcde") + 1 + date.length();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == expectedSize);
    }

    void fileCacheRemoveDataNonWritableTest()
    {
      resetAndAddCacheData();
      bool sysret = system("chmod 444 /tmp/cache");
      UNUSED_PARAM(sysret);
      EXPECT_TRUE (rtFileCache::instance()->removeData(NULL) == RT_ERROR);
      sysret = system("chmod 777 /tmp/cache");
    }

    void fileCacheClearCacheTest()
    {
      rtFileCache::instance()->clearCache();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }

    void fileCacheGetHttpCacheDataAvailableTest()
    {
      resetAndAddCacheData();
      rtHttpCacheData data;
      EXPECT_TRUE (rtFileCache::instance()->httpCacheData("http://fileserver/a.jpeg",data) == RT_OK);
    }

    void fileCacheGetHttpCacheDataUnAvailableTest()
    {
      resetAndAddCacheData();
      rtHttpCacheData data;
      EXPECT_TRUE (rtFileCache::instance()->httpCacheData("http://fileserver/b.jpeg",data) == RT_ERROR);
    }

    void fileCacheAddNullUrlToCacheTest()
    {
      rtFileCache::instance()->clearCache();
      EXPECT_TRUE (addDataToCache(NULL,"","abcdef",6,true) == RT_ERROR);
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }

    void fileCacheAddProperUrlToCacheTest()
    {
      resetAndAddCacheData();
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://fileserver/a.jpeg",data);
      int expectedSize = strlen(mNonExpireDate) + 1 + + strlen("abcde") + 1 + to_string(data.expirationDateUnix()).length();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == expectedSize);
    }

    void fileCacheAddProperUrlToCacheNonWritableTest()
    {
      rtFileCache::instance()->clearCache();
      bool sysret = system("rm -rf /tmp/cache");
      UNUSED_PARAM(sysret);
      EXPECT_TRUE (addDataToCache("http://fileserver/a.jpeg","","abcde",5,true) == RT_ERROR);
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
      sysret = system("mkdir /tmp/cache");
      sysret = system("chmod 777 /tmp/cache");
    }

    void cleanupCacheTest()
    {
      resetAndAddCacheData();
      FILE* fp  = fopen("testRevalidationUpdate","w");
      fprintf(fp, "data updated");
      fclose(fp);
      bool sysret = system("cp testRevalidationUpdate /tmp/cache/.");
      UNUSED_PARAM(sysret);
      int64_t oldMaxSize  = rtFileCache::instance()->maxCacheSize();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() > 0);
      rtFileCache::instance()->setMaxCacheSize(0);
      rtFileCache::instance()->cleanup();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
      rtFileCache::instance()->setMaxCacheSize(oldMaxSize);
    }

    void createNewDirectoryCacheTest()
    {
      rtFileCache::instance()->setCacheDirectory("/tmp/cache1");
      rtString dir;
      EXPECT_TRUE (rtFileCache::instance()->cacheDirectory(dir) == RT_OK);
      EXPECT_TRUE (strcmp(dir.cString(),"/tmp/cache1") == 0);
      rtFileCache::instance()->setCacheDirectory("/tmp/cache");
    }

    void improperCacheFileFailReadTest()
    {
      rtFileCache::instance()->setCacheDirectory("/tmp/cache");
      rtString fileName("/tmp/cache/a.jpeg");
      rtString hashName = rtFileCache::instance()->hashedFileName(fileName);
      rtString resultFile("/tmp/cache/");
      resultFile.append(hashName.cString());
      FILE* fp = fopen(resultFile.cString(),"w");
      fclose(fp);
      rtHttpCacheData data;
      EXPECT_FALSE  (rtFileCache::instance()->readFileHeader(fileName,data));
    }
  private:

     void resetAndAddCacheData()
     {
       rtFileCache::instance()->clearCache();
       addDataToCache("http://fileserver/a.jpeg","","abcde",5);
     }
};

TEST_F(pxFileCacheTest, fileCacheCompleteTest)
{
  fileCacheCreateNewTest();
  fileCacheSetNullCacheDirectoryTest();
  fileCacheSetEmptyCacheDirectoryTest();
  fileCachePopulateExistingFilesWoCacheTest();
  fileCachePopulateExistingFilesWithCacheTest();
  fileCacheSetMaxCacheSizeTest();
  fileCacheSetDirectoryTest();
  fileCacheRemoveDataUrlNullTest();
  fileCacheRemoveDataUrlEmptyTest();
  fileCacheRemoveDataUrlUnavailableTest();
  fileCacheRemoveDataUrlAvailableTest();
  fileCacheRemoveDataSingleUrlAvailableTest();
  fileCacheRemoveDataNonWritableTest();
  fileCacheClearCacheTest();
  fileCacheGetHttpCacheDataAvailableTest();
  fileCacheGetHttpCacheDataUnAvailableTest();
  fileCacheAddNullUrlToCacheTest();
  fileCacheAddProperUrlToCacheTest();
  fileCacheAddProperUrlToCacheNonWritableTest();
  cleanupCacheTest();
  createNewDirectoryCacheTest();
  improperCacheFileFailReadTest();
}

class rtHttpCacheTest : public testing::Test, public commonTestFns
{
  public:
    virtual void SetUp()
    {
      defaultCacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nAccept-Ranges: bytes\nContent-Length: 4020\nContent-Type: image/jpeg\n";
    }

    virtual void TearDown()
    {
    }

    void populateCacheHeader (rtString& outheader, const char* cacheControl, bool isEtagPresent=true, bool isExpired=false)
    {
      outheader.append(defaultCacheHeader);
      if (isEtagPresent)
      {
        outheader.append("ETag: \"fb4-53e51895552f0\"\n");
      }

      outheader.append("Cache-Control: ");
      outheader.append(cacheControl);
      outheader.append("\n");

      if (isExpired)
      {
        outheader.append(mExpireDate);
      }
      else
      {
        outheader.append(mNonExpireDate);
      }
    }

    void  dataValiditySuccessTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=2000, public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      EXPECT_TRUE (data.isValid() == true);
    }

    void  dataValidityFailureEmptyImageTest()
    {
      rtHttpCacheData data("http://fileserver/a.jpeg");
      EXPECT_TRUE (data.isValid() == false);
    }

    void  dataValidityFailureExpiredImageTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=0, public",true,true);
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      EXPECT_TRUE (data.isValid() == false);
    }

    void  dataExpiredTrueTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=0, public",true,true);
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      EXPECT_TRUE (data.isExpired() == true);
    }

    void  dataExpiredFalseTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=2000, public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      EXPECT_TRUE (data.isExpired() == false);
    }

    void  expirationDateTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      rtString ipExpireDate(mNonExpireDateVal);
      struct tm timeinfo;
      memset(&timeinfo,0,sizeof(struct tm));
      strptime(ipExpireDate.cString(), "%a, %d %b %Y %H:%M:%S %Z", &timeinfo);
      time_t expireDateInGM = timegm(&timeinfo);
      char expExpireDate[100];
      memset(expExpireDate,0,100);
      strftime(expExpireDate, 100, "%Y-%m-%d %H:%M:%S", localtime(&expireDateInGM));
      EXPECT_TRUE (data.expirationDate() == expExpireDate);
    }

    void  dataWritableToCacheTrueTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      EXPECT_TRUE (data.isWritableToCache() == true);
    }

    void  dataZeroLengthWritableToCacheFalseTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-store, public");
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), NULL, 0);
      EXPECT_TRUE (data.isWritableToCache() == false);
    }

    void  dataNoStoreWritableToCacheFalseTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-store, public");
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), "abcde", 5);
      EXPECT_TRUE (data.isWritableToCache() == false);
    }

    void  setAttributesTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-store, public");
      rtHttpCacheData data("http://fileserver/a.jpeg");
      data.setAttributes((char *)cacheHeader.cString());

      map<rtString, rtString> retrivedAttributes;
      map<rtString, rtString> actualAttributes;

      actualAttributes["HTTP/1.1 200 OK"] = "";
      actualAttributes["Date"] = " Sun, 09 Oct 2016 21:22:50 GMT";
      actualAttributes["Server"] = " Apache/2.4.7 (Ubuntu)";
      actualAttributes["Last-Modified"] = " Sat, 08 Oct 2016 02:46:40 GMT";
      actualAttributes["ETag"] = " \"fb4-53e51895552f0\"";
      actualAttributes["Accept-Ranges"] = " bytes";
      actualAttributes["Cache-Control"] = " no-store, public";
      actualAttributes["Content-Length"] = " 4020";
      actualAttributes["Expires"] = mNonExpireDateVal;
      actualAttributes["Content-Type"] = " image/jpeg";
      data.attributes(retrivedAttributes);
      for (map<rtString,rtString>::iterator it =  actualAttributes.begin(); it != actualAttributes.end(); it++)
      {
        EXPECT_TRUE(actualAttributes[it->first] == retrivedAttributes[it->first]);
      }
    }

    void  initDataTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-store, public");
      const char* cacheData = "abcde";
      rtHttpCacheData data("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
      rtData& storedData = data.contentsData();
      EXPECT_TRUE ( strcmp(cacheData,(const char*)storedData.data()) == 0);
    }

    void  setDataTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-store, public");
      const char* cacheData = "abcde";
      rtHttpCacheData data("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
      rtData newData;
      const char* newcontents = "pqrstu";
      newData.init((const uint8_t *)newcontents,strlen(newcontents));
      data.setData(newData);
      rtData& storedData = data.contentsData();
      EXPECT_TRUE ( strcmp("pqrstu",(const char*)storedData.data()) == 0);
    }

    void  readEtagTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=2000, public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      rtString tag;
      rtError ret = data.etag(tag);
      EXPECT_TRUE (ret == RT_OK);
      EXPECT_TRUE (strcmp(tag.cString(), " \"fb4-53e51895552f0\"") == 0);
    }

    void  readEtagNotPresentTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=2000, public",false,false);
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      rtString tag;
      rtError ret = data.etag(tag);
      EXPECT_TRUE (ret == RT_ERROR);
    }

    void readDataFileAccessFailedTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "max-age=2000, public");
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://fileserver/a.jpeg",cacheHeader.cString(), cacheData, strlen(cacheData));
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR);
    }

    void nocacheCompleteResponseTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-cache, public");
      const char* cacheData = "abcde";
      addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR); //This is test page, so verifying download is happening or not
    }

    void nocacheExpiresParamTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-cache=Expires, public");
      const char* cacheData = "abcde";
      addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR);
    }

    void mustRevalidateUnExpiredTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "no-cache, public");
      const char* cacheData = "abcde";
      addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
      rtHttpCacheData data("http://fileserver/test.jpeg");
      rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
      bool revalidateWhole = false;
      bool revalidateHeadersOnly = false;
      data.calculateRevalidationNeed(revalidateWhole,revalidateHeadersOnly);
      EXPECT_TRUE (revalidateWhole == true);
      EXPECT_TRUE (revalidateHeadersOnly == false);
    }

    void mustRevalidateTrueExpiredTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "public must-revalidate",true,true);
      const char* cacheData = "abcde";
      addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData),true);
      rtHttpCacheData data("http://fileserver/test.jpeg");
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == true);
    }

    void mustRevalidateFalseExpiredTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "public");
      const char* cacheData = "abcde";
      addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData),true);
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtHttpCacheData data("http://fileserver/test.jpeg");
      rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == false);
   }

   void mustRevalidateFalseExpiredContentsInvalidTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "public");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData),true);
     rtHttpCacheData data("http://fileserver/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
     rtData contents;
     EXPECT_TRUE (RT_ERROR == data.data(contents));
   }

   void mustRevalidateTruenocacheUnExpiredTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "no-cache");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://fileserver/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidate == true);
   }

   void mustRevalidateTruenocacheExpiresFiledTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "public no-cache=Expires");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://fileserver/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidateOnlyHeaders == true);
     EXPECT_TRUE (revalidate == false);
   }
/*
   void dataPresentAfterHeadersRevalidationTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "public no-cache=Expires");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/test.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
     rtHttpCacheDataMock data("http://fileserver/test.jpeg");
     data.setHttpResponseHeaderData(cacheHeader);
     data.setHttpResponseRealData(cacheData);
     rtFileCache::instance()->httpCacheData("http://fileserver/test.jpeg",data);
     rtData contents;
     data.data(contents);
     EXPECT_TRUE ( strcmp(cacheData,(const char*)contents.data()) == 0);
   }

   void dataPresentAfterFullRevalidationTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "no-cache");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/test1.jpeg",cacheHeader.cString(),cacheData,strlen(cacheData));
     rtHttpCacheDataMock data("http://fileserver/test1.jpeg");
     data.setHttpResponseHeaderData(cacheHeader);
     data.setHttpResponseRealData(cacheData);
     rtFileCache::instance()->httpCacheData("http://fileserver/test1.jpeg",data);
     rtData contents;
     data.data(contents);
     EXPECT_TRUE ( strcmp(cacheData,(const char*)contents.data()) == 0);
   }

   void dataUpdatedAfterFullRevalidationTest()
   {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "no-cache");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/testRevalidationUpdate",cacheHeader.cString(),cacheData,strlen(cacheData));
     const char* updatedData = "data updated";
     rtHttpCacheDataMock data("http://fileserver/testRevalidationUpdate");
     data.setHttpResponseHeaderData(cacheHeader);
     data.setHttpResponseRealData(updatedData);
     rtFileCache::instance()->httpCacheData("http://fileserver/testRevalidationUpdate",data);
     rtData contents;
     data.data(contents);
     rtData& storedData = data.contentsData();
     EXPECT_TRUE ( strcmp("data updated",(const char*)storedData.data()) == 0);
   }


    void dataUpdatedAfterEtagTest()
    {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "public");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/testEtag",cacheHeader.cString(),cacheData,strlen(cacheData));
     rtHttpCacheDataMock data("http://fileserver/testEtag");
     data.setHttpResponseHeaderData(cacheHeader);
     data.setHttpResponseRealData(cacheData);
     rtFileCache::instance()->httpCacheData("http://fileserver/testEtag",data);
     rtData contents;
     data.data(contents);
     rtData& storedData = data.contentsData();
     EXPECT_TRUE ( strcmp("data updated",(const char*)storedData.data()) == 0);
    }
*/
    void dataUpdatedAfterEtagDownloadFailedTest()
    {
     rtString cacheHeader("");
     populateCacheHeader(cacheHeader, "public");
     const char* cacheData = "abcde";
     addDataToCache("http://fileserver/testEtagFail",cacheHeader.cString(),cacheData,strlen(cacheData));
     rtHttpCacheData data("http://fileserver/testEtagFail");
     rtFileCache::instance()->httpCacheData("http://fileserver/testEtagFail",data);
     rtData contents;
     EXPECT_TRUE (RT_ERROR == data.data(contents));
    }

#ifdef UNAVAILABLE_MEMORY_TEST_ENABLED // {
    void memoryUnAvailableTest()
    {
      rtString cacheHeader("");
      populateCacheHeader(cacheHeader, "public");
      const char* cacheData = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 30 Nov 3020 21:22:50 GMT\nContent-Type: image/jpeg\nabcdefghijklmnopqrstuvwxyz\0";
      addDataToCache("http://fileserver/testEtag",cacheHeader.cString(),cacheData,strlen(cacheData));
      FILE* fp  = fopen("testEtag","w");
      fprintf(fp, "data updated");
      fclose(fp);
      rtHttpCacheData data("http://fileserver/testEtag");
      rtFileCache::instance()->httpCacheData("http://fileserver/testEtag",data);
      failRealloc = true;
      EXPECT_TRUE (false == data.readFileData());
      failRealloc = false;
    }
#endif // } UNAVAILABLE_MEMORY_TEST_ENABLED

    void handleDownloadRequest404Test()
    {
      rtHttpCacheData data("http://www.pxscene.org/examples/px-reference/gallery/fancy1.js");
      vector<rtString> headers;
      bool ret = data.handleDownloadRequest(headers,true);
      EXPECT_TRUE(false == ret); 
    }
  
    void handleDownloadRequestProperTest()
    {
      rtHttpCacheData data("http://www.pxscene.org/examples/px-reference/gallery/fancy.js");
      vector<rtString> headers;
      bool ret = data.handleDownloadRequest(headers,true);
      EXPECT_TRUE(true == ret); 
    }

    void filePointerTest()
    {
      rtHttpCacheData data("http://www.pxscene.org/examples/px-reference/gallery/fancy.js");
      EXPECT_TRUE(NULL == data.filePointer());
    }

    void deferCacheReadFailTest()
    {
      rtData d;
      rtHttpCacheData data("http://www.pxscene.org/examples/px-reference/gallery/fancy.js");
      EXPECT_TRUE(RT_ERROR == data.deferCacheRead(d));
    }

    private:
      rtString defaultCacheHeader;
};

TEST_F(rtHttpCacheTest, httpCacheCompleteTest)
{
  dataValiditySuccessTest();
  dataValidityFailureEmptyImageTest();
  dataValidityFailureExpiredImageTest();
  dataExpiredTrueTest();
  dataExpiredFalseTest();
  expirationDateTest();
  dataWritableToCacheTrueTest();
  dataZeroLengthWritableToCacheFalseTest();
  dataNoStoreWritableToCacheFalseTest();
  setAttributesTest();
  initDataTest();
  setDataTest();
  readEtagTest();
  readEtagNotPresentTest();
  readDataFileAccessFailedTest();
  nocacheCompleteResponseTest();
  nocacheExpiresParamTest();
  mustRevalidateUnExpiredTest();
  mustRevalidateTrueExpiredTest();
  mustRevalidateFalseExpiredTest();
  mustRevalidateFalseExpiredContentsInvalidTest();
  mustRevalidateTruenocacheUnExpiredTest();
  mustRevalidateTruenocacheExpiresFiledTest();
/*
  dataPresentAfterHeadersRevalidationTest();
  dataPresentAfterFullRevalidationTest();
  dataUpdatedAfterFullRevalidationTest();
  dataUpdatedAfterEtagTest();
*/
  dataUpdatedAfterEtagDownloadFailedTest();
#ifdef UNAVAILABLE_MEMORY_TEST_ENABLED // {
  memoryUnAvailableTest();
#endif // } UNAVAILABLE_MEMORY_TEST_ENABLED
  handleDownloadRequest404Test();
  handleDownloadRequestProperTest();
  filePointerTest();
  deferCacheReadFailTest();
}

class rtFileDownloaderTest : public testing::Test, public commonTestFns
{
  public:
    virtual void SetUp()
    {
      //common header and data is used for all tests and readFile call is removed
      fixedHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nContent-Type: image/jpeg\n";
      fixedHeader.append(mNonExpireDate);
      fixedData = "<html><body>Hi</body></html>";
      testSem = sem_open("/semaphore", O_CREAT, 0644, 0);
      expectedStatusCode = 0;
      expectedHttpCode = 0;
      expectedCachePresence = false;
      continueDownloadHandleCheck = false;
    }

    virtual void TearDown()
    {
      int ret = sem_close(testSem);
      ret = sem_unlink("/semaphore");
      UNUSED_PARAM(ret);
    }

    char* getHeader()
    {
      char* header = (char*) malloc(fixedHeader.length()+1);
      memset(header,0,fixedHeader.length()+1);
      strncpy(header,fixedHeader.cString(),fixedHeader.length());
      return header;
    }

    char* getBodyData()
    {
      char* bodyData = (char*) malloc(fixedData.length()+1);
      memset(bodyData,0,fixedData.length()+1);
      strncpy(bodyData,fixedData.cString(),fixedData.length());
      return bodyData;
    }

    void downloadFileCacheDataUnAvailableTest()
    {
      rtFileDownloadRequest* downloadRequest = new rtFileDownloadRequest("http://px-apps.sys.comcast.net/pxscene-samples/images/tiles/notfound",this);
      downloadRequest->setHeaderData(NULL, 0);
      downloadRequest->setDownloadedData(NULL, 0);
      downloadRequest->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 404;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(downloadRequest);
      sem_wait(testSem);
    }

    void downloadFileCacheDataExpiredAvailableNoRevalidateTest()
    {
      rtFileCache::instance()->clearCache();
      addDataToCache("http://fileserver/file.jpeg","","abcde",5,true);
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void downloadFileCacheDataProperAvailableTest()
    {
      rtFileCache::instance()->clearCache();
      addDataToCache("http://fileserver/file.jpeg",getHeader(),getBodyData(),fixedData.length());
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void downloadFileAddToCacheTest()
    {
      // TODO TESTS images files downloaded from pxscene-samples need expiry date
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
      rtHttpCacheData data("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg");
      //EXPECT_TRUE (RT_OK ==rtFileCache::instance()->httpCacheData("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",data));
    }

    #define DEFER_CACHE_BUFFER_SIZE 	 (16*1024) // 16 K Added similar to CURL_MAX_WRITE_SIZE (the usual default is 16K)
    void setDeferCacheReadTest()
    {
      // TODO TESTS images files downloaded from pxscene-samples need expiry date
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallbackForDeferCache);
      request->setDeferCacheRead(true);
      request->setCachedFileReadSize(DEFER_CACHE_BUFFER_SIZE);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void setUseCallbackDataSizeTest()
    {
      // TODO TESTS images files downloaded from pxscene-samples need expiry date
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallbackForUseCallbackDataSize);
      request->setDownloadProgressCallbackFunction(rtFileDownloaderTest::downloadProgressCallbackForUseCallbackDataSize, this);
      request->setUseCallbackDataSize(true);
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void checkAndDownloadFromNetworkSuccess()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      EXPECT_TRUE (ret == true);
      EXPECT_TRUE (request->httpStatusCode() == 200);
      EXPECT_TRUE (request->downloadStatusCode() == 0);
    }

    void checkAndDownloadFromNetworkFailure()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/notfound",this);
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      UNUSED_PARAM(ret);
      EXPECT_TRUE (request->httpStatusCode() != 200);
    }

    void disableCacheTest()
    {
      rtFileCache::instance()->clearCache();
      const char *url = "http://fileserver/file_notfound.jpeg";
      rtFileDownloadRequest* request = new rtFileDownloadRequest(url,this);
      request->setCacheEnabled(false);
      request->setCallbackFunction(NULL);
      rtFileDownloader::instance()->downloadFile(request);
      // Once downloadFile() finished 'request' is deleted
      // EXPECT_TRUE (request->isDataCached() == false);
      rtHttpCacheData cachedData(url);
      rtError ret = rtFileCache::instance()->httpCacheData(url, cachedData);
      EXPECT_TRUE(ret == RT_ERROR);
    }

    void startFileDownloadInBackgroundTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 6;
      expectedHttpCode = 0;
      startFileDownloadInBackground(request);
      sem_wait(testSem);
    }

    void setFileUrlTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
      request->setFileUrl("http://fileserver/file.jpeg");
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 6;
      expectedHttpCode = 0;
      startFileDownloadInBackground(request);
      sem_wait(testSem);
    }

    void setProxyTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
      request->setFileUrl("http://fileserver1/file.jpeg");
      request->setProxy("undefined");
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      EXPECT_TRUE (ret == false);
      EXPECT_TRUE (strcmp( request->errorString(), "") != 0);
    }

    void errorStringTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhos123t/filenotpresent.jpeg",this);
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      EXPECT_TRUE (ret == false);
      EXPECT_TRUE (strcmp( request->errorString(), "") != 0);
    }

    void setCallbackFunctionThreadSafeTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
      rtFileDownloader::addFileDownloadRequest(request);
      rtFileDownloader::setCallbackFunctionThreadSafe(request, rtFileDownloaderTest::downloadCallback, this);
      expectedStatusCode = 0;
      expectedCachePresence = false;
      expectedHttpCode = 0;
      EXPECT_TRUE (request->executeCallback(0) == true);
      sem_wait(testSem);
    }

    void setCallbackFunctionNullTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
      rtFileDownloader::setCallbackFunctionThreadSafe(request, NULL, this);
      EXPECT_TRUE (request->executeCallback(0) == false);
    }

    void setCallbackFunctionNullInDownloadFileTest()
    {
      void (*callbackFunction)(rtFileDownloadRequest*);
      callbackFunction = rtFileDownloader::instance()->mDefaultCallbackFunction;
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      rtFileDownloader::setCallbackFunctionThreadSafe(request, NULL, this);
      rtFileDownloader::instance()->setDefaultCallbackFunction(rtFileDownloaderTest::defaultDownloadCallback);
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
      EXPECT_TRUE (defaultCallbackExecuted ==true);
      defaultCallbackExecuted = false;
      rtFileDownloader::instance()->setDefaultCallbackFunction(callbackFunction);
    }

    void setDefaultCallbackFunctionNullTest()
    {
      const char *url = "http://fileserver/file.jpeg";
      rtFileCache::instance()->clearCache();
      addDataToCache(url,getHeader(),getBodyData(),fixedData.length());
      void (*callbackFunction)(rtFileDownloadRequest*);
      callbackFunction = rtFileDownloader::instance()->mDefaultCallbackFunction;
      rtFileDownloadRequest* request = new rtFileDownloadRequest(url,this);
      request->setCallbackFunction(NULL);
      rtFileDownloader::instance()->setDefaultCallbackFunction(NULL);
      rtFileDownloader::instance()->downloadFile(request);
      // Once downloadFile() finished 'request' is deleted
      // EXPECT_TRUE (true == request->isDataCached());
      rtHttpCacheData cachedData(url);
      rtError ret = rtFileCache::instance()->httpCacheData(url, cachedData);
      EXPECT_TRUE(ret == RT_OK);
      rtFileDownloader::instance()->setDefaultCallbackFunction(callbackFunction);
    }

    void setCallbackDataTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",NULL);
      rtFileDownloader::addFileDownloadRequest(request);
      rtFileDownloader::setCallbackFunctionThreadSafe(request, rtFileDownloaderTest::downloadCallback, NULL);
      request->setCallbackData(this);
      expectedStatusCode = 0;
      expectedCachePresence = false;
      expectedHttpCode = 0;
      EXPECT_TRUE (request->executeCallback(0) == true);
      sem_wait(testSem);
    }

    void setDownloadHandleExpiresTimeTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",NULL);
      request->setDownloadHandleExpiresTime(3);
      EXPECT_TRUE (request->downloadHandleExpiresTime() == 3);
    }

    void addToDownloadQueueTest()
    {
      rtFileCache::instance()->clearCache();
      addDataToCache("http://fileserver/file.jpeg",getHeader(),getBodyData(),fixedData.length());
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      request->setCallbackData(this);
      rtFileDownloader::instance()->addToDownloadQueue(request);
      sem_wait(testSem);
    }

    void startNextDownloadInBackgroundTest()
    {
      //todo more actions once startNextDownloadInBackground() is implemented
      rtFileDownloader::instance()->startNextDownloadInBackground();
    }

    void raiseDownloadPriorityTest()
    {
      rtFileCache::instance()->clearCache();
      expectedStatusCode = 6;
      expectedHttpCode = 0;
      expectedCachePresence = false;
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      request->setCallbackData(this);
      rtFileDownloader::instance()->addToDownloadQueue(request);
      rtFileDownloader::instance()->raiseDownloadPriority(request);
      sem_wait(testSem);
    }

    void nextDownloadRequestTest()
    {
      //todo when nextDownloadRequest() is implemented
      EXPECT_TRUE (rtFileDownloader::instance()->nextDownloadRequest() == NULL);
    }

    void removeDownloadRequestTest()
    {
      //todo more actions once removeDownloadRequest() is implemented
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://fileserver/file.jpeg",this);
      rtFileDownloader::instance()->removeDownloadRequest(request);
    }

    void clearFileCacheTest()
    {
      //todo more actions once clearFileCache() is implemented
      rtFileDownloader::instance()->clearFileCache();
    }

    void setHTTPFailOnErrorTest()
    {
      rtFileDownloadRequest req("http://fileserver/notfound",NULL);
      req.setHTTPFailOnError(true);
      EXPECT_TRUE (req.isHTTPFailOnError() == true);
    }

    void setHTTPErrorTest()
    {
      rtFileDownloadRequest req("http://fileserver/notfound",NULL);
      req.setHTTPError("httperror");
      EXPECT_TRUE (strcmp("httperror",req.httpErrorBuffer()) == 0);
    }

    void setCurlDefaultTimeoutTest()
    {
      rtFileDownloadRequest req("http://fileserver/notfound",NULL);
      req.setCurlDefaultTimeout(true);
      EXPECT_TRUE (req.isCurlDefaultTimeoutSet() == true);
    }

    // download progress test begins
    void setDownloadProgressCallbackFunctionTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest request("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      request.setDownloadProgressCallbackFunction(rtFileDownloaderTest::downloadProgressCallback, this);
      EXPECT_TRUE (request.mDownloadProgressCallbackFunction == &rtFileDownloaderTest::downloadProgressCallback);
      EXPECT_TRUE (request.mDownloadProgressUserPtr == this);
    }

    void executeDownloadProgressCallbackPresentTest()
    {
      size_t size = 0, nmemb = 0;
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest request("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      request.setDownloadProgressCallbackFunction(rtFileDownloaderTest::downloadProgressCallback, this);
      EXPECT_TRUE ((size * nmemb) == request.executeDownloadProgressCallback(NULL, size, nmemb));
    }

    void executeDownloadProgressCallbackAbsentTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest request("https://px-apps.sys.comcast.net/pxscene-samples/images/tiles/008.jpg",this);
      EXPECT_TRUE (false == request.executeDownloadProgressCallback(NULL, 0, 0));
    }

    void setDataIsCachedTest()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest req("http://fileserver/notfound",NULL);
      req.setDataIsCached(false);
      EXPECT_TRUE (req.isDataCached() == false);
      req.setDataIsCached(true);
      EXPECT_TRUE (req.isDataCached() == true);
    }
    // download progress test ends

    static void downloadCallbackForDeferCache(rtFileDownloadRequest* fileDownloadRequest)
    {
      rtHttpCacheData cachedData;
      if (fileDownloadRequest != NULL && fileDownloadRequest->callbackData() != NULL)
      {
        rtFileDownloaderTest* callbackData = (rtFileDownloaderTest*) fileDownloadRequest->callbackData();
        EXPECT_TRUE (callbackData->expectedHttpCode == fileDownloadRequest->httpStatusCode());
        EXPECT_TRUE (callbackData->expectedStatusCode ==  fileDownloadRequest->downloadStatusCode());
        EXPECT_TRUE (callbackData->expectedCachePresence == rtFileDownloader::instance()->checkAndDownloadFromCache(fileDownloadRequest,cachedData));
        if(fileDownloadRequest->isDataCached())
        {
		  char* invalidData;
          size_t dataSizeFromDeferCache = 0;

          fileDownloadRequest->downloadedData(invalidData, dataSizeFromDeferCache);

          fileDownloadRequest->setDeferCacheRead(false);
          rtHttpCacheData cachedData(fileDownloadRequest->fileUrl().cString());
          if (true == rtFileDownloader::instance()->checkAndDownloadFromCache(fileDownloadRequest,cachedData))
          {
            char* data;
            size_t dataSize = 0;

            fileDownloadRequest->downloadedData(data, dataSize);
            EXPECT_TRUE (dataSize == dataSizeFromDeferCache);
          }
        }
        sem_post(callbackData->testSem);
      }
    }

    static void downloadCallbackForUseCallbackDataSize(rtFileDownloadRequest* fileDownloadRequest)
    {
      rtHttpCacheData cachedData;
      if (fileDownloadRequest != NULL && fileDownloadRequest->callbackData() != NULL)
      {
        rtFileDownloaderTest* callbackData = (rtFileDownloaderTest*) fileDownloadRequest->callbackData();
        EXPECT_TRUE ( false == fileDownloadRequest->isDataCached());
        sem_post(callbackData->testSem);
      }
    }

    static void downloadCallback(rtFileDownloadRequest* fileDownloadRequest)
    {
      rtHttpCacheData cachedData;
      if (fileDownloadRequest != NULL && fileDownloadRequest->callbackData() != NULL)
      {
        rtFileDownloaderTest* callbackData = (rtFileDownloaderTest*) fileDownloadRequest->callbackData();
        EXPECT_TRUE (callbackData->expectedHttpCode == fileDownloadRequest->httpStatusCode());
        EXPECT_TRUE (callbackData->expectedStatusCode ==  fileDownloadRequest->downloadStatusCode());
        EXPECT_TRUE (callbackData->expectedCachePresence == rtFileDownloader::instance()->checkAndDownloadFromCache(fileDownloadRequest,cachedData));
        sem_post(callbackData->testSem);
      }
    }

    static void defaultDownloadCallback(rtFileDownloadRequest* fileDownloadRequest)
    {
      rtHttpCacheData cachedData;
      if (fileDownloadRequest != NULL && fileDownloadRequest->callbackData() != NULL)
      {
        rtFileDownloaderTest* callbackData = (rtFileDownloaderTest*) fileDownloadRequest->callbackData();
        defaultCallbackExecuted = true;
        sem_post(callbackData->testSem);
      }
    }

    static size_t downloadProgressCallbackForUseCallbackDataSize(void *ptr, size_t size, size_t nmemb, void *userData)
    {
      // Lets consider an example of due to user/caller interuption this callback consumed only half of the data.
      int consumed= (size*nmemb)/2;
      UNUSED_PARAM (ptr);
      UNUSED_PARAM(userData);

      return consumed;
    }

    static size_t downloadProgressCallback(void *ptr, size_t size, size_t nmemb, void *userData)
    {
      UNUSED_PARAM (ptr);
      UNUSED_PARAM (size);
      UNUSED_PARAM (nmemb);
      UNUSED_PARAM (userData);
      return 0;
    }

  private:
    int expectedStatusCode;
    int expectedHttpCode;
    bool expectedCachePresence;
    sem_t* testSem;
    //used for mock functions
    rtString fixedHeader;
    rtString fixedData;
};

TEST_F(rtFileDownloaderTest, checkCacheTests)
{
  downloadFileCacheDataUnAvailableTest();
  downloadFileCacheDataExpiredAvailableNoRevalidateTest();
  downloadFileCacheDataProperAvailableTest();
  disableCacheTest();
  downloadFileAddToCacheTest();
  setDeferCacheReadTest();
  setUseCallbackDataSizeTest();
  checkAndDownloadFromNetworkSuccess();
  checkAndDownloadFromNetworkFailure();
  startFileDownloadInBackgroundTest();
  setFileUrlTest();
  setProxyTest();
  errorStringTest();
  setCallbackFunctionThreadSafeTest();
  setCallbackFunctionNullTest();
  setCallbackDataTest();
  setDownloadHandleExpiresTimeTest();
  addToDownloadQueueTest();
  setCallbackFunctionNullInDownloadFileTest();
  setDefaultCallbackFunctionNullTest();
  startNextDownloadInBackgroundTest();
  // commenting out because it is difficult to maintain hold of download requests before raising priority
  //raiseDownloadPriorityTest();
  nextDownloadRequestTest();
  removeDownloadRequestTest();
  setHTTPFailOnErrorTest();
  setHTTPErrorTest();
  setCurlDefaultTimeoutTest();
  setDownloadProgressCallbackFunctionTest();
  executeDownloadProgressCallbackPresentTest();
  executeDownloadProgressCallbackAbsentTest();
  setDataIsCachedTest();
  clearFileCacheTest();
}
