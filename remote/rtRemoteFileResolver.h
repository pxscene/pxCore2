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

#include "rtRemoteIResolver.h"

#include <string>
#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>

#include "rtRemoteTypes.h"
#include "rtRemoteIResolver.h"
#include "rtRemoteEndpoint.h"
#include "rtRemoteSocketUtils.h"

class rtRemoteEnvironment;


class rtRemoteFileResolver : public rtRemoteIResolver
{
public:
  rtRemoteFileResolver(rtRemoteEnvironment* env);
  ~rtRemoteFileResolver();

public:
  virtual rtError open(sockaddr_storage const& rpc_endpoint) override;
  virtual rtError close() override;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) override;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t timeout) override;
  virtual rtError unregisterObject(std::string const& name) override;

private:
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  FILE*             m_db_fp;
  rtRemoteEnvironment* m_env;
};
