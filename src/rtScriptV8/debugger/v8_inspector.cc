#include "v8_inspector.h"

const char TAG_CONNECT[] = "#connect";
const char TAG_DISCONNECT[] = "#disconnect";
const char BREAK_START_MSG[] = "{\"id\":%d,\"method\":\"Debugger.setBreakpointByUrl\",\"params\":{\"urlRegex\":\"%s\",\"lineNumber\":0,\"columnNumber\":0}}";
int breakCounter = 1000;
static const uint8_t PROTOCOL_JSON[] = {
#include "v8_inspector_protocol_json.h"  // NOLINT(build/include_order)
};

std::string GetWsUrl(int port, const std::string& id) {
  char buf[1024];
  snprintf(buf, sizeof(buf), "localhost:%d/%s", port, id.c_str());
  return buf;
}

void PrintDebuggerReadyMessage(int port, const std::string& id) {
  fprintf(stderr, "Debugger listening on port %d\n",port);
  fflush(stderr);
}

void Escape(std::string* string) {
  for (char& c : *string) {
    c = (c == '\"' || c == '\\') ? '_' : c;
  }
}

void DisposeInspector(InspectorSocket* socket, int status) {
  delete socket;
}

void DisconnectAndDisposeIO(InspectorSocket* socket) {
  if (socket) {
    inspector_close(socket, DisposeInspector);
  }
}

void OnBufferAlloc(uv_handle_t* handle, size_t len, uv_buf_t* buf) {
  buf->base = new char[len];
  buf->len = len;
}

void SendHttpResponse(InspectorSocket* socket, const char* response,
                      size_t len) {
  const char HEADERS[] = "HTTP/1.0 200 OK\r\n"
                         "Content-Type: application/json; charset=UTF-8\r\n"
                         "Cache-Control: no-cache\r\n"
                         "Content-Length: %zu\r\n"
                         "\r\n";
  char header[sizeof(HEADERS) + 20];
  int header_len = snprintf(header, sizeof(header), HEADERS, len);
  inspector_write(socket, header, header_len);
  inspector_write(socket, response, len);
}

void SendVersionResponse(InspectorSocket* socket) {
  const char VERSION_RESPONSE_TEMPLATE[] =
      "[ {"
      "  \"Browser\": \"node.js/10.0\","
      "  \"Protocol-Version\": \"1.1\","
      "  \"User-Agent\": \"Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36"
            "(KHTML, like Gecko) Chrome/45.0.2446.0 Safari/537.36\","
      "  \"WebKit-Version\": \"537.36 (@198122)\""
      "} ]";
  char buffer[sizeof(VERSION_RESPONSE_TEMPLATE) + 128];
  size_t len = snprintf(buffer, sizeof(buffer), VERSION_RESPONSE_TEMPLATE);
  SendHttpResponse(socket, buffer, len);
}

std::string GetProcessTitle() {
  char title[2048];
  int err = uv_get_process_title(title, sizeof(title));
  if (err == 0) {
    return title;
  } else {
    return "Node.js";
  }
}

void SendTargentsListResponse(InspectorSocket* socket,
                              const std::string& script_name_,
                              const std::string& script_path_,
                              const std::string& id,
                              const std::string& ws_url) {
  const char LIST_RESPONSE_TEMPLATE[] =
      "[ {"
      "  \"description\": \"node.js instance\","
      "  \"devtoolsFrontendUrl\": "
            "\"https://chrome-devtools-frontend.appspot.com/serve_file/"
            "@1.2"
            "/inspector.html?experiments=true&v8only=true"
            "&ws=%s\","
      "  \"faviconUrl\": \"https://nodejs.org/static/favicon.ico\","
      "  \"id\": \"%s\","
      "  \"title\": \"%s\","
      "  \"type\": \"node\","
      "  \"url\": \"%s\","
      "  \"webSocketDebuggerUrl\": \"ws://%s\""
      "} ]";
  std::string title = script_name_.empty() ? GetProcessTitle() : script_name_;

  // This attribute value is a "best effort" URL that is passed as a JSON
  // string. It is not guaranteed to resolve to a valid resource.
  std::string url = "file://" + script_path_;

  Escape(&title);
  Escape(&url);

  int buf_len = sizeof(LIST_RESPONSE_TEMPLATE) + ws_url.length() * 2 +
                id.length() + title.length() + url.length();
  std::string buffer(buf_len, '\0');

  int len = snprintf(&buffer[0], buf_len, LIST_RESPONSE_TEMPLATE,
                     ws_url.c_str(), id.c_str(), title.c_str(), url.c_str(),
                     ws_url.c_str());
  buffer.resize(len);
  SendHttpResponse(socket, buffer.data(), len);
}

