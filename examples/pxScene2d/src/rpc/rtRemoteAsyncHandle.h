#ifndef __RT_REMOTE_ASYNC_HANDLE__
#define __RT_REMOTE_ASYNC_HANDLE__

#include "rtRemoteCorrelationKey.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteMessage.h"

class rtRemoteClient;

class rtRemoteAsyncHandle
{
  friend class rtRemoteStream;

public:
  ~rtRemoteAsyncHandle();

  rtRemoteMessagePtr response() const;
  rtError wait(uint32_t timeoutInMilliSeconds);

private:
  rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtRemoteCorrelationKey k);
  void complete(rtRemoteMessagePtr const& doc, rtError e);

  static rtError onResponseHandler_Dispatch(std::shared_ptr<rtRemoteClient>& client,
        rtRemoteMessagePtr const& msg, void* argp)
    { return reinterpret_cast<rtRemoteAsyncHandle *>(argp)->onResponseHandler(client, msg); }

  rtError onResponseHandler(std::shared_ptr<rtRemoteClient>& client,
    rtRemoteMessagePtr const& msg);

private:
  rtRemoteEnvironment*    m_env;
  rtRemoteCorrelationKey       m_key;
  rtRemoteMessagePtr            m_doc;
  rtError                 m_error;
};


#endif
