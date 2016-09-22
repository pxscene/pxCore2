#ifndef __RT_REMOTE_ENDPOINT_HANDLE_H__
#define __RT_REMOTE_ENDPOINT_HANDLE_H__

#include "rtRemoteEndpoint.h"
#include "rtError.h"
#include <unistd.h>

class rtRemoteIEndpointHandle
{
public:
  virtual ~rtRemoteIEndpointHandle()
  {
    if (m_fd != -1)
      ::close(m_fd);
    m_fd = -1;
  }

  /* Should create fd */
  virtual rtError open() = 0;
  virtual rtError close() = 0;
  
  inline int fd() const
    { return m_fd; }

  inline void setFd(int fd)
	{ m_fd = fd; }

protected:
  rtRemoteIEndpointHandle(rtRemoteEndpointPtr endpoint)
    : m_addr(endpoint)
    , m_fd(-1)
  {
    // empty
  }

protected:
  rtRemoteEndpointPtr m_addr;
  int m_fd;
};

#endif