void SendProtocolJson(InspectorSocket* socket) {
  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  inflateInit(&strm);
  static const size_t kDecompressedSize =
      PROTOCOL_JSON[0] * 0x10000u +
      PROTOCOL_JSON[1] * 0x100u +
      PROTOCOL_JSON[2];
  strm.next_in = const_cast<uint8_t*>(PROTOCOL_JSON + 3);
  strm.avail_in = sizeof(PROTOCOL_JSON) - 3;
  std::vector<char> data(kDecompressedSize);
  strm.next_out = reinterpret_cast<Byte*>(&data[0]);
  strm.avail_out = data.size();
  inflate(&strm, Z_FINISH);
  inflateEnd(&strm);
  SendHttpResponse(socket, &data[0], data.size());
}

const char* match_path_segment(const char* path, const char* expected) {
  size_t len = strlen(expected);
  if(0 == strncasecmp(path, expected, len)) {
    if (path[len] == '/') return path + len + 1;
    if (path[len] == '\0') return path + len;
  }
  return nullptr;
}

// UUID RFC: https://www.ietf.org/rfc/rfc4122.txt
// Used ver 4 - with numbers
std::string GenerateID() {
  uint16_t buffer[8];
  RAND_bytes(reinterpret_cast<unsigned char*>(buffer), sizeof(buffer));
  char uuid[256];
  snprintf(uuid, sizeof(uuid), "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
           buffer[0],  // time_low
           buffer[1],  // time_mid
           buffer[2],  // time_low
           (buffer[3] & 0x0fff) | 0x4000,  // time_hi_and_version
           (buffer[4] & 0x3fff) | 0x8000,  // clk_seq_hi clk_seq_low
           buffer[5],  // node
           buffer[6],
           buffer[7]);
  return uuid;
}

void InterruptCallback(v8::Isolate*, void* agent) {
  static_cast<AgentImpl*>(agent)->DispatchMessages();
}

void DataCallback(uv_stream_t* stream, ssize_t read, const uv_buf_t* buf) {
  InspectorSocket* socket = inspector_from_stream(stream);
  static_cast<AgentImpl*>(socket->data)->OnRemoteDataIO(socket, read, buf);
}

/* ChannelImpl implementation */
ChannelImpl::ChannelImpl(AgentImpl* agent, v8::Isolate* isolate): agent_(agent),mIsolate(isolate) { }
ChannelImpl::~ChannelImpl() {}

void ChannelImpl::sendResponse(int callId, std::unique_ptr<v8_inspector::StringBuffer> message) {
    sendMessageToFrontend(message->string());
 }
void ChannelImpl::sendNotification(std::unique_ptr<v8_inspector::StringBuffer> message) {
  sendMessageToFrontend(message->string());
}

void ChannelImpl::flushProtocolNotifications() { }

void ChannelImpl::sendMessageToFrontend(const v8_inspector::StringView& message) {
  if (false == agent_->ignoreResponse)
  {
    int length = static_cast<int>(message.length());
    v8::Local<v8::String> message1 =
        (message.is8Bit()
             ? v8::String::NewFromOneByte(
                   mIsolate,
                   reinterpret_cast<const uint8_t*>(message.characters8()),
                   v8::NewStringType::kNormal, length)
             : v8::String::NewFromTwoByte(
                   mIsolate,
                   reinterpret_cast<const uint16_t*>(message.characters16()),
                   v8::NewStringType::kNormal, length))
            .ToLocalChecked();
    std::string data;
    for (int i=0; i<length; i++) {
      data.append((char*)(message.characters16()+i));
    }
    agent_->Write(agent_->frontend_session_id_, data);
  }
  agent_->ignoreResponse = false;
}

V8NodeInspector::V8NodeInspector(AgentImpl* agent, Environment* env,
                v8::Platform* platform)
                : agent_(agent),
                  env_(env),
                  platform_(platform),
                  terminated_(false),
                  running_nested_loop_(false),
                  inspector_(V8Inspector::create(env->isolate(), this)) {
}

