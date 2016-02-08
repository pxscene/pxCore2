#include "rtRemoteObjectLocator.h"
#include "rtRemoteObject.h"
#include "rtSocketUtils.h"

#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtLog.h>
#include <sstream>
#include <algorithm>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

static const int kMaxMessageLength = (1024 * 16);

static bool
same_endpoint(sockaddr_storage const& addr1, sockaddr_storage const& addr2)
{
  if (addr1.ss_family != addr2.ss_family)
    return false;

  if (addr1.ss_family == AF_INET)
  {
    sockaddr_in const* in1 = reinterpret_cast<sockaddr_in const*>(&addr1);
    sockaddr_in const* in2 = reinterpret_cast<sockaddr_in const*>(&addr2);

    if (in1->sin_port != in2->sin_port)
      return false;

    return in1->sin_addr.s_addr == in2->sin_addr.s_addr;
  }

  assert(false);
  return false;
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
  , m_pipe_write(-1)
  , m_pipe_read(-1)
{
  memset(&m_rpc_endpoint, 0, sizeof(m_rpc_endpoint));

  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);

  int arr[2];
  int ret = pipe(arr);
  if (ret == 0)
  {
    m_pipe_read = arr[0];
    m_pipe_write = arr[1];
  }

  m_command_handlers.insert(cmd_handler_map_t::value_type("session.open.request",
        &rtRemoteObjectLocator::on_open_session));
  m_command_handlers.insert(cmd_handler_map_t::value_type("get.byname.request",
        &rtRemoteObjectLocator::on_get_byname));
  m_command_handlers.insert(cmd_handler_map_t::value_type("get.byindex.request.",
        &rtRemoteObjectLocator::on_get_byindex));;
  m_command_handlers.insert(cmd_handler_map_t::value_type("set.byname.request",
        &rtRemoteObjectLocator::on_set_byname));
  m_command_handlers.insert(cmd_handler_map_t::value_type("set.byindex.request",
        &rtRemoteObjectLocator::on_set_byindex));
}

rtRemoteObjectLocator::~rtRemoteObjectLocator()
{
  if (m_rpc_fd != -1)
    close(m_rpc_fd);
  if (m_pipe_read != -1)
    close(m_pipe_read);
  if (m_pipe_write != -1)
    close(m_pipe_write);
}

rtError
rtRemoteObjectLocator::open(char const* dstaddr, uint16_t port, char const* srcaddr)
{
  rtError err = RT_OK;

  err = rtParseAddress(m_rpc_endpoint, srcaddr, 0);
  if (err != RT_OK)
    return err;

  err = open_rpc_listener();
  if (err != RT_OK)
    return err;

  m_resolver = new rtRemoteObjectResolver(m_rpc_endpoint);
  err = m_resolver->open(dstaddr, port, srcaddr);
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

  object_reference entry;
  entry.object = obj;
  m_objects.insert(refmap_t::value_type(name, entry));

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
  rt_sockbuf_t buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_rpc_fd, &maxFd);
    rtPushFd(&read_fds, m_pipe_read, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_rpc_fd, &maxFd);
    rtPushFd(&err_fds, m_pipe_read, &maxFd);

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

    // right now we just use this to signal "hey" more fds added
    // later we'll use this to shutdown
    if (FD_ISSET(m_pipe_read, &read_fds))
    {
      char ch;
      read(m_pipe_read, &ch, 1);
      continue;
    }

    if (FD_ISSET(m_rpc_fd, &read_fds))
      do_accept(m_rpc_fd);

    for (auto& c : m_client_list)
    {
      if (FD_ISSET(c.fd, &err_fds))
      {
        // TODO
        rtLogWarn("error on fd: %d", c.fd);
      }

      if (FD_ISSET(c.fd, &read_fds))
      {
        if (do_readn(c.fd, buff, c.peer) != RT_OK)
          on_client_disconnect(c);
        }
      }
    }

    auto end = std::remove_if(m_client_list.begin(), m_client_list.end(),
        [](connected_client const& c)
        {
          return c.fd == -1;
        });
    m_client_list.erase(end, m_client_list.end());
}

void
rtRemoteObjectLocator::do_accept(int fd)
{
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
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remote_endpoint).c_str(), ret);

  connected_client client;
  client.peer = remote_endpoint;
  client.fd = ret;
  m_client_list.push_back(client);
}

rtError
rtRemoteObjectLocator::do_readn(int fd, rt_sockbuf_t& buff, sockaddr_storage const& peer)
{
  rtJsonDocPtr_t doc;
  rtError err = rtReadMessage(fd, buff, doc);
  if (err != RT_OK)
    return err;

  do_dispatch(doc, fd, peer);
  return RT_OK;
}

