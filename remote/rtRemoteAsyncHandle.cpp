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

#include "rtRemoteAsyncHandle.h"
#include "rtRemoteEnvironment.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"

#include <chrono>

rtRemoteAsyncHandle::rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtRemoteCorrelationKey k)
  : m_env(env)
  , m_key(k)
  , m_error(RT_ERROR_IN_PROGRESS)
{
  RT_ASSERT(m_key != rtGuid::null());
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
  rtRemoteMessagePtr const& doc)
{
  complete(doc, RT_OK);
  return RT_OK;
}

rtError
rtRemoteAsyncHandle::waitUntil(uint32_t timeoutInMilliseconds, std::function<rtError()> connectionState)
{
  if (m_error != RT_ERROR_IN_PROGRESS)
    return m_error;

  if (timeoutInMilliseconds == 0)
    timeoutInMilliseconds = m_env->Config->environment_request_timeout();

  rtError e = RT_OK;

  auto nowTime = std::chrono::steady_clock::now();
  auto stopTime = nowTime + std::chrono::milliseconds(timeoutInMilliseconds);

  for (; stopTime > nowTime; nowTime = std::chrono::steady_clock::now())
  {
    if ((e = connectionState()) != RT_OK)
    {
      break;
    }

    auto remainingDuration =
      std::chrono::duration_cast<std::chrono::milliseconds>(stopTime - nowTime);

    // Wake up at least once per second to check connection state
    auto waitDuration =
      std::min(std::max(std::chrono::milliseconds(1), remainingDuration),
               std::chrono::milliseconds(1000u));

    if (!m_env->Config->server_use_dispatch_thread())
    {
      rtRemoteCorrelationKey k = kInvalidCorrelationKey;
      rtLogDebug("Waiting for item with key = %s", m_key.toString().c_str());

      m_error = RT_ERROR_TIMEOUT;
      e = m_env->processSingleWorkItem(waitDuration, true, &k);

      if ( e == RT_ERROR_TIMEOUT )
      {
        continue;
      }

      rtLogDebug("Got response with key = %s, m_error = %d, error = %d", k.toString().c_str(), m_error, e);

      if ( (e == RT_OK) && ((m_error == RT_OK) || (k == m_key)) )
      {
        rtLogDebug("Got successful response: m_key = %s, key = %s, m_error = %d",
                m_key.toString().c_str(), k.toString().c_str(), m_error);
        m_env->removeResponseHandler(m_key);
        m_key = kInvalidCorrelationKey;
        e = m_error = RT_OK;
        break;
      }
    }
    else
    {
      e = m_env->waitForResponse(waitDuration, m_key);

      if ( e != RT_ERROR_TIMEOUT )
      {
        break;
      }
    }
  }

  if ((e != RT_OK) || (m_error != RT_OK))
  {
    rtLogError("Got error response m_error = %d (%s), error = %d (%s)", m_error, rtStrError(m_error), e, rtStrError(e));
  }

  return e;
}


void
rtRemoteAsyncHandle::complete(rtRemoteMessagePtr const& doc, rtError e)
{
  m_doc = doc;
  m_error = e;
}

rtRemoteMessagePtr
rtRemoteAsyncHandle::response() const
{
  return m_doc;
}
