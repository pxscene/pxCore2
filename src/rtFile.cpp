/*

 rtCore Copyright 2005-2017 John Robinson

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

// rtFile.cpp

#include <stdlib.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

// TODO do we need this
#ifndef WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#endif

#ifdef WIN32
#include <Windows.h>
#include <limits>
#include <direct.h>
#ifdef max
#undef max
#endif
#endif

#include "rtFile.h"

rtData::rtData(): mData(NULL), mLength(0) {}
rtData::~rtData() { term(); }

rtError rtData::init(uint32_t length) {
  term();
  mData = new uint8_t[length];
  if (mData) {
    mLength = length;
    return RT_OK;
  }
  else return RT_FAIL;
}

rtError rtData::init(uint8_t* data, uint32_t length) {
  rtError e = RT_FAIL;
  if (init(length) == RT_OK) {
    memcpy(mData, data, length);
    e = RT_OK;
  }
  return e;
}

rtError rtData::term() { delete [] mData; mLength = 0; return RT_OK; }
uint8_t* rtData::data() { return mData; }
uint32_t rtData::length() { return mLength; }

rtError rtStoreFile(const char* f, rtData& data)
{
  rtError e = RT_FAIL;

#ifdef WIN32
  // TODO should do W Variants... convert from utf8 
  // and break out to win/rtFile impl...
  HANDLE hFile = CreateFileA(f, GENERIC_WRITE, 0, NULL, TRUNCATE_EXISTING | CREATE_NEW,
    FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    DWORD dwBytesWritten = 0;
    WriteFile(hFile, data.data(), data.length(), &dwBytesWritten, NULL);
    CloseHandle(hFile);
  }
#else
  int fd = open(f, O_CREAT | O_TRUNC | O_WRONLY,0644);
  if (fd >= 0)
  {
    if (data.length() > 0)
      e = (write(fd, (void*)data.data(), data.length()) == data.length())?RT_OK:RT_FAIL;
    else
      e = RT_OK;
    close(fd);
  }
#endif
  return e;
}

rtError rtLoadFile(const char* f, rtData& data) 
{
  rtError e = RT_FAIL;
#ifdef WIN32
  FILE * pFile = fopen(f, "rb");
  if (pFile)
  {
	fseek(pFile, 0, SEEK_END);
	unsigned int lSize = ftell(pFile);
	if (lSize < std::numeric_limits<int>::max()) {
	  rewind(pFile);
	  data.init(lSize);
	  if (fread((void*)data.data(), 1, lSize, pFile) == lSize) {
			e = RT_OK;
	  }
	}
	fclose(pFile);
  }
#else
  struct stat st;
  int fd = open(f, O_RDONLY);
  if (fd >= 0) {
    if (fstat(fd, &st) == 0) {
      if (st.st_size <= UINT32_MAX) {
	uint32_t l = (uint32_t)st.st_size;
	data.init(l);
	if (read(fd, (void*)data.data(), l) == l)
	  e = RT_OK;
      }
    }
    close(fd);
  }
#endif
  return e;
}
