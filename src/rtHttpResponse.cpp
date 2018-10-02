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

rtDefineObject(rtHttpResponse, rtObject);

rtDefineProperty(rtHttpResponse, statusCode);
rtDefineProperty(rtHttpResponse, message);
rtDefineProperty(rtHttpResponse, headers);

rtDefineMethod(rtHttpResponse, addListener);

rtHttpResponse::rtHttpResponse()
  : mEmit(new rtEmit())
  , mStatusCode(0)
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
  // TODO
  UNUSED_PARAM(v);
  return RT_OK;
}

rtError rtHttpResponse::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
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
    mHeaders = rtString(data, size);
  } else {
    mHeaders = rtString();
  }
}

void rtHttpResponse::setDownloadedData(const char* data, size_t size)
{
  if (size > 0) {
    mDownloadedData = rtString(data, size);
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
