#ifndef __RT_REMOTE_UTILS_H__
#define __RT_REMOTE_UTILS_H__

#include "rtRemoteTypes.h"
#include <string>
#include <sys/socket.h>
#include <stdint.h>
#include <rapidjson/document.h>

rtError rtRemoteEndpointAddressToSocket(rtRemoteEndpointPtr addr, sockaddr_storage& ss);
rtError rtRemoteSocketToEndpointAddress(sockaddr_storage const& ss, rtConnType const& connType, rtRemoteEndpointPtr& endpoint);
rtError rtRemoteParseUri(std::string const& uri, std::string& scheme, std::string& path, std::string& host, uint16_t* port);

bool    rtRemoteSameEndpoint(sockaddr_storage const& first, sockaddr_storage const& second);
bool    rtRemoteSameEndpoint(rtRemoteEndpointPtr const& first, rtRemoteEndpointPtr const& second);

rtNetType  rtRemoteParseNetType(std::string const& host);
rtCastType rtRemoteParseCastType(std::string const& host);

rtError rtRemoteDocumentToEndpoint(rtJsonDocPtr const& doc, rtRemoteEndpointPtr& endpoint);
rtError rtRemoteEndpointToDocument(rtRemoteEndpointPtr& endpoint, rtJsonDocPtr& doc);
rtError rtRemoteCombineDocuments(rtJsonDocPtr& target, rtJsonDocPtr& source);

#endif