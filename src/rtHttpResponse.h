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

#ifndef RT_HTTP_RESPONSE_H
#define RT_HTTP_RESPONSE_H

#include "rtObject.h"
#include "rtString.h"

#include <map>

class rtHttpResponse : public rtObject
{
public:
  rtDeclareObject(rtHttpResponse, rtObject);

  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(message, errorMessage, rtString);
  rtReadOnlyProperty(rawHeaders, rawHeaders, rtString);
  rtReadOnlyProperty(headers, headers, rtObjectRef);

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
  rtMethod2ArgAndNoReturn("once", once, rtString, rtFunctionRef);
  rtMethodNoArgAndNoReturn("removeAllListeners", removeAllListeners);
  rtMethod1ArgAndNoReturn("removeAllListeners", removeAllListenersByName, rtString);

  rtHttpResponse();
  ~rtHttpResponse();

  rtError statusCode(int32_t& v) const;
  rtError errorMessage(rtString& v) const;
  rtError headers(rtObjectRef& v) const;
  rtError rawHeaders(rtString& v) const { v = mHeaders; return RT_OK; };
  rtError addListener(const rtString& eventName, const rtFunctionRef& f);
  rtError once(const rtString& eventName, const rtFunctionRef& f);
  rtError removeAllListeners();
  rtError removeAllListenersByName(const rtString& eventName);

  void setStatusCode(int32_t v);
  void setErrorMessage(const rtString& v);
  void setHeaders(const char* data, size_t size);
  void setDownloadedData(const char* data, size_t size);

  void onData();
  void onEnd();

  static rtError parseHeaders(const rtString& data, std::map<rtString, rtString>& headerMap);
  static rtError parseHeader(const rtString& data, rtString& key, rtString& value);
  static rtString toLowercaseStr(const rtString& str);

private:
  int32_t mStatusCode;
  rtString mErrorMessage;
  rtString mHeaders;
  rtString mDownloadedData;
  rtEmitRef mEmit;
};

#endif //RT_HTTP_RESPONSE_H
