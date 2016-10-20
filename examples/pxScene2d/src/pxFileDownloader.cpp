#define XRELOG_NOCTRACE
#include "pxFileDownloader.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"

#include <curl/curl.h>
#include <sstream>
#include <iostream>
#include <pxFileCache.h>

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


void startFileDownloadInBackground(void* data)
{
    pxFileDownloadRequest* downloadRequest = (pxFileDownloadRequest*)data;
    pxFileDownloader::getInstance()->downloadFile(downloadRequest);
}


pxFileDownloader* pxFileDownloader::mInstance = NULL;

pxFileDownloader::pxFileDownloader() 
    : mNumberOfCurrentDownloads(0), mDefaultCallbackFunction(NULL)
{
}

pxFileDownloader::~pxFileDownloader()
{
}

pxFileDownloader* pxFileDownloader::getInstance()
{
    if (mInstance == NULL)
    {
        mInstance = new pxFileDownloader();
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
    CURL *curl_handle = NULL;
    CURLcode res = CURLE_OK;
    
    bool useProxy = !downloadRequest->getProxy().isEmpty();
    rtString proxyServer = downloadRequest->getProxy();
    bool headerOnly = downloadRequest->getHeaderOnly();
    MemoryStruct chunk;

    curl_handle = curl_easy_init();

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

    struct curl_slist *list = NULL;
    vector<rtString>& additionalHttpHeaders = downloadRequest->getAdditionalHttpHeaders();
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
          }
        }
        delete downloadRequest;
        return;
    }

    long httpCode = -1;
    if (curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &httpCode) == CURLE_OK)
    {
        downloadRequest->setHttpStatusCode(httpCode);
    }
    curl_slist_free_all(list);
    curl_easy_cleanup(curl_handle);

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
    if (!downloadRequest->executeCallback(res))
    {
      if (mDefaultCallbackFunction != NULL)
      {
        (*mDefaultCallbackFunction)(downloadRequest);
      }
      else
      {
        //no listeners, delete request
      }
    }
    delete downloadRequest;
}

void pxFileDownloader::downloadFileInBackground(pxFileDownloadRequest* downloadRequest)
{
    rtThreadPool* mainThreadPool = rtThreadPool::globalInstance();
    
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
