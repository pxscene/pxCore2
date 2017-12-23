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

// rtNode.h

#ifndef RTNODE_H
#define RTNODE_H

#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"



// TODO eliminate std::string
#include <string>
#include <map>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#pragma GCC diagnostic ignored "-Wall"
#endif

#include "uv.h"
#include "v8.h"
#include "libplatform/libplatform.h"

#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

using namespace rtScriptNodeUtils;


#define SANDBOX_IDENTIFIER  ( (const char*) "_sandboxStuff" )
#define SANDBOX_JS          ( (const char*) "rcvrcore/sandbox.js")

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif


#define USE_CONTEXTIFY_CLONES



namespace node
{
class Environment;
}

class rtScriptNode;
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

class rtNodeContext: rtIScriptContext  // V8
{
public:
  rtNodeContext(v8::Isolate *isolate, v8::Platform* platform);
#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContext(v8::Isolate *isolate, rtNodeContextRef clone_me);
#endif

 virtual ~rtNodeContext();

  virtual rtError add(const char *name, const rtValue& val);
  virtual rtValue get(const char *name);
  //rtValue get(std::string name);

  virtual bool    has(const char *name);
  bool    has(std::string name);

  //bool   find(const char *name);  //DEPRECATED

  virtual rtError runScript(const char        *script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  //rtError runScript(const std::string &script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  virtual rtError runFile  (const char *file,          rtValue* retVal = NULL, const char *args = NULL); // BLOCKS

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
  v8::Platform                  *mPlatform;
  void* mContextifyContext;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint32_t, rtNodeContextRef> rtNodeContexts;
typedef std::map<uint32_t, rtNodeContextRef>::const_iterator rtNodeContexts_iterator;

class rtScriptNode
{
public:
  rtScriptNode();
  rtScriptNode(bool initialize);
  ~rtScriptNode();
    
  void init();

  void pump();

  rtNodeContextRef getGlobalContext() const;
  rtNodeContextRef createContext(bool ownThread = false);

#if 0
#ifndef RUNINMAIN
  bool isInitialized();
  bool needsToEnd() { /*rtLogDebug("needsToEnd returning %d\n",mNeedsToEnd);*/ return mNeedsToEnd;};
  void setNeedsToEnd(bool end) { /*rtLogDebug("needsToEnd being set to %d\n",end);*/ mNeedsToEnd = end;}
#endif
#endif

  v8::Isolate   *getIsolate() { return mIsolate; };
  v8::Platform   *getPlatform() { return mPlatform; };
  void collectGarbage();
private:
#if 0
#ifdef ENABLE_DEBUG_MODE
  void init();
#else
  void init(int argc, char** argv);
#endif
#endif

  void term();

  void nodePath();

  v8::Isolate                   *mIsolate;
  v8::Platform                  *mPlatform;
  v8::Persistent<v8::Context>    mContext;


#ifdef USE_CONTEXTIFY_CLONES
  rtNodeContextRef mRefContext;
#endif

  bool mTestGc;
#ifndef RUNINMAIN
  bool mNeedsToEnd;
#endif

  void init2();
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

