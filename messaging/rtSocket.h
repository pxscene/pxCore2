/* 
 * Copyright [2017] [Comcast, Corp.]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef __RT_SOCKET_H__
#define __RT_SOCKET_H__

#include "rtIpAddress.h"
#include "rtError.h"

typedef int rtSocketHandle;

class rtSocket;
class rtTcpListener;
class rtTcpClient;

class rtSocket
{
  friend class rtTcpListener;
  friend class rtTcpClient;

public:
  rtSocket();
  virtual ~rtSocket();

  rtError close();
  rtError connect(rtIpEndPoint const& e);
  rtError connect(char const* addr, uint16_t port);
  rtError bind(rtIpEndPoint const& e);
  rtError listen();

  rtIpEndPoint localEndPoint() const;
  rtIpEndPoint remoteEndPoint() const;

  rtSocket* accept();

  int send(void* buff, int n);
  int recv(void* buff, int n);

private:
  rtSocket(rtSocket const& rhs);
  rtSocket const& operator = (rtSocket const& rhs);

private:
  rtError createSocket(int family);
  rtError getBindInterface(int fd, rtIpEndPoint& endpoint, socklen_t len, bool local);

private:
  rtSocketHandle m_handle;
  rtIpEndPoint  m_localEndPoint;
  rtIpEndPoint  m_remoteEndPoint;
};

class rtTcpClient
{
public:
  rtTcpClient(rtSocket* soc);
  ~rtTcpClient();

  rtError read(void* buff, int n);
  rtError send(void* buff, int n);

  inline rtIpEndPoint localEndPoint() const
    { return m_soc->localEndPoint(); }

  inline rtIpEndPoint remoteEndPoint() const
    { return m_soc->remoteEndPoint(); }

private:
  rtSocket* m_soc;
};

class rtTcpListener
{
public:
  rtTcpListener();
  rtTcpListener(rtIpEndPoint const& bindEndpoint);
  ~rtTcpListener();

public:
  rtError start();
  rtTcpClient* acceptClient();

  inline rtIpEndPoint localEndPoint() const
    { return m_soc->localEndPoint(); }

  inline rtIpEndPoint remoteEndPoint() const
    { return m_soc->remoteEndPoint(); }

private:
  rtSocket* m_soc;
};

#endif
