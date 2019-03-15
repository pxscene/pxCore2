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
  rtMethod2ArgAndNoReturn("once", once, rtString, rtFunctionRef);
  rtMethodNoArgAndNoReturn("removeAllListeners", removeAllListeners);
  rtMethod1ArgAndNoReturn("removeAllListeners", removeAllListenersByName, rtString);
  rtMethodNoArgAndNoReturn("abort", abort);
  rtMethodNoArgAndNoReturn("end", end);
  rtMethod1ArgAndNoReturn("write", write, rtValue);
  rtMethod2ArgAndNoReturn("setTimeout", setTimeout, int32_t, rtFunctionRef);
  rtMethod2ArgAndNoReturn("setHeader", setHeader, rtString, rtString);
  rtMethod1ArgAndReturn("getHeader", getHeader, rtString, rtString);
  rtMethod1ArgAndNoReturn("removeHeader", removeHeader, rtString);

  rtHttpRequest(const rtString& url = rtString());
  rtHttpRequest(const rtObjectRef& options);
  ~rtHttpRequest();

  rtError addListener(const rtString& eventName, const rtFunctionRef& f);
  rtError once(const rtString& eventName, const rtFunctionRef& f);
  rtError removeAllListeners();
  rtError removeAllListenersByName(const rtString& eventName);
  rtError abort() const;
  rtError end();
  rtError write(const rtValue& chunk);
  rtError setTimeout(int32_t msecs, const rtFunctionRef& f);
  rtError setHeader(const rtString& name, const rtString& value);
  rtError getHeader(const rtString& name, rtString& s);
  rtError removeHeader(const rtString& name);

  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  static void onDownloadCompleteAndRelease(rtFileDownloadRequest* downloadRequest);

  rtString url() const;
  std::vector<rtString> headers() const;
  rtString method() const;
  const uint8_t* writeData() const;
  size_t writeDataSize() const;
  bool inQueue() const;

private:
  rtEmitRef mEmit;
  rtString mUrl;
  std::vector<rtString> mHeaders;
  rtString mMethod;
  uint8_t* mWriteData;
  size_t mWriteDataSize;
  bool mInQueue;
};

#endif //RT_HTTP_REQUEST_H