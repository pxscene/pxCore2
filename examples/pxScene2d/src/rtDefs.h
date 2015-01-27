#ifndef _RT_DEFS
#define _RT_DEFS

#include <stdint.h>

#include "rtError.h"

//typedef enum {RT_OK, RT_FAIL} rt_error;
typedef unsigned long rt_error;

#ifndef finline
#define finline inline
#endif

#ifndef NULL
#define NULL 0
#endif

#endif
