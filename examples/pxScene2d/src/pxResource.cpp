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

// pxResource.cpp
#include "pxScene2d.h"

#include "rtThreadQueue.h"
#include "pxContext.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include "rtRef.h"
#include "pxResource.h"
#include "pxUtil.h"
#include "rtThreadPool.h"
#include "rtPathUtils.h"


using namespace std;

extern rtThreadQueue* gUIThreadQueue;
extern pxContext context;

rtThreadPool textureCreateThreadPool(1);

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
  if(((rtPromise*)mReady.getPtr())->status() == false)
  {
    rtValue nullValue;
    mReady.send("reject",nullValue);
  }
  mName = "";
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
    /* need not pass archive here, as this flow go for network downloads */
    loadResource();
  }
  else if( !downloadRequestActive)
  {
    rtValue statusCode = getLoadStatus("statusCode");
    //rtLogDebug("download was not active for: %s code: %d", mUrl.cString(), statusCode.toInt32());
    if( statusCode.toInt32() == 0)
    {
      if( isInitialized())
        pListener->resourceReady("resolve");
      else
      {
        // TODO so this isn't just a copy/paste of code
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
  //mListeners.clear();
  mListenersMutex.unlock();

}

void pxResource::notifyListenersResourceDirty()
{
  mListenersMutex.lock();
  if( mListeners.size() == 0)
  {
    mListenersMutex.unlock();
    return;
  }
  for (list<pxResourceListener*>::iterator it = mListeners.begin();
       it != mListeners.end(); ++it)
  {
    (*it)->resourceDirty();

  }
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


rtImageResource::rtImageResource()
: pxResource(), mTexture(), mDownloadedTexture(), mTextureMutex(), mDownloadComplete(false), init_w(0), init_h(0), init_sx(0.0f), init_sy(0.0f), mData()
{
  // empty
}

rtImageResource::rtImageResource(const char* url, const char* proxy, int32_t iw /* = 0 */,  int32_t ih /* = 0 */,
                                                                       float sx /* = 1.0f*/,  float sy /* = 1.0f*/ )
    : pxResource(), mTexture(), mDownloadedTexture(), mTextureMutex(), mDownloadComplete(false),
      init_w(iw), init_h(ih), init_sx(sx), init_sy(sy), mData()
{
  setUrl(url, proxy);
}

rtImageResource::~rtImageResource()
{
  //rtLogDebug("destructor for rtImageResource for %s\n",mUrl.cString());
  //pxImageManager::removeImage( mUrl);
  if (mTexture.getPtr())
  {
    mTexture->setTextureListener(NULL);
  }
}

unsigned long rtImageResource::Release()
{

  long l = rtAtomicDec(&mRefCount);
  if (l == 0)
  {
    pxImageManager::removeImage( mName);
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

void rtImageResource::releaseData()
{
  if (mTexture.getPtr())
  {
    mTextureMutex.lock();
    if (mDownloadComplete)
    {
      mTexture->unloadTextureData();
    }
    mTextureMutex.unlock();
  }
  pxResource::releaseData();
}

void rtImageResource::reloadData()
{
  if (!mTexture.getPtr())
  {
    mTextureMutex.lock();
    if (mDownloadComplete)
    {
      mTexture->loadTextureData();
    }
    mTextureMutex.unlock();
  }
  pxResource::reloadData();
}

uint64_t rtImageResource::textureMemoryUsage()
{
  uint64_t textureMemory = 0;
  if (mTexture.getPtr() != NULL)
  {
    textureMemory = ((uint64_t)(mTexture->width()) * (uint64_t)(mTexture->height()) * (uint64_t)4);
  }
  return textureMemory;
}

void rtImageResource::textureReady()
{
  if (gUIThreadQueue)
  {
    AddRef();
    gUIThreadQueue->addTask(pxResource::onResourceDirtyUI, this, NULL);
  }
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

pxTextureRef rtImageResource::getTexture(bool initializing)
{
  if (!mTexture.getPtr() && (isInitialized() || initializing))
  {
    mTextureMutex.lock();
    if (mDownloadComplete)
    {
      mTexture = mDownloadedTexture;
      mDownloadedTexture = NULL;
      if (mTexture.getPtr())
      {
        mTexture->setTextureListener(this);
      }
    }
    mTextureMutex.unlock();
  }
  return mTexture;
}

void prepareImageResource(void* data)
{
  rtImageResource* imageResource = (rtImageResource*)data;
  imageResource->prepare();
}

void rtImageResource::prepare()
{
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
  static bool enableInternalContextOnce = true;
  if (enableInternalContextOnce)
  {
    context.enableInternalContext(true);
  }
  enableInternalContextOnce = false;
  mDownloadedTexture->prepareForRendering();
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
  mTextureMutex.lock();
  mDownloadComplete = true;
  mTextureMutex.unlock();
  setLoadStatus("statusCode", 0);
  // Since this object can be released before we get a async completion
  // We need to maintain this object's lifetime
  // TODO review overall flow and organization
  if (gUIThreadQueue)
  {
    gUIThreadQueue->addTask(pxResource::onDownloadCompleteUI, this, (void*)"resolve");
  }
}

void rtImageResource::setTextureData(pxOffscreen& imageOffscreen, const char* data, const size_t dataSize)
{
  mTextureMutex.lock();
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
  mDownloadedTexture = context.createTexture(imageOffscreen, data, dataSize);
  mTextureMutex.unlock();
  rtThreadTask* task = new rtThreadTask(prepareImageResource, (void*)this, "");
  textureCreateThreadPool.executeTask(task);
#else
  mDownloadedTexture = context.createTexture(imageOffscreen, data, dataSize);
  mDownloadComplete = true;
  mTextureMutex.unlock();
#endif //ENABLE_BACKGROUND_TEXTURE_CREATION
}

void rtImageResource::setupResource()
{
  getTexture(true);
  init();
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

void pxResource::releaseData()
{
}

void pxResource::reloadData()
{
}

uint64_t pxResource::textureMemoryUsage()
{
  return 0;
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
void pxResource::loadResource(rtObjectRef archive)
{
  if(((rtPromise*)mReady.getPtr())->status())
  {
    //create a new promise if the old one is complete
    mReady = new rtPromise();
  }
  setLoadStatus("statusCode", -1);
  pxArchive* arc = (pxArchive*)archive.getPtr();
  //rtLogDebug("rtImageResource::loadResource statusCode should be -1; is statusCode=%d\n",mLoadStatus.get<int32_t>("statusCode"));
  if (mUrl.beginsWith("http:") || mUrl.beginsWith("https:"))
  {
      setLoadStatus("sourceType", "http");
      mDownloadRequest = new rtFileDownloadRequest(mUrl, this, pxResource::onDownloadComplete);
      mDownloadRequest->setProxy(mProxy);
      mDownloadRequest->setCallbackFunctionThreadSafe(pxResource::onDownloadComplete);
#ifdef ENABLE_CORS_FOR_RESOURCES
      mDownloadRequest->setCORS(mCORS);
#endif
      mDownloadInProgressMutex.lock();
      mDownloadInProgress = true;
      mDownloadInProgressMutex.unlock();
      AddRef(); //ensure this object is not deleted while downloading
      rtFileDownloader::instance()->addToDownloadQueue(mDownloadRequest);
  }
  else if ((arc != NULL ) && (arc->isFile() == false))
  {
    loadResourceFromArchive(arc);
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

void pxResource::onResourceDirtyUI(void* context, void* /*data*/)
{
  pxResource* res = (pxResource*)context;

  res->notifyListenersResourceDirty();

  // Release here since we had to addRef when setting up callback to
  // this function
  res->Release();
}


void rtImageResource::loadResourceFromFile()
{
  pxOffscreen imageOffscreen;
  rtString status = "resolve";

  rtError loadImageSuccess = RT_FAIL;

  do
  {
    if (mData.length() != 0)
    {
      // We have BASE64 or SVG string already...
      loadImageSuccess = RT_OK;
      break;
    }

    loadImageSuccess = rtLoadFile(mUrl, mData);
    if (loadImageSuccess == RT_OK)
      break;

    if (rtIsPathAbsolute(mUrl))
      break;

    rtModuleDirs *dirs = rtModuleDirs::instance();

    for (rtModuleDirs::iter it = dirs->iterator(); it.first != it.second; it.first++)
    {
      if (rtLoadFile(rtConcatenatePath(*it.first, mUrl.cString()).c_str(), mData) == RT_OK)
      {
        loadImageSuccess = RT_OK;
        break;
      }
    }
  } while(0);

  if (loadImageSuccess == RT_OK)
  {
    loadImageSuccess = pxLoadImage((const char *) mData.data(), mData.length(), imageOffscreen,
                                      init_w, init_h, init_sx, init_sy);
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
    mTexture = context.createTexture(imageOffscreen, (const char *) mData.data(), mData.length());
    mTexture->setTextureListener(this);

    mData.term(); // Dump the source data...

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

  mTextureMutex.lock();
  mDownloadComplete = true;
  mTextureMutex.unlock();
}

void rtImageResource::loadResourceFromArchive(rtObjectRef archiveRef)
{
  pxArchive* archive = (pxArchive*)archiveRef.getPtr();
  pxOffscreen imageOffscreen;
  rtString status = "resolve";

  rtError loadImageSuccess = RT_FAIL;

  if(mData.length() == 0)
  {
    if ((NULL != archive) && (RT_OK == archive->getFileData(mUrl, mData)))
    {
      loadImageSuccess = RT_OK;
    }
    else
    {
      loadImageSuccess = RT_RESOURCE_NOT_FOUND;
      rtLogError("Could not load image file from archive %s.", mUrl.cString());
    }
  }
  else
  {
    // We have BASE64 or SVG string already...
    loadImageSuccess = RT_OK;
  }

  if (loadImageSuccess == RT_OK)
  {
    loadImageSuccess = pxLoadImage((const char *) mData.data(), mData.length(), imageOffscreen,
                                      init_w, init_h, init_sx, init_sy);
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
    mTexture = context.createTexture(imageOffscreen, (const char *) mData.data(), mData.length());
    mTexture->setTextureListener(this);

    mData.term(); // Dump the source data...

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

  mTextureMutex.lock();
  mDownloadComplete = true;
  mTextureMutex.unlock();
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

uint32_t rtImageResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      pxOffscreen imageOffscreen;
      if (pxLoadImage(fileDownloadRequest->downloadedData(),
                      fileDownloadRequest->downloadedDataSize(),
                      imageOffscreen, init_w, init_h, init_sx, init_sy) == RT_OK)
      {
        setTextureData(imageOffscreen, fileDownloadRequest->downloadedData(),
                                         fileDownloadRequest->downloadedDataSize());
#ifdef ENABLE_BACKGROUND_TEXTURE_CREATION
        return PX_RESOURCE_LOAD_WAIT;
#else
        return PX_RESOURCE_LOAD_SUCCESS;
#endif  //ENABLE_BACKGROUND_TEXTURE_CREATION
      }

      return PX_RESOURCE_LOAD_FAIL;
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
      int32_t result = loadResourceData(fileDownloadRequest);
      if(result == PX_RESOURCE_LOAD_FAIL)
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
      else if (result == PX_RESOURCE_LOAD_SUCCESS)
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
    pxImageManager::removeImageA( mName);
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

uint32_t rtImageAResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest->downloadStatusCode() == 0)
  {
    char* data;
    size_t dataSize;
    fileDownloadRequest->downloadedData(data, dataSize);

    if (pxLoadAImage(data, dataSize, mTimedOffscreenSequence) == RT_OK)
    {
      return PX_RESOURCE_LOAD_SUCCESS;
    }
  }
  return PX_RESOURCE_LOAD_FAIL;
}

void rtImageAResource::loadResourceFromFile()
{
  //TODO
  setLoadStatus("statusCode",PX_RESOURCE_STATUS_UNKNOWN_ERROR);
}

void rtImageAResource::loadResourceFromArchive(rtObjectRef archiveRef)
{
  UNUSED_PARAM(archiveRef);
  //TODO
  setLoadStatus("statusCode",PX_RESOURCE_STATUS_UNKNOWN_ERROR);
}

ImageMap pxImageManager::mImageMap;
rtRef<rtImageResource> pxImageManager::emptyUrlResource = 0;

rtRef<rtImageResource> pxImageManager::getImage(const char* url, const char* proxy    /* = NULL  */, const rtCORSRef& cors /* = NULL  */,
                                                int32_t iw /* = 0    */,   int32_t ih /* = 0     */,
                                                  float sx /* = 1.0f */,   float sy   /* = 1.0f  */, rtObjectRef archive)
{
  //rtLogDebug("pxImageManager::getImage\n");
  // Handle empty url
  if(!url || strlen(url) == 0)
  {
    if( !emptyUrlResource)
    {
      //rtLogDebug("Creating empty Url rtImageResource\n");
      emptyUrlResource = new rtImageResource(NULL, NULL, iw, ih, sx, sy);
      //rtLogDebug("Done creating empty Url rtImageResource\n");
    }
    //rtLogDebug("Returning empty Url rtImageResource\n");
    return emptyUrlResource;
  }

  rtString md5uri;
  rtString uri_string(url);

  int32_t index_of_comma = uri_string.find(0,','); // find the data.
  int32_t index_of_slash = uri_string.find(0,'/'); // find the data.

  rtString key = url;
  rtString svgUrl = url;

  // Correctly formed URI string ?
  if(index_of_comma >= 0 && 
     index_of_slash >= 0 && 
     uri_string.beginsWith("data:image/"))
  {
    rtString md5     = md5sum(uri_string);
    rtString imgType = uri_string.substring(index_of_slash, index_of_comma - index_of_slash);

    md5uri = "md5sum" + imgType + "," + md5;

    key = md5uri;
    svgUrl = md5uri;
  }

  if (false == ((key.beginsWith("http:")) || (key.beginsWith("https:"))))
  {
    // if running from archived app, we need to search the different url for relative paths
    // url format is <appname_resource path> eg: football.zip, images/ball.png <football.zip_images/ball.png>
    pxArchive* arc = (pxArchive*)archive.getPtr();
    if (NULL != arc)
    {
      if (false == arc->isFile())
      {
        key = arc->getName();
        key.append("_");
        if(uri_string.beginsWith("data:image/"))
        {
          key.append(svgUrl);
        }
        else
        {
          key.append(url);
        }
      }
    }
  }
  
  // For SVG  (and scaled PNG/JPG in the future) at a given SxSy SCALE ... append to key
  if(sx != 1.0 || sy != 1.0)
  {
    rtValue xx = sx;
    rtValue yy = sy;

    // Append scale factors
    key += "sx" + xx.toString() + "sy" + yy.toString();
    svgUrl += "sx" + xx.toString() + "sy" + yy.toString();
  }

  // For SVG  (and scaled PNG/JPG in the future) at a given WxH DIMENSIONS ... append to key
  if(iw > 0 || ih > 0)
  {
    rtValue ww = iw;
    rtValue hh = ih;

    // Append dimensions
    key +=  ww.toString() + "x" + hh.toString();
    svgUrl +=  ww.toString() + "x" + hh.toString();
  }

  rtRef<rtImageResource> pResImage;

  ImageMap::iterator it = mImageMap.find(key.cString());
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
    pResImage = new rtImageResource(url, proxy, iw, ih, sx, sy);
    pResImage->setCORS(cors);
    pResImage->setName(key);
    mImageMap.insert(make_pair(key.cString(), pResImage));

    // Is this a Data URI ?
    if(index_of_comma >= 0 && 
       index_of_slash >= 0)
    {
      // data: [<mediatype>][;base64],<data>
      //
      // data:image/png;base64,<data>
      // data:image/jpg;base64,<data>
      // data:image/svg,<data>
      //
      //
      rtData   data;
      rtString dataUri( (const char*) uri_string.cString() + index_of_comma + 1); // Skip ahead +1 ... "after commma"

      if(uri_string.beginsWith("data:image/svg,")) // SVG
      {
        pResImage->initUriData(dataUri);

        pResImage->setUrl(svgUrl); // DUMP the URL
      }
      else
      if(uri_string.beginsWith("data:image/")) // BASE64 PNG/JPG
      {
        if( base64_decode( dataUri, data ) == RT_OK)
        {
          pResImage->initUriData( data );
          pResImage->setUrl(svgUrl);         // DUMP the URL
        }
      }
    }//ENDIF - index_of_comma + index_of_slash

    pResImage->loadResource(archive);
  }

  return pResImage;
}

void pxImageManager::removeImage(rtString name)
{
  //rtLogDebug("pxImageManager::removeImage(\"%s\")\n",imageUrl.cString());
  ImageMap::iterator it = mImageMap.find(name.cString());
  if (it != mImageMap.end())
  {
    mImageMap.erase(it);
  }
}

ImageAMap pxImageManager::mImageAMap;
rtRef<rtImageAResource> pxImageManager::emptyUrlImageAResource = 0;
/** static pxImageManager::getImage */
rtRef<rtImageAResource> pxImageManager::getImageA(const char* url, const char* proxy, const rtCORSRef& cors, rtObjectRef archive)
{
  if(!url || strlen(url) == 0) {
    if( !emptyUrlImageAResource) {
      emptyUrlImageAResource = new rtImageAResource();
    }
    return emptyUrlImageAResource;
  }

  rtRef<rtImageAResource> pResImageA;
  rtString key = url;
  if (false == ((key.beginsWith("http:")) || (key.beginsWith("https:"))))
  {
    // if running from archived app, we need to search the different url for relative paths
    // url format is <appname_resource path> eg: football.zip, images/ball.png <football.zip_images/ball.png>
    pxArchive* arc = (pxArchive*)archive.getPtr();
    if (NULL != arc)
    {
      if (false == arc->isFile())
      {
        key = arc->getName();
        key.append("_");
        key.append(url);
      }
    }
  }

  ImageAMap::iterator it = mImageAMap.find(key);
  if (it != mImageAMap.end())
  {
    pResImageA = it->second;
  }
  else
  {
    pResImageA = new rtImageAResource(url, proxy);
    pResImageA->setCORS(cors);
    pResImageA->setName(key);
    pResImageA->loadResource(archive);
    mImageAMap.insert(make_pair(key, pResImageA));
  }

  return pResImageA;
}

void pxImageManager::removeImageA(rtString name)
{
  ImageAMap::iterator it = mImageAMap.find(name);
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
