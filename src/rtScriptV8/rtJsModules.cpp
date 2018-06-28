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

#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtJsModules.h"
#include <rtMutex.h>

#include <string>
#include <fstream>
#include <sstream>

#if defined(USE_STD_THREADS)
#include <thread>
#include <mutex>
#endif

#include "v8_headers.h"

#if !defined(_WIN32)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef  S_ISREG
# define S_ISREG(x)  (((x) & _S_IFMT) == _S_IFREG)
#endif
#ifndef  S_ISDIR
# define S_ISDIR(x)  (((x) & _S_IFMT) == _S_IFDIR)
#endif
#ifndef  S_ISFIFO
# define S_ISFIFO(x) (((x) & _S_IFMT) == _S_IFIFO)
#endif
#ifndef  S_ISCHR
# define S_ISCHR(x)  (((x) & _S_IFMT) == _S_IFCHR)
#endif
#ifndef  S_ISBLK
# define S_ISBLK(x)  0
#endif
#ifndef  S_ISLINK
# define S_ISLNK(x)  0
#endif
#ifndef  S_ISSOCK
# define S_ISSOCK(x) 0
#endif
#endif

using namespace std;

//-----------------------------------

namespace rtScriptV8Utils
{

static void uvGetPlatform(const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  const char *platform = "unknown";
#ifdef _WIN32
  platform = "win32";
#elif defined(__linux__)
  platform = "linux";
#elif defined(__APPLE__)
  platform = "macosx";
#endif
  args.GetReturnValue().Set<v8::String>(String::NewFromUtf8(args.GetIsolate(), platform, NewStringType::kNormal).ToLocalChecked());
}

static void uvGetHrTime(const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);
  uint64_t hrtime = uv_hrtime();
  int nanoseconds = hrtime % (int)1e9;
  int seconds = hrtime / 1e9;

  Local<Array> arr = Array::New(isolate, 2);
  arr->Set(0, Integer::New(isolate, seconds));
  arr->Set(1, Integer::New(isolate, nanoseconds));

  args.GetReturnValue().Set<Array>(arr);
}

uv_loop_t *getEventLoopFromArgs(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  assert(args.Data()->IsExternal());
  v8::External *val = v8::External::Cast(*args.Data());
  assert(val != NULL);
  uv_loop_t *loop = (uv_loop_t *)val->Value();
  return loop;
}

static void uvFsAccess(const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().Set(-1);

  if (args.Length() != 2) {
    return;
  }

  assert(args.Length() == 2);
  assert(args[0]->IsString());
  assert(args[1]->IsString());

  rtString filePath = toString(args[0]->ToString());
  rtString fileOpenMode = toString(args[1]->ToString());

  int mode = 0;
  size_t i, l;
  for (i = 0, l = fileOpenMode.length(); i < l; ++i) {
    switch (fileOpenMode[i]) {
    case 'r': case 'R': mode |= R_OK; break;
    case 'w': case 'W': mode |= W_OK; break;
    case 'x': case 'X': mode |= X_OK; break;
    default:
      rtLogWarn("Unknown file access mode '%s'", fileOpenMode.cString());
      return;
    }
  }
  
  uv_fs_t req;
  int ret = uv_fs_access(loop, &req, filePath.cString(), mode, NULL);

  args.GetReturnValue().Set(ret);
}

static void uvFsGetSize(const v8::FunctionCallbackInfo<v8::Value>& args) 
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().Set(-1);

  if (args.Length() != 1) {
    return;
  }

  assert(args.Length() == 1);
  assert(args[0]->IsString());

  rtString filePath = toString(args[0]->ToString());

  uv_fs_t req;
  int ret = uv_fs_stat(loop, &req, filePath.cString(), NULL);

  if (ret < 0) {
    return;
  }

  args.GetReturnValue().Set((uint32_t)req.statbuf.st_size);
}

static int stringToFlags(const char* s) 
{
  bool read = false;
  bool write = false;
  int flags = 0;
  while (s && s[0]) {
    switch (s[0]) {
    case 'r': read = true; break;
    case 'w': write = true; flags |= O_TRUNC | O_CREAT; break;
    case 'a': write = true; flags |= O_APPEND | O_CREAT; break;
    case '+': read = true; write = true; break;
    case 'x': flags |= O_EXCL; break;

#ifndef _WIN32
    case 's': flags |= O_SYNC; break;
#endif

    default:
      return 0;
    }
    s++;
  }
  flags |= read ? (write ? O_RDWR : O_RDONLY) :
    (write ? O_WRONLY : 0);
  return flags;
}

static void uvFsOpen(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().SetNull();

  if (args.Length() != 3) {
    return;
  }

  assert(args.Length() == 3);
  assert(args[0]->IsString());
  assert(args[1]->IsString());

  rtString filePath = toString(args[0]->ToString());
  rtString fileOpenMode = toString(args[1]->ToString());

  int flags = stringToFlags(fileOpenMode.cString());
  int mode = (int)args[2]->ToInteger()->Value();

  uv_fs_t req;
  int ret = uv_fs_open(loop, &req, filePath.cString(), flags, mode, NULL);

  if (ret < 0) {
    return;
  }

  args.GetReturnValue().Set(v8::External::New(isolate, (void *)req.result));
}

