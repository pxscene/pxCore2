
#include <list>

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
#include <sstream>
#include <unistd.h>
#include <time.h>
#include <dlfcn.h>
#include <semaphore.h>

#include "test_includes.h" // Needs to be included last

using namespace std;
bool failRealloc = false;
bool defaultCallbackExecuted = false;
extern void startFileDownloadInBackground(void* data);
extern bool continueDownloadHandleCheck;
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

class commonTestFns
{
  protected:
     rtError addDataToCache(const char* url, const char* headerMetaData, const char* data, int size)
     {
       rtHttpCacheData cacheData(url,headerMetaData,data,size);
       return rtFileCache::instance()->addToCache(cacheData);
     }
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
      rtFileCache::instance()->populateExistingFiles();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }

    void fileCachePopulateExistingFilesWithCacheTest()
    {
      rtFileCache::instance()->initCache();
      bool sysret = system("touch /tmp/cache/a.txt");
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
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://localhost:8080/a.jpeg") == RT_OK);
    }
    
    void fileCacheRemoveDataUrlAvailableTest()
    { 
      resetAndAddCacheData();
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://localhost:8080/a.jpeg") == RT_OK); 
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0); 
    }
    
    void fileCacheRemoveDataSingleUrlAvailableTest()
    { 
      resetAndAddCacheData();
      addDataToCache("http://localhost:8080/b.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5);
      EXPECT_TRUE (rtFileCache::instance()->removeData("http://localhost:8080/a.jpeg") == RT_OK); 
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/b.jpeg",data);
      stringstream stream;
      stream << data.expirationDateUnix();
      string date = stream.str().c_str();
      int expectedSize = strlen("Expires: Sun 02 Oct 2016 22:33:33 UTC") + 1 + strlen("abcde") + 1 + date.length();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == expectedSize); 
    }
    
    void fileCacheRemoveDataNonWritableTest()
    { 
      resetAndAddCacheData();
      bool sysret = system("chmod 444 /tmp/cache");
      EXPECT_TRUE (rtFileCache::instance()->removeData(NULL) == RT_ERROR);
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/a.jpeg",data);
      stringstream stream;
      stream << data.expirationDateUnix();
      string date = stream.str().c_str();
      int expectedSize = strlen("Expires: Sun 02 Oct 2017 22:33:33 UTC") + 1 + strlen("abcde") + 1 + date.length(); //11 is the size of expiration date
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == expectedSize); 
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
      EXPECT_TRUE (rtFileCache::instance()->httpCacheData("http://localhost:8080/a.jpeg",data) == RT_OK);
    }
    
    void fileCacheGetHttpCacheDataUnAvailableTest()
    { 
      resetAndAddCacheData();
      rtHttpCacheData data;
      EXPECT_TRUE (rtFileCache::instance()->httpCacheData("http://localhost:8080/b.jpeg",data) == RT_ERROR);
    }
    
    void fileCacheAddNullUrlToCacheTest()
    {
      rtFileCache::instance()->clearCache();
      EXPECT_TRUE (addDataToCache(NULL,"Expires: Sun 02 Oct 2016 22:33:33 UTC","abcdef",6) == RT_ERROR);
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == 0);
    }
    
    void fileCacheAddProperUrlToCacheTest()
    {
      resetAndAddCacheData();
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/a.jpeg",data);
      int expectedSize = strlen("Expires: Sun 02 Oct 2017 22:33:33 UTC") + 1 + + strlen("abcde") + 1 + to_string(data.expirationDateUnix()).length();
      EXPECT_TRUE (rtFileCache::instance()->cacheSize() == expectedSize);
    }
    
    void fileCacheAddProperUrlToCacheNonWritableTest()
    {
      rtFileCache::instance()->clearCache();
      bool sysret = system("rm -rf /tmp/cache");
      EXPECT_TRUE (addDataToCache("http://localhost:8080/a.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5) == RT_ERROR);
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
       addDataToCache("http://localhost:8080/a.jpeg","Expires: Sun 02 Oct 2017 22:33:33 UTC\0","abcde",5);
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
    }
  
    virtual void TearDown()
    {
    }
  
    void  dataValiditySuccessTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isValid() == true);
    }
  
    void  dataValidityFailureEmptyImageTest()
    {
      rtHttpCacheData data("http://localhost:8080/a.jpeg"); 
      EXPECT_TRUE (data.isValid() == false);
    }
 
    void  dataValidityFailureExpiredImageTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=0, public\nExpires: Mon, 10 Oct 2016 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isValid() == false);
    }

    void  dataExpiredTrueTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=0, public\nExpires: Mon, 10 Oct 2016 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isExpired() == true);
    }
  
    void  dataExpiredFalseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isExpired() == false);
    }
  
    void  expirationDateTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 

      rtString ipExpireDate("Mon, 10 Oct 2017 21:22:50 GMT");
      struct tm timeinfo;
      memset(&timeinfo,0,sizeof(struct tm));
      strptime(ipExpireDate.cString(), " %a, %d %b %Y %H:%M:%S %Z", &timeinfo);
      time_t expireDateInGM = timegm(&timeinfo);
      char expExpireDate[100];
      memset(expExpireDate,0,100);
      strftime(expExpireDate, 100, "%Y-%m-%d %H:%M:%S", localtime(&expireDateInGM));
      EXPECT_TRUE (data.expirationDate() == expExpireDate);
    }
  
    void  dataWritableToCacheTrueTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isWritableToCache() == true);
    }
  
    void  dataZeroLengthWritableToCacheFalseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg"; 
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, NULL, 0); 
      EXPECT_TRUE (data.isWritableToCache() == false);
    }
  
    void  dataNoStoreWritableToCacheFalseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg"; 
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, "abcde", 5); 
      EXPECT_TRUE (data.isWritableToCache() == false);
    }

    void  setAttributesTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      rtHttpCacheData data("http://localhost:8080/a.jpeg"); 
      data.setAttributes((char *)cacheHeader);

      map<rtString, rtString> retrivedAttributes;
      map<rtString, rtString> actualAttributes;

      actualAttributes["HTTP/1.1 200 OK"] = "";
      actualAttributes["Date"] = " Sun, 09 Oct 2016 21:22:50 GMT";
      actualAttributes["Server"] = " Apache/2.4.7 (Ubuntu)";
      actualAttributes["Last-Modified"] = " Sat, 08 Oct 2017 02:46:40 GMT";
      actualAttributes["ETag"] = " \"fb4-53e51895552f0\"";
      actualAttributes["Accept-Ranges"] = " bytes";
      actualAttributes["Cache-Control"] = " no-store, public";
      actualAttributes["Content-Length"] = " 4020";
      actualAttributes["Expires"] = " Mon, 10 Oct 2017 21:22:50 GMT";
      actualAttributes["Content-Type"] = " image/jpeg";
      data.attributes(retrivedAttributes);
      for (map<rtString,rtString>::iterator it =  actualAttributes.begin(); it != actualAttributes.end(); it++)
      {
        EXPECT_TRUE(actualAttributes[it->first] == retrivedAttributes[it->first]);
      }
    }
  
    void  initDataTest()
    {
      const char* cacheHeader = "\nHTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      rtHttpCacheData data("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData)); 
      rtData& storedData = data.contentsData();
      EXPECT_TRUE ( strcmp(cacheData,(const char*)storedData.data()) == 0);
    }
  
    void  setDataTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      rtHttpCacheData data("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData)); 
      rtData newData;
      char* newcontents = "pqrstu";
      newData.init((uint8_t*)newcontents,strlen(newcontents));
      data.setData(newData);
      rtData& storedData = data.contentsData();
      EXPECT_TRUE ( strcmp("pqrstu",(const char*)storedData.data()) == 0);
    }

    void  readEtagTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData));
      rtString tag;
      rtError ret = data.etag(tag);
      EXPECT_TRUE (ret == RT_OK);
      EXPECT_TRUE (strcmp(tag.cString(), " \"fb4-53e51895552f0\"") == 0);
    }

    void  readEtagNotPresentTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData));
      rtString tag;
      rtError ret = data.etag(tag);
      EXPECT_TRUE (ret == RT_ERROR);
    }

    void readDataFileAccessFailedTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost:8080/a.jpeg",cacheHeader, cacheData, strlen(cacheData));
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR);
    }

    void nocacheCompleteResponseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR); //This is test page, so verifying download is happening or not
    }

    void nocacheExpiresParamTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache=Expires, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR);
    }

    void mustRevalidateUnExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public must-revalidate\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data("http://localhost:8080/test.jpeg");
      rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
      bool revalidate = false,revalidateOnlyHeaders = false;
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == false);
    }

    void mustRevalidateTrueExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public must-revalidate\nExpires: Mon, 10 Oct 2015 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data("http://localhost:8080/test.jpeg");
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == true);
    }

    void mustRevalidateFalseExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2015 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtHttpCacheData data("http://localhost:8080/test.jpeg");
      rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == false);
   }

   void mustRevalidateFalseExpiredContentsInvalidTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2015 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost:8080/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
     rtData contents;
     EXPECT_TRUE (RT_ERROR == data.data(contents));
   }

   void mustRevalidateTruenocacheUnExpiredTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost:8080/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidate == true);
   }

   void mustRevalidateTruenocacheExpiresFiledTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\nCache-Control: public no-cache=Expires\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost:8080/test.jpeg");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidateOnlyHeaders == true);
     EXPECT_TRUE (revalidate == false);
   }

   void dataPresentAfterHeadersRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\nCache-Control: public no-cache=Expires\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/dataRevalidationUnUpdated",cacheHeader,cacheData,strlen(cacheData));

     FILE* fp  = fopen("dataRevalidationUnUpdated","w");
     fprintf(fp, "abcde");
     fclose(fp);
     bool sysret = system("cp dataRevalidationUnUpdated /var/www/.");
     sysret = system("rm dataRevalidationUnUpdated");

     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost:8080/dataRevalidationUnUpdated");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/dataRevalidationUnUpdated",data);
     rtData contents;
     data.data(contents);
     EXPECT_TRUE ( strcmp(cacheData,(const char*)contents.data()) == 0);
   }

   void dataPresentAfterFullRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/dataRevalidationUnUpdated",cacheHeader,cacheData,strlen(cacheData));

     FILE* fp  = fopen("dataRevalidationUnUpdated","w");
     fprintf(fp, "abcde");
     fclose(fp);
     bool sysret = system("cp dataRevalidationUnUpdated /var/www/.");
     sysret = system("rm dataRevalidationUnUpdated");
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost:8080/dataRevalidationUnUpdated");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/dataRevalidationUnUpdated",data);
     rtData contents;
     data.data(contents);
     EXPECT_TRUE ( strcmp(cacheData,(const char*)contents.data()) == 0);
   }

   void dataUpdatedAfterFullRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0";
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/testRevalidationUpdate",cacheHeader,cacheData,strlen(cacheData));

     FILE* fp  = fopen("testRevalidationUpdate","w");
     fprintf(fp, "data updated");
     fclose(fp);
     bool sysret = system("cp testRevalidationUpdate /var/www/.");
     sysret = system("rm testRevalidationUpdate");
     rtHttpCacheData data("http://localhost:8080/testRevalidationUpdate");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/testRevalidationUpdate",data);
     rtData contents;
     data.data(contents);
     rtData& storedData = data.contentsData();
     EXPECT_TRUE ( strcmp("data updated",(const char*)storedData.data()) == 0);
     sysret = system("rm -rf /var/www/testRevalidationUpdate");
   }

   void dataNotUpdatedAfterFullRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0";
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/testRevalidationUpdateFailed",cacheHeader,cacheData,strlen(cacheData));
     rtHttpCacheData data("http://localhost:8080/testRevalidationUpdateFailed");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/testRevalidationUpdateFailed",data);
     rtData contents;
     EXPECT_TRUE (RT_ERROR == data.data(contents));
   }

    void dataUpdatedAfterEtagTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/testEtag",cacheHeader,cacheData,strlen(cacheData));
     FILE* fp  = fopen("testEtag","w");
     fprintf(fp, "data updated");
     fclose(fp);
     bool sysret = system("cp testEtag /var/www/.");
     sysret = system("rm testEtag");
     rtHttpCacheData data("http://localhost:8080/testEtag");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/testEtag",data);
     rtData contents;
     data.data(contents);
     rtData& storedData = data.contentsData();
     EXPECT_TRUE ( strcmp("data updated",(const char*)storedData.data()) == 0);
     sysret = system("rm -rf /var/www/testEtag");
    }

    void dataUpdatedAfterEtagDownloadFailedTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost:8080/testEtag",cacheHeader,cacheData,strlen(cacheData));
     rtHttpCacheData data("http://localhost:8080/testEtag");
     rtFileCache::instance()->httpCacheData("http://localhost:8080/testEtag",data);
     rtData contents;
     rtError ret = data.data(contents);
     EXPECT_TRUE (0 == contents.length());
    }

    void memoryUnAvailableTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\nabcdefghijklmnopqrstuvwxyz\0";
      addDataToCache("http://localhost:8080/testEtag",cacheHeader,cacheData,strlen(cacheData));
      FILE* fp  = fopen("testEtag","w");
      fprintf(fp, "data updated");
      fclose(fp);
      bool sysret = system("cp testEtag /var/www/.");
      sysret = system("rm testEtag");
      rtHttpCacheData data("http://localhost:8080/testEtag");
      rtFileCache::instance()->httpCacheData("http://localhost:8080/testEtag",data);
      failRealloc = true;
      EXPECT_TRUE (false == data.readFileData());
      failRealloc = false;
      sysret = system("rm /var/www/testEtag");
    }

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
  dataPresentAfterHeadersRevalidationTest();
  dataPresentAfterFullRevalidationTest();
  dataUpdatedAfterFullRevalidationTest();
  dataNotUpdatedAfterFullRevalidationTest();
  dataUpdatedAfterEtagTest();
  dataUpdatedAfterEtagDownloadFailedTest();
  memoryUnAvailableTest();
}

