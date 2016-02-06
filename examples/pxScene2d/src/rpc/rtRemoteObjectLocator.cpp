#include "rtRemoteObjectLocator.h"
#include "rtSocketUtils.h"

#include <netdb.h>
#include <ifaddrs.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtLog.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>


static rtError 
read_until(int fd, char* buff, int n)
{
  ssize_t bytes_read = 0;
  ssize_t bytes_to_read = n;

  while (bytes_read < bytes_to_read)
  {
    ssize_t n = read(fd, buff + bytes_read, (bytes_to_read - bytes_read));
    if (n == -1)
    {
      rtLogError("failed to read from fd %d. %s", fd, strerror(errno));
      return RT_FAIL;;
    }
    bytes_read += n;
  }
  return RT_OK;
}

static void
dump_document(rapidjson::Document const& doc)
{
  rapidjson::StringBuffer buff;
  rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
  doc.Accept(writer);
  printf("\n%s\n", buff.GetString());
}

rtRemoteObjectLocator::rtRemoteObjectLocator()
  : m_rpc_fd(-1)
  , m_thread(0)
{
  memset(&m_rpc_endpoint, 0, sizeof(m_rpc_endpoint));

  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);
}

rtRemoteObjectLocator::~rtRemoteObjectLocator()
{
  if (m_rpc_fd != -1)
    close(m_rpc_fd);
}

rtError
rtRemoteObjectLocator::open(char const* dstaddr, uint16_t port, char const* srcaddr)
{
  rtError err = RT_OK;


  err = rtParseAddress(m_rpc_endpoint, srcaddr, 0);
  if (err != RT_OK)
    return err;

  m_resolver = new rtRemoteObjectResolver(m_rpc_endpoint);
  err = m_resolver->open(dstaddr, port, srcaddr);
  if (err != RT_OK)
    return err;

  err = open_rpc_listener();
  if (err != RT_OK)
    return err;

  return RT_OK;
}

rtError
rtRemoteObjectLocator::registerObject(std::string const& name, rtObjectRef const& obj)
{
  pthread_mutex_lock(&m_mutex);
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
  {
    pthread_mutex_unlock(&m_mutex);
    rtLogWarn("object %s is already registered", name.c_str());
    return EEXIST;
  }
  m_objects.insert(refmap_t::value_type(name, obj));
  m_resolver->registerObject(name);
  pthread_mutex_unlock(&m_mutex);
  return RT_OK;
}

void*
rtRemoteObjectLocator::run_listener(void* argp)
{
  rtRemoteObjectLocator* locator = reinterpret_cast<rtRemoteObjectLocator *>(argp);
  locator->run_listener();
  return NULL;
}

void
rtRemoteObjectLocator::run_listener()
{
  buff_t buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_rpc_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_rpc_fd, &maxFd);

    for (auto const& c : m_client_list)
    {
      rtPushFd(&read_fds, c.fd, &maxFd);
      rtPushFd(&err_fds, c.fd, &maxFd);
    }

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_rpc_fd, &read_fds))
      do_accept(m_rpc_fd);

    for (auto const& c : m_client_list)
    {
      if (FD_ISSET(c.fd, &read_fds))
        do_readn(c.fd, buff);

      if (FD_ISSET(c.fd, &err_fds))
      {
        // TODO
      }
    }
  }
}

void
rtRemoteObjectLocator::do_accept(int fd)
{
  assert(false);

  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);

  int ret = accept(fd, reinterpret_cast<sockaddr *>(&remote_endpoint), &len);
  if (ret == -1)
  {
    int err = errno;
    rtLogWarn("error accepting new tcp connect. %s", strerror(err));
    return;
  }

  rtSocketGetLength(m_rpc_endpoint, &len);

  void* addr = NULL;
  rtGetInetAddr(m_rpc_endpoint, &addr);

  char addr_buff[128];
  inet_ntop(m_rpc_endpoint.ss_family, addr, addr_buff, len);

  uint16_t port;
  rtGetPort(remote_endpoint, &port);

  rtLogInfo("new tcp connection from %s:%d", addr_buff, port);

  connected_client client;
  client.peer = remote_endpoint;
  client.fd = ret;
  m_client_list.push_back(client);
}

