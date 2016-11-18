#include "gtest/gtest.h"
#include <list>
#define private public
#define protected public
#include "pxFileCache.h"
#include "pxHttpCache.h"
#include "pxResource.h"
#include "pxFileDownloader.h"
#include "rtString.h"
#include "pxScene2d.h"
#include <string.h>
#include <sstream>

class commonTestFns
{
  protected:
     rtError addDataToCache(const char* url, const char* headerMetaData, const char* data, int size)
     {
       rtHttpCacheData cacheData(url,headerMetaData,data,size);
       return rtFileCache::getInstance()->addToCache(cacheData);
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
      rtFileCache::getInstance()->clearCache();
      rtFileCache::destroy();      
    }

    void fileCacheSetMaxCacheSizeTest ()
    { 
       rtFileCache::getInstance()->setMaxCacheSize(200);
       int64_t cacheSize;
       cacheSize  = rtFileCache::getInstance()->maxCacheSize();
       EXPECT_TRUE (cacheSize == 200);
    }
    
    void fileCacheSetDirectoryTest()
    { 
      rtFileCache::getInstance()->setCacheDirectory("/tmp/cache");
      rtString directory;
      rtFileCache::getInstance()->cacheDirectory(directory);
      EXPECT_TRUE (directory == "/tmp/cache");
    }
    
    void fileCacheRemoveDataUrlNullTest()
    { 
      EXPECT_TRUE (rtFileCache::getInstance()->removeData(NULL) == RT_ERROR);
    }
    
    void fileCacheRemoveDataUrlEmptyTest()
    { 
      EXPECT_TRUE (rtFileCache::getInstance()->removeData("") == RT_OK);
    }
    
    void fileCacheRemoveDataUrlUnavailableTest()
    { 
      EXPECT_TRUE (rtFileCache::getInstance()->removeData("http://localhost/a.jpeg") == RT_OK);
    }
    
