// pxCore CopyRight 2007-2015 John Robinson
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

extern rtThreadQueue gUIThreadQueue;
extern pxContext context;

pxResource::~pxResource() 
{
  //rtLogDebug("pxResource::~pxResource()\n");
  if (mDownloadRequest != NULL)
  {
    rtLogInfo("pxResource::~pxResource(): mDownloadRequest not null\n");
    // if there is a previous request pending then set the callback to NULL
    // the previous request will not be processed and the memory will be freed when the download is complete
    mDownloadRequest->setCallbackFunctionThreadSafe(NULL);
    mDownloadRequest = 0;
  }
  gUIThreadQueue.removeAllTasksForObject(this);
  //mListeners.clear();
  //rtLogDebug("Leaving pxResource::~pxResource()\n");
}

rtError pxResource::setUrl(const char* url)
{
  //rtLogDebug("pxResource::setUrl for url=\"%s\"\n",url);
  mUrl = url;
  
  return RT_OK;
}

rtError pxResource::ready(rtObjectRef& r) const
{
  r = mReady;
  return RT_OK;
}

rtError pxResource::loadStatus(rtObjectRef& v) const
{
  v = mLoadStatus;
  return RT_OK;
}

rtValue pxResource::getLoadStatus(rtString key)
{
  rtValue value;
  mLoadStatus.get(key, value);
  return value;
}


void pxResource::addListener(pxResourceListener* pListener) 
{
  if( mUrl.isEmpty())
    return;
    
  if( !mDownloadRequest)
  {
    if( mLoadStatus.get<int32_t>("statusCode") == 0)
      pListener->resourceReady("resolve");
    else
      pListener->resourceReady("reject");    
  } 
  else 
  {
    mListenersMutex.lock();
    mListeners.push_back(pListener);
    mListenersMutex.unlock();
  }
  
}

void pxResource::removeListener(pxResourceListener* pListener)
{
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
  // If no listeners are left and a download is still in progress,
  // let's reduce the download priority.
  if( mListeners.size() == 0 && mDownloadRequest != NULL )
  {
    mInitialized = false;
    mDownloadRequest->setCallbackFunctionThreadSafe(NULL);
    mDownloadRequest = 0;    
  }
  mListenersMutex.unlock();
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
    int lisenersSize = mListeners.size();
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

rtImageResource::rtImageResource(const char* url)
{
  setUrl(url);
}
rtImageResource::~rtImageResource() 
{
  //rtLogDebug("destructor for rtImageResource for %s\n",mUrl.cString());
  //pxImageManager::removeImage( mUrl);
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
  mLoadStatus.set("statusCode", -1);
  //rtLogDebug("rtImageResource::loadResource statusCode should be -1; is statusCode=%d\n",mLoadStatus.get<int32_t>("statusCode"));
  if (mUrl.beginsWith("http:") || mUrl.beginsWith("https:"))
  {
      mLoadStatus.set("sourceType", "http");
      mDownloadRequest = new rtFileDownloadRequest(mUrl, this);
      // setup for asynchronous load and callback
      mDownloadRequest->setCallbackFunction(pxResource::onDownloadComplete);
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
  
  res->notifyListeners(resolution);
  res->mReady.send(resolution, res);

  // Release here since we had to addRef when setting up callback to 
  // this function
  res->Release();
}
void rtImageResource::loadResourceFromFile()
{
  pxOffscreen imageOffscreen;
  rtString status = "resolve";
  rtError loadImageSuccess = pxLoadImage(mUrl, imageOffscreen);
  if ( loadImageSuccess != RT_OK)
  {
    rtLogWarn("image load failed"); // TODO: why?
    if (loadImageSuccess == RT_RESOURCE_NOT_FOUND)
    {
      mLoadStatus.set("statusCode",PX_RESOURCE_STATUS_FILE_NOT_FOUND);
    }
    else 
    {
      mLoadStatus.set("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
    }

    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    gUIThreadQueue.addTask(onDownloadCompleteUI, this, (void*)"reject");
    //mTexture->notifyListeners( mTexture, RT_FAIL, errorCode);

  }
  else
  {
    // create offscreen texture for local image
    mTexture = context.createTexture(imageOffscreen);
    mLoadStatus.set("statusCode",0);
    // Since this object can be released before we get a async completion
    // We need to maintain this object's lifetime
    // TODO review overall flow and organization
    AddRef();
    gUIThreadQueue.addTask(onDownloadCompleteUI, this, (void *) "resolve");

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
    ((pxResource*)fileDownloadRequest->callbackData())->mDownloadRequest = 0;
  }
}

bool rtImageResource::loadResourceData(rtFileDownloadRequest* fileDownloadRequest)
{
      pxOffscreen imageOffscreen;
      if (pxLoadImage(fileDownloadRequest->downloadedData(),
                      fileDownloadRequest->downloadedDataSize(),
                      imageOffscreen) == RT_OK)
      {
        mTexture = context.createTexture(imageOffscreen);
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
    if (fileDownloadRequest->downloadStatusCode() == 0 &&
        fileDownloadRequest->httpStatusCode() == 200 &&
        fileDownloadRequest->downloadedData() != NULL)
    {
      if(!loadResourceData(fileDownloadRequest))
      {
        rtLogError("Resource Decode Failed: %s", fileDownloadRequest->fileUrl().cString());
        mLoadStatus.set("statusCode", PX_RESOURCE_STATUS_DECODE_FAILURE);
        mLoadStatus.set("httpStatusCode", (uint32_t)fileDownloadRequest->httpStatusCode());
        // Since this object can be released before we get a async completion
        // We need to maintain this object's lifetime
        // TODO review overall flow and organization
        AddRef();        
        gUIThreadQueue.addTask(pxResource::onDownloadCompleteUI, this, (void*)"reject");        
      }
      else
      {
        //rtLogInfo("Image Decode Successful: %s", fileDownloadRequest->getFileUrl().cString());
        // ToDo: Could context.createTexture ever fail and return null here?
       // mTexture = context.createTexture(imageOffscreen);
        mLoadStatus.set("statusCode", 0);
        val = "resolve";
        // Since this object can be released before we get a async completion
        // We need to maintain this object's lifetime
        // TODO review overall flow and organization
        AddRef();
        gUIThreadQueue.addTask(pxResource::onDownloadCompleteUI, this, (void*)"resolve");
      }
    }
    else 
    {
      rtLogWarn("Resource Download Failed: %s Error: %s HTTP Status Code: %ld",
                fileDownloadRequest->fileUrl().cString(),
                fileDownloadRequest->errorString().cString(),
                fileDownloadRequest->httpStatusCode());
      mLoadStatus.set("statusCode", PX_RESOURCE_STATUS_HTTP_ERROR);
      mLoadStatus.set("httpStatusCode",(uint32_t)fileDownloadRequest->httpStatusCode());
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();        
      gUIThreadQueue.addTask(pxResource::onDownloadCompleteUI, this, (void*)"reject");      
    }
  }

}
/**
 * rtImageResource 
 */
ImageMap pxImageManager::mImageMap;
rtRef<rtImageResource> pxImageManager::emptyUrlResource = 0;
/** static pxImageManager::getImage */
rtRef<rtImageResource> pxImageManager::getImage(const char* url)
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
    pResImage = new rtImageResource(url);
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

rtDefineObject(pxResource, rtObject);
rtDefineProperty(pxResource,url);
rtDefineProperty(pxResource,ready);
rtDefineProperty(pxResource,loadStatus);

rtDefineObject(rtImageResource, pxResource);
rtDefineProperty(rtImageResource, w);
rtDefineProperty(rtImageResource, h); 
