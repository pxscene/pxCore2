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

// rtFileDownloader.h

#ifndef RT_FILE_DOWNLOADER_H
#define RT_FILE_DOWNLOADER_H

#include "rtCore.h"
#include "rtString.h"
#ifdef ENABLE_HTTP_CACHE
#include <rtFileCache.h>
#endif
#include "rtCORS.h"

// TODO Eliminate std::string
#include <string.h>
#include <vector>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#endif

#include <curl/curl.h>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

class rtFileDownloadRequest
{
public:
   rtFileDownloadRequest(const char* imageUrl, void* callbackData, void (*callbackFunction)(rtFileDownloadRequest*) = NULL);
  ~rtFileDownloadRequest();

  void setFileUrl(const char* imageUrl);
  rtString fileUrl() const;
  void setProxy(const char* proxyServer);
  rtString proxy() const;
  void setErrorString(const char* errorString);
  rtString errorString();
  void setCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*));
  void setDownloadProgressCallbackFunction(size_t (*callbackFunction)(void *ptr, size_t size, size_t nmemb, void *userData), void *userPtr);
  void setCallbackFunctionThreadSafe(void (*callbackFunction)(rtFileDownloadRequest*));
  long httpStatusCode();
  void setHttpStatusCode(long statusCode);
  bool executeCallback(int statusCode);
  size_t executeDownloadProgressCallback(void *ptr, size_t size, size_t nmemb);
  void setDownloadedData(char* data, size_t size);
  void downloadedData(char*& data, size_t& size);
  char* downloadedData();
  size_t downloadedDataSize();
  void setHeaderData(char* data, size_t size);
  char* headerData();
  size_t headerDataSize();
  void setAdditionalHttpHeaders(std::vector<rtString>& additionalHeaders);
  std::vector<rtString>& additionalHttpHeaders();
  void setDownloadStatusCode(int statusCode);
  int downloadStatusCode();
  void* callbackData();
  void setCallbackData(void* callbackData);
  void setHeaderOnly(bool val);
  bool headerOnly();
  void setDownloadHandleExpiresTime(int timeInSeconds);
  int downloadHandleExpiresTime();
#ifdef ENABLE_HTTP_CACHE
  void setCacheEnabled(bool val);
  bool cacheEnabled();
  void setDataIsCached(bool val);
  bool isDataCached();
  size_t getCachedFileReadSize(void);
  void setCachedFileReadSize(size_t cachedFileReadSize);
  void setDeferCacheRead(bool val);
  bool deferCacheRead();
  FILE* cacheFilePointer(void);
#endif //ENABLE_HTTP_CACHE
  void setProgressMeter(bool val);
  bool isProgressMeterSwitchOff();
  void setUseCallbackDataSize(bool val);
  bool useCallbackDataSize();
  void setHTTPFailOnError(bool val);
  bool isHTTPFailOnError();
  void setHTTPError(const char* httpError);
  char* httpErrorBuffer(void);
  void setCurlDefaultTimeout(bool val);
  bool isCurlDefaultTimeoutSet();
  void setCORS(const rtCORSRef& cors);
  rtCORSRef cors() const;
  void cancelRequest();
  bool isCanceled();

private:
  rtString mFileUrl;
  rtString mProxyServer;
  rtString mErrorString;
  long mHttpStatusCode;
  void (*mCallbackFunction)(rtFileDownloadRequest*);
  size_t (*mDownloadProgressCallbackFunction)(void *ptr, size_t size, size_t nmemb, void *userData);
  void *mDownloadProgressUserPtr;
  char* mDownloadedData;
  size_t mDownloadedDataSize;
  int mDownloadStatusCode;
  void* mCallbackData;
  rtMutex mCallbackFunctionMutex;
  char* mHeaderData;
  size_t mHeaderDataSize;
  std::vector<rtString> mAdditionalHttpHeaders;
  bool mHeaderOnly;
  int mDownloadHandleExpiresTime;
#ifdef ENABLE_HTTP_CACHE
  bool mCacheEnabled;
  bool mIsDataInCache;
  bool mDeferCacheRead;
  size_t mCachedFileReadSize;
#endif //ENABLE_HTTP_CACHE
  bool mIsProgressMeterSwitchOff;
  bool mHTTPFailOnError;
  char mHttpErrorBuffer[CURL_ERROR_SIZE];
  bool mDefaultTimeout;
  rtCORSRef mCORS;
  bool mCanceled;
  bool mUseCallbackDataSize;
  rtMutex mCanceledMutex;
};

struct rtFileDownloadHandle
{
  rtFileDownloadHandle(CURL* handle) : curlHandle(handle), expiresTime(-1) {}
  rtFileDownloadHandle(CURL* handle, int time) : curlHandle(handle), expiresTime(time) {}
  CURL* curlHandle;
  int expiresTime;
};

class rtFileDownloader
{
public:

    static rtFileDownloader* instance();
    static void setCallbackFunctionThreadSafe(rtFileDownloadRequest* downloadRequest, void (*callbackFunction)(rtFileDownloadRequest*), void* owner);
    static void cancelDownloadRequestThreadSafe(rtFileDownloadRequest* downloadRequest, void* owner);
    static bool isDownloadRequestCanceled(rtFileDownloadRequest* downloadRequest, void* owner);

    virtual bool addToDownloadQueue(rtFileDownloadRequest* downloadRequest);
    virtual void raiseDownloadPriority(rtFileDownloadRequest* downloadRequest);
    virtual void removeDownloadRequest(rtFileDownloadRequest* downloadRequest);

    void clearFileCache();
    void downloadFile(rtFileDownloadRequest* downloadRequest);
    void setDefaultCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*));
    bool downloadFromNetwork(rtFileDownloadRequest* downloadRequest);
    void checkForExpiredHandles();

private:
    rtFileDownloader();
    ~rtFileDownloader();

    void startNextDownload(rtFileDownloadRequest* downloadRequest);
    rtFileDownloadRequest* nextDownloadRequest();
    void startNextDownloadInBackground();
    void downloadFileInBackground(rtFileDownloadRequest* downloadRequest);
#ifdef ENABLE_HTTP_CACHE
    bool checkAndDownloadFromCache(rtFileDownloadRequest* downloadRequest,rtHttpCacheData& cachedData);
#endif
    CURL* retrieveDownloadHandle();
    void releaseDownloadHandle(CURL* curlHandle, int expiresTime);
    static void addFileDownloadRequest(rtFileDownloadRequest* downloadRequest);
    static void clearFileDownloadRequest(rtFileDownloadRequest* downloadRequest);
    //todo: hash mPendingDownloadRequests;
    //todo: string list mPendingDownloadOrderList;
    //todo: list mActiveDownloads;
    unsigned int mNumberOfCurrentDownloads;
    //todo: hash m_priorityDownloads;
    void (*mDefaultCallbackFunction)(rtFileDownloadRequest*);
    std::vector<rtFileDownloadHandle> mDownloadHandles;
    bool mReuseDownloadHandles;
    rtString mCaCertFile;
    rtMutex mFileCacheMutex;
    static rtFileDownloader* mInstance;
    static std::vector<rtFileDownloadRequest*>* mDownloadRequestVector;
    static rtMutex* mDownloadRequestVectorMutex;
};

#endif //RT_FILE_DOWNLOADER_H
