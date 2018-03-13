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

#pragma once

#include <string.h>
#include "rtError.h"
#include "stdlib.h"


class rtData
{
public:
rtData(): mData(NULL), mLength(0) {}
  virtual ~rtData()
  {
    term();
  }
    
  // allocs the buffer and initializes it with the data pointed to by p
  rtError initWithBytes(void * p, unsigned long length)
  {
    rtError e = RT_ERROR;
    term();
    mData = malloc(length);
    if (mData)
    {
      memcpy(mData, p, length);
      e = RT_OK;
    }
    return e;
  }

  // allocs the buffer but does not initialize the contents
  rtError initWithLength(unsigned long length)
  {
    rtError e = RT_ERROR;
    term();
    mData = malloc(length);
    if (mData)
    {
      mLength = length;
      e = RT_OK;
    }
    return e;
  }

  rtError term()
  {
    if (mData)
    {
      free(mData);
      mData = NULL;
    }
    mLength = 0;
    return RT_OK;
  }

  void* bytes() const { return mData; }

  unsigned long length() const { return mLength; }

private:
  void* mData;
  unsigned long mLength;
};

