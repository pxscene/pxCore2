#ifndef __RT_SOCKET_UTILS_H__
#define __RT_SOCKET_UTILS_H__

#include <sys/socket.h>
#include "../rtError.h"
#include <memory>
#include <string>
#include <vector>

#include "rtRemoteTypes.h"

rtError rtParseAddress(sockaddr_storage& ss, char const* addr, uint16_t port, uint32_t* index);
rtError rtSocketGetLength(sockaddr_storage const& ss, socklen_t* len);
rtError rtGetInterfaceAddress(char const* name, sockaddr_storage& ss);
rtError rtGetInetAddr(sockaddr_storage const& ss, void** addr);
rtError rtGetPort(sockaddr_storage const& ss, uint16_t* port);
rtError rtPushFd(fd_set* fds, int fd, int* max_fd);
rtError rtReadUntil(int fd, char* buff, int n);
rtError rtReadMessage(int fd, rtSocketBuffer& buff, rtJsonDocPtr& doc);
rtError rtParseMessage(char const* buff, int n, rtJsonDocPtr& doc);
std::string rtSocketToString(sockaddr_storage const& ss);

// this really doesn't belong here, but putting it here for now
rtError rtSendDocument(rapidjson::Document const& doc, int fd, sockaddr_storage const* dest);
rtError rtGetPeerName(int fd, sockaddr_storage& endpoint);
rtError rtGetSockName(int fd, sockaddr_storage& endpoint);
rtError	rtCloseSocket(int& fd);
rtError rtGetDefaultInterface(sockaddr_storage& addr, uint16_t port);

#endif
