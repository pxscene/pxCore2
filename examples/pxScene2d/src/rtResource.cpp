// pxCore CopyRight 2007-2015 John Robinson
// rtResource.cpp

#include "rtString.h"
#include "rtRefT.h"
//#include "pxCore.h"
//#include "pxOffscreen.h"
//#include "pxUtil.h"
#include "pxScene2d.h"
//#include "pxOffscreen.h"

#include "rtResource.h"

//#include "pxContext.h"

//#include "pxFileDownloader.h"
//#include <map>


rtError rtResource::setUrl(const char* url)
{
  mUrl = url;
  
  return RT_OK;
}

void rtResourceImage::init()
{
  if( mInitialized) 
    return; 
    
  mInitialized = true;
  //loadResource();
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

void rtResource::setLoadStatus(rtString key, rtValue val)
{
  //rtLogInfo("rtResource::setLoadStatus with %s, %s\n",key.cString(), val.toString().cString());
  mLoadStatus.set(key,val);

}

rtResourceImage::rtResourceImage(const char* url)
{
  setUrl(url);
}



//void rtResourceImage::loadResource()
//{
  //mTextureCacheObject.loadImage( mUrl);
//}

//ImageMap pxImageManager::mImageMap;
//rtRefT<rtResourceImage> pxImageManager::getImage(const char* url)
//{
  //rtRefT<rtResourceImage> pImage;

  //// Handle empty url?  But what to do about it?
  
  //ImageMap::iterator it = mImageMap.find(url);
  //if (it != mImageMap.end())
  //{
    //rtLogDebug("Found rtResourceImage in map for %s\n",s);
    //pImage = it->second;
    //return pFont;  
    
  //}
  //else 
  //{
    //rtLogDebug("Create rtResourceImage in map for %s\n",s);
    //pImage = new rtResourceImage(url);
    //mImageMap.insert(make_pair(s, pImage));
    //pImage->loadImage();
  //}
  
  //return pFont;
//}

//void pxImageManager::removeImage(rtString imageUrl)
//{
  //ImageMap::iterator it = mImageMap.find(ImageName);
  //if (it != mImageMap.end())
  //{  
    //mImageMap.erase(it);
  //}
//}
rtDefineObject(rtResource, rtObject);
rtDefineProperty(rtResource,url);
rtDefineProperty(rtResource,ready);
rtDefineProperty(rtResource,loadStatus);

rtDefineObject(rtResourceImage, rtResource);
rtDefineProperty(rtResourceImage, w);
rtDefineProperty(rtResourceImage, h); 
