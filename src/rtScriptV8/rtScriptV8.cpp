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


#if defined WIN32
#include <Windows.h>
#include <direct.h>
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include <uv.h>

#include <string>
#include <fstream>

#include <iostream>
#include <sstream>

#include <unicode/uvernum.h>
#include <unicode/putil.h>
#include <unicode/udata.h>
#include <unicode/uidna.h>

/* if this is defined, we have a 'secondary' entry point.
compare following to utypes.h defs for U_ICUDATA_ENTRY_POINT */
#define SMALL_ICUDATA_ENTRY_POINT \
  SMALL_DEF2(U_ICU_VERSION_MAJOR_NUM, U_LIB_SUFFIX_C_NAME)
#define SMALL_DEF2(major, suff) SMALL_DEF(major, suff)
#ifndef U_LIB_SUFFIX_C_NAME
#define SMALL_DEF(major, suff) icusmdt##major##_dat
#else
#define SMALL_DEF(major, suff) icusmdt##suff##major##_dat
#endif

extern "C" const char U_DATA_API SMALL_ICUDATA_ENTRY_POINT[];

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "rtWrapperUtilsV8.h"
#include "rtJsModulesV8.h"


#ifdef  RT_V8_TEST_BINDINGS
#pragma optimize("", off)
#endif


#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "rtScriptV8.h"

#include "node.h"

#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"
#include "rtPromise.h"
#include "rtFunctionWrapperV8.h"
#include "rtObjectWrapperV8.h"

#include <string>
#include <map>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#pragma GCC diagnostic ignored "-Wall"
#endif

#include  "v8_headers.h"
#include "libplatform/libplatform.h"

#include "rtObjectWrapperV8.h"
#include "rtFunctionWrapperV8.h"
#include "rtWrapperUtilsV8.h"

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

static rtAtomic sNextId = 100;

class V8ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
  virtual void* Allocate(size_t size)
  {
    void *ret = AllocateUninitialized(size);
    if (!ret) {
      return NULL;
    }
    memset(ret, 0, size);
    return ret;
  }
  virtual void* AllocateUninitialized(size_t size)
  {
    return malloc(size);
  }
  virtual void Free(void* data, size_t) { free(data); }
};

V8ArrayBufferAllocator* array_buffer_allocator = NULL;

#if 0
template <class TypeName>
inline v8::Local<TypeName> V8StrongPersistentToLocal(
  const v8::Persistent<TypeName>& persistent) {
  return *reinterpret_cast<v8::Local<TypeName>*>(
    const_cast<v8::Persistent<TypeName>*>(&persistent));
}

template <class TypeName>
inline v8::Local<TypeName> V8WeakPersistentToLocal(
  v8::Isolate* isolate,
  const v8::Persistent<TypeName>& persistent) {
  return v8::Local<TypeName>::New(isolate, persistent);
}

template <class TypeName>
inline v8::Local<TypeName> V8PersistentToLocal(
  v8::Isolate* isolate,
  const v8::Persistent<TypeName>& persistent) {
  if (persistent.IsWeak()) {
    return V8WeakPersistentToLocal(isolate, persistent);
  }
  else {
    return V8StrongPersistentToLocal(persistent);
  }
}
#endif

using namespace rtScriptV8Utils;


class rtV8Context;

typedef rtRef<rtV8Context> rtV8ContextRef;

class rtV8Context: rtIScriptContext  // V8
{
public:
  rtV8Context(v8::Isolate *isolate, v8::Platform* platform, uv_loop_t *loop);

  virtual ~rtV8Context();

  virtual rtError add(const char *name, const rtValue& val);
  virtual rtValue get(const char *name);

  virtual bool    has(const char *name);

  virtual rtError runScript(const char        *script, rtValue* retVal = NULL, const char *args = NULL);
  virtual rtError runFile(const char *file, rtValue* retVal = NULL, const char *args = NULL);

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  Local<Value> loadV8Module(const rtString &name);

private:
  void addMethod(const char *name, v8::FunctionCallback callback, void *data = NULL);
  void setupModuleLoading();
  void setupModuleBindings();

private:
   v8::Isolate                   *mIsolate;
   v8::Platform                  *mPlatform;
   v8::Persistent<v8::Context>    mContext;
   uv_loop_t                     *mUvLoop;

