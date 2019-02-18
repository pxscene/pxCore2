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

#include "rtString.h"
#include "pxScene2d.h"

#include "test_includes.h" // Needs to be included last

#ifndef UNUSED_PARAM
#define UNUSED_PARAM(x) (void (x))
#endif

using namespace std;

// NOTE:  [BAD_svg] SVG data has '\0' null terminators tainting it - in a manner similar to Nanosvg taints whilst parsing.
//
rtString  TV_svg("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'><path fill='#fff' d='M592 0H48C21.5 0 0 21.5 0 48v320c0 26.5 21.5 48 48 48h245.1v32h-160c-17.7 0-32 14.3-32 32s14.3 32 32 32h384c17.7 0 32-14.3 32-32s-14.3-32-32-32h-160v-32H592c26.5 0 48-21.5 48-48V48c0-26.5-21.5-48-48-48zm-16 352H64V64h512v288z'/></svg>");
rtString BAD_svg("<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 640 512'\0<path fill='#fff' d='M592 0H48C21.5 0 0 21.5 0 48v320c0 26.5 21.5 48 48 48h245.1v32h-160c-17.7 0-32 14.3-32 32s14.3 32 32 32h384c17.7 0 32-14.3 32-32s-14.3-32-32-32h-160v-32H592c26.5 0 48-21.5 48-48V48c0-26.5-21.5-48-48-48zm-16 352H64V64h512v288z'\0</svg\0");

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void onDownloadCompleteGOOD(rtFileDownloadRequest* req)
{
  char *data = NULL;
  size_t len = 0;

  req->downloadedData(data, len);

  pxOffscreen o;
  pxLoadImage(data, len, o, 300, 300); // Raster @ WxH: 300 x 300 (aspect fill)

  printf(  "\nDEBUG:  GOOD >> img:  WxH: %d x %d \n\n", o.width(), o.height());

  // Expected to PASS.
  EXPECT_TRUE (o.width()  == 300);
  EXPECT_TRUE (o.height() == 240);
}
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

void onDownloadCompleteBAD(rtFileDownloadRequest* req)
{
  char *data = NULL;
  size_t len = 0;

  req->downloadedData(data, len);

  pxOffscreen o;
  pxLoadImage(data, len, o, 300, 300); // Raster @ WxH: 300 x 300 (aspect fill)

  // printf(  "\nDEBUG:  BAD >> img:  WxH: %d x %d \n\n", o.width(), o.height());

  // Expected to PASS. But 0x0 is the BUG being tested for - we're ensuring its detectable.

  EXPECT_TRUE (o.width()  == 0);
  EXPECT_TRUE (o.height() == 0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 

class rtSvgCacheTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      rtFileCache::instance()->clearCache();

      memset(mNonExpireDate, 0, sizeof(mNonExpireDate));
      memset(mNonExpireDateVal, 0, sizeof(mNonExpireDateVal));
      memset(mExpireDate, 0, sizeof(mExpireDate));
      strcpy(mExpireDate,"Expires: Sun, 02 Oct 2016 22:33:33 UTC\n");
      time_t t = time(NULL);
      tm* timePtr = localtime(&t);
      sprintf(mNonExpireDateVal,"Sun, 02 Oct %d 22:33:33 UTC",(timePtr->tm_year+1+1900));
      sprintf(mNonExpireDate,"Expires:%s\n",mNonExpireDateVal);

      // Populate the cache
      addDataToCache("http://fileserver/tv.svg","",   TV_svg.cString() , TV_svg.length() );
      addDataToCache("http://fileserver/bad.svg","", BAD_svg.cString(), BAD_svg.length() );
    }

    virtual void TearDown()
    {
      //UNUSED_PARAM();
    }

    void firstDecodeSVG_test()
    {
      // Load UNTINTED -SVG- from cache
      rtFileDownloadRequest* good = new rtFileDownloadRequest("http://fileserver/tv.svg",NULL, onDownloadCompleteGOOD);
      rtFileDownloader::instance()->downloadFile(good);
    }

    void secondDecodeSVG_test()
    {
      // Load UNTINTED -SVG- from cache
      rtFileDownloadRequest* good = new rtFileDownloadRequest("http://fileserver/tv.svg",NULL, onDownloadCompleteGOOD);
      rtFileDownloader::instance()->downloadFile(good);
    }

    void cachedDecodeSVG_test()
    {
      // Load TINTED -SVG- from cache
      rtFileDownloadRequest* bad = new rtFileDownloadRequest("http://fileserver/bad.svg",NULL, onDownloadCompleteBAD);
      rtFileDownloader::instance()->downloadFile(bad);
    }

  private:
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

    char mNonExpireDate[1000];
    char mNonExpireDateVal[1000];
    char mExpireDate[1000];
};

TEST_F(rtSvgCacheTest, checkSvgCache)
{
  firstDecodeSVG_test();  // should PASS
  secondDecodeSVG_test(); // should PASS
  cachedDecodeSVG_test(); // should PASS ... but with 0 x 0 dimensions 
}
