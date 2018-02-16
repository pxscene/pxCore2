#ifndef __RT_REMOTE_ENDPOINT_HANDLE_H__
#define __RT_REMOTE_ENDPOINT_HANDLE_H__

#include "rtRemoteEndpoint.h"
#include "rtError.h"

class rtRemoteIEndpointHandle
{
public:
  virtual ~rtRemoteIEndpointHandle()
  {
    if (m_fd != kInvalidSocket)
      rtCloseSocket(m_fd);
  }

  /* Should create fd */
  virtual rtError open() = 0;
  virtual rtError close() = 0;
  
  inline socket_t fd() const
    { return m_fd; }

  inline void setFd(socket_t fd)
	{ m_fd = fd; }

protected:
  rtRemoteIEndpointHandle(rtRemoteEndpointPtr endpoint)
    : m_addr(endpoint)
    , m_fd(kInvalidSocket)
  {
    // empty
  }

protected:
  rtRemoteEndpointPtr m_addr;
  socket_t m_fd;
};

#endif