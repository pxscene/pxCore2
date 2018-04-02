/*

 pxCore Copyright 2005-2017 John Robinson

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

// pxResource.cpp
#include "pxScene2d.h"

#include "rtThreadQueue.h"
#include "pxContext.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include "rtRef.h"
#include "pxResource.h"
#include "pxUtil.h"

using namespace std;

extern rtThreadQueue* gUIThreadQueue;
extern pxContext context;

pxResource::~pxResource() 
{
  //rtLogDebug("pxResource::~pxResource()\n");
  if (mDownloadRequest != NULL)
  {
    //rtLogInfo("pxResource::~pxResource(): mDownloadRequest not null\n");
    // if there is a previous request pending then set the callback to NULL
    // the previous request will not be processed and the memory will be freed when the download is complete
    rtFileDownloader::setCallbackFunctionThreadSafe(mDownloadRequest, NULL, this);
    mDownloadRequest = NULL;
  }

  if (gUIThreadQueue)
  {
    gUIThreadQueue->removeAllTasksForObject(this);
  }
  //mListeners.clear();
  //rtLogDebug("Leaving pxResource::~pxResource()\n");
}

rtError pxResource::setUrl(const char* url, const char* proxy)
{
  //rtLogDebug("pxResource::setUrl for url=\"%s\"\n",url);
  mUrl = url;
  mProxy = proxy;
  
  return RT_OK;
}

rtError pxResource::ready(rtObjectRef& r) const
{
  r = mReady;
  return RT_OK;
}

rtError pxResource::loadStatus(rtObjectRef& v) const
{
  mLoadStatusMutex.lock();
  v = mLoadStatus;
  mLoadStatusMutex.unlock();
  return RT_OK;
}

rtValue pxResource::getLoadStatus(rtString key)
{
  rtValue value;
  mLoadStatusMutex.lock();
  mLoadStatus.get(key, value);
  mLoadStatusMutex.unlock();
  return value;
}


void pxResource::addListener(pxResourceListener* pListener) 
{
  if( mUrl.isEmpty())
    return;

  bool downloadRequestActive = false;
  bool isDownloadCanceled = rtFileDownloader::isDownloadRequestCanceled(mDownloadRequest, this);

  mDownloadInProgressMutex.lock();
  downloadRequestActive = mDownloadInProgress;
  mDownloadInProgressMutex.unlock();


  if (isDownloadCanceled)
  {
    //if the download was canceled then download again
    //rtLogDebug("download was canceled then download again: %s", mUrl.cString());
    mListenersMutex.lock();
    bool found = false;
    for (list<pxResourceListener*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
    {
      if((*it) == pListener)
      {
        found = true;
        break;
      }
    }
    if (!found)
    {
      // only add unique listeners
      mListeners.push_back(pListener);
    }
    mListenersMutex.unlock();
    loadResource();
  }
  else if( !downloadRequestActive)
  {
    rtValue statusCode = getLoadStatus("statusCode");
    //rtLogDebug("download was not active for: %s code: %d", mUrl.cString(), statusCode.toInt32());
    if( statusCode.toInt32() == 0)
      pListener->resourceReady("resolve");
    else
      pListener->resourceReady("reject");    
  } 
  else 
  {
    mListenersMutex.lock();
    bool found = false;
    for (list<pxResourceListener*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
    {
      if((*it) == pListener)
      {
        found = true;
        break;
      }

    }
    if (!found)
    {
      // only add unique listeners
      mListeners.push_back(pListener);
    }

    mListenersMutex.unlock();
  }
  
}

void pxResource::removeListener(pxResourceListener* pListener)
{
  size_t numberOfListeners = 0;
  mListenersMutex.lock();
  for (list<pxResourceListener*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
  {
    if((*it) == pListener) 
    {
      // remove listener
      mListeners.erase(it);
      break;
    }

  }
  numberOfListeners = mListeners.size();
  mListenersMutex.unlock();
  if (numberOfListeners <= 0 && mDownloadRequest != NULL)
  {
    //rtLogDebug("canceling url: %s", mUrl.cString());
    rtFileDownloader::cancelDownloadRequestThreadSafe(mDownloadRequest, this);
    clearDownloadRequest();
  }
}

void pxResource::notifyListeners(rtString readyResolution)
{
  //rtLogDebug("notifyListeners for url=%s # of listeners=%d\n",mUrl.cString(),mListeners.size());
 
  mReady.send(readyResolution,this); 
  mListenersMutex.lock();
  if( mListeners.size() == 0)
  {
    mListenersMutex.unlock();
    return;
  }
  for (list<pxResourceListener*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
  {
    (*it)->resourceReady(readyResolution);

  }
  //rtLogDebug("notifyListeners for url=%s Ending\n");
  mListeners.clear();
  mListenersMutex.unlock();
  
}
void pxResource::raiseDownloadPriority()
{
  if (!priorityRaised && !mUrl.isEmpty() && mDownloadRequest != NULL)
  {
    rtLogWarn(">>>>>>>>>>>>>>>>>>>>>>>Inside pxResource::raiseDownloadPriority and download is in progress for %s\n",mUrl.cString());
    mListenersMutex.lock();
    size_t lisenersSize = mListeners.size();
    mListenersMutex.unlock();
    if( lisenersSize == 0) 
      rtLogInfo("But size is 0, so no one cares!!!!!\n");
    priorityRaised = true;
    rtFileDownloader::instance()->raiseDownloadPriority(mDownloadRequest);
  }
}
/**********************************************************************/
/**********************************************************************/
/** rtImageResource */
/**********************************************************************/
/**********************************************************************/