    void fileCacheRemoveDataUrlAvailableTest()
    { 
      resetAndAddCacheData();
      EXPECT_TRUE (rtFileCache::getInstance()->removeData("http://localhost/a.jpeg") == RT_OK); 
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == 0); 
    }
    
    void fileCacheRemoveDataSingleUrlAvailableTest()
    { 
      resetAndAddCacheData();
      addDataToCache("http://localhost/b.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5);
      EXPECT_TRUE (rtFileCache::getInstance()->removeData("http://localhost/a.jpeg") == RT_OK); 
      rtHttpCacheData data;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/b.jpeg",data);
      stringstream stream;
      stream << data.expirationDateUnix();
      string date = stream.str().c_str();
      int expectedSize = strlen("Expires: Sun 02 Oct 2016 22:33:33 UTC") + 1 + strlen("abcde") + 1 + date.length();
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == expectedSize); 
    }
    
    void fileCacheRemoveDataNonWritableTest()
    { 
      resetAndAddCacheData();
      system("chmod 400 /tmp/cache");
      EXPECT_TRUE (rtFileCache::getInstance()->removeData(NULL) == RT_ERROR);
      rtHttpCacheData data;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/a.jpeg",data);
      stringstream stream;
      stream << data.expirationDateUnix();
      string date = stream.str().c_str();
      int expectedSize = strlen("Expires: Sun 02 Oct 2017 22:33:33 UTC") + 1 + strlen("abcde") + 1 + date.length(); //11 is the size of expiration date
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == expectedSize); 
      system("chmod 777 /tmp/cache");
    }
    
    void fileCacheClearCacheTest()
    { 
      rtFileCache::getInstance()->clearCache();
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == 0);
    }
    
    void fileCacheGetHttpCacheDataAvailableTest()
    { 
      resetAndAddCacheData();
      rtHttpCacheData data;
      EXPECT_TRUE (rtFileCache::getInstance()->getHttpCacheData("http://localhost/a.jpeg",data) == RT_OK);
    }
    
    void fileCacheGetHttpCacheDataUnAvailableTest()
    { 
      resetAndAddCacheData();
      rtHttpCacheData data;
      EXPECT_TRUE (rtFileCache::getInstance()->getHttpCacheData("http://localhost/b.jpeg",data) == RT_ERROR);
    }
    
    void fileCacheAddNullUrlToCacheTest()
    {
      rtFileCache::getInstance()->clearCache();
      EXPECT_TRUE (addDataToCache(NULL,"Expires: Sun 02 Oct 2016 22:33:33 UTC","abcdef",6) == RT_ERROR);
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == 0);
    }
    
    void fileCacheAddProperUrlToCacheTest()
    {
      resetAndAddCacheData();
      rtHttpCacheData data;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/a.jpeg",data);
      int expectedSize = strlen("Expires: Sun 02 Oct 2017 22:33:33 UTC") + 1 + + strlen("abcde") + 1 + to_string(data.expirationDateUnix()).length();
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == expectedSize);
    }
    
    void fileCacheAddProperUrlToCacheNonWritableTest()
    {
      rtFileCache::getInstance()->clearCache();
      system("chmod 400 /tmp/cache");
      EXPECT_TRUE (addDataToCache("http://localhost/a.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5) == RT_ERROR);
      EXPECT_TRUE (rtFileCache::getInstance()->cacheSize() == 0);
      system("chmod 777 /tmp/cache");
    }

  private:
     
     void resetAndAddCacheData()
     {
       rtFileCache::getInstance()->clearCache();
       addDataToCache("http://localhost/a.jpeg","Expires: Sun 02 Oct 2017 22:33:33 UTC\0","abcde",5);
     }
};

TEST_F(pxFileCacheTest, fileCacheCompleteTest)
{
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
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isValid() == true);
    }
  
    void  dataValidityFailureEmptyImageTest()
    {
      rtHttpCacheData data("http://localhost/a.jpeg"); 
      EXPECT_TRUE (data.isValid() == false);
    }
 
    void  dataValidityFailureExpiredImageTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=0, public\nExpires: Mon, 10 Oct 2016 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isValid() == false);
    }

    void  dataExpiredTrueTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2016 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=0, public\nExpires: Mon, 10 Oct 2016 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isExpired() == true);
    }
  
    void  dataExpiredFalseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isExpired() == false);
    }
  
    void  expirationDateTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.expirationDate() == "2017-10-11 02:52:50");
    }
  
    void  dataWritableToCacheTrueTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: max-age=2000, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg"; 
      const char* cacheData = "This is image data";
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, cacheData, strlen(cacheData)); 
      EXPECT_TRUE (data.isWritableToCache() == true);
    }
  
    void  dataWritableToCacheFalseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg"; 
      rtHttpCacheData data("http://localhost/a.jpeg",cacheHeader, NULL, 0); 
      EXPECT_TRUE (data.isWritableToCache() == false);
    }
  
    void  setAttributesTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      rtHttpCacheData data("http://localhost/a.jpeg"); 
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
  
    void  setDataTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-store, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      rtHttpCacheData data("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData)); 
      rtData& storedData = data.getContentsData();
      EXPECT_TRUE ( strcmp(cacheData,(const char*)storedData.data()) == 0);
    }
  
    void nocacheCompleteResponseTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR); //This is test page, so verifying download is happening or not
    }

    void nocacheExpiresParamTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache=Expires, public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_ERROR);
    }

    void mustRevalidateUnExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2016 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2017 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data("http://localhost/test.jpeg");;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
      rtData contents;
      EXPECT_TRUE (data.data(contents) == RT_OK);
    }

    void mustRevalidateTrueExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public must-revalidate\nExpires: Mon, 10 Oct 2015 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      rtHttpCacheData data("http://localhost/test.jpeg");
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == true);
    }

    void mustRevalidateFalseExpiredTest()
    {
      const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: public\nExpires: Mon, 10 Oct 2015 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
      const char* cacheData = "abcde";
      addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
      bool revalidate = false,revalidateOnlyHeaders = false;
      rtHttpCacheData data("http://localhost/test.jpeg");
      rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
      data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
      EXPECT_TRUE (revalidate == false);
   }

   void mustRevalidateTruenocacheUnExpiredTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost/test.jpeg");
     rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidate == true);
   }

   void mustRevalidateTruenocacheExpiresFiledTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\nCache-Control: public no-cache=Expires\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost/test.jpeg");
     rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
     data.calculateRevalidationNeed(revalidate,revalidateOnlyHeaders);
     EXPECT_TRUE (revalidateOnlyHeaders == true);
     EXPECT_TRUE (revalidate == false);
   }

   void dataPresentAfterHeadersRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\nCache-Control: public no-cache=Expires\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost/test.jpeg");
     rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
     rtData contents;
     data.data(contents);
     EXPECT_TRUE ( strcmp(cacheData,(const char*)contents.data()) == 0);
   }

   void dataPresentAfterFullRevalidationTest()
   {
     const char* cacheHeader = "HTTP/1.1 200 OK\nDate: Sun, 09 Oct 2015 21:22:50 GMT\nServer: Apache/2.4.7 (Ubuntu)\nLast-Modified: Sat, 08 Oct 2015 02:46:40 GMT\nETag: \"fb4-53e51895552f0\"\nAccept-Ranges: bytes\nContent-Length: 4020\nCache-Control: no-cache\nExpires: Mon, 10 Oct 2017 21:22:50 GMT\nContent-Type: image/jpeg\n\0"; 
     const char* cacheData = "abcde";
     addDataToCache("http://localhost/test.jpeg",cacheHeader,cacheData,strlen(cacheData));
     bool revalidate = false,revalidateOnlyHeaders = false;
     rtHttpCacheData data("http://localhost/test.jpeg");
     rtFileCache::getInstance()->getHttpCacheData("http://localhost/test.jpeg",data);
     rtData contents;
     data.data(contents);
     rtData& storedData = data.getContentsData();
     EXPECT_TRUE ( strcmp(cacheData,(const char*)storedData.data()) == 0);
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
  dataWritableToCacheFalseTest();
  setAttributesTest();
  setDataTest();
  nocacheCompleteResponseTest();
  nocacheExpiresParamTest();
  mustRevalidateUnExpiredTest();
  mustRevalidateTrueExpiredTest();
  mustRevalidateFalseExpiredTest();
  mustRevalidateTruenocacheUnExpiredTest();
  mustRevalidateTruenocacheExpiresFiledTest();
  dataPresentAfterHeadersRevalidationTest();
  //dataPresentAfterFullRevalidationTest();
}

