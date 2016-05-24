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
{rtLogInfo("\n rtNode::rtIsMainThread");
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
{rtLogInfo("\n rtNode::uvThread");
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
      fprintf(stderr, "FATAL - No node instance... uvThread() exiting...");
    }
  }

  printf("uvThread() - EXITING...\n");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void *jsThread(void *ptr)
{rtLogInfo("\n rtNode::jsThread");
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

  printf("jsThread() - EXITING...\n");

  return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef  USE_UV_TIMERS

static void timerCallback(uv_timer_t* )
{rtLogInfo("\n rtNode::timerCallback");
#ifdef USE_UV_TIMERS
    #ifdef RUNINMAIN
    if (gLoop)
      gLoop->runOnce();
    #else
    rtLogDebug("uv timer callback");
    #endif
#else
     // Do nothing...
#endif
}

#endif

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static inline bool file_exists(const char *file)
{rtLogInfo("\n rtNode::file_exists");
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


rtNodeContext::rtNodeContext(v8::Isolate *isolate) :
     mKillUVWorker(false),  js_worker(0), uv_worker(0),
     mIsolate(isolate), mEnv(NULL), mRefCount(0)
{rtLogInfo("\n rtNode::rtNodeContext");
  assert(isolate); // MUST HAVE !

  createEnvironment();
}


void rtNodeContext::createEnvironment()
{rtLogInfo("\n rtNode::createEnvironment");
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
}

rtNodeContext::~rtNodeContext()
{rtLogInfo("\n rtNode::~rtNodeContext");
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  if(uv_worker)
  {rtLogInfo("\n rtNode::uv_worker join");
    mKillUVWorker = true; // kill UV loop

    (void) pthread_join(uv_worker, NULL);

    uv_worker = 0;
  }

  if(js_worker)
  {rtLogInfo("\n rtNode::js_worker join");
    (void) pthread_join(js_worker, NULL);

    js_worker = 0;
  }

  if(mEnv)
  {rtLogInfo("\n rtNode::RunAtExit called");
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
{rtLogInfo("\n rtNodeContext::add");
  if(name == NULL)
  {
    rtLogError("no symbolic name for rtValue");
    // TODO: test for uniquiness !
    return;
  }

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
{rtLogInfo("\n rtNodeContext::startTimers");
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

rtObjectRef rtNodeContext::runScript(const char *script, const char *args /*= NULL*/)
{rtLogInfo("\n rtNodeContext::runScript");
  if(script == NULL)
  {
    rtLogError("no script given.");

    return  rtObjectRef(0);// JUNK
  }

  return runScript(std::string(script), args);
}


rtObjectRef rtNodeContext::runScript(const std::string &script, const char *args /*= NULL*/)
{rtLogInfo("\n rtNodeContext::runScript");
  if(script.empty())
  {
    rtLogError(" - no script given.");

    return  rtObjectRef(0);// JUNK
  }

  UNUSED_PARAM(args);

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

//    printf("DEBUG:  %15s()    - RESULT = %s\n", __FUNCTION__, *utf8);  // TODO:  Probably need an actual RESULT return mechanisim
  }//scope

    return rtObjectRef(0);// JUNK
}


rtObjectRef rtNodeContext::runScriptThreaded(const char *script, const char *args /*= NULL*/)
{rtLogInfo("\n rtNodeContext::runScript");
  if(script == NULL)
  {
    rtLogError(" - no script given.");

    return  rtObjectRef(0);// JUNK
  }

  UNUSED_PARAM(args);

  js_script = script;

#ifndef  USE_UV_TIMERS
  // call runScript() in this thread...
  if(pthread_create(&js_worker, NULL, jsThread, (void *) this))
  {rtLogInfo("\n rtNodeContext::pthread_Create::js_worker");
    fprintf(stderr, "Error creating JS thread\n");
  }
#endif

  if(pthread_create(&uv_worker, NULL, uvThread, (void *) this))
  {rtLogInfo("\n rtNodeContext::pthread_Create::uv_worker");
    fprintf(stderr, "Error creating UV thread\n");
  }

  return rtObjectRef(0);
}

std::string readFile(const char *file)
{rtLogInfo("\n rtNodeContext::readFile");
  std::ifstream       src_file(file);
  std::stringstream   src_script;

  src_script << src_file.rdbuf(); // slurp up file

  std::string s = src_script.str();

  return s;
}

rtObjectRef rtNodeContext::runFile(const char *file, const char *args /*= NULL*/)
{rtLogInfo("\n rtNodeContext::runFile");
  UNUSED_PARAM(args);

  if(file == NULL)
  {
    rtLogError(" - no script given.");

    return  rtObjectRef(0);// JUNK
  }

//  printf("\n#### [%p]  %s() >> Running \"%s\"\n", this, __FUNCTION__, file.c_str());

  // Read the script file
  js_script = readFile(file);

  return runScript(js_script);
}


rtObjectRef rtNodeContext::runFileThreaded(const char *file, const char *args /*= NULL*/)
{rtLogInfo("\n rtNodeContext::runFileThreaded");
  UNUSED_PARAM(args);

  if(file == NULL)
  {
    rtLogError(" - no script given.");

    return  rtObjectRef(0);// JUNK
  }

  // Read the script file
  js_script = readFile(file);

  runScriptThreaded(js_script.c_str(), args);

  return rtObjectRef(0);
}


void rtNodeContext::uvWorker()
{rtLogInfo("\n rtNodeContext::uvWorker");
  bool more;

  printf("\n Start >>> %s() !!!! \n", __FUNCTION__);

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

  } while (more == true && mKillUVWorker == false);

  printf("\n End >>> %s() !!!! \n", __FUNCTION__);

  int code = EmitExit(mEnv);

  UNUSED_PARAM(code);

  RunAtExit(mEnv);
}

rtObjectRef rtNodeContext:: runFile(const char *file)  // DEPRECATED
{rtLogInfo("\n rtNodeContext::runFile");
  #warning "runFile() - is DEPRECATED ... going away soon."
  return runFileThreaded(file);
}

rtObjectRef rtNodeContext:: runThread(const char *file)  // DEPRECATED
{rtLogInfo("\n rtNodeContext::runThread");
  #warning "runThread() - is DEPRECATED ... going away soon."
  return runFileThreaded(file);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

rtNode::rtNode() : mPlatform(NULL)
{rtLogInfo("\n rtNodeContext::mPlatform");
  __rt_main_thread__ = pthread_self();

  nodePath();

  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!
}

rtNode::rtNode(int argc, char** argv) : mPlatform(NULL)
{rtLogInfo("\n rtNodeContext::rtNode");
  __rt_main_thread__ = pthread_self(); //  NB

  nodePath();

  mIsolate     = Isolate::New();
  node_isolate = mIsolate; // Must come first !!

  init(argc, argv);
}

rtNode::~rtNode()
{rtLogInfo("\n rtNodeContext::~rtNode");
  term();
}

void rtNode::nodePath()
{rtLogInfo("\n rtNodeContext::NodePath");
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
{rtLogInfo("\n rtNode::init");
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
{rtLogInfo("\n rtNode::term");
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
{rtLogInfo("\n rtNode::getGlobalContext");
  return rtNodeContextRef();
}

rtNodeContextRef rtNode::createContext(bool ownThread)
{rtLogInfo("\n rtNode::createContext");
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
