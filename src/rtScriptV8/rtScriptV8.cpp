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

// rtNode.cpp

#if defined WIN32
#include <Windows.h>
#include <direct.h>
#define __PRETTY_FUNCTION__ __FUNCTION__
#else
#include <unistd.h>
#endif

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

#include "rtWrapperUtils.h"

#ifndef WIN32
#pragma GCC diagnostic pop
#endif

#include "rtScriptV8.h"


#include "rtCore.h"
#include "rtObject.h"
#include "rtValue.h"
#include "rtAtomic.h"
#include "rtScript.h"


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

#if !defined(WIN32) & !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

class rtV8Context;

typedef rtRef<rtV8Context> rtV8ContextRef;

class rtV8Context: rtIScriptContext  // V8
{
public:
  rtV8Context(v8::Isolate *isolate, v8::Platform* platform);

  virtual ~rtV8Context();

  virtual rtError add(const char *name, const rtValue& val)
  {
     return RT_OK;
  }
  virtual rtValue get(const char *name)
  {
     return rtValue();
  }

  virtual bool    has(const char *name)
  {
     return true;
  }

  virtual rtError runScript(const char        *script, rtValue* retVal = NULL, const char *args = NULL)
  {
     return RT_OK;
  }
  virtual rtError runFile  (const char *file,          rtValue* retVal = NULL, const char *args = NULL)
  {
     return RT_OK;
  }

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

private:
   v8::Isolate                   *mIsolate;
   v8::Platform                  *mPlatform;
   int mRefCount;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<uint32_t, rtV8ContextRef> rtV8Contexts;
typedef std::map<uint32_t, rtV8ContextRef>::const_iterator rtV8Contexts_iterator;

class rtScriptV8: public rtIScript
{
public:
  rtScriptV8();
  virtual ~rtScriptV8();

  unsigned long AddRef()
  {
    return rtAtomicInc(&mRefCount);
  }

  unsigned long Release();

  rtError init() 
  { 
     return RT_OK; 
  }
  rtError term() 
  {
     return RT_OK; 
  }

  rtString engine() { return "v8"; }

  rtError createContext(const char *lang, rtScriptContextRef& ctx)
  {
     return RT_OK;
  }

  rtError pump()
  {
     return RT_OK;
  }

  v8::Isolate   *getIsolate() 
  { 
     return mIsolate; 
  }
  v8::Platform   *getPlatform() 
  { 
     return mPlatform; 
  }

  rtError collectGarbage() 
  { 
     return RT_OK; 
  }
  void* getParameter(rtString param) 
  { 
     return NULL; 
  }

private:
  v8::Isolate                   *mIsolate;
  v8::Platform                  *mPlatform;

  int mRefCount;
};


using namespace v8;

rtV8Context::rtV8Context(Isolate *isolate, Platform *platform) :
     mIsolate(isolate), mRefCount(0), mPlatform(platform)
{
}

rtV8Context::~rtV8Context()
{
}

rtScriptV8::rtScriptV8():mRefCount(0)
{
  mIsolate = NULL;
  mPlatform = NULL;
  init();
}

rtScriptV8::~rtScriptV8()
{
}

unsigned long rtScriptV8::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)
    {
     delete this;
    }
    return l;
}

unsigned long rtV8Context::Release()
{
    long l = rtAtomicDec(&mRefCount);
    if (l == 0)
    {
     delete this;
    }
    return l;
}

rtError createScriptV8(rtScriptRef& script)
{
  script = new rtScriptV8();
  return RT_OK;
}
