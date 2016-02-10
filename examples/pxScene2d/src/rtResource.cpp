// pxCore CopyRight 2007-2015 John Robinson
// rtResource.cpp
#include "pxScene2d.h"

#include "rtThreadQueue.h"
#include "pxContext.h"
#include "pxFileDownloader.h"
#include "rtString.h"
#include "rtRefT.h"
#include "rtResource.h"
#include "pxUtil.h"

extern rtThreadQueue gUIThreadQueue;
extern pxContext context;



rtResource::~rtResource() 
{
  printf("rtResource::~rtResource()\n");
  if (mDownloadRequest != NULL)
  {
    printf("rtResource::~rtResource(): mDownloadRequest not null\n");
    // if there is a previous request pending then set the callback to NULL
    // the previous request will not be processed and the memory will be freed when the download is complete
    mDownloadRequest->setCallbackFunctionThreadSafe(NULL);
    mDownloadRequest = 0;
  }
  printf("Leaving rtResource::~rtResource()\n");
}

rtError rtResource::setUrl(const char* url)
{
  //printf("rtResource::setUrl for url=\"%s\"\n",url);
  mUrl = url;
  
  return RT_OK;
}

rtError rtResource::ready(rtObjectRef& r) const
{
  r = mReady;
  return RT_OK;
}

rtError rtResource::loadStatus(rtObjectRef& v) const
{
  v = mLoadStatus;
  return RT_OK;
}

rtValue rtResource::getLoadStatus(rtString key)
{
  rtValue value;
  mLoadStatus.get(key, value);
  return value;
}


void rtResource::addListener(rtResourceListener* pListener) 
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
    mListeners.push_back(pListener);
  }
  
}

void rtResource::removeListener(rtResourceListener* /*pListener*/)
{
  /*
  for (list<rtResourceListener*>::iterator it = mListeners.begin();
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
  */
}

void rtResource::notifyListeners(rtString readyResolution)
{
  //printf("notifyListeners for url=%s # of listeners=%d\n",mUrl.cString(),mListeners.size());
 
  mReady.send(readyResolution,this); 
  if( mListeners.size() == 0)
    return;
  for (list<rtResourceListener*>::iterator it = mListeners.begin();
         it != mListeners.end(); ++it)
  {
    (*it)->resourceReady(readyResolution);

  }
  //printf("notifyListeners for url=%s Ending\n");
  mListeners.clear();
  
}
void rtResource::raiseDownloadPriority()
{
  if (!priorityRaised && !mUrl.isEmpty() && mDownloadRequest != NULL)
  {
    printf(">>>>>>>>>>>>>>>>>>>>>>>Inside rtResource::raiseDownloadPriority and download is in progress for %s\n",mUrl.cString());
    if( mListeners.size() == 0) 
      printf("But size is 0, so no one cares!!!!!\n");
    priorityRaised = true;
    pxFileDownloader::getInstance()->raiseDownloadPriority(mDownloadRequest);
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
  printf("destructor for rtImageResource for %s\n",mUrl.cString());
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
  //printf("rtImageResource::init\n");
  if( mInitialized) 
    return; 
    
  mInitialized = true;

}

int32_t rtImageResource::w() const 
{ 
  //printf("tImageResource::w()\n");
  if(mTexture.getPtr())  
    return mTexture->width(); 
  else 
    return 0;  
}
rtError rtImageResource::w(int32_t& v) const 
{ 
  //printf("tImageResource::w(int32_t)\n");
  if(mTexture.getPtr()) 
    v = mTexture->width(); 
  else  
    v = 0;  
  return RT_OK; 
}
int32_t rtImageResource::h() const 
{ 
  //printf("tImageResource::h()\n");
  if(mTexture.getPtr())
    return mTexture->height(); 
  else 
    return 0;
}
rtError rtImageResource::h(int32_t& v) const 
{ 
  //printf("tImageResource::h(int32_t)\n");
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
void rtResource::loadResource()
{
  mLoadStatus.set("statusCode", -1);
  //printf("rtImageResource::loadResource statusCode should be -1; is statusCode=%d\n",mLoadStatus.get<int32_t>("statusCode"));
  if (mUrl.beginsWith("http:") || mUrl.beginsWith("https:"))
  {
    mLoadStatus.set("sourceType", "http");
    mDownloadRequest = new pxFileDownloadRequest(mUrl, this);
    // setup for asynchronous load and callback
    mDownloadRequest->setCallbackFunction(rtResource::onDownloadComplete);
    pxFileDownloader::getInstance()->addToDownloadQueue(mDownloadRequest);
  }
  else
  {
    loadResourceFromFile();
  }

}
void rtResource::onDownloadCompleteUI(void* context, void* data)
{
  rtResource* res = (rtImageResource*)context;
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
      mLoadStatus.set("statusCode",RT_RESOURCE_STATUS_FILE_NOT_FOUND);
    }
    else 
    {
      mLoadStatus.set("statusCode", RT_RESOURCE_STATUS_DECODE_FAILURE);
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
    gUIThreadQueue.addTask(onDownloadCompleteUI, this,(void*)"resolve");

  }
  
}


// Static callback that gets called when fileDownloadRequest completes 
void rtResource::onDownloadComplete(pxFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL && fileDownloadRequest->getCallbackData() != NULL) 
  {
    // Call virtual processDownlodedResource for specialized handling - 
    // Call directly rather than queuing
    ((rtResource*)fileDownloadRequest->getCallbackData())->processDownloadedResource(fileDownloadRequest);
    // Clear download data
    ((rtResource*)fileDownloadRequest->getCallbackData())->mDownloadRequest = 0;
  }
}