rtImageResource::rtImageResource(const char* url, const char* proxy) : pxResource(), mTexture(), mTextureMutex(), mImageOffscreen(),
                                                                       mCompressedData(NULL), mCompressedDataSize(0)
{
  setUrl(url, proxy);
}
rtImageResource::~rtImageResource() 
{
  //rtLogDebug("destructor for rtImageResource for %s\n",mUrl.cString());
  //pxImageManager::removeImage( mUrl);
  clearDownloadedData();
}
  
unsigned long rtImageResource::Release() 
{

  long l = rtAtomicDec(&mRefCount);
  if (l == 0) 
  {
    pxImageManager::removeImage( mUrl);      
    delete this;
    
  }
  return l;
}
void rtImageResource::init()
{
  //rtLogDebug("rtImageResource::init\n");
  if( mInitialized) 
    return; 
    
  mInitialized = true;

}

int32_t rtImageResource::w() const 
{ 
  //rtLogDebug("tImageResource::w()\n");
  if(mTexture.getPtr())  
    return mTexture->width(); 
  else 
    return 0;  
}
rtError rtImageResource::w(int32_t& v) const 
{ 
  //rtLogDebug("tImageResource::w(int32_t)\n");
  if(mTexture.getPtr()) 
    v = mTexture->width(); 
  else  
    v = 0;  
  return RT_OK; 
}
int32_t rtImageResource::h() const 
{ 
  //rtLogDebug("tImageResource::h()\n");
  if(mTexture.getPtr())
    return mTexture->height(); 
  else 
    return 0;
}
rtError rtImageResource::h(int32_t& v) const 
{ 
  //rtLogDebug("tImageResource::h(int32_t)\n");
  if(mTexture.getPtr())
    v = mTexture->height(); 
  else 
    v = 0;
  return RT_OK; 
}

pxTextureRef rtImageResource::getTexture()
{
  if (!mTexture.getPtr())
  {
    mTextureMutex.lock();
    if (mCompressedData != NULL)
    {
      mTexture = context.createTexture(mImageOffscreen, mCompressedData, mCompressedDataSize);
      delete [] mCompressedData;
      mCompressedData = NULL;
    }
    mTextureMutex.unlock();
  }
  return mTexture;
}

void rtImageResource::setTextureData(pxOffscreen& imageOffscreen, const char* data, const size_t dataSize)
{
  mTextureMutex.lock();
  mImageOffscreen = imageOffscreen;
  if (mCompressedData != NULL)
  {
    delete [] mCompressedData;
    mCompressedData = NULL;
  }
  if (data == NULL)
  {
    mCompressedData = NULL;
    mCompressedDataSize = 0;
  }
  else
  {
    mCompressedData = new char[dataSize];
    mCompressedDataSize = dataSize;
    memcpy(mCompressedData, data, mCompressedDataSize);
  }
  mTextureMutex.unlock();
}

