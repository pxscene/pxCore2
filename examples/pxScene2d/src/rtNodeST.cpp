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
  // Since this is single threaded version we're always on the js thread
  return true;
}

static inline bool file_exists(const char *file)
{
  struct stat buffer;
  return (stat (file, &buffer) == 0);
}

rtNodeContext::rtNodeContext(v8::Isolate *isolate) :
     mIsolate(isolate), mEnv(NULL), mRefCount(0)
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
}

rtNodeContext::~rtNodeContext()
{
  Locker                locker(mIsolate);
  Isolate::Scope isolate_scope(mIsolate);
  HandleScope     handle_scope(mIsolate);

  if(mEnv)
  {
    RunAtExit(mEnv);

    //mEnv->Dispose();
    mEnv = NULL;
  }

  if(exec_argv)
  {
    delete[] exec_argv;
    exec_argv = NULL;
    exec_argc = 0;
  }

  // TODO:  Might not be needed in ST case...
  //
  // Un-Register wrappers.
  // rtObjectWrapper::destroyPrototype();
  // rtFunctionWrapper::destroyPrototype();

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

  global->Set(String::NewFromUtf8(mIsolate, name), rt2js(mIsolate, val));
}

rtObjectRef rtNodeContext::runScript(const char *script, const char *args /*= NULL*/)
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

std::string readFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;

  src_script << src_file.rdbuf(); // slurp up file

  std::string s = src_script.str();

  return s;
}

rtObjectRef rtNodeContext::runFile(const char *file, const char* /*args = NULL*/)
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


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


rtNode::rtNode() : mPlatform(NULL)
{
                              //0123456 789ABCDEF012 345 67890ABCDEF
  static const char *args2   = "rtNode\0--expose-gc\0-e\0console.log(\"rtNode Intialized\");\0\0";
  static const char *argv2[] = {&args2[0], &args2[7], &args2[19], &args2[22], NULL};
  int          argc   = sizeof(argv2)/sizeof(char*) - 1;

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

void rtNode::pump()
{
  Locker                  locker(mIsolate);
  Isolate::Scope   isolate_scope(mIsolate);
  HandleScope       handle_scope(mIsolate);    // Create a stack-allocated handle scope.
  
  uv_run(uv_default_loop(), UV_RUN_NOWAIT);//UV_RUN_ONCE);

  // Enable this to expedite garbage collection for testing... warning perf hit
  #if 0
  static int i = 0;

  if (i++ > 100)
  {
    mIsolate->RequestGarbageCollectionForTesting(v8::Isolate::kFullGarbageCollection);
    i++;
  }
  #endif

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


