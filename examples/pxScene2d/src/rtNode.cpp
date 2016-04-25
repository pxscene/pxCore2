// pxCore CopyRight 2007-2015 John Robinson
// rtNode.cpp

#include <unistd.h>
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

#include "env.h"
#include "env-inl.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "pxCore.h"

#include "rtNode.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"


#ifdef RUNINMAIN

#include "pxEventLoop.h"
extern pxEventLoop* gLoop;

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef WIN32
static DWORD __rt_main_thread__;
#else
static pthread_t __rt_main_thread__;
#endif

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
  //printf("uvThread() - ENTER\n");

  if(ptr)
  {
    rtNodeContext *ctx = (rtNodeContext *) ptr;

    if(ctx)
    {
      ctx->uvWorker(); // BLOCKS
    }
    else
    {
      fprintf(stderr, "FATAL - No node instance... uvThread() exiting...");
    }
  }

  printf("uvThread() - EXITING...\n");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *jsThread(void *ptr)
{
  if(ptr)
  {
    rtNodeContext *ctx = (rtNodeContext*) ptr;

    if(ctx)
    {
      ctx->runFile(ctx->js_file);
    }
    else
    {
      fprintf(stderr, "FATAL - No Instance... jsThread() exiting...");
    }

    static bool only_one = false;
    if(only_one == false)
    {
      only_one = true;
      ctx->uvWorker();
    }
  }

  printf("jsThread() - EXITING...\n");

  return NULL;
}

static void timerCallback(uv_timer_t* )
{
    #ifdef RUNINMAIN
    if (gLoop)
      gLoop->runOnce();
    #else
    rtLogDebug("uv timer callback");
    #endif
}

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
     mKillUVWorker(false),  js_worker(0), uv_worker(0),
     mIsolate(isolate), mEnv(NULL), mRefCount(0) //, mTimer() //, mContext(0)
{
  assert(isolate); // MUST HAVE !

  AddRef();

  createEnvironment();
}


void rtNodeContext::createEnvironment()
{  
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

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

  mEnv = CreateEnvironment(
        mIsolate,
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
}

rtNodeContext::~rtNodeContext()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);
  
  if(uv_worker)
  {
    printf("\nINFO: %s() - kill UV loop", __PRETTY_FUNCTION__); fflush(stdout);
   
    mKillUVWorker = true; // kill UV loop
    
    (void) pthread_join(uv_worker, NULL);
  }
  
  if(mEnv)
  {
    RunAtExit(mEnv);

    mEnv->Dispose();
    mEnv = NULL;
  }
  
  if(exec_argv)
  {
    delete[] exec_argv;
    exec_argv = NULL;
    exec_argc = 0;
  }
    
  Release();
  
  mContext.Reset();
  // NOTE: 'mIsolate' is owned by rtNode.  Don't destroy here !
}


void rtNodeContext::add(const char *name, rtValue const& val)
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

//  printf("\n#### [%p]  %s() >> Adding \"%s\"\n", this, __FUNCTION__, name);

  // Get a Local context...
  Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
  Context::Scope context_scope(local_context);

  Handle<Object> global = local_context->Global();

  global->Set(String::NewFromUtf8(mIsolate, name), rt2js(mIsolate, val));
}

void rtNodeContext::startTimers()
{
    rtLogInfo("starting background thread for event loop processing");

    // we start a timer in case there aren't any other evens to the keep the
    // nodejs event loop alive. Fire a time repeatedly.
    uv_timer_init(uv_default_loop(), &mTimer);
    
    #ifdef RUNINMAIN
    uv_timer_start(&mTimer, timerCallback, 0, 5);
    #else
    uv_timer_start(&mTimer, timerCallback, 1000, 1000);
    #endif
}

rtObjectRef rtNodeContext::runThread(const char *file)
{
  if(file_exists(file) == false)
  {
    printf("DEBUG:  %15s()    - NO FILE\n", __FUNCTION__);

    return rtObjectRef(0);  // ERROR
  }

  startTimers();
  startThread(file);

  return rtObjectRef(0);// JUNK

  //return startThread(s.c_str());
}

rtObjectRef rtNodeContext::runScript(const char *script)
{
  {//scope

    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    // Get a Local context...
    Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
    Context::Scope context_scope(local_context);

    Local<String> source = String::NewFromUtf8(mIsolate, script);

    // Compile the source code.
    Local<Script> run_script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = run_script->Run();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(result);

    printf("DEBUG:  %15s()    - RESULT = %s\n", __FUNCTION__, *utf8);  // TODO:  Probably need an actual RESULT return mechanisim

  }//scope

    return rtObjectRef(0);// JUNK
}

int rtNodeContext::startThread(const char *js)
{
  js_file = js;

//  printf("\n startThread() - ENTER\n");

  if(pthread_create(&js_worker, NULL, jsThread, (void *) this))
  {
    fprintf(stderr, "Error creating thread\n");
    return 1;
  }
}


rtObjectRef rtNodeContext::runFile(const char *file)
{
//  int exec_argc = 0;
//  const char** exec_argv;

  printf("\n#### [%p]  %s() >> Running \"%s\"\n", this, __FUNCTION__, file);

  // - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Read the script file
  std::ifstream       js_file(file);
  std::stringstream   src_file;

  src_file << js_file.rdbuf(); // slurp up file

  std::string s = src_file.str();

  {//scope- - - - - - - - - - - - - - -

    Locker                locker(mIsolate);
    Isolate::Scope isolate_scope(mIsolate);
    HandleScope     handle_scope(mIsolate);    // Create a stack-allocated handle scope.

    Local<Context> local_context = node::PersistentToLocal<Context>(mIsolate, mContext);
    Context::Scope context_scope(local_context);

    Local<String> source = String::NewFromUtf8(mIsolate, s.c_str());

    // Compile the source code.
    Local<Script> script = Script::Compile(source);

    // Run the script to get the result.
    Local<Value> result = script->Run();

    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(result);

  }//scope- - - - - - - - - - - - - - -

//  static bool only_one = false;
//  if(only_one == false)
//  {
//    only_one = true;
//    uvWorker();
//  }

  return rtObjectRef(0);
}


void rtNodeContext::uvWorker()
{
  bool more;

  printf("\n Start >>> %s() !!!! \n", __FUNCTION__);

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

  } while (more == true && mKillUVWorker == false);

  printf("\n End >>> %s() !!!! \n", __FUNCTION__);

  int code = EmitExit(mEnv);
  
  UNUSED_PARAM(code);
  
  RunAtExit(mEnv);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtNode::rtNode() : mPlatform(NULL)
{  
  __rt_main_thread__ = pthread_self();
  
  nodePath();

  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!
}

rtNode::rtNode(int argc, char** argv) : mPlatform(NULL)
{
  __rt_main_thread__ = pthread_self();
  
  nodePath();

  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!

  init(argc, argv);
}

rtNode::~rtNode()
{
  term();
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
      printf("\nERROR: failed to set NODE_PATH\n");
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

    V8::Initialize();
    node_is_initialized = true;
  }
}

void rtNode::term()
{
  if(node_is_initialized)
  {
    node_isolate->Dispose();
    node_isolate = NULL;
    mIsolate = NULL;

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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
