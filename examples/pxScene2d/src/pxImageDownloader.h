#ifndef PX_IMAGE_DOWNLOADER_H
#define PX_IMAGE_DOWNLOADER_H

#include <stdlib.h>
#include <string.h>

#include <string>

using namespace std;

class pxImageDownloadRequest
{
public:
    pxImageDownloadRequest(string imageUrl, void* callbackData) 
        : mImageUrl(imageUrl), mProxyServer(),
            mErrorString(), mHttpStatusCode(0), mCallbackFunction(NULL),
            mDownloadedData(0), mDownloadedDataSize(), mDecodeAfterDownload(false),
            mDownloadStatusCode(0), mCallbackData(callbackData)
        { } 
        
    ~pxImageDownloadRequest()
    {
        free(mDownloadedData);
        mDownloadedData = NULL;
    }

    void setImageUrl(string imageUrl)
        { mImageUrl = imageUrl; }

    string getImageUrl() const
        { return mImageUrl; }
    
    void setProxy(string proxyServer)
    {
        mProxyServer = proxyServer;
    }
    
    string getProxy() const
    {
        return mProxyServer;
    }
    
    void setErrorString(string errorString)
    {
        mErrorString = errorString;
    }
    
    string getErrorString()
    {
        return mErrorString;
    }
    
    void setCallbackFunction(void (*callbackFunction)(pxImageDownloadRequest*))
    {
        mCallbackFunction = callbackFunction;
    }
    
    long getHttpStatusCode()
    {
        return mHttpStatusCode;
    }
    
    void setHttpStatusCode(long statusCode)
    {
        mHttpStatusCode = statusCode;
    }
    
    void executeCallback(int statusCode)
    {
        mDownloadStatusCode = statusCode;
        if (mCallbackFunction != NULL)
        {
            (*mCallbackFunction)(this);
        }
    }
    
    void setDownloadedData(char* data, size_t size)
    {
        mDownloadedData = data;
        mDownloadedDataSize = size;
    }
    
    void getDownloadedData(char*& data, size_t& size)
    {
        data = mDownloadedData;
        size = mDownloadedDataSize; 
    }
    
    char* getDownloadedData()
    {
        return mDownloadedData;
    }
    
    size_t getDownloadedDataSize()
    {
        return mDownloadedDataSize;
    }
    
    void enableDecodeAfterDownload(bool enable)
    {
        mDecodeAfterDownload = enable;
    }
    
    bool shouldDecodeAfterDownload()
    {
        return mDecodeAfterDownload;
    }
    
    void setDownloadStatusCode(int statusCode)
    {
        mDownloadStatusCode = statusCode;
    }
    
    int getDownloadStatusCode()
    {
        return mDownloadStatusCode;
    }
    
    void* getCallbackData()
    {
        return mCallbackData;
    }
    
    void setCallbackData(void* callbackData)
    {
        mCallbackData = callbackData;
    }

private:
    string mImageUrl;
    string mProxyServer;
    string mErrorString;
    long mHttpStatusCode;
    void (*mCallbackFunction)(pxImageDownloadRequest*);
    char* mDownloadedData;
    size_t mDownloadedDataSize;
    bool mDecodeAfterDownload;
    int mDownloadStatusCode;
    void* mCallbackData;
};

class pxImageDownloader
{
public:

    static pxImageDownloader* getInstance();

    virtual bool addToDownloadQueue(pxImageDownloadRequest* downloadRequest);
    virtual void raiseDownloadPriority(pxImageDownloadRequest* downloadRequest);
    virtual void removeDownloadRequest(pxImageDownloadRequest* downloadRequest);

    void clearImageCache();
    void downloadImage(pxImageDownloadRequest* downloadRequest);

private:
    pxImageDownloader();
    ~pxImageDownloader();

    void startNextDownload(pxImageDownloadRequest* downloadRequest);
    pxImageDownloadRequest* getNextDownloadRequest();
    void startNextDownloadInBackground();
    void downloadImageInBackground(pxImageDownloadRequest* downloadRequest);
    
    //todo: hash mPendingDownloadRequests;
    //todo: string list mPendingDownloadOrderList;
    //todo: list mActiveDownloads;
    unsigned int mNumberOfCurrentDownloads;
    //todo: hash m_priorityDownloads;
    
    static pxImageDownloader* mInstance;
};

#endif //PX_IMAGE_DOWNLOADER_H