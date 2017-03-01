#ifndef RT_FILE_DOWNLOADER_H
#define RT_FILE_DOWNLOADER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <curl/curl.h>
#include <vector>
#include "rtString.h"
#include "rtCore.h"
#ifdef ENABLE_HTTP_CACHE
#include <rtFileCache.h>
#endif

class rtFileDownloadRequest
{
public:
   rtFileDownloadRequest(const char* imageUrl, void* callbackData);
  ~rtFileDownloadRequest();

  void setFileUrl(const char* imageUrl);
  rtString fileUrl() const;
  void setProxy(const char* proxyServer);
  rtString proxy() const;
  void setErrorString(const char* errorString);
  rtString errorString();
  void setCallbackFunction(void (*callbackFunction)(rtFileDownloadRequest*));
  void setCallbackFunctionThreadSafe(void (*callbackFunction)(rtFileDownloadRequest*));
  long httpStatusCode();
  void setHttpStatusCode(long statusCode);
  bool executeCallback(int statusCode);
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
#endif

private:
  rtString mFileUrl;
  rtString mProxyServer;
  rtString mErrorString;
  long mHttpStatusCode;
  void (*mCallbackFunction)(rtFileDownloadRequest*);
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
#endif
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
    //todo: hash mPendingDownloadRequests;
    //todo: string list mPendingDownloadOrderList;
    //todo: list mActiveDownloads;
    unsigned int mNumberOfCurrentDownloads;
    //todo: hash m_priorityDownloads;
    void (*mDefaultCallbackFunction)(rtFileDownloadRequest*);
    std::vector<rtFileDownloadHandle> mDownloadHandles;
    bool mReuseDownloadHandles;
    
    static rtFileDownloader* mInstance;
};

#endif //RT_FILE_DOWNLOADER_H
