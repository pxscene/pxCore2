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

// rtNode.cpp

#ifdef RTSCRIPT_SUPPORT_NODE

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

#include "node.h"
#include "node_javascript.h"
#if NODE_VERSION_AT_LEAST(9,6,0)
#include "node_contextify.h"
#endif
#include "node_contextify_mods.h"

#include "env.h"
#include "env-inl.h"

#include "rtWrapperUtils.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "rtScriptV8Node.h"


#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"
#include "rtPathUtils.h"



// TODO eliminate std::string
#include <string>
#include <map>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#pragma GCC diagnostic ignored "-Wall"
#endif

#include "uv.h"
#include "v8.h"
#include "libplatform/libplatform.h"

#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

using namespace rtScriptV8NodeUtils;


#define SANDBOX_IDENTIFIER  ( (const char*) "_sandboxStuff" )
#define SANDBOX_JS          ( (const char*) "rcvrcore/sandbox.js")

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

#ifndef DISABLE_USE_CONTEXTIFY_CLONES
# define USE_CONTEXTIFY_CLONES
#endif
#ifdef RUNINMAIN
bool gIsPumpingJavaScript = false;
#endif

#if NODE_VERSION_AT_LEAST(8,12,0)
#define USE_NODE_PLATFORM
#endif

extern rtString g_debuggerAddress;
extern int g_debuggerPort;
extern bool g_debuggerEnabled;
namespace node
{
class Environment;
}

class rtScriptNode;
class rtNodeContext;

typedef rtRef<rtNodeContext> rtNodeContextRef;

#define NODE_DEBUGGER_ADDRESS "127.0.0.1"
#define NODE_DEBUGGER_PORT 9229
class rtNodeContext: rtIScriptContext  // V8
{
public:
  rtNodeContext(v8::Isolate *isolate, v8::Platform* platform);
#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContext(v8::Isolate *isolate, rtNodeContextRef clone_me);
#endif

 virtual ~rtNodeContext();

  virtual rtError add(const char *name, const rtValue& val);
  virtual rtValue get(const char *name);
  //rtValue get(std::string name);

  virtual bool    has(const char *name);
  bool    has(std::string name);

  //bool   find(const char *name);  //DEPRECATED

