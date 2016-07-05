class rtRemoteNameService
{
public:
  rtRemoteNameService();
  ~rtRemoteNameService();

public:
  rtError init();
  rtError close();

private:
  using CommandHandler = rtError (rtRemoteMulticastResolver::*)(rtJsonDocPtr const&, sockaddr_storage const&);
  using CommandHandlerMap = std::map< std::string, CommandHandler >;
  using RequestMap = std::map< rtCorrelationKey, rtJsonDocPtr >;
  using RegisteredObjectsMap = std::map< std::string, sockaddr_storage >;

  rtError onRegister(rtJsonDocPtr const& doc, sockaddr_storage const& soc);
  rtError onDeregister(rtJsonDocPtr const& doc, sockaddr_storage const& soc);
  rtError onUpdate(rtJsonDocPtr const& doc, sockaddr_storage const& soc);
  rtError onLookup(rtJsonDocPtr const& doc, sockaddr_storage const& soc);

  void runListener();
  void doRead(int fd, rtSocketBuffer& buff);
  void doDispatch(char const* buff, int n, sockaddr_storage* peer);
  // rtError openDbConnection();
  rtError openNsSocket();

private:
  CommandHandlerMap    m_command_handlers;
  RequestMap           m_pending_requests;
  RegisteredObjectsMap m_registered_objects;

  sockaddr_storage  m_ns_endpoint;
  int               m_ns_fd;
  socklen_t         m_ns_len;

  std::mutex                   m_mutex;
  std::unique_ptr<std::thread> m_read_thread;
  int		                       m_shutdown_pipe[2];
  pid_t                        m_pid;
};