void V8NodeInspector::runMessageLoopOnPause(int context_group_id) {
  if (running_nested_loop_)
    return;
  terminated_ = false;
  running_nested_loop_ = true;
  while (!terminated_) {
    agent_->WaitForFrontendMessage();
    while (v8::platform::PumpMessageLoop(platform_, env_->isolate()))
      {}
  }
  terminated_ = false;
  running_nested_loop_ = false;
}

double V8NodeInspector::currentTimeMS() {
  return uv_hrtime() * 1.0 / NANOS_PER_MSEC;
}

void V8NodeInspector::quitMessageLoopOnPause() {
  terminated_ = true;
}

void V8NodeInspector::connectFrontend() {
  session_ = inspector_->connect(1, new ChannelImpl(agent_, env_->isolate()), v8_inspector::StringView());
}

void V8NodeInspector::disconnectFrontend() {
  session_.reset();
}

void V8NodeInspector::dispatchMessageFromFrontend(const v8_inspector::StringView& message) {
  if (nullptr != session_) {
    session_->dispatchProtocolMessage(message);
  }
}

V8Inspector* V8NodeInspector::inspector() {
  return inspector_.get();
}

void V8NodeInspector::addContext(v8::Local<v8::Context> context, int contextGroupId)
{
  inspector_->contextCreated(
        v8_inspector::V8ContextInfo(context, 1, v8_inspector::StringView()));
}

AgentImpl::AgentImpl(Environment* env) : port_(0),
                                         wait_(false),
                                         shutting_down_(false),
                                         state_(State::kNew),
                                         parent_env_(env),
                                         data_written_(new uv_async_t()),
                                         client_socket_(nullptr),
                                         inspector_(nullptr),
                                         dispatching_messages_(false),
                                         frontend_session_id_(0),
                                         backend_session_id_(0),
                                         ignoreResponse(false),
                                         id_(GenerateID()) {
  uv_sem_init(&start_sem_, 0);
  memset(&io_thread_req_, 0, sizeof(io_thread_req_));
  uv_async_init(env->event_loop(), data_written_, nullptr);
  uv_unref(reinterpret_cast<uv_handle_t*>(data_written_));
  pthread_mutex_init(&lock_, NULL);
  pthread_cond_init(&incoming_message_cond_, NULL);
  platform_ = env->platform();
}

AgentImpl::~AgentImpl() {
  auto close_cb = [](uv_handle_t* handle) {
    delete reinterpret_cast<uv_async_t*>(handle);
  };
  uv_close(reinterpret_cast<uv_handle_t*>(data_written_), close_cb);
  data_written_ = nullptr;
  platform_ = nullptr;
  uv_sem_destroy(&start_sem_);
  pthread_mutex_destroy(&lock_);
  pthread_cond_destroy(&incoming_message_cond_);
}

bool AgentImpl::Start(const char* path,
                      int port, bool wait) {
  auto env = parent_env_;
  inspector_ = new V8NodeInspector(this, env, platform_);
  if (path != nullptr)
    script_name_ = path;

  int err = uv_loop_init(&child_loop_);

  if (0 != err)
    return false;

  port_ = port;
  wait_ = wait;

  err = uv_thread_create(&thread_, AgentImpl::ThreadCbIO, this);
  if (0 != err)
    return false;

  uv_sem_wait(&start_sem_);

  if (state_ == State::kError) {
    Stop();
    return false;
  }
  state_ = State::kAccepting;
  if (wait) {
    DispatchMessages();
  }
  return true;
}

void AgentImpl::Stop() {
  delete inspector_;
}

bool AgentImpl::IsStarted() {
  return !!platform_;
}

void AgentImpl::WaitForDisconnect() {
  shutting_down_ = true;
  fprintf(stderr, "Waiting for the debugger to disconnect...\n");
  fflush(stderr);
  inspector_->runMessageLoopOnPause(0);
}

void AgentImpl::addContext(v8::Local<v8::Context>& context, int group)
{
  inspector_->addContext(context, group);
}

void AgentImpl::breakOnStart(std::string url)
{
  char msg[1024];
  memset(msg, 0, 1024);
  sprintf(msg, BREAK_START_MSG, breakCounter++, url.c_str());
  std::string breakCmd(msg);
  inspector_->dispatchMessageFromFrontend(v8_inspector::StringView((const unsigned char*)breakCmd.c_str(), breakCmd.length()));
  //ignoreResponse = true;
}

// static
void AgentImpl::ThreadCbIO(void* agent) {
  static_cast<AgentImpl*>(agent)->WorkerRunIO();
}