  virtual rtError runScript(const char        *script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  //rtError runScript(const std::string &script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  virtual rtError runFile  (const char *file,          rtValue* retVal = NULL, const char *args = NULL); // BLOCKS

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  const char   *js_file;
  std::string   js_script;

  v8::Isolate              *getIsolate()      const { return mIsolate; };
  v8::Local<v8::Context>    getLocalContext() const { return PersistentToLocal<v8::Context>(mIsolate, mContext); };

  uint32_t                  getContextId()    const { return mContextId; };

private:
  v8::Isolate                   *mIsolate;
  #if NODE_VERSION_AT_LEAST(9,8,0)
  node::Persistent<v8::Context>    mContext;
  #else
  v8::Persistent<v8::Context>    mContext;
  #endif
  uint32_t                       mContextId;

  node::Environment*             mEnv;
  #if NODE_VERSION_AT_LEAST(9,8,0)
  node::Persistent<v8::Object>     mRtWrappers;
  #else
  v8::Persistent<v8::Object>     mRtWrappers;
  #endif

  void createEnvironment();

#ifdef USE_CONTEXTIFY_CLONES
  void clonedEnvironment(rtNodeContextRef clone_me);
#endif

  int mRefCount;
  rtAtomic mId;
  v8::Platform                  *mPlatform;
  void* mContextifyContext;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint32_t, rtNodeContextRef> rtNodeContexts;
typedef std::map<uint32_t, rtNodeContextRef>::const_iterator rtNodeContexts_iterator;

class rtScriptNode: public rtIScript
{
public:
  rtScriptNode();
  rtScriptNode(bool initialize);
  virtual ~rtScriptNode();

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  rtError init();

  rtString engine() { return "node/v8"; }

  rtError pump();

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);
  rtError createContext(const char *lang, rtScriptContextRef& ctx);
#if 0
#ifndef RUNINMAIN
  bool isInitialized();
  bool needsToEnd() { /*rtLogDebug("needsToEnd returning %d\n",mNeedsToEnd);*/ return mNeedsToEnd;};
  void setNeedsToEnd(bool end) { /*rtLogDebug("needsToEnd being set to %d\n",end);*/ mNeedsToEnd = end;}
#endif
#endif

  v8::Isolate   *getIsolate() { return mIsolate; };
  v8::Platform   *getPlatform() { return mPlatform; };

  rtError collectGarbage();
  void* getParameter(rtString param);
private:
#if 0
#ifdef ENABLE_DEBUG_MODE
  void init();
#else
  void init(int argc, char** argv);
#endif
#endif

  rtError term();

  void nodePath();

  v8::Isolate                   *mIsolate;
  v8::Platform                  *mPlatform;
  #if NODE_VERSION_AT_LEAST(9,8,0)
  node::Persistent<v8::Context>    mContext;
  #else
  v8::Persistent<v8::Context>    mContext;
  #endif


#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContextRef mRefContext;
#endif

  bool mTestGc;
#ifndef RUNINMAIN
  bool mNeedsToEnd;
#endif
  void init2(int argc, char** argv);

  int mRefCount;
};



#ifndef RUNINMAIN
extern uv_loop_t *nodeLoop;
#endif
//#include "rtThreadQueue.h"

//extern rtThreadQueue gUIThreadQueue;

#ifdef RUNINMAIN

//#include "pxEventLoop.h"
//extern pxEventLoop* gLoop;

#define ENTERSCENELOCK()
#define EXITSCENELOCK()
#else
#define ENTERSCENELOCK() rtWrapperSceneUpdateEnter();
#define EXITSCENELOCK()  rtWrapperSceneUpdateExit();
#endif

using namespace v8;
using namespace node;

extern args_t *s_gArgs;
namespace node
{
#if NODE_VERSION_AT_LEAST(8,9,4)
extern DebugOptions debug_options;
#else
extern bool use_debug_agent;
#if HAVE_INSPECTOR
extern bool use_inspector;
#endif
extern bool debug_wait_connect;
#endif
}

static int exec_argc;
static const char** exec_argv;
static rtAtomic sNextId = 100;



#ifdef RUNINMAIN
//extern rtNode script;
#endif
rtNodeContexts  mNodeContexts;

#ifdef ENABLE_NODE_V_6_9
ArrayBufferAllocator* array_buffer_allocator = NULL;
bool bufferAllocatorIsSet = false;
#endif
bool nodeTerminated = false;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef RUNINMAIN
#ifdef WIN32
static DWORD __rt_main_thread__;
#else
static pthread_t __rt_main_thread__;
#endif

// rtIsMainThread() - Previously:  identify the MAIN thread of 'node' which running JS code.
//
// rtIsMainThread() - Currently:   identify BACKGROUND thread which running JS code.
//
bool rtIsMainThreadNode()
{
  // Since this is single threaded version we're always on the js thread
  return true;
}
#endif
#if 0
static inline bool file_exists(const char *file)
{
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}
#endif

rtNodeContext::rtNodeContext(Isolate *isolate,Platform* platform) :
     js_file(NULL), mIsolate(isolate), mEnv(NULL), mRefCount(0),mPlatform(platform), mContextifyContext(NULL)
{
  assert(isolate); // MUST HAVE !
  mId = rtAtomicInc(&sNextId);

  createEnvironment();
}

#ifdef USE_CONTEXTIFY_CLONES
rtNodeContext::rtNodeContext(Isolate *isolate, rtNodeContextRef clone_me) :
      js_file(NULL), mIsolate(isolate), mEnv(NULL), mRefCount(0), mPlatform(NULL), mContextifyContext(NULL)
{
  assert(mIsolate); // MUST HAVE !
  mId = rtAtomicInc(&sNextId);

  clonedEnvironment(clone_me);
}
#endif

void rtNodeContext::createEnvironment()
{
  rtLogDebug(__FUNCTION__);
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  // Create a new context.
  Local<Context> local_context = Context::New(mIsolate);

#ifdef ENABLE_NODE_V_6_9

  local_context->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, mId));

  mContextId = GetContextId(local_context);

  mContext.Reset(mIsolate, local_context); // local to persistent

  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();



  mRtWrappers.Reset(mIsolate, global);

  // Create Environment.

#if NODE_VERSION_AT_LEAST(8,9,4)
#ifdef USE_NODE_PLATFORM
  node::MultiIsolatePlatform* platform = static_cast<node::MultiIsolatePlatform*>(mPlatform);
  IsolateData *isolateData = new IsolateData(mIsolate,uv_default_loop(),platform,array_buffer_allocator->zero_fill_field());
#else
  IsolateData *isolateData = new IsolateData(mIsolate,uv_default_loop(),array_buffer_allocator->zero_fill_field());
