#include "rtJsModules.h"
#include "rtWrapperUtils.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "rtObject.h"
#include "rtFileDownloader.h"

#include <vector>
#include <algorithm>
#include <string>
#include <map>
#include <ctype.h>

extern "C"
{
#include "zlib.h"
#include "duv.h"
}

#include <curl/curl.h>

#include "sha1.hpp"
#include "base64.hpp"

#ifdef WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#undef min
#undef max
#else
#include <sys/select.h>
#endif

static std::vector<rtRef<rtFunctionCallback> > gBindings;

static void rtRegisterJsBinding(duk_context *ctx, const char *name, rtFunctionCB cb)
{
  rtRef<rtFunctionCallback> cbObj(new rtFunctionCallback(cb, NULL));
  gBindings.push_back(cbObj);

  rt2duk(ctx, rtValue(cbObj.getPtr()));
  rtDukPutIdentToGlobal(ctx, name);
}

rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtHttpRequestBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtCreateInflateRawBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtCreateDeflateRawBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtCreateCryptoHashBinding(int numArgs, const rtValue* args, rtValue* result, void* context);

rtError rtProxyHasFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyGetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxySetFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtProxyDeleteFuncBinding(int numArgs, const rtValue* args, rtValue* result, void* context);

static void rtWebSocketManagerProc(void *data);

void rtSetupJsModuleBindings(duk_context *ctx)
{
  rtRegisterJsBinding(ctx, "httpGet", &rtHttpGetBinding);
  rtRegisterJsBinding(ctx, "_httpRequest", &rtHttpRequestBinding);
  rtRegisterJsBinding(ctx, "_createInflateRaw", &rtCreateInflateRawBinding);
  rtRegisterJsBinding(ctx, "_createDeflateRaw", &rtCreateDeflateRawBinding);
  rtRegisterJsBinding(ctx, "_createCryptoHash", &rtCreateCryptoHashBinding);

  rtRegisterJsBinding(ctx, "_hasProxyFunc", &rtProxyHasFuncBinding);
  rtRegisterJsBinding(ctx, "_getProxyFunc", &rtProxyGetFuncBinding);
  rtRegisterJsBinding(ctx, "_setProxyFunc", &rtProxySetFuncBinding);
  rtRegisterJsBinding(ctx, "_deleteProxyFunc", &rtProxyDeleteFuncBinding);

  rtThreadTask* task = new rtThreadTask(rtWebSocketManagerProc, NULL, "");
  rtThreadPool* mainThreadPool = rtThreadPool::globalInstance();
  mainThreadPool->executeTask(task);
}

