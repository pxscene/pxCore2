#include "pxTextureCacheObject.h"
#include "rtLog.h"
#include "pxUtil.h"
#include "pxContext.h"
#include "pxFileDownloader.h"
#include "pxScene2d.h"
#include "pxTimer.h"

extern pxContext context;

#include <map>
using namespace std;

typedef map<rtString, pxTextureRef> TextureMap;
TextureMap gCompleteTextureCache;

int gTextureDownloadsPending = 0; //must only be set in the main thread
rtMutex textureDownloadMutex;
bool textureDownloadsAvailable = false;
vector<ImageDownloadRequest> completedTextureDownloads;

void pxTextureDownloadComplete(pxFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL)
  {
    ImageDownloadRequest imageDownloadRequest;
    imageDownloadRequest.fileDownloadRequest = fileDownloadRequest;
    if (fileDownloadRequest->getDownloadStatusCode() == 0 &&
        fileDownloadRequest->getHttpStatusCode() == 200 &&
        fileDownloadRequest->getDownloadedData() != NULL)
    {
      pxOffscreen imageOffscreen;
      if (pxLoadImage(fileDownloadRequest->getDownloadedData(),
                      fileDownloadRequest->getDownloadedDataSize(),
                      imageOffscreen) != RT_OK)
      {
        rtLogError("Image Decode Failed: %s", fileDownloadRequest->getFileUrl().cString());
      }
      else
      {
        imageDownloadRequest.texture = context.createTexture(imageOffscreen);
      }
    }
    textureDownloadMutex.lock();
    completedTextureDownloads.push_back(imageDownloadRequest);
    textureDownloadsAvailable = true;
    textureDownloadMutex.unlock();
  }
}

void pxTextureCacheObject::checkForCompletedDownloads(int maxTimeInMilliseconds)
{
  double startTimeInMs = pxMilliseconds();
  if (gTextureDownloadsPending > 0)
  {
    textureDownloadMutex.lock();
    if (textureDownloadsAvailable)
    {
      for(vector<ImageDownloadRequest>::iterator it = completedTextureDownloads.begin(); it != completedTextureDownloads.end(); )
      {
        ImageDownloadRequest imageDownloadRequest = (*it);
        if (!imageDownloadRequest.fileDownloadRequest)
          continue;
        if (imageDownloadRequest.fileDownloadRequest->getCallbackData() != NULL)
        {
          pxTextureCacheObject *textureCacheObject = (pxTextureCacheObject *) imageDownloadRequest.fileDownloadRequest->getCallbackData();
          textureCacheObject->onImageDownloadComplete(imageDownloadRequest);
        }

        delete imageDownloadRequest.fileDownloadRequest;
        textureDownloadsAvailable = false;
        gTextureDownloadsPending--;
        it = completedTextureDownloads.erase(it);
        double currentTimeInMs = pxMilliseconds();
        if ((maxTimeInMilliseconds >= 0) && (currentTimeInMs - startTimeInMs > maxTimeInMilliseconds))
        {
          break;
        }
      }
      if (gTextureDownloadsPending < 0)
      {
        //this is a safety check (hopefully never used)
        //to ensure downloads are still processed in the event of a gTextureDownloadsPending bug in the future
        gTextureDownloadsPending = 0;
      }
    }
    textureDownloadMutex.unlock();
  }
}

pxTextureCacheObject::~pxTextureCacheObject()
{
  if (mImageDownloadRequest != NULL)
  {
    // if there is a previous request pending then set the callback to NULL
    // the previous request will not be processed and the memory will be freed when the download is complete
    mImageDownloadRequest->setCallbackFunctionThreadSafe(NULL);
  }
}

