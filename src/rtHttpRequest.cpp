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

#include "rtHttpRequest.h"

#include "rtHttpResponse.h"

rtDefineObject(rtHttpRequest, rtObject);

rtDefineMethod(rtHttpRequest, addListener);
rtDefineMethod(rtHttpRequest, abort);
rtDefineMethod(rtHttpRequest, end);
rtDefineMethod(rtHttpRequest, write);
rtDefineMethod(rtHttpRequest, setTimeout);
rtDefineMethod(rtHttpRequest, setHeader);

rtHttpRequest::rtHttpRequest(const rtString& url)
  : mEmit(new rtEmit())
  , mUrl(url)
  , mInQueue(false)
{
}

rtHttpRequest::rtHttpRequest(const rtObjectRef& options)
  : mEmit(new rtEmit())
  , mInQueue(false)
{
  rtString url;

  rtString proto = options.get<rtString>("protocol");
  rtString host = options.get<rtString>("host");
  rtString hostname = options.get<rtString>("hostname");
  rtString path = options.get<rtString>("path");
  rtString method = options.get<rtString>("method");
  rtObjectRef headers = options.get<rtObjectRef>("headers");
  uint32_t port = options.get<uint32_t>("port");

  mMethod = method;

  url.append(proto.cString());
  url.append("//");
  if (!host.isEmpty()) {
    url.append(host.cString());
  } else if (!hostname.isEmpty()) {
    url.append(hostname.cString());
  }
  if (port > 0) {
    rtValue portValue(port);
    url.append(":" + portValue.toString());
  }
  url.append(path.cString());

  mUrl = url;

  if (headers) {
    rtValue allKeys;
    headers->Get("allKeys", &allKeys);
    rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
    for (uint32_t i = 0, l = arr->length(); i < l; ++i) {
      rtValue key;
      if (arr->Get(i, &key) == RT_OK && !key.isEmpty()) {
        rtString s = key.toString();
        rtString val = headers.get<rtString>(s);
        mHeaders.push_back(s + ": " + val);
      }
    }
  }
}

rtHttpRequest::~rtHttpRequest()
{
}

rtError rtHttpRequest::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
}

rtError rtHttpRequest::abort() const
{
  // TODO
  return RT_OK;
}

rtError rtHttpRequest::end()
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }

  rtFileDownloadRequest* req = new rtFileDownloadRequest(mUrl.cString(), this, rtHttpRequest::onDownloadComplete);
  req->setAdditionalHttpHeaders(mHeaders);
  req->setMethod(mMethod);
  req->setReadData(mWriteData);
  if (rtFileDownloader::instance()->addToDownloadQueue(req)) {
    mInQueue = true;
    return RT_OK;
  }

  rtLogError("%s: failed to add in queue", __FUNCTION__);
  return RT_FAIL;
}

rtError rtHttpRequest::write(const rtString& chunk)
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }

  mWriteData = chunk;
  return RT_OK;
}

rtError rtHttpRequest::setTimeout(int32_t msecs, const rtFunctionRef& f)
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }

  // TODO
  UNUSED_PARAM(msecs);
  UNUSED_PARAM(f);
  return RT_OK;
}

rtError rtHttpRequest::setHeader(const rtString& name, const rtString& value)
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }

  mHeaders.push_back(name + ": " + value);
  return RT_OK;
}

void rtHttpRequest::onDownloadComplete(rtFileDownloadRequest* downloadRequest)
{
  rtHttpRequest* req = (rtHttpRequest*)downloadRequest->callbackData();

  rtHttpResponse* resp = new rtHttpResponse();

  resp->setStatusCode((int32_t)downloadRequest->httpStatusCode());
  resp->setErrorMessage(downloadRequest->errorString());
  resp->setHeaders(downloadRequest->headerData(), downloadRequest->headerDataSize());
  resp->setDownloadedData(downloadRequest->downloadedData(), downloadRequest->downloadedDataSize());

  rtObjectRef ref = resp; 
  req->mEmit.send("response", ref);

  resp->onData();

  resp->onEnd();
}

rtString rtHttpRequest::url() const
{
  return mUrl;
}

std::vector<rtString> rtHttpRequest::headers() const
{
  return mHeaders;
}

rtString rtHttpRequest::method() const
{
  return mMethod;
}

rtString rtHttpRequest::writeData() const
{
  return mWriteData;
}

bool rtHttpRequest::inQueue() const
{
  return mInQueue;
}
