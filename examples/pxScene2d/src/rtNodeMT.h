#ifndef RTNODE_H
#define RTNODE_H

#include "rtObject.h"
#include "rtValue.h"

#include <string>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "uv.h"
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#if 1
#ifndef WIN32
#pragma GCC diagnostic pop
#endif
#endif

namespace node
{
  class Environment;
}

class rtNode;
class rtNodeContext;

typedef rtRefT<rtNodeContext> rtNodeContextRef;


#define UNUSED_PARAM(x) ((void)x)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef struct args_
{
  int    argc;
  char **argv;

  args_() { argc = 0; argv = NULL; }
  args_(int n = 0, char** a = NULL) : argc(n), argv(a) {}
}
args_t;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtNodeContext  // V8
{
public:
  rtNodeContext(v8::Isolate *isolate);
  ~rtNodeContext();

  void add(const char *name, rtValue  const& val);

  rtObjectRef runScript(const char        *script,  const char *args = NULL); // BLOCKS
  rtObjectRef runScript(const std::string &script,  const char *args = NULL); // BLOCKS
  rtObjectRef runFile  (const char *file,           const char *args); // BLOCKS

  rtObjectRef runScriptThreaded(const char *script, const char *args = NULL); // THREADED
  rtObjectRef runFileThreaded(const char *file,     const char *args = NULL); // THREADED

  rtObjectRef runFile(  const char *file);  // DEPRECATED
  rtObjectRef runThread(const char *file);  // DEPRECATED

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release()
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)  delete this;
    return l;
  }

  void uvWorker();

  bool             mKillUVWorker;

  const char      *js_file;
  std::string      js_script;

  rtNode          *node;

private:
  v8::Isolate                   *mIsolate;
  v8::Persistent<v8::Context>    mContext;

  node::Environment*             mEnv;
  v8::Persistent<v8::Object>     rtWrappers;

  uv_timer_t       mTimer;

  pthread_t        js_worker;
  pthread_mutex_t  js_mutex;
  pthread_mutexattr_t js_mutex_attr;

  pthread_t        uv_worker;
  pthread_mutex_t  uv_mutex;
  pthread_mutexattr_t uv_mutex_attr;


  bool createThread_UV();
  bool   killThread_UV();

  bool createThread_JS();
  bool   killThread_JS();

  void startTimers();

  void createEnvironment();

  int mRefCount;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtNode
{
public:
  rtNode();
  ~rtNode();

  void pump(); //DUMMY for now

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);

  v8::Isolate   *getIsolate() { return mIsolate; };

private:
  void init(int argc, char** argv);
  void term();

  void nodePath();

  v8::Isolate   *mIsolate;
  v8::Platform  *mPlatform;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

