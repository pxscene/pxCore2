
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
  int set_src(char const* srcaddr, int16_t port);
  int set_dest(char const* dstaddr, int16_t port);
  int set_src_v4(char const* srcaddr, int16_t port);
  int set_src_v6(char const* srcaddr, int16_t port);
  void run_listener();
  void do_read();

private:
  typedef std::map< std::string, rtObjectRef > refmap_t;
  typedef std::vector<char> buff_t;

  sockaddr_storage  m_dest;
  sockaddr_storage  m_src;
  int               m_fd;
  int               m_family;
  refmap_t          m_objects;
  pthread_t         m_thread;
  bool              m_run;
  buff_t            m_buff;
  pthread_mutex_t   m_mutex;
  pid_t             m_pid;
};
