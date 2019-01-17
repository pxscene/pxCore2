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
rtDefineMethod(rtHttpRequest, once);
rtDefineMethod(rtHttpRequest, removeAllListeners);
rtDefineMethod(rtHttpRequest, removeAllListenersByName);
rtDefineMethod(rtHttpRequest, abort);
rtDefineMethod(rtHttpRequest, end);
rtDefineMethod(rtHttpRequest, write);
rtDefineMethod(rtHttpRequest, setTimeout);
rtDefineMethod(rtHttpRequest, setHeader);
rtDefineMethod(rtHttpRequest, getHeader);
rtDefineMethod(rtHttpRequest, removeHeader);

rtHttpRequest::rtHttpRequest(const rtString& url)
  : mEmit(new rtEmit())
  , mUrl(url)
  , mWriteData(NULL)
  , mWriteDataSize(0)
  , mInQueue(false)
{
}

rtHttpRequest::rtHttpRequest(const rtObjectRef& options)
  : mEmit(new rtEmit())
  , mWriteData(NULL)
  , mWriteDataSize(0)
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
    rtString portStr = ":" + portValue.toString();
    if (!url.endsWith(portStr.cString())) {
      url.append(portStr);
    }
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
  if (mWriteData)
    free(mWriteData);
}

rtError rtHttpRequest::addListener(const rtString& eventName, const rtFunctionRef& f)
{
  return mEmit->addListener(eventName, f);
}

rtError rtHttpRequest::once(const rtString& eventName, const rtFunctionRef& f)
{
  return mEmit->addListener(eventName, f, true);
}

rtError rtHttpRequest::removeAllListeners()
{
  return mEmit->clearListeners();
}

rtError rtHttpRequest::removeAllListenersByName(const rtString& eventName)
{
  return mEmit->clearListeners(eventName.cString());
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

  rtFileDownloadRequest* req = new rtFileDownloadRequest(mUrl.cString(), this, rtHttpRequest::onDownloadCompleteAndRelease);
  req->setAdditionalHttpHeaders(mHeaders);
  req->setMethod(mMethod);
  req->setReadData(mWriteData, mWriteDataSize);
  if (rtFileDownloader::instance()->addToDownloadQueue(req)) {
    AddRef();
    mInQueue = true;
    return RT_OK;
  }

  rtLogError("%s: failed to add in queue", __FUNCTION__);
  return RT_FAIL;
}

rtError rtHttpRequest::write(const rtValue& chunk)
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }

  if (mWriteData)
    free(mWriteData);

  mWriteData = NULL;
  mWriteDataSize = 0;

  if (!chunk.isEmpty()) {
    if (chunk.getType() == RT_objectType) {
      rtObjectRef obj = chunk.toObject();

      uint32_t len = obj.get<uint32_t>("length");
      if (len > 0) {
        rtLogInfo("write %u bytes (Buffer)", len);
        mWriteData = (uint8_t*)malloc(len);
        mWriteDataSize = len;
      }
      for (uint32_t i = 0; i < len; i++) {
        mWriteData[i] = obj.get<uint8_t>(i);
      }
    } else if (chunk.getType() == RT_stringType) {
      rtString str = chunk.toString();

      uint32_t len = static_cast<uint32_t>(str.byteLength());
      if (len > 0) {
        rtLogInfo("write %u bytes (string)", len);
        mWriteData = (uint8_t*)malloc(len);
        mWriteDataSize = len;
        memcpy(mWriteData, str.cString(), len);
      }
    } else {
      rtLogInfo("unknown write data type");
    }
  }

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

  this->removeHeader(name);
  mHeaders.push_back(name + ": " + value);
  return RT_OK;
}

rtError rtHttpRequest::getHeader(const rtString& name, rtString& s)
{
  size_t h_len = mHeaders.size();
  for( size_t i = 0; i < h_len; i ++)
  {
    rtString header = mHeaders[i];
    if (header.beginsWith(name.cString()))
    {
      s = header.substring(name.length() + 2, 0);
      return RT_OK;
    } 
  }
  return RT_OK;
}

rtError rtHttpRequest::removeHeader(const rtString& name)
{
  if (mInQueue) {
    rtLogError("%s: already in queue", __FUNCTION__);
    return RT_FAIL;
  }
  int need_remove_idx = -1;
  size_t h_len = mHeaders.size();
  for( size_t i = 0; i < h_len; i ++)
  {
    rtString header = mHeaders[i];
    if (header.beginsWith(name.cString()))
    {
      need_remove_idx = i;
      break;
    } 
  }
  if ( need_remove_idx >= 0) 
  {
    std::vector<rtString>::iterator it = mHeaders.begin();
    std::advance(it, need_remove_idx);
    mHeaders.erase(it);
  }
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

  if (downloadRequest->errorString().isEmpty()) {
    req->mEmit.send("response", ref);
    resp->onData();
    resp->onEnd();
  } else {
    req->mEmit.send("error", downloadRequest->errorString());
  }
}

void rtHttpRequest::onDownloadCompleteAndRelease(rtFileDownloadRequest* downloadRequest)
{
  onDownloadComplete(downloadRequest);
  rtHttpRequest* req = (rtHttpRequest*)downloadRequest->callbackData();
  if (req != NULL)
  {
    req->Release();
  }
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

const uint8_t* rtHttpRequest::writeData() const
{
  return mWriteData;
}

size_t rtHttpRequest::writeDataSize() const
{
  return mWriteDataSize;
}

bool rtHttpRequest::inQueue() const
{
  return mInQueue;
}