// static
void AgentImpl::OnSocketConnectionIO(uv_stream_t* server, int status) {
  if (status == 0) {
    InspectorSocket* socket = new InspectorSocket();
    socket->data = server->data;
    if (inspector_accept(server, socket,
                         AgentImpl::OnInspectorHandshakeIO) != 0) {
      delete socket;
    }
  }
}

// static
bool AgentImpl::OnInspectorHandshakeIO(InspectorSocket* socket,
                                       enum inspector_handshake_event state,
                                       const std::string& path) {
  AgentImpl* agent = static_cast<AgentImpl*>(socket->data);
  switch (state) {
  case kInspectorHandshakeHttpGet:
    return agent->RespondToGet(socket, path);
  case kInspectorHandshakeUpgrading:
    return path.length() == agent->id_.length() + 1 &&
           path.find(agent->id_) == 1;
  case kInspectorHandshakeUpgraded:
    agent->OnInspectorConnectionIO(socket);
    return true;
  case kInspectorHandshakeFailed:
    delete socket;
    return false;
  default:
    return false;
  }
}

void AgentImpl::OnRemoteDataIO(InspectorSocket* socket,
                               ssize_t read,
                               const uv_buf_t* buf) {
  if (read > 0) {
    std::string str = std::string(buf->base, read);
    if (wait_&& str.find("\"Runtime.runIfWaitingForDebugger\"")
        != std::string::npos) {
      wait_ = false;
      uv_sem_post(&start_sem_);
    }
    PostIncomingMessage(str);
   } else {
    // EOF
    if (client_socket_ == socket) {
      std::string message = std::string(TAG_DISCONNECT, sizeof(TAG_DISCONNECT) - 1);
      client_socket_ = nullptr;
      PostIncomingMessage(message);
    }
    DisconnectAndDisposeIO(socket);
  }
  if (buf) {
    delete[] buf->base;
  }
}

bool AgentImpl::RespondToGet(InspectorSocket* socket, const std::string& path) {
  const char* command = match_path_segment(path.c_str(), "/json");
  if (command == nullptr)
    return false;

  if (match_path_segment(command, "list") || command[0] == '\0') {
    SendTargentsListResponse(socket, script_name_, script_path_, id_,
                             GetWsUrl(port_, id_));
  } else if (match_path_segment(command, "protocol")) {
    SendProtocolJson(socket);
  } else if (match_path_segment(command, "version")) {
    SendVersionResponse(socket);
  } else {
    const char* pid = match_path_segment(command, "activate");
    if (pid != id_)
      return false;
    const char TARGET_ACTIVATED[] = "Target activated";
    SendHttpResponse(socket, TARGET_ACTIVATED, sizeof(TARGET_ACTIVATED) - 1);
  }
  return true;
}

// static
void AgentImpl::WriteCbIO(uv_async_t* async) {
  AgentImpl* agent = static_cast<AgentImpl*>(async->data);
  InspectorSocket* socket = agent->client_socket_;
  if (socket) {
    MessageQueue outgoing_messages;
    agent->SwapBehindLock(&agent->outgoing_message_queue_, &outgoing_messages);
    for (const MessageQueue::value_type& outgoing : outgoing_messages) {
      if (outgoing.first == agent->frontend_session_id_) {
        std::string message = outgoing.second;
        inspector_write(socket, message.c_str(), message.length());
      }
    }
  }
}

