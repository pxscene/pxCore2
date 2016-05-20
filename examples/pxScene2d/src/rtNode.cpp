// pxCore CopyRight 2007-2015 John Robinson
// rtNode.cpp

#include <unistd.h>
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

#include "rtObject.h"
#include "rtValue.h"

#include "rtNode.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

// #define USE_UV_TIMERS

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
  extern bool use_debug_agent;
  extern bool debug_wait_connect;
}

static int exec_argc;
static const char** exec_argv = NULL;


args_t *s_gArgs;


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
      while(ctx->mKillUVWorker == false)
      {
        ctx->uvWorker(); // BLOCKS
      }
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
    #ifdef RUNINMAIN
    if (gLoop)
      gLoop->runOnce();
    #else
      rtLogDebug("uv timer callback");
    #endif
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
     mKillUVWorker(false), mIsolate(isolate), mEnv(NULL), mRefCount(0), 
     js_worker(0), uv_worker(0)
     
{
  assert(isolate); // MUST HAVE !

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
#if 0
  #ifdef  USE_UV_TIMERS

    rtLogError("... Start  UV TIMERS ...");

    startTimers(); // keep alive

    createThread_UV();

  #endif
#endif

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

    mEnv->Dispose();
    mEnv = NULL;
  }

  if(exec_argv)
  {
    delete[] exec_argv;
    exec_argv = NULL;
    exec_argc = 0;
  }

  // Un-Register wrappers.
    rtObjectWrapper::destroyPrototype();
  rtFunctionWrapper::destroyPrototype();

  Release();

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

  global->Set(String::NewFromUtf8(mIsolate, name), rt2js(mIsolate, val));
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

#ifndef  USE_UV_TIMERS
 // createThread_UV();
#endif

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
  rtLogDebug(" %s  ... ENTER",__PRETTY_FUNCTION__);

  if(script == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  js_script = script;

#ifndef  USE_UV_TIMERS
  createThread_JS();
#endif

  createThread_UV();

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
  if(file == NULL)
  {
    rtLogError(" %s  ... no script given.",__PRETTY_FUNCTION__);

    return  rtObjectRef(0);// JUNK
  }

  // Read the script file
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
    pthread_mutex_init(&uv_mutex, NULL);

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

    rtLogDebug(" %s - Kill JS - DONE", __PRETTY_FUNCTION__); // JUNK
  }

  return true;
}


bool rtNodeContext::createThread_JS()
{
  if(js_worker == 0)  // only once
  {
    if(pthread_create(&js_worker, NULL, jsThread, (void *) this))
    {
      rtLogError(" %s  ... Error creating JS thread",__PRETTY_FUNCTION__);

      return false;
    }

    rtLogDebug(" %s  ... Created  JS_THREAD ...",__PRETTY_FUNCTION__);

    pthread_mutex_init(&js_mutex, NULL);
  }

  return true;
}


void rtNodeContext::uvWorker()
{
  rtLogDebug(" %s - ENTER", __PRETTY_FUNCTION__);

  bool more;

#ifndef  USE_UV_TIMERS

  // we start a timer in case there aren't any other evens to the keep the
  // nodejs event loop alive. Fire a time repeatedly.
  uv_timer_init(uv_default_loop(), &mTimer);

  // TODO experiment crank up the timers so we can pump cocoa messages on main thread
  #ifdef RUNINMAIN
    uv_timer_start(&mTimer, timerCallback, 0, 5);
  #else
    uv_timer_start(&mTimer, timerCallback, 1000, 1000);
  #endif

#endif //  USE_UV_TIMERS

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
  static const char *args2   = "rtNode\0--expose-gc\0-e\0console.log(\"rtNode Intialized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[19], &args2[22], NULL};
//  static const char *argv2[] = {&args2[0], &args2[7], NULL};
  int                 argc   = sizeof(argv2)/sizeof(char*) - 1;

  static args_t aa(argc, (char**)argv2);

  s_gArgs = &aa;


  char **argv = aa.argv;

  __rt_main_thread__ = pthread_self(); //  NB

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

    V8::Initialize();
    node_is_initialized = true;
  }
}

void rtNode::term()
{
  if(node_isolate)
  {
// JRJRJR  Causing crash???  ask Hugh
    node_isolate->Dispose();
    node_isolate = NULL;
    mIsolate = NULL;
  }

  if(node_is_initialized ) //&& false)
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
