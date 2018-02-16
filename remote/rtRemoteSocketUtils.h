#ifndef __RT_REMOTE_SOCKET_UTILS_H__
#define __RT_REMOTE_SOCKET_UTILS_H__

#ifdef RT_PLATFORM_WINDOWS

// to not include winsock 1.x included
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <process.h> // for _getpid
#include <io.h> // for _access

#undef min
#undef max

#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define access _access
#define getpid _getpid
#define F_OK 0

#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <errno.h>
#endif

#include <fcntl.h>
#include <rtError.h>
#include <memory>
#include <string>
#include <vector>

#include "rtRemoteSocketBuffer.h"
#include "rtRemoteMessage.h"

#ifndef UNIX_PATH_MAX
#define UNIX_PATH_MAX    108
#endif


#ifdef RT_PLATFORM_WINDOWS
#define kInvalidSocket INVALID_SOCKET
#else
#define kInvalidSocket (-1)
#endif


#define kUnixSocketTemplateRoot "/tmp/rt_remote_soc"



#ifdef RT_PLATFORM_WINDOWS
typedef SOCKET socket_t;

#define SHUT_RD SD_RECEIVE
#define SHUT_WR SD_SEND
#define SHUT_RDWR SD_BOTH

#define NET_FAILED(err) ((err) == SOCKET_ERROR)

int net_errno();

#else
typedef int socket_t;

#define NET_FAILED(err) ((err) < 0)

inline int net_errno() { return errno; }

#endif

rtError rtParseAddress(sockaddr_storage& ss, char const* addr, uint16_t port, uint32_t* index);
rtError rtParseAddress(sockaddr_storage& ss, char const* s);
rtError rtSocketGetLength(sockaddr_storage const& ss, socklen_t* len);
rtError rtGetInterfaceAddress(char const* name, sockaddr_storage& ss);
rtError rtGetInetAddr(sockaddr_storage const& ss, void** addr);
rtError rtGetPort(sockaddr_storage const& ss, uint16_t* port);
rtError rtPushFd(fd_set* fds, socket_t fd, int* maxFd);
rtError rtReadUntil(socket_t fd, char* buff, int n);
rtError rtReadMessage(socket_t fd, rtRemoteSocketBuffer& buff, rtRemoteMessagePtr& doc);
rtError rtParseMessage(char const* buff, int n, rtRemoteMessagePtr& doc);
std::string rtSocketToString(sockaddr_storage const& ss);

// this really doesn't belong here, but putting it here for now
rtError rtSendDocument(rtRemoteMessage const& m, socket_t fd, sockaddr_storage const* dest);
rtError rtGetPeerName(socket_t fd, sockaddr_storage& endpoint);
rtError rtGetSockName(socket_t fd, sockaddr_storage& endpoint);
rtError	rtCloseSocket(socket_t& fd);
rtError rtGetDefaultInterface(sockaddr_storage& addr, uint16_t port);
rtError rtCreateUnixSocketName(int pid, char* buff, int n);

rtError rtRemoteSocketInit();
rtError rtRemoteSocketShutdown();

void rtSocketSetBlocking(socket_t fd, bool blocking);

#endif