#endif //USE_NODE_PLATFORM

  mEnv = CreateEnvironment(isolateData,
#else
  mEnv = CreateEnvironment(mIsolate,
                           uv_default_loop(),
#endif
                           local_context,
                           s_gArgs->argc,
                           s_gArgs->argv,
                           exec_argc,
                           exec_argv);

#if !NODE_VERSION_AT_LEAST(8,9,4)
   array_buffer_allocator->set_env(mEnv);
#endif
  mIsolate->SetAbortOnUncaughtExceptionCallback(
        ShouldAbortOnUncaughtException);
#ifdef ENABLE_DEBUG_MODE
#if !NODE_VERSION_AT_LEAST(8,9,4)
  // Start debug agent when argv has --debug
  if (use_debug_agent)
  {
    rtLogWarn("use_debug_agent\n");
#if HAVE_INSPECTOR
    if (use_inspector)
    {
      char currentPath[100];
      memset(currentPath,0,sizeof(currentPath));
      const char *rv = getcwd(currentPath,sizeof(currentPath));
      (void)rv;
      StartDebug(mEnv, currentPath, debug_wait_connect, mPlatform);
    }
    else
#endif
    {
      StartDebug(mEnv, NULL, debug_wait_connect);
    }
  }
#else
#if HAVE_INSPECTOR
#ifdef USE_NODE_PLATFORM
    if (g_debuggerEnabled) {
      if (0 == g_debuggerPort) {
        g_debuggerPort = NODE_DEBUGGER_PORT;
      }
      if (0 == g_debuggerAddress.length()) {
        g_debuggerAddress = NODE_DEBUGGER_ADDRESS;
      }
      rtString currentPath;
      rtGetCurrentDirectory(currentPath);
      node::InspectorStart(mEnv, currentPath.cString(), g_debuggerAddress, g_debuggerPort);
    }
#endif //USE_NODE_PLATFORM
#endif
#endif
#endif
// Load Environment.
  {
    Environment::AsyncCallbackScope callback_scope(mEnv);
    LoadEnvironment(mEnv);
  }
#if defined(ENABLE_DEBUG_MODE) && !NODE_VERSION_AT_LEAST( 8, 9, 4 )
  if (use_debug_agent)
  {
    rtLogWarn("use_debug_agent\n");
    EnableDebug(mEnv);
  }
#endif
    rtObjectWrapper::exportPrototype(mIsolate, global);
    rtFunctionWrapper::exportPrototype(mIsolate, global);

    {
      SealHandleScope seal(mIsolate);
#ifndef RUNINMAIN
      EmitBeforeExit(mEnv);
#else
      bool more;
#ifdef ENABLE_NODE_V_6_9
#ifndef USE_NODE_PLATFORM
      v8::platform::PumpMessageLoop(mPlatform, mIsolate);
#endif //USE_NODE_PLATFORM
#endif //ENABLE_NODE_V_6_9
      more = uv_run(mEnv->event_loop(), UV_RUN_ONCE);
#ifdef USE_NODE_PLATFORM
      node::MultiIsolatePlatform* platform = static_cast<node::MultiIsolatePlatform*>(mPlatform);
      platform->DrainBackgroundTasks(mIsolate);
#endif //USE_NODE_PLATFORM
      if (more == false)
      {
        EmitBeforeExit(mEnv);
      }
#endif

    }
#else
  local_context->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, mId));

  mContextId = GetContextId(local_context);

  mContext.Reset(mIsolate, local_context); // local to persistent

  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  // Register wrappers.
  rtObjectWrapper::exportPrototype(mIsolate, global);
  rtFunctionWrapper::exportPrototype(mIsolate, global);

  mRtWrappers.Reset(mIsolate, global);

  // Create Environment.
  mEnv = CreateEnvironment(mIsolate,
                           uv_default_loop(),
                           local_context,
                           s_gArgs->argc,
                           s_gArgs->argv,
                           exec_argc,
                           exec_argv);

  // Start debug agent when argv has --debug
#ifdef ENABLE_DEBUG_MODE
  if (use_debug_agent)
  {
    rtLogWarn("use_debug_agent\n");
    StartDebug(mEnv, debug_wait_connect);
  }
#endif

  // Load Environment.
  LoadEnvironment(mEnv);

  // Enable debugger
  if (use_debug_agent)
  {
    EnableDebug(mEnv);
  }
#endif //ENABLE_NODE_V_6_9
}

#ifdef USE_CONTEXTIFY_CLONES

void rtNodeContext::clonedEnvironment(rtNodeContextRef clone_me)
{
  rtLogDebug(__FUNCTION__);
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  // Get parent Local context...
  Local<Context> local_context = clone_me->getLocalContext();
  Context::Scope context_scope(local_context);

  // Create dummy sandbox for ContextifyContext::makeContext() ...
  Local<Object> sandbox = Object::New(mIsolate);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  if( clone_me->has(SANDBOX_IDENTIFIER) )
  {
    rtValue       val_array = clone_me->get(SANDBOX_IDENTIFIER);
    rtObjectRef       array = val_array.toObject();

    int len = array.get<int>("length");

    rtString s;
    for(int i = 0; i < len; i++)
    {
      array.get<rtString>( (uint32_t) i, s);  // get 'name' for object
      rtValue obj = clone_me->get(s);         // get object for 'name'

      if( obj.isEmpty() == false)
      {
          // Copy to var/module 'sandbox' under construction...
          Local<Value> module = local_context->Global()->Get( String::NewFromUtf8(mIsolate, s.cString() ) );
          sandbox->Set( String::NewFromUtf8(mIsolate, s.cString()), module);
      }
      else
      {
        rtLogError("## FATAL:   '%s' is empty !! - UNEXPECTED", s.cString());
      }
    }
  }
  else
  {
    rtLogWarn("## WARNING:   '%s' is undefined !! - UNEXPECTED", SANDBOX_IDENTIFIER);
  }
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Clone a new context.
  {
  #if NODE_VERSION_AT_LEAST(10,0,0)
    contextify::ContextOptions options;
    std::stringstream   ctxname;
    ctxname << "SparkContext:" << mId;
    rtString currentPath;
    rtGetCurrentDirectory(currentPath);
    options.name = String::NewFromUtf8(mIsolate, ctxname.str().c_str() );
    options.origin =   String::NewFromUtf8(mIsolate, currentPath.cString() );
    options.allow_code_gen_strings = Boolean::New(mIsolate, true);
    options.allow_code_gen_wasm = Boolean::New(mIsolate, true);
    Local<Context>  clone_local = node::contextify::makeContext(mIsolate, sandbox, options); // contextify context with 'sandbox'
#else
    Local<Context>  clone_local = node::makeContext(mIsolate, sandbox); // contextify context with 'sandbox'
#endif

    clone_local->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, mId));
