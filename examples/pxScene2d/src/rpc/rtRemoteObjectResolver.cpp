#include "rtRemoteObjectResolver.h"
#include "rtSocketUtils.h"

#include <rtLog.h>

#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static std::string
createSearchRequest(std::string const& name, pid_t pid)
{
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember("object-id", name, doc.GetAllocator());
  doc.AddMember("type", "search", doc.GetAllocator());
  doc.AddMember("source-id", pid, doc.GetAllocator());

  rapidjson::GenericStringBuffer<rapidjson::UTF8<> > buff;
  rapidjson::Writer<rapidjson::GenericStringBuffer<rapidjson::UTF8<> > > writer(buff);
  doc.Accept(writer);
  return std::string(buff.GetString(), buff.GetSize());
}

rtRemoteObjectResolver::rtRemoteObjectResolver(sockaddr_storage const& rpc_endpoint)
  : m_mcast_fd(-1)
  , m_ucast_fd(-1)
  , m_ucast_len(0)
  , m_read_thread(0)
  , m_pid(getpid())
{
  memset(&m_mcast_dest, 0, sizeof(m_mcast_dest));
  memset(&m_mcast_src, 0, sizeof(m_mcast_src));
  memset(&m_ucast_endpoint, 0, sizeof(m_ucast_endpoint));

  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);

  m_command_handlers.insert(cmd_handler_map_t::value_type("search", &rtRemoteObjectResolver::on_search));
  m_command_handlers.insert(cmd_handler_map_t::value_type("locate", &rtRemoteObjectResolver::on_locate));
    
  char addr_buff[128];

  void* addr = NULL;
  rtGetInetAddr(rpc_endpoint, &addr);

  socklen_t len;
  rtSocketGetLength(rpc_endpoint, &len);
  char const* p = inet_ntop(rpc_endpoint.ss_family, addr, addr_buff, len);
  if (p)
    m_rpc_addr = p;

  rtGetPort(rpc_endpoint, &m_rpc_port);
}

rtRemoteObjectResolver::~rtRemoteObjectResolver()
{
  if (m_mcast_fd != -1)
    close(m_mcast_fd);
  if (m_ucast_fd != -1)
    close(m_ucast_fd);
}

rtError
rtRemoteObjectResolver::open(char const* dstaddr, uint16_t port, char const* srcaddr)
{
  rtError err = RT_OK;

  err = rtParseAddress(m_mcast_dest, dstaddr, port);
  if (err != RT_OK)
    return err;

  // TODO: no need to parse srcaddr multiple times, just copy result
  err = rtParseAddress(m_mcast_src, srcaddr, port);
  if (err != RT_OK)
    return err;

  err = rtParseAddress(m_ucast_endpoint, srcaddr, 0);
  if (err != RT_OK)
    return err;

  err = open_unicast_socket();
  if (err != RT_OK)
    return err;

  return RT_OK;
}

rtError
rtRemoteObjectResolver::open_unicast_socket()
{
  int ret = 0;
  int err = 0;

  m_ucast_fd = socket(m_ucast_endpoint.ss_family, SOCK_DGRAM, 0);
  if (m_ucast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ucast_endpoint.ss_family, strerror(err));
    return RT_FAIL;
  }

  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);

  // listen on ANY port
  ret = bind(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to bind unicast endpoint: %s", strerror(err));
    return RT_FAIL;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ucast_endpoint, &m_ucast_len);
  ret = getsockname(m_ucast_fd, reinterpret_cast<sockaddr *>(&m_ucast_endpoint), &m_ucast_len);
  if (ret < 0)
  {
    err = errno;
    rtLogError("failed to get socketname. %s", strerror(err));
    return RT_FAIL;
  }
  else
  {
    sockaddr_in* saddr = reinterpret_cast<sockaddr_in *>(&m_ucast_endpoint);
    rtLogInfo("local udp socket bound to %s:%d", inet_ntoa(saddr->sin_addr), ntohs(saddr->sin_port));
  }

  // btw, when we use this socket to send multicast, use the right interface
  in_addr mcast_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;
  err = setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_IF, reinterpret_cast<char *>(&mcast_interface),
    sizeof(mcast_interface));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set outgoing multicast interface. %s", strerror(err));
    return RT_FAIL;
  }

  #if 0
  char loop = 1;
  if (setsockopt(m_ucast_fd, IPPROTO_IP, IP_MULTICAST_LOOP, (char *)&loop, sizeof(loop)) < 0)
  {
    rtLogError("failed to disable multicast loopback: %s", strerror(errno));
    return RT_FAIL;
  }
  #endif

  return RT_OK;
}

