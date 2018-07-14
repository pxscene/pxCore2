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

#include "rtRemoteServer.h"
#include "rtRemoteCorrelationKey.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteFunction.h"
#include "rtRemoteObject.h"
#include "rtRemoteObjectCache.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteClient.h"
#include "rtRemoteValueReader.h"
#include "rtRemoteValueWriter.h"
#include "rtRemoteConfig.h"
#include "rtRemoteFactory.h"

#include <limits>
#include <sstream>
#include <set>
#include <algorithm>

#include <sys/stat.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <rtLog.h>
#include <dirent.h>

namespace
{
  bool
  isValidPid(char const* s)
  {
    while (s && *s)
    {
      if (!isdigit(*s++))
        return false;
    }
    return true;
  } // isValidPid

  int
  parsePid(char const* s)
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
  } // parsePid

  void
  cleanupStaleUnixSockets()
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
        if (isValidPid(result->d_name))
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
          int pid = parsePid(result->d_name);
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
  } // cleanupStaleUnixSockets

  bool
  isUnixDomain(rtRemoteEnvironment* env)
  {
    if (!env)
      return false;
  
    std::string family = env->Config->server_socket_family();
    if (!strcmp(family.c_str(), "unix"))
      return true;

    return false;
  } // isUnixDomain

  bool
  sameEndpoint(sockaddr_storage const& addr1, sockaddr_storage const& addr2)
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

    RT_ASSERT(false);
    return false;
  } // sameEndpoint
} // namespace

rtRemoteServer::rtRemoteServer(rtRemoteEnvironment* env)
  : m_listen_fd(-1)
  , m_resolver(nullptr)
  , m_keep_alive_interval(std::numeric_limits<uint32_t>::max())
  , m_env(env)
{
  memset(&m_rpc_endpoint, 0, sizeof(m_rpc_endpoint));

  m_shutdown_pipe[0] = -1;
  m_shutdown_pipe[1] = -1;

  int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
  if (ret != 0)
  {
    rtError e = rtErrorFromErrno(ret);
    rtLogWarn("failed to create shutdown pipe. %s", rtStrError(e));
  }

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeOpenSessionRequest, 
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onOpenSession_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByNameRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onGet_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByIndexRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onGet_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByNameRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onSet_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByIndexRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onSet_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeMethodCallRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onMethodCall_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeKeepAliveRequest,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onKeepAlive_Dispatch, this)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeKeepAliveResponse,
    rtRemoteCallback<rtRemoteMessageHandler>(&rtRemoteServer::onKeepAliveResponse_Dispatch, this)));
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
rtRemoteServer::registerObject(std::string const& objectId, rtObjectRef const& obj)
{
  rtObjectRef ref = m_env->ObjectCache->findObject(objectId);
  if (!ref)
  {
    m_env->ObjectCache->insert(objectId, obj);
    m_env->ObjectCache->markUnevictable(objectId, true);
  }
  m_resolver->registerObject(objectId, m_rpc_endpoint);
  return RT_OK;
}

rtError
rtRemoteServer::unregisterObject(std::string const& objectId)
{
  rtError e = RT_OK;

  if (m_env)
  {
    e = m_env->ObjectCache->markUnevictable(objectId, false);
    if (e != RT_OK)
    {
      rtLogInfo("failed to mark object %s for removal. %s",
          objectId.c_str(), rtStrError(e));
    }
  }

  if (m_resolver)
  {
    e = m_resolver->unregisterObject(objectId);
    if (e != RT_OK)
    {
      rtLogInfo("failed to remove resolver entry for %s. %s", objectId.c_str(),
          rtStrError(e));
    }
  }

  return e;
}

