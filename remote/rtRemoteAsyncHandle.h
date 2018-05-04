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

#ifndef __RT_REMOTE_ASYNC_HANDLE__
#define __RT_REMOTE_ASYNC_HANDLE__

#include <functional>

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
  rtError waitUntil(uint32_t timeoutInMilliSeconds, std::function<rtError()> connectionState);

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
