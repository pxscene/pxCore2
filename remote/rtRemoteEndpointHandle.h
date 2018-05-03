/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

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
