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

#ifndef __RT_REMOTE_UTILS_H__
#define __RT_REMOTE_UTILS_H__

#include "rtRemoteMessage.h"
#include "rtRemoteTypes.h"
#include "rtRemoteEndpoint.h"

#include <rtError.h>
#include <string>
#include <sys/socket.h>
#include <stdint.h>

rtError rtRemoteEndpointAddressToSocket(rtRemoteEndPointPtr addr, sockaddr_storage& ss);
rtError rtRemoteSocketToEndpointAddress(sockaddr_storage const& ss, rtConnType const& connType, rtRemoteEndPointPtr& endpoint);
rtError rtRemoteParseUri(std::string const& uri, std::string& scheme, std::string& path, std::string& host, uint16_t* port);

bool    rtRemoteSameEndpoint(sockaddr_storage const& first, sockaddr_storage const& second);
bool    rtRemoteSameEndpoint(rtRemoteEndPointPtr const& first, rtRemoteEndPointPtr const& second);

rtNetType  rtRemoteParseNetType(std::string const& host);
rtCastType rtRemoteParseCastType(std::string const& host);

rtError rtRemoteDocumentToEndpoint(rtRemoteMessagePtr const& doc, rtRemoteEndPointPtr& endpoint);
rtError rtRemoteEndpointToDocument(rtRemoteEndPointPtr& endpoint, rtRemoteMessagePtr& doc);
rtError rtRemoteCombineDocuments(rtRemoteMessagePtr& target, rtRemoteMessagePtr& source);

#endif
