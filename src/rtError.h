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

// TODO review base numbering scheme for different error classes... general vs rtremote vs... 
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
#define RT_ERROR_NOT_IMPLEMENTED 10
#define RT_ERROR_TYPE_MISMATCH 11

// errors specific to rtRemote
#define RT_ERROR_TIMEOUT 1000
#define RT_ERROR_DUPLICATE_ENTRY 1001
#define RT_ERROR_OBJECT_NOT_FOUND 1002
#define RT_ERROR_PROTOCOL_ERROR 1003
#define RT_ERROR_INVALID_OPERATION 1004
#define RT_ERROR_IN_PROGRESS 1005
#define RT_ERROR_QUEUE_EMPTY 1006
#define RT_ERROR_STREAM_CLOSED 1007

// errors specific to security model
#define RT_ERROR_NOT_ALLOWED 2000

typedef int rtError;

const char* rtStrError(rtError e);

// TODO where all is this used??
inline rtError
rtErrorFromErrno(int err)
{
  // RT_ASSERT(err <= 65535); // uint16_max
  return err == 0 ? RT_OK : (err | (RT_ERROR_CLASS_SYSERROR << 16));
}

#endif