void rtImageResource::clearDownloadedData()
{
  mTextureMutex.lock();
  if (mCompressedData != NULL)
  {
    delete [] mCompressedData;
    mCompressedData = NULL;
  }
  mCompressedDataSize = 0;
  mTextureMutex.unlock();
}

void rtImageResource::setupResource()
{
  getTexture();
}

void pxResource::clearDownloadRequest()
{
  mDownloadInProgressMutex.lock();
  mDownloadInProgress = false;
  mDownloadInProgressMutex.unlock();
}

void pxResource::setLoadStatus(const char* name, rtValue value)
{
  mLoadStatusMutex.lock();
  mLoadStatus.set(name, value);
  mLoadStatusMutex.unlock();
}

/** 
 * rtImageResource::loadResource()
 * 
 * This method will actually start the download of the image for mUrl.
 * 
 * ImageManager calls this method when the url is not already present
 * in the cache map.
 * 
 * */
void pxResource::loadResource()
{
  if(((rtPromise*)mReady.getPtr())->status())
  {
    //create a new promise if the old one is complete
    mReady = new rtPromise();
  }
  setLoadStatus("statusCode", -1);
  //rtLogDebug("rtImageResource::loadResource statusCode should be -1; is statusCode=%d\n",mLoadStatus.get<int32_t>("statusCode"));
  if (mUrl.beginsWith("http:") || mUrl.beginsWith("https:"))
  {
      setLoadStatus("sourceType", "http");
      mDownloadRequest = new rtFileDownloadRequest(mUrl, this, pxResource::onDownloadComplete);
      mDownloadRequest->setProxy(mProxy);
      mDownloadRequest->setCallbackFunctionThreadSafe(pxResource::onDownloadComplete);
      mDownloadInProgressMutex.lock();
      mDownloadInProgress = true;
      mDownloadInProgressMutex.unlock();
      AddRef(); //ensure this object is not deleted while downloading
      rtFileDownloader::instance()->addToDownloadQueue(mDownloadRequest);
  }
  else
  {
    loadResourceFromFile();
  }

}
void pxResource::onDownloadCompleteUI(void* context, void* data)
{
  pxResource* res = (rtImageResource*)context;
  rtString resolution = (char*)data;

  res->setupResource();
  res->notifyListeners(resolution);


  // Release here since we had to addRef when setting up callback to 
  // this function
  res->Release();
}

void pxResource::onDownloadCanceledUI(void* context, void* data)
{
  // no need to notify on canceled downloads
  (void)data;
  pxResource* res = (rtImageResource*)context;

  res->Release();
}

void rtImageResource::loadResourceFromFile()
{
  pxOffscreen imageOffscreen;
  rtString status = "resolve";
  rtData d;
  rtError loadImageSuccess = rtLoadFile(mUrl, d);
  if (loadImageSuccess == RT_OK)
  {
    loadImageSuccess = pxLoadImage((const char *) d.data(), d.length(), imageOffscreen);
  }
  else
  {
    loadImageSuccess = RT_RESOURCE_NOT_FOUND;
    rtLogError("Could not load image file %s.", mUrl.cString());
  }
  if ( loadImageSuccess != RT_OK)
  {
    rtLogWarn("image load failed"); // TODO: why?
    if (loadImageSuccess == RT_RESOURCE_NOT_FOUND)
    {
      setLoadStatus("statusCode",PX_RESOURCE_STATUS_FILE_NOT_FOUND);
    }
    else 
    {
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
    }

    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();

    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void*)"reject");
    }
    //mTexture->notifyListeners( mTexture, RT_FAIL, errorCode);

  }
  else
  {
    // create offscreen texture for local image
    mTexture = context.createTexture(imageOffscreen, (const char *) d.data(), d.length());
    setLoadStatus("statusCode",0);
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    if (gUIThreadQueue)
    {
      gUIThreadQueue->addTask(onDownloadCompleteUI, this, (void *) "resolve");
    }

  }

}


