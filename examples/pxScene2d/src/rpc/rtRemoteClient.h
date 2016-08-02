#ifndef __RT_RPC_TRANSPORT_H__
#define __RT_RPC_TRANSPORT_H__

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
#include <rapidjson/document.h>

#include "rtRemoteStream.h"
#include "rtRemoteTypes.h"
#include "rtRemoteMessage.h"
#include "rtSocketUtils.h"

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

  rtRemoteClient(rtRemoteEnvironment* env, int fd, sockaddr_storage const& local_endpoint,
    sockaddr_storage const& remote_endpoint);
  rtRemoteClient(rtRemoteEnvironment* env, sockaddr_storage const& remote_endpoint);
  ~rtRemoteClient();

  rtError open();
  rtError startSession(std::string const& objectName, uint32_t timeout = 0);

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

  rtError send(rtJsonDocPtr const& msg);

  inline sockaddr_storage getRemoteEndpoint() const
    { return m_stream->getRemoteEndpoint(); }

  inline sockaddr_storage getLocalEndpoint() const
    { return m_stream->getLocalEndpoint(); }

private:
  rtError sendGet(rtJsonDocPtr const& req, rtCorrelationKey k, rtValue& value);
  rtError sendSet(rtJsonDocPtr const& req, rtCorrelationKey k);
  rtError sendCall(rtJsonDocPtr const& req, rtCorrelationKey k, rtValue& result); 

  static rtError onIncomingMessage_Dispatcher(rtJsonDocPtr const& doc, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onIncomingMessage(doc); }

  static rtError onInactivity_Dispatcher(time_t lastMessage, time_t now, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onInactivity(lastMessage, now); }

  static rtError onStreamStateChanged_Dispatcher(std::shared_ptr<rtRemoteStream> const& stream,
      rtRemoteStream::State state, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onStreamStateChanged(stream, state); }

  rtError onIncomingMessage(rtJsonDocPtr const& msg);
  rtError onInactivity(time_t lastMessage, time_t now);
  rtError onStreamStateChanged(std::shared_ptr<rtRemoteStream> const& stream, rtRemoteStream::State state);
  rtError connectRpcEndpoint();
  rtError sendKeepAlive();

  static rtError onSynchronousResponse_Handler(std::shared_ptr<rtRemoteClient>& client,
        rtJsonDocPtr const& msg, void* argp)
    { return reinterpret_cast<rtRemoteClient *>(argp)->onSynchronousResponse(client, msg); }

  rtError onStartSession(rtJsonDocPtr const& doc);
  rtError waitForResponse(rtCorrelationKey k, rtJsonDocPtr& res, int timeout);
  rtError onSynchronousResponse(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& doc);
  // rtError sendSynchronousRequest(rtJsonDocPtr const& req, rtJsonDocPtr& res, int timeout);
  // bool moreToProcess(rtCorrelationKey k);

private:
  std::shared_ptr<rtRemoteStream>   m_stream;
  std::vector<std::string>          m_object_list;
  std::mutex                        m_mutex;
  rtRemoteEnvironment*              m_env;
  rtRemoteCallback<StateChangedHandler>     m_state_changed_handler;
};

#endif
