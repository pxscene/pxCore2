#define XRELOG_NOCTRACE
#include "pxFileDownloader.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "pxTimer.h"

#include <sstream>
#include <iostream>
#include <thread>

using namespace std;

#define CA_CERTIFICATE "cacert.pem"

//#define PX_REUSE_DOWNLOAD_HANDLES

const int kCurlTimeoutInSeconds = 30;
const int kMaxDownloadHandles = 6;
const unsigned int kDefaultDownloadHandleExpiresTime = 5 * 60;
const int kDownloadHandleTimerIntervalInMilliSeconds = 30 * 1000;

std::thread* downloadHandleExpiresCheckThread = NULL;
bool continueDownloadHandleCheck = true;
rtMutex downloadHandleMutex;

struct MemoryStruct {
    MemoryStruct()
        : headerSize(0)
        , headerBuffer()
        , contentsSize(0)
        , contentsBuffer()
    {
        headerBuffer = (char*)malloc(1);
        contentsBuffer = (char*)malloc(1);
    } 

  size_t headerSize;
  char* headerBuffer;
  size_t contentsSize;
  char* contentsBuffer;
};

static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
  
  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    /* out of memory! */ 
    cout << "out of memory when downloading image\n";
    return 0;
  }
 
  memcpy(&(mem->headerBuffer[mem->headerSize]), contents, downloadSize);
  mem->headerSize += downloadSize;
  mem->headerBuffer[mem->headerSize] = 0;
  
  return downloadSize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->contentsBuffer = (char*)realloc(mem->contentsBuffer, mem->contentsSize + downloadSize + 1);
  if(mem->contentsBuffer == NULL) {
    /* out of memory! */ 
    cout << "out of memory when downloading image\n";
    return 0;
  }
 
  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;
  
  return downloadSize;
}


void startFileDownloadInBackground(void* data)
{
    pxFileDownloadRequest* downloadRequest = (pxFileDownloadRequest*)data;
    pxFileDownloader::getInstance()->downloadFile(downloadRequest);
}

pxFileDownloader* pxFileDownloader::mInstance = NULL;


void onDownloadHandleCheck()
{
  rtLogDebug("inside onDownloadHandleCheck");
  bool checkHandles = true;
  while (checkHandles)
  {
    usleep(kDownloadHandleTimerIntervalInMilliSeconds * 1000);
    pxFileDownloader::getInstance()->checkForExpiredHandles();
    downloadHandleMutex.lock();
    checkHandles = continueDownloadHandleCheck;
    downloadHandleMutex.unlock();
  }
}


pxFileDownloader::pxFileDownloader() 
    : mNumberOfCurrentDownloads(0), mDefaultCallbackFunction(NULL), mDownloadHandles(), mReuseDownloadHandles(false)
{
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  rtLogWarn("enabling curl handle reuse");
  for (int i = 0; i < kMaxDownloadHandles; i++)
  {
    mDownloadHandles.push_back(pxFileDownloadHandle(curl_easy_init()));
  }
  mReuseDownloadHandles = true;
#endif
}

pxFileDownloader::~pxFileDownloader()
{
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  downloadHandleMutex.lock();
  for (vector<pxFileDownloadHandle>::iterator it = mDownloadHandles.begin(); it != mDownloadHandles.end();++it)
  {
    CURL* curlHandle = (*it).curlHandle;
    if (curlHandle != NULL)
    {
      curl_easy_cleanup(curlHandle);
    }
    it = mDownloadHandles.erase(it);
  }
  mReuseDownloadHandles = false;
  downloadHandleMutex.unlock();
  if (pxFileDownloader::getInstance() == this)
  {
    //cleanup curl and shutdown the reuse handle thread if this is the singleton object
    downloadHandleMutex.lock();
    continueDownloadHandleCheck = false;
    downloadHandleMutex.unlock();
    if (downloadHandleExpiresCheckThread)
    {
      rtLogDebug("close thread and wait");
      downloadHandleExpiresCheckThread->join();
      rtLogDebug("done with join");
      delete downloadHandleExpiresCheckThread;
      downloadHandleExpiresCheckThread = NULL;
    }
  }
#endif
}

