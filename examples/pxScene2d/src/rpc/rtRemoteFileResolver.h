#include "rtRemoteResolver.h"
#include <condition_variable>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include <stdint.h>
#include <netinet/in.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#include "rtRemoteTypes.h"
#include "rtRemoteResolver.h"
#include "rtSocketUtils.h"


class rtRemoteFileResolver : public rtRemoteIResolver
{
public:
  rtRemoteFileResolver();
  ~rtRemoteFileResolver();

public:
  virtual rtError open(sockaddr_storage const& rpc_endpoint) override;
  virtual rtError close() override;
  virtual rtError registerObject(std::string const& name, sockaddr_storage const& endpoint) override;
  virtual rtError locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t timeout) override;

private:
  std::string       m_rpc_addr;
  uint16_t          m_rpc_port;
  FILE*             m_db_fp; 
};