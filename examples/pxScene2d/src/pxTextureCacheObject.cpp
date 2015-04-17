#include "pxTextureCacheObject.h"
#include "rtLog.h"
#include "pxUtil.h"
#include "pxContext.h"
#include "pxFileDownloader.h"
#include "pxScene2d.h"

extern pxContext context;

#include <map>
using namespace std;

typedef map<rtString, pxTextureRef> TextureMap;
TextureMap gCompleteTextureCache;

int gTextureDownloadsPending = 0; //must only be set in the main thread
rtMutex textureDownloadMutex;
bool textureDownloadsAvailable = false;
vector<pxFileDownloadRequest*> completedTextureDownloads;

void pxTextureDownloadComplete(pxFileDownloadRequest* fileDownloadRequest)
{
  if (fileDownloadRequest != NULL)
  {
    textureDownloadMutex.lock();
    completedTextureDownloads.push_back(fileDownloadRequest);
    textureDownloadsAvailable = true;
    textureDownloadMutex.unlock();
  }
}

void pxTextureCacheObject::checkForCompletedDownloads()
{
  if (gTextureDownloadsPending > 0)
  {
    textureDownloadMutex.lock();
    if (textureDownloadsAvailable)
    {
      for(vector<pxFileDownloadRequest*>::iterator it = completedTextureDownloads.begin(); it != completedTextureDownloads.end(); ++it)
      {
        pxFileDownloadRequest* fileDownloadRequest = (*it);
        if (!fileDownloadRequest)
          continue;
        if (fileDownloadRequest->getCallbackData() != NULL)
        {
          pxTextureCacheObject *textureCacheObject = (pxTextureCacheObject *) fileDownloadRequest->getCallbackData();
          textureCacheObject->onFileDownloadComplete(fileDownloadRequest);
        }

        delete fileDownloadRequest;
        textureDownloadsAvailable = false;
        gTextureDownloadsPending--;
      }
      completedTextureDownloads.clear();
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

void pxTextureCacheObject::onFileDownloadComplete(pxFileDownloadRequest* downloadRequest)
{
  mImageDownloadRequest = NULL;
  if (downloadRequest == NULL)
  {
      return;
  }
  if (downloadRequest->getDownloadStatusCode() == 0 &&
          downloadRequest->getHttpStatusCode() == 200 &&
          downloadRequest->getDownloadedData() != NULL)
  {
      pxOffscreen imageOffscreen;
      if (pxLoadImage(downloadRequest->getDownloadedData(),
              downloadRequest->getDownloadedDataSize(),
              imageOffscreen) != RT_OK)
      {
          rtLogError("Image Decode Failed: %s", downloadRequest->getFileURL().cString());
          if (mParent != NULL)
          {
            mParent->onTextureReady(this, RT_FAIL);
          }
      }
      else
      {
          mTexture = context.createTexture(imageOffscreen);
          gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(mURL.cString(),
                  mTexture));
          rtLogDebug("image %d, %d", mTexture->width(), mTexture->height());
          if (mParent != NULL)
          {
            mParent->onTextureReady(this, RT_OK);
          }
      }
  }
  else
  {
      rtLogWarn("Image Download Failed: %s Error: %s HTTP Status Code: %ld",
              downloadRequest->getFileURL().cString(),
              downloadRequest->getErrorString().cString(),
              downloadRequest->getHttpStatusCode());
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
      mImageDownloadRequest->setCallbackFunction(pxTextureDownloadComplete);
      pxFileDownloader::getInstance()->addToDownloadQueue(mImageDownloadRequest);
    }
    else 
    {
      pxOffscreen imageOffscreen;
      if (pxLoadImage(s, imageOffscreen) != RT_OK)
      {
        rtLogWarn("image load failed"); // TODO: why?
        mParent->onTextureReady(this, RT_FAIL);
      }
      else
      {
        mTexture = context.createTexture(imageOffscreen);
        gCompleteTextureCache.insert(pair<rtString,pxTextureRef>(s, mTexture));
        if (mParent != NULL)
        {
          mParent->onTextureReady(this, RT_OK);
        }
      }
    }
  }
}
