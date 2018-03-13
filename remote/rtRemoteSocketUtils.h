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

#ifndef __RT_REMOTE_SOCKET_UTILS_H__
#define __RT_REMOTE_SOCKET_UTILS_H__

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <rtError.h>
#include <memory>
#include <string>
#include <vector>

#include "rtRemoteSocketBuffer.h"
#include "rtRemoteMessage.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif

#define kInvalidSocket (-1)
#define kUnixSocketTemplateRoot "/tmp/rt_remote_soc"

rtError rtParseAddress(sockaddr_storage& ss, char const* addr, uint16_t port, uint32_t* index);
rtError rtParseAddress(sockaddr_storage& ss, char const* s);
rtError rtSocketGetLength(sockaddr_storage const& ss, socklen_t* len);
rtError rtGetInterfaceAddress(char const* name, sockaddr_storage& ss);
rtError rtGetInetAddr(sockaddr_storage const& ss, void** addr);
rtError rtGetPort(sockaddr_storage const& ss, uint16_t* port);
rtError rtPushFd(fd_set* fds, int fd, int* maxFd);
rtError rtReadUntil(int fd, char* buff, int n);
rtError rtReadMessage(int fd, rtRemoteSocketBuffer& buff, rtRemoteMessagePtr& doc);
rtError rtParseMessage(char const* buff, int n, rtRemoteMessagePtr& doc);
std::string rtSocketToString(sockaddr_storage const& ss);

// this really doesn't belong here, but putting it here for now
rtError rtSendDocument(rtRemoteMessage const& m, int fd, sockaddr_storage const* dest);
rtError rtGetPeerName(int fd, sockaddr_storage& endpoint);
rtError rtGetSockName(int fd, sockaddr_storage& endpoint);
rtError	rtCloseSocket(int& fd);
rtError rtGetDefaultInterface(sockaddr_storage& addr, uint16_t port);
rtError rtCreateUnixSocketName(pid_t pid, char* buff, int n);

#endif
