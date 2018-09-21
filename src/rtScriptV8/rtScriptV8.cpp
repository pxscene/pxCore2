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

#ifdef RTSCRIPT_SUPPORT_V8

#ifndef USE_SYSTEM_V8
extern unsigned char natives_blob_bin_data[];
extern int natives_blob_bin_size;
extern unsigned char snapshot_blob_bin_data[];
extern int snapshot_blob_bin_size;
#endif /* USE_SYSTEM_V8 */

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
#include <list>

#include <unicode/uvernum.h>
#include <unicode/putil.h>
#include <unicode/udata.h>
#include <unicode/uidna.h>

#ifndef USE_SYSTEM_V8
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
#endif /* USE_SYSTEM_V8 */

#include "rtWrapperUtils.h"
#include "rtScriptV8Node.h"

#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"
#include "rtPromise.h"
#include "rtFunctionWrapper.h"
#include "rtObjectWrapper.h"

#include <string>
#include <map>

#include "rtFileDownloader.h"
#include <rtMutex.h>

#include <string>
#include <fstream>
#include <sstream>

#if defined(USE_STD_THREADS)
#include <thread>
#include <mutex>
#endif

#if !defined(_WIN32)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef  S_ISREG
# define S_ISREG(x)  (((x) & _S_IFMT) == _S_IFREG)
#endif
#ifndef  S_ISDIR
# define S_ISDIR(x)  (((x) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef  S_ISFIFO
# define S_ISFIFO(x) (((x) & _S_IFMT) == _S_IFIFO)
#endif
#ifndef  S_ISCHR
# define S_ISCHR(x)  (((x) & _S_IFMT) == _S_IFCHR)
#endif
#ifndef  S_ISBLK
# define S_ISBLK(x)  0
#endif
#ifndef  S_ISLINK
# define S_ISLNK(x)  0
#endif
#ifndef  S_ISSOCK
# define S_ISSOCK(x) 0
#endif
#endif

#include "headers.h"
#include "libplatform/libplatform.h"

static rtAtomic sNextId = 100;

bool gIsPumpingJavaScript = false;

bool rtIsMainThreadNode()
{
  return true;
}

namespace rtScriptV8NodeUtils
{
  extern rtV8FunctionItem v8ModuleBindings[];

  rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
} 

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
  virtual void Free(void* data, size_t length, AllocationMode mode) {
    free(data);
  }
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

using namespace rtScriptV8NodeUtils;


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
  bool resolveModulePath(const rtString &name, rtString &data);

private:
   v8::Isolate                   *mIsolate;
   v8::Platform                  *mPlatform;
   v8::Persistent<v8::Context>    mContext;
   uv_loop_t                     *mUvLoop;

   std::map<rtString, Persistent<Value> *> mLoadedModuleCache;

   rtRef<rtFunctionCallback>      mHttpGetBinding;

   int mRefCount;

   rtAtomic mId;
   rtString mDirname;

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

rtV8Context::rtV8Context(Isolate *isolate, Platform *platform, uv_loop_t *loop) :
     mIsolate(isolate), mPlatform(platform), mUvLoop(loop), mRefCount(0), mDirname(rtString())
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
  global->Set(String::NewFromUtf8(mIsolate, "_isV8", NewStringType::kNormal).ToLocalChecked(), 
    v8::Integer::New(isolate, 1));
  global->Set(String::NewFromUtf8(mIsolate, "global", NewStringType::kNormal).ToLocalChecked(),
    v8::Object::New(isolate));

  rtObjectWrapper::exportPrototype(mIsolate, global);
  rtFunctionWrapper::exportPrototype(mIsolate, global);

  setupModuleLoading();
  setupModuleBindings();

  v8::platform::PumpMessageLoop(mPlatform, mIsolate);

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