#ifdef ENABLE_NODE_V_6_9
    Local<Context> envCtx = Environment::GetCurrent(mIsolate)->context();
    Local<String> symbol_name = FIXED_ONE_BYTE_STRING(mIsolate, "_contextifyPrivate");
    Local<Private> private_symbol_name = Private::ForApi(mIsolate, symbol_name);
    MaybeLocal<Value> maybe_value = sandbox->GetPrivate(envCtx,private_symbol_name);
    Local<Value> decorated;
    if (true == maybe_value.ToLocal(&decorated))
    {
      mContextifyContext = decorated.As<External>()->Value();
    }
#else
    Local<String> hidden_name = FIXED_ONE_BYTE_STRING(mIsolate, "_contextifyHidden");
    mContextifyContext = sandbox->GetHiddenValue(hidden_name).As<External>()->Value();
#endif

    mContextId = GetContextId(clone_local);

    mContext.Reset(mIsolate, clone_local); // local to persistent
    // commenting below code as templates are isolcate specific
/*
    Context::Scope context_scope(clone_local);

    Handle<Object> clone_global = clone_local->Global();

    // Register wrappers in this cloned context...
      rtObjectWrapper::exportPrototype(mIsolate, clone_global);
    rtFunctionWrapper::exportPrototype(mIsolate, clone_global);

    mRtWrappers.Reset(mIsolate, clone_global);
*/
}

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
}

#endif // USE_CONTEXTIFY_CLONES

rtNodeContext::~rtNodeContext()
{
  rtLogDebug(__FUNCTION__);
  //Make sure node is not destroyed abnormally
  if (true == node_is_initialized)
  {
    if(mEnv)
    {
      Locker                locker(mIsolate);
      Isolate::Scope isolate_scope(mIsolate);
      HandleScope     handle_scope(mIsolate);

      RunAtExit(mEnv);
  #if !NODE_VERSION_AT_LEAST(8,9,4)
    #ifdef ENABLE_NODE_V_6_9
      if (nodeTerminated)
      {
        array_buffer_allocator->set_env(NULL);
      }
      else
      {
        mEnv->Dispose();
      }
    #else
      mEnv->Dispose();
    #endif // ENABLE_NODE_V_6_9
  #endif
      mEnv = NULL;
      #ifndef USE_CONTEXTIFY_CLONES
      HandleMap::clearAllForContext(mId);
      #endif
    }
    else
    {
    // clear out persistent javascript handles
      HandleMap::clearAllForContext(mId);
#if defined(ENABLE_NODE_V_6_9) && defined(USE_CONTEXTIFY_CLONES)
      // JRJR  This was causing HTTPS to crash in gl content reloads
      // what does this do exactly... am I leaking now  why is this only a 6.9 thing?
      #ifndef USE_NODE_10
      node::deleteContextifyContext(mContextifyContext);
      #endif
#endif
      mContextifyContext = NULL;
    }
    if(exec_argv)
    {
      #ifdef USE_NODE_10
        for (int i=0; i<exec_argc; i++) {
          if (NULL != exec_argv[i]) {
            free((void*)exec_argv[i]);
            exec_argv[i] = NULL;
          }
        }
      #endif
      delete[] exec_argv;
      exec_argv = NULL;
      exec_argc = 0;
    }

    // TODO:  Might not be needed in ST case...
    //
    // Un-Register wrappers.
    // rtObjectWrapper::destroyPrototype();
    // rtFunctionWrapper::destroyPrototype();
    mContext.Reset();
    mRtWrappers.Reset();

    Release();
  }
  // NOTE: 'mIsolate' is owned by rtNode.  Don't destroy here !
}