void
rtRemoteServer::runListener()
{
  time_t lastKeepAliveCheck = 0;

  while (true)
  {
    int maxFd = 0;

    fd_set readFds;
    fd_set errFds;

    FD_ZERO(&readFds);
    rtPushFd(&readFds, m_listen_fd, &maxFd);
    rtPushFd(&readFds, m_shutdown_pipe[0], &maxFd);

    FD_ZERO(&errFds);
    rtPushFd(&errFds, m_listen_fd, &maxFd);

    timeval timeout;
    timeout.tv_sec = 1; // TODO: move to rtremote.conf (rt.remote.server.select.timeout)
    timeout.tv_usec = 0;

    int ret = select(maxFd + 1, &readFds, NULL, &errFds, &timeout);
    if (ret == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    // right now we just use this to signal "hey" more fds added
    // later we'll use this to shutdown
    if (FD_ISSET(m_shutdown_pipe[0], &readFds))
    {
      rtLogInfo("got shutdown signal");
      return;
    }

    if (FD_ISSET(m_listen_fd, &readFds))
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
  sockaddr_storage remoteEndpoint;
  memset(&remoteEndpoint, 0, sizeof(remoteEndpoint));

  socklen_t len = sizeof(sockaddr_storage);

  int ret = accept(fd, reinterpret_cast<sockaddr *>(&remoteEndpoint), &len);
  if (ret == -1)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogWarn("error accepting new tcp connect. %s", rtStrError(e));
    return;
  }
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remoteEndpoint).c_str(), ret);

  sockaddr_storage localEndpoint;
  memset(&localEndpoint, 0, sizeof(sockaddr_storage));
  rtGetSockName(fd, localEndpoint);

  std::shared_ptr<rtRemoteClient> newClient(new rtRemoteClient(m_env, ret, localEndpoint, remoteEndpoint));
  newClient->setStateChangedHandler(&rtRemoteServer::onClientStateChanged_Dispatch, this);
  newClient->open();
  m_connected_clients.push_back(newClient);
}

rtError
rtRemoteServer::onClientStateChanged(std::shared_ptr<rtRemoteClient> const& client,
  rtRemoteClient::State state)
{
  rtError e = RT_ERROR_OBJECT_NOT_FOUND;

  if (state == rtRemoteClient::State::Shutdown)
  {
    rtLogInfo("client shutdown");
    std::unique_lock<std::mutex> lock(m_mutex);
    auto itr = std::remove_if(
      m_connected_clients.begin(),
      m_connected_clients.end(),
      [&client](std::shared_ptr<rtRemoteClient> const& c) { return c.get() == client.get(); }
    );
    if (itr != m_connected_clients.end())
    {
      rtLogInfo("removing reference to stream");
      m_connected_clients.erase(itr);
      e = RT_OK;
    }

    auto ditr = m_disconnected_callback_map.find(client.get());
    if (ditr != m_disconnected_callback_map.end())
    {
        std::vector<ClientDisconnectedCB>::const_iterator cbitr;
        for(cbitr = ditr->second.cbegin(); cbitr != ditr->second.cend(); ++cbitr)
        {
            if(cbitr->func)
                cbitr->func(cbitr->data);
        }
        ditr->second.clear();
        m_disconnected_callback_map.erase(ditr);
    }
  }

  return e;
}

rtError
rtRemoteServer::onIncomingMessage(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& msg)
{
  rtLogInfo("onIncomingMessage");

  rtError e = RT_OK;
  if (m_env->Config->server_use_dispatch_thread())
    m_env->enqueueWorkItem(client, msg);
  else
    e = processMessage(client, msg);
  return e;
}

rtError
rtRemoteServer::processMessage(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& msg)
{
  rtError e = RT_FAIL;
  char const* msgType = rtMessage_GetMessageType(*msg);

  auto itr = m_command_handlers.find(msgType);
  if (itr == m_command_handlers.end())
  {
    rtLogError("processMessage: no command handler for:%s. RT_ERROR_PROTOCOL_ERROR", msgType);
    return RT_ERROR_PROTOCOL_ERROR;
  }

  rtRemoteCallback<rtRemoteMessageHandler> handler = itr->second;
  if (handler.Func != nullptr)
    e = handler.Func(client, msg, handler.Arg);
  else
    e = RT_ERROR_INVALID_ARG;

  if (e != RT_OK)
    rtLogWarn("failed to run command for %s. %s", msgType, rtStrError(e));

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
rtRemoteServer::findObject(std::string const& objectId, rtObjectRef& obj, uint32_t timeout,
        clientDisconnectedCallback cb, void *cbdata)
{
  rtError err = RT_OK;
  obj = m_env->ObjectCache->findObject(objectId);

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    sockaddr_storage objectEndpoint;
    err = m_resolver->locateObject(objectId, objectEndpoint, timeout);

    rtLogDebug("object %s found at endpoint: %s", objectId.c_str(),
    rtSocketToString(objectEndpoint).c_str());

    if (err == RT_OK)
    {
      std::shared_ptr<rtRemoteClient> client;
      std::string const endpointName = rtSocketToString(objectEndpoint);

      std::unique_lock<std::mutex> lock(m_mutex);
      auto itr = m_object_map.find(endpointName);
      if (itr != m_object_map.end())
        client = itr->second;

      // if a client is already "hosting" this object, 
      // then just re-use it
      for (auto i : m_object_map)
      {
        if (sameEndpoint(i.second->getRemoteEndpoint(), objectEndpoint))
        {
          client = i.second;
          break;
        }
      }
      m_mutex.unlock();

      if (!client)
      {
        client.reset(new rtRemoteClient(m_env, objectEndpoint));
        client->setStateChangedHandler(&rtRemoteServer::onClientStateChanged_Dispatch, this);
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
        rtRemoteObject* remote(new rtRemoteObject(objectId, client));
        err = client->startSession(objectId);
        if (err == RT_OK)
          obj = remote;

        ClientDisconnectedCB CB = {cb, cbdata};
        auto ditr = m_disconnected_callback_map.find(client.get());
        if (ditr == m_disconnected_callback_map.end())
        {
            std::vector<ClientDisconnectedCB> new_cb_vector;
            new_cb_vector.push_back(CB);
            m_disconnected_callback_map.insert(ClientDisconnectedCBMap::value_type(client.get(), new_cb_vector));
        }
        else
        {
            auto cbitr = std::find_if(ditr->second.begin(), ditr->second.end(),
                    [CB](const ClientDisconnectedCB &cb) { return cb == CB; });

            if(cbitr == ditr->second.end())
                ditr->second.push_back(CB);
        }
      }
    }
  }

  return (obj ? RT_OK : RT_FAIL);
}

