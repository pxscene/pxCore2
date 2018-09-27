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

// rtFile.cpp

#include <stdlib.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
// required by std::numeric_limits
#include <limits>
#include "rtFile.h"
// remove unused headers

rtData::rtData(): mData(NULL), mLength(0) {}
rtData::~rtData() { term(); }


rtData::rtData(rtData &d) : mData(d.data()), mLength(d.length())                   {};
rtData::rtData(const uint8_t* data, size_t length) : mData( (uint8_t* ) data), mLength( (uint32_t) length)  {};


rtError rtData::init(size_t length) {
  term();
  mData = new uint8_t[length+1];
  memset(mData, 0, length+1);
  if (mData) {
    mLength = (uint32_t) length;
    return RT_OK;
  }
  else return RT_FAIL;
}

rtError rtData::init(const uint8_t* data, size_t length) {
  rtError e = RT_FAIL;
  if (init(length) == RT_OK) {
    memcpy(mData, data, length);
    e = RT_OK;
  }
  return e;
}

rtError rtData::term() { delete [] mData; mData=NULL; mLength = 0; return RT_OK; }
uint8_t* rtData::data() { return mData; }
uint32_t rtData::length() { return mLength; }

rtError rtStoreFile(const char* f, rtData& data)
{
  rtError e = RT_FAIL;
	// use fopen fwrite fclose from stdio.h
	FILE * fd = fopen(f, "wb");
	if (fd)
	{
		if (data.length() > 0)
			e = (fwrite((void*)data.data(), 1, data.length(), fd) == data.length()) ? RT_OK : RT_FAIL;
		else
			e = RT_OK;
		fclose(fd);
	}
	return e;
}

rtError rtLoadFile(const char* f, rtData& data)
{
	rtError e = RT_FAIL;
	// use fopen fread fclose and etc from stdio.h
	FILE * pFile = fopen(f, "rb");
	if (pFile)
	{
		fseek(pFile, 0, SEEK_END);
		size_t lSize = ftell(pFile);
		if (lSize < std::numeric_limits<int>::max()) {
			rewind(pFile);
			data.init(lSize);
			if (fread((void*)data.data(), 1, lSize, pFile) == lSize) {
				e = RT_OK;
			}
		}
		fclose(pFile);
	}
	return e;
}
