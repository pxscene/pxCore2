#include "rtRemoteEndpoint.h"
#include "rtRemoteTypes.h"
#include "rtRemoteUtils.h"
#include "rtSocketUtils.h"
#include <rtLog.h>

#include <sstream>
#include <string>
#include <errno.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

// BASE //
rtRemoteIEndpoint::rtRemoteIEndpoint(std::string const& scheme)
  : m_scheme(scheme)
{ }

rtRemoteIEndpoint::~rtRemoteIEndpoint() { }

// LOCAL //
rtRemoteEndpointLocal::rtRemoteEndpointLocal(std::string const& scheme, std::string const& path)
  : rtRemoteIEndpoint(scheme)
  , m_path(path)
{ }

bool
rtRemoteEndpointLocal::isSocket() const
{
  return m_scheme.compare("tcp") == 0 || m_scheme.compare("udp") == 0;
}

std::string
rtRemoteEndpointLocal::toUriString()
{
  std::stringstream buff;
  buff << m_scheme;
  buff << "://";
  buff << m_path;
  return buff.str();
}

// NETWORK //
rtRemoteEndpointRemote::rtRemoteEndpointRemote(std::string const& scheme, std::string const& host, int port)
  : rtRemoteIEndpoint(scheme)
  , m_host(host)
  , m_port(port)
{ }

std::string
rtRemoteEndpointRemote::toUriString()
{
  std::stringstream buff;
  buff << m_scheme;
  buff << "://";
  buff << m_host;
  buff << ":";
  buff << m_port;
  return buff.str();
}

// NETWORK + PATH
rtRemoteEndpointDistributed::rtRemoteEndpointDistributed(std::string const& scheme, std::string const& host, int port, std::string const& path)
  : rtRemoteIEndpoint(scheme)
  , rtRemoteEndpointRemote(scheme, host, port)
  , rtRemoteEndpointLocal(scheme, path)
{ }

std::string
rtRemoteEndpointDistributed::toUriString()
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