void pxTextureCacheObject::onImageDownloadComplete(ImageDownloadRequest imageDownloadRequest)
{
  mDownloadInProgress = false;
  mImageDownloadRequest = NULL;
  if (imageDownloadRequest.fileDownloadRequest == NULL)
  {
      return;
  }
  if (imageDownloadRequest.fileDownloadRequest->getDownloadStatusCode() == 0 &&
      imageDownloadRequest.fileDownloadRequest->getHttpStatusCode() == 200 &&
      imageDownloadRequest.fileDownloadRequest->getDownloadedData() != NULL)
  {
      if (imageDownloadRequest.texture.getPtr() == NULL)
      {
          rtLogError("Image Decode Failed: %s", imageDownloadRequest.fileDownloadRequest->getFileUrl().cString());
          gCompleteTextureCache.erase(mUrl.cString());
          mTexture->notifyListeners(mTexture,RT_FAIL, RT_TEXTURE_STATUS_DECODE_FAILURE);
          mTexture = 0;
      }
      else
      {
          pxTextureRef tmpTexture;
          mTexture = imageDownloadRequest.texture;
          //replace "empty" texture in the vector
          TextureMap::iterator it = gCompleteTextureCache.find(mUrl.cString());
          // replace existing element that should have had type == PX_TEXTURE_UNKNOWN
          if( it != gCompleteTextureCache.end()) 
          {
            if( it->second->getType() != PX_TEXTURE_UNKNOWN) {
              rtLogWarn("Image being downloaded already had texture\n"); }
            tmpTexture = it->second; 
            gCompleteTextureCache.erase(mUrl.cString());
          }
          gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(mUrl.cString(), mTexture));
    
          // Notify listeners on old and new mTexture
          tmpTexture->notifyListeners(mTexture,RT_OK, RT_TEXTURE_STATUS_OK);  
          mTexture->notifyListeners(mTexture,RT_OK, RT_TEXTURE_STATUS_OK);  
                  
          rtLogDebug("image %d, %d", mTexture->width(), mTexture->height());

      }
  }
  else
  {
      rtLogWarn("Image Download Failed: %s Error: %s HTTP Status Code: %ld",
                imageDownloadRequest.fileDownloadRequest->getFileUrl().cString(),
                imageDownloadRequest.fileDownloadRequest->getErrorString().cString(),
                imageDownloadRequest.fileDownloadRequest->getHttpStatusCode());
      
      gCompleteTextureCache.erase(mUrl.cString());
      mTexture->notifyListeners(mTexture,RT_FAIL, RT_TEXTURE_STATUS_HTTP_ERROR, imageDownloadRequest.fileDownloadRequest->getHttpStatusCode());
      mTexture = 0;
  }
}

rtError pxTextureCacheObject::url(rtString& s) const
{ 
  s = mUrl; 
  return RT_OK;
}

rtError pxTextureCacheObject::setUrl(const char* s)
{
  if (!s || !mUrl.compare(s)) 
    return RT_OK;  
  mUrl = s;

  loadImage(mUrl);
  return RT_OK;
}

void pxTextureCacheObject::setParent(pxObject* parent)
{
    mParent = parent;
}


void pxTextureCacheObject::notifyTextureReady(pxTexture* texture, rtError rtnCode, int statusCode, int httpStatusCode=0)
{
  mTexture = texture;
  setStatus(statusCode, httpStatusCode);
  if( mParent) 
  {
    mParent->onTextureReady(this, rtnCode);
  }
}

void pxTextureCacheObject::loadImage(rtString url)
{
  TextureMap::iterator it = gCompleteTextureCache.find(url);
  if (it != gCompleteTextureCache.end())
  {
    mTexture = it->second;
    // If it's here, but an "empty" texture, register as listener
    if( mTexture->getType() == PX_TEXTURE_UNKNOWN) 
    {
      rtLogDebug("pxTextureCacheObject::loadImage adding listener for %s\n", url.cString());
      mTexture->addListener(this);
    }
    else 
    {
      rtLogDebug("pxTextureCacheObject::loadImageSENDING ON TEXTURE READY for %s\n",url.cString());
      notifyTextureReady(mTexture,RT_OK,RT_TEXTURE_STATUS_OK);
    }

  }
  else
  {
    rtLogDebug("pxTextureCacheObject::loadImage startingDownload for %s\n",url.cString());
    startImageDownload(url);

  }
}

