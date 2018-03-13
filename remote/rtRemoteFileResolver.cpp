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

#include "rtRemoteFileResolver.h"
#include "rtRemoteSocketUtils.h"
#include "rtRemoteMessage.h"
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"

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

class FileLocker
{
public:
  FileLocker(FILE* f) : m_file(f)
  {
    lock();
  }

  ~FileLocker()
  {
    unlock();
  }

  void lock()
  {
    if (m_file)
    {
      int ret = flock(fileno(m_file), LOCK_EX);
      if (ret == -1)
      {
        rtError e = rtErrorFromErrno(errno);
        rtLogWarn("failed to lock file. %s", rtStrError(e));
      }
    }
  }

  void unlock()
  {
    if (m_file)
    {
      int ret = flock(fileno(m_file), LOCK_UN);
      if (ret == -1)
      {
        rtError e = rtErrorFromErrno(errno);
        rtLogWarn("failed to unlock file. %s", rtStrError(e));
      }
      m_file = nullptr;
    }
  }

private:
  FILE* m_file;
};


rtRemoteFileResolver::rtRemoteFileResolver(rtRemoteEnvironment* env)
: m_db_fp(nullptr)
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
  std::string dbPath = m_env->Config->resolver_file_db_path();
  m_db_fp = fopen(dbPath.c_str(), "w+");
  if (m_db_fp == nullptr)
  {
    rtError e = rtErrorFromErrno(errno);
    rtLogError("could not open database file %s. %s", dbPath.c_str(), rtStrError(e));
    return e;
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
  if (m_db_fp == nullptr)
  {
    rtLogError("no database connection");
    return RT_ERROR_INVALID_ARG;
  }

  // read in existing records into DOM
  std::vector<char> buff(1024 * 64);
  fseek(m_db_fp, 0, SEEK_SET);

  FileLocker fileLocker(m_db_fp);
  rapidjson::FileReadStream is(m_db_fp, &buff[0], buff.capacity());

  rapidjson::Document doc;
  doc.ParseStream(is);

  rapidjson::Pointer("/" + name + "/" + kFieldNameIp).Set(doc, m_rpc_addr);
  rapidjson::Pointer("/" + name + "/" + kFieldNamePort).Set(doc, m_rpc_port);

  // write updated json back to file
  buff[0] = '\0';

  fseek(m_db_fp, 0, SEEK_SET);  
  rapidjson::FileWriteStream os(m_db_fp, &buff[0], buff.capacity());
  rapidjson::Writer<rapidjson::FileWriteStream> writer(os);
  doc.Accept(writer);
  fflush(m_db_fp);

  return RT_OK;
}

rtError
rtRemoteFileResolver::locateObject(std::string const& name, sockaddr_storage& endpoint,
    uint32_t)
{
  if (m_db_fp == nullptr)
  {
    rtLogError("no database connection");
    return RT_ERROR_INVALID_ARG;
  }

  std::vector<char> buff(1024 * 64);

  FileLocker fileLocker(m_db_fp);
  fseek(m_db_fp, 0, SEEK_SET);  
  rapidjson::FileReadStream is(m_db_fp, &buff[0], buff.capacity());

  rapidjson::Document doc;
  doc.ParseStream(is);

  fileLocker.unlock();
  
  // check if name is registered
  if (!rapidjson::Pointer("/" + name).Get(doc))
    return RT_RESOURCE_NOT_FOUND;
  
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
  if (m_db_fp != nullptr)
    fclose(m_db_fp);
  return RT_OK;
}

rtError
rtRemoteFileResolver::unregisterObject(std::string const& /*name*/)
{
  return RT_ERROR_NOT_IMPLEMENTED;
}
