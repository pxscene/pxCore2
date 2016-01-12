// pxCore CopyRight 2007-2015 John Robinson
// rtResource.h

#ifndef RT_RESOURCE_H
#define RT_RESOURCE_H

#include "rtRefT.h"
#include "rtString.h"

// TODO rtDefs vs rtCore.h
#include "rtDefs.h"
//#include "rtCore.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"


class rtResource : public rtObject
{
public: 
  rtDeclareObject(rtResource, rtObject);
  
  rtReadOnlyProperty(url,url,rtString);
  rtReadOnlyProperty(ready,ready,rtObjectRef);
  rtReadOnlyProperty(loadStatus,loadStatus,rtObjectRef);
    
  rtResource(){  
    mReady = new rtPromise;
    mLoadStatus = new rtMapObject; 
   }
  ~rtResource() {}

  virtual unsigned long AddRef() {
    return rtAtomicInc(&mRefCount);
  }
  
  virtual unsigned long Release() {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  rtError url(rtString& s) const { s = mUrl; return RT_OK;}
  rtError setUrl(const char* s);

  rtError ready(rtObjectRef& r) const;
  rtError loadStatus(rtObjectRef& v) const;
  
  void setLoadStatus(rtString key, rtValue val);
  void sendPromise(rtString val) { mReady.send(val, this);}
    
  virtual void init() = 0;
 
protected: 
 // virtual void loadResource() = 0;
  
  rtString mUrl;
  
  // Use rtObjectRef or template as below?  Same thing?
  rtObjectRef mLoadStatus;
  rtObjectRef mReady;

};

class rtResourceImage : public rtResource
{
public:
  rtResourceImage(const char* url = 0);
  ~rtResourceImage() {}
  
  rtDeclareObject(rtResourceImage, rtResource);

  // Need these, or use from texture?  
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);  

  int32_t w() const { return mw;  }
  rtError w(int32_t& v) const { v = mw;  return RT_OK; }
  int32_t h() const { return mh; }
  rtError h(int32_t& v) const { v = mh; return RT_OK; } 

  // These are not exposed to javascript
  // For convenience for now, to get from texture
  void setW(int32_t v) { mw = v; }
  void setH(int32_t v) { mh = v; }
  
   
  virtual void init();
//  virtual void loadResource();
  
  
  
protected:

  int32_t mw;
  int32_t mh;

};
/*
// Weak Map
typedef map<rtString, rtResourceImage*> ImageMap;
class pxImageManager
{
  
  public: 
    static rtRefT<rtResourceImage> getImage(const char* url);
    static void removeImage(rtString imageUrl);
    
  protected: 
    static ImageMap mImageMap;
    
};
*/


#endif