void pxTextureCacheObject::startImageDownload(rtString url)
{
    rtLogDebug("StartImageDownload for %s\n",url.cString());
    
    // Create empty texture as a placeholder in the cache map
    mTexture = context.createTexture();
    mTexture->addListener(this);
    gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(url, mTexture));

    rtLogDebug("Image texture cache miss");
    const char* s = url.cString();
    const char *result = strstr(s, "http");
    int position = result - s;
    if (position == 0 && strlen(s) > 0)
    {
      if (mImageDownloadRequest != NULL)
      {
        if(!mImageDownloadRequest->getFileUrl().compare(url))
        {
          printf("DOWNLOAD ALREADY IN THE QUEUE FOR %s\n",url.cString());
          return;  // Download already in progress
          
        }
        // if there is a previous request pending then set the callback to NULL
        // the previous request will not be processed and the memory will be freed when the download is complete
        mImageDownloadRequest->setCallbackFunctionThreadSafe(NULL);
      }
      mImageDownloadRequest =
        new pxFileDownloadRequest(s, this);
      gTextureDownloadsPending++;
      setStatus(RT_TEXTURE_STATUS_DOWNLOADING);
      mImageDownloadRequest->setCallbackFunction(pxTextureDownloadComplete);
      pxFileDownloader::getInstance()->addToDownloadQueue(mImageDownloadRequest);
      mDownloadInProgress = true;
    }
    else 
    {
      pxOffscreen imageOffscreen;
      rtError loadImageSuccess = pxLoadImage(s, imageOffscreen);
      if ( loadImageSuccess != RT_OK)
      {
        rtLogWarn("image load failed"); // TODO: why?
        int errorCode = RT_TEXTURE_STATUS_DECODE_FAILURE;
        if (loadImageSuccess == RT_RESOURCE_NOT_FOUND)
        {
          errorCode = RT_TEXTURE_STATUS_FILE_NOT_FOUND;
        }

        mTexture->notifyListeners( mTexture, RT_FAIL, errorCode);

      }
      else
      {
        pxTextureRef tmpTexture;
        // create offscreen texture for local image
        mTexture = context.createTexture(imageOffscreen);
        TextureMap::iterator it = gCompleteTextureCache.find(s);
        // replace existing element that should have had type == PX_TEXTURE_UNKNOWN
        if(it != gCompleteTextureCache.end()) 
        {
          if( it->second->getType() != PX_TEXTURE_UNKNOWN) {
            rtLogWarn("Local image already had texture\n"); }
          // notify listeners of the old texture
          tmpTexture = it->second;
          gCompleteTextureCache.erase(mUrl.cString());
        }
        else 
        {
          // This should really never happen because we added it already
          rtLogWarn("Local image was not in map\n");
        }
        gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(s, mTexture));
        // notify listeners of this texture
        mTexture->notifyListeners(mTexture,RT_OK, RT_TEXTURE_STATUS_OK);
        tmpTexture->notifyListeners(mTexture,RT_OK, RT_TEXTURE_STATUS_OK);

      }
    }  
}


int pxTextureCacheObject::getStatusCode()
{
  return mStatusCode;
}

int pxTextureCacheObject::getHttpStatusCode()
{
  return mHttpStatusCode;
}

void pxTextureCacheObject::setStatus(int statusCode, int httpStatusCode)
{
  mStatusCode = statusCode;
  mHttpStatusCode = httpStatusCode;
}

void pxTextureCacheObject::raiseDownloadPriority()
{
  if (!mUrl.isEmpty() && mImageDownloadRequest != NULL)
  {
    pxFileDownloader::getInstance()->raiseDownloadPriority(mImageDownloadRequest);
  }
}
