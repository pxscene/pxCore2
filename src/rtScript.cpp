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

// rtScript.cpp

#include "rtScript.h"

class rtIScript
{
public:
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  
  virtual rtError init();
  virtual rtError createContext(const char* lang, rtScriptContextRef);
  virtual rtError pump();
  virtual rtError collectGarbage();
};

typedef rtRef<rtIScript> rtScriptRef;
 
//#define SUPPORT_NODE
#ifdef RTSCRIPT_USE_NODE
#include "rtScriptNode/rtScriptNode.h"
rtScriptNode gScriptNode(false);
#define gScript_ gScriptNode
#endif

//#define SUPPORT_DUK
#ifdef RTSCRIPT_USE_DUKTAPE
#include "rtScriptDuk/rtScriptDuk.h"
rtScriptDuk gScriptDuk(false);
#define gScript_ gScriptDuk
#endif


rtScript::rtScript()  {}
rtScript::~rtScript() {}

rtError rtScript::init()
{

  gScript_.init();
  return RT_OK;
}

rtError rtScript::term()
{
  return RT_OK;
}

rtError rtScript::pump() 
{
  //gScriptNode.pump();
  gScript_.pump();
  return RT_OK;
}

rtError rtScript::collectGarbage() 
{
  //gScriptNode.collectGarbage();
  gScript_.collectGarbage();
  return RT_OK;
}

#if 0
typedef (createScript)

typedef struct 
{
  char *lang,
  rtScriptRef scriptRef;
} scriptRegistry;

scriptRegistry[] = {
  {"javascript", NULL}
};

#endif

rtError rtScript::createContext(const char *lang, rtScriptContextRef& ctx)
{
  rtError e = RT_FAIL;
  #ifdef RTSCRIPT_USE_DUKTAPE
  rtDukContextRef dukCtx;
  dukCtx = gScriptDuk.createContext();
  if (dukCtx)
  {
    ctx = (rtIScriptContext*)dukCtx.getPtr();
    e = RT_OK;
  }
  #else
  rtNodeContextRef nodeCtx;
  nodeCtx = gScriptNode.createContext();
  if (nodeCtx)
  {
    ctx = (rtIScriptContext*)nodeCtx.getPtr();
    e = RT_OK;
  }  
  #endif
  return e;
}