class pxFileDownloaderTest : public testing::Test, public commonTestFns
{
  public:
    virtual void SetUp()
    {
      contentsData  = NULL;
      expirationDate = 0;
      expectedStatusCode = 0;
      expectedHttpCode = 0;
      expectedCachePresence = false;
      headerData = "";
      contentDataSize = 0;
    }

    virtual void TearDown()
    {
      //rtFileCache::getInstance()->clearCache();
      if (NULL != contentsData)
        free (contentsData);
      contentDataSize = 0;
    }

    void downloadFileCacheDataUnAvailableTest()
    {
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/test.js",this);
      request->setCallbackFunction(pxFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 404;
      expectedCachePresence = false;
      pxFileDownloader::getInstance()->downloadFile(request);
    }

    void downloadFileCacheDataExpiredAvailableNoRevalidateTest()
    {
      rtFileCache::getInstance()->clearCache();
      addDataToCache("http://localhost/a.jpeg","Expires: Sun 02 Oct 2016 22:33:33 UTC","abcde",5);
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/a.jpeg",this);
      request->setCallbackFunction(pxFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      pxFileDownloader::getInstance()->downloadFile(request);
    }

    void downloadFileCacheDataProperAvailableTest()
    {
      rtFileCache::getInstance()->clearCache();
      readFile();
      addDataToCache("http://localhost/sampleimage.jpeg",headerData.c_str(),contentsData,contentDataSize);
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/sampleimage.jpeg",this);
      request->setCallbackFunction(pxFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 200;
      expectedCachePresence = true;
      pxFileDownloader::getInstance()->downloadFile(request);
    }

    void checkAndDownloadFromNetworkSuccess()
    {
      rtFileCache::getInstance()->clearCache();
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/sampleimage.jpeg",this);
      bool ret = pxFileDownloader::getInstance()->downloadFromNetwork(request);
      EXPECT_TRUE (ret == true);
      EXPECT_TRUE (request->getHttpStatusCode() == 200);
      EXPECT_TRUE (request->getDownloadStatusCode() == 0);
      delete request;
    }

    void checkAndDownloadFromNetworkFailure()
    {
      rtFileCache::getInstance()->clearCache();
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/sampleimage1.jpeg",this);
      bool ret = pxFileDownloader::getInstance()->downloadFromNetwork(request);
      EXPECT_TRUE (request->getHttpStatusCode() == 404);
      delete request;
    }

    void disableCacheTest()
    {
      pxFileDownloadRequest* request = new pxFileDownloadRequest("http://localhost/sampleimage1.jpeg",this);
      request->setCacheEnabled(false);
      request->setCallbackFunction(pxFileDownloaderTest::downloadCallback);
      expectedStatusCode = 0;
      expectedHttpCode = 404;
      expectedCachePresence = false;
      pxFileDownloader::getInstance()->downloadFile(request);
    }

    static void downloadCallback(pxFileDownloadRequest* fileDownloadRequest)
    {
      rtHttpCacheData cachedData;
      if (fileDownloadRequest != NULL && fileDownloadRequest->getCallbackData() != NULL)
      {
        pxFileDownloaderTest* callbackData = (pxFileDownloaderTest*) fileDownloadRequest->getCallbackData();
        EXPECT_TRUE (callbackData->expectedHttpCode == fileDownloadRequest->getHttpStatusCode());
        EXPECT_TRUE (callbackData->expectedStatusCode ==  fileDownloadRequest->getDownloadStatusCode());
        EXPECT_TRUE (callbackData->expectedCachePresence == pxFileDownloader::getInstance()->checkAndDownloadFromCache(fileDownloadRequest,cachedData)); 
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
};

void pxFileDownloaderTest::readFile()
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

TEST_F(pxFileDownloaderTest, checkCacheTests)
{
  downloadFileCacheDataUnAvailableTest();
  downloadFileCacheDataExpiredAvailableNoRevalidateTest();
  downloadFileCacheDataProperAvailableTest();
  checkAndDownloadFromNetworkSuccess();
  checkAndDownloadFromNetworkFailure();
  disableCacheTest();
}
