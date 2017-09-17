#include "rtBuffer.h"
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>


rtBuffer::rtBuffer()
{
  m_vec.reserve(4096);
  m_pos = 0;
};

rtBuffer::~rtBuffer()
{
}

void
rtBuffer::clear()
{
  m_vec.clear();
}

void
rtBuffer::rewind()
{
  m_pos = 0;
}

int
rtBuffer::position() const
{
  return m_pos;
}

void
rtBuffer::putInt8(int8_t n)
{
  m_vec.push_back(n);
}

int8_t
rtBuffer::getInt8() const
{
  return m_vec[m_pos++];
}

uint8_t
rtBuffer::getUInt8() const
{
  return static_cast<uint8_t>(getUInt8());
}

void
rtBuffer::putInt16(int16_t n)
{
  int16_t n2 = htons(n);
  uint8_t* b = reinterpret_cast<uint8_t *>(&n2);
  m_vec.push_back(b[0]);
  m_vec.push_back(b[1]);
}

int16_t
rtBuffer::getInt16() const
{
  int16_t n = 0;
  memcpy(&n, &m_vec[m_pos], 2);
  m_pos += 2;
  return ntohs(n);
}

uint16_t
rtBuffer::getUInt16() const
{
  return static_cast<uint16_t>(getInt16());
}

void
rtBuffer::putInt32(int32_t n)
{
  int32_t n2 = htonl(n);
  uint8_t* b = reinterpret_cast<uint8_t *>(&n2);
  m_vec.push_back(b[0]);
  m_vec.push_back(b[1]);
  m_vec.push_back(b[2]);
  m_vec.push_back(b[3]);
}

int32_t
rtBuffer::getInt32() const
{
  int32_t n = 0;
  memcpy(&n, &m_vec[m_pos], 4);
  m_pos += 4;
  return ntohl(n);
}

uint32_t
rtBuffer::getUInt32() const
{
  return static_cast<uint32_t>(getInt32());
}

void
rtBuffer::putUInt8(uint8_t n)
{
  putInt8(static_cast<int8_t>(n));
}

void
rtBuffer::putUInt16(uint16_t n)
{
  putInt16(static_cast<int16_t>(n));
}

void
rtBuffer::putUInt32(uint32_t n)
{
  putUInt32(static_cast<uint32_t>(n));
}