static void uvFsRead(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().SetNull();

  if (args.Length() != 3 || !args[0]->IsExternal()) {
    return;
  }

  assert(args.Length() == 3);
  assert(args[0]->IsExternal());

  v8::External *val = v8::External::Cast(*args[0]);
  void *fd = (void *)val->Value();
  int size = (int)args[1]->ToInteger()->Value();
  int offset = (int)args[2]->ToInteger()->Value();

  void *ptr = malloc(size);
  uv_buf_t buf = uv_buf_init((char *)ptr, size);

  uv_fs_t req;
  uv_fs_read(loop, &req, static_cast<uv_file>(reinterpret_cast<uint64_t>(fd)), &buf, 1, offset, NULL);

  if (req.result <= 0) {
    free(ptr);
    return;
  }

  Local<ArrayBuffer> arrBuf = ArrayBuffer::New(isolate, ptr, req.result, ArrayBufferCreationMode::kInternalized);
  free(ptr);

  args.GetReturnValue().Set(arrBuf);
}

static void uvFsClose(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().Set(-1);

  if (args.Length() != 1 || !args[0]->IsExternal()) {
    return;
  }

  assert(args.Length() == 1);
  assert(args[0]->IsExternal());

  v8::External *val = v8::External::Cast(*args[0]);
  void *fd = (void *)val->Value();

  uv_fs_t req;
  int ret = uv_fs_close(loop, &req, static_cast<uv_file>(reinterpret_cast<uint64_t>(fd)), NULL);

  if (ret < 0) {
    return;
  }

  args.GetReturnValue().Set(1);
}

static void uvTimerNew(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().SetNull();

  if (args.Length() != 0) {
    return;
  }

  uv_timer_t *timer = new uv_timer_t();
  uv_timer_init(loop, timer);

  args.GetReturnValue().Set(v8::External::New(isolate, (void *)timer));
}

struct cbData
{
  Isolate *mIsolate;
  v8::Persistent<v8::Context> mContext;
  v8::Persistent<v8::Function> mFunc;
};

static void v8TimerCallback(uv_timer_t* handle)
{
  cbData *data = (cbData *)handle->data;

  Local<v8::Context> ctx = PersistentToLocal(data->mIsolate, data->mContext);
  Local<v8::Function> func = PersistentToLocal(data->mIsolate, data->mFunc);
  Local<v8::Value> argv[1] = { func.As<Value>() };

  func->Call(ctx, argv[0], 1, argv);
}

static void uvTimerStart(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().Set(-1);

  if (args.Length() != 4 || !args[0]->IsExternal()) {
    return;
  }

  assert(args.Length() == 4);
  assert(args[0]->IsExternal());
  assert(args[3]->IsFunction());

  v8::External *val = v8::External::Cast(*args[0]);
  uv_timer_t *handle = (uv_timer_t *)val->Value();
  int timeout = (int)args[1]->ToInteger()->Value();
  int repeat = (int)args[2]->ToInteger()->Value();

  v8::Persistent<v8::Function> func(isolate, Local<v8::Function>::Cast(args[3]));

  cbData *data = new cbData();

  handle->data = data;
  data->mIsolate = isolate;
  data->mContext.Reset(isolate, isolate->GetCurrentContext());
  data->mFunc.Reset(isolate, Local<v8::Function>::Cast(args[3]));

  uv_timer_start(handle, &v8TimerCallback, timeout, repeat);

  args.GetReturnValue().Set(1);
}

static void uvTimerStop(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().Set(-1);

  if (args.Length() != 1 || !args[0]->IsExternal()) {
    return;
  }

  assert(args.Length() == 1);
  assert(args[0]->IsExternal());

  v8::External *val = v8::External::Cast(*args[0]);
  uv_timer_t *handle = (uv_timer_t *)val->Value();

  uv_timer_stop(handle);

  args.GetReturnValue().Set(1);
}

static std::string v8ReadFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;

  src_script << src_file.rdbuf(); // slurp up file

  std::string s = src_script.str();

  return s;
}

static void uvRunInContext(const v8::FunctionCallbackInfo<v8::Value>& args)
{
  Isolate* isolate = args.GetIsolate();
  HandleScope scope(isolate);

  uv_loop_t *loop = getEventLoopFromArgs(args);

  args.GetReturnValue().SetNull();

  /*if (args.Length() != 1) {
    return;
  }*/

  assert(args.Length() >= 1);
  assert(args[0]->IsString());

  rtString sourceCode = toString(args[0]->ToString());

  Locker                locker(isolate);
  Isolate::Scope isolate_scope(isolate);
  HandleScope     handle_scope(isolate);

  Local<Context> local_context = isolate->GetCurrentContext();
  Context::Scope context_scope(local_context);

  TryCatch tryCatch(isolate);
  Local<String> source = String::NewFromUtf8(isolate, sourceCode.cString());
  Local<Script> run_script = Script::Compile(source);
  Local<Value> result = run_script->Run();

  if (tryCatch.HasCaught()) {
    String::Utf8Value trace(tryCatch.StackTrace());
    rtLogWarn("uvRunInContext: '%s'", *trace);
    return;
  }

  args.GetReturnValue().Set(result);
}

rtV8FunctionItem v8ModuleBindings[] = {
  { "uv_platform", &uvGetPlatform },
  { "uv_hrtime", &uvGetHrTime },
  { "uv_fs_access", &uvFsAccess },
  { "uv_fs_size", &uvFsGetSize },
  { "uv_fs_open", &uvFsOpen },
  { "uv_fs_read", &uvFsRead },
  { "uv_fs_close", &uvFsClose },
  { "uv_timer_new", &uvTimerNew },
  { "uv_timer_start", &uvTimerStart },
  { "uv_timer_stop", &uvTimerStop },
  { "uv_run_in_context", &uvRunInContext },
  { NULL, NULL },
};

} // namespace
