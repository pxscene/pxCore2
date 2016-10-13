#ifndef _PX_HTTPCACHE
#define _PX_HTTPCACHE

#include "rtFile.h"
#include "rtString.h"
#include "rtValue.h"
#include <map>
#include <time.h>
#include <pxFileDownloader.h>

using namespace std;

class rtHttpCacheData
{
  public:
    rtHttpCacheData();
    rtHttpCacheData(const char* url);
    rtHttpCacheData(const char* url, const char* headerMetadata, const char* data, int size=0);

    ~rtHttpCacheData();
    /* returns the expiration date of the cache data in localtime */
    rtString expirationDate();

    /* returns true if cache data is expired */
    bool isExpired(); 

    /* returns true if data exists in the cache and the cache is not expired */
    bool isValid();

    /* returns true if the cache data can be saved to disk (not expired,no-store is not in cache control etc) */
    bool isWritableToCache();

    /* sets the attributes.  the rawAttributes string contains the headers string. */
    void setAttributes(const char* rawAttributes);

    /* returns a map of all the headers associated with the cached data */
    rtError attributes(map<rtString, rtString>& cacheAttributes);

    /* returns the file data in the cache.  This is a blocking call and will check the network for updated data if etag is used */
    rtError data(rtData& data);

    /* sets the image file data to be stored in cache */
    void setData(const rtData& cacheData); 

    /* returns the url associated with the cache */
    rtError url(rtString& url);

    /* returns the Etag associated with the data */
    rtError etag(rtString& tag);

    /* returns true if server has updated the image data between requests */
    bool isUpdated();

    /* returns header data */
    rtData& getHeaderData();

    /* returns image data */
    rtData& getContentsData();

    void setFilePointer(FILE* fp);

    static void onDownloadComplete(pxFileDownloadRequest* downloadRequest);
  private:
    /* populates the map with header attribute and value */
    void populateHeaderMap();
 
    /* set the expiration date of cache data based on max-age and expires field in header */
    void setExpirationDate();

    rtString mUrl;
    rtData mHeaderMetaData;
    rtData mData;
    map<rtString, rtString> mHeaderMap;
    int mDataSize;
    time_t mExpirationDate;
    FILE* fp;
    bool mUpdated;
#ifdef USE_STD_THREADS
    std::mutex mMutex;
    std::condition_variable mCond;
#else
    pthread_mutex_t mMutex;
    pthread_cond_t mCond;
#endif
    pxFileDownloadRequest* mDownloadRequest;
};
#endif