pxFileDownloader* pxFileDownloader::getInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new pxFileDownloader();
#ifdef PX_REUSE_DOWNLOAD_HANDLES
        downloadHandleExpiresCheckThread = new std::thread(onDownloadHandleCheck);
#endif //PX_REUSE_DOWNLOAD_HANDLES
    }
    return mInstance;
}

bool pxFileDownloader::addToDownloadQueue(pxFileDownloadRequest* downloadRequest)
{
    bool submitted = false;
    //todo: check the download queue before starting download
    submitted = true;
    downloadFileInBackground(downloadRequest);
    //startNextDownloadInBackground();
    return submitted;
}

void pxFileDownloader::startNextDownloadInBackground()
{
    //todo
}

void pxFileDownloader::raiseDownloadPriority(pxFileDownloadRequest* downloadRequest)
{
  if (downloadRequest != NULL)
  {
    rtThreadPool *mainThreadPool = rtThreadPool::globalInstance();
    mainThreadPool->raisePriority(downloadRequest->getFileUrl());
  }
}

void pxFileDownloader::removeDownloadRequest(pxFileDownloadRequest* downloadRequest)
{
    (void)downloadRequest;
    //todo
}

void pxFileDownloader::clearFileCache()
{
    //todo
}

void pxFileDownloader::downloadFile(pxFileDownloadRequest* downloadRequest)
{
#ifdef ENABLE_HTTP_CACHE
    bool isDataInCache = false;
#endif
    bool nwDownloadSuccess = false;

#ifdef ENABLE_HTTP_CACHE
    rtHttpCacheData cachedData(downloadRequest->getFileUrl().cString());
    if (true == downloadRequest->getCacheEnabled())
    {
      if (true == checkAndDownloadFromCache(downloadRequest,cachedData))
      {
        isDataInCache = true;
      }
    }

    if (false == isDataInCache)
#endif
    {
      nwDownloadSuccess = downloadFromNetwork(downloadRequest);
    }

    if (!downloadRequest->executeCallback(downloadRequest->getDownloadStatusCode()))
    {
      if (mDefaultCallbackFunction != NULL)
      {
        (*mDefaultCallbackFunction)(downloadRequest);
      }
    }

#ifdef ENABLE_HTTP_CACHE
    // Store the network data in cache
    if ((true == nwDownloadSuccess) && (true == downloadRequest->getCacheEnabled()) && (downloadRequest->getHttpStatusCode() != 206) && (downloadRequest->getHttpStatusCode() != 302) && (downloadRequest->getHttpStatusCode() != 307))
    {
      rtHttpCacheData downloadedData(downloadRequest->getFileUrl(),downloadRequest->getHeaderData(),downloadRequest->getDownloadedData(),downloadRequest->getDownloadedDataSize());
      if (downloadedData.isWritableToCache())
      {
        if (NULL == rtFileCache::getInstance())
          rtLogWarn("cache data not added");
        else
          rtFileCache::getInstance()->addToCache(downloadedData);
      }
    }

    // Store the updated data in cache
    if ((true == isDataInCache) && (cachedData.isUpdated()))
    {
      rtString url;
      cachedData.url(url);

      if (NULL == rtFileCache::getInstance())
          rtLogWarn("Adding url to cache failed (%s) due to in-process memory issues", url.cString());
      rtFileCache::getInstance()->removeData(url);
      if (cachedData.isWritableToCache())
      {
        rtError err = rtFileCache::getInstance()->addToCache(cachedData);
        if (RT_OK != err)
          rtLogWarn("Adding url to cache failed (%s)", url.cString());
      }
    }

    if (true == isDataInCache)
    {
      downloadRequest->setHeaderData(NULL,0);
      downloadRequest->setDownloadedData(NULL,0);
    }
#endif

    delete downloadRequest;
}

