#ifndef __RT_REMOTE_ENDPOINT_HANDLE_H__
#define __RT_REMOTE_ENDPOINT_HANDLE_H__

#include "rtRemoteEndPoint.h"
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
  rtRemoteIEndpointHandle(rtRemoteEndPointPtr endpoint)
    : m_addr(endpoint)
    , m_fd(kInvalidSocket)
  {
    // empty
  }

protected:
  rtRemoteEndPointPtr m_addr;
  socket_t m_fd;
};

#endif