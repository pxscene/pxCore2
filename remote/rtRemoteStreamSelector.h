#ifndef __RT_REMOTE_STREAM_SELECTOR_H__
#define __RT_REMOTE_STREAM_SELECTOR_H__

#include "rtError.h"

#include <memory>
#include <mutex>
#include <vector>
#include <thread>
#include <condition_variable>

class rtRemoteStream;
class rtRemoteEnvironment;

class rtRemoteStreamSelector
{
public:
  rtRemoteStreamSelector(rtRemoteEnvironment* env);

  rtError start();
  rtError registerStream(std::shared_ptr<rtRemoteStream> const& s);
  rtError shutdown();

private:
  static void* pollFds(void* argp);
  rtError doPollFds();

private:
  // TODO: should this be std::weak_ptr
  std::vector< std::shared_ptr<rtRemoteStream> >  m_streams;
  pthread_t                                       m_thread;
  std::mutex                                      m_mutex;
  std::condition_variable                         m_streams_cond;
  int                                             m_shutdown_pipe[2];
  rtRemoteEnvironment*                            m_env;
  bool                                            m_running;
};

#endif