rtError
rtRemoteServer::unregisterDisconnectedCallback( clientDisconnectedCallback cb, void *cbdata )
{
    ClientDisconnectedCB CB = {cb, cbdata};

    auto findClient = [&]() -> ClientDisconnectedCBMap::iterator {
        for (auto &&client = m_disconnected_callback_map.begin();
             client != m_disconnected_callback_map.end();
             ++client) {
            const std::vector<ClientDisconnectedCB> &installed_callbacks = client->second;
            auto &&callback = find_if(installed_callbacks.cbegin(), installed_callbacks.cend(), [&](const ClientDisconnectedCB& c) {
                    return c == CB;
                });

            if (callback != installed_callbacks.end())
                return client;
        }
        return m_disconnected_callback_map.end();
    };

    std::unique_lock<std::mutex> lock(m_mutex);
    auto client = findClient();

    if (client == m_disconnected_callback_map.end()) {
        rtLogInfo("%p : %p client not found", cb, cbdata);
        return RT_ERROR_INVALID_ARG;
    }

    std::vector<ClientDisconnectedCB> &allClientCallbacks = client->second;
    allClientCallbacks.erase(
        std::remove_if(allClientCallbacks.begin(),
                       allClientCallbacks.end(),
                       [&] (const ClientDisconnectedCB& item) { return item == CB; }),
        allClientCallbacks.end()
        );
    rtLogInfo("%p : %p removed callback pair from the callbacks list", cb, cbdata);

    return RT_OK;
}

rtError
rtRemoteServer::openRpcListener()
{
  int ret = 0;
  char path[UNIX_PATH_MAX];

  memset(path, 0, sizeof(path));
  cleanupStaleUnixSockets();

  if (isUnixDomain(m_env))
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

    struct sockaddr_un *unAddr = reinterpret_cast<sockaddr_un*>(&m_rpc_endpoint);
    unAddr->sun_family = AF_UNIX;
    strncpy(unAddr->sun_path, path, UNIX_PATH_MAX);
  }
  else if (m_env->Config->server_listen_interface() != "lo")
  {
      rtError e = rtParseAddress(m_rpc_endpoint, m_env->Config->server_listen_interface().c_str(), 0, nullptr);
      if (e != RT_OK)
          rtGetDefaultInterface(m_rpc_endpoint, 0);
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
rtRemoteServer::onOpenSession(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& req)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*req);
  char const* objectId = rtMessage_GetObjectId(*req);

  #if 0
  std::unique_lock<std::mutex> lock(m_mutex);
  auto itr = m_objects.find(id);
  if (itr != m_objects.end())
  {
    sockaddr_storage const soc = client->getRemoteEndpoint();
    for (auto const& c : m_clients)
    {
      if (sameEndpoint(soc, c.peer))
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

  rtRemoteMessagePtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeOpenSessionResponse, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(objectId), res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key.toString(), res->GetAllocator());
  err = client->send(res);

  return err;
}