bool rtV8Context::resolveModulePath(const rtString &name, rtString &data)
{
  uv_fs_t req;
  std::list<rtString> dirs;
  std::list<rtString> endings;
  bool found = false;
  rtString path;

  if (name.beginsWith("./") || name.beginsWith("../")) {
    if (!mDirname.isEmpty()) {
      dirs.push_back(mDirname);
    }
  }

  dirs.push_back(""); // this dir
  dirs.push_back("v8_modules/");
  dirs.push_back("node_modules/");
  //dirs.push_back("../external/libnode-v6.9.0/lib/");
  //dirs.push_back("../external/libnode-v6.9.0/lib/internal/");

  endings.push_back(".js");
  // not parsing package.json
  endings.push_back("/index.js");
  endings.push_back("/lib/index.js");

  std::list<rtString>::const_iterator it, jt;
  for (it = dirs.begin(); !found && it != dirs.end(); ++it) {
    rtString s = *it;
    if (!s.isEmpty() && !s.endsWith("/")) {
      s.append("/");
    }
    s.append(name.beginsWith("./") ? name.substring(2) : name);
    for (jt = endings.begin(); !found && jt != endings.end(); ++jt) {
      path = s;
      if (!path.endsWith((*jt).cString())) {
        path.append(*jt);
      }
      found = uv_fs_stat(mUvLoop, &req, path.cString(), NULL) == 0;
      uv_fs_req_cleanup(&req);
    }
  }

  if (found)
    data = path;
  return found;
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
  if (fd) {
    uv_fs_close(loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);
  }
  data = rtString(chunk, size);
  free(chunk);
  return true;

fail:
  rtLogError("%s errno: %d", __FUNCTION__, errno);
  uv_fs_req_cleanup(&req);
  if (fd) {
    uv_fs_close(loop, &req, fd, NULL);
    uv_fs_req_cleanup(&req);
  }
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
  rtString path;
  if (!resolveModulePath(name, path)) {
    rtLogWarn("module '%s' not found", name.cString());
    return Local<Value>();
  } else {
    rtLogInfo("module '%s' found at '%s'", name.cString(), path.cString());
  }

  rtString contents;
  if (!bloatFileFromPath(mUvLoop, path, contents)) {
    rtLogWarn("module '%s' not found", name.cString());
    return Local<Value>();
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
  rtString currentDirname = mDirname;
  int32_t pos = -1, p = -1;
  while ((p = path.find(p + 1, '/')) != -1) {
    pos = p;
  }
  mDirname = pos != -1 ? path.substring(0, pos + 1) : "";
  MaybeLocal<v8::Value> ret =  func->Call(localContext, result.ToLocalChecked(), 0, NULL);
  mDirname = currentDirname;

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
  assert(args.Length() >= 1);
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

  local_context->Global()->Set(String::NewFromUtf8(mIsolate, name), rt2js(local_context, val));

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
    return js2rt(local_context, object, &error);
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
      *retVal = js2rt(local_context, result, &error);

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

rtScriptV8::rtScriptV8():mIsolate(NULL), mPlatform(NULL), mUvLoop(NULL), mV8Initialized(false), mRefCount(0)
{
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
#ifndef USE_SYSTEM_V8
    UErrorCode status = U_ZERO_ERROR;
    udata_setCommonData(&SMALL_ICUDATA_ENTRY_POINT, &status);

    v8::V8::InitializeICU();

    StartupData nativesBlob;
    nativesBlob.data = (const char*)natives_blob_bin_data;
    nativesBlob.raw_size = natives_blob_bin_size;
    v8::V8::SetNativesDataBlob(&nativesBlob);

    StartupData snapshotBlob;
    snapshotBlob.data = (const char*)snapshot_blob_bin_data;
    snapshotBlob.raw_size = snapshot_blob_bin_size;
    v8::V8::SetSnapshotDataBlob(&snapshotBlob);
#endif

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

using namespace std;

//-----------------------------------

namespace rtScriptV8NodeUtils
{

  static void uvGetPlatform(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);
    const char *platform = "unknown";
#ifdef _WIN32
    platform = "win32";
#elif defined(__linux__)
    platform = "linux";
#elif defined(__APPLE__)
    platform = "macosx";
#endif
    args.GetReturnValue().Set<v8::String>(String::NewFromUtf8(args.GetIsolate(), platform, NewStringType::kNormal).ToLocalChecked());
  }

  static void uvGetHrTime(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);
    uint64_t hrtime = uv_hrtime();
    int nanoseconds = hrtime % (int)1e9;
    int seconds = hrtime / 1e9;

    Local<Array> arr = Array::New(isolate, 2);
    arr->Set(0, Integer::New(isolate, seconds));
    arr->Set(1, Integer::New(isolate, nanoseconds));

    args.GetReturnValue().Set<Array>(arr);
  }

  uv_loop_t *getEventLoopFromArgs(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    assert(args.Data()->IsExternal());
    v8::External *val = v8::External::Cast(*args.Data());
    assert(val != NULL);
    uv_loop_t *loop = (uv_loop_t *)val->Value();
    return loop;
  }

  static void uvFsAccess(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().Set(-1);

    if (args.Length() != 2) {
      return;
    }

    assert(args.Length() == 2);
    assert(args[0]->IsString());
    assert(args[1]->IsString());

    rtString filePath = toString(args[0]->ToString());
    rtString fileOpenMode = toString(args[1]->ToString());

    int mode = 0;
    size_t i, l;
    for (i = 0, l = fileOpenMode.length(); i < l; ++i) {
      switch (fileOpenMode[i]) {
      case 'r': case 'R': mode |= R_OK; break;
      case 'w': case 'W': mode |= W_OK; break;
      case 'x': case 'X': mode |= X_OK; break;
      default:
        rtLogWarn("Unknown file access mode '%s'", fileOpenMode.cString());
        return;
      }
    }

    uv_fs_t req;
    int ret = uv_fs_access(loop, &req, filePath.cString(), mode, NULL);

    args.GetReturnValue().Set(ret);
  }

  static void uvFsGetSize(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().Set(-1);

    if (args.Length() != 1) {
      return;
    }

    assert(args.Length() == 1);
    assert(args[0]->IsString());

    rtString filePath = toString(args[0]->ToString());

    uv_fs_t req;
    int ret = uv_fs_stat(loop, &req, filePath.cString(), NULL);

    if (ret < 0) {
      return;
    }

    args.GetReturnValue().Set((uint32_t)req.statbuf.st_size);
  }

  static int stringToFlags(const char* s)
  {
    bool read = false;
    bool write = false;
    int flags = 0;
    while (s && s[0]) {
      switch (s[0]) {
      case 'r': read = true; break;
      case 'w': write = true; flags |= O_TRUNC | O_CREAT; break;
      case 'a': write = true; flags |= O_APPEND | O_CREAT; break;
      case '+': read = true; write = true; break;
      case 'x': flags |= O_EXCL; break;

#ifndef _WIN32
      case 's': flags |= O_SYNC; break;
#endif

      default:
        return 0;
      }
      s++;
    }
    flags |= read ? (write ? O_RDWR : O_RDONLY) :
      (write ? O_WRONLY : 0);
    return flags;
  }

  static void uvFsOpen(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().SetNull();

    if (args.Length() != 3) {
      return;
    }

    assert(args.Length() == 3);
    assert(args[0]->IsString());
    assert(args[1]->IsString());

    rtString filePath = toString(args[0]->ToString());
    rtString fileOpenMode = toString(args[1]->ToString());

    int flags = stringToFlags(fileOpenMode.cString());
    int mode = (int)args[2]->ToInteger()->Value();

    uv_fs_t req;
    int ret = uv_fs_open(loop, &req, filePath.cString(), flags, mode, NULL);

    if (ret < 0) {
      return;
    }

    args.GetReturnValue().Set(v8::External::New(isolate, (void *)req.result));
  }

  static void uvFsRead(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().SetNull();

    if (args.Length() != 3 || !args[0]->IsExternal()) {
      return;
    }

    assert(args.Length() == 3);
    assert(args[0]->IsExternal());

    v8::External *val = v8::External::Cast(*args[0]);
    void *fd = (void *)val->Value();
    int size = (int)args[1]->ToInteger()->Value();
    int offset = (int)args[2]->ToInteger()->Value();

    void *ptr = malloc(size);
    uv_buf_t buf = uv_buf_init((char *)ptr, size);

    uv_fs_t req;
    uv_fs_read(loop, &req, static_cast<uv_file>(reinterpret_cast<uint64_t>(fd)), &buf, 1, offset, NULL);

    if (req.result <= 0) {
      free(ptr);
      return;
    }

    Local<String> ret = String::NewFromOneByte(isolate, (const uint8_t *)ptr, v8::NewStringType::kNormal, req.result).ToLocalChecked();

    free(ptr);

    args.GetReturnValue().Set(ret);
  }

  static void uvFsClose(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().Set(-1);

    if (args.Length() != 1 || !args[0]->IsExternal()) {
      return;
    }

    assert(args.Length() == 1);
    assert(args[0]->IsExternal());

    v8::External *val = v8::External::Cast(*args[0]);
    void *fd = (void *)val->Value();

    uv_fs_t req;
    int ret = uv_fs_close(loop, &req, static_cast<uv_file>(reinterpret_cast<uint64_t>(fd)), NULL);

    if (ret < 0) {
      return;
    }

    args.GetReturnValue().Set(1);
  }

  static void uvTimerNew(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().SetNull();

    if (args.Length() != 0) {
      return;
    }

    uv_timer_t *timer = new uv_timer_t();
    uv_timer_init(loop, timer);

    args.GetReturnValue().Set(v8::External::New(isolate, (void *)timer));
  }

  struct cbData
  {
    Isolate *mIsolate;
    v8::Persistent<v8::Context> mContext;
    v8::Persistent<v8::Function> mFunc;
  };

  static void v8TimerCallback(uv_timer_t* handle)
  {
    cbData *data = (cbData *)handle->data;

    Local<v8::Context> ctx = PersistentToLocal(data->mIsolate, data->mContext);
    Local<v8::Function> func = PersistentToLocal(data->mIsolate, data->mFunc);
    Local<v8::Value> argv[1] = { func.As<Value>() };

    std::ignore = func->Call(ctx, argv[0], 1, argv);
  }

  static void uvTimerStart(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    std::ignore = getEventLoopFromArgs(args);

    args.GetReturnValue().Set(-1);

    if (args.Length() != 4 || !args[0]->IsExternal()) {
      return;
    }

    assert(args.Length() == 4);
    assert(args[0]->IsExternal());
    assert(args[3]->IsFunction());

    v8::External *val = v8::External::Cast(*args[0]);
    uv_timer_t *handle = (uv_timer_t *)val->Value();
    int timeout = (int)args[1]->ToInteger()->Value();
    int repeat = (int)args[2]->ToInteger()->Value();

    v8::Persistent<v8::Function> func(isolate, Local<v8::Function>::Cast(args[3]));

    cbData *data = new cbData();

    handle->data = data;
    data->mIsolate = isolate;
    data->mContext.Reset(isolate, isolate->GetCurrentContext());
    data->mFunc.Reset(isolate, Local<v8::Function>::Cast(args[3]));

    uv_timer_start(handle, &v8TimerCallback, timeout, repeat);

    args.GetReturnValue().Set(1);
  }

  static void uvTimerStop(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    std::ignore = getEventLoopFromArgs(args);

    args.GetReturnValue().Set(-1);

    if (args.Length() != 1 || !args[0]->IsExternal()) {
      return;
    }

    assert(args.Length() == 1);
    assert(args[0]->IsExternal());

    v8::External *val = v8::External::Cast(*args[0]);
    uv_timer_t *handle = (uv_timer_t *)val->Value();

    uv_timer_stop(handle);

    args.GetReturnValue().Set(1);
  }

  static std::string v8ReadFile(const char *file)
  {
    std::ifstream       src_file(file);
    std::stringstream   src_script;

    src_script << src_file.rdbuf(); // slurp up file

    std::string s = src_script.str();

    return s;
  }

  static void uvRunInContext(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    std::ignore = getEventLoopFromArgs(args);

    args.GetReturnValue().SetNull();

    /*if (args.Length() != 1) {
    return;
    }*/

    assert(args.Length() >= 1);
    assert(args[0]->IsString());

    rtString sourceCode = toString(args[0]->ToString());

    Locker                locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope     handle_scope(isolate);

    Local<Context> local_context = isolate->GetCurrentContext();
    Context::Scope context_scope(local_context);

    TryCatch tryCatch(isolate);
    Local<String> source = String::NewFromUtf8(isolate, sourceCode.cString());
    Local<Script> run_script = Script::Compile(source);
    Local<Value> result = run_script->Run();

    if (tryCatch.HasCaught()) {
      String::Utf8Value trace(tryCatch.StackTrace());
      rtLogWarn("uvRunInContext: '%s'", *trace);
      return;
    }

    args.GetReturnValue().Set(result);
  }

  static void v8CopyProperties(Isolate *isolate, Local<Context> &fromContext, Local<Context> &toContext, Local<Object> sandboxObj)
  {
    HandleScope scope(isolate);

    Local<Object> global = toContext->Global()->GetPrototype()->ToObject(isolate);
    Local<Function> clone_property_method;

    Local<Array> names = global->GetOwnPropertyNames();
    int length = names->Length();
    for (int i = 0; i < length; i++) {
      Local<String> key = names->Get(i)->ToString(isolate);
      auto maybe_has = sandboxObj->HasOwnProperty(toContext, key);

      // Check for pending exceptions
      if (!maybe_has.IsJust())
        break;

      bool has = maybe_has.FromJust();

      if (!has) {
        // Could also do this like so:
        //
        // PropertyAttribute att = global->GetPropertyAttributes(key_v);
        // Local<Value> val = global->Get(key_v);
        // sandbox->ForceSet(key_v, val, att);
        //
        // However, this doesn't handle ES6-style properties configured with
        // Object.defineProperty, and that's exactly what we're up against at
        // this point.  ForceSet(key,val,att) only supports value properties
        // with the ES3-style attribute flags (DontDelete/DontEnum/ReadOnly),
        // which doesn't faithfully capture the full range of configurations
        // that can be done using Object.defineProperty.
        if (clone_property_method.IsEmpty()) {
          Local<String> code = String::NewFromUtf8(isolate,
            "(function cloneProperty(source, key, target) {\n"
            "  if (key === 'Proxy') return;\n"
            "  try {\n"
            "    var desc = Object.getOwnPropertyDescriptor(source, key);\n"
            "    if (desc.value === source) desc.value = target;\n"
            "    Object.defineProperty(target, key, desc);\n"
            "  } catch (e) {\n"
            "   // Catch sealed properties errors\n"
            "  }\n"
            "})");

          Local<Script> script =
            Script::Compile(toContext, code).ToLocalChecked();
          clone_property_method = Local<Function>::Cast(script->Run());
          assert(clone_property_method->IsFunction());
        }
        Local<Value> args[] = { global, key, sandboxObj };
        clone_property_method->Call(global, sizeof(args) / sizeof(args[0]), args);
      }
    }
  }

  static void uvRunInNewContext(const v8::FunctionCallbackInfo<v8::Value>& args)
  {
    Isolate* isolate = args.GetIsolate();
    HandleScope scope(isolate);

    uv_loop_t *loop = getEventLoopFromArgs(args);

    args.GetReturnValue().SetNull();

    assert(args.Length() >= 2);
    assert(args[0]->IsString());
    assert(args[1]->IsObject());

    rtString sourceCode = toString(args[0]->ToString());
    Local<Object> sandbox = args[1].As<Object>();

    Locker                locker(isolate);
    Isolate::Scope isolate_scope(isolate);
    HandleScope     handle_scope(isolate);

    Local<Context> toContext = Context::New(isolate);

    v8::Persistent<v8::Context> pContext;
    pContext.Reset(isolate, toContext);

    Local<Context> fromContext = isolate->GetCallingContext();

    Context::Scope contextScope(toContext);

    TryCatch tryCatch(isolate);

    v8CopyProperties(isolate, fromContext, toContext, sandbox);

    for (int i = 0; v8ModuleBindings[i].mName != NULL; ++i) {
      Local<FunctionTemplate> fTemplate = v8::FunctionTemplate::New(isolate,
        v8ModuleBindings[i].mCallback, v8::External::New(isolate, loop));
      toContext->Global()->Set(String::NewFromUtf8(isolate,
        v8ModuleBindings[i].mName, NewStringType::kNormal).ToLocalChecked(), fTemplate->GetFunction());
    }

    Local<String> source = String::NewFromUtf8(isolate, sourceCode.cString());
    MaybeLocal<Script> run_script = Script::Compile(toContext, source);
    if (run_script.IsEmpty()) {
      rtLogWarn("uvRunInNewContext: compilation failed");
      return;
    }
    MaybeLocal<Value> result = run_script.ToLocalChecked()->Run(toContext);

    if (result.IsEmpty() || tryCatch.HasCaught()) {
      String::Utf8Value trace(tryCatch.StackTrace());
      rtLogWarn("uvRunInNewContext: '%s'", *trace);
      return;
    }

    args.GetReturnValue().Set(result.ToLocalChecked());
  }

  rtV8FunctionItem v8ModuleBindings[] = {
    { "uv_platform", &uvGetPlatform },
    { "uv_hrtime", &uvGetHrTime },
    { "uv_fs_access", &uvFsAccess },
    { "uv_fs_size", &uvFsGetSize },
    { "uv_fs_open", &uvFsOpen },
    { "uv_fs_read", &uvFsRead },
    { "uv_fs_close", &uvFsClose },
    { "uv_timer_new", &uvTimerNew },
    { "uv_timer_start", &uvTimerStart },
    { "uv_timer_stop", &uvTimerStop },
    { "uv_run_in_context", &uvRunInContext },
    { "uv_run_in_new_context", &uvRunInNewContext },
    { NULL, NULL },
  };

  class rtHttpResponse : public rtObject
  {
  public:
    rtDeclareObject(rtHttpResponse, rtObject);
    rtReadOnlyProperty(statusCode, statusCode, int32_t);
    rtReadOnlyProperty(message, errorMessage, rtString);
    rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);
    rtMethodNoArgAndNoReturn("abort", abort);

    rtHttpResponse() : mStatusCode(0) {
      mEmit = new rtEmit();
    }

    rtError statusCode(int32_t& v) const { v = mStatusCode;  return RT_OK; }
    rtError errorMessage(rtString& v) const { v = mErrorMessage;  return RT_OK; }
    rtError addListener(rtString eventName, const rtFunctionRef& f) { mEmit->addListener(eventName, f); return RT_OK; }
    rtError abort() const { return RT_OK; }

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
  rtDefineMethod(rtHttpResponse, abort);

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
    if (numArgs < 1) {
      return RT_ERROR_INVALID_ARG;
    }

    rtString resourceUrl;
    if (args[0].getType() == RT_stringType) {
      resourceUrl = args[0].toString();
    }
    else {
      if (args[0].getType() != RT_objectType) {
        return RT_ERROR_INVALID_ARG;
      }
      rtObjectRef obj = args[0].toObject();

      rtString proto = obj.get<rtString>("protocol");
      rtString host = obj.get<rtString>("host");
      rtString path = obj.get<rtString>("path");

      resourceUrl.append(proto.cString());
      resourceUrl.append("//");
      resourceUrl.append(host.cString());
      resourceUrl.append(path.cString());
    }

    rtValue ret;
    rtObjectRef resp(new rtHttpResponse());

    if (numArgs > 1 && args[1].getType() == RT_functionType) {
      args[1].toFunction().sendReturns(resp, ret);
      rtFileDownloadRequest *downloadRequest = new rtFileDownloadRequest(resourceUrl, resp.getPtr(), rtHttpResponse::onDownloadComplete);
      downloadRequest->setDownloadProgressCallbackFunction(rtHttpResponse::onDownloadInProgress, resp.getPtr());
      rtFileDownloader::instance()->addToDownloadQueue(downloadRequest);
    }

    *result = resp;

    return RT_OK;
  }

} // namespace

#endif