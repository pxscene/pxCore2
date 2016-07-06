rtRemoteNameService::rtRemoteNameService()
{
  : m_ns_fd(-1)
  , m_ns_len(0)
  , m_command_handlers()
  , m_pid(getpid())
{
  memset(&m_ns_endpoint, 0, sizeof(m_ns_endpoint));

  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeNsRegister, &rtRemoteNameService::onRegister));
  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeNsDeregister, &rtRemoteNameService::onDeregister));
  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeNsUpdate, &rtRemoteNameService::onUpdate));
  m_command_handlers.insert(CommandHandlerMap::value_type(kMessageTypeNsLookup, &rtRemoteNameService::onLookup));

  m_shutdown_pipe[0] = -1;
  m_shutdown_pipe[1] = -1;

  int ret = pipe2(m_shutdown_pipe, O_CLOEXEC);
  if (ret == -1)
  {
    rtError e = rtErrorFromErrno(ret);
    rtLogWarn("failed to create shutdown pipe. %s", rtStrError(e));
  }
}

rtRemoteNameService::~rtRemoteNameService()
{
// TODO
}

rtError
rtRemoteNameService::init()
{
  rtError err = RT_OK;

  // TODO eventually, use a db rather than map.  Open it here.

  // get socket info ready
  char const* srcaddr = rtRemoteSetting<char const *>("rt.rpc.resolver.multicast_interface");
  err = rtParseAddress(m_ns_endpoint, srcaddr, 0, nullptr);
  if (err != RT_OK)
  {
    err = rtGetDefaultInterface(m_ns_endpoint, 0);
    if (err != RT_OK)
      return err;
  }

  // open unicast socket
  err = openNsSocket();
  if (err != RT_OK)
  {
      rtLogWarn("failed to open name service unicast socket. %s", rtStrError(err));
      return err;
  }
}

rtError
rtRemoteNameService::close()
{
// TODO
}

// rtError 
// rtRemoteNameService::openDbConnection(){}

rtError
rtRemoteNameService::openNsSocket()
{
  int ret = 0;

  m_ns_fd = socket(m_ns_endpoint.ss_family, SOCK_DGRAM, 0);
  if (m_ns_fd < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to create unicast socket with family: %d. %s", 
      m_ns_endpoint.ss_family, rtStrError(e));
    return e;
  }
  fcntl(m_ns_fd, F_SETFD, fcntl(m_ns_fd, F_GETFD) | FD_CLOEXEC);

  rtSocketGetLength(m_ns_endpoint, &m_ns_len);

  // listen on ANY port
  ret = bind(m_ns_fd, reinterpret_cast<sockaddr *>(&m_ns_endpoint), m_ns_len);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to bind unicast endpoint: %s", rtStrError(e));
    return e;
  }

  // now figure out which port we're bound to
  rtSocketGetLength(m_ns_endpoint, &m_ns_len);
  ret = getsockname(m_ns_fd, reinterpret_cast<sockaddr *>(&m_ns_endpoint), &m_ns_len);
  if (ret < 0)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("failed to get socketname. %s", rtStrError(e));
    return e;
  }
  else
  {
    rtLogInfo("local udp socket bound to %s", rtSocketToString(m_ns_endpoint).c_str());
  }

  return RT_OK;
}

/**
 * Callback for registering objects and associated Well-known Endpoints
 */
rtError
rtRemoteNameService::onRegister(rtJsonDocPtr const& doc, sockaddr_storage const& soc)
{
  assert(doc->HasMember(kFieldNameIp));
  assert(doc->HasMember(kFieldNamePort));
  
  sockaddr_storage endpoint;
  rtError err = rtParseAddress(endpoint, (*doc)[kFieldNameIp].GetString(),
                (*doc)[kFieldNamePort].GetInt(), nullptr);

  if (err != RT_OK)
    return err;
  
  std::unique_lock<std::mutex> lock(m_mutex);
  m_registered_objects[name] = endpoint;
  return RT_OK;
}

/**
 * Callback for deregistering objects
 */
rtError
rtRemoteNameService::onDeregister(rtJsonDocPtr const& doc, sockaddr_storage const& soc)
{
  // TODO
  return RT_OK;
}

rtError
rtRemoteNameService::onUpdate(rtJsonDocPtr const& doc, sockaddr_storage const& soc)
{
  // TODO
  return RT_OK;
}

