#include "rtRemoteServer.h"
#include "rtRemoteObject.h"
#include "rtObjectCache.h"
#include "rtSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteClient.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtRemoteConfig.h"
#include "rtRemoteFactory.h"

#include <sstream>
#include <set>
#include <algorithm>

#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtLog.h>
#include <dirent.h>

static bool
is_valid_pid(char const* s)
{
  while (s && *s)
  {
    if (!isdigit(*s++))
      return false;
  }
  return true;
}

static int
parse_pid(char const* s)
{
  int pid = -1;

  char const* p = s + strlen(s) - 1;
  while (p && *p)
  {
    if (!isdigit(*p))
      break;
    p--;
  }
  if (p)
  {
    p++;
    pid = strtol(p, nullptr, 10);
  }
  return pid;
}

static void
cleanup_stale_unix_sockets()
{
  DIR* d = opendir("/proc/");
  if (!d)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("failed to open directory /proc. %s", rtStrError(e));
    return;
  }

  dirent* entry = reinterpret_cast<dirent *>(malloc(1024));
  dirent* result = nullptr;

  std::set<int> active_pids;

  int ret = 0;
  do
  {
    ret = readdir_r(d, entry, &result);
    if (ret == 0 && (result != nullptr))
    {
      if (is_valid_pid(result->d_name))
      {
        int pid = static_cast<int>(strtol(result->d_name, nullptr, 10));
        active_pids.insert(pid);
      }
    }
  }
  while ((result != nullptr) && ret == 0);
  closedir(d);

  d = opendir("/tmp");
  if (!d)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("failed to open directory /tmp. %s", rtStrError(e));
    return;
  }

  char path[UNIX_PATH_MAX];
  do
  {
    ret = readdir_r(d, entry, &result);
    if (ret == 0 && (result != nullptr))
    {
      memset(path, 0, sizeof(path));
      strcpy(path, "/tmp/");
      strcat(path, result->d_name);
      if (strncmp(path, kUnixSocketTemplateRoot, strlen(kUnixSocketTemplateRoot)) == 0)
      {
        int pid = parse_pid(result->d_name);
        if (active_pids.find(pid) == active_pids.end())
        {
          rtLogInfo("removing inactive unix socket %s", path);
          int ret = unlink(path);
          if (ret == -1)
          {
            rtError e = rtErrorFromErrno(errno);
            rtLogWarn("failed to remove inactive unix socket %s. %s",
              path, rtStrError(e));
          }
        }
      }
    }
  }
  while ((result != nullptr) && ret == 0);

  closedir(d);
  free(entry);
}

static bool
is_unix_domain(rtRemoteEnvironment* env)
{
  if (!env)
    return false;
 
  char const* s = env->Config->getString("rt.rpc.server.socket_family");
  if (s && !strcmp(s, "unix"))
    return true;

  return false;
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

  if (addr1.ss_family == AF_UNIX)
  {
    sockaddr_un const* un1 = reinterpret_cast<sockaddr_un const*>(&addr1);
    sockaddr_un const* un2 = reinterpret_cast<sockaddr_un const*>(&addr2);

    return 0 == strncmp(un1->sun_path, un2->sun_path, UNIX_PATH_MAX);
  }

  assert(false);
  return false;
}

