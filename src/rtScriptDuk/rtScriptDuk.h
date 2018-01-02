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

#ifndef RTSCRIPTDUK_H
#define RTSCRIPTDUK_H

#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"

#include "rtScript.h"

// TODO eliminate std::string
#include <string>
#include <map>

extern "C" {
#include "duv.h"
}

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifndef PX_PLATFORM_MAC
#ifndef __clang__
#pragma GCC diagnostic ignored "-Werror"
#endif
#endif

#pragma GCC diagnostic ignored "-Wall"
#endif

#include "uv.h"
#include "include/libplatform/libplatform.h"

#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

#define SANDBOX_IDENTIFIER  ( (const char*) "_sandboxStuff" )
#define SANDBOX_JS          ( (const char*) "rcvrcore/sandbox.js")

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

#define USE_CONTEXTIFY_CLONES

class rtScriptDuk;
class rtDukContext;

typedef rtRef<rtDukContext> rtDukContextRef;

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

class rtDukContext: rtIScriptContext
{
public:
  rtDukContext();
#ifdef USE_CONTEXTIFY_CLONES
  rtDukContext(rtDukContextRef clone_me);
#endif

  ~rtDukContext();

  rtError add(const char *name, rtValue  const& val);
  rtValue get(const char *name);
  //rtValue get(std::string name);

  bool    has(const char *name);
  //bool    has(std::string name);

  //bool   find(const char *name);  //DEPRECATED

  rtError runScript(const char        *script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  //rtError runScript(const std::string &script, rtValue* retVal = NULL, const char *args = NULL); // BLOCKS
  rtError runFile  (const char *file,          rtValue* retVal = NULL, const char *args = NULL); // BLOCKS

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  const char   *js_file;
  std::string   js_script;

  duk_context              *dukCtx;
  uv_loop_t                *uvLoop;

private:
  void createEnvironment();

#ifdef USE_CONTEXTIFY_CLONES
  void clonedEnvironment(rtDukContextRef clone_me);
#endif

  int mRefCount;
  rtAtomic mId;
  void* mContextifyContext;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint32_t, rtDukContextRef> rtDukContexts;
typedef std::map<uint32_t, rtDukContextRef>::const_iterator rtNodeContexts_iterator;

class rtScriptDuk
{
public:
  rtScriptDuk();
  rtScriptDuk(bool initialize);
  void init();
  ~rtScriptDuk();

  void pump();

  rtDukContextRef getGlobalContext() const;
  rtDukContextRef createContext(bool ownThread = false);
#ifndef RUNINMAIN
  bool isInitialized();
  bool needsToEnd() { /*rtLogDebug("needsToEnd returning %d\n",mNeedsToEnd);*/ return mNeedsToEnd;};
  void setNeedsToEnd(bool end) { /*rtLogDebug("needsToEnd being set to %d\n",end);*/ mNeedsToEnd = end;}
#endif

  //std::string name() const;

  void collectGarbage();
private:
#ifdef ENABLE_DEBUG_MODE
  void init2();
#else
  void init2(int argc, char** argv);
#endif
  void term();

  void nodePath();

  duk_context                   *dukCtx;
  std::vector<uv_loop_t *>       uvLoops;
  uv_thread_t                    dukTid;
  bool                           node_is_initialized;

#ifdef USE_CONTEXTIFY_CLONES
  rtDukContextRef mRefContext;
#endif

  bool mTestGc;
#ifndef RUNINMAIN
  bool mNeedsToEnd;
#endif

};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif // RTNODE_H