bool rtImageResource::loadResourceData(pxFileDownloadRequest* fileDownloadRequest)
{
      pxOffscreen imageOffscreen;
      if (pxLoadImage(fileDownloadRequest->getDownloadedData(),
                      fileDownloadRequest->getDownloadedDataSize(),
                      imageOffscreen) == RT_OK)
      {
        mTexture = context.createTexture(imageOffscreen);
        return true;
      }
      
      return false;
}
/** rtResource processDownloadedResource */
void rtResource::processDownloadedResource(pxFileDownloadRequest* fileDownloadRequest)
{
  rtString val = "reject";
  if (fileDownloadRequest != NULL)
  {
    if (fileDownloadRequest->getDownloadStatusCode() == 0 &&
        fileDownloadRequest->getHttpStatusCode() == 200 &&
        fileDownloadRequest->getDownloadedData() != NULL)
    {
      if(!loadResourceData(fileDownloadRequest))
      {
        rtLogError("Image Decode Failed: %s", fileDownloadRequest->getFileUrl().cString());
        mLoadStatus.set("statusCode", RT_RESOURCE_STATUS_DECODE_FAILURE);
        mLoadStatus.set("httpStatusCode", (uint32_t)fileDownloadRequest->getHttpStatusCode());
        // Since this object can be released before we get a async completion
        // We need to maintain this object's lifetime
        // TODO review overall flow and organization
        AddRef();        
        gUIThreadQueue.addTask(rtResource::onDownloadCompleteUI, this, (void*)"reject");        
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
        gUIThreadQueue.addTask(rtResource::onDownloadCompleteUI, this, (void*)"resolve"); 
      }
    }
    else 
    {
      rtLogWarn("Image Download Failed: %s Error: %s HTTP Status Code: %ld",
                fileDownloadRequest->getFileUrl().cString(),
                fileDownloadRequest->getErrorString().cString(),
                fileDownloadRequest->getHttpStatusCode());
      mLoadStatus.set("statusCode", RT_RESOURCE_STATUS_HTTP_ERROR);
      mLoadStatus.set("httpStatusCode",(uint32_t)fileDownloadRequest->getHttpStatusCode());
      // Since this object can be released before we get a async completion
      // We need to maintain this object's lifetime
      // TODO review overall flow and organization
      AddRef();        
      gUIThreadQueue.addTask(rtResource::onDownloadCompleteUI, this, (void*)"reject");      
    }
  }

}
/**
 * rtImageResource 
 */
ImageMap pxImageManager::mImageMap;
rtRefT<rtImageResource> pxImageManager::emptyUrlResource = 0;
/** static pxImageManager::getImage */
rtRefT<rtImageResource> pxImageManager::getImage(const char* url)
{
  //printf("pxImageManager::getImage\n");
  // Handle empty url
  if(!url || strlen(url) == 0) {
    if( !emptyUrlResource) {
      //printf("Creating empty Url rtImageResource\n");
      emptyUrlResource = new rtImageResource;
      //printf("Done creating empty Url rtImageResource\n");
    }
    //printf("Returning empty Url rtImageResource\n");
    return emptyUrlResource;
  }
  
  rtRefT<rtImageResource> pResImage;
  
  ImageMap::iterator it = mImageMap.find(url);
  if (it != mImageMap.end())
  {
    //rtLogInfo("Found rtImageResource in map for \"%s\"\n",url);
    //if( it->second->getRefCount() == 0)
      //printf("ZERO REF COUNT IN GETIMAGE!\n");
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
  //printf("pxImageManager::removeImage(\"%s\")\n",imageUrl.cString());
  ImageMap::iterator it = mImageMap.find(imageUrl);
  if (it != mImageMap.end())
  {  
    mImageMap.erase(it);
  }
  //mImageMap.erase(imageUrl);
}

rtDefineObject(rtResource, rtObject);
rtDefineProperty(rtResource,url);
rtDefineProperty(rtResource,ready);
rtDefineProperty(rtResource,loadStatus);

rtDefineObject(rtImageResource, rtResource);
rtDefineProperty(rtImageResource, w);
rtDefineProperty(rtImageResource, h); 