rtError rtNodeContext::add(const char *name, rtValue const& val)
{
  if(name == NULL)
  {
    rtLogDebug(" rtNodeContext::add() - no symbolic name for rtValue");
    return RT_FAIL;
  }
  else if(this->has(name))
  {
    rtLogDebug(" rtNodeContext::add() - ALREADY HAS '%s' ... over-writing.", name);
   // return; // Allow for "Null"-ing erasure.
  }

  if(val.isEmpty())
  {
    rtLogDebug(" rtNodeContext::add() - rtValue is empty");
    return RT_FAIL;
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  // Get a Local context...
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  local_context->Global()->Set( String::NewFromUtf8(mIsolate, name), rt2js(local_context, val));

  return RT_OK;
}

#if 0
rtValue rtNodeContext::get(std::string name)
{
  return get( name.c_str() );
}
#endif

rtValue rtNodeContext::get(const char *name)
{
  if(name == NULL)
  {
    rtLogError(" rtNodeContext::get() - no symbolic name for rtValue");
    return rtValue();
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  // Get a Local context...
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  // Get the object
  Local<Value> object = global->Get( String::NewFromUtf8(mIsolate, name) );

  if(object->IsUndefined() || object->IsNull() )
  {
    rtLogError("FATAL: '%s' is Undefined ", name);
    return rtValue();
  }
  else
  {
    rtWrapperError error; // TODO - handle error
    return js2rt(local_context, object, &error);
  }
}

#if 0
bool rtNodeContext::has(std::string name)
{
  return has( name.c_str() );
}
#endif

bool rtNodeContext::has(const char *name)
{
  if(name == NULL)
  {
    rtLogError(" rtNodeContext::has() - no symbolic name for rtValue");
    return false;
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  // Get a Local context...
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

#ifdef ENABLE_NODE_V_6_9
  TryCatch try_catch(mIsolate);
#else
  TryCatch try_catch;
#endif // ENABLE_NODE_V_6_9

  Handle<Value> value = global->Get(String::NewFromUtf8(mIsolate, name) );

  if (try_catch.HasCaught())
  {
     rtLogError("\n ## has() - HasCaught()  ... ERROR");
     return false;
  }

  // No need to check if |value| is empty because it's taken care of
  // by TryCatch above.

  return ( !value->IsUndefined() && !value->IsNull() );
}

// DEPRECATED - 'has()' is replacement for 'find()'
//
// bool rtNodeContext::find(const char *name)
// {
//   rtNodeContexts_iterator it = mNodeContexts.begin();
//
//   while(it != mNodeContexts.end())
//   {
//     rtNodeContextRef ctx = it->second;
//
//     rtLogWarn("\n ######## CONTEXT !!! ID: %d  %s  '%s'",
//       ctx->getContextId(),
//       (ctx->has(name) ? "*HAS*" : "does NOT have"),
//       name);
//
//     it++;
//   }
//
//   rtLogWarn("\n ");
//
//   return false;
// }

#if 0
rtError rtNodeContext::runScript(const char* script, rtValue* retVal /*= NULL*/, const char *args /*= NULL*/)
{
  if(script == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  // rtLogDebug(" %s  ... Running...",__PRETTY_FUNCTION__);

  return runScript(std::string(script), retVal, args);
}
#endif

#if 1
//rtError rtNodeContext::runScript(const std::string &script, rtValue* retVal /*= NULL*/, const char* /* args = NULL*/)
rtError rtNodeContext::runScript(const char* script, rtValue* retVal /*= NULL*/, const char *args /*= NULL*/)
{
  rtLogDebug(__FUNCTION__);
  if(!script || strlen(script) == 0)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  {//scope
    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    // Get a Local context...
    Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
    Context::Scope context_scope(local_context);
// !CLF TODO: TEST FOR MT
#ifdef RUNINMAIN
#ifdef ENABLE_NODE_V_6_9
  TryCatch tryCatch(mIsolate);
#else
  TryCatch tryCatch;
#endif // ENABLE_NODE_V_6_9
#endif
    Local<String> source = String::NewFromUtf8(mIsolate, script);

    // Compile the source code.
    MaybeLocal<Script> run_script = Script::Compile(local_context, source);
    if (run_script.IsEmpty()) {
      #if NODE_VERSION_AT_LEAST(8,10,0)
      String::Utf8Value trace(mIsolate, tryCatch.StackTrace(local_context).ToLocalChecked());
      #else
      String::Utf8Value trace(tryCatch.StackTrace(local_context).ToLocalChecked());
      #endif
      rtLogWarn("%s", *trace);
      return RT_FAIL;
    }

    // Run the script to get the result.
    MaybeLocal<Value> result = (run_script.ToLocalChecked())->Run(local_context);
// !CLF TODO: TEST FOR MT
#ifdef RUNINMAIN
   if (tryCatch.HasCaught())
    {
      #if NODE_VERSION_AT_LEAST(8,10,0)
      String::Utf8Value trace(mIsolate, tryCatch.StackTrace(local_context).ToLocalChecked());
      #else
      String::Utf8Value trace(tryCatch.StackTrace(local_context).ToLocalChecked());
      #endif
      rtLogWarn("%s", *trace);

      return RT_FAIL;
    }
#endif

    if(retVal)
    {
      // Return val
      rtWrapperError error;
      *retVal = js2rt(local_context, result.ToLocalChecked(), &error);

      if(error.hasError())
      {
        rtLogError("js2rt() - return from script error");
        return RT_FAIL;
      }
    }

   return RT_OK;

  }//scope

  return RT_FAIL;
}
#endif

static std::string readFile(const char *file)
{
  std::string s("");
  try {
    std::ifstream       src_file(file);
    std::stringstream   src_script;

    src_script << src_file.rdbuf(); // slurp up file

    s = src_script.str();
  }
  catch (std::ifstream::failure e) {
    rtLogError("Exception opening/reading/closing file [%s]\n", e.what());
  }
  catch(...) {
    rtLogError("Exception opening/reading/closing file \n");
  }
  return s;
}

rtError rtNodeContext::runFile(const char *file, rtValue* retVal /*= NULL*/, const char* args /*= NULL*/)
{
  if(file == NULL)
  {
    rtLogError(" %s  ... file == NULL ... no script given.",__PRETTY_FUNCTION__);

    return RT_FAIL;
  }

  // Read the script file
  js_file   = file;
  js_script = readFile(file);
  
  if( js_script.empty() ) // load error
  {
    rtLogError(" %s  ... [%s] load error / not found.",__PRETTY_FUNCTION__, file);

    return RT_FAIL;
  }

  return runScript(js_script.c_str(), retVal, args);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtScriptNode::rtScriptNode():mRefCount(0)
#ifndef RUNINMAIN
#ifdef USE_CONTEXTIFY_CLONES
: mRefContext(), mNeedsToEnd(false)
#else
: mNeedsToEnd(false)
#endif
#endif
{
  rtLogDebug(__FUNCTION__);
  mTestGc = false;
  mIsolate = NULL;
  mPlatform = NULL;
  init();
}

rtScriptNode::rtScriptNode(bool initialize):mRefCount(0)
#ifndef RUNINMAIN
#ifdef USE_CONTEXTIFY_CLONES
: mRefContext(), mNeedsToEnd(false)
#else
: mNeedsToEnd(false)
#endif
#endif
{
  rtLogDebug(__FUNCTION__);
  mTestGc = false;
  mIsolate = NULL;
  mPlatform = NULL;
  if (initialize)
  {
    init();
  }
}

unsigned long rtScriptNode::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)
    {
     delete this;
    }
    return l;
}

rtError rtScriptNode::init()
{
  rtLogDebug(__FUNCTION__);
  char const* s = getenv("RT_TEST_GC");
  if (s && strlen(s) > 0)
    mTestGc = true;
  else
    mTestGc = false;

  if (mTestGc)
    rtLogWarn("*** PERFORMANCE WARNING *** : gc being invoked in render thread");

  int argc = 0;
// TODO Please make this better... less hard coded...

                              //0123456 789ABCDEF012 345 67890ABCDEF
#ifdef ENABLE_DEBUG_MODE
  static const char* debug_node69Config = "rtNode\0--inspect\0--experimental-vm-modules\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* debug_node69Configv[] = {&debug_node69Config[0], &debug_node69Config[7], &debug_node69Config[17], &debug_node69Config[43], &debug_node69Config[46], NULL};

  static const char* debug_nodeHeapConfig = "rtNode\0--inspect\0--experimental-vm-modules\0--expose-gc\0--max_old_space_size=64\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* debug_nodeHeapConfigv[] = {&debug_nodeHeapConfig[0], &debug_nodeHeapConfig[7], &debug_nodeHeapConfig[17], &debug_nodeHeapConfig[43], &debug_nodeHeapConfig[55], &debug_nodeHeapConfig[79], &debug_nodeHeapConfig[82], NULL};

  static const char* debug_exposeGcConfig = "rtNode\0--inspect\0--experimental-vm-modules\0--expose-gc\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* debug_exposeGcConfigv[] = {&debug_exposeGcConfig[0], &debug_exposeGcConfig[7], &debug_exposeGcConfig[17], &debug_exposeGcConfig[43], &debug_exposeGcConfig[55], &debug_exposeGcConfig[58], NULL};
  
  static const char* node69Config = "rtNode\0--experimental-vm-modules\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* node69Configv[] = {&node69Config[0], &node69Config[7], &node69Config[33], &node69Config[36], NULL};

  static const char* nodeHeapConfig = "rtNode\0--experimental-vm-modules\0--expose-gc\0--max_old_space_size=64\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* nodeHeapConfigv[] = {&nodeHeapConfig[0], &nodeHeapConfig[7], &nodeHeapConfig[33], &nodeHeapConfig[45], &nodeHeapConfig[69], &nodeHeapConfig[72], NULL};

  static const char* exposeGcConfig = "rtNode\0--experimental-vm-modules\0--expose-gc\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char* exposeGcConfigv[] = {&exposeGcConfig[0], &exposeGcConfig[7], &exposeGcConfig[33], &exposeGcConfig[45], &exposeGcConfig[48], NULL};


  static char** argv2 = NULL;
  if (g_debuggerEnabled == true) {
  #if ENABLE_V8_HEAP_PARAMS
  #ifdef ENABLE_NODE_V_6_9
    argc = sizeof(debug_node69Configv)/sizeof(char*) - 1;
    argv2 = (char**)debug_node69Configv;
  #else
    rtLogWarn("v8 old heap space configured to 64mb\n");
    argc = sizeof(debug_nodeHeapConfigv)/sizeof(char*) - 1;
    argv2 = (char**)debug_nodeHeapConfigv; 
  #endif // ENABLE_NODE_V_6_9
  #else
  #ifdef ENABLE_NODE_V_6_9
    argc = sizeof(debug_node69Configv)/sizeof(char*) - 1;
    argv2 = (char**)debug_node69Configv;
  #else
    argc = sizeof(debug_exposeGcConfigv)/sizeof(char*) - 1;
    argv2 = (char**)debug_exposeGcConfigv;
  #endif // ENABLE_NODE_V_6_9
  #endif //ENABLE_V8_HEAP_PARAMS
  }
  else
  {
  #if ENABLE_V8_HEAP_PARAMS
  #ifdef ENABLE_NODE_V_6_9
    argc = sizeof(node69Configv)/sizeof(char*) - 1;
    argv2 = (char**)node69Configv;
  #else
    argc = sizeof(nodeHeapConfigv)/sizeof(char*) - 1;
    argv2 = (char **) nodeHeapConfigv;
  #endif // ENABLE_NODE_V_6_9
  #else
  #ifdef ENABLE_NODE_V_6_9
    argc = sizeof(node69Configv)/sizeof(char*) - 1;
    argv2 = (char**)node69Configv;
  #else
    argc = sizeof(exposeGcConfigv)/sizeof(char*) - 1;
    argv2 = (char**)exposeGcConfigv;
  #endif // ENABLE_NODE_V_6_9
  #endif //ENABLE_V8_HEAP_PARAMS
  }
#else
#if ENABLE_V8_HEAP_PARAMS
#ifdef ENABLE_NODE_V_6_9
  static const char *args2   = "rtNode\0--experimental-vm-modules\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[33], &args2[36], NULL};
#else
  rtLogWarn("v8 old heap space configured to 64mb\n");
  static const char *args2   = "rtNode\0--experimental-vm-modules\0--expose-gc\0--max_old_space_size=64\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[33], &args2[45], &args2[69], &args2[72], NULL};
#endif // ENABLE_NODE_V_6_9
#else
#ifdef ENABLE_NODE_V_6_9
   static const char *args2   = "rtNode\0--experimental-vm-modules\0-e\0console.log(\"rtNode Initalized\");\0\0";
   static const char *argv2[] = {&args2[0], &args2[7], &args2[33], &args2[36], NULL};
#else
  static const char *args2   = "rtNode\0--experimental-vm-modules\0--expose-gc\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[33], &args2[45], &args2[48], NULL};
#endif // ENABLE_NODE_V_6_9
#endif //ENABLE_V8_HEAP_PARAMS
  argc   = sizeof(argv2)/sizeof(char*) - 1;
#endif //ENABLE_DEBUG_MODE
  static args_t aa(argc, (char**)argv2);

  s_gArgs = &aa;


  char **argv = aa.argv;

#ifdef RUNINMAIN
#ifdef WIN32
  __rt_main_thread__ = GetCurrentThreadId();
#else
  __rt_main_thread__ = pthread_self(); //  NB
#endif
#endif
  nodePath();


#ifdef ENABLE_NODE_V_6_9
  rtLogWarn("rtNode::rtNode() calling init \n");
  init2(argc, argv);
#else
  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!

  init2(argc, argv);
#endif // ENABLE_NODE_V_6_9
  return RT_OK;
}

rtScriptNode::~rtScriptNode()
{
  // rtLogInfo(__FUNCTION__);
  term();
}

rtError rtScriptNode::pump()
{
//#ifndef RUNINMAIN
//  return;
//#else
#ifdef RUNINMAIN
  // found a problem where if promise triggered by one event loop gets resolved by other event loop.
  // It is causing the dependencies between data running between two event loops failed, if one one 
  // loop didn't complete before other. So, promise not registered by first event loop, before the second
  // event looop sends back the ready event
  if (gIsPumpingJavaScript == false) 
  {
    gIsPumpingJavaScript = true;
#endif
    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.
#ifdef ENABLE_NODE_V_6_9
#ifndef USE_NODE_PLATFORM
    v8::platform::PumpMessageLoop(mPlatform, mIsolate);
#endif //USE_NODE_PLATFORM
#endif //ENABLE_NODE_V_6_9
    mIsolate->RunMicrotasks();
    uv_run(uv_default_loop(), UV_RUN_NOWAIT);//UV_RUN_ONCE);
#ifdef USE_NODE_PLATFORM
    node::MultiIsolatePlatform* platform = static_cast<node::MultiIsolatePlatform*>(mPlatform);
    platform->DrainBackgroundTasks(mIsolate);
#endif //USE_NODE_PLATFORM
    // Enable this to expedite garbage collection for testing... warning perf hit
    if (mTestGc)
    {
      static int sGcTickCount = 0;

      if (sGcTickCount++ > 60)
      {
        Local<Context> local_context = Context::New(mIsolate);
        Context::Scope contextScope(local_context);
        mIsolate->RequestGarbageCollectionForTesting(Isolate::kFullGarbageCollection);
        sGcTickCount = 0;
      }
    }
#ifdef RUNINMAIN
    gIsPumpingJavaScript = false;
  }
#endif
//#endif // RUNINMAIN
  return RT_OK;
}

rtError rtScriptNode::collectGarbage()
{
//#ifndef RUNINMAIN
//  return;
//#else
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  Local<Context> local_context = Context::New(mIsolate);
  Context::Scope contextScope(local_context);
  mIsolate->LowMemoryNotification();
//#endif // RUNINMAIN
  return RT_OK;
}

void* rtScriptNode::getParameter(rtString param)
{
  if (param.compare("isolate") == 0)
    return getIsolate();
  return NULL;
}

#if 0
rtNode::forceGC()
{
  mIsolate->RequestGarbageCollectionForTesting(Isolate::kFullGarbageCollection);
}
#endif

void rtScriptNode::nodePath()
{
  const char* NODE_PATH = ::getenv("NODE_PATH");

  if(NODE_PATH == NULL)
  {
    char cwd[1024] = {};

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
#ifdef WIN32
	  _putenv_s("NODE_PATH", cwd);
#else
	  ::setenv("NODE_PATH", cwd, 1); // last arg is 'overwrite' ... 0 means DON'T !
#endif
	  rtLogInfo("NODE_PATH=%s", cwd);
    }
    else
    {
      rtLogError(" - failed to set NODE_PATH");
    }
  }
}
#ifndef RUNINMAIN
bool rtNode::isInitialized()
{
  //rtLogDebug("rtNode::isInitialized returning %d\n",node_is_initialized);
  return node_is_initialized;
}
#endif

#if 1
void rtScriptNode::init2(int argc, char** argv)
{
  // Hack around with the argv pointer. Used for process.title = "blah".
  argv = uv_setup_args(argc, argv);

  rtLogInfo(__FUNCTION__);

#if 0
#warning Using DEBUG AGENT...
  use_debug_agent = true; // JUNK
#endif

  if(node_is_initialized == false)
  {
    rtLogWarn("About to Init\n");
    Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

#ifdef ENABLE_NODE_V_6_9
   rtLogWarn("using node version %s\n", NODE_VERSION);
   v8::V8::InitializeICU();
   V8::InitializeExternalStartupData(argv[0]);

#ifdef USE_NODE_PLATFORM
   Platform* platform = node::CreatePlatform(0, NULL);
#else
   Platform* platform = platform::CreateDefaultPlatform();
#endif // USE_NODE_PLATFORM
   mPlatform = platform;
  #ifndef USE_NODE_10
    v8::V8::InitializePlatform(platform);
  #endif
   V8::Initialize();
   Isolate::CreateParams params;
   array_buffer_allocator = new ArrayBufferAllocator();
   const char* source1 = "function pxSceneFooFunction(){ return 0;}";
   static v8::StartupData data = v8::V8::CreateSnapshotDataBlob(source1);
   params.snapshot_blob = &data;
   params.array_buffer_allocator = array_buffer_allocator;
   mIsolate     = Isolate::New(params);
   node_isolate = mIsolate; // Must come first !!
#else
    V8::Initialize();
#endif // ENABLE_NODE_V_6_9
    rtLogWarn("rtNode::init() node_is_initialized=%d\n",node_is_initialized);
    node_is_initialized = true;

    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    Local<Context> ctx = Context::New(mIsolate);
    ctx->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, 99));
    mContext.Reset(mIsolate, ctx);
    rtLogWarn("DONE in rtNode::init()\n");
  }
}
#endif