   std::map<rtString, Persistent<Value> *> mLoadedModuleCache;

   rtRef<rtFunctionCallback>      mHttpGetBinding;

   int mRefCount;

   rtAtomic mId;

   const char   *js_file;
   std::string   js_script;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint32_t, rtV8ContextRef> rtV8Contexts;
typedef std::map<uint32_t, rtV8ContextRef>::const_iterator rtV8Contexts_iterator;

class rtScriptV8: public rtIScript
{
public:
  rtScriptV8();
  virtual ~rtScriptV8();

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  rtError init();
  rtError term();

  rtString engine() { return "v8"; }

  rtV8ContextRef createContext();
  rtError createContext(const char *lang, rtScriptContextRef& ctx);

  rtError pump();

  v8::Isolate   *getIsolate() 
  { 
     return mIsolate; 
  }
  v8::Platform   *getPlatform() 
  { 
     return mPlatform; 
  }

  rtError collectGarbage();
  void* getParameter(rtString param);

private:
  v8::Isolate                   *mIsolate;
  v8::Persistent<v8::Context>    mContext;
  v8::Platform                  *mPlatform;
  uv_loop_t                     *mUvLoop;

  bool                           mV8Initialized;

  int mRefCount;
};


using namespace v8;

#ifdef  RT_V8_TEST_BINDINGS

rtError rtTestArrayReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtTestMapReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtTestObjectReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context);

rtError rtTestArrayReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtArrayObject* ret = new rtArrayObject();

  ret->pushBack(rtValue(1));
  ret->pushBack(rtValue(2));
  ret->pushBack(rtValue(3));

  *result = ret;

  return RT_OK;
}

rtError rtTestMapReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
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

class rtTestObject : public rtObject
{
public:
  rtDeclareObject(rtTestObject, rtObject);
  rtProperty(propA, propA, setPropA, int);
  rtReadOnlyProperty(propB, propB, int);
  rtProperty(propC, propC, setPropC, rtFunctionRef);
  rtMethodNoArgAndNoReturn("methodA", methodA);
  rtMethodNoArgAndReturn("methodB", methodB, int);
  rtMethod1ArgAndReturn("methodC", methodC, rtString, rtString);

  rtTestObject() : mPropA(1), mPropB(2) {}

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

rtDefineObject(rtTestObject, rtObject);
rtDefineProperty(rtTestObject, propA);
rtDefineProperty(rtTestObject, propB);
rtDefineProperty(rtTestObject, propC);
rtDefineMethod(rtTestObject, methodA);
rtDefineMethod(rtTestObject, methodB);
rtDefineMethod(rtTestObject, methodC);

rtError rtTestObjectReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtTestObject* ret = new rtTestObject();

  *result = ret;

  return RT_OK;
}

rtError rtTestPromiseReturnResolvedBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtPromise* ret = new rtPromise();

  ret->resolve(2);

  *result = ret;

  return RT_OK;
}

rtError rtTestPromiseReturnRejectedBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtPromise* ret = new rtPromise();

  ret->reject(3);

  *result = ret;

  return RT_OK;
}

static void testPromiseDoWork(uv_work_t* req)
{

}

static void testPromiseDoResolveCallback(uv_work_t* req, int status)
{
  rtPromise *promise = (rtPromise *)req->data;
  promise->resolve(3);
  promise->Release();
}

static void testPromiseDoRejectCallback(uv_work_t* req, int status)
{
  rtPromise *promise = (rtPromise *)req->data;
  promise->reject(promise);
  promise->Release();
}

rtError rtTestPromiseReturnBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtPromise* ret = new rtPromise();

  ret->AddRef();

  uv_work_t *work = new uv_work_t();
  work->data = (void*)ret;

  uv_queue_work(uv_default_loop(), work, &testPromiseDoWork, &testPromiseDoResolveCallback);

  *result = ret;

  return RT_OK;
}

rtError rtTestPromiseReturnRejectBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  rtPromise* ret = new rtPromise();

  ret->AddRef();

  uv_work_t *work = new uv_work_t();
  work->data = (void*)ret;

  uv_queue_work(uv_default_loop(), work, &testPromiseDoWork, &testPromiseDoRejectCallback);

  *result = ret;

  return RT_OK;
}

rtRef<rtFunctionCallback> g_testArrayReturnFunc;
rtRef<rtFunctionCallback> g_testMapReturnFunc;
rtRef<rtFunctionCallback> g_testObjectReturnFunc;
rtRef<rtFunctionCallback> g_testPromiseResolvedReturnFunc;
rtRef<rtFunctionCallback> g_testPromiseReturnFunc;
rtRef<rtFunctionCallback> g_testPromiseRejectedReturnFunc;
rtRef<rtFunctionCallback> g_testPromiseReturnRejectFunc;

#endif

rtV8Context::rtV8Context(Isolate *isolate, Platform *platform, uv_loop_t *loop) :
     mIsolate(isolate), mRefCount(0), mPlatform(platform), mUvLoop(loop)
{
  rtLogInfo(__FUNCTION__);
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  mId = rtAtomicInc(&sNextId);

  // Create a new context.
  Local<Context> localContext = Context::New(mIsolate);

  localContext->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, mId));

  mContext.Reset(mIsolate, localContext); // local to persistent

  Context::Scope contextScope(localContext);

  Handle<Object> global = localContext->Global();
  global->Set(String::NewFromUtf8(mIsolate, "isV8", NewStringType::kNormal).ToLocalChecked(), 
    v8::Integer::New(isolate, 1));
  global->Set(String::NewFromUtf8(mIsolate, "global", NewStringType::kNormal).ToLocalChecked(),
    v8::Object::New(isolate));

  rtObjectWrapper::exportPrototype(mIsolate, global);
  rtFunctionWrapper::exportPrototype(mIsolate, global);

  setupModuleLoading();
  setupModuleBindings();

  v8::platform::PumpMessageLoop(mPlatform, mIsolate);

#ifdef  RT_V8_TEST_BINDINGS
  g_testArrayReturnFunc = new rtFunctionCallback(rtTestArrayReturnBinding);
  g_testMapReturnFunc = new rtFunctionCallback(rtTestMapReturnBinding);
  g_testObjectReturnFunc = new rtFunctionCallback(rtTestObjectReturnBinding);
  g_testPromiseResolvedReturnFunc = new rtFunctionCallback(rtTestPromiseReturnResolvedBinding);
  g_testPromiseReturnFunc = new rtFunctionCallback(rtTestPromiseReturnBinding);
  g_testPromiseRejectedReturnFunc = new rtFunctionCallback(rtTestPromiseReturnRejectedBinding);
  g_testPromiseReturnRejectFunc = new rtFunctionCallback(rtTestPromiseReturnRejectBinding);

  add("_testArrayReturnFunc", g_testArrayReturnFunc.getPtr());
  add("_testMapReturnFunc", g_testMapReturnFunc.getPtr());
  add("_testObjectReturnFunc", g_testObjectReturnFunc.getPtr());
  add("_testPromiseResolvedReturnFunc", g_testPromiseResolvedReturnFunc.getPtr());
  add("_testPromiseReturnFunc", g_testPromiseReturnFunc.getPtr());
  add("_testPromiseRejectedReturnFunc", g_testPromiseRejectedReturnFunc.getPtr());
  add("_testPromiseReturnRejectFunc", g_testPromiseReturnRejectFunc.getPtr());
#endif

  mHttpGetBinding = new rtFunctionCallback(rtHttpGetBinding, loop);

  add("httpGet", mHttpGetBinding.getPtr());
}

rtV8Context::~rtV8Context()
{
  if (!mLoadedModuleCache.empty()) {
    for (auto it = mLoadedModuleCache.begin(); it != mLoadedModuleCache.end(); ++it) {
      delete (*it).second;
    }
  }
}


