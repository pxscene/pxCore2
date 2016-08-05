#include "rtRemoteAsyncHandle.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteTypes.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"

rtRemoteAsyncHandle::rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtRemoteCorrelationKey k)
  : m_env(env)
  , m_key(k)
  , m_error(RT_ERROR_IN_PROGRESS)
{
  m_env->registerResponseHandler(&rtRemoteAsyncHandle::onResponseHandler_Dispatch,
    this, m_key);
}

rtRemoteAsyncHandle::~rtRemoteAsyncHandle()
{
  if (m_key != kInvalidCorrelationKey)
    m_env->removeResponseHandler(m_key);
}

rtError
rtRemoteAsyncHandle::onResponseHandler(std::shared_ptr<rtRemoteClient>& /*client*/,
  rtJsonDocPtr const& doc)
{
  m_doc = doc;
  return RT_OK;
}

rtError
rtRemoteAsyncHandle::wait(uint32_t timeoutInMilliseconds)
{
  if (m_error != RT_ERROR_IN_PROGRESS)
    return m_error;

  if (timeoutInMilliseconds == 0)
    timeoutInMilliseconds = m_env->Config->environment_request_timeout();

  if (!m_env->Config->server_use_dispatch_thread())
  {
    time_t timeout = time(nullptr) + ((timeoutInMilliseconds+500) / 1000);

    rtRemoteCorrelationKey k = kInvalidCorrelationKey;
    while (timeout > time(nullptr))
    {
      // timeout is broken @see impl of processSingleWorkItem
      rtError e = m_env->processSingleWorkItem(
          std::chrono::milliseconds(timeoutInMilliseconds), &k);

      if ((e == RT_OK) && (k == m_key))
      {
        m_env->removeResponseHandler(m_key);
        m_key = kInvalidCorrelationKey;
        return RT_OK;
      }
    }
  }

  return m_env->waitForResponse(std::chrono::milliseconds(timeoutInMilliseconds), m_key);
}


void
rtRemoteAsyncHandle::complete(rtJsonDocPtr const& doc, rtError e)
{
  m_doc = doc;
  m_error = e;
}

rtJsonDocPtr
rtRemoteAsyncHandle::response() const
{
  return m_doc;
}
