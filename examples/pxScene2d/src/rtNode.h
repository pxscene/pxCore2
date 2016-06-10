#ifndef RTNODE_H
#define RTNODE_H

#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "uv.h"
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

#if 1
 #ifndef WIN32
  #pragma GCC diagnostic pop
 #endif
#endif

//#define USE_CONTEXTIFY_CLONES

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

#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContext(v8::Isolate *isolate, rtNodeContextRef clone_me);
#endif

  ~rtNodeContext();

  void add(const char *name, rtValue  const& val);

  rtObjectRef runScript(const char        *script,  const char *args = NULL); // BLOCKS
  rtObjectRef runScript(const std::string &script,  const char *args = NULL); // BLOCKS
  rtObjectRef runFile  (const char *file,           const char *args = NULL); // BLOCKS

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

  const char   *js_file;
  std::string   js_script;
  
  rtNode   *node;

  v8::Isolate              *getIsolate()      const { return mIsolate; };
  v8::Local<v8::Context>    getLocalContext() const { return PersistentToLocal<v8::Context>(mIsolate, mContext); };

private:
  v8::Isolate                   *mIsolate;
  v8::Persistent<v8::Object>     mSandbox;
  v8::Persistent<v8::Context>    mContext;

  node::Environment*             mEnv;
  v8::Persistent<v8::Object>     mRtWrappers;

  void createEnvironment();

#ifdef USE_CONTEXTIFY_CLONES
  void clonedEnvironment(rtNodeContextRef clone_me);
#endif

  int mRefCount;
  rtAtomic mId;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class rtNode
{
public:
  rtNode();
  ~rtNode();

  void pump();

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);

  v8::Isolate   *getIsolate() { return mIsolate; };
  
private:
  void init(int argc, char** argv);
  void term();

  void nodePath();

  v8::Isolate                   *mIsolate;
  //v8::Platform                  *mPlatform;
   
  v8::Persistent<v8::Context>    mContext;


#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContextRef mRefContext;
#endif

  bool mTestGc;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

