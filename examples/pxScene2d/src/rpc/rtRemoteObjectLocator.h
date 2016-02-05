
#include <pthread.h>
#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <map>
#include <string>

class rtRemoteTransport;

class rtRemoteObjectLocator
{
public:
  rtRemoteObjectLocator();
  ~rtRemoteObjectLocator();

public:
  rtError open(char const* dstaddr, int16_t dstport, char const* srcaddr);
  rtError registerObject(std::string const& name, rtObjectRef const& obj);
  rtError startListener();
  rtObjectRef findObject(std::string const& name);

private:
  static void* run_listener(void* argp);

private:
  void run_listener();
  void do_read(int fd);

  rtError open_unicast_socket();
  rtError open_multicast_socket();

private:
  typedef std::map< std::string, rtObjectRef > refmap_t;
  typedef std::vector<char> buff_t;

  sockaddr_storage  m_mcast_dest;
  sockaddr_storage  m_mcast_src;
  int               m_mcast_fd;
  int               m_mcast_family;

  sockaddr_storage  m_ucast_endpoint;
  int               m_ucast_fd;
  int               m_ucast_family;

  refmap_t          m_objects;
  pthread_t         m_read_thread;
  buff_t            m_read_buff;
  bool              m_read_run;

  pthread_mutex_t   m_mutex;
  pid_t             m_pid;
};