class rtHttpResponse : public rtObject
{
public:
  rtDeclareObject(rtHttpResponse, rtObject);
  rtReadOnlyProperty(statusCode, statusCode, int32_t);
  rtReadOnlyProperty(message, errorMessage, rtString);
  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtHttpResponse() : mStatusCode(0) 
  {
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

class rtWsSocket;

class rtWsSocketManager 
{
public:
   void add(rtWsSocket *sock);
   void remove(rtWsSocket *sock);
   void poll();

   static rtWsSocketManager &get()
   {
      static rtWsSocketManager mng;
      return mng;
   }

private:
   int find(rtWsSocket *sock);

private:
   std::vector<rtWsSocket *> mSockets;
};

void rtWebSocketManagerProc(void *data)
{
   for (;;) {
      rtWsSocketManager::get().poll();
   }
}

class rtWsSocket : public rtObject
{
public:
   rtDeclareObject(rtWsSocket, rtObject);
   rtMethod2ArgAndReturn("on", onFunc, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("addListener", addListener, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("removeListener", removeListener, rtString, rtFunctionRef, rtObjectRef);
   rtMethodNoArgAndNoReturn("end", endFunc);
   rtMethodNoArgAndNoReturn("destroy", destroyFunc);
   rtMethodNoArgAndNoReturn("close", closeFunc);
   rtMethodNoArgAndNoReturn("pause", pauseFunc);
   rtMethodNoArgAndNoReturn("resume", resumeFunc);
   rtMethod1ArgAndNoReturn("setTimeout", setTimeout, int32_t);
   rtMethod1ArgAndNoReturn("setNoDelay", setNoDelay, bool);
   rtMethod2ArgAndNoReturn("write", write, rtObjectRef, rtString);
   rtMethod3ArgAndNoReturn("write2", write2, rtObjectRef, rtString, rtFunctionRef);

   rtWsSocket(duk_context *ctx, CURL *handle): mCtx(ctx), mHandle(handle)
   {
      mEmit = new rtEmit();
   }

   rtError onFunc(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { return addListener(eventName, f, objRef); }
   rtError addListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->addListener(eventName, f); objRef = this; return RT_OK; }
   rtError removeListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->delListener(eventName, f); objRef = this; return RT_OK; }

   rtError endFunc();
   rtError destroyFunc();
   rtError closeFunc();
   rtError pauseFunc() { return RT_OK; }
   rtError resumeFunc() { return RT_OK; }

   rtError setTimeout(int32_t timeout) { return RT_OK; }
   rtError setNoDelay(bool arg) { return RT_OK; }

   rtError write(rtObjectRef data, const rtString &encoding);
   rtError write2(rtObjectRef data, const rtString &encoding, rtFunctionRef cb);

   CURL           *getCurlHandle() { return mHandle; }
   curl_socket_t   getFd();

   void            onSocketDataReady();

private:
   rtEmitRef mEmit;
   duk_context *mCtx;
   CURL *mHandle;
};

rtDefineObject(rtWsSocket, rtObject);
rtDefineMethod(rtWsSocket, onFunc);
rtDefineMethod(rtWsSocket, addListener);
rtDefineMethod(rtWsSocket, removeListener);
rtDefineMethod(rtWsSocket, endFunc);
rtDefineMethod(rtWsSocket, closeFunc);
rtDefineMethod(rtWsSocket, destroyFunc);
rtDefineMethod(rtWsSocket, pauseFunc);
rtDefineMethod(rtWsSocket, resumeFunc);
rtDefineMethod(rtWsSocket, setTimeout);
rtDefineMethod(rtWsSocket, setNoDelay);
rtDefineMethod(rtWsSocket, write);
rtDefineMethod(rtWsSocket, write2);

rtError rtWsSocket::endFunc()
{
   rtWsSocketManager::get().remove(this);

   curl_easy_cleanup(mHandle);
   mHandle = NULL;
   return RT_OK;
}

rtError rtWsSocket::destroyFunc()
{
   rtWsSocketManager::get().remove(this);

   curl_easy_cleanup(mHandle);
   mHandle = NULL;
   return RT_OK;
}

rtError rtWsSocket::closeFunc()
{
   mEmit.send("close");
   return RT_OK;
}

curl_socket_t rtWsSocket::getFd()
{
   long sd = -1;
   int res = curl_easy_getinfo(mHandle, CURLINFO_LASTSOCKET, &sd);
   assert(res == CURLE_OK);
   return (curl_socket_t)sd;
}

void rtWsSocket::onSocketDataReady()
{
   size_t n;
   CURLcode rc;
   char buf[16384];

   while (1) {
      rc = curl_easy_recv(mHandle, buf, sizeof(buf), &n);

      if (rc != CURLE_AGAIN) {
         break;
      }
   }

   if (rc == CURLE_OK && n > 0) {
      duk_push_fixed_buffer(mCtx, n);
      void *bufdata = duk_get_buffer_data(mCtx, -1, NULL);
      memcpy(bufdata, buf, n);
      duk_push_buffer_object(mCtx, -1, 0, n, DUK_BUFOBJ_NODEJS_BUFFER);

      mEmit.send("data", duk2rt(mCtx, NULL));

      duk_pop_2(mCtx);

   } else if (rc != CURLE_OK && rc != CURLE_AGAIN) {
      mEmit.send("error", curl_easy_strerror(rc));
   } else if (rc == CURLE_OK && n == 0) {
      mEmit.send("close");
   }
}

rtError rtWsSocket::write(rtObjectRef obj, const rtString &encoding)
{
   if (encoding != "binary") {
      return RT_FAIL;
   }

   rt2duk(mCtx, obj);

   if (!duk_is_buffer_data(mCtx, -1)) {
      duk_pop(mCtx);
      return RT_FAIL;
   }

   duk_size_t size;
   char *data = (char*)duk_get_buffer_data(mCtx, -1, &size);

   assert(data != NULL && size > 0);
   assert(mHandle != NULL);

   size_t written = 0;
   CURLcode rc = CURLE_OK;

   do {
      rc = curl_easy_send(mHandle, data, size, &written);
   } while (rc == CURLE_AGAIN);

   assert(rc == CURLE_OK);
   assert(written == size);

   return RT_OK;
}

rtError rtWsSocket::write2(rtObjectRef data, const rtString &encoding, rtFunctionRef cb)
{
   rtError res = write(data, encoding);
   if (cb.ptr() != NULL) {
    cb.send(rtValue((int32_t)res));
   }
   return res;
}

class rtHttpWsHandshake : public rtObject 
{
public:
   rtDeclareObject(rtHttpWsHandshake, rtObject);
   rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
   rtMethodNoArgAndNoReturn("removeAllListeners", removeAllListeners);
   rtMethodNoArgAndNoReturn("end", endFunc);
   rtMethodNoArgAndNoReturn("onUpgrade", onUpgrade);

   rtHttpWsHandshake(duk_context *ctx, const rtString &url, rtObjectRef headers) 
      : mCtx(ctx), mMultiHandle(NULL), mHandle(NULL) 
   {
      mEmit = new rtEmit();
      mUrl = url;
      mHandshakeDone = false;
      mWsSock = NULL;
      initHeaders(headers);
   }

   rtError endFunc();

   rtError addListener(rtString eventName, const rtFunctionRef& f) { mEmit->addListener(eventName, f); return RT_OK;  }
   rtError removeAllListeners() { return RT_OK; }

   rtError onUpgrade();

private:
   void initHeaders(rtObjectRef headers);
   static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata);
   void onHandshakeDone() { mHandshakeDone = true; }
   void onHeaderCallback(const std::string &s) { mRawHeaders += s; }

private:
   duk_context *mCtx;
   rtEmitRef mEmit;
   rtString mUrl;
   std::vector<std::string> mReqHeaders;
   CURLM *mMultiHandle;
   CURL  *mHandle;
   bool mHandshakeDone;
   std::string mRawHeaders;
   rtObjectRef mpObj;
   rtWsSocket *mWsSock;
};

rtDefineObject(rtHttpWsHandshake, rtObject);
rtDefineMethod(rtHttpWsHandshake, addListener);
rtDefineMethod(rtHttpWsHandshake, removeAllListeners);
rtDefineMethod(rtHttpWsHandshake, endFunc);
rtDefineMethod(rtHttpWsHandshake, onUpgrade);

void rtHttpWsHandshake::initHeaders(rtObjectRef headers)
{
   rtObjectRef keys = headers.get<rtObjectRef>("allKeys");
   if (keys) {
      mReqHeaders.clear();
      uint32_t len = keys.get<uint32_t>("length");
      for (uint32_t i = 0; i < len; i++) {
         rtString key = keys.get<rtString>(i);
         rtString val = headers.get<rtString>(key.cString());
         mReqHeaders.push_back(std::string(key.cString()) + ": " + std::string(val.cString()));
      }
   }
}

size_t rtHttpWsHandshake::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata)
{
   rtHttpWsHandshake *ws = (rtHttpWsHandshake*)userdata;

   // if last header
   if (nitems * size == 2) {
      curl_easy_setopt(ws->mHandle, CURLOPT_CONNECT_ONLY, 1);
      curl_easy_setopt(ws->mHandle, CURLOPT_FORBID_REUSE, 1);
      curl_easy_pause(ws->mHandle, CURLPAUSE_ALL);
      ws->onHandshakeDone();
   } else {
      ws->onHeaderCallback(std::string(buffer, size * nitems));
   }
   return nitems * size;
}

static const int notokenSymbols[256] = 
{
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
   1, 0, 1, 0, 0, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
   1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
   0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
};


static int parseHeaders(std::string &headers, std::map<std::string, std::string> &parsedHeaders) 
{
   char *p = (char *)headers.c_str();
   char *end = p + headers.size();
   char *header_key_begin, *header_value_begin;

   // skip first header line
   while (p < end && !(*(p-1) == '\r' && *p == '\n')) { ++p; }

   p++;

   while (p < end) {
      /* skipping possible whitespace */
      while (p < end && (*p == ' ' || *p == '\t')) { ++p; }
      header_key_begin = p;

      /* reading token */
      while (p < end && !notokenSymbols[(int)(unsigned char)*p]) { ++p; }

      if (p == end || *p != ':') {
         break;
      }
      *p++ = 0;

      while (p < end && (*p == ' ' || *p == '\t')) { ++p; }
      
      header_value_begin = p;

      while (p < end && !(*(p-1) == '\r' && *p == '\n')) { ++p; }
      
      if (p < end) {
         p[-1] = 0;
         *p++ = 0;
      }

      parsedHeaders[header_key_begin] = header_value_begin;
   }
   return 0;
}

void rtWsSocketManager::add(rtWsSocket *sock)
{
   if (find(sock) != -1) 
      return;
   mSockets.push_back(sock);
}

void rtWsSocketManager::remove(rtWsSocket *sock)
{
   int idx = find(sock);
   if (idx != -1) {
      std::swap(mSockets[idx], mSockets.back());
      mSockets.pop_back();
   }
}

int rtWsSocketManager::find(rtWsSocket *sock)
{
   for (int i = 0; i < mSockets.size(); ++i) {
      if (mSockets[i] == sock) {
         return i;
      }
   }
   return -1;
}

void rtWsSocketManager::poll()
{
   fd_set readFds;
   curl_socket_t maxFd = 0;

   FD_ZERO(&readFds);

   std::vector<rtWsSocket *> socketsCopy = mSockets;

   for (int i = 0; i < socketsCopy.size(); ++i) {
      curl_socket_t fd = socketsCopy[i]->getFd();
      maxFd = std::max(fd, maxFd);
      FD_SET(fd, &readFds);
   }

   struct timeval tval = { 0, 10000 };
   select(maxFd + 1, &readFds, NULL, NULL, &tval);

   for (int i = 0; i < socketsCopy.size(); ++i) {
      if (find(socketsCopy[i]) == -1) {
         continue;
      }
      curl_socket_t fd = socketsCopy[i]->getFd();
      if (FD_ISSET(fd, &readFds)) {
         mSockets[i]->onSocketDataReady();
      }
   }
}

static std::string toLowerString(const std::string &c)
{
   std::string res;
   for (int i = 0; i < c.size(); ++i) {
      res += tolower(c[i]);
   }
   return res;
}

rtError rtHttpWsHandshake::endFunc()
{
   mMultiHandle = curl_multi_init();
   mHandle = curl_easy_init();

   curl_easy_setopt(mHandle, CURLOPT_URL, mUrl.cString());
   curl_easy_setopt(mHandle, CURLOPT_MAXCONNECTS, 1L);
   curl_easy_setopt(mHandle, CURLOPT_HEADERFUNCTION, headerCallback);
   curl_easy_setopt(mHandle, CURLOPT_HEADERDATA, (void*)this);
   //curl_easy_setopt(mHandle, CURLOPT_VERBOSE, 1L);
   curl_easy_setopt(mHandle, CURLOPT_TCP_NODELAY, 0L);
   curl_easy_setopt(mHandle, CURLOPT_TIMEOUT, 0);

   if (!mReqHeaders.empty()) {
      struct curl_slist *chunk = NULL;
      for (int i = 0; i < mReqHeaders.size(); ++i) {
         chunk = curl_slist_append(chunk, mReqHeaders[i].c_str());
      }
      curl_easy_setopt(mHandle, CURLOPT_HTTPHEADER, chunk);
   }

   curl_multi_add_handle(mMultiHandle, mHandle);
   
   while (!mHandshakeDone) {
      int runningHandles = 0;
      CURLMcode  rc = curl_multi_perform(mMultiHandle, &runningHandles);
      if (rc != CURLM_OK && rc != CURLM_CALL_MULTI_PERFORM) {
         mEmit.send("error", rtValue(curl_multi_strerror(rc)));
         return RT_FAIL;
      }
   }

   assert(mHandshakeDone);

   mpObj = new rtMapObject();
   std::map<std::string, std::string> mp;
   if (parseHeaders(mRawHeaders, mp) == 0) {
      for (std::map<std::string, std::string>::iterator it = mp.begin(); it != mp.end(); ++it) {
         mpObj.set(toLowerString((*it).first).c_str(), rtValue((*it).second.c_str()));
      }
   }

   mWsSock = new rtWsSocket(mCtx, mHandle);
   rtWsSocketManager::get().add(mWsSock);

//#ifdef  USE_UV_TIMERS
   // need defer execution for next event loop
   /*
   uv_timer_t *timerObj = new uv_timer_t();
   uv_timer_init(uv_default_loop(), timerObj);
   g_pendingHandshakes.push_back(this);
   uv_timer_start(timerObj, rtProcessHandshakesCallback, 0, 0);
   */
//#else
   //onUpgrade();
//#endif

   return RT_OK;
}

rtError rtHttpWsHandshake::onUpgrade()
{
   assert(mWsSock != NULL);
   mEmit.send("upgrade", mpObj, rtObjectRef(mWsSock));
   return RT_OK;
}

rtError rtHttpRequestBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 1) {
      return RT_ERROR_INVALID_ARG;
   }

   if (args[0].getType() != RT_objectType) {
      return RT_ERROR_INVALID_ARG;
   }

   duk_context *ctx = (duk_context *)args[numArgs].toVoidPtr();

   rtValue val;
   rtValue headers;

   bool isWebSocket = false;

   if (args[0].toObject().get("headers", headers) == RT_OK) {
      if (headers.toObject().get("Upgrade", val) == RT_OK && val.toString() == "websocket") {
         isWebSocket = true;
      }
   }

   if (!isWebSocket) {
      // for now only websocket is supported
      return RT_FAIL;
   }

   rtValue url;
   if (args[0].toObject().get("url", url) != RT_OK || url.getType() != RT_stringType) {
      return RT_FAIL;
   }

   rtObjectRef requestObj(new rtHttpWsHandshake(ctx, url.toString(), headers.toObject()));
   *result = requestObj;

   return RT_OK;
}

