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

#ifndef RT_HTTP_REQUEST_H
#define RT_HTTP_REQUEST_H

#include "rtObject.h"
#include "rtString.h"
#include "rtFileDownloader.h"

class rtHttpRequest : public rtObject
{
public:
  rtDeclareObject(rtHttpRequest, rtObject);

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethodNoArgAndNoReturn("abort", abort);
  rtMethodNoArgAndNoReturn("end", end);
  rtMethod1ArgAndNoReturn("write", write, rtString);
  rtMethod2ArgAndNoReturn("setTimeout", setTimeout, int32_t, rtFunctionRef);
  rtMethod2ArgAndNoReturn("setHeader", setHeader, rtString, rtString);

  rtHttpRequest(const rtString& url = rtString());
  rtHttpRequest(const rtObjectRef& options);
  ~rtHttpRequest();

  rtError addListener(rtString eventName, const rtFunctionRef& f);
  rtError abort() const;
  rtError end();
  rtError write(const rtString& chunk);
  rtError setTimeout(int32_t msecs, const rtFunctionRef& f);
  rtError setHeader(const rtString& name, const rtString& value);

  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);

  rtString url() const;
  std::vector<rtString> headers() const;
  rtString method() const;
  rtString writeData() const;
  bool inQueue() const;

private:
  rtEmitRef mEmit;
  rtString mUrl;
  std::vector<rtString> mHeaders;
  rtString mMethod;
  rtString mWriteData;
  bool mInQueue;
};

#endif //RT_HTTP_REQUEST_H