bool pxFileDownloader::downloadFromNetwork(pxFileDownloadRequest* downloadRequest)
{
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    
    bool useProxy = !downloadRequest->getProxy().isEmpty();
    rtString proxyServer = downloadRequest->getProxy();
    bool headerOnly = downloadRequest->getHeaderOnly();
    MemoryStruct chunk;

    curl_handle = pxFileDownloader::getInstance()->getDownloadHandle();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, downloadRequest->getFileUrl().cString());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); //when redirected, follow the redirections
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
    if (false == headerOnly)
    {
      curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
      curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    }
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, kCurlTimeoutInSeconds);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPIDLE, 60);
    curl_easy_setopt(curl_handle, CURLOPT_TCP_KEEPINTVL, 30);
    
    int downloadHandleExpiresTime = downloadRequest->downloadHandleExpiresTime();
    
    vector<rtString>& additionalHttpHeaders = downloadRequest->getAdditionalHttpHeaders();
    struct curl_slist *list = NULL;
    for (unsigned int headerOption = 0;headerOption < additionalHttpHeaders.size();headerOption++)
    {
      list = curl_slist_append(list, additionalHttpHeaders[headerOption].cString());
    }
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, list);
    //CA certificates
    // !CLF: Use system CA Cert rather than CA_CERTIFICATE fo now.  Revisit!
   // curl_easy_setopt(curl_handle,CURLOPT_CAINFO,CA_CERTIFICATE);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 2);
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, true);

    /* some servers don't like requests that are made without a user-agent
     field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    if (useProxy)

    {
        curl_easy_setopt(curl_handle, CURLOPT_PROXY, proxyServer.cString());
        curl_easy_setopt(curl_handle, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    }

    if (true == headerOnly)
    {
      curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);
    }
    /* get it! */
    res = curl_easy_perform(curl_handle);
    downloadRequest->setDownloadStatusCode(res);

    /* check for errors */
    if (res != CURLE_OK) 
    {
        stringstream errorStringStream;
        
        errorStringStream << "Download error for: " << downloadRequest->getFileUrl().cString()
                << ".  Error code : " << res << ".  Using proxy: ";
        if (useProxy)
        {
            errorStringStream << "true - " << proxyServer.cString();
        }
        else
        {
            errorStringStream << "false";
        }
        
        downloadRequest->setErrorString(errorStringStream.str().c_str());
        pxFileDownloader::getInstance()->releaseDownloadHandle(curl_handle, downloadHandleExpiresTime);
        
        //clean up contents on error
        if (chunk.contentsBuffer != NULL)
        {
            free(chunk.contentsBuffer);
            chunk.contentsBuffer = NULL;
        }
        
        if (chunk.headerBuffer != NULL)
        {
            free(chunk.headerBuffer);
            chunk.headerBuffer = NULL;
        }
        downloadRequest->setDownloadedData(NULL, 0);
        return false;
    }

    long httpCode = -1;
    if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
    {
        downloadRequest->setHttpStatusCode(httpCode);
    }
    curl_slist_free_all(list);
    pxFileDownloader::getInstance()->releaseDownloadHandle(curl_handle, downloadHandleExpiresTime);

    //todo read the header information before closing
    if (chunk.headerBuffer != NULL)
    {
        downloadRequest->setHeaderData(chunk.headerBuffer, chunk.headerSize);
    }

    //don't free the downloaded data (contentsBuffer) because it will be used later
    if (false == headerOnly)
    {
      downloadRequest->setDownloadedData(chunk.contentsBuffer, chunk.contentsSize);
    }
    return true;
}