class rtFileDownloaderTest : public testing::Test, public commonTestFns
{
  public:
    virtual void SetUp()
    {
      testSem = sem_open("/semaphore", O_CREAT, 0644, 1);
      contentsData  = NULL;
      expirationDate = 0;
      expectedStatusCode = 0;
      expectedHttpCode = 0;
      expectedCachePresence = false;
      headerData = "";
      contentDataSize = 0;
      continueDownloadHandleCheck = false;
      bool sysRet = system("wget https://cdn.pixabay.com/photo/2013/11/22/02/06/lotus-215460_960_720.jpg");
      if (0 == sysRet)
      {
        mDownloadImageFailed = false;
        sysRet = system("cp lotus-215460_960_720.jpg /var/www/sampleimage.jpeg");
        sysRet = system("cp lotus-215460_960_720.jpg supportfiles/sampleimage.jpeg");
      }
    }

    virtual void TearDown()
    {
      if (NULL != contentsData)
        free (contentsData);
      contentDataSize = 0;
      int ret = sem_close(testSem);
      ret = sem_unlink("/semaphore");
      mDownloadImageFailed = true;
    }

    void downloadFileCacheDataUnAvailableTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/test.js",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 404;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void downloadFileCacheDataExpiredAvailableNoRevalidateTest()
    {
      rtFileCache::instance()->clearCache();
      addDataToCache("http://localhost:8080/a.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5);
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/a.jpeg",this);
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
      readFile();
      addDataToCache("http://localhost:8080/sampleimage1.jpeg",headerData.c_str(),contentsData,contentDataSize);
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage1.jpeg",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void downloadFileAddToCacheTest()
    {
      printf("downloadFileAddToCacheTest begin ********************** \n");
      fflush(stdout);
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/test.html",this);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);

      rtHttpCacheData data("http://localhost:8080/test.html");
      EXPECT_TRUE (RT_OK ==rtFileCache::instance()->httpCacheData("http://localhost:8080/test.html",data));
      printf("downloadFileAddToCacheTest end ********************** \n");
      fflush(stdout);
    }

    void downloadFileCacheDataUpdateAgainTest()
    {
      rtFileCache::instance()->clearCache();
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0";
      const char* cacheData = "abcde";
      addDataToCache("http://localhost:8080/testRevalidationUpdate",cacheHeader,cacheData,strlen(cacheData));

      FILE* fp  = fopen("testRevalidationUpdate","w");
      fprintf(fp, "data updated");
      fclose(fp);
      bool sysret = system("cp testRevalidationUpdate /var/www/.");
      sysret = system("rm testRevalidationUpdate");
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/testRevalidationUpdate", this);
      request->setCallbackFunction(NULL);
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
      sysret = system("rm -rf /var/www/testRevalidationUpdate");
   }

    void checkAndDownloadFromNetworkSuccess()
    {
      if (false == mDownloadImageFailed)
      {
        rtFileCache::instance()->clearCache();
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
        EXPECT_TRUE (ret == true);
        EXPECT_TRUE (request->httpStatusCode() == 200);
        EXPECT_TRUE (request->downloadStatusCode() == 0);
        delete request;
      }
    }

    void checkAndDownloadFromNetworkFailure()
    {
      rtFileCache::instance()->clearCache();
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage2.jpeg",this);
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      EXPECT_TRUE (request->httpStatusCode() == 404);
      delete request;
    }

    void disableCacheTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage2.jpeg",this);
      request->setCacheEnabled(false);
      request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 404;
      expectedCachePresence = false;
      rtFileDownloader::instance()->downloadFile(request);
      sem_wait(testSem);
    }

    void startFileDownloadInBackgroundTest()
    {
      if (false == mDownloadImageFailed)
      {
        rtFileCache::instance()->clearCache();
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
        bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
        expectedStatusCode = 0;
        expectedHttpCode = 200;
        startFileDownloadInBackground(request);
        sem_wait(testSem);
      }
    }

    void setFileUrlTest()
    {
      if (false == mDownloadImageFailed)
      {
        rtFileCache::instance()->clearCache();
        rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
        request->setFileUrl("http://localhost:8080/sampleimage.jpeg");
        request->setCallbackFunction(rtFileDownloaderTest::downloadCallback);
        bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
        expectedStatusCode = 0;
        expectedHttpCode = 200;
        startFileDownloadInBackground(request);
        sem_wait(testSem);
      }
    }

    void setProxyTest()
    {
      if (false == mDownloadImageFailed)
      {
        rtFileCache::instance()->clearCache();
        rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
        request->setFileUrl("http://localhost1/sampleimage.jpeg");
        request->setProxy("undefined");
        bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
        EXPECT_TRUE (ret == false);
        EXPECT_TRUE (strcmp( request->errorString(), "") != 0);
      }
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
      request->setCallbackFunctionThreadSafe(rtFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedCachePresence = false;
      expectedHttpCode = 0;
      EXPECT_TRUE (request->executeCallback(0) == true);
      sem_wait(testSem);
    }

    void setCallbackFunctionNullTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",this);
      request->setCallbackFunctionThreadSafe(NULL);
      EXPECT_TRUE (request->executeCallback(0) == false);
    }

    void setCallbackFunctionNullInDownloadFileTest()
    {
      if (false == mDownloadImageFailed)
      {
        void (*callbackFunction)(rtFileDownloadRequest*);
        callbackFunction = rtFileDownloader::instance()->mDefaultCallbackFunction;
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        request->setCallbackFunctionThreadSafe(NULL);
        rtFileDownloader::instance()->setDefaultCallbackFunction(rtFileDownloaderTest::defaultDownloadCallback);
        rtFileDownloader::instance()->downloadFile(request);
        sem_wait(testSem);
        EXPECT_TRUE (defaultCallbackExecuted ==true);
        defaultCallbackExecuted = false;
        rtFileDownloader::instance()->setDefaultCallbackFunction(callbackFunction);
      }
    }

    void setDefaultCallbackFunctionNullTest()
    {
      if (false == mDownloadImageFailed)
      {
        void (*callbackFunction)(rtFileDownloadRequest*);
        callbackFunction = rtFileDownloader::instance()->mDefaultCallbackFunction;
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        request->setCallbackFunction(NULL);
        rtFileDownloader::instance()->setDefaultCallbackFunction(NULL);
        rtFileDownloader::instance()->downloadFile(request);
        EXPECT_TRUE (200 == request->httpStatusCode());
        rtFileDownloader::instance()->setDefaultCallbackFunction(callbackFunction);
      }
    }

    void setCallbackDataTest()
    {
      rtFileDownloadRequest* request = new rtFileDownloadRequest("",NULL);
      request->setCallbackFunctionThreadSafe(rtFileDownloaderTest::downloadCallback);
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
      request->setDownloadHandleExpiresTime(30);
      EXPECT_TRUE (request->downloadHandleExpiresTime() == 30);
    }

    void downloadedDataTest()
    {
      rtFileCache::instance()->clearCache();
      readFile();
      addDataToCache("http://localhost:8080/sampleimage1.jpeg",headerData.c_str(),contentsData,contentDataSize);
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage1.jpeg",this);
      bool ret = rtFileDownloader::instance()->downloadFromNetwork(request);
      char *data = new char [1000];
      size_t size = 0;
      memset (data, 0, 1000);
      request->downloadedData(data, size);
      EXPECT_TRUE (size > 0);
      delete[] data;
    }

    void addToDownloadQueueTest()
    {
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      rtFileCache::instance()->clearCache();
      readFile();
      addDataToCache("http://localhost:8080/sampleimage1.jpeg",headerData.c_str(),contentsData,contentDataSize);
      rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage1.jpeg",this);
      request->setCallbackFunctionThreadSafe(rtFileDownloaderTest::downloadCallback);
      rtFileDownloader::instance()->addToDownloadQueue(request);
      sleep(5);
      sem_wait(testSem);
    }

    void startNextDownloadInBackgroundTest()
    {
      //todo more actions once startNextDownloadInBackground() is implemented
      rtFileDownloader::instance()->startNextDownloadInBackground();
    }

    void raiseDownloadPriorityTest()
    {
      if (false == mDownloadImageFailed)
      {
        expectedStatusCode = 0;
        expectedHttpCode = 200;
        expectedCachePresence = false;
        rtFileCache::instance()->clearCache();
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        request->setCallbackFunctionThreadSafe(rtFileDownloaderTest::downloadCallback);
        rtFileDownloader::instance()->addToDownloadQueue(request);
        rtFileDownloader::instance()->raiseDownloadPriority(request);
        sleep(5);
        sem_wait(testSem);
      }
    }

    void nextDownloadRequestTest()
    {
      //todo when nextDownloadRequest() is implemented
      EXPECT_TRUE (rtFileDownloader::instance()->nextDownloadRequest() == NULL);
    }

    void removeDownloadRequestTest()
    {
      if (false == mDownloadImageFailed)
      {
        //todo more actions once removeDownloadRequest() is implemented
        rtFileDownloadRequest* request = new rtFileDownloadRequest("http://localhost:8080/sampleimage.jpeg",this);
        rtFileDownloader::instance()->removeDownloadRequest(request);
      }
    }

    void clearFileCacheTest()
    {
      //todo more actions once clearFileCache() is implemented
      rtFileDownloader::instance()->clearFileCache();
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

  private:
    int expectedStatusCode;
    int expectedHttpCode;
    bool expectedCachePresence;
    string headerData;
    char *contentsData;
    int contentDataSize;
    time_t expirationDate;
    void readFile();
    sem_t* testSem;
    bool mDownloadImageFailed;
};

void rtFileDownloaderTest::readFile()
{
  FILE* fp  = fopen("image","r");
  char buffer[100];
  int bytesCount = 0;
  int totalBytes = 0;
  char indChar;

  while ( !feof(fp) )
  {
    indChar = fgetc(fp);
    if (indChar == '|')
    {
      break;
    }
    headerData.append(1,indChar);
  }

  char buf;
  string date;
  while ( !feof(fp) )
  {
    buf = fgetc(fp);
    if (buf == '|')
    {
      break;
    }
    date.append(1,buf);
  }

  stringstream stream(date);
  stream >> expirationDate;

  while (!feof(fp) )
  {
    bytesCount = fread(buffer,1,100,fp);
    if (NULL == contentsData)
      contentsData = (char *)malloc(bytesCount);
    else
      contentsData = (char *)realloc(contentsData,totalBytes+bytesCount);
    if (NULL == contentsData)
    {
      fclose(fp);
      return;
    }
    memcpy(contentsData+totalBytes,buffer,bytesCount);
    totalBytes += bytesCount;
    memset(buffer,0,100);
  }
  fclose(fp);
  contentDataSize = totalBytes;
}

TEST_F(rtFileDownloaderTest, checkCacheTests)
{
  downloadFileCacheDataUnAvailableTest();
  downloadFileCacheDataExpiredAvailableNoRevalidateTest();
  downloadFileCacheDataProperAvailableTest();
  downloadFileCacheDataUpdateAgainTest();
  downloadFileAddToCacheTest();
  checkAndDownloadFromNetworkSuccess();
  checkAndDownloadFromNetworkFailure();
  disableCacheTest();
  startFileDownloadInBackgroundTest(); 
  setFileUrlTest();
  setProxyTest();
  errorStringTest();
  setCallbackFunctionThreadSafeTest();
  setCallbackFunctionNullTest();
  setCallbackFunctionNullInDownloadFileTest();
  setDefaultCallbackFunctionNullTest();
  setCallbackDataTest();
  setDownloadHandleExpiresTimeTest();
  downloadedDataTest();
  startNextDownloadInBackgroundTest();
  addToDownloadQueueTest();
  raiseDownloadPriorityTest();
  nextDownloadRequestTest();
  removeDownloadRequestTest();
  clearFileCacheTest();
}