void
rtRemoteObjectLocator::do_readn(int fd, buff_t& buff)
{
  int length = 0;
  rtError err = read_until(fd, reinterpret_cast<char *>(&length), 4);
  if (err != RT_OK)
  {
    rtLogError("failed to read from socket");
    return;
  }

  int n = ntohl(length);

  err = read_until(fd, &buff[0], n);
  if (err != RT_OK)
  {
    rtLogError("failed to read message from socket");
    return;
  }

  sockaddr_storage peer;
  socklen_t len = sizeof(sockaddr_in);

  int ret = getpeername(fd, reinterpret_cast<sockaddr *>(&peer), &len);
  if (ret != 0)
  {
    rtLogError("failed to get tcp peer name. %s", strerror(errno));
    return;
  }

  do_dispatch(&buff[0], n, &peer);
}

void
rtRemoteObjectLocator::do_dispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  printf("read: \"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);

  rapidjson::Document doc;
  rapidjson::MemoryStream stream(buff, n);
  if (doc.ParseStream<rapidjson::kParseDefaultFlags>(stream).HasParseError())
  {
    int begin = doc.GetErrorOffset();
    int end = begin + 16;
    if (end > n)
      end = n;
    int length = (end - begin);

    rtLogWarn("unparsable JSON read: %d", doc.GetParseError());
    rtLogWarn("\"%.*s\"\n", length, buff);
  }
  else
  {
    if (!doc.HasMember("type"))
    {
      rtLogWarn("recived JSON payload without type");
      return;
    }

    std::string cmd = doc["type"].GetString();

    auto itr = m_command_handlers.find(cmd);
    if (itr == m_command_handlers.end())
    {
      rtLogWarn("no command handler registered for: %s", cmd.c_str());
      return;
    }

    socklen_t len;
    rtSocketGetLength(*peer, &len);

    // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
    #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

    rtError err = CALL_MEMBER_FN(*this, itr->second)(doc, reinterpret_cast<sockaddr *>(peer), len);
    if (err != RT_OK)
    {
      rtLogWarn("failed to run command for %s. %d", cmd.c_str(), err);
      return;
    }
  }
}

rtError
rtRemoteObjectLocator::start()
{
  rtError err = m_resolver->start();
  if (err != RT_OK)
    return err;

  pthread_create(&m_thread, NULL, &rtRemoteObjectLocator::run_listener, this);
  return RT_OK;
}

rtError
rtRemoteObjectLocator::findObject(std::string const& name, rtObjectRef& obj, uint32_t timeout)
{
  rtError err = RT_OK;

  // first check local
  auto itr = m_objects.find(name);
  if (itr != m_objects.end())
    obj = itr->second;

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    sockaddr_storage rpc_endpoint;
    err = m_resolver->resolveObject(name, rpc_endpoint, timeout);

    if (err == RT_OK)
    {
      rtLogInfo("I found it!");
      // TODO: create sesssion with 'rpc_endpoint'
    }
  }

  return (obj ? RT_OK : RT_FAIL);
}

rtError
rtRemoteObjectLocator::open_rpc_listener()
{
  int err = 0;
  int ret = 0;

  m_rpc_fd = socket(m_rpc_endpoint.ss_family, SOCK_STREAM, 0);
  if (m_rpc_fd < 0)
  {
    err = errno;
    rtLogError("failed to create TCP socket. %s", strerror(err));
  }

  socklen_t len;
  rtSocketGetLength(m_rpc_endpoint, &len);

  ret = bind(m_rpc_fd, reinterpret_cast<sockaddr *>(&m_rpc_endpoint), len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind socket. %s", strerror(err));
    return RT_FAIL;
  }

  rtSocketGetLength(m_rpc_endpoint, &len);
  ret = getsockname(m_rpc_fd, reinterpret_cast<sockaddr *>(&m_rpc_endpoint), &len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("getsockname: %s", strerror(err));
    return RT_FAIL;
  }
  else
  {
    sockaddr_in* saddr = reinterpret_cast<sockaddr_in *>(&m_rpc_endpoint);
    rtLogInfo("locate tcp socket bound to: %s:%d", inet_ntoa(saddr->sin_addr), ntohs(saddr->sin_port));
  }

  ret = fcntl(m_rpc_fd, F_SETFL, O_NONBLOCK);
  if (ret < 0)
  {
    err = errno;
    rtLogError("fcntl: %s", strerror(errno));
    return RT_FAIL;
  }

  ret = listen(m_rpc_fd, 2);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to put socket in listen mode. %s", strerror(err));
    return RT_FAIL;
  }

  return RT_OK;
}
