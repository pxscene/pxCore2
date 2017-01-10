// rtCore CopyRight 2007-2015 John Robinson
// rtFile.h

#ifndef _RT_FILE_H
#define _RT_FILE_H

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#include "rtDefs.h"

class rtData 
{
 public:
  rtData();
  ~rtData();

  // TODO copy constructor and assignment
  
  rtError init(uint32_t length);
  rtError init(uint8_t* data, uint32_t length);

  rtError term();

  uint8_t* data();
  uint32_t length();

 private:
  uint8_t* mData;
  uint32_t mLength;
};

rtError rtLoadFile(const char* f, rtData& data);
rtError rtStoreFile(const char* f, rtData& data);

#if 1
class rtFilePointer
{
public:
  rtFilePointer()
    : mFile(NULL)
  { }

  rtFilePointer(FILE* f)
    : mFile(f)
  { }

  ~rtFilePointer()
  {
    if (mFile)
      fclose(mFile);
  }

  FILE* getPtr() const { return mFile; }

private:
  rtFilePointer(const rtFilePointer& )
  {
    assert(false);
  }

  const rtFilePointer& operator = (const rtFilePointer& )
  {
    assert(false);
    return *this;
  }

  // http://www.artima.com/cppsource/safebool2.html
  // http://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Safe_bool
  typedef void (rtFilePointer::*bool_type)() const;
  void comparisonNotSupported() const { }

public:
  operator bool_type() const
  {
    return (mFile != NULL) ? &rtFilePointer::comparisonNotSupported : 0;
  }

private:
  FILE* mFile;
};
#endif

#endif