rtError rtScriptNode::term()
{
  rtLogInfo(__FUNCTION__);
  nodeTerminated = true;
#if 0
#ifdef USE_CONTEXTIFY_CLONES
  if( mRefContext.getPtr() )
  {
    mRefContext->Release();
  }
#endif
#endif
  if(node_isolate)
  {
    rtLogWarn("\n++++++++++++++++++ DISPOSE\n\n");
    node_isolate = NULL;
    mIsolate     = NULL;
  }

  if(node_is_initialized)
  {
    //V8::Dispose();

    node_is_initialized = false;

    //V8::ShutdownPlatform();
    if(mPlatform)
    {
#ifdef USE_NODE_PLATFORM
      #ifndef USE_NODE_10
      node::NodePlatform* platform_ = static_cast<node::NodePlatform*>(mPlatform);
      platform_->Shutdown();
      node::FreePlatform(platform_);
      #endif
#else
      delete mPlatform;
#endif // USE_NODE_PLATFORM
      mPlatform = NULL;
    }

  //  if(mPxNodeExtension)
  //  {
  //    delete mPxNodeExtension;
  //    mPxNodeExtension = NULL;
  //  }
  }

  //HandleMap::printAll();

  return RT_OK;
}


inline bool fileExists(const std::string& name)
{
  struct stat buffer;
  return (stat (name.c_str(), &buffer) == 0);
}