static bool bloatFileFromPath(uv_loop_t *loop, const rtString &path, rtString &data)
{
  uv_fs_t req;
  int fd = 0;
  uint64_t size;
  char* chunk;
  uv_buf_t buf;

  if (uv_fs_open(loop, &req, path.cString(), 0, 0644, NULL) < 0) {
    goto fail;
  }
  uv_fs_req_cleanup(&req);
  fd = req.result;
  if (uv_fs_fstat(loop, &req, fd, NULL) < 0) {
    goto fail;
  }
  uv_fs_req_cleanup(&req);
  size = req.statbuf.st_size;
  chunk = (char*)malloc(size);
  buf = uv_buf_init(chunk, static_cast<unsigned int>(size));
  if (uv_fs_read(loop, &req, fd, &buf, 1, 0, NULL) < 0) {
    free(chunk);
    goto fail;
  }
  uv_fs_req_cleanup(&req);
  data = rtString(chunk, size);
  free(chunk);
  return true;

fail:
  uv_fs_req_cleanup(&req);
  if (fd) {
    uv_fs_close(loop, &req, fd, NULL);
  }
  uv_fs_req_cleanup(&req);
  return false;
}

static rtString getTryCatchResult(Local<Context> context, const TryCatch &tryCatch)
{
  if (tryCatch.HasCaught()) {
    MaybeLocal<Value> val = tryCatch.StackTrace(context);
    if (val.IsEmpty()) {
      return rtString();
    }
    Local<Value> ret = val.ToLocalChecked();
    String::Utf8Value trace(ret);
    return rtString(*trace);
  }
  return rtString();
}

Local<Value> rtV8Context::loadV8Module(const rtString &name)
{
  rtString path = name;
  if (!name.endsWith(".js")) {
    path.append(".js");
  }

  rtString contents;
  if (!bloatFileFromPath(mUvLoop, path, contents)) {
    rtString path1("v8_modules/");
    path1.append(path.cString());
    path = path1;
    if (!bloatFileFromPath(mUvLoop, path, contents)) {
      rtLogWarn("module '%s' not found", name.cString());
      return Local<Value>();
    }
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  EscapableHandleScope  handle_scope(mIsolate);

  Local<Context> localContext = PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(localContext);

  // check in cache
  if (mLoadedModuleCache.find(path) != mLoadedModuleCache.end()) {
    Persistent<Value> *loadedModule = mLoadedModuleCache[path];
    return  handle_scope.Escape(PersistentToLocal<Value>(mIsolate, *loadedModule));
  }

  rtString contents1 = "(function(){var module=this; var exports=(this.exports = new Object());";
  contents1.append(contents.cString());
  contents1.append(" return this.exports; })");

  v8::Local<v8::String> source =
    v8::String::NewFromUtf8(mIsolate, contents1.cString(), v8::NewStringType::kNormal).ToLocalChecked();

  TryCatch tryCatch(mIsolate);
 
  v8::MaybeLocal<v8::Script> script = v8::Script::Compile(localContext, source);

  if (script.IsEmpty()) {
    rtLogWarn("module '%s' compilation failed (%s)", name.cString(), getTryCatchResult(localContext, tryCatch).cString());
    return  Local<Value>();
  }

  v8::MaybeLocal<v8::Value> result = script.ToLocalChecked()->Run(localContext);

  if (result.IsEmpty() || !result.ToLocalChecked()->IsFunction()) {
    rtLogWarn("module '%s' unexpected result (%s)", name.cString(), getTryCatchResult(localContext, tryCatch).cString());
    return  Local<Value>();
  }

  v8::Function *func = v8::Function::Cast(*result.ToLocalChecked());
  MaybeLocal<v8::Value> ret =  func->Call(localContext, result.ToLocalChecked(), 0, NULL);

  if (ret.IsEmpty()) {
    rtLogWarn("module '%s' unexpected call result (%s)", name.cString(), getTryCatchResult(localContext, tryCatch).cString());
    return  Local<Value>();
  }

  Local<Value> toRet = ret.ToLocalChecked();
  Persistent<Value> *toRetPersistent = new Persistent<Value>(mIsolate, toRet);

  // store in cache
  mLoadedModuleCache[path] = toRetPersistent;

  return handle_scope.Escape(toRet);
}


static void requireCallback(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  assert(args.Data()->IsExternal());
  v8::External *val = v8::External::Cast(*args.Data());
  assert(val != NULL);
  rtV8Context *ctx = (rtV8Context *)val->Value();
  assert(args.Length() == 1);
  assert(args[0]->IsString());

  rtString moduleName = toString(args[0]->ToString());
  args.GetReturnValue().Set(ctx->loadV8Module(moduleName));
}


void rtV8Context::addMethod(const char *name, v8::FunctionCallback callback, void *data)
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  Local<FunctionTemplate> fTemplate = v8::FunctionTemplate::New(mIsolate, callback, v8::External::New(mIsolate, data));
  Local<Context> localContext = PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(localContext);
  localContext->Global()->Set(
    String::NewFromUtf8(mIsolate, name, NewStringType::kNormal).ToLocalChecked(),
    fTemplate->GetFunction()
  );
}

