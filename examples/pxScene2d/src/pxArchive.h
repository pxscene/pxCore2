// pxCore CopyRight 2007-2015 John Robinson
// pxArchive.h

#ifndef _PX_ARCHIVE_H
#define _PX_ARCHIVE_H

#include "rtRefT.h"
#include "rtString.h"

#include "rtDefs.h"
#include "rtCore.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"
#include "pxFileDownloader.h"

#include "rtZip.h"
#ifdef ENABLE_HTTP_CACHE
#include "pxFileCache.h"
#endif
class pxArchive: public rtObject
{
public:
  rtDeclareObject(pxArchive,rtObject);
  rtReadOnlyProperty(ready,ready,rtObjectRef);
  rtReadOnlyProperty(loadStatus,loadStatus,rtObjectRef);
  rtReadOnlyProperty(fileNames,fileNames,rtObjectRef);
  rtMethod1ArgAndReturn("getFileAsString",getFileAsString,rtString,rtString);

  pxArchive();
  virtual ~pxArchive();

  rtError initFromUrl(const rtString& url);
  rtError ready(rtObjectRef& r) const;

  rtError loadStatus(rtObjectRef& v) const;

  rtError getFileAsString(const char* fileName, rtString& s);
  rtError fileNames(rtObjectRef& names) const;

private:
  static void onDownloadComplete(pxFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
#ifdef ENABLE_HTTP_CACHE
  bool checkAndDownloadFromCache();
#endif
  void process(void* data, size_t dataSize);

  bool mIsFile;
  rtString mUrl;

  rtObjectRef mLoadStatus;
  rtObjectRef mReady;

  rtData mData;
  pxFileDownloadRequest* mDownloadRequest;

  rtZip mZip;
};

#endif
