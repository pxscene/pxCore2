#include "rtRpcTransport.h"
#include "rtRpcTransport.h"
#include "rtSocketUtils.h"

#include "rapidjson/rapidjson.h"

#include <rtLog.h>

#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <rapidjson/document.h>

rtRpcTransport::rtRpcTransport(sockaddr_storage const& ss)
  : m_fd(-1)
  , m_thread(0)
  , m_corkey(1)
{
  m_remote_endpoint = ss;
  pthread_mutex_init(&m_mutex, NULL);
  pthread_cond_init(&m_cond, NULL);
}

rtRpcTransport::~rtRpcTransport()
{
  if (m_fd > 0)
  {
    shutdown(m_fd, SHUT_RDWR);
    close(m_fd);
  }
}

rtError
rtRpcTransport::start()
{
  rtError err = connect_rpc_endpoint();
  if (err != RT_OK)
  {
    rtLogWarn("failed to connect to rpc endpoint");
    return err;
  }

  pthread_create(&m_thread, NULL, &rtRpcTransport::run_listener, this);
  return RT_OK;
}

rtError
rtRpcTransport::connect_rpc_endpoint()
{
  m_fd = socket(m_remote_endpoint.ss_family, SOCK_STREAM, 0);
  if (m_fd < 0)
  {
    rtLogError("failed to create socket. %s", strerror(errno));
    return RT_FAIL;
  }

  socklen_t len;
  rtSocketGetLength(m_remote_endpoint, &len);

  int ret = connect(m_fd, reinterpret_cast<sockaddr *>(&m_remote_endpoint), len);
  if (ret < 0)
  {
    rtLogError("failed to connect to remote rpc endpoint. %s", strerror(errno));
    return RT_FAIL;
  }

  rtLogInfo("new tcp connection to: %s", rtSocketToString(m_remote_endpoint).c_str());
  return RT_OK;
}

rtError
rtRpcTransport::start_session(std::string const& object_id)
{
  rtError err = RT_OK;

  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember("type", "session.open.request", doc.GetAllocator());
  doc.AddMember("object-id", object_id, doc.GetAllocator());

  int key = next_key();
  doc.AddMember("corkey", key, doc.GetAllocator());

  err = rtSendDocument(doc, m_fd, NULL);
  if (err != RT_OK)
    return err;

  request_map_t::const_iterator itr;

  // TODO: std::future/std::promise
  pthread_mutex_lock(&m_mutex);
  while ((itr = m_requests.find(key)) == m_requests.end())
    pthread_cond_wait(&m_cond, &m_mutex);
  pthread_mutex_unlock(&m_mutex);

  return RT_OK;
}

rtError
rtRpcTransport::send_keep_alive()
{
  rapidjson::Document doc;
  doc.SetObject();
  doc.AddMember("type", "keep-alive", doc.GetAllocator());
  rapidjson::Value ids(rapidjson::kArrayType);
  for (auto const& id : m_object_list)
    ids.PushBack(rapidjson::Value().SetString(id.c_str(), id.size()), doc.GetAllocator());
  doc.AddMember("ids", ids, doc.GetAllocator());
  return rtSendDocument(doc, m_fd, NULL);
}

void*
rtRpcTransport::run_listener(void* argp)
{
  rtRpcTransport* obj = reinterpret_cast<rtRpcTransport *>(argp);
  obj->run_listener();
  return NULL;
}

rtError
rtRpcTransport::run_listener()
{
  rt_sockbuf_t buff;
  buff.reserve(1024 * 4);
  buff.resize(1024 * 4);

  while (true)
  {
    int maxFd = 0;

    fd_set read_fds;
    fd_set err_fds;

    FD_ZERO(&read_fds);
    rtPushFd(&read_fds, m_fd, &maxFd);

    FD_ZERO(&err_fds);
    rtPushFd(&err_fds, m_fd, &maxFd);

    int ret = select(maxFd + 1, &read_fds, NULL, &err_fds, NULL);
    if (ret == -1)
    {
      int err = errno;
      rtLogWarn("select failed. %s", strerror(errno));
      if (err == EBADF)
        return RT_OK;
      else
        continue;
    }

    if (FD_ISSET(m_fd, &read_fds))
      readn(m_fd, buff);
  }

  return RT_OK;
}

rtError
rtRpcTransport::readn(int fd, rt_sockbuf_t& buff)
{
  rtJsonDocPtr_t doc;
  rtError err = rtReadMessage(fd, buff, doc);
  if (err != RT_OK)
    return err;

  #ifdef RT_RPC_DEBUG
  rtLogDebug("read:\n\t\"%.*s\"\n", (int) buff.size(), &buff[0]);
  #endif

  if (!doc->HasMember("type"))
  {
    rtLogWarn("received JSON message with no type");
    return RT_FAIL;
  }

  std::string cmd = (*doc)["type"].GetString();

  // TODO: this could be done with std::future/std::promise
  int key = -1;
  if (doc->HasMember("corkey"))
    key = (*doc)["corkey"].GetInt();

  // should be std::future/std::promise
  if (key != -1)
  {
    pthread_mutex_lock(&m_mutex);
    m_requests[key] = doc;
    pthread_cond_broadcast(&m_cond);
    pthread_mutex_unlock(&m_mutex);
  }

  // https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn
  #define CALL_MEMBER_FN(object,ptrToMember)  ((object).*(ptrToMember))

  auto itr = m_message_handlers.find(cmd);
  if (itr != m_message_handlers.end())
    err = CALL_MEMBER_FN(*this, itr->second)(doc); // , peer);

  return err;
}

