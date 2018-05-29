#include "rtJsModules.h"

static std::vector<rtRef<rtFunctionCallback> > gBindings;
extern rtThreadQueue gUIThreadQueue;

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

rtDefineObject(rtHttpResponse, rtObject);
rtDefineProperty(rtHttpResponse, statusCode);
rtDefineProperty(rtHttpResponse, message);
rtDefineProperty(rtHttpResponse, headers);
rtDefineMethod(rtHttpResponse, addListener);


void handleRequest(struct httpRequest* req)
{
  rtFileDownloadRequest *downloadRequest = new rtFileDownloadRequest(req->mUrl, req);
  downloadRequest->setCallbackFunction(rtHttpResponse::onDownloadComplete);
  //downloadRequest->setDownloadProgressCallbackFunction(rtHttpResponse::onDownloadInProgress, req);
  rtFileDownloader::instance()->addToDownloadQueue(downloadRequest);
}

void rtHttpResponse::onDownloadComplete(rtFileDownloadRequest* downloadRequest)
{
  struct httpRequest* request = (struct httpRequest*) downloadRequest->callbackData();
  request->mJsonResponse = downloadRequest->downloadedData(); 
  rtHttpResponse* httpResponse = (rtHttpResponse*)request->mHttpResponse.getPtr();
  httpResponse->populateHeaders(downloadRequest);
  //may need to populate status code here
  httpResponse->mStatusCode = downloadRequest->httpStatusCode();
  httpResponse->mErrorMessage = downloadRequest->errorString();
  gUIThreadQueue.addTask(onDownloadCompleteUI, (void *)httpResponse, (void*)request);
}

void rtHttpResponse::onDownloadCompleteUI(void* context, void* data)
{
  struct httpRequest* request = (struct httpRequest*)data;
  rtHttpResponse* httpResponse = (rtHttpResponse*)request->mHttpResponse.getPtr();
  request->mCallbackFunction.send(request->mHttpResponse);
  request->mCallbackFunction = NULL;
  httpResponse->mEmit.send("data", (const char*)request->mJsonResponse, request->mJsonResponse.length());
  rtObjectRef e = new rtMapObject;
  httpResponse->mEmit.send(httpResponse->mErrorMessage.isEmpty() ? "end" : "error", e);
  httpResponse->clearResources();
  request->mCallbackFunction = NULL;
  request->mHttpResponse = NULL;
  //delete request->mHttpResponse;
  //delete request;
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
//  args[1].toFunction().send(resp);
  struct httpRequest* req = new httpRequest();
  req->mUrl = args[0].toString();
  req->mHttpResponse = resp;
  req->mCallbackFunction = args[1].toFunction();
  handleRequest(req);
  //*result = resp;
  return RT_OK;
}

void rtHttpResponse::populateHeaders(rtFileDownloadRequest* request)
{
  if (request->headerData() != NULL)
  { 
    mHeaderMetaData.init((uint8_t*)request->headerData(), request->headerDataSize());
  }
  size_t pos=0,prevpos = 0;
  std::string headerString((char*)mHeaderMetaData.data());
  pos = headerString.find_first_of("\n",0);
  std::string attribute("");
  while (pos !=  std::string::npos)
  {
    attribute = headerString.substr(prevpos,pos-prevpos);
    if (attribute.size() >  0)
    {
      //parsing the header attribute and value pair
      std::string key(""),value("");
      size_t name_end_pos = attribute.find_first_of(":");
      if (name_end_pos == std::string::npos)
      {
        key = attribute;
      }
      else
      {
        key = attribute.substr(0,name_end_pos);
      }
      size_t cReturn_nwLnPos  = key.find_first_of("\r");
      if (std::string::npos != cReturn_nwLnPos)
        key.erase(cReturn_nwLnPos,1);
      cReturn_nwLnPos  = key.find_first_of("\n");
      if (std::string::npos != cReturn_nwLnPos)
        key.erase(cReturn_nwLnPos,1);
      if (name_end_pos == std::string::npos)
      {
        if (key.size() > 0)
        {
          mHeaders.set(key.c_str(),rtString(""));
        }
      }
      else
      {
       value = attribute.substr(name_end_pos+1,attribute.length());
       cReturn_nwLnPos  = value.find_first_of("\r");
       if (std::string::npos != cReturn_nwLnPos)
         value.erase(cReturn_nwLnPos,1);
       cReturn_nwLnPos  = value.find_first_of("\n");
       if (std::string::npos != cReturn_nwLnPos)
         value.erase(cReturn_nwLnPos,1);
       if (key.size() > 0)
       {
         mHeaders.set(key.c_str(),value.c_str());
       }
      }
    }
    prevpos = pos+1;
    pos = headerString.find_first_of("\n",prevpos);
  }
}