void
rtRemoteObjectLocator::do_dispatch(rtJsonDocPtr_t const& doc, int fd, sockaddr_storage const& peer)
{
  if (!doc->HasMember("type"))
  {
    rtLogWarn("received JSON payload without type");
    return;
  }

  std::string cmd = (*doc)["type"].GetString();

  auto itr = m_command_handlers.find(cmd);
  if (itr == m_command_handlers.end())
  {
    rtLogWarn("no command handler registered for: %s", cmd.c_str());
    return;
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  rtError err = CALL_MEMBER_FN(*this, itr->second)(doc, fd, peer);
  if (err != RT_OK)
  {
    rtLogWarn("failed to run command for %s. %d", cmd.c_str(), err);
    return;
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
    obj = itr->second.object;

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    sockaddr_storage rpc_endpoint;
    err = m_resolver->resolveObject(name, rpc_endpoint, timeout);

    if (err == RT_OK)
    {
      std::shared_ptr<rtRpcTransport> transport;
      std::string const transport_name = rtSocketToString(rpc_endpoint);

      auto itr = m_transports.find(transport_name);
      if (itr != m_transports.end())
        transport = itr->second;

      if (!transport)
      {
        transport.reset(new rtRpcTransport(rpc_endpoint));
        err = transport->start();
        if (err != RT_OK)
        {
          rtLogWarn("failed to start transport. %d", err);
          return err;
        }

        // we have race condition here. if the transport doesn't exist, two threads may
        // create one but only one will get inserted into the m_transports map. I'm not
        // sure that this really matters much
        pthread_mutex_lock(&m_mutex);
        m_transports.insert(tport_map_t::value_type(transport_name, transport));
        pthread_mutex_unlock(&m_mutex);
      }

      if (transport)
      {
        rtRemoteObject* remote(new rtRemoteObject(name, transport));
        err = transport->start_session(name);
        if (err == RT_OK)
          obj = remote;
      }
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

rtError
rtRemoteObjectLocator::on_open_session(rtJsonDocPtr_t const& doc, int /*fd*/, sockaddr_storage const& soc)
{
  if (!doc->HasMember("object-id"))
  {
    rtLogWarn("open session message missing object-id field");
    return RT_FAIL;
  }

  if (!doc->HasMember("corkey"))
  {
    rtLogWarn("message doesn't contain correlation key");
    return RT_FAIL;
  }

  int key = (*doc)["corkey"].GetInt();

  int fd = -1;
  std::string const id = (*doc)["object-id"].GetString();

  pthread_mutex_lock(&m_mutex);
  auto itr = m_objects.find(id);
  if (itr != m_objects.end())
  {
    for (auto const& c : m_client_list)
    {
      if (same_endpoint(soc, c.peer))
      {
        rtLogInfo("new session for %s added to %s", rtSocketToString(soc).c_str(), id.c_str());
        itr->second.client_fds.push_back(c.fd);
        fd = c.fd;
        break;
      }
    }
  }
  pthread_mutex_unlock(&m_mutex);

  rtError err = RT_OK;

  // send ack
  if (fd != -1)
  {
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember("type", "session.open.response", doc.GetAllocator());
    doc.AddMember("object-id", id, doc.GetAllocator());
    doc.AddMember("corkey", key, doc.GetAllocator());
    err = rtSendDocument(doc, fd, NULL);
  }

  return err;
}

rtError
rtRemoteObjectLocator::on_client_disconnect(connected_client& client)
{
  int client_fd = client.fd;

  if (client.fd != -1)
  {
    close(client.fd);
    client.fd = -1;
  }

  for (auto& i : m_objects)
  {
    auto end = std::remove_if(i.second.client_fds.begin(), i.second.client_fds.end(),
        [client_fd](int fd) { return fd == client_fd; });
    i.second.client_fds.erase(end, i.second.client_fds.end());
  }

  return RT_OK;
}

rtObjectRef
rtRemoteObjectLocator::get_object(std::string const& id) const
{
  rtObjectRef obj;

  pthread_mutex_lock(&m_mutex);
  auto itr = m_objects.find(id);
  if (itr != m_objects.end())
    obj = itr->second.object;
  pthread_mutex_unlock(&m_mutex);

  return obj;
}

rtError
rtRemoteObjectLocator::on_get_byname(rtJsonDocPtr_t const& doc, int fd, sockaddr_storage const& /*soc*/)
{
  int key = (*doc)["corkey"].GetInt();

  std::string const id = (*doc)["object-id"].GetString();
  std::string const& property_name = (*doc)["name"].GetString();

  rtJsonDocPtr_t res(new rapidjson::Document());
  res->SetObject();
  res->AddMember("type", "get.byname.response", res->GetAllocator());
  res->AddMember("corkey", key, res->GetAllocator());
  res->AddMember("object-id", id, res->GetAllocator());

  rtObjectRef obj = get_object(id);
  if (!obj)
  {
    res->AddMember("status", 1, res->GetAllocator());
    res->AddMember("status-message", std::string("object not found"), res->GetAllocator());
  }
  else
  {
    rtValue value;
    rtError err = obj->Get(property_name.c_str(), &value);
    if (err == RT_OK)
    {
      rtValueWriter::write(value, *res);
      res->AddMember("status", 0, res->GetAllocator());
    }
    else
    {
      res->AddMember("status", static_cast<int32_t>(err), res->GetAllocator());
      err = rtSendDocument(*res, fd, NULL);
    }
  }

  return RT_OK;
}

rtError
rtRemoteObjectLocator::on_get_byindex(rtJsonDocPtr_t const& doc, int /*fd*/, sockaddr_storage const& /*soc*/)
{
  rtObjectRef obj;
  if (!obj)
    return RT_FAIL;
  return RT_FAIL;
}

rtError
rtRemoteObjectLocator::on_set_byname(rtJsonDocPtr_t const& doc, int /*fd*/, sockaddr_storage const& /*soc*/)
{
  rtObjectRef obj;
  if (!obj)
    return RT_FAIL;

  std::string const& property_name = (*doc)["name"].GetString();

  rtValue value;
  rtError err = rtValueReader::read(value, *doc);
  if (err != RT_OK)
    return err;

  err = obj->Set(property_name.c_str(), &value);
  if (err == RT_OK)
  {
    // TODO
  }

  return err;
}

rtError
rtRemoteObjectLocator::on_set_byindex(rtJsonDocPtr_t const& doc, int /*fd*/, sockaddr_storage const& /*soc*/)
{
  rtObjectRef obj;
  if (!obj)
    return RT_FAIL;
  return RT_FAIL;
}