// Static callback that gets called when fileDownloadRequest completes 
void pxResource::onDownloadComplete(rtFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL && fileDownloadRequest->callbackData() != NULL) 
  {
    // Call virtual processDownlodedResource for specialized handling - 
    // Call directly rather than queuing
    ((pxResource*)fileDownloadRequest->callbackData())->processDownloadedResource(fileDownloadRequest);
    // Clear download data
    ((pxResource*)fileDownloadRequest->callbackData())->clearDownloadRequest();
  }
}

bool rtImageResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      pxOffscreen imageOffscreen;
      if (pxLoadImage(fileDownloadRequest->downloadedData(),
                      fileDownloadRequest->downloadedDataSize(),
                      imageOffscreen) == RT_OK)
      {
        setTextureData(imageOffscreen, fileDownloadRequest->downloadedData(),
                                         fileDownloadRequest->downloadedDataSize());
        return true;
      }
      
      return false;
}
/** pxResource processDownloadedResource */
void pxResource::processDownloadedResource(rtFileDownloadRequest* fileDownloadRequest)
{
  rtString val = "reject";
  if (fileDownloadRequest != NULL)
  {
    bool wasCanceled = fileDownloadRequest->isCanceled();
    if (wasCanceled)
    {
      //rtLogDebug("download was canceled, no need to notify: %s", fileDownloadRequest->fileUrl().cString());
      setLoadStatus("statusCode", 0);
      setLoadStatus("httpStatusCode",(uint32_t)fileDownloadRequest->httpStatusCode());
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(pxResource::onDownloadCanceledUI, this, (void*)"reject");
      }
    }
    else if (fileDownloadRequest->downloadStatusCode() == 0 &&
        fileDownloadRequest->httpStatusCode() == 200 &&
        fileDownloadRequest->downloadedData() != NULL)
    {
      if(!loadResourceData(fileDownloadRequest))
      {
        rtLogError("Resource Decode Failed: %s with proxy: %s", fileDownloadRequest->fileUrl().cString(), fileDownloadRequest->proxy().cString());
        setLoadStatus("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
        setLoadStatus("httpStatusCode", (uint32_t)fileDownloadRequest->httpStatusCode());
        // Since this object can be released before we get a async completion
        // We need to maintain this object's lifetime
        // TODO review overall flow and organization
        if (gUIThreadQueue)
        {
          gUIThreadQueue->addTask(pxResource::onDownloadCompleteUI, this, (void*)"reject");
        }
      }
      else
      {
        //rtLogInfo("File download Successful: %s", fileDownloadRequest->fileUrl().cString());
        // ToDo: Could context.createTexture ever fail and return null here?
       // mTexture = context.createTexture(imageOffscreen);
        setLoadStatus("statusCode", 0);
        val = "resolve";
        // Since this object can be released before we get a async completion
        // We need to maintain this object's lifetime
        // TODO review overall flow and organization
        if (gUIThreadQueue)
        {
          gUIThreadQueue->addTask(pxResource::onDownloadCompleteUI, this, (void*)"resolve");
        }
      }
    }
    else 
    {
      rtLogWarn("Resource Download Failed: %s Error: %s HTTP Status Code: %ld",
                fileDownloadRequest->fileUrl().cString(),
                fileDownloadRequest->errorString().cString(),
                fileDownloadRequest->httpStatusCode());
      setLoadStatus("statusCode", PX_RESOURCE_STATUS_HTTP_ERROR);
      setLoadStatus("httpStatusCode",(uint32_t)fileDownloadRequest->httpStatusCode());
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      if (gUIThreadQueue)
      {
        gUIThreadQueue->addTask(pxResource::onDownloadCompleteUI, this, (void*)"reject");
      }
    }
  }

}
/**
 * rtImageResource 
 */

rtImageAResource::rtImageAResource(const char* url, const char* proxy) : pxResource(), mTimedOffscreenSequence()
{
  mTimedOffscreenSequence.init();
  setUrl(url, proxy);
}

rtImageAResource::~rtImageAResource()
{
}

unsigned long rtImageAResource::Release()
{
  long l = rtAtomicDec(&mRefCount);
  if (l == 0)
  {
    pxImageManager::removeImageA( mUrl);
    delete this;

  }
  return l;
}

