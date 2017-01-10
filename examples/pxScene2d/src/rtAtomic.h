// rtCore CopyRight 2007-2015 John Robinson
// rtAtomic.h

#ifndef RT_ATOMIC_H
#define RT_ATOMIC_H

#ifdef WIN32
#include <Windows.h>
#define rtAtomic          volatile int32_t
#define rtAtomicInc(ptr)  (InterlockedIncrement(ptr))
#define rtAtomicDec(ptr)  (InterlockedDecrement(ptr))
#else
#define rtAtomic          volatile int32_t
#define rtAtomicInc(ptr)  (__sync_fetch_and_add(ptr, 1)+1)
#define rtAtomicDec(ptr)  (__sync_fetch_and_sub(ptr, 1)-1)
#endif

#endif
