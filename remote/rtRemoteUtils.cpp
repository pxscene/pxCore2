#include "rtRemoteUtils.h"
#include "rtRemoteTypes.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessage.h"

#include <sstream>
#include <string>
#include <memory>

rtError
rtRemoteEndpointAddressToSocket(rtRemoteEndPointPtr addr, sockaddr_storage& ss)
{
  if (auto local = std::dynamic_pointer_cast<rtRemoteFileEndPoint>(addr))
  {
    return rtParseAddress(ss, local->path().c_str(), 0, nullptr);
  }
  else if (auto net = std::dynamic_pointer_cast<rtRemoteIPEndPoint>(addr))
  {
    return rtParseAddress(ss, net->host().c_str(), net->port(), nullptr);
  }
  else
  {
    return RT_FAIL;
  }
}

rtError
rtRemoteSocketToEndpointAddress(sockaddr_storage const& ss, rtConnType const& connType, rtRemoteEndPointPtr& endpoint)
{
  std::stringstream buff;
  
  std::string scheme;
  if (connType == rtConnType::STREAM)
  {
    scheme = "tcp";
  }
  else if (connType == rtConnType::DGRAM)
  {
    scheme = "udp";
  }
  else
  {
    rtLogError("no connection protocol indicated while converting from socket to endpoint address");
    return RT_FAIL;
  }

  buff << scheme;
  buff << "://";

  void* addr = NULL;
  rtGetInetAddr(ss, &addr);

  char addrBuff[128];
  memset(addrBuff, 0, sizeof(addrBuff));

  if (ss.ss_family == AF_UNIX)
  {
    strncpy(addrBuff, (const char*)addr, sizeof(addrBuff) -1);
    buff << addrBuff;
    endpoint = std::make_shared<rtRemoteFileEndPoint>(scheme, addrBuff);
    return RT_OK;
  }
  else
  {
    inet_ntop(ss.ss_family, addr, addrBuff, sizeof(addrBuff));
    uint16_t port;
    rtGetPort(ss, &port);
    buff << addrBuff;
    buff << ":";
    buff << port;
    endpoint = std::make_shared<rtRemoteIPEndPoint>(scheme, addrBuff, port);
    return RT_OK;
  }
  return RT_OK;
}

rtError
rtRemoteParseUri(std::string const& uri, std::string& scheme, std::string& path, std::string& host, uint16_t* port)
{
  size_t index = uri.find("://");
  if (index == std::string::npos)
  {
   rtLogError("Invalid uri: %s. Expected: <scheme>://<host>[:<port>][<path>]", uri.c_str());
   return RT_FAIL;
  }

  // extract scheme
  scheme = uri.substr(0, index);

  // We either have a path or host now.  Let's pull the remaining info.
  index += 3;
  char ch = uri.at(index);
  if (ch == '/' || ch == '.')
  { // local socket
    path = uri.substr(index, std::string::npos);
  }
  else
  { // network socket
    // get port
    std::string portString;
    size_t portIndex = uri.find_last_of(":");
    if (portIndex == std::string::npos // no port. no colon found
      || uri.at(portIndex-1) == ':' // no port. colon was part of ipv6 addr
      || portIndex == index-3) // no port.  last colon equals colon in ://
    {
      rtLogWarn("No port included included in URI: %s. Defaulting to 0", uri.c_str());
      portString = "0";
      portIndex = std::string::npos; // set this for host extraction below
    }
    else
    {
      portString = uri.substr(portIndex+1, std::string::npos);
    }
    *port = stoi(portString);
    
    // get host
    host = uri.substr(index, portIndex - index);
  }
  return RT_OK;
}

bool
rtRemoteSameEndpoint(sockaddr_storage const& first, sockaddr_storage const& second)
{
  if (first.ss_family != second.ss_family)
    return false;

  if (first.ss_family == AF_INET)
  {
    sockaddr_in const* in1 = reinterpret_cast<sockaddr_in const*>(&first);
    sockaddr_in const* in2 = reinterpret_cast<sockaddr_in const*>(&second);

    if (in1->sin_port != in2->sin_port)
      return false;

    return in1->sin_addr.s_addr == in2->sin_addr.s_addr;
  }

#ifndef RT_PLATFORM_WINDOWS
  if (first.ss_family == AF_UNIX)
  {
    sockaddr_un const* un1 = reinterpret_cast<sockaddr_un const*>(&first);
    sockaddr_un const* un2 = reinterpret_cast<sockaddr_un const*>(&second);

    return 0 == strncmp(un1->sun_path, un2->sun_path, UNIX_PATH_MAX);
  }
#endif

  RT_ASSERT(false);
  return false;
}

bool
rtRemoteSameEndpoint(rtRemoteEndPointPtr const& first, rtRemoteEndPointPtr const& second)
{
  if (auto firstLocal = std::dynamic_pointer_cast<rtRemoteFileEndPoint>(first))
  {
    if (auto secondLocal = std::dynamic_pointer_cast<rtRemoteFileEndPoint>(second))
      return *firstLocal == *secondLocal;
    else
      return false;
  }
  else if (auto firstRemote = std::dynamic_pointer_cast<rtRemoteIPEndPoint>(first))
  {
    if (auto secondRemote = std::dynamic_pointer_cast<rtRemoteIPEndPoint>(second))
      return *firstRemote == *secondRemote;
    else
      return false;
  }
  else
  {
    RT_ASSERT(false);
    return false;
  }
}

