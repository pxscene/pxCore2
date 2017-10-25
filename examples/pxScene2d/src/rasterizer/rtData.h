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

