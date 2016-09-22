#ifndef __RT_REMOTE_STREAM_SELECTOR_H__
#define __RT_REMOTE_STREAM_SELECTOR_H__

#include "rtError.h"

#include <memory>
#include <mutex>
#include <vector>
#include <thread>


class rtRemoteStream;

class rtRemoteStreamSelector
{
public:
  rtRemoteStreamSelector();

  rtError start();
  rtError registerStream(std::shared_ptr<rtRemoteStream> const& s);
  rtError removeStream(std::shared_ptr<rtRemoteStream> const& s);
  rtError shutdown();

private:
  static void* pollFds(void* argp);
  rtError doPollFds();

private:
  std::vector< std::shared_ptr<rtRemoteStream> >  m_streams;
  pthread_t                                       m_thread;
  std::mutex                                      m_mutex;
  int                                             m_shutdown_pipe[2];
};

#endif