rtError
rtRemoteNameService::onLookup(rtJsonDocPtr const& doc, sockaddr_storage const& soc)
{
  auto senderId = doc->FindMember(kFieldNameSenderId);
  assert(senderId != doc->MemberEnd());
  if (senderId->value.GetInt() == m_pid)
  {
    return RT_OK;
  }

  int key = rtMessage_GetCorrelationKey(*doc);

  auto itr = m_hosted_objects.end();

  char const* objectId = rtMessage_GetObjectId(*doc);

  std::unique_lock<std::mutex> lock(m_mutex);
  itr = m_registered_objects.find(objectId);
  lock.unlock();

  if (itr != m_registered_objects.end())
  { // object is registered

    // get IP and port
    sockaddr_storage endpoint = itr->second;
    std::string       ep_addr;
    uint16_t          ep_port;
    char buff[128];

    void* addr = nullptr;
    rtGetInetAddr(endpoint, &addr);

    socklen_t len;
    rtSocketGetLength(endpoint, &len);
    char const* p = inet_ntop(endpoint.ss_family, addr, buff, len);
    if (p)
      ep_addr = p;
    rtGetPort(endpoint, &ep_port);

    // create and send response
    rapidjson::Document doc;
    doc.SetObject();
    doc.AddMember(kFieldNameMessageType, kMessageTypeLocate, doc.GetAllocator());
    doc.AddMember(kFieldNameObjectId, std::string(objectId), doc.GetAllocator());
    doc.AddMember(kFieldNameIp, ep_addr, doc.GetAllocator());
    doc.AddMember(kFieldNamePort, ep_port, doc.GetAllocator());
    doc.AddMember(kFieldNameSenderId, senderId->value.GetInt(), doc.GetAllocator());
    doc.AddMember(kFieldNameCorrelationKey, key, doc.GetAllocator());

    return rtSendDocument(doc, m_ns_fd, &soc);
  }
}

void
rtRemoteNameService::runListener()
{
  rtSocketBuffer buff;
  buff.reserve(1024 * 1024);
  buff.resize(1024 * 1024);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_ns_fd, &maxFd);
    rtPushFd(&read_fds, m_shutdown_pipe[0], &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_ns_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      rtError e = rtErrorFromErrno(errno);
      rtLogWarn("select failed: %s", rtStrError(e));
      continue;
    }

    if (FD_ISSET(m_shutdown_pipe[0], &read_fds))
    {
      rtLogInfo("got shutdown signal");
      return;
    }

    if (FD_ISSET(m_ns_fd, &read_fds))
      doRead(m_ns_fd, buff);
  }
}

void
rtRemoteNameService::doRead(int fd, rtSocketBuffer& buff)
{
  // we only suppor v4 right now. not sure how recvfrom supports v6 and v4
  sockaddr_storage src;
  socklen_t len = sizeof(sockaddr_in);

  #if 0
  ssize_t n = read(m_mcast_fd, &m_read_buff[0], m_read_buff.capacity());
  #endif

  ssize_t n = recvfrom(fd, &buff[0], buff.capacity(), 0, reinterpret_cast<sockaddr *>(&src), &len);
  if (n > 0)
    doDispatch(&buff[0], static_cast<int>(n), &src);
}

void
rtRemoteNameService::doDispatch(char const* buff, int n, sockaddr_storage* peer)
{
  // rtLogInfo("new message from %s:%d", inet_ntoa(src.sin_addr), htons(src.sin_port));
  // printf("read: %d\n", int(n));
  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n***IN***\t\"%.*s\"\n", n, buff); // static_cast<int>(m_read_buff.size()), &m_read_buff[0]);
  #endif

  rtJsonDocPtr doc;
  rtError err = rtParseMessage(buff, n, doc);
  if (err != RT_OK)
    return;

  char const* message_type = rtMessage_GetMessageType(*doc);

  auto itr = m_command_handlers.find(message_type);
  if (itr == m_command_handlers.end())
  {
    rtLogWarn("no command handler registered for: %s", message_type);
    return;
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  err = CALL_MEMBER_FN(*this, itr->second)(doc, *peer);
  if (err != RT_OK)
  {
    rtLogWarn("failed to run command for %s. %d", message_type, err);
    return;
  }
}