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

// rtScript.h

#ifndef _RT_SCRIPT_H
#define _RT_SCRIPT_H

#include "rtError.h"
#include "rtValue.h"
#include "rtRef.h"

bool rtWrapperSceneUpdateHasLock();
void rtWrapperSceneUpdateEnter();
void rtWrapperSceneUpdateExit();

class rtIScriptContext
{
public:
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;
  
  virtual rtError add(const char *name, const rtValue& val) = 0;
  virtual rtValue get(const char *name) = 0;

  virtual bool    has(const char *name) = 0;

  virtual rtError runScript(const char *script, rtValue* retVal = NULL, const char *args = NULL) = 0;
  virtual rtError runFile  (const char *file,   rtValue* retVal = NULL, const char *args = NULL) = 0;
};

typedef rtRef<rtIScriptContext> rtScriptContextRef;

class rtIScript
{
public:
  virtual unsigned long AddRef() = 0;
  virtual unsigned long Release() = 0;

  virtual rtError init() = 0;
  virtual rtError term() = 0;

  virtual rtString engine() = 0;
  
  virtual rtError createContext(const char *lang, rtScriptContextRef& ctx) = 0;

  virtual rtError pump() = 0;

  virtual rtError collectGarbage() = 0;
};

typedef rtRef<rtIScript> rtScriptRef;

class rtScript
{
public:
  rtScript();
  ~rtScript();

  rtError init();
  rtError term();

  rtString engine();
  
  rtError createContext(const char *lang, rtScriptContextRef& ctx);

  rtError pump();

  rtError collectGarbage();

private:
  rtScriptRef mScript;
};


#endif