class rtInflateRaw : public rtObject
{
public:
   rtDeclareObject(rtInflateRaw, rtObject);
   rtProperty(writeInProgress, writeInProgress, setWriteInProgress, bool);
   rtProperty(pendingClose, pendingClose, setPendingClose, bool);

   rtMethod2ArgAndReturn("on", onFunc, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("addListener", addListener, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("removeListener", removeListener, rtString, rtFunctionRef, rtObjectRef);

   rtMethod1ArgAndNoReturn("write", write, rtObjectRef);
   rtMethod1ArgAndNoReturn("flush", flush, rtFunctionRef);
   rtMethodNoArgAndNoReturn("close", close);

   rtInflateRaw(duk_context *ctx, int windowBits) : mCtx(ctx), mWriteInProgress(false), mPendingClose(false) 
   {
      mEmit = new rtEmit();
      init(windowBits);
   }

   rtError onFunc(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { return addListener(eventName, f, objRef); }
   rtError addListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->addListener(eventName, f); objRef = this; return RT_OK; }
   rtError removeListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->delListener(eventName, f); objRef = this; return RT_OK; }

   rtError writeInProgress(bool& b) const { b = mWriteInProgress; return RT_OK; }
   rtError setWriteInProgress(bool b) { mWriteInProgress = b; return RT_OK; }

   rtError pendingClose(bool& b) const { b = mPendingClose; return RT_OK; }
   rtError setPendingClose(bool b) { mPendingClose = b; return RT_OK; }

   rtError write(rtObjectRef obj);
   rtError close();
   rtError flush(rtFunctionRef fRef);

private:
   void init(int windowBits);
   void onData(void *data, int dataSz);

private:
   duk_context *mCtx;
   rtEmitRef mEmit;
   bool mWriteInProgress;
   bool mPendingClose;
   z_stream mZStream;
   std::vector<unsigned char> mInBuf;
   std::vector<unsigned char> mOutBuf;
   int mInBufPos, mInBufSz;
   int mOutBufSz;
};

rtDefineObject(rtInflateRaw, rtObject);
rtDefineProperty(rtInflateRaw, writeInProgress);
rtDefineProperty(rtInflateRaw, pendingClose);
rtDefineMethod(rtInflateRaw, onFunc);
rtDefineMethod(rtInflateRaw, addListener);
rtDefineMethod(rtInflateRaw, removeListener);
rtDefineMethod(rtInflateRaw, write);
rtDefineMethod(rtInflateRaw, flush);
rtDefineMethod(rtInflateRaw, close);

void rtInflateRaw::init(int windowBits)
{
   const int bufSize = 16384;
   mInBuf.resize(bufSize);
   mOutBuf.resize(bufSize);
   mInBufPos = 0;
   mInBufSz = mOutBufSz = 0;
   memset(&mZStream, 0, sizeof(mZStream));
   inflateInit2(&mZStream, windowBits);
}

void rtInflateRaw::onData(void *data, int dataSz)
{
   duk_push_fixed_buffer(mCtx, dataSz);
   void *bufdata = duk_get_buffer_data(mCtx, -1, NULL);
   memcpy(bufdata, data, dataSz);
   duk_push_buffer_object(mCtx, -1, 0, dataSz, DUK_BUFOBJ_NODEJS_BUFFER);

   mEmit.send("data", duk2rt(mCtx, NULL));

   duk_pop_2(mCtx);
}

rtError rtInflateRaw::write(rtObjectRef obj)
{
   rt2duk(mCtx, obj);

   if (!duk_is_buffer_data(mCtx, -1)) {
      duk_pop(mCtx);
      return RT_FAIL;
   }

   duk_size_t size;
   char *data = (char*)duk_get_buffer_data(mCtx, -1, &size);

   if (data == NULL) {
      duk_pop(mCtx);
      return RT_FAIL;
   }

   while (size != 0) {

      if (mInBufPos > 0) {
         if (mInBufSz > 0) {
            memmove(&mInBuf[0], &mInBuf[mInBufPos], mInBufSz);
         }
         mInBufPos = 0;
      }

      int inAvail = mInBuf.size() - mInBufSz;
      int toCopy = std::min(inAvail, (int)size);

      assert(toCopy > 0);

      memcpy(&mInBuf[mInBufSz], data, toCopy);

      data += toCopy;
      size -= toCopy;
      mInBufSz += toCopy;

      mZStream.next_in = &mInBuf[0];
      mZStream.avail_in = mInBufSz;
      mZStream.next_out = &mOutBuf[mOutBufSz];
      mZStream.avail_out = mOutBuf.size() - mOutBufSz;

      int rc = inflate(&mZStream, Z_NO_FLUSH);
      
      mInBufPos = mZStream.next_in - &mInBuf[0];
      mInBufSz = mZStream.avail_in;

      if (rc != Z_OK && rc != Z_STREAM_END) {
         mEmit.send("error", rtValue(rc));
         return RT_FAIL;
      }

      mOutBufSz = mZStream.next_out - &mOutBuf[0];

      assert(mOutBufSz > 0);

      onData(&mOutBuf[0], mOutBufSz);

      mOutBufSz = 0;
   }

   duk_pop(mCtx);
   return RT_OK;
}

rtError rtInflateRaw::flush(rtFunctionRef func)
{
   if (mInBufPos > 0) {
      if (mInBufSz > 0) {
         memmove(&mInBuf[0], &mInBuf[mInBufPos], mInBufSz);
      }
      mInBufPos = 0;
   }

   mZStream.next_in = &mInBuf[0];
   mZStream.avail_in = mInBufSz;
   mZStream.next_out = &mOutBuf[mOutBufSz];
   mZStream.avail_out = mOutBuf.size() - mOutBufSz;

   int rc = inflate(&mZStream, Z_FINISH);

   if (rc != Z_OK && rc != Z_STREAM_END) {
      mEmit.send("error", rtValue(rc));
      return RT_FAIL;
   }

   assert(rc == Z_STREAM_END);

   mOutBufSz = mZStream.next_out - &mOutBuf[0];

   if (mOutBufSz > 0) {
      onData(&mOutBuf[0], mOutBufSz);
   }

   func.send();

   return RT_OK;
}

rtError rtInflateRaw::close()
{
   inflateEnd(&mZStream);
   return RT_OK;
}

rtError rtCreateInflateRawBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 1) {
      return RT_ERROR_INVALID_ARG;
   }

   if (args[0].getType() != RT_objectType) {
      return RT_ERROR_INVALID_ARG;
   }

   int windowBits = 0;

   rtValue val;
   if (args[0].toObject().get("windowBits", val) == RT_OK) {
      windowBits = val.toInt32();
      if (windowBits > 0) {
         windowBits = -windowBits;
      }
   }

   duk_context *ctx = (duk_context *)args[numArgs].toVoidPtr();

   rtObjectRef res(new rtInflateRaw(ctx, windowBits));
   *result = res;

   return RT_OK;
}

