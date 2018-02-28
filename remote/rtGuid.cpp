#include "rtGuid.h"
#include <string.h>

#ifdef RT_PLATFORM_WINDOWS

#include <Objbase.h>
#pragma comment(lib, "Ole32.lib")

#else

#ifdef RT_REMOTE_KERNEL_GUID
#include <stdio.h>
#else
#include <uuid/uuid.h>
#endif

#endif

rtGuid const&
rtGuid::null()
{
  static rtGuid _null;
  return _null;
}

rtGuid::rtGuid()
{
}

rtGuid::rtGuid(rtGuid const& rhs)
{
  m_id = rhs.m_id;
}

rtGuid const&
rtGuid::operator=(rtGuid const& rhs)
{
  if (this != &rhs)
    m_id = rhs.m_id;
  return *this;
}

bool
rtGuid::operator==(rtGuid const& rhs) const
{
  return m_id == rhs.m_id;
}

bool
rtGuid::operator!=(rtGuid const& rhs) const
{
  return m_id.compare(rhs.m_id) != 0;
}

bool
rtGuid::operator<(rtGuid const& rhs) const
{
  return m_id.compare(rhs.m_id) < 0;
}

rtGuid::~rtGuid()
{
}

rtGuid
rtGuid::newRandom()
{
  rtGuid guid;

  char buff[64];
  memset(buff, 0, sizeof(buff));

#ifdef RT_PLATFORM_WINDOWS

  GUID guidObj;
  CoCreateGuid(&guidObj);

  const char *hex2str = "0123456789abcdef";
  for (int i = 0, j = 0; i < 16; ++i) 
  {
    buff[j++] = hex2str[((unsigned char)guidObj.Data4[i] & 0xF0) >> 4];
    buff[j++] = hex2str[((unsigned char)guidObj.Data4[i] & 0x0F)];
    bool isHyphenNeeded = i == 3 || i == 5 || i == 7 || i == 9;
    if (isHyphenNeeded)
    {
      buff[j++] = '-';
    }
  }
  if (strlen(buff) > 0)
     guid.m_id = buff;

#else
  #ifndef RT_REMOTE_KERNEL_GUID
  uuid_t id;
  uuid_generate_random(id);

  uuid_unparse_lower(id, buff);
  guid.m_id = buff;
  #else
  FILE* f = fopen("/proc/sys/kernel/random/uuid", "r");
  if (f)
  {
    fgets(buff, sizeof(buff), f);
    fclose(f);
  }
  if (strlen(buff) > 0)
    guid.m_id = buff;
  #endif
#endif

  return guid;
}

rtGuid
rtGuid::fromString(char const* s)
{
  rtGuid guid;
  guid.m_id = s;
  return guid;
}

rtGuid
rtGuid::newTime()
{
#ifdef RT_PLATFORM_WINDOWS
   return newRandom();
#else
  #ifndef RT_REMOTE_KERNEL_GUID
  rtGuid guid;
  uuid_t id;
  uuid_generate_time(id);

  char buff[48];
  memset(buff, 0, sizeof(buff));
  uuid_unparse_lower(id, buff);
  guid.m_id = buff;
  return guid;
  #else
  return newRandom();
  #endif
#endif
}

std::string
rtGuid::toString() const
{
  return m_id;
}
