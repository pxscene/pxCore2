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

#include "rtJsModules.h"
#include "rtWrapperUtils.h"
#include "rtObject.h"
#include "rtFileDownloader.h"

static std::vector<rtRef<rtFunctionCallback> > gBindings;

static void rtRegisterJsBinding(duk_context *ctx, const char *name, rtFunctionCB cb)
{
  rtRef<rtFunctionCallback> cbObj(new rtFunctionCallback(cb, NULL));
  gBindings.push_back(cbObj);

  rt2duk(ctx, rtValue(cbObj.getPtr()));
  rtDukPutIdentToGlobal(ctx, name);
}

rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context);

void rtSetupJsModuleBindings(duk_context *ctx)
{
  rtRegisterJsBinding(ctx, "httpGet", &rtHttpGetBinding);
}

class rtHttpResponse : public rtObject
{
public:
  rtDeclareObject(rtHttpResponse, rtObject);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(message, errorMessage, rtString);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtHttpResponse() : mStatusCode(0) {
    mEmit = new rtEmit();
  }

  rtError statusCode(int32_t& v) const { v = mStatusCode;  return RT_OK; }
  rtError errorMessage(rtString& v) const { v = mErrorMessage;  return RT_OK; }
  rtError addListener(rtString eventName, const rtFunctionRef& f) { mEmit->addListener(eventName, f); return RT_OK;  }

  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  static size_t onDownloadInProgress(void *ptr, size_t size, size_t nmemb, void *userData);

private:
  int32_t mStatusCode;
  rtString mErrorMessage;
  rtEmitRef mEmit;
};

rtDefineObject(rtHttpResponse, rtObject);
rtDefineProperty(rtHttpResponse, statusCode);
rtDefineProperty(rtHttpResponse, message);
rtDefineMethod(rtHttpResponse, addListener);

void rtHttpResponse::onDownloadComplete(rtFileDownloadRequest* downloadRequest)
{
  rtHttpResponse* resp = (rtHttpResponse*)downloadRequest->callbackData();

  resp->mStatusCode = downloadRequest->httpStatusCode();
  resp->mErrorMessage = downloadRequest->errorString();

  resp->mEmit.send(resp->mErrorMessage.isEmpty() ? "end" : "error", (rtIObject *)resp);
}

size_t rtHttpResponse::onDownloadInProgress(void *ptr, size_t size, size_t nmemb, void *userData)
{
  rtHttpResponse* resp = (rtHttpResponse*)userData;

  if (size * nmemb > 0) {
    resp->mEmit.send("data", rtString((const char *)ptr, size*nmemb));
  }
  return 0;
}

rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  if (numArgs != 2) {
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_stringType) {
    return RT_ERROR_INVALID_ARG;
  }

  if (args[1].getType() != RT_functionType) {
    return RT_ERROR_INVALID_ARG;
  }

  rtObjectRef resp(new rtHttpResponse());
  args[1].toFunction().send(resp);

  rtFileDownloadRequest *downloadRequest = new rtFileDownloadRequest(args[0].toString(), resp.getPtr());
  downloadRequest->setCallbackFunction(rtHttpResponse::onDownloadComplete);
  downloadRequest->setDownloadProgressCallbackFunction(rtHttpResponse::onDownloadInProgress, resp.getPtr());
  rtFileDownloader::instance()->addToDownloadQueue(downloadRequest);
  
  *result = resp;

  return RT_OK;
}

