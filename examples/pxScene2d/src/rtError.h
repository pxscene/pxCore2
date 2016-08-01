// rtCore CopyRight 2005-2015 John Robinson
// rtError.h

#ifndef RT_ERROR_H
#define RT_ERROR_H

#include <stdint.h>

#ifdef RT_DEBUG
#include <assert.h>
#define RT_ASSERT(X) assert((X))
#else
#define RT_ASSERT(X) if (!(X)) rtLogError("rt assert: '%s' failed", #X);
#endif

#define RT_ERROR_CLASS_SYSERROR 0x8000
#define RT_ERROR_CLASS_BUILTIN 0x00000000

#define RT_OK                           0
#define RT_ERROR                        1
#define RT_FAIL                         1
#define RT_ERROR_NOT_ENOUGH_ARGS        2
#define RT_ERROR_INVALID_ARG            3
#define RT_PROP_NOT_FOUND               4
#define RT_OBJECT_NOT_INITIALIZED       5
#define RT_PROPERTY_NOT_FOUND           6
#define RT_OBJECT_NO_LONGER_AVAILABLE   7

#define RT_RESOURCE_NOT_FOUND		8
#define RT_NO_CONNECTION		9

// errors specific to rtRemote
#define RT_ERROR_TIMEOUT 1000
#define RT_ERROR_DUPLICATE_ENTRY 1001
#define RT_ERROR_OBJECT_NOT_FOUND 1002
#define RT_ERROR_PROTOCOL_ERROR 1003
#define RT_ERROR_INVALID_OPERATION 1004
#define RT_ERROR_IN_PROGRESS 1005

typedef uint32_t rtError;

const char* rtStrError(rtError e);

inline rtError
rtErrorFromErrno(int err)
{
  // RT_ASSERT(err <= 65535); // uint16_max
  return err == 0 ? RT_OK : (err | (RT_ERROR_CLASS_SYSERROR << 16));
}

#endif
