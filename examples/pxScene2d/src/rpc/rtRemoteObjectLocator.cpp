#include "rtRemoteObjectLocator.h"
#include "rtRemoteObject.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"
#include "rtRpcClient.h"

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

static const int kMaxMessageLength = (1024 * 16);

static void
set_status(rapidjson::Document& doc, int status, char const* status_message = NULL)
{
  doc.AddMember(kFieldNameStatusCode, status, doc.GetAllocator());
  if (status_message)
    doc.AddMember(kFieldNameStatusMessage, std::string(status_message), doc.GetAllocator());
}

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

  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeOpenSessionRequest, &rtRemoteObjectLocator::on_open_session));
  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeGetByNameRequest, &rtRemoteObjectLocator::on_get));
  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeGetByIndexRequest, &rtRemoteObjectLocator::on_get));;
  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeSetByNameRequest, &rtRemoteObjectLocator::on_set));
  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeSetByIndexRequest, &rtRemoteObjectLocator::on_set));
  m_command_handlers.insert(cmd_handler_map_t::value_type(
    kMessageTypeMethodCallRequest, &rtRemoteObjectLocator::on_method_call));
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
  char const* message_type = rtMessage_GetMessageType(*doc);

  auto itr = m_command_handlers.find(message_type);
  if (itr == m_command_handlers.end())
  {
    rtLogWarn("no command handler registered for: %s", message_type);
    return;
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  rtError err = CALL_MEMBER_FN(*this, itr->second)(doc, fd, peer);
  if (err != RT_OK)
  {
    rtLogWarn("failed to run command for %s. %d", message_type, err);
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
      std::shared_ptr<rtRpcClient> transport;
      std::string const transport_name = rtSocketToString(rpc_endpoint);

      auto itr = m_transports.find(transport_name);
      if (itr != m_transports.end())
        transport = itr->second;

      if (!transport)
      {
        transport.reset(new rtRpcClient(rpc_endpoint));
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
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  int fd = -1;

  pthread_mutex_lock(&m_mutex);
  auto itr = m_objects.find(id);
  if (itr != m_objects.end())
  {
    for (auto const& c : m_client_list)
    {
      if (same_endpoint(soc, c.peer))
      {
        rtLogInfo("new session for %s added to %s", rtSocketToString(soc).c_str(), id);
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
    doc.AddMember(kFieldNameMessageType, kMessageTypeOpenSessionResponse, doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, std::string(id), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key, doc.GetAllocator());
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
rtRemoteObjectLocator::on_get(rtJsonDocPtr_t const& doc, int fd, sockaddr_storage const& /*soc*/)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr_t res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = get_object(id);
  if (!obj)
  {
    res->AddMember(kFieldNameStatusCode, 1, res->GetAllocator());
    res->AddMember(kFieldNameStatusMessage, std::string("object not found"), res->GetAllocator());
  }
  else
  {
    rtError err;
    rtValue value;

    uint32_t    index;
    char const* name = rtMessage_GetPropertyName(*doc);

    if (name)
    {
      err = obj->Get(name, &value);
      if (err != RT_OK)
        rtLogWarn("failed to get property: %s", name);
    }
    else
    {
      index = rtMessage_GetPropertyIndex(*doc);
      if (index != kInvalidPropertyIndex)
        err = obj->Get(index, &value);
    }
    if (err == RT_OK)
    {
      rapidjson::Value val;
      if (value.getType() == RT_functionType)
      {
        val.SetObject();
        val.AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());
        if (name)
          val.AddMember(kFieldNameFunctionName, std::string(name), res->GetAllocator());
        else
          val.AddMember(kFieldNameFunctionIndex, index, res->GetAllocator());
        val.AddMember(kFieldNameValueType, static_cast<int>(RT_functionType), res->GetAllocator());
      }
      else
      {
        err = rtValueWriter::write(value, val, *res);
        if (err != RT_OK)
          rtLogWarn("failed to write value: %d", err);
      }
      res->AddMember("value", val, res->GetAllocator());
      res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    }
    else
    {
      res->AddMember(kFieldNameStatusCode, static_cast<int32_t>(err), res->GetAllocator());
    }
    err = rtSendDocument(*res, fd, NULL);
  }

  return RT_OK;
}

rtError
rtRemoteObjectLocator::on_set(rtJsonDocPtr_t const& doc, int fd, sockaddr_storage const& /*soc*/)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr_t res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = get_object(id);
  if (!obj)
  {
    res->AddMember(kFieldNameStatusCode, 1, res->GetAllocator());
    res->AddMember(kFieldNameStatusMessage, std::string("object not found"), res->GetAllocator());
  }
  else
  {
    uint32_t index;
    rtError err = RT_FAIL;

    rtValue value;

    auto itr = doc->FindMember("value");
    if (itr != doc->MemberEnd())
      err = rtValueReader::read(value, itr->value);

    if (err == RT_OK)
    {
      char const* name = rtMessage_GetPropertyName(*doc);
      if (name)
      {
        err = obj->Set(name, &value);
        rtLogDebug("set %s: %d", name, err);
      }
      else
      {
        index = rtMessage_GetPropertyIndex(*doc);
        if (index != kInvalidPropertyIndex)
          err = obj->Set(index, &value);

      }
    }

    if (err == RT_OK)
    {
      res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    }

    res->AddMember(kFieldNameStatusCode, static_cast<int>(err), res->GetAllocator());
    err = rtSendDocument(*res, fd, NULL);
  }
  return RT_OK;
}

rtError
rtRemoteObjectLocator::on_method_call(rtJsonDocPtr_t const& doc, int fd, sockaddr_storage const& /*soc*/)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);
  rtError err   = RT_OK;

  rapidjson::Document res;
  res.SetObject();
  res.AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res.GetAllocator());
  res.AddMember(kFieldNameCorrelationKey, key, res.GetAllocator());

  rtObjectRef obj = get_object(id);
  if (!obj)
  {
    set_status(res, 1, "object not found");
  }
  else
  {
    auto function_name = doc->FindMember(kFieldNameFunctionName);
    if (function_name == doc->MemberEnd())
    {
      rtLogInfo("message missing %s field", kFieldNameFunctionName);
      return RT_FAIL;
    }

    rtFunctionRef func;
    err = obj.get<rtFunctionRef>(function_name->value.GetString(), func);
    if (err == RT_OK)
    {
      // virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
      std::vector<rtValue> argv;

      rapidjson::Value const& args = (*doc)[kFieldNameFunctionArgs];
      for (rapidjson::Value::ConstValueIterator itr = args.Begin(); itr != args.End(); ++itr)
      {
        rtValue arg;
        rtValueReader::read(arg, *itr);
        argv.push_back(arg);
      }

      rtValue return_value;
      err = func->Send(static_cast<int>(argv.size()), &argv[0], &return_value);
      if (err == RT_OK)
      {
        rapidjson::Value val;
        rtValueWriter::write(return_value, val, res);
        res.AddMember(kFieldNameFunctionReturn, val, res.GetAllocator());
      }
      
      set_status(res, 0);
    }
    else
    {
      set_status(res, 1, "ENOTFOUND");
    }
  }

  err = rtSendDocument(res, fd, NULL);
  if (err != RT_OK)
    rtLogWarn("failed to send response. %d", err);

  return RT_OK;
}
