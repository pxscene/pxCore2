/*

 pxCore Copyright 2005-2017 John Robinson

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

// pxArchive.h

#ifndef _PX_ARCHIVE_H
#define _PX_ARCHIVE_H

#include "rtCore.h"
#include "rtRef.h"
#include "rtString.h"


#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "rtPromise.h"
#include "rtFileDownloader.h"

#include "rtZip.h"
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
  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteUI(void* context, void* data);
  void process(void* data, size_t dataSize);

  bool mIsFile;
  rtString mUrl;

  rtObjectRef mLoadStatus;
  rtObjectRef mReady;

  rtData mData;
  rtFileDownloadRequest* mDownloadRequest;

  rtZip mZip;
};

#endif
