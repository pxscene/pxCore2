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
#include "rtCORS.h"
#include <map>
class rtFileDownloadRequest;

#define PX_RESOURCE_STATUS_OK             0
#define PX_RESOURCE_STATUS_DOWNLOADING    1
#define PX_RESOURCE_STATUS_FILE_NOT_FOUND 2
#define PX_RESOURCE_STATUS_NETWORK_ERROR  3
#define PX_RESOURCE_STATUS_DECODE_FAILURE 4
#define PX_RESOURCE_STATUS_HTTP_ERROR     5
#define PX_RESOURCE_STATUS_UNKNOWN_ERROR  6


// errors specific to rtRemote
#define PX_RESOURCE_LOAD_SUCCESS 0
#define PX_RESOURCE_LOAD_FAIL 1
#define PX_RESOURCE_LOAD_WAIT 2


class pxResourceListener 
{
public: 
  virtual void resourceReady(rtString readyResolution) = 0;
  virtual void resourceDirty() = 0;
};

class pxResource : public rtObject
{
public: 
  rtDeclareObject(pxResource, rtObject);
  
  rtReadOnlyProperty(url,url,rtString);
  rtReadOnlyProperty(proxy,proxy,rtString);
  rtReadOnlyProperty(ready,ready,rtObjectRef);
  rtReadOnlyProperty(loadStatus,loadStatus,rtObjectRef);
    
  pxResource():mUrl(0),mDownloadRequest(NULL),mDownloadInProgress(false), priorityRaised(false),mReady(), mListeners(),
               mListenersMutex(), mDownloadInProgressMutex(), mLoadStatusMutex(), mName("")
  {
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
  virtual void loadResource(rtObjectRef archive = NULL);
  void clearDownloadRequest();
  virtual void setupResource() {}
  virtual void prepare() {}
  void setLoadStatus(const char* name, rtValue value);
  virtual void releaseData();
  virtual void reloadData();
  virtual uint64_t textureMemoryUsage();
  void setCORS(const rtCORSRef& cors) { mCORS = cors; }
  void setName(rtString name) { mName = name; }
protected:   
  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
  static void onDownloadCanceledUI(void* context, void* data);
  static void onResourceDirtyUI(void* context, void* data);
  virtual void processDownloadedResource(rtFileDownloadRequest* fileDownloadRequest);
  virtual uint32_t loadResourceData(rtFileDownloadRequest* fileDownloadRequest) = 0;
  
  void notifyListeners(rtString readyResolution);
  void notifyListenersResourceDirty();

  virtual void loadResourceFromFile() = 0;
  virtual void loadResourceFromArchive(rtObjectRef archiveRef) = 0;
  
  rtString mUrl;
  rtString mProxy;

  rtFileDownloadRequest* mDownloadRequest;
  bool mDownloadInProgress;
  bool priorityRaised;

  rtObjectRef mLoadStatus;
  rtObjectRef mReady;
  std::list<pxResourceListener*> mListeners;
  rtMutex mListenersMutex;
  rtMutex mDownloadInProgressMutex;
  mutable rtMutex mLoadStatusMutex;
  rtCORSRef mCORS;
  rtString mName;
};

class rtImageResource : public pxResource, public pxTextureListener
{
public:
  rtImageResource();
  rtImageResource(const char* url, const char* proxy = 0, int32_t iw = 0, int32_t ih = 0, float sx = 1.0f, float sy = 1.0f);

  virtual ~rtImageResource();

  rtDeclareObject(rtImageResource, pxResource);

  virtual unsigned long Release() ;

  // Need these, or use from texture?  

  rtReadOnlyProperty(w, w, int32_t);
  rtReadOnlyProperty(h, h, int32_t);  

  virtual int32_t w() const;
  virtual rtError w(int32_t& v) const;
  virtual int32_t h() const;
  virtual rtError h(int32_t& v) const; 

  pxTextureRef getTexture(bool initializing = false);
  void setTextureData(pxOffscreen& imageOffscreen, const char* data, const size_t dataSize);
  virtual void setupResource();
  virtual void prepare();

  virtual void init();
  
  int32_t initW()  { return init_w;  };
  int32_t initH()  { return init_h;  };
  
  float   initSX() { return init_sx; };
  float   initSY() { return init_sy; };

  void initUriData(const uint8_t* data, size_t length) { mData.init(data, length);                                };
  void initUriData(rtData&   d)                        { mData.init(d.data(), d.length());                        };
  void initUriData(rtString& s)                        { mData.init( (const uint8_t* ) s.cString(), s.length() ); };

  virtual void releaseData();
  virtual void reloadData();
  virtual uint64_t textureMemoryUsage();
  virtual void textureReady();
  
protected:
  virtual uint32_t loadResourceData(rtFileDownloadRequest* fileDownloadRequest);

private:

  void loadResourceFromFile();
  void loadResourceFromArchive(rtObjectRef archiveRef);

  pxTextureRef mTexture;
  pxTextureRef mDownloadedTexture;
  rtMutex mTextureMutex;
  bool mDownloadComplete;

  // convey "create-time" dimension & scale preference (SVG only)
  int32_t   init_w,  init_h;
  float     init_sx, init_sy;

  rtData    mData;
};

class rtImageAResource : public pxResource
{
public:
  rtImageAResource(const char* url = 0, const char* proxy = 0);
  virtual ~rtImageAResource();

  rtDeclareObject(rtImageAResource, pxResource);

  virtual unsigned long Release() ;

  virtual void init();
  pxTimedOffscreenSequence& getTimedOffscreenSequence() { return mTimedOffscreenSequence; }
  virtual void setupResource() { init(); }

protected:
  virtual uint32_t loadResourceData(rtFileDownloadRequest* fileDownloadRequest);

private:

  void loadResourceFromFile();
  void loadResourceFromArchive(rtObjectRef archiveRef);
  pxTimedOffscreenSequence mTimedOffscreenSequence;

};

// Weak Map
typedef std::map<rtString, rtImageResource*> ImageMap;
typedef std::map<rtString, rtImageAResource*> ImageAMap;
class pxImageManager
{  
  public: 
    static rtRef<rtImageResource> getImage(const char* url, const char* proxy = NULL, const rtCORSRef& cors = NULL,
                                          int32_t iw = 0, int32_t ih = 0, float sx = 1.0f, float sy = 1.0f, rtObjectRef archive = NULL);
  
    static void removeImage(rtString name);

    static rtRef<rtImageAResource> getImageA(const char* url, const char* proxy = NULL, const rtCORSRef& cors = NULL, rtObjectRef archive = NULL);
    static void removeImageA(rtString name);
    
  private: 
    static ImageMap mImageMap;
    static rtRef<rtImageResource> emptyUrlResource;

    static ImageAMap mImageAMap;
    static rtRef<rtImageAResource> emptyUrlImageAResource;
};

#endif // PX_RESOURCE


