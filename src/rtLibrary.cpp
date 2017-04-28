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

// rtLibrary.cpp

#include "rtLibrary.h"
#include "rtLog.h"

#ifndef WIN32
#include <dlfcn.h>
#endif

#ifdef WIN32
rtError rtLoadLibrary(const char* name, rtLibrary* lib)
{
  // TODO:
  return RT_FAIL;
}
rtError rtLookupFunction(rtLibrary lib, const char* func, rtFunctionAddr* addr)
{
  // TODO:
  return RT_FAIL;
}
#else
rtError rtLoadLibrary(const char* name, rtLibrary* lib)
{
  if (!name)
    return RT_ERROR_INVALID_ARG;

  void* handle = dlopen(name, RTLD_NOW);
  if (!handle)
    return RT_FAIL;

  *lib = handle;
  return RT_OK;
}

rtError rtLookupFunction(rtLibrary lib, const char* func, rtFunctionAddr* addr)
{
  if (!lib)
    return RT_ERROR_INVALID_ARG;
  if (!addr)
    return RT_ERROR_INVALID_ARG;
  if (!func)
    return RT_ERROR_INVALID_ARG;

  void* sym = dlsym(lib, func);
  if (!sym)
    return RT_FAIL;

  *addr = sym;
  return RT_OK;
}
#endif