rtRemoteServer::rtRemoteServer(rtRemoteEnvironment* env)
  : m_listen_fd(-1)
  , m_resolver(nullptr)
  , m_keep_alive_interval(15)
  , m_env(env)
{
  memset(&m_rpc_endpoint, 0, sizeof(m_rpc_endpoint));

  m_shutdown_pipe[0] = -1;
  m_shutdown_pipe[1] = -1;
  m_queueing_mode = !m_env->Config->getBool("rt.rpc.server.use_dispatch_thread");

  int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
  if (ret != 0)
  {
    rtError e = rtErrorFromErrno(ret);
    rtLogWarn("failed to create shutdown pipe. %s", rtStrError(e));
  }

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeOpenSessionRequest,
    std::bind(&rtRemoteServer::onOpenSession, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByNameRequest,
    std::bind(&rtRemoteServer::onGet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByIndexRequest,
    std::bind(&rtRemoteServer::onGet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByNameRequest,
    std::bind(&rtRemoteServer::onSet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByIndexRequest,
    std::bind(&rtRemoteServer::onSet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeMethodCallRequest,
    std::bind(&rtRemoteServer::onMethodCall, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeKeepAliveRequest,
    std::bind(&rtRemoteServer::onKeepAlive, this, std::placeholders::_1, std::placeholders::_2)));
}

rtRemoteServer::~rtRemoteServer()
{
  if (m_shutdown_pipe[0] != -1)
  {
    char buff[] = {"shutdown"};
    ssize_t n = write(m_shutdown_pipe[1], buff, sizeof(buff));
    if (n == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("failed to write. %s", rtStrError(e));
    }

    if (m_thread)
    {
      m_thread->join();
      m_thread.reset();
    }
  }

  if (m_listen_fd != -1)
    ::close(m_listen_fd);
  if (m_shutdown_pipe[0] != -1)
    ::close(m_shutdown_pipe[0]);
  if (m_shutdown_pipe[1] != -1)
    ::close(m_shutdown_pipe[1] = -1);

  if (m_resolver)
  {
    m_resolver->close();
    delete m_resolver;
  }
}

rtError
rtRemoteServer::open()
{
  rtError err = openRpcListener();
  if (err != RT_OK)
    return err;

  m_resolver = rtRemoteFactory::rtRemoteCreateResolver(m_env);
  err = start();
  if (err != RT_OK)
  {
    rtLogError("failed to start rpc server. %s", rtStrError(err));
    return err;
  }

  return err;
}

rtError
rtRemoteServer::registerObject(std::string const& name, rtObjectRef const& obj)
{
  rtObjectRef ref = m_env->ObjectCache->findObject(name);
  if (!ref)
    m_env->ObjectCache->insert(name, obj, -1);
  m_resolver->registerObject(name, m_rpc_endpoint);
  return RT_OK;
}

void
rtRemoteServer::runListener()
{
  time_t lastKeepAliveCheck = 0;

  rtSocketBuffer buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_listen_fd, &maxFd);
    rtPushFd(&read_fds, m_shutdown_pipe[0], &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_listen_fd, &maxFd);

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
    if (ret == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    // right now we just use this to signal "hey" more fds added
    // later we'll use this to shutdown
    if (FD_ISSET(m_shutdown_pipe[0], &read_fds))
    {
      rtLogInfo("got shutdown signal");
      return;
    }

    if (FD_ISSET(m_listen_fd, &read_fds))
      doAccept(m_listen_fd);

    time_t now = time(nullptr);
    if (now - lastKeepAliveCheck > 1)
    {
      rtError err = removeStaleObjects();
      if (err == RT_OK)
        lastKeepAliveCheck = now;
    }
  }
}

void
rtRemoteServer::doAccept(int fd)
{
  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);

  int ret = accept(fd, reinterpret_cast<sockaddr *>(&remote_endpoint), &len);
  if (ret == -1)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("error accepting new tcp connect. %s", rtStrError(e));
    return;
  }
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remote_endpoint).c_str(), ret);

  sockaddr_storage local_endpoint;
  memset(&local_endpoint, 0, sizeof(sockaddr_storage));
  rtGetSockName(fd, local_endpoint);

  std::shared_ptr<rtRemoteClient> newClient(new rtRemoteClient(m_env, ret, local_endpoint, remote_endpoint));
  newClient->setMessageCallback(std::bind(&rtRemoteServer::onIncomingMessage, this, std::placeholders::_1, std::placeholders::_2));
  newClient->open();
  m_connected_clients.push_back(newClient);
}

rtError
rtRemoteServer::onIncomingMessage(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg)
{
  rtError e = RT_OK;
  if (m_queueing_mode)
    m_env->enqueueWorkItem(client, msg);
  else
    e = processMessage(client, msg);
  return e;
}

rtError
rtRemoteServer::processMessage(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg)
{
  rtError e = RT_FAIL;

  char const* message_type = rtMessage_GetMessageType(*msg);

  auto itr = m_command_handlers.find(message_type);
  if (itr == m_command_handlers.end())
    return RT_OK;

  e = itr->second(client, msg);
  if (e != RT_OK)
    rtLogWarn("failed to run command for %s. %s", message_type, rtStrError(e));

  return e;
}

