// rtCore CopyRight 2005-2015 John Robinson
// rtError.h

#ifndef RT_ERROR_H
#define RT_ERROR_H

#include <stdint.h>

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
#define RT_TIMEOUT 1000

typedef uint32_t rtError;

const char* rtStrError(rtError e);

#endif
