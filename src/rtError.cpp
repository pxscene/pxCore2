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

// rtError.cpp

#include "rtError.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define RT_ERROR_CASE(ERR) case ERR: s = # ERR; break;
#define RT_ERROR_CLASS(ERR) (((ERR) >> 16) & 0x0000ffff)
#define RT_ERROR_CODE(ERR) ((ERR) & 0x0000ffff)

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t key;

static const char* rtStrError_BuiltIn(rtError e);
static const char* rtStrError_SystemError(int e);

void rtErrorInitThreadSpecificKey()
{
  pthread_key_create(&key, NULL);
}

const char* rtStrError(rtError e)
{
  const char* s = NULL;

  int klass = RT_ERROR_CLASS(e);
  switch (klass)
  {
    case RT_ERROR_CLASS_BUILTIN:
    s = rtStrError_BuiltIn(RT_ERROR_CODE(e));
    break;

    case RT_ERROR_CLASS_SYSERROR:
    s = rtStrError_SystemError(RT_ERROR_CODE(e));
    break;
  }
  return s;
}

const char* rtStrError_SystemError(int e)
{
  pthread_once(&once, rtErrorInitThreadSpecificKey);

  char* buff = reinterpret_cast<char *>(pthread_getspecific(key));
  if (buff == NULL)
  {
    buff = reinterpret_cast<char *>(malloc(256));
    if (buff)
      buff[0] = '\0';
    pthread_setspecific(key, buff);
  }

  assert(buff != NULL);

  // TODO: The below #ifdef is not the best approach. @see man strerror_r and
  // /usr/include/string.h for proper check of which version of strerror_r is 
  // available. Need to check for osx portability also
  if (buff)
  {
    #ifdef __linux__
    buff = strerror_r(e, buff, 256);
    #else
    strerror(e);
    #endif
  }

  return buff;
}

const char* rtStrError_BuiltIn(rtError e)
{
  const char* s = "UNKNOWN";
  switch (e)
  {
    RT_ERROR_CASE(RT_OK);
    RT_ERROR_CASE(RT_FAIL);
    RT_ERROR_CASE(RT_ERROR_NOT_ENOUGH_ARGS);
    RT_ERROR_CASE(RT_ERROR_INVALID_ARG);
    RT_ERROR_CASE(RT_PROP_NOT_FOUND);
    RT_ERROR_CASE(RT_OBJECT_NOT_INITIALIZED);
    RT_ERROR_CASE(RT_PROPERTY_NOT_FOUND);
    RT_ERROR_CASE(RT_OBJECT_NO_LONGER_AVAILABLE);
    RT_ERROR_CASE(RT_RESOURCE_NOT_FOUND);
    RT_ERROR_CASE(RT_NO_CONNECTION);
    RT_ERROR_CASE(RT_ERROR_NOT_IMPLEMENTED);
    RT_ERROR_CASE(RT_ERROR_TYPE_MISMATCH);

    RT_ERROR_CASE(RT_ERROR_TIMEOUT);
    RT_ERROR_CASE(RT_ERROR_DUPLICATE_ENTRY);
    RT_ERROR_CASE(RT_ERROR_OBJECT_NOT_FOUND);
    RT_ERROR_CASE(RT_ERROR_PROTOCOL_ERROR);
    RT_ERROR_CASE(RT_ERROR_INVALID_OPERATION);
    RT_ERROR_CASE(RT_ERROR_IN_PROGRESS);
    RT_ERROR_CASE(RT_ERROR_QUEUE_EMPTY);
    RT_ERROR_CASE(RT_ERROR_STREAM_CLOSED);

    RT_ERROR_CASE(RT_ERROR_NOT_ALLOWED);
    default:
      break;
  }
  return s;
}