rtError
rtRemoteServer::onGet(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);
  char const* objectId = rtMessage_GetObjectId(*doc);

  rtRemoteMessagePtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key.toString(), res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(objectId), res->GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(objectId);
  if (!obj)
  {
    res->AddMember(kFieldNameStatusCode, 1, res->GetAllocator());
    res->AddMember(kFieldNameStatusMessage, std::string("object not found"), res->GetAllocator());
  }
  else
  {
    rtError err = RT_OK;
    rtValue value;

    uint32_t    index = 0;
    char const* name = rtMessage_GetPropertyName(*doc);

    if (name)
    {
      err = obj->Get(name, &value);
      if (err != RT_OK)
      {
        rtLogWarn("failed to get property: %s. %s", name, rtStrError(err));
      }
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
        val.AddMember(kFieldNameObjectId, std::string(objectId), res->GetAllocator());
        if (name)
        {
          rtFunctionRef ref = value.toFunction();
          rtRemoteFunction* remoteFunc = dynamic_cast<rtRemoteFunction *>(ref.getPtr());
          if (remoteFunc != nullptr)
            val.AddMember(kFieldNameFunctionName, remoteFunc->getName(), res->GetAllocator());
          else
            val.AddMember(kFieldNameFunctionName, std::string(name), res->GetAllocator());
        }
        else
        {
          val.AddMember(kFieldNameFunctionIndex, index, res->GetAllocator());
        }
        val.AddMember(kFieldNameValueType, static_cast<int>(RT_functionType), res->GetAllocator());
      }
      else
      {
        err = rtRemoteValueWriter::write(m_env, value, val, *res);
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

    err = client->send(res);
  }

  return RT_OK;
}

rtError
rtRemoteServer::onSet(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);
  char const* objectId = rtMessage_GetObjectId(*doc);

  rtRemoteMessagePtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key.toString(), res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(objectId), res->GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(objectId);
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
    RT_ASSERT(itr != doc->MemberEnd());

    if (itr != doc->MemberEnd())
      err = rtRemoteValueReader::read(m_env, value, itr->value, client);

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
    err = client->send(res);
  }
  return RT_OK;
}

rtError
rtRemoteServer::onMethodCall(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*doc);
  char const* objectId = rtMessage_GetObjectId(*doc);
  rtError err   = RT_OK;

  rtRemoteMessagePtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key.toString(), res->GetAllocator());

  rtObjectRef obj = m_env->ObjectCache->findObject(objectId);
  if (!obj && (strcmp(objectId, "global") != 0))
  {
    rtMessage_SetStatus(*res, 1, "failed to find object with id: %s", objectId);
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
          rtRemoteValueReader::read(m_env, arg, *itr, client);
          argv.push_back(arg);
        }
      }

      rtValue return_value;
      err = func->Send(static_cast<int>(argv.size()), &argv[0], &return_value);
      if (err == RT_OK)
      {
        rapidjson::Value val;
        rtRemoteValueWriter::write(m_env, return_value, val, *res);
        res->AddMember(kFieldNameFunctionReturn, val, res->GetAllocator());
      }

      rtMessage_SetStatus(*res, 0);
    }
    else
    {
      rtMessage_SetStatus(*res, 1, "object not found");
    }
  }

  err = client->send(res);
  if (err != RT_OK)
    rtLogWarn("failed to send response. %d", err);

  return RT_OK;
}

rtError
rtRemoteServer::onKeepAlive(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& req)
{
  rtRemoteCorrelationKey key = rtMessage_GetCorrelationKey(*req);

  auto itr = req->FindMember(kFieldNameKeepAliveIds);
  if (itr != req->MemberEnd())
  {
    auto now = std::chrono::steady_clock::now();

    std::unique_lock<std::mutex> lock(m_mutex);
    for (rapidjson::Value::ConstValueIterator id  = itr->value.Begin(); id != itr->value.End(); ++id)
    {
      rtError e = m_env->ObjectCache->touch(id->GetString(), now);
      if (e != RT_OK)
      {
        rtLogWarn("error updating last used time for: %s, %s", 
            id->GetString(), rtStrError(e));
      }
    }
  }
  else
  {
    rtLogWarn("got keep-alive without any interesting information");
  }

  rtRemoteMessagePtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameCorrelationKey, key.toString(), res->GetAllocator());
  res->AddMember(kFieldNameMessageType, kMessageTypeKeepAliveResponse, res->GetAllocator());
  return client->send(res);
}

rtError
rtRemoteServer::onKeepAliveResponse(std::shared_ptr<rtRemoteClient>& /*client*/, rtRemoteMessagePtr const& /*req*/)
{
    return RT_OK;
}

rtError
rtRemoteServer::removeStaleObjects()
{
  std::unique_lock<std::mutex> lock(m_mutex);
  for (auto itr = m_object_map.begin(); itr != m_object_map.end();)
  {
    // TODO: I'm not sure if this works. This is a map of connections to remote peers.
    // when connecting to mulitple remote objects, we'll re-use the remote connection.
    if (itr->second.use_count() == 1)
      itr = m_object_map.erase(itr);
    else
      ++itr;
  }
  lock.unlock();
  return m_env->ObjectCache->removeUnused(); // m_keep_alive_interval, num_removed);
}