rtError
rtRemoteServer::start()
{
  rtError err = m_resolver->open(m_rpc_endpoint);
  if (err != RT_OK)
  {
    rtLogWarn("failed to open resolver. %s", rtStrError(err));
    return err;
  }

  try
  {
    m_thread.reset(new std::thread(&rtRemoteServer::runListener, this));
  }
  catch (std::exception const& err)
  {
    rtLogError("failed to start listener thread. %s", err.what());
    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtRemoteServer::findObject(std::string const& name, rtObjectRef& obj, uint32_t timeout)
{
  rtError err = RT_OK;
  obj = m_env->ObjectCache->findObject(name);

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    sockaddr_storage rpc_endpoint;
    err = m_resolver->locateObject(name, rpc_endpoint, timeout);

    rtLogDebug("object %s found at endpoint: %s", name.c_str(),
      rtSocketToString(rpc_endpoint).c_str());

    if (err == RT_OK)
    {
      std::shared_ptr<rtRemoteClient> client;
      std::string const endpointName = rtSocketToString(rpc_endpoint);

      std::unique_lock<std::mutex> lock(m_mutex);
      auto itr = m_object_map.find(endpointName);
      if (itr != m_object_map.end())
        client = itr->second;

      // if a client is already "hosting" this object, 
      // then just re-use it
      for (auto i : m_object_map)
      {
        if (same_endpoint(i.second->getRemoteEndpoint(), rpc_endpoint))
        {
          client = i.second;
          break;
        }
      }
      m_mutex.unlock();

      if (!client)
      {
        client.reset(new rtRemoteClient(m_env, rpc_endpoint));
        client->setMessageCallback(std::bind(&rtRemoteServer::onIncomingMessage, this, 
              std::placeholders::_1, std::placeholders::_2));
        err = client->open();
        if (err != RT_OK)
        {
          rtLogWarn("failed to start new client. %s", rtStrError(err));
          return err;
        }

        // we have race condition here. if the transport doesn't exist, two threads may
        // create one but only one will get inserted into the m_connected_client map. I'm not
        // sure that this really matters much
        std::unique_lock<std::mutex> lock(m_mutex);
        m_object_map.insert(ClientMap::value_type(endpointName, client));
      }

      if (client)
      {
        rtRemoteObject* remote(new rtRemoteObject(name, client));
        err = client->startSession(name);
        if (err == RT_OK)
          obj = remote;
      }
    }
  }

  return (obj ? RT_OK : RT_FAIL);
}

rtError
rtRemoteServer::openRpcListener()
{
  int ret = 0;
  char path[UNIX_PATH_MAX];

  memset(path, 0, sizeof(path));
  cleanup_stale_unix_sockets();

  if (is_unix_domain(m_env))
  {
    rtError e = rtCreateUnixSocketName(0, path, sizeof(path));
    if (e != RT_OK)
      return e;

    ret = unlink(path); // reuse path if needed
    if (ret == -1 && errno != ENOENT)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogInfo("error trying to remove %s. %s", path, rtStrError(e));
    }

    struct sockaddr_un *un_addr = reinterpret_cast<sockaddr_un*>(&m_rpc_endpoint);
    un_addr->sun_family = AF_UNIX;
    strncpy(un_addr->sun_path, path, UNIX_PATH_MAX);
  }
  else
  {
    rtGetDefaultInterface(m_rpc_endpoint, 0);
  }

  m_listen_fd = socket(m_rpc_endpoint.ss_family, SOCK_STREAM, 0);
  if (m_listen_fd < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to create socket. %s", rtStrError(e));
    return e;
  }

  fcntl(m_listen_fd, F_SETFD, fcntl(m_listen_fd, F_GETFD) | FD_CLOEXEC);

  if (m_rpc_endpoint.ss_family != AF_UNIX)
  {
    uint32_t one = 1;
    if (-1 == setsockopt(m_listen_fd, SOL_TCP, TCP_NODELAY, &one, sizeof(one)))
      rtLogError("setting TCP_NODELAY failed");
  }

  socklen_t len;
  rtSocketGetLength(m_rpc_endpoint, &len);

  ret = ::bind(m_listen_fd, reinterpret_cast<sockaddr *>(&m_rpc_endpoint), len);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to bind socket. %s", rtStrError(e));
    return e;
  }

  rtGetSockName(m_listen_fd, m_rpc_endpoint);
  rtLogInfo("local rpc listener on: %s", rtSocketToString(m_rpc_endpoint).c_str());

  ret = fcntl(m_listen_fd, F_SETFL, O_NONBLOCK);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("fcntl: %s", rtStrError(e));
    return e;
  }

  ret = listen(m_listen_fd, 2);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to put socket in listen mode. %s", rtStrError(e));
    return e;
  }

  return RT_OK;
}

rtError
rtRemoteServer::onOpenSession(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& req)
{
  rtCorrelationKey key = rtMessage_GetCorrelationKey(*req);
  char const* id = rtMessage_GetObjectId(*req);

  #if 0

  std::unique_lock<std::mutex> lock(m_mutex);
  auto itr = m_objects.find(id);
  if (itr != m_objects.end())
  {
    sockaddr_storage const soc = client->getRemoteEndpoint();
    for (auto const& c : m_clients)
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
  lock.unlock();
  #endif

  rtError err = RT_OK;

  rtJsonDocPtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionResponse, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  err = client->sendDocument(*res);

  return err;
}

rtError
rtRemoteServer::onGet(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(id);
  if (!obj)
  {
    res->AddMember(kFieldNameStatusCode, 1, res->GetAllocator());
    res->AddMember(kFieldNameStatusMessage, std::string("object not found"), res->GetAllocator());
  }
  else
  {
    rtError err = RT_OK;
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
        err = rtValueWriter::write(m_env, value, val, *res);
        if (err != RT_OK)
          rtLogWarn("failed to write value: %d", err);
      }
      res->AddMember(kFieldNameValue, val, res->GetAllocator());
      res->AddMember(kFieldNameStatusCode, 0, res->GetAllocator());
    }
    else
    {
      res->AddMember(kFieldNameStatusCode, static_cast<int32_t>(err), res->GetAllocator());
    }

    err = client->sendDocument(*res);
  }

  return RT_OK;
}

