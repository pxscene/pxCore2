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

#ifndef __RT_REMOTE_CLIENT_H__
#define __RT_REMOTE_CLIENT_H__

#include <atomic>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <rtError.h>
#include <rtValue.h>

#include <sys/socket.h>

#include "rtRemoteCorrelationKey.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteMessage.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteStream.h"
#include "Hub.h"


class rtRemoteClient
  : public std::enable_shared_from_this<rtRemoteClient>
  , public rtRemoteStream::CallbackHandler
{
public:
  enum class State
  {
    Started,
    Shutdown
  };

  using StateChangedHandler = rtError (*)(std::shared_ptr<rtRemoteClient> const& client,
    State state, void* argp);

  rtRemoteClient(rtRemoteEnvironment* env, int fd, sockaddr_storage const& localEndpoint,
    sockaddr_storage const& remoteEndpoint);
  rtRemoteClient(rtRemoteEnvironment* env, uWS::WebSocket<uWS::SERVER>* ws);
  rtRemoteClient(rtRemoteEnvironment* env, sockaddr_storage const& remoteEndpoint);
  ~rtRemoteClient();

  rtError open();
  rtError startSession(std::string const& objectId, uint32_t timeout = 0);

  rtError sendSet(std::string const& objectId, uint32_t    propertyIdx , rtValue const& value);
  rtError sendSet(std::string const& objectId, char const* propertyName, rtValue const& value);
  rtError sendGet(std::string const& objectId, uint32_t    propertyIdx,  rtValue& result);
  rtError sendGet(std::string const& objectId, char const* propertyName, rtValue& result);
  rtError sendCall(std::string const& objectId, std::string const& methodName,
    int argc, rtValue const* argv, rtValue& result);

  void registerKeepAliveForObject(std::string const& s);
  rtError setStateChangedHandler(StateChangedHandler handler, void* argp);

  void removeKeepAliveForObject(std::string const& s);

  inline rtRemoteEnvironment* getEnvironment() const
    { return m_env; }

  rtError send(rtRemoteMessagePtr const& msg);

  sockaddr_storage getRemoteEndpoint() const;
  sockaddr_storage getLocalEndpoint() const;

  rtError onWebSocketMessage(char* buffer, size_t bufferLen);

private:
  rtError sendGet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& value);
  rtError sendSet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k);
  rtError sendCall(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& result); 

  // from rtRemoteStream::CallbackHandler
  virtual rtError onMessage(rtRemoteMessagePtr const& msg);
  virtual rtError onStateChanged(std::shared_ptr<rtRemoteStream> const& stream, rtRemoteStream::State state);

  rtError connectRpcEndpoint();
  rtError sendKeepAlive();

  static rtError onSynchronousResponse_Handler(std::shared_ptr<rtRemoteClient>& client,
        rtRemoteMessagePtr const& msg, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onSynchronousResponse(client, msg); }

  rtError onStartSession(rtRemoteMessagePtr const& doc);
  rtError waitForResponse(rtRemoteCorrelationKey k, rtRemoteMessagePtr& res, int timeout);
  rtError onSynchronousResponse(std::shared_ptr<rtRemoteClient>& client, rtRemoteMessagePtr const& doc);
  // rtError sendSynchronousRequest(rtRemoteMessagePtr const& req, rtRemoteMessagePtr& res, int timeout);
  // bool moreToProcess(rtRemoteCorrelationKey k);

private:
  inline std::shared_ptr<rtRemoteStream> getStream()
  {
    std::shared_ptr<rtRemoteStream> s;
    std::unique_lock<std::recursive_mutex> lock(m_mutex);
    if (m_stream)
      s = m_stream;
    return s;
  }
  rtError checkStream();

  std::shared_ptr<rtRemoteStream>           m_stream;
  std::vector<std::string>                  m_objects;
  std::recursive_mutex mutable              m_mutex;
  rtRemoteEnvironment*                      m_env;
  uWS::WebSocket<uWS::SERVER>*              m_ws_instance;
  rtRemoteCallback<StateChangedHandler>     m_state_changed_handler;
};

#endif