rtNetType
rtRemoteParseNetType(std::string const& host)
{
  int ret;
  struct addrinfo hint, *res = NULL;
  memset(&hint, 0, sizeof hint);
  
  hint.ai_family = AF_UNSPEC;
  hint.ai_flags = AI_NUMERICHOST;

  ret = getaddrinfo(host.c_str(), NULL, &hint, &res);
  if (ret)
  {
    freeaddrinfo(res);
    rtLogWarn("unable to determine NetType (network layer protocol) for host: %s", host.c_str());
    return rtNetType::NONE;
  }
  else
  {
    if (res->ai_family == AF_INET)
      return rtNetType::IPV4;
    else if (res->ai_family == AF_INET6)
      return rtNetType::IPV6;
    else
    {
      freeaddrinfo(res);
      rtLogWarn("unable to determine NetType (network layer protocol) for host: %s", host.c_str());
      return rtNetType::NONE;
    }
  }
}

rtCastType
rtRemoteParseCastType(std::string const& host)
{
  std::string prefix;
  rtNetType netType = rtRemoteParseNetType(host);
  if (netType == rtNetType::IPV4)
  {
    prefix = host.substr(0, host.find('.'));
    if (stoi(prefix) >= 224 && stoi(prefix) <= 239)
      return rtCastType::MULTICAST;
    else
      return rtCastType::UNICAST;
  }
  else if (netType == rtNetType::IPV6)
  {
    prefix = host.substr(0, 2);
    if (prefix.compare("FF") == 0)
      return rtCastType::MULTICAST;
    else
      return rtCastType::UNICAST;
  }
  else
  {
    return rtCastType::NONE;
  }       
}

rtError
rtRemoteEndpointToDocument(rtRemoteEndPointPtr& endpoint, rtRemoteMessagePtr& doc)
{
  if (auto remoteEndpoint = std::dynamic_pointer_cast<rtRemoteIPEndPoint>(endpoint))
  {
    doc->AddMember(kFieldNameEndpointType, kEndpointTypeRemote, doc->GetAllocator());
    doc->AddMember(kFieldNameScheme, remoteEndpoint->scheme(), doc->GetAllocator());
    doc->AddMember(kFieldNameIp, remoteEndpoint->host(), doc->GetAllocator());
    doc->AddMember(kFieldNamePort, remoteEndpoint->port(), doc->GetAllocator());
    return RT_OK;
  }
  else if (auto localEndpoint = std::dynamic_pointer_cast<rtRemoteFileEndPoint>(endpoint))
  {
    doc->AddMember(kFieldNameEndpointType, kEndpointTypeLocal, doc->GetAllocator());
    doc->AddMember(kFieldNameScheme, localEndpoint->scheme(), doc->GetAllocator());
    doc->AddMember(kFieldNamePath, localEndpoint->path(), doc->GetAllocator());
    return RT_OK;
  }
  else
  {
    return RT_FAIL;
  }
  return RT_OK;
}

rtError
rtRemoteDocumentToEndpoint(rtRemoteMessagePtr const& doc, rtRemoteEndPointPtr& endpoint)
{
  RT_ASSERT(doc->HasMember(kFieldNameScheme));
  RT_ASSERT(doc->HasMember(kFieldNameEndpointType));
  std::string type, scheme;
  type   = (*doc)[kFieldNameEndpointType].GetString();
  scheme = (*doc)[kFieldNameScheme].GetString();
  
  if (type.compare(kEndpointTypeLocal) == 0)
  {
    RT_ASSERT(doc->HasMember(kFieldNamePath));
    std::string path;
    path = (*doc)[kFieldNamePath].GetString();
    // create and return local endpoint address
    endpoint = std::make_shared<rtRemoteFileEndPoint>(scheme, path);
    return RT_OK;
  }
  else if (type.compare(kEndpointTypeRemote) == 0)
  {
    RT_ASSERT(doc->HasMember(kFieldNameIp));
    RT_ASSERT(doc->HasMember(kFieldNamePort));
    std::string host;
    uint16_t port;
    host = (*doc)[kFieldNameIp].GetString();
    port = (*doc)[kFieldNamePort].GetInt();
    // create and return net endpoint address
    endpoint = std::make_shared<rtRemoteIPEndPoint>(scheme, host, port);
    return RT_OK;
  }
  else
  {
    rtLogError("unknown endpoint type: %s", type.c_str());
    return RT_ERROR;
  }
  return RT_OK;  
}

rtError
rtRemoteCombineDocuments(rtRemoteMessagePtr& target, rtRemoteMessagePtr& source)
{
  RT_ASSERT(target->IsObject());
  RT_ASSERT(source->IsObject());
  for (rapidjson::Value::MemberIterator itr = source->MemberBegin(); itr != source->MemberEnd(); ++itr)
        target->AddMember(itr->name, itr->value, target->GetAllocator());
  return RT_OK;
}

#ifndef RT_PLATFORM_WINDOWS
size_t getline(char **lineptr, size_t *n, FILE *stream)
{
   char *bufptr = NULL;
   char *p = bufptr;
   size_t size;
   int c;

   if (lineptr == NULL) {
      return -1;
   }
   if (stream == NULL) {
      return -1;
   }
   if (n == NULL) {
      return -1;
   }
   bufptr = *lineptr;
   size = *n;

   c = fgetc(stream);
   if (c == EOF) {
      return -1;
   }
   if (bufptr == NULL) {
      bufptr = (char *)malloc(128);
      if (bufptr == NULL) {
         return -1;
      }
      size = 128;
   }
   p = bufptr;
   while(c != EOF) {
      if ((p - bufptr) > (size - 1)) {
         size = size + 128;
         bufptr = (char *)realloc(bufptr, size);
         if (bufptr == NULL) {
            return -1;
         }
      }
      *p++ = c;
      if (c == '\n') {
         break;
      }
      c = fgetc(stream);
   }

   *p++ = '\0';
   *lineptr = bufptr;
   *n = size;

   return p - bufptr - 1;
}
#endif
