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

#ifndef __RT_RPC_STREAM_H__
#define __RT_RPC_STREAM_H__

#include "rtRemoteTypes.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteAsyncHandle.h"
#include "rtRemoteCallback.h"
#include "Hub.h"

#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <thread>

class rtRemoteStreamSelector;

class rtRemoteStream : public std::enable_shared_from_this<rtRemoteStream>
{
  friend class rtRemoteStreamSelector;

public:
  enum class State
  {
    Opened,
    Inactive,
    Closed
  };

  class CallbackHandler {
  public:
    virtual ~CallbackHandler() { }
    virtual rtError onMessage(rtRemoteMessagePtr const& doc) = 0;
    virtual rtError onStateChanged(std::shared_ptr<rtRemoteStream> const& stream, State state) = 0;
  };

  rtRemoteStream(rtRemoteEnvironment* env, int fd,
    sockaddr_storage const& local_endpoint, sockaddr_storage const& remote_endpoint);

  rtRemoteStream(rtRemoteEnvironment* env, uWS::WebSocket<uWS::SERVER>* ws);
  ~rtRemoteStream();

  rtRemoteStream(rtRemoteStream const&) = delete;
  rtRemoteStream& operator = (rtRemoteStream const&) = delete;

  inline bool isConnected() const
    { return m_fd != -1; }

  rtError open();
  rtError close();
  rtError connect();
  rtError connectTo(sockaddr_storage const& endpoint);
  rtError send(rtRemoteMessagePtr const& msg);
  rtRemoteAsyncHandle sendWithWait(rtRemoteMessagePtr const& msg, rtRemoteCorrelationKey k);

  rtError setCallbackHandler(std::shared_ptr<CallbackHandler> const& callbackHandler);

  inline bool isOpen() const
    { return m_fd != kInvalidSocket; }

  inline sockaddr_storage getLocalEndpoint() const
    { return m_local_endpoint; }

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_remote_endpoint; }

private:
  rtError onIncomingMessage(rtRemoteSocketBuffer& buff);
  rtError onInactivity();

private:
  int                                   m_fd;
  std::weak_ptr<CallbackHandler>        m_callback_handler;
  sockaddr_storage                      m_local_endpoint;
  sockaddr_storage                      m_remote_endpoint;
  uWS::WebSocket<uWS::SERVER>*          m_ws;
  rtRemoteEnvironment*                  m_env;
};

#endif
