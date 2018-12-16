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

#include "rtHttpResponse.h"

#include <algorithm>

rtDefineObject(rtHttpResponse, rtObject);

rtDefineProperty(rtHttpResponse, statusCode);
rtDefineProperty(rtHttpResponse, message);
rtDefineProperty(rtHttpResponse, headers);
rtDefineProperty(rtHttpResponse, rawHeaders);

rtDefineMethod(rtHttpResponse, addListener);
rtDefineMethod(rtHttpResponse, once);
rtDefineMethod(rtHttpResponse, removeAllListeners);
rtDefineMethod(rtHttpResponse, removeAllListenersByName);

rtHttpResponse::rtHttpResponse()
  : mStatusCode(0),
    mEmit(new rtEmit())
{
}

rtHttpResponse::~rtHttpResponse()
{
}

rtError rtHttpResponse::statusCode(int32_t& v) const
{
  v = mStatusCode;
  return RT_OK;
}

rtError rtHttpResponse::errorMessage(rtString& v) const
{
  v = mErrorMessage;
  return RT_OK;
}

rtError rtHttpResponse::headers(rtObjectRef& v) const
{
  rtObjectRef i = new rtMapObject();
  std::map<rtString, rtString> headerMap;
  rtHttpResponse::parseHeaders(mHeaders, headerMap);
  for (std::map<rtString,rtString>::const_iterator it=headerMap.begin(); it!=headerMap.end(); ++it) {
    i.set(it->first, it->second);
  }
  v = i;
  return RT_OK;
}

rtError rtHttpResponse::addListener(const rtString& eventName, const rtFunctionRef& f)
{
  return mEmit->addListener(eventName, f);
}

rtError rtHttpResponse::once(const rtString& eventName, const rtFunctionRef& f)
{
  return mEmit->addListener(eventName, f, true);
}

rtError rtHttpResponse::removeAllListeners()
{
  return mEmit->clearListeners();
}

rtError rtHttpResponse::removeAllListenersByName(const rtString& eventName)
{
  return mEmit->clearListeners(eventName.cString());
}

void rtHttpResponse::setStatusCode(int32_t v)
{
  mStatusCode = v;
}

void rtHttpResponse::setErrorMessage(const rtString& v)
{
  mErrorMessage = v;
}

void rtHttpResponse::setHeaders(const char* data, size_t size)
{
  if (size > 0) {
    mHeaders = rtString(data, (uint32_t) size);
  } else {
    mHeaders = rtString();
  }
}

void rtHttpResponse::setDownloadedData(const char* data, size_t size)
{
  if (size > 0) {
    mDownloadedData = rtString(data, (uint32_t) size);
  } else {
    mDownloadedData = rtString();
  }
}

void rtHttpResponse::onData()
{
  mEmit.send("data", mDownloadedData);
}

void rtHttpResponse::onEnd()
{
  if (mErrorMessage.isEmpty()) {
    mEmit.send("end");
  } else {
    mEmit.send("error", mErrorMessage);
  }
}

rtError rtHttpResponse::parseHeaders(const rtString& data, std::map<rtString, rtString>& headerMap)
{
  headerMap.clear();

  if (data.isEmpty())
    return RT_OK;

  int32_t len = data.length();
  int32_t attr1 = 0, attr2;
  attr2 = data.find(attr1, '\n');
  attr2 = -1 != attr2 ? attr2 : (attr1 < len ? len : -1);

  while (-1 != attr2) {
    rtString attribute = attr2 == attr1 ? "" : data.substring(attr1, attr2-attr1);
    rtString key, value;
    if (parseHeader(attribute, key, value) == RT_OK) {
      headerMap.insert(std::pair<rtString, rtString>(key, value));
    }
    attr1 = attr2+1;
    attr2 = data.find(attr1, '\n');
    attr2 = -1 != attr2 ? attr2 : (attr1 < len ? len : -1);
  }

  return RT_OK;
}

rtError rtHttpResponse::parseHeader(const rtString& data, rtString& key, rtString& value)
{
  if (data.isEmpty())
    return RT_FAIL;

  int32_t key2 = data.find(0, ':');
  if (key2 <= 0)
    return RT_FAIL;

  rtString k = data.substring(0, key2);
  rtString v = data.substring(key2 + 1);
  const char* bytePtr = v.cString();
  for (; *bytePtr == ' ' || *bytePtr == '\t'; bytePtr++);
  if (bytePtr != v.cString()) {
    v = v.substring(bytePtr - v.cString());
  }
  int32_t value2 = v.find(0, '\r');
  if (-1 != value2) {
    v = value2 == 0 ? "" : v.substring(0, value2);
  }
  key = toLowercaseStr(k);
  value = v;

  return RT_OK;
}

rtString rtHttpResponse::toLowercaseStr(const rtString& str)
{
  std::string s(str.cString(), str.byteLength());
  std::transform(s.begin(), s.end(), s.begin(), ::tolower);
  return rtString(s.c_str());
}