class rtDeflateRaw : public rtObject
{
public:
   rtDeclareObject(rtDeflateRaw, rtObject);
   rtProperty(writeInProgress, writeInProgress, setWriteInProgress, bool);
   rtProperty(pendingClose, pendingClose, setPendingClose, bool);

   rtMethod2ArgAndReturn("on", onFunc, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("addListener", addListener, rtString, rtFunctionRef, rtObjectRef);
   rtMethod2ArgAndReturn("removeListener", removeListener, rtString, rtFunctionRef, rtObjectRef);

   rtMethod1ArgAndNoReturn("write", write, rtObjectRef);
   rtMethod1ArgAndNoReturn("flush", flush, rtFunctionRef);
   rtMethodNoArgAndNoReturn("close", close);

   rtDeflateRaw(duk_context *ctx, int windowBits, int memLevel, int flush) 
      : mCtx(ctx), mWriteInProgress(false), mPendingClose(false) 
   {
      mEmit = new rtEmit();
      init(windowBits, memLevel, flush);
   }

   rtError onFunc(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { return addListener(eventName, f, objRef); }
   rtError addListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->addListener(eventName, f); objRef = this; return RT_OK; }
   rtError removeListener(rtString eventName, const rtFunctionRef& f, rtObjectRef &objRef) { mEmit->delListener(eventName, f); objRef = this; return RT_OK; }

   rtError writeInProgress(bool& b) const { b = mWriteInProgress; return RT_OK; }
   rtError setWriteInProgress(bool b) { mWriteInProgress = b; return RT_OK; }

   rtError pendingClose(bool& b) const { b = mPendingClose; return RT_OK; }
   rtError setPendingClose(bool b) { mPendingClose = b; return RT_OK; }

   rtError write(rtObjectRef obj);
   rtError close();
   rtError flush(rtFunctionRef fRef);

private:
   void init(int windowBits, int memLevel, int flush);
   void onData(void *data, int dataSz);

private:
   duk_context *mCtx;
   rtEmitRef mEmit;
   bool mWriteInProgress;
   bool mPendingClose;
   z_stream mZStream;
   std::vector<unsigned char> mInBuf;
   std::vector<unsigned char> mOutBuf;
   int mInBufPos, mInBufSz;
   int mOutBufSz;
   int mFlush;
};

rtDefineObject(rtDeflateRaw, rtObject);
rtDefineProperty(rtDeflateRaw, writeInProgress);
rtDefineProperty(rtDeflateRaw, pendingClose);
rtDefineMethod(rtDeflateRaw, onFunc);
rtDefineMethod(rtDeflateRaw, addListener);
rtDefineMethod(rtDeflateRaw, removeListener);
rtDefineMethod(rtDeflateRaw, write);
rtDefineMethod(rtDeflateRaw, flush);
rtDefineMethod(rtDeflateRaw, close);

void rtDeflateRaw::init(int windowBits, int memLevel, int flush)
{
   const int bufSize = 16586;
   mInBuf.resize(bufSize);
   mOutBuf.resize(bufSize);
   mInBufPos = 0;
   mInBufSz = mOutBufSz = 0;
   memset(&mZStream, 0, sizeof(mZStream));
   deflateInit2(&mZStream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, windowBits, memLevel, Z_DEFAULT_STRATEGY);
   mFlush = flush;
}

void rtDeflateRaw::onData(void *data, int dataSz)
{
   duk_push_fixed_buffer(mCtx, dataSz);
   void *bufdata = duk_get_buffer_data(mCtx, -1, NULL);
   memcpy(bufdata, data, dataSz);
   duk_push_buffer_object(mCtx, -1, 0, dataSz, DUK_BUFOBJ_NODEJS_BUFFER);

   mEmit.send("data", duk2rt(mCtx, NULL));

   duk_pop_2(mCtx);
}

rtError rtDeflateRaw::write(rtObjectRef obj)
{
   rt2duk(mCtx, obj);

   if (!duk_is_buffer_data(mCtx, -1)) {
      duk_pop(mCtx);
      return RT_FAIL;
   }

   duk_size_t size;
   char *data = (char*)duk_get_buffer_data(mCtx, -1, &size);

   if (data == NULL) {
      duk_pop(mCtx);
      return RT_FAIL;
   }

   while (size != 0) {

      if (mInBufPos > 0) {
         if (mInBufSz > 0) {
            memmove(&mInBuf[0], &mInBuf[mInBufPos], mInBufSz);
         }
         mInBufPos = 0;
      }

      int inAvail = mInBuf.size() - mInBufSz;
      int toCopy = std::min(inAvail, (int)size);

      assert(toCopy > 0);

      memcpy(&mInBuf[mInBufSz], data, toCopy);

      data += toCopy;
      size -= toCopy;
      mInBufSz += toCopy;

      mZStream.next_in = &mInBuf[0];
      mZStream.avail_in = mInBufSz;
      mZStream.next_out = &mOutBuf[mOutBufSz];
      mZStream.avail_out = mOutBuf.size() - mOutBufSz;

      int rc = deflate(&mZStream, mFlush);

      mInBufPos = mZStream.next_in - (unsigned char *)&mInBuf[0];
      mInBufSz = mZStream.avail_in;

      if (rc != Z_OK && rc != Z_STREAM_END) {
         mEmit.send("error", rtValue(rc));
         return RT_FAIL;
      }

      mOutBufSz = mZStream.next_out - (unsigned char *)&mOutBuf[0];

      assert(mOutBufSz > 0);

      onData(&mOutBuf[0], mOutBufSz);

      mOutBufSz = 0;
   }

   duk_pop(mCtx);
   return RT_OK;
}

rtError rtDeflateRaw::flush(rtFunctionRef func)
{
   if (mInBufPos > 0) {
      if (mInBufSz > 0) {
         memmove(&mInBuf[0], &mInBuf[mInBufPos], mInBufSz);
      }
      mInBufPos = 0;
   }

   mZStream.next_in = &mInBuf[0];
   mZStream.avail_in = mInBufSz;
   mZStream.next_out = &mOutBuf[mOutBufSz];
   mZStream.avail_out = mOutBuf.size() - mOutBufSz;

   int rc = deflate(&mZStream, Z_FINISH);

   if (rc != Z_OK && rc != Z_STREAM_END) {
      mEmit.send("error", rtValue(rc));
      return RT_FAIL;
   }

   assert(rc == Z_STREAM_END);

   mOutBufSz = mZStream.next_out - (unsigned char *)&mOutBuf[0];

   if (mOutBufSz > 0) {
      onData(&mOutBuf[0], mOutBufSz);
   }

   func.send();

   return RT_OK;
}

rtError rtDeflateRaw::close()
{
   deflateEnd(&mZStream);
   return RT_OK;
}

rtError rtCreateDeflateRawBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 1) {
      return RT_ERROR_INVALID_ARG;
   }

   if (args[0].getType() != RT_objectType) {
      return RT_ERROR_INVALID_ARG;
   }

   duk_context *ctx = (duk_context *)args[numArgs].toVoidPtr();

   int flush = 0;
   int windowBits = 0;
   int memLevel = 0;

   rtValue val;
   if (args[0].toObject().get("flush", val) == RT_OK) {
      flush = val.toInt32();
   }
   if (args[0].toObject().get("windowBits", val) == RT_OK) {
      windowBits = val.toInt32();
      if (windowBits > 0) {
         windowBits = -windowBits;
      }
   }
   if (args[0].toObject().get("memLevel", val) == RT_OK) {
      memLevel = val.toInt32();
   }

   rtObjectRef res(new rtDeflateRaw(ctx, windowBits, memLevel, flush));
   *result = res;

   return RT_OK;
}

