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

rtError rtProxyHasFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyGetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxySetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyDeleteFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyTestArrayReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyTestMapReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyTestObjectReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);

void rtSetupJsModuleBindings(duk_context *ctx)
{
  rtRegisterJsBinding(ctx, "httpGet", &rtHttpGetBinding);
  rtRegisterJsBinding(ctx, "_hasProxyFunc", &rtProxyHasFuncBinding);
  rtRegisterJsBinding(ctx, "_getProxyFunc", &rtProxyGetFuncBinding);
  rtRegisterJsBinding(ctx, "_setProxyFunc", &rtProxySetFuncBinding);
  rtRegisterJsBinding(ctx, "_deleteProxyFunc", &rtProxyDeleteFuncBinding);
  rtRegisterJsBinding(ctx, "_testArrayReturnFunc", &rtProxyTestArrayReturnBinding);
  rtRegisterJsBinding(ctx, "_testMapReturnFunc", &rtProxyTestMapReturnBinding);
  rtRegisterJsBinding(ctx, "_testObjectReturnFunc", &rtProxyTestObjectReturnBinding);
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

rtError rtProxyHasFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 2) {
       return RT_ERROR_INVALID_ARG;
   }

   bool res = false;

   if (args[0].getType() == RT_objectType) {
       rtObjectRef obj = args[0].toObject();
       rtIObject* objPtr = obj.getPtr();
       rtValue val;
       if (args[1].getType() == RT_stringType) {
           rtString key;
           args[1].getString(key);
           res = objPtr->Get(key.cString(), &val) != RT_PROP_NOT_FOUND;
       } else {
           int32_t key;
           args[1].getInt32(key);
           res = objPtr->Get(key, &val) != RT_PROP_NOT_FOUND;
       }
   }

   *result = rtValue(res);
   return RT_OK;
}

rtError rtProxyGetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 2) {
       return RT_ERROR_INVALID_ARG;
   }

   rtError err = RT_PROP_NOT_FOUND;
   rtValue val;

   if (args[0].getType() == RT_objectType) {
       rtObjectRef obj = args[0].toObject();
       rtIObject* objPtr = obj.getPtr();
       if (args[1].getType() == RT_stringType) {
           rtString key;
           args[1].getString(key);
           err = objPtr->Get(key.cString(), &val);
       } else {
           int32_t key;
           args[1].getInt32(key);
           err = objPtr->Get(key, &val);
       }
   }

   *result = val;
   return err;
}

rtError rtProxySetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 3) {
       return RT_ERROR_INVALID_ARG;
   }

   rtError err = RT_PROP_NOT_FOUND;

   if (args[0].getType() == RT_objectType) {
       rtObjectRef obj = args[0].toObject();
       rtIObject* objPtr = obj.getPtr();
       if (args[1].getType() == RT_stringType) {
           rtString key;
           args[1].getString(key);
           err = objPtr->Set(key.cString(), &args[2]);
       } else {
           int32_t key;
           args[1].getInt32(key);
           err = objPtr->Set(key, &args[2]);
       }
   }

   *result = rtValue(err != RT_PROP_NOT_FOUND);

   return err;
}


rtError rtProxyDeleteFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    return RT_OK;
}

rtError rtProxyTestArrayReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    rtArrayObject* ret = new rtArrayObject();

    ret->pushBack(rtValue(1));
    ret->pushBack(rtValue(2));
    ret->pushBack(rtValue(3));

    *result = ret;

    return RT_OK;
}

rtError rtProxyTestMapReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    rtMapObject* ret = new rtMapObject();

    rtValue v1(1);
    ret->Set("a1", &v1);
    rtValue v2(2);
    ret->Set("a2", &v2);
    rtValue v3(3);
    ret->Set("a3", &v3);

    *result = ret;

    return RT_OK;
}

class rtProxyTestObject : public rtObject
{
public:
   rtDeclareObject(rtProxyTestObject, rtObject);
   rtProperty(propA, propA, setPropA, int);
   rtReadOnlyProperty(propB, propB, int);
   rtProperty(propC, propC, setPropC, rtFunctionRef);
   rtMethodNoArgAndNoReturn("methodA", methodA);
   rtMethodNoArgAndReturn("methodB", methodB, int);
   rtMethod1ArgAndReturn("methodC", methodC, rtString, rtString);

   rtProxyTestObject() : mPropA(1), mPropB(2) {}

   rtError propA(int& v) const { v = mPropA;  return RT_OK; }
   rtError setPropA(int v) { mPropA = v;  return RT_OK; }
   rtError propB(int& v) const { v = mPropB;  return RT_OK; }
   rtError propC(rtFunctionRef& v) const { v = mPropC;  return RT_OK; }
   rtError setPropC(rtFunctionRef v) { mPropC = v;  return RT_OK; }

   rtError methodA() { return RT_OK; }
   rtError methodB(int &b) { b = 123; return RT_OK; }
   rtError methodC(const rtString &in1, rtString &out1) { out1 = in1; return RT_OK; }

private:
   int mPropA;
   int mPropB;
   rtFunctionRef mPropC;
};

rtDefineObject(rtProxyTestObject, rtObject);
rtDefineProperty(rtProxyTestObject, propA);
rtDefineProperty(rtProxyTestObject, propB);
rtDefineProperty(rtProxyTestObject, propC);
rtDefineMethod(rtProxyTestObject, methodA);
rtDefineMethod(rtProxyTestObject, methodB);
rtDefineMethod(rtProxyTestObject, methodC);

rtError rtProxyTestObjectReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    rtProxyTestObject* ret = new rtProxyTestObject();

    *result = ret;

    return RT_OK;
}

