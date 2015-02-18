#define XRELOG_NOCTRACE
#include "pxImageDownloader.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"

#include <curl/curl.h>
#include <sstream>
#include <iostream>

using namespace std;

#define CA_CERTIFICATE "cacert.pem"

const int kCurlTimeoutInSeconds = 30;

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


void startImageDownloadInBackground(void* data)
{
    pxImageDownloadRequest* downloadRequest = (pxImageDownloadRequest*)data;
    pxImageDownloader::getInstance()->downloadImage(downloadRequest);
}


pxImageDownloader* pxImageDownloader::mInstance = NULL;

pxImageDownloader::pxImageDownloader() 
    : mNumberOfCurrentDownloads(0), mDefaultCallbackFunction(NULL)
{
}

pxImageDownloader::~pxImageDownloader()
{
}

pxImageDownloader* pxImageDownloader::getInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new pxImageDownloader();
    }
    return mInstance;
}

bool pxImageDownloader::addToDownloadQueue(pxImageDownloadRequest* downloadRequest)
{
    bool submitted = false;
    //todo: check the download queue before starting download
    submitted = true;
    downloadImageInBackground(downloadRequest);
    //startNextDownloadInBackground();
    return submitted;
}

void pxImageDownloader::startNextDownloadInBackground()
{
    //todo
}

void pxImageDownloader::raiseDownloadPriority(pxImageDownloadRequest* downloadRequest)
{
    (void)downloadRequest;
    //todo
}

void pxImageDownloader::removeDownloadRequest(pxImageDownloadRequest* downloadRequest)
{
    (void)downloadRequest;
    //todo
}

void pxImageDownloader::clearImageCache()
{
    //todo
}

void pxImageDownloader::downloadImage(pxImageDownloadRequest* downloadRequest)
{
    bool decode = downloadRequest->shouldDecodeAfterDownload();
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    
    bool useProxy = !downloadRequest->getProxy().isEmpty();
    rtString proxyServer = downloadRequest->getProxy();

    MemoryStruct chunk;

    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, downloadRequest->getImageURL().cString());
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1); //when redirected, follow the redirections
    curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, kCurlTimeoutInSeconds);
    curl_easy_setopt(curl_handle, CURLOPT_NOSIGNAL, 1);

    //CA certificates
    curl_easy_setopt(curl_handle,CURLOPT_CAINFO,CA_CERTIFICATE);
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

    /* get it! */
    res = curl_easy_perform(curl_handle);
    
    downloadRequest->setDownloadStatusCode(res);

    /* check for errors */
    if (res != CURLE_OK) 
    {
        stringstream errorStringStream;
        
        errorStringStream << "Download error for: " << downloadRequest->getImageURL().cString()
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

        curl_easy_cleanup(curl_handle);
        
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
        if (!downloadRequest->executeCallback(res))
        {
          if (mDefaultCallbackFunction != NULL)
          {
            (*mDefaultCallbackFunction)(downloadRequest);
          }
          else
          {
            //no listeners, delete request
            delete downloadRequest;
          }
        }
        return;
    }

    long httpCode = -1;
    if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
    {
        downloadRequest->setHttpStatusCode(httpCode);
    }

    curl_easy_cleanup(curl_handle);

    //todo read the header information before closing
    if (chunk.headerBuffer != NULL)
    {
        //only free up header buffer because content buffer will be needed for image
        free(chunk.headerBuffer);
        chunk.headerBuffer = NULL;
    }

    if (decode)
    {
        //todo - decode
    }

    //don't free the downloaded data (contentsBuffer) because it will be used later
    downloadRequest->setDownloadedData(chunk.contentsBuffer, chunk.contentsSize);
    if (!downloadRequest->executeCallback(res))
    {
      if (mDefaultCallbackFunction != NULL)
      {
        (*mDefaultCallbackFunction)(downloadRequest);
      }
      else
      {
        //no listeners, delete request
        delete downloadRequest;
      }
    }
}

void pxImageDownloader::downloadImageInBackground(pxImageDownloadRequest* downloadRequest)
{
    rtThreadPool* mainThreadPool = rtThreadPool::globalInstance();
    
    rtThreadTask* task = new rtThreadTask(startImageDownloadInBackground, (void*)downloadRequest);
    
    mainThreadPool->executeTask(task);
}

pxImageDownloadRequest* pxImageDownloader::getNextDownloadRequest()
{
    //todo
    return NULL;
}

void pxImageDownloader::setDefaultCallbackFunction(void (*callbackFunction)(pxImageDownloadRequest*))
{
  mDefaultCallbackFunction = callbackFunction;
}