rtError
rtRemoteServer::onSet(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(id);
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

    auto itr = doc->FindMember(kFieldNameValue);
    assert(itr != doc->MemberEnd());

    if (itr != doc->MemberEnd())
      err = rtValueReader::read(value, itr->value, client);

    if (err == RT_OK)
    {
      char const* name = rtMessage_GetPropertyName(*doc);
      if (name)
      {
        err = obj->Set(name, &value);
      }
      else
      {
        index = rtMessage_GetPropertyIndex(*doc);
        if (index != kInvalidPropertyIndex)
          err = obj->Set(index, &value);
      }
    }

    res->AddMember(kFieldNameStatusCode, static_cast<int>(err), res->GetAllocator());
    err = client->sendDocument(*res);
  }
  return RT_OK;
}

rtError
rtRemoteServer::onMethodCall(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);
  rtError err   = RT_OK;

  rapidjson::Document res;
  res.SetObject();
  res.AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res.GetAllocator());
  res.AddMember(kFieldNameCorrelationKey, key, res.GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(id);
  if (!obj && (strcmp(id, "global") != 0))
  {
    rtMessage_SetStatus(res, 1, "failed to find object with id: %s", id);
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
    if (obj) // member function
    {
      err = obj.get<rtFunctionRef>(function_name->value.GetString(), func);
    }
    else
    {
      func = m_env->ObjectCache->findFunction(function_name->value.GetString());
    }

    if (err == RT_OK && !!func)
    {
      // virtual rtError Send(int numArgs, const rtValue* args, rtValue* result) = 0;
      std::vector<rtValue> argv;

      auto itr = doc->FindMember(kFieldNameFunctionArgs);
      if (itr != doc->MemberEnd())
      {
        // rapidjson::Value const& args = (*doc)[kFieldNameFunctionArgs];
        rapidjson::Value const& args = itr->value;
        for (rapidjson::Value::ConstValueIterator itr = args.Begin(); itr != args.End(); ++itr)
        {
          rtValue arg;
          rtValueReader::read(arg, *itr, client);
          argv.push_back(arg);
        }
      }

      rtValue return_value;
      err = func->Send(static_cast<int>(argv.size()), &argv[0], &return_value);
      if (err == RT_OK)
      {
        rapidjson::Value val;
        rtValueWriter::write(m_env, return_value, val, res);
        res.AddMember(kFieldNameFunctionReturn, val, res.GetAllocator());
      }

      rtMessage_SetStatus(res, 0);
    }
    else
    {
      rtMessage_SetStatus(res, 1, "object not found");
    }
  }

  err = client->sendDocument(res);

  if (err != RT_OK)
    rtLogWarn("failed to send response. %d", err);

  return RT_OK;
}

rtError
rtRemoteServer::onKeepAlive(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& req)
{
  uint32_t key = rtMessage_GetCorrelationKey(*req);

  auto itr = req->FindMember(kFieldNameKeepAliveIds);
  if (itr != req->MemberEnd())
  {
    time_t now = time(nullptr);

    std::unique_lock<std::mutex> lock(m_mutex);
    for (rapidjson::Value::ConstValueIterator id  = itr->value.Begin(); id != itr->value.End(); ++id)
    {
      rtError e = m_env->ObjectCache->touch(id->GetString(), now);
      if (e != RT_OK)
        rtLogWarn("error updating last used time for: %s, %s", 
            id->GetString(), rtStrError(e));
    }
  }
  else
  {
    rtLogInfo("got keep-alive without any interesting information");
  }

  rapidjson::Document res;
  res.SetObject();
  res.AddMember(kFieldNameCorrelationKey, key, res.GetAllocator());
  res.AddMember(kFieldNameMessageType, kMessageTypeKeepAliveResponse, res.GetAllocator());

  return client->sendDocument(res);
}

rtError
rtRemoteServer::removeStaleObjects()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  for (auto itr = m_object_map.begin(); itr != m_object_map.end();)
  {
    if (itr->second.use_count() == 1)
      itr = m_object_map.erase(itr);
    else
      ++itr;
  }
  lock.unlock();
  return m_env->ObjectCache->removeUnused(); // m_keep_alive_interval, num_removed);
}
