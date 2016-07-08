#include "rtRemoteFileResolver.h"
#include "rtSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"

#include <condition_variable>
#include <thread>
#include <mutex>

#include <rtLog.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <ifaddrs.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <rapidjson/document.h>
#include <rapidjson/memorystream.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <rapidjson/pointer.h>


rtRemoteFileResolver::rtRemoteFileResolver(rtRemoteEnvPtr env)
: m_db_fp(NULL)
, m_env(env)
{}

rtRemoteFileResolver::~rtRemoteFileResolver()
{
  rtError err = this->close();
  if (err != RT_OK)
    rtLogWarn("failed to close resolver: %s", rtStrError(err));
}

rtError
rtRemoteFileResolver::open(sockaddr_storage const& rpc_endpoint)
{
  const char * const dbPath = m_env->Config->getString("rt.rpc.resolver.file_path");
  m_db_fp = fopen(dbPath, "r+");
  if (m_db_fp == NULL)
  {
    rtLogError("could not connect to database");
    return RT_FAIL;
  }

  char buff[128];
  void* addr = nullptr;
  rtGetInetAddr(rpc_endpoint, &addr);

  socklen_t len;
  rtSocketGetLength(rpc_endpoint, &len);
  char const* p = inet_ntop(rpc_endpoint.ss_family, addr, buff, len);
  if (p)
    m_rpc_addr = p;

  rtGetPort(rpc_endpoint, &m_rpc_port);
  return RT_OK;
}

rtError
rtRemoteFileResolver::registerObject(std::string const& name, sockaddr_storage const&)
{
  if (m_db_fp == NULL)
  {
    rtLogError("no database connection");
    return RT_FAIL;
  }

  // read in existing records into DOM
  rapidjson::Document doc;
  char readBuffer[65536];
  fseek(m_db_fp, 0, SEEK_SET);
  flock(fileno(m_db_fp), LOCK_EX);
  rapidjson::FileReadStream is(m_db_fp, readBuffer, sizeof(readBuffer));
  doc.ParseStream(is);
  flock(fileno(m_db_fp), LOCK_UN);

  rapidjson::Pointer("/" + name + "/" + kFieldNameIp).Set(doc, m_rpc_addr);
  rapidjson::Pointer("/" + name + "/" + kFieldNamePort).Set(doc, m_rpc_port);

  // write updated json back to file
  char writeBuffer[65536];
  flock(fileno(m_db_fp), LOCK_EX);
  fseek(m_db_fp, 0, SEEK_SET);  
  rapidjson::FileWriteStream os(m_db_fp, writeBuffer, sizeof(writeBuffer));
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  doc.Accept(writer);
  flock(fileno(m_db_fp), LOCK_UN);
  fflush(m_db_fp);

  return RT_OK;
}

rtError
rtRemoteFileResolver::locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t)
{
  if (m_db_fp == NULL)
  {
    rtLogError("no database connection");
    return RT_FAIL;
  }

  // read file into DOM object
  rapidjson::Document doc;
  char readBuffer[65536];
  flock(fileno(m_db_fp), LOCK_EX);
  fseek(m_db_fp, 0, SEEK_SET);  
  rapidjson::FileReadStream is(m_db_fp, readBuffer, sizeof(readBuffer));
  doc.ParseStream(is);
  flock(fileno(m_db_fp), LOCK_UN);
  
  // check if name is registered
  if (!rapidjson::Pointer("/" + name).Get(doc))
    return RT_FAIL;
  
  // pull registered IP and port
  rapidjson::Value *ip = rapidjson::Pointer("/" + name + "/" + kFieldNameIp).Get(doc);
  rapidjson::Value *port = rapidjson::Pointer("/" + name + "/" + kFieldNamePort).Get(doc);
  rtError err = rtParseAddress(endpoint, ip->GetString(), port->GetInt(), nullptr);
  if (err != RT_OK)
    return err;
    
  return RT_OK;
}

rtError
rtRemoteFileResolver::close()
{
  fclose(m_db_fp);
  return RT_OK;
}