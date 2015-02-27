// rtCore CopyRight 2007-2015 John Robinson
// rtFile.cpp

#include <stdlib.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

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

rtError rtLoadFile(const char* f, rtData& data) {
  rtError e = RT_FAIL;
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
  return e;
}
