#ifndef __RT_REMOTE_ENDPOINT_H__
#define __RT_REMOTE_ENDPOINT_H__

#include "rtRemoteTypes.h"
#include "rtError.h"
#include <sys/socket.h>
#include <string>
#include <sstream>
#include <stdint.h>

/* Abstract base class for endpoint addresses */
class rtRemoteIEndpoint
{
public:
	rtRemoteIEndpoint(std::string const& scheme): m_scheme(scheme) { };
	virtual ~rtRemoteIEndpoint() { };

	virtual std::string toUriString() = 0;

	inline std::string scheme() const
	  { return m_scheme; }

protected:
	std::string m_scheme;
};

using rtRemoteEndpointPtr = std::shared_ptr< rtRemoteIEndpoint >;

/* Local endpoints.
 * Used to stored address information for unix domain sockets,
 * named pipes, files, shared memory, etc.
 */
class rtRemoteEndpointLocal : public virtual rtRemoteIEndpoint
{
public:
  rtRemoteEndpointLocal(std::string const& scheme, std::string const& path)
  : rtRemoteIEndpoint(scheme)
  , m_path(path)
  { }
	
  virtual std::string toUriString() override
  {
    std::stringstream buff;
    buff << m_scheme;
    buff << "://";
    buff << m_path;
    return buff.str();
  }
	
  inline bool operator==(rtRemoteEndpointLocal const& rhs) const
    { return m_path.compare(rhs.path()) == 0
		  && m_scheme.compare(rhs.scheme()) == 0;
	}

  inline std::string path() const
	{ return m_path; }

  inline bool isSocket() const
    { return m_scheme.compare("tcp") == 0 || m_scheme.compare("udp") == 0; }

protected:
  std::string m_path;
};


/* Remote endpoints.
 * Used to stored address information for remote sockets
 * (tcp, udp, http, etc.)
 */
class rtRemoteEndpointRemote : public virtual rtRemoteIEndpoint
{
public:
  rtRemoteEndpointRemote(std::string const& scheme, std::string const& host, int port)
    : rtRemoteIEndpoint(scheme)
    , m_host(host)
    , m_port(port)
  { }
	
  virtual std::string toUriString() override
  {
    std::stringstream buff;
    buff << m_scheme;
    buff << "://";
    buff << m_host;
    buff << ":";
    buff << m_port;
    return buff.str();
  }

  inline bool operator==(rtRemoteEndpointRemote const& rhs) const
    { return m_host.compare(rhs.host()) == 0
	      && m_port == rhs.port()
          && m_scheme.compare(rhs.scheme()) == 0;
	}

  inline std::string host() const
	{ return m_host; }

  inline int port() const
    { return m_port; }

protected:
  std::string         m_host;
  int                 m_port;
};


/* Remote endpoints that include path information
 *
 * Currently not used.  If use case arises, must be integrated throughout the code.
 */
class rtRemoteEndpointDistributed : public rtRemoteEndpointRemote, public rtRemoteEndpointLocal
{
public:
  rtRemoteEndpointDistributed(std::string const& scheme, std::string const& host, int port, std::string const& path)
    : rtRemoteIEndpoint(scheme)
    , rtRemoteEndpointRemote(scheme, host, port)
    , rtRemoteEndpointLocal(scheme, path)
  { }
	
  virtual std::string toUriString() override
  {
    std::stringstream buff;
    buff << m_scheme;
    buff << "://";
    buff << m_host;
    buff << ":";
    buff << m_port;
    buff << m_path;
    return buff.str();
  }

  inline bool operator==(rtRemoteEndpointDistributed const& rhs) const
	{ return m_host.compare(rhs.host()) == 0
		  && m_port == rhs.port()
		  && m_path.compare(rhs.path()) == 0
		  && m_scheme.compare(rhs.scheme()) == 0;
    }
};

#endif