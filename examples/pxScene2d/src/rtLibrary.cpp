#include "rtLibrary.h"
#include "rtLog.h"

#include <dlfcn.h>

// TODO: win32 impls

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

