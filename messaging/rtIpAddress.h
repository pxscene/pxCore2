/* 
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
#ifndef __RT_IPADDRESS_H__
#define __RT_IPADDRESS_H__

#include <stdint.h>
#include <string>
#include <sys/socket.h>

class rtIpAddress;
class rtIpEndPoint;
class rtSocket;

/**
 *
 */
class rtIpAddress
{
  friend class rtIpEndPoint;
  friend class rtSocket;

public:
  std::string toString() const;
  socklen_t length() const;
  bool operator == (rtIpAddress const & rhs) const;

public:
  static rtIpAddress fromString(char const* s);

private:
  rtIpAddress();

private:
  sockaddr_storage m_addr;
};


/**
 *
 */
class rtIpEndPoint
{
  friend class rtSocket;

public:
  rtIpEndPoint(rtIpAddress const& addr, uint16_t port);

public:
  bool operator == (rtIpEndPoint const& rhs) const;

public:
  rtIpAddress address() const;
  uint16_t port() const;
  std::string toString() const;

  static rtIpEndPoint fromString(char const* s);
  static rtIpEndPoint const& invalid();

private:
  rtIpAddress m_addr;
};
#endif
