#include "rtRpcServer.h"
#include "rtRemoteObject.h"
#include "rtObjectCache.h"
#include "rtSocketUtils.h"
#include "rtRpcMessage.h"
#include "rtRpcClient.h"
#include "rtValueReader.h"
#include "rtValueWriter.h"
#include "rtRpcConfig.h"

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

rtRpcServer::rtRpcServer()
  : m_listen_fd(-1)
  , m_resolver(nullptr)
  , m_pipe_write(-1)
  , m_pipe_read(-1)
  , m_keep_alive_interval(15)
{
  memset(&m_rpc_endpoint, 0, sizeof(m_rpc_endpoint));

  int arr[2];
  int ret = pipe(arr);
  if (ret == 0)
  {
    m_pipe_read = arr[0];
    m_pipe_write = arr[1];
  }

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeOpenSessionRequest,
    std::bind(&rtRpcServer::onOpenSession, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByNameRequest,
    std::bind(&rtRpcServer::onGet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeGetByIndexRequest,
    std::bind(&rtRpcServer::onGet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByNameRequest,
    std::bind(&rtRpcServer::onSet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeSetByIndexRequest,
    std::bind(&rtRpcServer::onSet, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeMethodCallRequest,
    std::bind(&rtRpcServer::onMethodCall, this, std::placeholders::_1, std::placeholders::_2)));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeKeepAliveRequest,
    std::bind(&rtRpcServer::onKeepAlive, this, std::placeholders::_1, std::placeholders::_2)));
}

rtRpcServer::~rtRpcServer()
{
  if (m_listen_fd != -1)
    ::close(m_listen_fd);
  if (m_pipe_read != -1)
    ::close(m_pipe_read);
  if (m_pipe_write != -1)
    ::close(m_pipe_write);

  if (m_resolver)
  {
    m_resolver->close();
    delete m_resolver;
  }
}

rtError
rtRpcServer::open()
{
  rtError err = openRpcListener();
  if (err != RT_OK)
    return err;

  m_resolver = rtRpcCreateResolver();
  err = start();
  if (err != RT_OK)
  {
    rtLogError("failed to start rpc server. %s", rtStrError(err));
    return err;
  }

  return err;
}

rtError
rtRpcServer::registerObject(std::string const& name, rtObjectRef const& obj)
{
  rtObjectRef ref = rtObjectCache::findObject(name);
  if (!ref)
    rtObjectCache::insert(name, obj, -1);
  m_resolver->registerObject(name, m_rpc_endpoint);
  return RT_OK;
}

void
rtRpcServer::runListener()
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
    rtPushFd(&read_fds, m_pipe_read, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_listen_fd, &maxFd);
    rtPushFd(&err_fds, m_pipe_read, &maxFd);

    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, &timeout);
    if (ret == -1)
    {
      rtLogWarn("select failed: %s", rtStrError(errno).c_str());
      continue;
    }

    // right now we just use this to signal "hey" more fds added
    // later we'll use this to shutdown
    if (FD_ISSET(m_pipe_read, &read_fds))
    {
      char ch;
      ssize_t nBytesRead = read(m_pipe_read, &ch, 1);
      if (nBytesRead != 1)
	rtLogError("failed to read from pipe. %d", errno);
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
rtRpcServer::doAccept(int fd)
{
  sockaddr_storage remote_endpoint;
  memset(&remote_endpoint, 0, sizeof(remote_endpoint));

  socklen_t len = sizeof(sockaddr_storage);

  int ret = accept(fd, reinterpret_cast<sockaddr *>(&remote_endpoint), &len);
  if (ret == -1)
  {
    rtLogWarn("error accepting new tcp connect. %s", rtStrError(errno).c_str());
    return;
  }
  rtLogInfo("new connection from %s with fd:%d", rtSocketToString(remote_endpoint).c_str(), ret);

  sockaddr_storage local_endpoint;
  memset(&local_endpoint, 0, sizeof(sockaddr_storage));
  rtGetSockName(fd, local_endpoint);

  std::shared_ptr<rtRpcClient> newClient(new rtRpcClient(ret, local_endpoint, remote_endpoint));
  newClient->setMessageCallback(std::bind(&rtRpcServer::onIncomingMessage, this, std::placeholders::_1, std::placeholders::_2));
  newClient->open();
  m_connected_clients.push_back(newClient);
}

rtError
rtRpcServer::onIncomingMessage(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& msg)
{
  char const* message_type = rtMessage_GetMessageType(*msg);

  auto itr = m_command_handlers.find(message_type);
  if (itr == m_command_handlers.end())
    return RT_OK;

  rtError err = itr->second(client, msg);
  if (err != RT_OK)
    rtLogWarn("failed to run command for %s. %s", message_type, rtStrError(err));

  return err;
}

rtError
rtRpcServer::start()
{
  rtError err = m_resolver->open(m_rpc_endpoint);
  if (err != RT_OK)
  {
    rtLogWarn("failed to open resolver. %s", rtStrError(err));
    return err;
  }

  try
  {
    m_thread.reset(new std::thread(&rtRpcServer::runListener, this));
  }
  catch (std::exception const& err)
  {
    rtLogError("failed to start listener thread. %s", err.what());
    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtRpcServer::findObject(std::string const& name, rtObjectRef& obj, uint32_t timeout)
{
  rtError err = RT_OK;
  obj = rtObjectCache::findObject(name);

  // if object is not registered with us locally, then check network
  if (!obj)
  {
    sockaddr_storage rpc_endpoint;
    err = m_resolver->locateObject(name, rpc_endpoint, timeout);

    rtLogDebug("object %s found at endpoint: %s", name.c_str(),
      rtSocketToString(rpc_endpoint).c_str());

    if (err == RT_OK)
    {
      std::shared_ptr<rtRpcClient> client;
      std::string const endpointName = rtSocketToString(rpc_endpoint);

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

      if (!client)
      {
        client.reset(new rtRpcClient(rpc_endpoint));
	client->setMessageCallback(std::bind(&rtRpcServer::onIncomingMessage, this, 
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
rtRpcServer::openRpcListener()
{
  int ret = 0;

  rtGetDefaultInterface(m_rpc_endpoint, 0);

  m_listen_fd = socket(m_rpc_endpoint.ss_family, SOCK_STREAM, 0);
  if (m_listen_fd < 0)
  {
    rtLogError("failed to create TCP socket. %s", rtStrError(errno).c_str());
  }

  fcntl(m_listen_fd, F_SETFD, fcntl(m_listen_fd, F_GETFD) | FD_CLOEXEC);


  socklen_t len;
  rtSocketGetLength(m_rpc_endpoint, &len);

  ret = ::bind(m_listen_fd, reinterpret_cast<sockaddr *>(&m_rpc_endpoint), len);
  if (ret < 0)
  {
    rtLogError("failed to bind socket. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  rtGetSockName(m_listen_fd, m_rpc_endpoint);
  rtLogInfo("local rpc listener on: %s", rtSocketToString(m_rpc_endpoint).c_str());

  ret = fcntl(m_listen_fd, F_SETFL, O_NONBLOCK);
  if (ret < 0)
  {
    rtLogError("fcntl: %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  ret = listen(m_listen_fd, 2);
  if (ret < 0)
  {
    rtLogError("failed to put socket in listen mode. %s", rtStrError(errno).c_str());
    return RT_FAIL;
  }

  return RT_OK;
}

rtError
rtRpcServer::onOpenSession(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& req)
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
rtRpcServer::onGet(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeGetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = rtObjectCache::findObject(id);
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
        err = rtValueWriter::write(value, val, *res);
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
rtRpcServer::onSet(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);

  rtJsonDocPtr res(new rapidjson::Document());
  res->SetObject();
  res->AddMember(kFieldNameMessageType, kMessageTypeSetByNameResponse, res->GetAllocator());
  res->AddMember(kFieldNameCorrelationKey, key, res->GetAllocator());
  res->AddMember(kFieldNameObjectId, std::string(id), res->GetAllocator());

  rtObjectRef obj = rtObjectCache::findObject(id);
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
rtRpcServer::onMethodCall(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& doc)
{
  uint32_t key = rtMessage_GetCorrelationKey(*doc);
  char const* id = rtMessage_GetObjectId(*doc);
  rtError err   = RT_OK;

  rapidjson::Document res;
  res.SetObject();
  res.AddMember(kFieldNameMessageType, kMessageTypeMethodCallResponse, res.GetAllocator());
  res.AddMember(kFieldNameCorrelationKey, key, res.GetAllocator());

  rtObjectRef obj = rtObjectCache::findObject(id);
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
      func = rtObjectCache::findFunction(function_name->value.GetString());
    }

    if (err == RT_OK)
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
	rtValueWriter::write(return_value, val, res);
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
rtRpcServer::onKeepAlive(std::shared_ptr<rtRpcClient>& client, rtJsonDocPtr const& req)
{
  uint32_t key = rtMessage_GetCorrelationKey(*req);

  auto itr = req->FindMember(kFieldNameKeepAliveIds);
  if (itr != req->MemberEnd())
  {
    time_t now = time(nullptr);

    std::unique_lock<std::mutex> lock(m_mutex);
    for (rapidjson::Value::ConstValueIterator id  = itr->value.Begin(); id != itr->value.End(); ++id)
    {
      rtError e = rtObjectCache::touch(id->GetString(), now);
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
rtRpcServer::removeStaleObjects()
{
  return rtObjectCache::removeUnused(); // m_keep_alive_interval, num_removed);
}
