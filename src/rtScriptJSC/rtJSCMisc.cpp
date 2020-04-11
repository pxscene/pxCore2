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

#include "rtJSCMisc.h"

#include "rtLog.h"

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>

#if defined(USE_GLIB)
#include <glib.h>
#endif

#if defined(USE_UV) || defined(RTSCRIPT_SUPPORT_V8)
#include <uv.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
  JS_EXPORT void JSRemoteInspectorStart(void);
  JS_EXPORT void JSRemoteInspectorSetLogToSystemConsole(bool logToSystemConsole);
  JS_EXPORT void JSRemoteInspectorSetInspectionEnabledByDefault(bool);
#ifdef __cplusplus
}
#endif

namespace WTF {
void initializeMainThread();
};

namespace JSC {
void initializeThreading();
};

namespace RtJSC {

static void dispatchPending();
static void dispatchTimeouts();

#if defined(USE_GLIB)
static GMainLoop *gMainLoop = nullptr;
#endif

static std::thread::id gMainThreadId;

void initMainLoop()
{
  WTF::initializeMainThread();
  JSC::initializeThreading();
#if defined(USE_GLIB)
  if (!gMainLoop && g_main_depth() == 0) {
    gMainLoop = g_main_loop_new (nullptr, false);
  }
#endif
  // JSRemoteInspectorStart();
  // JSRemoteInspectorSetInspectionEnabledByDefault(false);
  JSRemoteInspectorSetLogToSystemConsole(true);
  gMainThreadId = std::this_thread::get_id();
}

void pumpMainLoop()
{
  static bool isPumping = false;
  if (isPumping)
    return;
  isPumping = true;

  dispatchPending();
  dispatchTimeouts();

#if defined(USE_UV) || defined(RTSCRIPT_SUPPORT_V8)
  uv_run(uv_default_loop(), UV_RUN_NOWAIT);
#endif

#if defined(USE_GLIB)
  if (gMainLoop && g_main_depth() == 0) {
    gboolean ret;
    do {
      ret = g_main_context_iteration(nullptr, false);
    } while(ret);
  }
#endif
  
  //dispatchPending();
  isPumping = false;
}

static std::list<std::function<void ()>> gPendingFun;
static std::mutex gDispatchMutex;

void printException(JSContextRef ctx, JSValueRef exception)
{
  JSStringRef exceptStr = JSValueToStringCopy(ctx, exception, nullptr);
  rtString errorStr = jsToRtString(exceptStr);
  JSStringRelease(exceptStr);
  rtLogError("Got Exception: %s", errorStr.cString());
}

rtString jsToRtString(JSStringRef str)
{
  if (!str)
    return rtString();
  size_t len = JSStringGetMaximumUTF8CStringSize(str);
  std::unique_ptr<char[]> buffer(new char[len]);
  len = JSStringGetUTF8CString(str, buffer.get(), len);
  return rtString(buffer.get(), len); // does a copy
}

std::string readFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;
  src_script << src_file.rdbuf();
  return src_script.str();
}

std::vector<uint8_t> readBinFile(const char *file)
{
  std::ifstream ifs {file, std::ios::binary|std::ios::ate };

  std::ifstream::pos_type len = ifs.tellg();
  ifs.seekg(0, std::ios::beg);

  std::vector<uint8_t> contents;
  contents.reserve(len);
  contents.resize(len);

  ifs.read(reinterpret_cast<char*>(&contents[0]), len);
  return contents;
}

bool fileExists(const char* name)
{
  struct stat buffer;
  bool ret = (stat (name, &buffer) == 0);
  return ret;
}

static void dispatchPending()
{
  std::unique_lock<std::mutex> lock(gDispatchMutex);
  std::list<std::function<void ()>> pending = std::move(gPendingFun);
  gDispatchMutex.unlock();
  for(auto& fun : pending)
    fun();
}

void dispatchOnMainLoop(std::function<void ()>&& fun)
{
  std::unique_lock<std::mutex> lock(gDispatchMutex);
  gPendingFun.push_back(std::move(fun));
}

struct TimeoutInfo
{
  std::function<int ()> callback;
  std::chrono::time_point<std::chrono::steady_clock> fireTime;
  std::chrono::milliseconds interval;
  bool repeat;
  uint32_t tag;
  bool canceled;
};

struct TimeoutInfoComparator
{
  constexpr bool operator()(const TimeoutInfo *lhs, const TimeoutInfo *rhs) const {
    return !((lhs->fireTime < rhs->fireTime) ||
             ((lhs->fireTime == rhs->fireTime) && (lhs->tag < rhs->tag)));
  }
};

class TimeoutQueue : public std::priority_queue<TimeoutInfo*, std::vector<TimeoutInfo*>, TimeoutInfoComparator>
{
public:
  void pushTimeouts(const std::vector<TimeoutInfo*>& timerVec)
  {
    if (!timerVec.size())
      return;
    c.reserve(c.size() + timerVec.size());
    c.insert(c.end(),timerVec.begin(), timerVec.end());
    std::make_heap(c.begin(), c.end(), comp);
  }
  bool updateForInfo(const TimeoutInfo* info)
  {
    auto it = std::find(c.begin(), c.end(), info);
    if (it != c.end()) {
      std::make_heap(c.begin(), c.end(), comp);
      return true;
    }
    return false;
  }
};

static std::map<uint32_t, TimeoutInfo*> gTimeoutMap;
static TimeoutQueue gTimeoutQueue;
static uint32_t gTimeoutIdx = 0;

uint32_t installTimeout(double intervalMs, bool repeat, std::function<int ()>&& fun)
{
  if (intervalMs < 0)
    intervalMs = 0;

  auto currentTime = std::chrono::steady_clock::now();
  auto interval = std::chrono::milliseconds(static_cast<uint32_t>(intervalMs));

  TimeoutInfo *info = new TimeoutInfo;
  info->interval = interval;
  info->fireTime = currentTime + info->interval;
  info->repeat = repeat;
  info->callback = std::move(fun);
  info->canceled = false;
  info->tag = ++gTimeoutIdx;  // FIXME:

  gTimeoutMap[info->tag] = info;
  gTimeoutQueue.push(info);
  return info->tag;
}

void clearTimeout(uint32_t tag)
{
  auto it = gTimeoutMap.find(tag);
  if (it != gTimeoutMap.end()) {
    TimeoutInfo* info = it->second;
    constexpr auto steadyMinimal = std::chrono::steady_clock::time_point::min();
    info->fireTime = steadyMinimal;
    info->canceled = true;
    gTimeoutMap.erase(it);
    gTimeoutQueue.updateForInfo(info);
  }
}

static void dispatchTimeouts()
{
  const auto currentTime = std::chrono::steady_clock::now();

  std::vector<TimeoutInfo*> timeoutsToRepeat;
  while(!gTimeoutQueue.empty()) {
    TimeoutInfo* info = gTimeoutQueue.top();
    if (!info->canceled && info->fireTime > currentTime) {
      break;
    }
    gTimeoutQueue.pop();
    if (info->canceled) {
      delete info;
      continue;
    }
    int rc = info->callback();
    if (rc != 0 || !info->repeat || info->canceled) {
      if (!info->canceled)
        gTimeoutMap.erase(info->tag);
      delete info;
      continue;
    }
    info->fireTime = currentTime + info->interval;
    timeoutsToRepeat.push_back(info);
  }
  gTimeoutQueue.pushTimeouts(timeoutsToRepeat);
}

void assertIsMainThread()
{
  assert(std::this_thread::get_id() == gMainThreadId);
}

}  // RtJSC
