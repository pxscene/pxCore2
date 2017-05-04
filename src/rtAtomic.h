/*

 pxCore Copyright 2005-2017 John Robinson

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

// rtAtomic.h

#ifndef RT_ATOMIC_H
#define RT_ATOMIC_H

#ifdef WIN32
#include <Windows.h>
#define rtAtomic          volatile int32_t
#define rtAtomicInc(ptr)  (InterlockedIncrement((volatile unsigned int *)ptr))
#define rtAtomicDec(ptr)  (InterlockedDecrement((volatile unsigned int *)ptr))
#else
#define rtAtomic          volatile int32_t
#define rtAtomicInc(ptr)  (__sync_add_and_fetch(ptr, 1))
#define rtAtomicDec(ptr)  (__sync_sub_and_fetch(ptr, 1))
#endif

#endif
