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

#include <string>
#include <fstream>

#include <iostream>
#include <sstream>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "rtWrapperUtils.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "rtScriptV8.h"


#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"
#include "rtFunctionWrapper.h"
#include "rtObjectWrapper.h"


#include <string>
#include <map>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#pragma GCC diagnostic ignored "-Wall"
#endif

#include  "v8_headers.h"
#include "libplatform/libplatform.h"

#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

#define USE_CONTEXTIFY_CLONES

class V8ArrayBufferAllocator : public v8::ArrayBuffer::Allocator {
public:
  virtual void* Allocate(size_t size)
  {
    void *ret = AllocateUninitialized(size);
    if (!ret)
    {
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
  rtV8Context(v8::Isolate *isolate, v8::Platform* platform);
#ifdef USE_CONTEXTIFY_CLONES
  rtV8Context(v8::Isolate *isolate, rtV8ContextRef clone_me);
#endif

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

private:
   v8::Isolate                   *mIsolate;
   v8::Platform                  *mPlatform;
   v8::Persistent<v8::Context>    mContext;
   int mRefCount;

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

  bool                           mV8Initialized;

  int mRefCount;
};


using namespace v8;

#define RT_V8_TEST_BINDINGS
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

rtRef<rtFunctionCallback> g_testArrayReturnFunc;
rtRef<rtFunctionCallback> g_testMapReturnFunc;
rtRef<rtFunctionCallback> g_testObjectReturnFunc;

#endif

rtV8Context::rtV8Context(Isolate *isolate, Platform *platform) :
     mIsolate(isolate), mRefCount(0), mPlatform(platform)
{
  rtLogInfo(__FUNCTION__);
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  // Create a new context.
  Local<Context> localContext = Context::New(mIsolate);

  mContext.Reset(mIsolate, localContext); // local to persistent

  Context::Scope contextScope(localContext);

  Handle<Object> global = localContext->Global();

  rtObjectWrapper::exportPrototype(mIsolate, global);
  rtFunctionWrapper::exportPrototype(mIsolate, global);

  v8::platform::PumpMessageLoop(mPlatform, mIsolate);

  g_testArrayReturnFunc = new rtFunctionCallback(rtTestArrayReturnBinding);
  g_testMapReturnFunc = new rtFunctionCallback(rtTestMapReturnBinding);
  g_testObjectReturnFunc = new rtFunctionCallback(rtTestObjectReturnBinding);

  add("_testArrayReturnFunc", g_testArrayReturnFunc.getPtr());
  add("_testMapReturnFunc", g_testMapReturnFunc.getPtr());
  add("_testObjectReturnFunc", g_testObjectReturnFunc.getPtr());
}

#ifdef USE_CONTEXTIFY_CLONES
rtV8Context::rtV8Context(Isolate *isolate, rtV8ContextRef clone_me)
{
  assert(0);
}
#endif

rtV8Context::~rtV8Context()
{
}

rtError rtV8Context::add(const char *name, const rtValue& val)
{
  if (name == NULL)
  {
    rtLogDebug(" rtNodeContext::add() - no symbolic name for rtValue");
    return RT_FAIL;
  }
  else if (this->has(name))
  {
    rtLogDebug(" rtNodeContext::add() - ALREADY HAS '%s' ... over-writing.", name);
    // return; // Allow for "Null"-ing erasure.
  }

  if (val.isEmpty())
  {
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
  if (name == NULL)
  {
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

  if (object->IsUndefined() || object->IsNull())
  {
    rtLogError("FATAL: '%s' is Undefined ", name);
    return rtValue();
  }
  else
  {
    rtWrapperError error; // TODO - handle error
    return v82rt(local_context, object, &error);
  }
}

bool rtV8Context::has(const char *name)
{
  if (name == NULL)
  {
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

  if (try_catch.HasCaught())
  {
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
  if (!script || strlen(script) == 0)
  {
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
    if (tryCatch.HasCaught())
    {
      String::Utf8Value trace(tryCatch.StackTrace());
      rtLogWarn("%s", *trace);

      return RT_FAIL;
    }

    if (retVal)
    {
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
  if (file == NULL)
  {
    rtLogError(" %s  ... no script given.", __PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  // Read the script file
  js_file = file;
  js_script = v8ReadFile(file);

  if (js_script.empty()) // load error
  {
    rtLogError(" %s  ... load error / not found.", __PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  return runScript(js_script.c_str(), retVal, args);
}

rtScriptV8::rtScriptV8():mRefCount(0), mV8Initialized(false)
{
  mIsolate = NULL;
  mPlatform = NULL;
  init();
}

rtScriptV8::~rtScriptV8()
{
  term();
}

unsigned long rtScriptV8::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)
    {
     delete this;
    }
    return l;
}

rtError rtScriptV8::init()
{
  rtLogInfo(__FUNCTION__);

  if (mV8Initialized == false)
  {
    V8::InitializeICU();
    Platform* platform = platform::CreateDefaultPlatform();
    mPlatform = platform;
    V8::InitializePlatform(platform);
    V8::Initialize();
    Isolate::CreateParams params;
    array_buffer_allocator = new V8ArrayBufferAllocator();
    const char* source1 = "function pxSceneFooFunction(){ return 0;}";
    static v8::StartupData data = v8::V8::CreateSnapshotDataBlob(source1);
    params.snapshot_blob = &data;
    params.array_buffer_allocator = array_buffer_allocator;
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
  if (mV8Initialized == true)
  {
    V8::ShutdownPlatform();
    if (mPlatform)
    {
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
  return new rtV8Context(mIsolate, mPlatform);
}

rtError rtScriptV8::pump()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  v8::platform::PumpMessageLoop(mPlatform, mIsolate);
  mIsolate->RunMicrotasks();
  uv_run(uv_default_loop(), UV_RUN_NOWAIT);

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
    if (l == 0)
    {
     delete this;
    }
    return l;
}

rtError createScriptV8(rtScriptRef& script)
{
  script = new rtScriptV8();
  return RT_OK;
}
