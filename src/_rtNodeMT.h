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

// rtNodeMT.h

#ifndef RTNODE_H
#define RTNODE_H

#include "rtObject.h"
#include "rtValue.h"

#include <string>
#include <map>

#ifndef WIN32
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include "uv.h"
#include "v8.h"
#include "libplatform/libplatform.h"

#include "jsbindings/rtObjectWrapper.h"
#include "jsbindings/rtFunctionWrapper.h"

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

typedef rtRef<rtNodeContext> rtNodeContextRef;


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
  rtValue get(const char *name);
  rtValue get(std::string name);

  bool    has(const char *name);
  bool    has(std::string name);

  bool   find(const char *name);

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

  unsigned long Release();

  v8::Isolate              *getIsolate()      const { return mIsolate; };
  v8::Local<v8::Context>    getLocalContext() const { return PersistentToLocal<v8::Context>(mIsolate, mContext); };

  uint32_t                  getContextId()    const { return mContextId; };

  void uvWorker();

  bool             mKillUVWorker;

  const char      *js_file;
  std::string      js_script;

  rtNode          *node;

private:
  v8::Isolate                   *mIsolate;
  v8::Persistent<v8::Context>    mContext;
  uint32_t                       mContextId;

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

  void pump(); //DUMMY for now

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);

  v8::Isolate   *getIsolate() { return mIsolate; };
  void garbageCollect();

private:
  void init(int argc, char** argv);
  void term();

  void nodePath();

  v8::Isolate   *mIsolate;
  v8::Platform  *mPlatform;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

