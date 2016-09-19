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


class rtRemoteClient: public std::enable_shared_from_this<rtRemoteClient>
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

  void keepAlive(std::string const& s);
  rtError setStateChangedHandler(StateChangedHandler handler, void* argp);

  void removeKeepAlive(std::string const& s);

  inline rtRemoteEnvironment* getEnvironment() const
    { return m_env; }

  rtError send(rtRemoteMessagePtr const& msg);

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_stream->getRemoteEndpoint(); }

  inline sockaddr_storage getLocalEndpoint() const
    { return m_stream->getLocalEndpoint(); }

private:
  rtError sendGet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& value);
  rtError sendSet(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k);
  rtError sendCall(rtRemoteMessagePtr const& req, rtRemoteCorrelationKey k, rtValue& result); 

  static rtError onIncomingMessage_Dispatcher(rtRemoteMessagePtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onIncomingMessage(doc); }

  static rtError onInactivity_Dispatcher(time_t lastMessage, time_t now, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onInactivity(lastMessage, now); }

  static rtError onStreamStateChanged_Dispatcher(std::shared_ptr<rtRemoteStream> const& stream,
      rtRemoteStream::State state, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onStreamStateChanged(stream, state); }

  rtError onIncomingMessage(rtRemoteMessagePtr const& msg);
  rtError onInactivity(time_t lastMessage, time_t now);
  rtError onStreamStateChanged(std::shared_ptr<rtRemoteStream> const& stream, rtRemoteStream::State state);
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
    std::unique_lock<std::mutex> lock(m_mutex);
    if (m_stream)
      s = m_stream;
    return s;
  }

  std::shared_ptr<rtRemoteStream>           m_stream;
  std::vector<std::string>                  m_object_list;
  std::mutex                                m_mutex;
  rtRemoteEnvironment*                      m_env;
  rtRemoteCallback<StateChangedHandler>     m_state_changed_handler;
};

#endif
