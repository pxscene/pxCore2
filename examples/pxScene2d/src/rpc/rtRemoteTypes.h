#ifndef __RT_TYPES_H__
#define __RT_TYPES_H__

#include <chrono>
#include <condition_variable>
#include <map>
#include <memory>
#include <mutex>
#include <thread>
#include <queue>
#include <vector>

#include <rtError.h>
#include <rtObject.h>
#include <rapidjson/document.h>

#define kInvalidSocket (-1)

class rtRemoteClient;
class rtRemoteEnvironment;
class rtRemoteServer;
class rtRemoteConfig;
class rtRemoteStreamSelector;
class rtObjectCache;

using rtSocketBuffer = std::vector<char>;
using rtJsonDocPtr = std::shared_ptr< rapidjson::Document >;
using rtCorrelationKey = uint32_t;
using MessageHandler = rtError (*)(std::shared_ptr<rtRemoteClient>& client, rtJsonDocPtr const& msg, void* argp);

template<class TFunc>
struct Callback
{
  Callback() : Func(nullptr), Arg(nullptr) { }
  Callback(TFunc func, void* arg) : Func(func), Arg(arg) { }
  TFunc Func;
  void* Arg;
};


class rtRemoteAsyncHandle
{
  friend class rtRemoteStream;

public:
  ~rtRemoteAsyncHandle();

  rtJsonDocPtr response() const;
  rtError wait(uint32_t timeoutInMilliSeconds);

private:
  rtRemoteAsyncHandle(rtRemoteEnvironment* env, rtCorrelationKey k);
  void complete(rtJsonDocPtr const& doc, rtError e);

  static rtError onResponseHandler_Dispatch(std::shared_ptr<rtRemoteClient>& client,
        rtJsonDocPtr const& msg, void* argp)
    { return reinterpret_cast<rtRemoteAsyncHandle *>(argp)->onResponseHandler(client, msg); }

  rtError onResponseHandler(std::shared_ptr<rtRemoteClient>& client,
    rtJsonDocPtr const& msg);

private:
  rtRemoteEnvironment*    m_env;
  rtCorrelationKey        m_key;
  rtJsonDocPtr            m_doc;
  rtError                 m_error;
};

struct rtRemoteEnvironment
{
  rtRemoteEnvironment(rtRemoteConfig* config);
  ~rtRemoteEnvironment();

  void shutdown();
  void start();
  bool isQueueEmpty() const
  {
    std::unique_lock<std::mutex> lock(m_queue_mutex);
    return m_queue.empty();
  }

  rtRemoteConfig const*     Config;
  rtRemoteServer*           Server;
  rtObjectCache*            ObjectCache;
  rtRemoteStreamSelector*   StreamSelector;


  uint32_t RefCount;
  bool     Initialized;

  void registerResponseHandler(MessageHandler handler, void* argp, rtCorrelationKey k);
  void removeResponseHandler(rtCorrelationKey k);
  void enqueueWorkItem(std::shared_ptr<rtRemoteClient> const& clnt, rtJsonDocPtr const& doc);
  rtError processSingleWorkItem(std::chrono::milliseconds timeout, rtCorrelationKey* key = nullptr);

private:
  struct WorkItem
  {
    std::shared_ptr<rtRemoteClient> Client;
    std::shared_ptr<rapidjson::Document> Message;
  };

  using ResponseHandlerMap = std::map< rtCorrelationKey, Callback<MessageHandler> >;
  using ResponseMap = std::map< rtCorrelationKey, rtJsonDocPtr >;

  void processRunQueue();

  mutable std::mutex            m_queue_mutex;
  std::condition_variable       m_queue_cond;
  std::queue<WorkItem>          m_queue;
  std::unique_ptr<std::thread>  m_worker;
  bool                          m_running;
  ResponseHandlerMap            m_response_handlers;
};

class rtRemoteConfigBuilder
{
public:
  rtRemoteConfigBuilder();

  static rtRemoteConfigBuilder* getDefaultConfig();
  static rtRemoteConfigBuilder* fromFile(char const* file);

  rtRemoteConfig* build() const;

  char const* getString(char const* key) const;
  uint16_t    getUInt16(char const* key) const;
  uint32_t    getUInt32(char const* key) const;
  int32_t     getInt32(char const* key) const;
  bool        getBool(char const* key) const;

private:
  std::map< std::string, std::string > m_map;
};

#endif
