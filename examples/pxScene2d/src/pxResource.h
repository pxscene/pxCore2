// pxCore CopyRight 2007-2015 John Robinson
// pxResource.h

#ifndef PX_RESOURCE_H
#define PX_RESOURCE_H

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
#include "pxTexture.h"
#include "rtMutex.h"
#ifdef ENABLE_HTTP_CACHE
#include "pxFileCache.h"
#endif
#include <map>
class pxFileDownloadRequest;

#define PX_RESOURCE_STATUS_OK             0
#define PX_RESOURCE_STATUS_DOWNLOADING    1
#define PX_RESOURCE_STATUS_FILE_NOT_FOUND 2
#define PX_RESOURCE_STATUS_NETWORK_ERROR  3
#define PX_RESOURCE_STATUS_DECODE_FAILURE 4
#define PX_RESOURCE_STATUS_HTTP_ERROR     5
#define PX_RESOURCE_STATUS_UNKNOWN_ERROR  6


class pxResourceListener 
{
public: 
  virtual void resourceReady(rtString readyResolution) = 0;
};

class pxResource : public rtObject
{
public: 
  rtDeclareObject(pxResource, rtObject);
  
  rtReadOnlyProperty(url,url,rtString);
  rtReadOnlyProperty(ready,ready,rtObjectRef);
  rtReadOnlyProperty(loadStatus,loadStatus,rtObjectRef);
    
  pxResource():mUrl(0),mDownloadRequest(0),priorityRaised(false),mReady(), mListenersMutex(){  
    mReady = new rtPromise;
    mLoadStatus = new rtMapObject; 
    mLoadStatus.set("statusCode", 0);
   }
  virtual ~pxResource();

  
  rtError url(rtString& s) const { s = mUrl; return RT_OK;}
  rtError setUrl(const char* s);
  rtString getUrl() { return mUrl;}

  rtError ready(rtObjectRef& r) const;
  rtError loadStatus(rtObjectRef& v) const;
  
  void setLoadStatus(rtString key, rtValue val);
  void sendPromise(rtString val) { mReady.send(val, this);}
    
  virtual void init() = 0;
  
  // Convenience method; not exposed to javascript
  rtValue getLoadStatus(rtString key);
  bool isInitialized() { return mInitialized; }
  
  bool isDownloadInProgress() { return (mDownloadRequest!=NULL);}  
  virtual void raiseDownloadPriority(); 
  void addListener(pxResourceListener* pListener);
  void removeListener(pxResourceListener* pListener);
  virtual void loadResource();
protected:   
  static void onDownloadComplete(pxFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
  virtual void processDownloadedResource(pxFileDownloadRequest* fileDownloadRequest);
  virtual bool loadResourceData(pxFileDownloadRequest* fileDownloadRequest) = 0;
  
  void notifyListeners(rtString readyResolution);

  virtual void loadResourceFromFile() = 0;

  
  rtString mUrl;
  pxFileDownloadRequest* mDownloadRequest;  
  bool priorityRaised;

  rtObjectRef mLoadStatus;
  rtObjectRef mReady;
  list<pxResourceListener*> mListeners;
  rtMutex mListenersMutex;
};

class rtImageResource : public pxResource
{
public:
  rtImageResource(const char* url = 0);
  ~rtImageResource(); 
  
  rtDeclareObject(rtImageResource, pxResource);
  
  virtual unsigned long Release() ;

  // Need these, or use from texture?  
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);  

  int32_t w() const;
  rtError w(int32_t& v) const;
  int32_t h() const;
  rtError h(int32_t& v) const; 

  pxTextureRef getTexture() { return mTexture; }  
 
  virtual void init();

protected:  
  virtual bool loadResourceData(pxFileDownloadRequest* fileDownloadRequest);
  
private: 

  void loadResourceFromFile();
  pxTextureRef mTexture;
 
};

// Weak Map
typedef map<rtString, rtImageResource*> ImageMap;
class pxImageManager
{
  
  public: 
    static rtRefT<rtImageResource> getImage(const char* url);
    static void removeImage(rtString imageUrl);
    
  private: 
    static ImageMap mImageMap;
    static rtRefT<rtImageResource> emptyUrlResource;

};



#endif // PX_RESOURCE


