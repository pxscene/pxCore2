#ifndef RTNODE_H
#define RTNODE_H

#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"

#include <string>
#include <map>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "uv.h"
#include "include/v8.h"
#include "include/libplatform/libplatform.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

#define SANDBOX_IDENTIFIER  ( (const char*) "_sandboxStuff" )
#define SANDBOX_JS          ( (const char*) "rcvrcore/sandbox.js")

#if 1
 #if !defined(WIN32) & !defined(ENABLE_DFB)
  #pragma GCC diagnostic pop
 #endif
#endif

#define USE_CONTEXTIFY_CLONES

namespace node
{
class Environment;
}

class rtNode;
class rtNodeContext;

typedef rtRefT<rtNodeContext> rtNodeContextRef;

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

  void    add(const char *name, rtValue  const& val);
  rtValue get(const char *name);
  rtValue get(std::string name);
  
  bool    has(const char *name);
  bool    has(std::string name);

  bool   find(const char *name);

  rtObjectRef runScript(const char        *script,  const char *args = NULL); // BLOCKS
  rtObjectRef runScript(const std::string &script,  const char *args = NULL); // BLOCKS
  rtObjectRef runFile  (const char *file,           const char *args = NULL); // BLOCKS

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
  v8::Persistent<v8::Context>    mContext;
  uint32_t                       mContextId;

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

typedef std::map<uint32_t, rtNodeContextRef> rtNodeContexts;
typedef std::map<uint32_t, rtNodeContextRef>::const_iterator rtNodeContexts_iterator;

class rtNode
{
public:
  rtNode();
  ~rtNode();

  void pump();

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);
#ifndef RUNINMAIN
  bool isInitialized();
  bool needsToEnd() { /*printf("needsToEnd returning %d\n",mNeedsToEnd);*/ return mNeedsToEnd;};
  void setNeedsToEnd(bool end) { /*printf("needsToEnd being set to %d\n",end);*/ mNeedsToEnd = end;}
#endif

  v8::Isolate   *getIsolate() { return mIsolate; };
  void garbageCollect();
private:
#ifdef ENABLE_DEBUG_MODE
  void init();
#else
  void init(int argc, char** argv);
#endif
  void term();

  void nodePath();

  v8::Isolate                   *mIsolate;
  //v8::Platform                  *mPlatform;

  v8::Persistent<v8::Context>    mContext;


#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContextRef mRefContext;
#endif

  bool mTestGc;
#ifndef RUNINMAIN
  bool mNeedsToEnd;
#endif

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

