#include "rtGuid.h"
#include <string.h>

rtGuid const&
rtGuid::null()
{
  static rtGuid _null;
  return _null;
}

rtGuid::rtGuid()
{
  uuid_clear(m_id);
}

rtGuid::rtGuid(rtGuid const& rhs)
{
  memcpy(m_id, rhs.m_id, sizeof(uuid_t));
}

rtGuid const&
rtGuid::operator=(rtGuid const& rhs)
{
  if (this != &rhs)
    memcpy(m_id, rhs.m_id, sizeof(uuid_t));
  return *this;
}

bool
rtGuid::operator==(rtGuid const& rhs) const
{
  return uuid_compare(m_id, rhs.m_id) == 0;
}

bool
rtGuid::operator!=(rtGuid const& rhs) const
{
  return uuid_compare(m_id, rhs.m_id) != 0;
}

bool
rtGuid::operator<(rtGuid const& rhs) const
{
  return uuid_compare(m_id, rhs.m_id) < 0;
}

rtGuid::rtGuid(uuid_t id)
{
  memcpy(m_id, id, sizeof(uuid_t));
}

rtGuid::~rtGuid()
{
}

rtGuid
rtGuid::newRandom()
{
  uuid_t id;
  uuid_generate_random(id);
  return rtGuid(id);
}

rtGuid
rtGuid::fromString(char const* s)
{
  uuid_t id;
  uuid_clear(id);

  return (s != nullptr && (uuid_parse(s, id) == 0))
    ? rtGuid(id)
    : rtGuid::null();
}

rtGuid
rtGuid::newTime()
{
  uuid_t id;
  uuid_generate_time(id);
  return rtGuid(id);
}

std::string
rtGuid::toString() const
{
  char buff[48];
  memset(buff, 0, sizeof(buff));
  uuid_unparse_lower(m_id, buff);
  return std::string(buff);
}