void AgentImpl::WorkerRunIO() {
  sockaddr_in addr;
  uv_tcp_t server;
  int err = uv_loop_init(&child_loop_);
  err = uv_async_init(&child_loop_, &io_thread_req_, AgentImpl::WriteCbIO);
  io_thread_req_.data = this;
  if (!script_name_.empty()) {
    uv_fs_t req;
    if (0 == uv_fs_realpath(&child_loop_, &req, script_name_.c_str(), nullptr))
      script_path_ = std::string(reinterpret_cast<char*>(req.ptr));
    uv_fs_req_cleanup(&req);
  }
  uv_tcp_init(&child_loop_, &server);
  uv_ip4_addr("0.0.0.0", port_, &addr);
  server.data = this;
  err = uv_tcp_bind(&server,
                    reinterpret_cast<const struct sockaddr*>(&addr), 0);
  if (err == 0) {
    err = uv_listen(reinterpret_cast<uv_stream_t*>(&server), 1,
                    OnSocketConnectionIO);
  }
  if (err != 0) {
    fprintf(stderr, "Unable to open devtools socket: %s\n", uv_strerror(err));
    state_ = State::kError;  // Safe, main thread is waiting on semaphore
    uv_close(reinterpret_cast<uv_handle_t*>(&io_thread_req_), nullptr);
    uv_close(reinterpret_cast<uv_handle_t*>(&server), nullptr);
    uv_loop_close(&child_loop_);
    uv_sem_post(&start_sem_);
    return;
  }
  PrintDebuggerReadyMessage(port_, id_);
  if (!wait_) {
    uv_sem_post(&start_sem_);
  }
  uv_run(&child_loop_, UV_RUN_DEFAULT);
  uv_close(reinterpret_cast<uv_handle_t*>(&io_thread_req_), nullptr);
  uv_close(reinterpret_cast<uv_handle_t*>(&server), nullptr);
  DisconnectAndDisposeIO(client_socket_);
  uv_run(&child_loop_, UV_RUN_NOWAIT);
  err = uv_loop_close(&child_loop_);
}

bool AgentImpl::AppendMessage(MessageQueue* queue, int session_id,
                              const std::string message) {
  pthread_mutex_lock(&lock_);
  bool trigger_pumping = queue->empty();
  queue->push_back(std::make_pair(session_id, message));
  pthread_mutex_unlock(&lock_);
  return trigger_pumping;
}

void AgentImpl::SwapBehindLock(MessageQueue* vector1, MessageQueue* vector2) {
  pthread_mutex_lock(&lock_);
  vector1->swap(*vector2);
  pthread_mutex_unlock(&lock_);
}

void AgentImpl::PostIncomingMessage(std::string message) {
  if (AppendMessage(&incoming_message_queue_, frontend_session_id_, message)) {
    v8::Isolate* isolate = parent_env_->isolate();
    platform_->CallOnForegroundThread(isolate,
                                      new DispatchOnInspectorBackendTask(this));
    isolate->RequestInterrupt(InterruptCallback, this);
    uv_async_send(data_written_);
  }
  NotifyMessageReceived();
}

void AgentImpl::WaitForFrontendMessage() {
  pthread_mutex_lock(&lock_);
  while (incoming_message_queue_.empty())
    pthread_cond_wait(&incoming_message_cond_, &lock_);
  pthread_mutex_unlock(&lock_);
}

void AgentImpl::NotifyMessageReceived() {
  pthread_cond_signal(&incoming_message_cond_);
}

void AgentImpl::OnInspectorConnectionIO(InspectorSocket* socket) {
  if (client_socket_) {
    DisconnectAndDisposeIO(socket);
    return;
  }
  client_socket_ = socket;
  inspector_read_start(socket, OnBufferAlloc, DataCallback);
  frontend_session_id_++;
  PostIncomingMessage(std::string(TAG_CONNECT, sizeof(TAG_CONNECT) - 1));
}

void AgentImpl::DispatchMessages() {
  // This function can be reentered if there was an incoming message while
  // V8 was processing another inspector request (e.g. if the user is
  // evaluating a long-running JS code snippet). This can happen only at
  // specific points (e.g. the lines that call inspector_ methods)
  if (dispatching_messages_)
    return;
  dispatching_messages_ = true;
  MessageQueue tasks;
  do {
    tasks.clear();
    SwapBehindLock(&incoming_message_queue_, &tasks);
    for (const MessageQueue::value_type& pair : tasks) {
      const std::string message = pair.second;
      if (message == TAG_CONNECT) {
        backend_session_id_++;
        state_ = State::kConnected;
        fprintf(stderr, "Debugger attached.\n");
        inspector_->connectFrontend();
      } else if (message == TAG_DISCONNECT) {
        if (shutting_down_) {
          state_ = State::kDone;
        } else {
          PrintDebuggerReadyMessage(port_, id_);
          state_ = State::kAccepting;
        }
        inspector_->quitMessageLoopOnPause();
        inspector_->disconnectFrontend();
      } else {
        inspector_->dispatchMessageFromFrontend(v8_inspector::StringView((const unsigned char*)message.c_str(), message.length()));
      }
    }
  } while (!tasks.empty());
  uv_async_send(data_written_);
  dispatching_messages_ = false;
}

void AgentImpl::Write(int session_id, const std::string message) {
  AppendMessage(&outgoing_message_queue_, session_id, message);
  int err = uv_async_send(&io_thread_req_);
}
