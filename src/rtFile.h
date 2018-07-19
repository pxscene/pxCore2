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

// rtFile.h

#ifndef _RT_FILE_H
#define _RT_FILE_H

#include <assert.h>
#include <rtCore.h>
#include <stdio.h> //TODO - needed for FILE, fopen, etc

/**
rtData is a wrapper that encapsulated an allocated buffer of bytes and owns the lifetime of those bytes.
*/
class rtData
{
 public:
  rtData();
  ~rtData();

  rtData(rtData &d);
  rtData(const uint8_t* data, size_t length);

  rtData& operator=(const rtData& d);

  // TODO copy constructor and assignment
  rtError init(size_t length);
  rtError init(const uint8_t* data, size_t length);

  rtError term();

  uint8_t* data();
  uint32_t length();

 private:
  uint8_t* mData;
  uint32_t mLength;
};

// Load or Store a file using an rtData managed buffer
rtError rtLoadFile(const char* f, rtData& data);
rtError rtStoreFile(const char* f, rtData& data);

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
