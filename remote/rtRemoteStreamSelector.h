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
