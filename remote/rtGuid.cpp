#include "rtGuid.h"
#include <string.h>

#ifdef RT_REMOTE_KERNEL_GUID 
#include <stdio.h>
#else
#include <uuid/uuid.h>
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
}

std::string
rtGuid::toString() const
{
  return m_id;
}