rtError
rtRemoteObjectResolver::on_search(rapidjson::Document const& doc, sockaddr* soc, socklen_t /*len*/)
{
  // sockaddr_in* v4 = reinterpret_cast<sockaddr_in *>(src);
  // rtLogInfo("new message from %s:%d", inet_ntoa(v4->sin_addr), htons(v4->sin_port));
  if (doc.HasMember("source-id"))
  {
    int pid = doc["source-id"].GetInt();
    if (m_pid == pid)
      return RT_OK;
  }

  std::string id = doc["object-id"].GetString();

  auto itr = m_registered_objects.end();

  if (doc.HasMember("object-id"))
  {
    pthread_mutex_lock(&m_mutex);
    itr = m_registered_objects.find(id);
    pthread_mutex_unlock(&m_mutex);
  }

  if (itr != m_registered_objects.end())
  {

    // END TODO

    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("object-id", id, doc.GetAllocator());
    doc.AddMember("type", "locate", doc.GetAllocator());

    doc.AddMember("ip", m_rpc_addr, doc.GetAllocator());
    doc.AddMember("port", m_rpc_port, doc.GetAllocator());

    rapidjson::StringBuffer buff;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buff);
    doc.Accept(writer);

    printf("send: \"%.*s\"\n", int(buff.GetSize()), buff.GetString());

    if (sendto(m_ucast_fd, buff.GetString(), buff.GetSize(), MSG_NOSIGNAL,soc, m_ucast_len) < 0)
      rtLogError("failed to send: %s", strerror(errno));
  }

  return RT_OK;
}

rtError
rtRemoteObjectResolver::on_locate(rapidjson::Document const& doc, sockaddr* /*soc*/, socklen_t /*len*/)
{
  // dump_document(doc);
  return RT_OK;
}

void*
rtRemoteObjectResolver::run_listener(void* argp)
{
  rtRemoteObjectResolver* resolver = reinterpret_cast<rtRemoteObjectResolver *>(argp);
  resolver->run_listener();
  return NULL;
}

rtError
rtRemoteObjectResolver::resolveObject(std::string const& name, sockaddr_storage& endpoint, uint32_t timeout)
{
  std::string req = createSearchRequest(name, m_pid);
  sockaddr_in* addr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest);

  socklen_t len;
  rtSocketGetLength(endpoint, &len);

  rtLogInfo("sendto: %s:%d", inet_ntoa(addr->sin_addr), ntohs(addr->sin_port));
  if (sendto(m_ucast_fd, req.c_str(), req.size(), MSG_NOSIGNAL, reinterpret_cast<sockaddr *>(&m_mcast_dest), len) < 0)
  {
    rtLogError("failed to send: %s", strerror(errno));
    return RT_FAIL;
  }

  return RT_OK;
}

void
rtRemoteObjectResolver::run_listener()
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
    rtPushFd(&read_fds, m_mcast_fd, &maxFd);
    rtPushFd(&read_fds, m_ucast_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_mcast_fd, &maxFd);
    rtPushFd(&err_fds, m_ucast_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed: %s", strerror(err));
      continue;
    }

    if (FD_ISSET(m_mcast_fd, &read_fds))
      do_read(m_mcast_fd, buff);

    if (FD_ISSET(m_ucast_fd, &read_fds))
      do_read(m_ucast_fd, buff);
  }
}

void
rtRemoteObjectResolver::do_read(int fd, buff_t& buff)
{
  // we only suppor v4 right now. not sure how recvfrom supports v6 and v4
  sockaddr_storage src;
  socklen_t len = sizeof(sockaddr_in);

  #if 0
  ssize_t n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  ssize_t n = recvfrom(fd, &buff[0], buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    do_dispatch(&buff[0], static_cast<int>(n), &src);
}

void
rtRemoteObjectResolver::do_dispatch(char const* buff, int n, sockaddr_storage* peer)
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
rtRemoteObjectResolver::start()
{
  rtError err = open_multicast_socket();
  if (err != RT_OK)
    return err;

  pthread_create(&m_read_thread, NULL, &rtRemoteObjectResolver::run_listener, this);
  return RT_OK;
}

rtError
rtRemoteObjectResolver::registerObject(std::string const& name)
{
  pthread_mutex_lock(&m_mutex);
  m_registered_objects.insert(name);
  pthread_mutex_unlock(&m_mutex);
  return RT_OK;
}

rtError
rtRemoteObjectResolver::open_multicast_socket()
{
  int err = 0;

  m_mcast_fd = socket(m_mcast_dest.ss_family, SOCK_DGRAM, 0);
  if (m_mcast_fd < 0)
  {
    err = errno;
    rtLogError("failed to create multicast socket. %s", strerror(err));
    return RT_FAIL;
  }

  // re-use because multiple applications may want to join group on same machine
  int reuse = 1;
  err = setsockopt(m_mcast_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to set reuseaddr. %s", strerror(err));
    return RT_FAIL;
  }

  sockaddr_in saddr = *(reinterpret_cast<sockaddr_in *>(&m_mcast_src));
  saddr.sin_addr.s_addr = INADDR_ANY;
  err = bind(m_mcast_fd, reinterpret_cast<sockaddr *>(&saddr), sizeof(sockaddr_in));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to bind socket. %s", strerror(err));
    return RT_FAIL;
  }

  // join group
  ip_mreq group;
  group.imr_multiaddr = reinterpret_cast<sockaddr_in *>(&m_mcast_dest)->sin_addr;
  group.imr_interface = reinterpret_cast<sockaddr_in *>(&m_mcast_src)->sin_addr;

  err = setsockopt(m_mcast_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group));
  if (err < 0)
  {
    err = errno;
    rtLogError("failed to join mcast group. %s", strerror(err));
    return RT_FAIL;
  }

  rtLogInfo("successfully joined multicast group: %s on interface: %s",
      inet_ntoa(group.imr_multiaddr), inet_ntoa(group.imr_interface));

  return RT_OK;
}
