#ifndef _RT_FILECACHE
#define _RT_FILECACHE

#include <stdlib.h>
#include <string>
#include <map>
#include "rtRef.h"
#include "rtString.h"
#include "rtFile.h"
#include "rtLog.h"
#include "rtHttpCache.h"
#include "rtMutex.h"

class rtFileCache
{
  public:
    /* set the maximum cache size. Default value is 20 MB */
    rtError setMaxCacheSize(int64_t bytes);

    /* returns the maximum cache size */
    int64_t maxCacheSize(); 

    /* returns the current cache size */
    int64_t cacheSize();

    /* sets the cache directory.Returns RT_OK on success and RT_ERROR on failure */
    rtError setCacheDirectory(const char* directory);

    /* returns the cache directory provisioned */
    rtError cacheDirectory(rtString&); 

    /* removes the data from the cache for the url. Returns RT_OK on success and RT_ERROR on failure */
    rtError removeData(const char* url);

    /* add the header,image data corresponding to a url to file cache. Returns RT_OK on success and RT_ERROR on failure */
    rtError addToCache(const rtHttpCacheData& data); 

    /* get the header,image data corresponding to a url from file cache. Returns RT_OK on success and RT_ERROR on failure */
    rtError httpCacheData(const char* url, rtHttpCacheData& cacheData);

    /* clear the complete cache */
    void clearCache();

    static rtFileCache* instance();
 
    static void destroy();
  private:
    /* private functions */
    rtFileCache();
    ~rtFileCache();

    /* initialize the cache */
    void initCache();

    /* cleans the cache till the size is more than cache data size and return the new size */
    int64_t cleanup(); 

    /* set the file size and time associated with the file */
    void setFileSizeAndTime(rtString& filename);

    /* calculates and returns the hash value of the url */
    rtString hashedFileName(const rtString& url);

    /* write the cache data to a file. Returns true on success and false on failure */
    bool writeFile(rtString& filename, const rtHttpCacheData& cacheData);

    /* delete the file from cache */
    bool deleteFile(rtString& filename);

    /* read the file and populate the header data only */
    bool readFileHeader(rtString& filename,rtHttpCacheData& cacheData);

    /* returns the filename in absolute path format */
    rtString absPath(rtString& filename);

    /* populate the existing files in cache along with size in mFileSizeMap */
    void populateExistingFiles();

    /* member variables */
    int64_t mMaxSize;
    int64_t mCurrentSize;
    rtString mDirectory;
    std::hash<std::string> hashFn;
    std::multimap<time_t,rtString> mFileTimeMap;
    std::map<rtString,int64_t> mFileSizeMap;
    rtMutex mCacheMutex;
    static rtFileCache* mCache;
};
#endif