rtNodeContextRef rtScriptNode::getGlobalContext() const
{
  return rtNodeContextRef();
}

rtNodeContextRef rtScriptNode::createContext(bool ownThread)
{
  UNUSED_PARAM(ownThread);    // not implemented yet.

  rtNodeContextRef ctxref;

#ifdef USE_CONTEXTIFY_CLONES
  if(mRefContext.getPtr() == NULL)
  {
    mRefContext = new rtNodeContext(mIsolate,mPlatform);
    ctxref = mRefContext;

    static std::string sandbox_path = rtGetRootModulePath(SANDBOX_JS);

    // Populate 'sandbox' vars in JS...
    if(fileExists(sandbox_path.c_str()))
    {
      mRefContext->runFile(sandbox_path.c_str());
    }
    else
    {
      rtLogError("## ERROR:   Could not find \"%s\" ...", sandbox_path.c_str());
    }
    // !CLF: TODO Why is ctxref being reassigned from the mRefContext already assigned?
    //ctxref = new rtNodeContext(mIsolate, mRefContext);
  }
  else
  {
    // rtLogInfo("\n createContext()  >>  CLONE CREATED !!!!!!");
    ctxref = new rtNodeContext(mIsolate, mRefContext); // CLONE !!!
  }
#else
    ctxref = new rtNodeContext(mIsolate,mPlatform);

#endif

  // TODO: Handle refs in map ... don't leak !
  // mNodeContexts[ ctxref->getContextId() ] = ctxref;  // ADD to map

  return ctxref;
}

rtError rtScriptNode::createContext(const char *lang, rtScriptContextRef& ctx)
{
  rtNodeContextRef nodeCtx = createContext();

  ctx = (rtIScriptContext*)nodeCtx.getPtr();
  return RT_OK;
}

unsigned long rtNodeContext::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)
    {
     delete this;
    }
    return l;
}


rtError createScriptNode(rtScriptRef& script)
{
  script = new rtScriptNode(false);
  return RT_OK;
}

#endif // RTSCRIPT_SUPPORT_NODE