void rtImageAResource::init()
{
  if( mInitialized)
    return;

  mInitialized = true;
}

bool rtImageAResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest->downloadStatusCode() == 0)
  {
    char* data;
    size_t dataSize;
    fileDownloadRequest->downloadedData(data, dataSize);

    if (pxLoadAImage(data, dataSize, mTimedOffscreenSequence) == RT_OK)
    {
      return true;
    }
  }
  return false;
}

void rtImageAResource::loadResourceFromFile()
{
  //TODO
  setLoadStatus("statusCode",PX_RESOURCE_STATUS_UNKNOWN_ERROR);
}


ImageMap pxImageManager::mImageMap;
rtRef<rtImageResource> pxImageManager::emptyUrlResource = 0;
/** static pxImageManager::getImage */
rtRef<rtImageResource> pxImageManager::getImage(const char* url, const char* proxy)
{
  //rtLogDebug("pxImageManager::getImage\n");
  // Handle empty url
  if(!url || strlen(url) == 0) {
    if( !emptyUrlResource) {
      //rtLogDebug("Creating empty Url rtImageResource\n");
      emptyUrlResource = new rtImageResource;
      //rtLogDebug("Done creating empty Url rtImageResource\n");
    }
    //rtLogDebug("Returning empty Url rtImageResource\n");
    return emptyUrlResource;
  }
  
  rtRef<rtImageResource> pResImage;
  
  ImageMap::iterator it = mImageMap.find(url);
  if (it != mImageMap.end())
  {
    //rtLogInfo("Found rtImageResource in map for \"%s\"\n",url);
    //if( it->second->getRefCount() == 0)
      //rtLogDebug("ZERO REF COUNT IN GETIMAGE!\n");
    pResImage = it->second;
    //if(!pResImage->isInitialized()) {
      //pResImage->loadResource();
      //pResImage->init();
    //}
  }
  else 
  {
    //rtLogInfo("Create rtImageResource in map for \"%s\"\n",url);
    pResImage = new rtImageResource(url, proxy);
    mImageMap.insert(make_pair(url, pResImage));
    pResImage->loadResource();
    pResImage->init();
  }
  
  return pResImage;
}

void pxImageManager::removeImage(rtString imageUrl)
{
  //rtLogDebug("pxImageManager::removeImage(\"%s\")\n",imageUrl.cString());
  ImageMap::iterator it = mImageMap.find(imageUrl);
  if (it != mImageMap.end())
  {  
    mImageMap.erase(it);
  }
  //mImageMap.erase(imageUrl);
}

ImageAMap pxImageManager::mImageAMap;
rtRef<rtImageAResource> pxImageManager::emptyUrlImageAResource = 0;
/** static pxImageManager::getImage */
rtRef<rtImageAResource> pxImageManager::getImageA(const char* url, const char* proxy)
{
  if(!url || strlen(url) == 0) {
    if( !emptyUrlImageAResource) {
      emptyUrlImageAResource = new rtImageAResource();
    }
    return emptyUrlImageAResource;
  }

  rtRef<rtImageAResource> pResImageA;

  ImageAMap::iterator it = mImageAMap.find(url);
  if (it != mImageAMap.end())
  {
    pResImageA = it->second;
  }
  else
  {
    pResImageA = new rtImageAResource(url, proxy);
    mImageAMap.insert(make_pair(url, pResImageA));
    pResImageA->loadResource();
    pResImageA->init();
  }

  return pResImageA;
}

void pxImageManager::removeImageA(rtString imageUrl)
{
  ImageAMap::iterator it = mImageAMap.find(imageUrl);
  if (it != mImageAMap.end())
  {
    mImageAMap.erase(it);
  }
}

rtDefineObject(pxResource, rtObject);
rtDefineProperty(pxResource,url);
rtDefineProperty(pxResource,proxy);
rtDefineProperty(pxResource,ready);
rtDefineProperty(pxResource,loadStatus);

rtDefineObject(rtImageResource, pxResource);
rtDefineProperty(rtImageResource, w);
rtDefineProperty(rtImageResource, h);

rtDefineObject(rtImageAResource, pxResource);