#ifdef ENABLE_HTTP_CACHE
bool pxFileDownloader::checkAndDownloadFromCache(pxFileDownloadRequest* downloadRequest,rtHttpCacheData& cachedData)
{
  rtError err;
  rtData data;
  if ((NULL != rtFileCache::getInstance()) && (RT_OK == rtFileCache::getInstance()->getHttpCacheData(downloadRequest->getFileUrl(),cachedData)))
  {
    err = cachedData.data(data);
    if (RT_OK !=  err)
    {
      return false;
    }

    downloadRequest->setHeaderData((char *)cachedData.getHeaderData().data(),cachedData.getHeaderData().length());
    downloadRequest->setDownloadedData((char *)cachedData.getContentsData().data(),cachedData.getContentsData().length());
    downloadRequest->setDownloadStatusCode(0);
    downloadRequest->setHttpStatusCode(200);
    return true;
  }
  return false;
}
#endif

void pxFileDownloader::downloadFileInBackground(pxFileDownloadRequest* downloadRequest)
{
    rtThreadPool* mainThreadPool = rtThreadPool::globalInstance();

    if (downloadRequest->downloadHandleExpiresTime() < -1)
    {
      downloadRequest->setDownloadHandleExpiresTime(kDefaultDownloadHandleExpiresTime);
    }
    
    rtThreadTask* task = new rtThreadTask(startFileDownloadInBackground, (void*)downloadRequest, downloadRequest->getFileUrl());
    
    mainThreadPool->executeTask(task);
}

pxFileDownloadRequest* pxFileDownloader::getNextDownloadRequest()
{
    //todo
    return NULL;
}

void pxFileDownloader::setDefaultCallbackFunction(void (*callbackFunction)(pxFileDownloadRequest*))
{
  mDefaultCallbackFunction = callbackFunction;
}

CURL* pxFileDownloader::getDownloadHandle()
{
  CURL* curlHandle = NULL;
#ifdef PX_REUSE_DOWNLOAD_HANDLES
  downloadHandleMutex.lock();
  if (!mReuseDownloadHandles || mDownloadHandles.empty())
  {
    curlHandle = curl_easy_init();
  }
  else
  {
    curlHandle = mDownloadHandles.back().curlHandle;
    mDownloadHandles.pop_back();
  }
  downloadHandleMutex.unlock();
#else
  curlHandle = curl_easy_init();
#endif //PX_REUSE_DOWNLOAD_HANDLES
  if (curlHandle == NULL)
  {
    curlHandle = curl_easy_init();
  }
  return curlHandle;
}

void pxFileDownloader::releaseDownloadHandle(CURL* curlHandle, int expiresTime)
{
  rtLogDebug("expires time: %d", expiresTime);
#ifdef PX_REUSE_DOWNLOAD_HANDLES
    downloadHandleMutex.lock();
    if(!mReuseDownloadHandles || mDownloadHandles.size() >= kMaxDownloadHandles || (expiresTime == 0))
    {
      curl_easy_cleanup(curlHandle);
    }
    else
    {
        if (expiresTime > 0)
        {
          expiresTime += (int)pxSeconds();
        }
    	mDownloadHandles.push_back(pxFileDownloadHandle(curlHandle, expiresTime));
    }
    downloadHandleMutex.unlock();
#else
    curl_easy_cleanup(curlHandle);
#endif //PX_REUSE_DOWNLOAD_HANDLES
}

void pxFileDownloader::checkForExpiredHandles()
{
  rtLogDebug("inside checkForExpiredHandles");
  downloadHandleMutex.lock();
  for (vector<pxFileDownloadHandle>::iterator it = mDownloadHandles.begin(); it != mDownloadHandles.end();)
  {
    pxFileDownloadHandle fileDownloadHandle = (*it);
    rtLogDebug("expires time: %d\n", fileDownloadHandle.expiresTime);
    if (fileDownloadHandle.expiresTime < 0)
    {
      ++it;
      continue;
    }
    else if (pxSeconds() > fileDownloadHandle.expiresTime)
    {
      rtLogDebug("erasing handle!!!\n");
      curl_easy_cleanup(fileDownloadHandle.curlHandle);
      it = mDownloadHandles.erase(it);
    }
    else
    {
      ++it;
    }
  }
  downloadHandleMutex.unlock();
}

