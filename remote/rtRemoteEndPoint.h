#ifndef __RT_REMOTE_ENDPOINT_H__
#define __RT_REMOTE_ENDPOINT_H__

#include "rtRemoteTypes.h"
#include "rtError.h"
#include <sys/socket.h>
#include <string>
#include <sstream>
#include <stdint.h>
#include <memory>

/* Abstract base class for endpoint addresses */
class rtRemoteEndPoint
{
public:
	rtRemoteEndPoint(std::string const& scheme): m_scheme(scheme) { };
	virtual ~rtRemoteEndPoint() { };

  static rtRemoteEndPoint* fromString(std::string const& s);

	virtual std::string toString() = 0;
  virtual sockaddr_storage toSockAddr() const = 0;

	inline std::string const& scheme() const
	  { return m_scheme; }

protected:
  rtRemoteEndPoint() { }

protected:
	std::string m_scheme;
};

using rtRemoteEndPointPtr = std::shared_ptr< rtRemoteEndPoint >;

/* Local endpoints.
 * Used to stored address information for unix domain sockets,
 * named pipes, files, shared memory, etc.
 */
class rtRemoteFileEndPoint : public virtual rtRemoteEndPoint
{
public:
  rtRemoteFileEndPoint(std::string const& scheme, std::string const& path)
    : rtRemoteEndPoint(scheme)
    , m_path(path) { }

  static rtRemoteFileEndPoint* fromString(std::string const& s);
  static rtRemoteFileEndPoint* fromSockAddr(sockaddr_storage const& s);

  virtual sockaddr_storage toSockAddr() const override;
  virtual std::string toString() override
  {
    std::stringstream buff;
    buff << m_scheme;
    buff << "://";
    buff << m_path;
    return buff.str();
  }

  inline bool operator == (rtRemoteFileEndPoint const& rhs) const
    { return m_path.compare(rhs.path()) == 0 && m_scheme.compare(rhs.scheme()) == 0; }

  inline std::string const& path() const
    { return m_path; }

private:
  rtRemoteFileEndPoint() : rtRemoteEndPoint() { }

protected:
  std::string m_path;
};


/* Remote endpoints.
 * Used to stored address information for remote sockets
 * (tcp, udp, http, etc.)
 */
class rtRemoteIPEndPoint : public virtual rtRemoteEndPoint
{
public:
  rtRemoteIPEndPoint(std::string const& scheme, std::string const& host, int port)
    : rtRemoteEndPoint(scheme)
    , m_host(host)
    , m_port(port) { }

  static rtRemoteIPEndPoint* fromString(std::string const& s);
  static rtRemoteIPEndPoint* fromSockAddr(std::string const& scheme, sockaddr_storage const& ss);
	
  virtual sockaddr_storage toSockAddr() const override;
  virtual std::string toString() override
  {
    std::stringstream buff;
    buff << m_scheme;
    buff << "://";
    buff << m_host;
    buff << ":";
    buff << m_port;
    return buff.str();
  }

  inline bool operator == (rtRemoteIPEndPoint const& rhs) const
    { return m_host.compare(rhs.host()) == 0
	      && m_port == rhs.port()
          && m_scheme.compare(rhs.scheme()) == 0;
	}

  inline std::string const& host() const
    { return m_host; }

  inline int port() const
    { return m_port; }

private:
  rtRemoteIPEndPoint() { }

protected:
  std::string m_host;
  int m_port;
};

#endif
