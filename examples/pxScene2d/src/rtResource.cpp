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

void rtImageResource::init()
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
rtValue rtResource::getLoadStatus(rtString key)
{
  rtValue value;
  mLoadStatus.get(key, value);
  return value;
}

rtImageResource::rtImageResource(const char* url)
{
  setUrl(url);
}



//void rtImageResource::loadResource()
//{
  //mTextureCacheObject.loadImage( mUrl);
//}

//ImageMap pxImageManager::mImageMap;
//rtRefT<rtImageResource> pxImageManager::getImage(const char* url)
//{
  //rtRefT<rtImageResource> pImage;

  //// Handle empty url?  But what to do about it?
  
  //ImageMap::iterator it = mImageMap.find(url);
  //if (it != mImageMap.end())
  //{
    //rtLogDebug("Found rtImageResource in map for %s\n",s);
    //pImage = it->second;
    //return pFont;  
    
  //}
  //else 
  //{
    //rtLogDebug("Create rtImageResource in map for %s\n",s);
    //pImage = new rtImageResource(url);
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

rtDefineObject(rtImageResource, rtResource);
rtDefineProperty(rtImageResource, w);
rtDefineProperty(rtImageResource, h); 
