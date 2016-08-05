#ifndef __RT_REMOTE_ASYNC_HANDLE__
#define __RT_REMOTE_ASYNC_HANDLE__

#include "rtRemoteMessage.h"
#include "rtRemoteCorrelationKey.h"

class rtRemoteEnvironment;

class rtRemoteAsyncHandle
{
  friend class rtRemoteStream;

public:
  ~rtRemoteAsyncHandle();

  rtJsonDocPtr response() const;
  rtError wait(uint32_t timeoutInMilliSeconds);

private:
  rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtRemoteCorrelationKey k);
  void complete(rtJsonDocPtr const& doc, rtError e);

  static rtError onResponseHandler_Dispatch(std::shared_ptr<rtRemoteClient>& client,
        rtJsonDocPtr const& msg, void* argp)
    { return reinterpret_cast<rtRemoteAsyncHandle *>(argp)->onResponseHandler(client, msg); }

  rtError onResponseHandler(std::shared_ptr<rtRemoteClient>& client,
    rtJsonDocPtr const& msg);

private:
  rtRemoteEnvironment*    m_env;
  rtRemoteCorrelationKey       m_key;
  rtJsonDocPtr            m_doc;
  rtError                 m_error;
};


#endif