rtJsonDocPtr_t
rtRpcTransport::wait_for_response(int key)
{
  rtJsonDocPtr_t response;

  // TODO: std::future/std::promise
  request_map_t::const_iterator itr;
  pthread_mutex_lock(&m_mutex);
  while ((itr = m_requests.find(key)) == m_requests.end())
    pthread_cond_wait(&m_cond, &m_mutex);
  response = itr->second;
  m_requests.erase(itr);
  pthread_mutex_unlock(&m_mutex);

  return response;
}

rtError
rtRpcTransport::get(std::string const& id, char const* name, rtValue* value)
{
  int key = next_key();

  rapidjson::Document req;
  req.SetObject();
  req.AddMember("type", "get.byname.request", req.GetAllocator());
  req.AddMember("name", std::string(name), req.GetAllocator());
  req.AddMember("object-id", id, req.GetAllocator());
  req.AddMember("corkey", key, req.GetAllocator());

  rtError err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = wait_for_response(key);
  if (!res)
    return RT_FAIL;

  err = rtValueReader::read(*value, (*res)["value"], shared_from_this());
  if (err != RT_OK)
    return err;

  return static_cast<rtError>((*res)["status"].GetInt());
}

rtError
rtRpcTransport::get(std::string const& id, uint32_t index, rtValue* value)
{
  int key = next_key();
  
  rapidjson::Document req;
  req.SetObject();
  req.AddMember("type", "get.byindex.request", req.GetAllocator());
  req.AddMember("index", index, req.GetAllocator());
  req.AddMember("object-id", id, req.GetAllocator());
  req.AddMember("corkey", key, req.GetAllocator());
  
  rtError err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;
  
  rtJsonDocPtr_t res = wait_for_response(key);
  if (!res)
    return RT_FAIL;

  err = rtValueReader::read(*value, *res, shared_from_this()); 
  if (err != RT_OK)
    return err;

  return static_cast<rtError>((*res)["status"].GetInt());
}

rtError
rtRpcTransport::set(std::string const& id, char const* name, rtValue const* value)
{
  int key = next_key();

  rapidjson::Document req;
  req.SetObject();
  req.AddMember("type", "set.byname.request", req.GetAllocator());
  req.AddMember("name", std::string(name), req.GetAllocator());
  req.AddMember("object-id", id, req.GetAllocator());
  req.AddMember("corkey", key, req.GetAllocator());

  rapidjson::Value val;
  rtError err = rtValueWriter::write(*value, val, req);
  if (err != RT_OK)
    return err;
  req.AddMember("value", val, req.GetAllocator());

  err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = wait_for_response(key);
  if (!res)
    return RT_FAIL;

  return static_cast<rtError>((*res)["status"].GetInt());
}

rtError
rtRpcTransport::set(std::string const& id, uint32_t index, rtValue const* value)
{
  int key = next_key();

  rapidjson::Document req;
  req.SetObject();
  req.AddMember("type", "set.byindex.request", req.GetAllocator());
  req.AddMember("index", index, req.GetAllocator());
  req.AddMember("object-id", id, req.GetAllocator());
  req.AddMember("corkey", key, req.GetAllocator());

  rapidjson::Value val;
  rtError err = rtValueWriter::write(*value, val, req);
  if (err != RT_OK)
    return err;
  req.AddMember("value", val, req.GetAllocator());

  err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = wait_for_response(key);
  if (!res)
    return RT_FAIL;

  return static_cast<rtError>((*res)["status"].GetInt()); 
}

rtError
rtRpcTransport::send(std::string const& id, std::string const& name, int argc, rtValue const* argv, rtValue* result)
{
  rtError err = RT_OK;

  int key = next_key();

  rapidjson::Document req;
  req.SetObject();
  req.AddMember("type", "method.call.request", req.GetAllocator());
  if (id.size() > 0)
    req.AddMember("object-id", id, req.GetAllocator());
  req.AddMember("name", name, req.GetAllocator());
  req.AddMember("corkey", key, req.GetAllocator());

  rapidjson::Value args(rapidjson::kArrayType);
  for (int i = 0; i < argc; ++i)
  {
    rapidjson::Value arg;
    rtError err = rtValueWriter::write(argv[i], arg, req);
    if (err != RT_OK)
      return err;
    args.PushBack(arg, req.GetAllocator());
  }
  req.AddMember("args", args, req.GetAllocator());

  err = rtSendDocument(req, m_fd, NULL);
  if (err != RT_OK)
    return err;

  rtJsonDocPtr_t res = wait_for_response(key);
  if (!res)
    return RT_FAIL;

  err = rtValueReader::read(*result, (*res)["return_value"], shared_from_this());
  if (err != RT_OK)
    return err;

  return static_cast<rtError>((*res)["status"].GetInt());
}
