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

#ifndef RT_SCRIPT_HEADERS_H
#define RT_SCRIPT_HEADERS_H

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wall"
#endif

#include <uv.h>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

#ifdef WIN32
#define USE_STD_THREADS
#include <mutex>
#include <condition_variable>
#endif

#endif

