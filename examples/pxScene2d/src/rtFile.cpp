// rtCore CopyRight 2007-2015 John Robinson
// rtFile.cpp

#include <stdlib.h>
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#ifdef WIN32
#include <Windows.h>
#include <limits>
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

rtError rtData::init(uint8_t* data, uint32_t length) {rtLogInfo("rtData::init");
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
{rtLogInfo("rtStoreFile");
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
{rtLogInfo("rtLoadFile");
  rtError e = RT_FAIL;
#ifdef WIN32
  HANDLE hFile = CreateFile(f, GENERIC_READ, 0, NULL, 0,
    FILE_ATTRIBUTE_NORMAL, NULL);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    LARGE_INTEGER size;
    if (GetFileSizeEx(hFile, &size))
    {
      if (size.QuadPart < std::numeric_limits<uint32_t>::max())
      {
        uint32_t l = static_cast<uint32_t>(size.QuadPart);
        data.init(l);

        DWORD dwBytesRead = 0;
        if (ReadFile(hFile, data.data(), l, &dwBytesRead, NULL))
        {

        }
      }
    }
    CloseHandle(hFile);
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
