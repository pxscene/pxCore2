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

// pxResource.h

#ifndef PX_RESOURCE_H
#define PX_RESOURCE_H

#include "rtRef.h"
#include "rtString.h"

#include "rtCore.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"
#include "pxTexture.h"
#include "rtMutex.h"
#include "pxUtil.h"
#ifdef ENABLE_HTTP_CACHE
#include "rtFileCache.h"
#endif
#include <map>
class rtFileDownloadRequest;

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
  rtReadOnlyProperty(proxy,proxy,rtString);
  rtReadOnlyProperty(ready,ready,rtObjectRef);
  rtReadOnlyProperty(loadStatus,loadStatus,rtObjectRef);
    
  pxResource():mUrl(0),mDownloadRequest(0),priorityRaised(false),mReady(), mListenersMutex(){  
    mReady = new rtPromise;
    mLoadStatus = new rtMapObject; 
    mLoadStatus.set("statusCode", 0);
   }
  virtual ~pxResource();

  
  rtError url(rtString& s) const { s = mUrl; return RT_OK;}
  rtError setUrl(const char* s, const char* proxy = NULL);
  rtString getUrl() { return mUrl;}

  rtError proxy(rtString& s) const { s = mProxy; return RT_OK;}
  rtString getProxy() { return mProxy;}

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
  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
  virtual void processDownloadedResource(rtFileDownloadRequest* fileDownloadRequest);
  virtual bool loadResourceData(rtFileDownloadRequest* fileDownloadRequest) = 0;
  
  void notifyListeners(rtString readyResolution);

  virtual void loadResourceFromFile() = 0;

  
  rtString mUrl;
  rtString mProxy;
  rtFileDownloadRequest* mDownloadRequest;  
  bool priorityRaised;

  rtObjectRef mLoadStatus;
  rtObjectRef mReady;
  std::list<pxResourceListener*> mListeners;
  rtMutex mListenersMutex;
};

class rtImageResource : public pxResource
{
public:
  rtImageResource(const char* url = 0, const char* proxy = 0);
  ~rtImageResource(); 
  
  rtDeclareObject(rtImageResource, pxResource);
  
  virtual unsigned long Release() ;

  // Need these, or use from texture?  
  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);  

  virtual int32_t w() const;
  virtual rtError w(int32_t& v) const;
  virtual int32_t h() const;
  virtual rtError h(int32_t& v) const; 

  pxTextureRef getTexture() { return mTexture; }  
 
  virtual void init();

protected:  
  virtual bool loadResourceData(rtFileDownloadRequest* fileDownloadRequest);
  
private: 

  void loadResourceFromFile();
  pxTextureRef mTexture;
 
};

class rtImageAResource : public pxResource
{
public:
  rtImageAResource(const char* url = 0, const char* proxy = 0);
  ~rtImageAResource();

  rtDeclareObject(rtImageAResource, pxResource);

  virtual unsigned long Release() ;

  virtual void init();
  pxTimedOffscreenSequence& getTimedOffscreenSequence() { return mTimedOffscreenSequence; }

protected:
  virtual bool loadResourceData(rtFileDownloadRequest* fileDownloadRequest);

private:

  void loadResourceFromFile();
  pxTimedOffscreenSequence mTimedOffscreenSequence;

};

// Weak Map
typedef std::map<rtString, rtImageResource*> ImageMap;
typedef std::map<rtString, rtImageAResource*> ImageAMap;
class pxImageManager
{
  
  public: 
    static rtRef<rtImageResource> getImage(const char* url, const char* proxy = NULL);
    static void removeImage(rtString imageUrl);

    static rtRef<rtImageAResource> getImageA(const char* url, const char* proxy = NULL);
    static void removeImageA(rtString imageAUrl);
    
  private: 
    static ImageMap mImageMap;
    static rtRef<rtImageResource> emptyUrlResource;

    static ImageAMap mImageAMap;
    static rtRef<rtImageAResource> emptyUrlImageAResource;

};



#endif // PX_RESOURCE