class rtCryptoHashSHA1 : public rtObject
{
public:
   rtDeclareObject(rtCryptoHashSHA1, rtObject);
   rtMethod1ArgAndNoReturn("update", update, rtString);
   rtMethod1ArgAndReturn("digest", digest, rtString, rtString);

   rtError update(const rtString &data);
   rtError digest(const rtString &alg, rtString &result);

private:
   SHA1 mSha1;
};

rtDefineObject(rtCryptoHashSHA1, rtObject);
rtDefineMethod(rtCryptoHashSHA1, update);
rtDefineMethod(rtCryptoHashSHA1, digest);

rtError rtCryptoHashSHA1::update(const rtString &data)
{
   mSha1.update(std::string(data.cString(), data.byteLength()));
   return RT_OK;
}

rtError rtCryptoHashSHA1::digest(const rtString &alg, rtString &result)
{
   std::string res = mSha1.final();
   std::string resOut;

   if (alg == "hex") {
      for (int i = 0; i < res.size(); ++i) {
         char c1 = ((res[i] & 0xF0) >> 4) & 0x0F;
         char c2 = (res[i] & 0x0F);
         if (c1 < 10) { resOut += c1 + '0'; }
         else { resOut += c1 - 10 + 'A'; }
         if (c2 < 10) { resOut += c2 + '0'; }
         else { resOut += c2 - 10 + 'A'; }
      }
   }  else {
      assert(alg == "base64");
      Base64::Encode(res, &resOut);
   }

   result = rtString(resOut.c_str());

   return RT_OK;
}

rtError rtCreateCryptoHashBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
   if (numArgs != 1) {
      return RT_ERROR_INVALID_ARG;
   }

   if (args[0].getType() != RT_stringType) {
      return RT_ERROR_INVALID_ARG;
   }

   if (args[0].toString() != "sha1") {
      return RT_FAIL;
   }

   rtObjectRef res(new rtCryptoHashSHA1());
   *result = res;

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
