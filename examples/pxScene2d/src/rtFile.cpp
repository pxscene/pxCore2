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

rtData::rtData(): m_data(NULL), m_length(0) {}
rtData::~rtData() { term(); }

rt_error rtData::init(uint32_t length) {
  term();
  m_data = new uint8_t[length];
  if (m_data) {
    m_length = length;
    return RT_OK;
  }
  else return RT_FAIL;
}

rt_error rtData::init(uint8_t* data, uint32_t length) {
  rt_error e = RT_FAIL;
  if (init(length) == RT_OK) {
    memcpy(m_data, data, length);
    e = RT_OK;
  }
  return e;
}

rt_error rtData::term() { delete(m_data); m_length = 0; return RT_OK; }
uint8_t* rtData::data() { return m_data; }
uint32_t rtData::length() { return m_length; }

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
