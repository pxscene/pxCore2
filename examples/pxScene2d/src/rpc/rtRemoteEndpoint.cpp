#include "rtRemoteEndpoint.h"

#include <sstream>
#include <string>

// BASE //
rtRemoteIEndpoint::rtRemoteIEndpoint(std::string const& scheme)
  : m_scheme(scheme)
{
  // empty
}

rtRemoteIEndpoint::~rtRemoteIEndpoint()
{
  // empty
}

// LOCAL //
rtRemoteEndpointLocal::rtRemoteEndpointLocal(std::string const& scheme, std::string const& path)
  : rtRemoteIEndpoint(scheme)
  , m_path(path)
{
  // empty
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

// REMOTE //
rtRemoteEndpointRemote::rtRemoteEndpointRemote(std::string const& scheme, std::string const& host, int port)
  : rtRemoteIEndpoint(scheme)
  , m_host(host)
  , m_port(port)
{
  // empty
}

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

// REMOTE + PATH //
rtRemoteEndpointDistributed::rtRemoteEndpointDistributed(std::string const& scheme, std::string const& host, int port, std::string const& path)
  : rtRemoteIEndpoint(scheme)
  , rtRemoteEndpointRemote(scheme, host, port)
  , rtRemoteEndpointLocal(scheme, path)
{
  // empty
}

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