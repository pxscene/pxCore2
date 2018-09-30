#include "inspector_agent.h"
#include <strings.h>
#include "inspector_socket.h"
#include "v8-platform.h"
#include "v8-inspector.h"
#include <openssl/rand.h>
#include <zlib.h>
#include <pthread.h>

#include "libplatform/libplatform.h"

#include <string.h>
#include <utility>
#include <vector>


class V8NodeInspector;

class AgentImpl {
 public:
  explicit AgentImpl(Environment* env);
  ~AgentImpl();

  // Start the inspector agent thread
  bool Start(const char* path, int port, bool wait);
  // Stop the inspector agent
  void Stop();

  bool IsStarted();
  bool IsConnected() {  return state_ == State::kConnected; }
  void WaitForDisconnect();
  void addContext(v8::Local<v8::Context>& context, int group);
  void breakOnStart(std::string url);
  
 private:
  using MessageQueue = std::vector<std::pair<int, std::string>>;
  enum class State { kNew, kAccepting, kConnected, kDone, kError };

  static void ThreadCbIO(void* agent);
  static void OnSocketConnectionIO(uv_stream_t* server, int status);
  static bool OnInspectorHandshakeIO(InspectorSocket* socket,
                                     enum inspector_handshake_event state,
                                     const std::string& path);
  static void WriteCbIO(uv_async_t* async);

  void WorkerRunIO();
  void OnInspectorConnectionIO(InspectorSocket* socket);
  void OnRemoteDataIO(InspectorSocket* stream, ssize_t read,
                      const uv_buf_t* b);
  void SetConnected(bool connected);
  void DispatchMessages();
  void Write(int session_id, const std::string message);
  bool AppendMessage(MessageQueue* vector, int session_id,
                     const std::string message);
  void SwapBehindLock(MessageQueue* vector1, MessageQueue* vector2);
  void PostIncomingMessage(std::string message);
  void WaitForFrontendMessage();
  void NotifyMessageReceived();
  bool RespondToGet(InspectorSocket* socket, const std::string& path);
  State ToState(State state);

  uv_sem_t start_sem_;
  pthread_mutex_t lock_;
  pthread_cond_t incoming_message_cond_;
  uv_thread_t thread_;
  uv_loop_t child_loop_;

  int port_;
  bool wait_;
  bool shutting_down_;
  State state_;
  Environment* parent_env_;

  uv_async_t* data_written_;
  uv_async_t io_thread_req_;
  InspectorSocket* client_socket_;
  V8NodeInspector* inspector_;
  v8::Platform* platform_;
  MessageQueue incoming_message_queue_;
  MessageQueue outgoing_message_queue_;
  bool dispatching_messages_;
  int frontend_session_id_;
  int backend_session_id_;

  std::string script_name_;
  std::string script_path_;
  const std::string id_;
  bool ignoreResponse;

  friend class ChannelImpl;
  friend class DispatchOnInspectorBackendTask;
  friend class SetConnectedTask;
  friend class V8NodeInspector;
  friend void InterruptCallback(v8::Isolate*, void* agent);
  friend void DataCallback(uv_stream_t* stream, ssize_t read,
                           const uv_buf_t* buf);
};

class DispatchOnInspectorBackendTask : public v8::Task {
 public:
  explicit DispatchOnInspectorBackendTask(AgentImpl* agent) : agent_(agent) {}

  void Run() override {
    agent_->DispatchMessages();
  }

 private:
  AgentImpl* agent_;
};


/* class to handle messages back from v8 inspector to front end */
class ChannelImpl final : public v8_inspector::V8Inspector::Channel {
 public:
  explicit ChannelImpl(AgentImpl* agent, v8::Isolate* isolate);
  virtual ~ChannelImpl();

 private:

  void sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) override;
  void sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) override;
  void flushProtocolNotifications() override;

  void sendMessageToFrontend(const v8_inspector::StringView& message);
  v8::Isolate* mIsolate;
  AgentImpl* const agent_;
};

// Used in V8NodeInspector::currentTimeMS() below.
#define NANOS_PER_MSEC 1000000

using V8Inspector = v8_inspector::V8Inspector;

/* class to handle messages back from front end to v8 inspector */
class V8NodeInspector : public v8_inspector::V8InspectorClient {
 public:
  V8NodeInspector(AgentImpl* agent, Environment* env, v8::Platform* platform);
  void runMessageLoopOnPause(int context_group_id) override;
  double currentTimeMS() override;
  void quitMessageLoopOnPause() override;

  void connectFrontend();
  void disconnectFrontend();
  void dispatchMessageFromFrontend(const v8_inspector::StringView& message);
  V8Inspector* inspector();
  void addContext(v8::Local<v8::Context> context, int contextGroupId);

 private:
  AgentImpl* agent_;
  Environment* env_;
  v8::Platform* platform_;
  bool terminated_;
  bool running_nested_loop_;
  std::unique_ptr<V8Inspector> inspector_;
  std::unique_ptr<v8_inspector::V8InspectorSession> session_;
};
