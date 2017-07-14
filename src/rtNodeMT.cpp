/*

 rtCore Copyright 2005-2017 John Robinson

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

#if defined WIN32
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#include <unistd.h>
#endif
#include <stdio.h>
#include <errno.h>

#include <fstream>

#include <iostream>
#include <sstream>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "node.h"
#include "node_javascript.h"

#include "env.h"
#include "env-inl.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "rtNode.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

#define USE_UV_TIMERS

#ifndef RUNINMAIN

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
  extern bool use_debug_agent;
  extern bool debug_wait_connect;
}

static int exec_argc;
static const char** exec_argv;
static rtAtomic sNextId = 100;


args_t *s_gArgs;

extern rtNode script;

rtNodeContexts  mNodeContexts;

#ifdef ENABLE_NODE_V_6_9
ArrayBufferAllocator* array_buffer_allocator = NULL;
bool bufferAllocatorIsSet = false;
#endif
bool nodeTerminated = false;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
  static DWORD __rt_main_thread__;
#else
  static pthread_t __rt_main_thread__;
#endif

// rtIsMainThread() - Previously:  identify the MAIN thread of 'node' which running JS code.
//
// rtIsMainThread() - Currently:   identify BACKGROUND thread which running JS code.
//
bool rtIsMainThread()
{
#ifdef WIN32
  return GetCurrentThreadId() == __rt_main_thread__;
#else
//  return pthread_self() == __rt_main_thread__;
  return pthread_self() != __rt_main_thread__;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *uvThread(void *ptr)
{
  rtLogDebug(" - uvThread ENTER...");

  if(ptr)
  {
    rtNodeContext *ctx = (rtNodeContext *) ptr;

    if(ctx)
    {
      ctx->uvWorker(); // BLOCKS
    }
    else
    {
      rtLogError(" .. FATAL - No node instance... ");
    }
  }

  rtLogDebug(" - uvThread EXITING ...");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *jsThread(void *ptr)
{
  rtLogDebug(" - jsThread ENTER...");

  if(ptr)
  {
    rtNodeContext *ctx = (rtNodeContext*) ptr;

    if(ctx)
    {
      ctx->runScript(ctx->js_script);
    }
    else
    {
      fprintf(stderr, "FATAL - No Instance... jsThread() exiting...");
      return NULL;
    }
  }

  rtLogError(" - jsThread EXITING...");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef  USE_UV_TIMERS

  #include "pxEventLoop.h"

  extern pxEventLoop* gLoop;

  static void timerCallback(uv_timer_t* )
  {
//    #ifdef RUNINMAIN
//    if (gLoop)
//      gLoop->runOnce();
//    #else
//      rtLogDebug("uv timer callback");
//    #endif
  }
#else

  static void timerCallback(uv_timer_t* )
  {
    // DUMMY
  }

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool file_exists(const char *file)
{
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


rtNodeContext::rtNodeContext(v8::Isolate *isolate) :
     mKillUVWorker(false), mIsolate(isolate), mEnv(NULL), 
     js_worker(0), uv_worker(0), mRefCount(0)
     
{
  assert(isolate); // MUST HAVE !

  mId = rtAtomicInc(&sNextId);

  createEnvironment();
}


void rtNodeContext::createEnvironment()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

#ifdef ENABLE_NODE_V_6_9
  Local<Context> local_context = Context::New(mIsolate);

  local_context->SetEmbedderData(HandleMap::kContextIdIndex, Integer::New(mIsolate, mId));

  mContextId = GetContextId(local_context);

  mContext.Reset(mIsolate, local_context); // local to persistent

  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  // Create Environment.

  mEnv = CreateEnvironment(mIsolate,
                           uv_default_loop(),
                           local_context,
                           s_gArgs->argc,
                           s_gArgs->argv,
                           exec_argc,
                           exec_argv);

   array_buffer_allocator->set_env(mEnv);

  mIsolate->SetAbortOnUncaughtExceptionCallback(
        ShouldAbortOnUncaughtException);

  // Load Environment.
  {
    Environment::AsyncCallbackScope callback_scope(mEnv);
    LoadEnvironment(mEnv);
  }

    rtObjectWrapper::exportPrototype(mIsolate, global);
    rtFunctionWrapper::exportPrototype(mIsolate, global);

    {
      SealHandleScope seal(mIsolate);
      bool more = uv_run(mEnv->event_loop(), UV_RUN_ONCE);
      if (more == false)
      {
        EmitBeforeExit(mEnv);
      }
    }
#else
  // Create a new context.
  mContext.Reset(mIsolate, Context::New(mIsolate));

  // Get a Local context.
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  // Register wrappers.
     rtObjectWrapper::exportPrototype(mIsolate, global);
   rtFunctionWrapper::exportPrototype(mIsolate, global);

  rtWrappers.Reset(mIsolate, global);

  mEnv = CreateEnvironment(mIsolate,
                           uv_default_loop(),
                           local_context,
                           s_gArgs->argc,
                           s_gArgs->argv,
                           exec_argc,
                           exec_argv);

  // Start debug agent when argv has --debug
  if (use_debug_agent)
  {
    StartDebug(mEnv, debug_wait_connect);
  }

  LoadEnvironment(mEnv);

  // Enable debugger
  if (use_debug_agent)
  {
    EnableDebug(mEnv);
  }
#endif //ENABLE_NODE_V_6_9
}

rtNodeContext::~rtNodeContext()
{
  killThread_UV();
  killThread_JS();

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  if(mEnv)
  {
   // EmitExit(mEnv);
    RunAtExit(mEnv);

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
    // mEnv->Dispose();
#endif // ENABLE_NODE_V_6_9
    mEnv = NULL;
  }

  if(exec_argv)
  {
    delete[] exec_argv;
    exec_argv = NULL;
    exec_argc = 0;
  }

  // TODO:  Next steps ... re-add? 
  //
  // Un-Register wrappers.
//    rtObjectWrapper::destroyPrototype();
//  rtFunctionWrapper::destroyPrototype();

  // Release();

  mContext.Reset();
  // NOTE: 'mIsolate' is owned by rtNode.  Don't destroy here !
}


void rtNodeContext::add(const char *name, rtValue const& val)
{
  if(name == NULL)
  {
    rtLogError(" %s  ... no symbolic name for rtValue.",__PRETTY_FUNCTION__);
    // TODO: test for uniquiness !
    return;
  }

  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  // Get a Local context...
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  local_context->Global()->Set( String::NewFromUtf8(mIsolate, name), rt2js(local_context, val));
}

rtValue rtNodeContext::get(std::string name)
{
  return get( name.c_str() );
}

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

bool rtNodeContext::has(std::string name)
{
  return has( name.c_str() );
}

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
     printf("\n ## has() - HasCaught()  ... ERROR");
     return false;
  }

  // No need to check if |value| is empty because it's taken care of
  // by TryCatch above.

  return ( !value->IsUndefined() && !value->IsNull() );
}

bool rtNodeContext::find(const char *name)
{    
  rtNodeContexts_iterator it = mNodeContexts.begin();
   
  while(it != mNodeContexts.end())
  {
    rtNodeContextRef ctx = it->second;
     
    printf("\n ######## CONTEXT !!! ID: %d  %s  '%s'",
      ctx->getContextId(), 
      (ctx->has(name) ? "*HAS*" : "does NOT have"),
      name);
     
    it++;
  }
  
  printf("\n ");

  return false;
}

void rtNodeContext::startTimers()
{
#ifdef  USE_UV_TIMERS

  rtLogInfo("starting background thread for event loop processing");

  // we start a timer in case there aren't any other evens to the keep the
  // nodejs event loop alive. Fire a time repeatedly.
  uv_timer_init(uv_default_loop(), &mTimer);

  #ifdef RUNINMAIN
  uv_timer_start(&mTimer, timerCallback, 0, 5);
  #else
  uv_timer_start(&mTimer, timerCallback, 1000, 1000);
  #endif

#endif
}

rtObjectRef rtNodeContext::runScript(const char *script, const char* args /*= NULL*/)
{
  if(script == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  rtLogDebug(" %s  ... Running...",__PRETTY_FUNCTION__);

  return runScript(std::string(script), args);
}

rtObjectRef rtNodeContext::runScript(const std::string &script, const char* /* args = NULL*/)
{
  if(script.empty())
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  createThread_UV();

  {//scope
    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    // Get a Local context...
    Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
    Context::Scope context_scope(local_context);

    Local<String> source = String::NewFromUtf8(mIsolate, script.c_str());

    // Compile the source code.
    Local<Script> run_script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = run_script->Run();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(result);
  }//scope

  return rtObjectRef(0);// JUNK
}


rtObjectRef rtNodeContext::runScriptThreaded(const char *script, const char* /* args = NULL*/)
{
  rtLogDebug(" %s - ENTER", __PRETTY_FUNCTION__);

  if(script == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  js_script = script;

  createThread_JS();

  return rtObjectRef(0);
}

std::string readFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;

  src_script << src_file.rdbuf(); // slurp up file

  std::string s = src_script.str();

  return s;
}

rtObjectRef rtNodeContext::runFile(const char *file, const char* /* args = NULL*/)
{
  rtLogDebug(" %s - ENTER", __PRETTY_FUNCTION__);

  if(file == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  // Read the script file
  js_file   = file;
  js_script = readFile(file);

  return runScript(js_script);
}


rtObjectRef rtNodeContext::runFileThreaded(const char *file, const char* args /*= NULL*/)
{
  rtLogDebug(" %s - ENTER", __PRETTY_FUNCTION__);

  if(file == NULL)
  {
     rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  // Read the script file
  js_script = readFile(file);

  runScriptThreaded(js_script.c_str(), args);

  return rtObjectRef(0);
}


bool rtNodeContext::createThread_UV()
{
  if(uv_worker == 0) // only once
  {
    pthread_mutexattr_init(&uv_mutex_attr);
    pthread_mutexattr_settype(&uv_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&uv_mutex, &uv_mutex_attr);

    if(pthread_create(&uv_worker, NULL, uvThread, (void *) this))
    {
      rtLogError(" %s  ... Error creating UV thread",__PRETTY_FUNCTION__);
      return false;
    }

    rtLogDebug(" %s  ... Created  UV_THREAD ...",__PRETTY_FUNCTION__);
  }
  return true;
}

bool rtNodeContext::killThread_UV()
{
  if(uv_worker)
  {
    rtLogError(" %s - Kill UV ", __PRETTY_FUNCTION__); // JUNK

    pthread_mutex_lock(&uv_mutex);   // ##
    mKillUVWorker = true;            // ## kill UV loop
    pthread_mutex_unlock(&uv_mutex); // ##

    uv_stop(mEnv->event_loop());

    (void) pthread_join(uv_worker, NULL);

    uv_worker = 0;
    pthread_mutex_destroy(&uv_mutex);

    rtLogDebug(" %s - Kill UV - DONE", __PRETTY_FUNCTION__); // JUNK
  }

  return true;
}


bool rtNodeContext::killThread_JS()
{
  if(js_worker)
  {
    rtLogError(" %s - Kill JS ", __PRETTY_FUNCTION__); // JUNK

    (void) pthread_join(js_worker, NULL);

    js_worker = 0;
    pthread_mutex_destroy(&js_mutex);
    pthread_mutexattr_destroy(&js_mutex_attr);

    rtLogDebug(" %s - Kill JS - DONE", __PRETTY_FUNCTION__); // JUNK
  }

  return true;
}


bool rtNodeContext::createThread_JS()
{
  if(js_worker == 0)  // only once
  {
    pthread_mutexattr_init(&js_mutex_attr);
    pthread_mutexattr_settype(&js_mutex_attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&js_mutex, &js_mutex_attr);

    if(pthread_create(&js_worker, NULL, jsThread, (void *) this))
    {
      rtLogError(" %s  ... Error creating JS thread",__PRETTY_FUNCTION__);

      return false;
    }

    rtLogDebug(" %s  ... Created  JS_THREAD ...",__PRETTY_FUNCTION__);
  }

  return true;
}


void rtNodeContext::uvWorker()
{
  rtLogDebug(" %s - ENTER", __PRETTY_FUNCTION__);

  bool more;

  startTimers();

  do
  {
    {//scope- - - - - - - - - - - - - - -
      Locker                  locker(mIsolate);
      Isolate::Scope   isolate_scope(mIsolate);
      HandleScope       handle_scope(mIsolate);    // Create a stack-allocated handle scope.

      Local<Context>   local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
      Context::Scope   context_scope(local_context);

      more = uv_run(mEnv->event_loop(), UV_RUN_ONCE);

      if (more == false)
      {
        EmitBeforeExit(mEnv);

        // Emit `beforeExit` if the loop became alive either after emitting
        // event, or after running some callbacks.
        more = uv_loop_alive(mEnv->event_loop());

        if (uv_run(mEnv->event_loop(), UV_RUN_NOWAIT) != 0)
        {
          more = true;
        }
      }
    }//scope - - - - - - - - - - - - - - -

    pthread_mutex_lock(&uv_mutex);     // ##
    if(mKillUVWorker == true)
    {
      pthread_mutex_unlock(&uv_mutex);   // ##

      rtLogDebug(" %s - told to EXIT  ", __PRETTY_FUNCTION__);
      break;
    }
    pthread_mutex_unlock(&uv_mutex);   // ##
  } while (more == true);

  uv_timer_stop(&mTimer);

  int code = EmitExit(mEnv);

  UNUSED_PARAM(code);

  RunAtExit(mEnv);

  rtLogDebug(" %s - EXIT  ", __PRETTY_FUNCTION__);
}

#if 1
rtObjectRef rtNodeContext:: runFile(const char *file)  // DEPRECATED
{
  #warning "runFile() - is DEPRECATED ... going away soon."
  return runFileThreaded(file);
}

rtObjectRef rtNodeContext:: runThread(const char *file)  // DEPRECATED
{
  #warning "runThread() - is DEPRECATED ... going away soon."
  return runFileThreaded(file);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtNode::rtNode() : mPlatform(NULL)
{
                              //0123456 789ABCDEF012 345 67890ABCDEF
#if ENABLE_V8_HEAP_PARAMS
#ifdef ENABLE_NODE_V_6_9
  static const char *args2   = "rtNode\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[10], NULL};
#else
  printf("v8 old heap space configured to 64mb\n");
  static const char *args2   = "rtNode\0--expose-gc\0--max_old_space_size=64\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[19], &args2[43], &args2[46], NULL};
#endif // ENABLE_NODE_V_6_9
#else
#ifdef ENABLE_NODE_V_6_9
  static const char *args2   = "rtNode\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[10], NULL};
#else
  static const char *args2   = "rtNode\0--expose-gc\0-e\0console.log(\"rtNode Initalized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[19], &args2[22], NULL};
#endif // ENABLE_NODE_V_6_9
#endif //ENABLE_V8_HEAP_PARAMS
  int                 argc   = sizeof(argv2)/sizeof(char*) - 1;

  static args_t aa(argc, (char**)argv2);

  s_gArgs = &aa;


  char **argv = aa.argv;

  __rt_main_thread__ = pthread_self(); //  NB

  nodePath();

#ifdef ENABLE_NODE_V_6_9
  init(argc, argv);
#else
  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!

  init(argc, argv);
#endif // ENABLE_NODE_V_6_9
}

rtNode::~rtNode()
{
  term();
}

void rtNode::pump()
{
  // DUMMY
}

void rtNode::nodePath()
{
  const char* NODE_PATH = ::getenv("NODE_PATH");

  if(NODE_PATH == NULL)
  {
    char cwd[1024] = {};

    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
      ::setenv("NODE_PATH", cwd, 1); // last arg is 'overwrite' ... 0 means DON'T !

      //printf("\n\n INFO: NODE_PATH = [%s] << NEW\n", cwd);
    }
    else
    {
      rtLogError(" - failed to set NODE_PATH");
    }
  }
  // else
  // {
  //    printf("\nINFO:  NODE_PATH =  [%s]  <<<<< ALREADY SET !\n", NODE_PATH);
  // }
}

void rtNode::init(int argc, char** argv)
{
  // Hack around with the argv pointer. Used for process.title = "blah".
  argv = uv_setup_args(argc, argv);

 // use_debug_agent = true; // JUNK

  if(node_is_initialized == false)
  {
    Init(&argc, const_cast<const char**>(argv), &exec_argc, &exec_argv);

//    mPlatform = platform::CreateDefaultPlatform();
//    V8::InitializePlatform(mPlatform);

#ifdef ENABLE_NODE_V_6_9
   printf("using node version 6.9\n");
   V8::InitializeICU();
   V8::InitializeExternalStartupData(argv[0]);
   Platform* platform = platform::CreateDefaultPlatform();
   V8::InitializePlatform(platform);
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
    node_is_initialized = true;
  }
}

void rtNode::term()
{
  nodeTerminated = true;
  if(node_isolate)
  {
// JRJRJR  Causing crash???  ask Hugh
//    node_isolate->Dispose();
    node_isolate = NULL;
    mIsolate = NULL;
  }

  if(node_is_initialized )
  {
    V8::Dispose();

    node_is_initialized = false;

  //  V8::ShutdownPlatform();
  //  if(mPlatform)
  //  {
  //    delete mPlatform;
  //    mPlatform = NULL;
  //  }

  //  if(mPxNodeExtension)
  //  {
  //    delete mPxNodeExtension;
  //    mPxNodeExtension = NULL;
  //  }
  }
}

inline bool fileExists(const std::string& name)
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

rtNodeContextRef rtNode::getGlobalContext() const
{
  return rtNodeContextRef();
}

rtNodeContextRef rtNode::createContext(bool ownThread)
{
  UNUSED_PARAM(ownThread);    // not implemented yet.

  rtNodeContextRef ctxref = new rtNodeContext(mIsolate);

  ctxref->node = this;

  return ctxref;
}

void rtNode::garbageCollect()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

  mIsolate->LowMemoryNotification();
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


