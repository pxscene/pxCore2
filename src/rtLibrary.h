#ifndef RT_LIBRARY_H
#define RT_LIBRARY_H

#include "rtError.h"

typedef void* rtLibrary;
typedef void* rtFunctionAddr;

rtError rtLoadLibrary(const char* name, rtLibrary* lib);
rtError rtLookupFunction(rtLibrary lib, const char* func, rtFunctionAddr* addr);

#endif
