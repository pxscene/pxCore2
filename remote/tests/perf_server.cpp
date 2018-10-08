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

#include <rtRemote.h>
#include <rtRemoteConfig.h>
#include <rtRemoteEnvironment.h>
#include <rtGuid.h>

#include <mutex>
#include <getopt.h>
#include <unistd.h>

static std::mutex shutdownMutex;
static bool testIsOver = false;

struct option longOptions[] = 
{
  { "test-id", required_argument, 0, 'i' },
  { "log-level", required_argument, 0, 'l' },
  { 0, 0, 0, 0 }
};

static FILE* logFile = nullptr;

void
logFileWriter(rtLogLevel level, const char* path, int line, int threadId, char* message)
{
  if (logFile)
  {
    char const* logLevel = rtLogLevelToString(level);
    fprintf(logFile, "%5s %s:%d -- Thread-%" RT_THREADID_FMT ": %s\n", logLevel, path, line,
        threadId, message);
  }
}

class rtTestObject : public rtObject
{
public:
  rtTestObject()
    : m_callbackTestRunning(false)
  {
  }

  rtDeclareObject(rtTestObject, rtObject);
  rtProperty(num, num, setNum, uint32_t);
  rtProperty(onMessage, onMessage, setOnMessage, rtFunctionRef);
  rtMethodNoArgAndNoReturn("shutdown", shutdown);
  rtMethodNoArgAndNoReturn("startCallbackTest", startCallbackTest);
  rtMethodNoArgAndNoReturn("stopCallbackTest", stopCallbackTest);

  uint32_t num() const { return m_num; }
  rtError  num(uint32_t& n) const { n = m_num; return RT_OK; }
  rtError  setNum(uint32_t n) { m_num = n; return RT_OK; }

  rtError onMessage(rtFunctionRef& func) const
    { func = m_func; return RT_OK; }

  rtError setOnMessage(rtFunctionRef const& func)
    { m_func = func; return RT_OK; }

  rtError shutdown()
  {
    rtLogInfo("got shutdown signal");
    std::unique_lock<std::mutex> lock(shutdownMutex);
    testIsOver = true;
    return RT_OK;
  }

  rtError startCallbackTest()
  {
    rtLogInfo("starting callback test");
    m_callbackThread.reset(new std::thread(&rtTestObject::floodCallback, this));
    return RT_OK;
  }

  rtError stopCallbackTest()
  {
    rtLogInfo("stopping callback test");
    return RT_OK;
  }

private:
  void floodCallback()
  {
    rtString arg;

    static int const kTestLogMessageLength = 250;
    for (int i = 0; i < kTestLogMessageLength; ++i)
      arg.append("x");

    while (true)
    {
      {
        std::unique_lock<std::mutex> lock(shutdownMutex);
        if (testIsOver)
        {
          rtLogInfo("exiting floodCallback");
          return;
        }
      }

      RT_ASSERT(m_func);

      rtError e = m_func.send(arg);
      RT_ASSERT(e == RT_OK);
    }
  }

private:
  uint32_t m_num;
  bool m_callbackTestRunning;
  std::unique_ptr<std::thread> m_callbackThread;
  rtFunctionRef m_func;
};

rtDefineObject(rtTestObject, rtObject);
rtDefineProperty(rtTestObject, num);
rtDefineProperty(rtTestObject, onMessage);
rtDefineMethod(rtTestObject, shutdown);

rtError
remoteRunUntil(rtRemoteEnvironment* env, uint32_t millisecondsFromNow, bool wait)
{
  rtError e = RT_OK;

  bool hasDipatchThread = env->Config->server_use_dispatch_thread();
  if (hasDipatchThread)
  {
    usleep(millisecondsFromNow * 1000);
    (void ) env;
  }
  else
  {
    auto endTime = std::chrono::milliseconds(millisecondsFromNow) + std::chrono::system_clock::now();
    while (endTime > std::chrono::system_clock::now())
    {
      {
        std::unique_lock<std::mutex> lock(shutdownMutex);
        if (testIsOver)
          return RT_OK;
      }

      e = rtRemoteRun(env, wait ? millisecondsFromNow : 16, wait);
      if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
        return e;
    }
  }
  return e;
}

int main(int argc, char* argv[])
{
  int optionIndex;
  std::string testId;

  rtError e;
  rtLogLevel logLevel = RT_LOG_INFO;

  while (true)
  {
    int c = getopt_long(argc, argv, "i:l:", longOptions, &optionIndex);
    if (c == -1)
      break;

    switch (c)
    {
      case 'i':
        testId = optarg;
        break;
      case 'l':
        logLevel = rtLogLevelFromString(optarg);
        break;
    }
  }

  char logFileName[256];
  snprintf(logFileName, sizeof(logFileName), "%s.server.log", testId.c_str());
  logFile = fopen(logFileName, "w");

  rtLogSetLevel(logLevel);
  rtLogSetLogHandler(logFileWriter);

  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();

  rtObjectRef obj(new rtTestObject());
  e = rtRemoteInit(env);
  RT_ASSERT(e == RT_OK);
  e = rtRemoteRegisterObject(env, testId.c_str(), obj);
  RT_ASSERT(e == RT_OK);

  bool running = true;
  while (running)
  {
    {
      std::unique_lock<std::mutex> lock(shutdownMutex);
      running = !testIsOver;
    }
    
    if (running)
    {
      e = remoteRunUntil(env, 1000, false);
      rtLogInfo("[%s] rtRemoteRun:%s", testId.c_str(), rtStrError(e));
    }
  }

  rtLogInfo("server shutting down for %s", testId.c_str());

  if (logFile)
    fclose(logFile);

  return 0;
}
