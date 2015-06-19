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
        rtLogError("Image Decode Failed: %s", fileDownloadRequest->getFileURL().cString());
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
      //completedTextureDownloads.clear();
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
          rtLogError("Image Decode Failed: %s", imageDownloadRequest.fileDownloadRequest->getFileURL().cString());
          setStatus(RT_TEXTURE_STATUS_DECODE_FAILURE);
          if (mParent != NULL)
          {
            mParent->onTextureReady(this, RT_FAIL);
          }
      }
      else
      {
          mTexture = imageDownloadRequest.texture;
          gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(mURL.cString(),
                  mTexture));
          rtLogDebug("image %d, %d", mTexture->width(), mTexture->height());
          setStatus(RT_TEXTURE_STATUS_OK);
          if (mParent != NULL)
          {
            mParent->onTextureReady(this, RT_OK);
          }
      }
  }
  else
  {
      rtLogWarn("Image Download Failed: %s Error: %s HTTP Status Code: %ld",
                imageDownloadRequest.fileDownloadRequest->getFileURL().cString(),
                imageDownloadRequest.fileDownloadRequest->getErrorString().cString(),
                imageDownloadRequest.fileDownloadRequest->getHttpStatusCode());
      setStatus(RT_TEXTURE_STATUS_HTTP_ERROR, imageDownloadRequest.fileDownloadRequest->getHttpStatusCode());
      if (mParent != NULL)
      {
        mParent->onTextureReady(this, RT_FAIL);
      }
  }
}

rtError pxTextureCacheObject::url(rtString& s) const
{ 
  s = mURL; 
  return RT_OK;
}

rtError pxTextureCacheObject::setURL(const char* s)
{
  mURL = s;
  if (!s) 
    return RT_OK;
  loadImage(mURL);
  return RT_OK;
}

void pxTextureCacheObject::setParent(pxObject* parent)
{
    mParent = parent;
}

void pxTextureCacheObject::loadImage(rtString url)
{
  TextureMap::iterator it = gCompleteTextureCache.find(url);
  if (it != gCompleteTextureCache.end())
  {
    mTexture = it->second;
    setStatus(RT_TEXTURE_STATUS_OK);
    if (mParent != NULL)
    {
      mParent->onTextureReady(this, RT_OK);
    }
  }
  else
  {
    rtLogWarn("Image texture cache miss");
    const char* s = url.cString();
    const char *result = strstr(s, "http");
    int position = result - s;
    if (position == 0 && strlen(s) > 0)
    {
      if (mImageDownloadRequest != NULL)
      {
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
        setStatus(errorCode);
        if (mParent != NULL)
        {
          mParent->onTextureReady(this, RT_FAIL);
        }
      }
      else
      {
        mTexture = context.createTexture(imageOffscreen);
        gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(s, mTexture));
        setStatus(RT_TEXTURE_STATUS_OK);
        if (mParent != NULL)
        {
          mParent->onTextureReady(this, RT_OK);
        }
      }
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
