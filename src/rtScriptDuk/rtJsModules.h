#ifndef RT_JS_MODULES_H
#define RT_JS_MODULES_H

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>
#include <rtMutex.h>
#include <rtFileDownloader.h>
#include <rtThreadQueue.h>
#include "rtWrapperUtils.h"

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

extern "C" {
#include "duv.h"

}
void rtSetupJsModuleBindings(duk_context *ctx);

/* container for the http request to track single download request and response */
struct httpRequest
{
  rtString mUrl;
  rtObjectRef mHttpResponse;
  rtFunctionRef mCallbackFunction;
  rtString mJsonResponse;
};

class rtHttpResponse : public rtObject
{
public:
  rtDeclareObject(rtHttpResponse, rtObject);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(message, errorMessage, rtString);
  rtReadOnlyProperty(headers, headers, rtObjectRef);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtHttpResponse() : mStatusCode(0),mData("") {
    mEmit = new rtEmit();
    mHeaders = new rtMapObject;
  }

  ~rtHttpResponse()
  {
  }

  void clearResources()
  {
    mEmit->clearListeners();
    mData = "";
    mHeaders = NULL;
    mEmit = NULL;
  }

  rtError statusCode(int32_t& v) const { v = mStatusCode;  return RT_OK; }
  rtError errorMessage(rtString& v) const { v = mErrorMessage;  return RT_OK; }
  rtError headers(rtObjectRef& v) const
  {
    v = mHeaders;
    return RT_OK;
  }

  rtError addListener(rtString eventName, const rtFunctionRef& f) { mEmit->addListener(eventName, f); return RT_OK;  }

  static void onDownloadComplete(rtFileDownloadRequest* downloadRequest);
  // decide in future whether to use this or not
  // static size_t onDownloadInProgress(void *ptr, size_t size, size_t nmemb, void *userData);
  /* this has to invoke callback function and emit data,end messages */
  /* this has to know http response object to emit events, callback function */
  static void onDownloadCompleteUI(void* context, void* data);

private:
  /* populate headers from the download request */
  void populateHeaders(rtFileDownloadRequest*);
  int32_t mStatusCode;
  rtString mErrorMessage;
  rtEmitRef mEmit;
  rtString mData;
  rtObjectRef mHeaders;
  rtData mHeaderMetaData;
};

#endif