void rtV8Context::setupModuleLoading()
{
  addMethod("require", &requireCallback, (void*)this);
}

void rtV8Context::setupModuleBindings()
{
  for (int i = 0; v8ModuleBindings[i].mName != NULL; ++i) {
    const rtV8FunctionItem &item = v8ModuleBindings[i];
    addMethod(item.mName, item.mCallback, (void*)mUvLoop);
  }
}

rtError rtV8Context::add(const char *name, const rtValue& val)
{
  if (name == NULL) {
    rtLogDebug(" rtNodeContext::add() - no symbolic name for rtValue");
    return RT_FAIL;
  }
  else if (this->has(name)) {
    rtLogDebug(" rtNodeContext::add() - ALREADY HAS '%s' ... over-writing.", name);
    // return; // Allow for "Null"-ing erasure.
  }

  if (val.isEmpty()) {
    rtLogDebug(" rtNodeContext::add() - rtValue is empty");
    return RT_FAIL;
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

                                             // Get a Local context...
  Local<Context> local_context = PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  local_context->Global()->Set(String::NewFromUtf8(mIsolate, name), rt2v8(local_context, val));

  return RT_OK;
}

rtValue rtV8Context::get(const char *name)
{
  if (name == NULL) {
    rtLogError(" rtNodeContext::get() - no symbolic name for rtValue");
    return rtValue();
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

                                             // Get a Local context...
  Local<Context> local_context = PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  // Get the object
  Local<Value> object = global->Get(String::NewFromUtf8(mIsolate, name));

  if (object->IsUndefined() || object->IsNull()) {
    rtLogError("FATAL: '%s' is Undefined ", name);
    return rtValue();
  }
  else {
    rtWrapperError error; // TODO - handle error
    return v82rt(local_context, object, &error);
  }
}

bool rtV8Context::has(const char *name)
{
  if (name == NULL) {
    rtLogError(" rtNodeContext::has() - no symbolic name for rtValue");
    return false;
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

                                             // Get a Local context...
  Local<Context> local_context = PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  TryCatch try_catch(mIsolate);
  Handle<Value> value = global->Get(String::NewFromUtf8(mIsolate, name));

  if (try_catch.HasCaught()) {
    rtLogError("\n ## has() - HasCaught()  ... ERROR");
    return false;
  }

  // No need to check if |value| is empty because it's taken care of
  // by TryCatch above.

  return (!value->IsUndefined() && !value->IsNull());
}

rtError rtV8Context::runScript(const char *script, rtValue* retVal /*= NULL*/, const char *args /*= NULL*/)
{
  rtLogInfo(__FUNCTION__);
  if (!script || strlen(script) == 0) {
    rtLogError(" %s  ... no script given.", __PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  {//scope
    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

                                               // Get a Local context...
    Local<Context> local_context = PersistentToLocal<Context>(mIsolate, mContext);
    Context::Scope context_scope(local_context);
    // !CLF TODO: TEST FOR MT
    TryCatch tryCatch(mIsolate);
    Local<String> source = String::NewFromUtf8(mIsolate, script);

    // Compile the source code.
    Local<Script> run_script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = run_script->Run();
    // !CLF TODO: TEST FOR MT
    if (tryCatch.HasCaught()) {
      String::Utf8Value trace(tryCatch.StackTrace());
      rtLogWarn("%s", *trace);

      return RT_FAIL;
    }

    if (retVal) {
      // Return val
      rtWrapperError error;
      *retVal = v82rt(local_context, result, &error);

      if (error.hasError())
      {
        rtLogError("v82rt() - return from script error");
        return RT_FAIL;
      }
    }
  }

  return RT_OK;
}

static std::string v8ReadFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;

  src_script << src_file.rdbuf(); // slurp up file

  std::string s = src_script.str();

  return s;
}

rtError rtV8Context::runFile(const char *file, rtValue* retVal /*= NULL*/, const char *args /*= NULL*/)
{
  if (file == NULL) {
    rtLogError(" %s  ... no script given.", __PRETTY_FUNCTION__);
    return RT_FAIL;
  }

  // Read the script file
  js_file = file;
  js_script = v8ReadFile(file);

  if (js_script.empty()) { // load error 
    rtLogError(" %s  ... load error / not found.", __PRETTY_FUNCTION__);
    return RT_FAIL;
  }

  rtError ret = runScript(js_script.c_str(), retVal, args);
  if (ret == RT_FAIL) {
    rtLogError("runFile v8 script '%s' failed", js_file);
  }

  return ret;
}

rtScriptV8::rtScriptV8():mRefCount(0), mV8Initialized(false)
{
  mIsolate = NULL;
  mPlatform = NULL;
  mUvLoop = NULL;
  init();
}

rtScriptV8::~rtScriptV8()
{
  term();
}

unsigned long rtScriptV8::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) {
     delete this;
    }
    return l;
}

rtError rtScriptV8::init()
{
  rtLogInfo(__FUNCTION__);

  if (mV8Initialized == false) {
    UErrorCode status = U_ZERO_ERROR;
    udata_setCommonData(&SMALL_ICUDATA_ENTRY_POINT, &status);

    v8::V8::InitializeICU();
    v8::V8::InitializeExternalStartupData("");
    mUvLoop = uv_default_loop();
    Platform* platform = platform::CreateDefaultPlatform();
    mPlatform = platform;
    V8::InitializePlatform(platform);
    V8::Initialize();

    Isolate::CreateParams params;
    params.array_buffer_allocator = v8::ArrayBuffer::Allocator::NewDefaultAllocator();
    mIsolate = Isolate::New(params);

#if 0
    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    Local<Context> ctx = Context::New(mIsolate);
    mContext.Reset(mIsolate, ctx);
#endif

    mV8Initialized = true;
  }
  
  return RT_OK;
}

rtError rtScriptV8::term()
{
  if (mV8Initialized == true) {
    V8::ShutdownPlatform();
    if (mPlatform) {
      delete mPlatform;
      mPlatform = NULL;
    }
    mV8Initialized = false;
  }

  return RT_OK;
}


rtError rtScriptV8::createContext(const char *lang, rtScriptContextRef& ctx)
{
  rtV8ContextRef v8Ctx = createContext();

  ctx = (rtIScriptContext*)v8Ctx.getPtr();
  return RT_OK;
}

rtV8ContextRef rtScriptV8::createContext()
{
  return new rtV8Context(mIsolate, mPlatform, mUvLoop);
}

rtError rtScriptV8::pump()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  v8::platform::PumpMessageLoop(mPlatform, mIsolate);
  mIsolate->RunMicrotasks();
  uv_run(mUvLoop, UV_RUN_NOWAIT);

  return RT_OK;
}

rtError rtScriptV8::collectGarbage()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  Local<Context> local_context = Context::New(mIsolate);
  Context::Scope contextScope(local_context);
  mIsolate->LowMemoryNotification();
  return RT_OK;
}

void* rtScriptV8::getParameter(rtString param)
{
  if (param.compare("isolate") == 0)
    return getIsolate();
  return NULL;
}

unsigned long rtV8Context::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) {
     delete this;
    }
    return l;
}

rtError createScriptV8(rtScriptRef& script)
{
  script = new rtScriptV8();
  return RT_OK;
}
