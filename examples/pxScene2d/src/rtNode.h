#ifndef RTNODE_H
#define RTNODE_H


#include "rtObject.h"
#include "rtValue.h"

#include "uv.h"
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include <string>

namespace node
{
class Environment;
}


class rtNode;
class rtNodeContext;

typedef rtRefT<rtNodeContext> rtNodeContextRef;


#define UNUSED_PARAM(x) ((x)=(x))

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

  //  rtStringRef <<< as an OUT parameter
  //
  void add(const char *name, rtValue  const& val);

  rtObjectRef runThread(const char *file);

  rtObjectRef runScript(const char *script);
  rtObjectRef runFile  (const char *file);


  void uvWorker();
  int  startUVThread();


  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release()
  {
    long l = rtAtomicDec(&mRefCount);
    if (l == 0) delete this;
    return l;
  }

  bool           mKillUVWorker;
  
  const char   *js_file;
  std::string   js_script;
  
  pthread_t     js_worker;
  pthread_t     uv_worker;

  rtNode   *node;

private:
  v8::Isolate                   *mIsolate;
  v8::Persistent<v8::Context>    mContext;

  node::Environment*             mEnv;
  v8::Persistent<v8::Object>     rtWrappers;

  void createEnvironment();
 // v8::Persistent<v8::ObjectTemplate>  globalTemplate;

  pthread_t uv_thread;

  int mRefCount;
  uv_timer_t mTimer;

  int startThread(const char *js);
  void startTimers();

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtNode
{
public:
  rtNode();
  rtNode(int argc, char** argv);
  ~rtNode();

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);

  void init(int argc, char** argv);

  v8::Isolate   *getIsolate() { return mIsolate; };
  
private:
  void term();

  void nodePath();

  v8::Isolate   *mIsolate;
  v8::Platform  *mPlatform;

//  v8::Extension *mPxNodeExtension;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

