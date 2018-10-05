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
#include <rtLog.h>

#include <getopt.h>
#include <unistd.h>

struct option longOptions[] =
{
  { "test-id", required_argument, 0, 'i' },
  { "num-iterations", required_argument, 0, 'n' },
  { "log-level", required_argument, 0, 'l' },
  { 0, 0, 0, 0 }
};

static FILE* logFile = nullptr;

static rtError
messageHandler(int /*argc*/, rtValue const* /*argv*/, rtValue* /*result*/, void* /*argp*/)
{
  // rtString s = argv[0].toString();
  // rtLogInfo("messageHandler:%s", s.cString());
  return RT_OK;
}

static rtError
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
      e = rtRemoteRun(env, wait ? millisecondsFromNow : 16, wait);
      if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
        return e;
    }
  }
  return e;
}

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

int main(int argc, char* argv[])
{

	int optionIndex;
	std::string testId;
  int count = 1000;

	rtError e;
  rtLogLevel logLevel = RT_LOG_INFO;

	while (true)
	{
		int c = getopt_long(argc, argv, "i:n:l:", longOptions, &optionIndex);
		if (c == -1)
			break;

		switch (c)
		{
			case 'i':
				testId = optarg;
				break;
			case 'n':
				count = static_cast<int>(strtol(optarg, nullptr, 10));
				break;
      case 'l':
        logLevel = rtLogLevelFromString(optarg);
        break;
		}
	}

  char logFileName[256];
  snprintf(logFileName, sizeof(logFileName), "%s.client.log", testId.c_str());

  logFile = fopen(logFileName, "w");

  rtLogSetLevel(logLevel);
  rtLogSetLogHandler(logFileWriter);

	rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  rtLogInfo("count:%d", count);
  rtLogInfo("id:%s", testId.c_str());

	e = rtRemoteInit(env);
	RT_ASSERT(e == RT_OK);

	rtObjectRef server;
	e = rtRemoteLocateObject(env, testId.c_str(), server);
	RT_ASSERT(e == RT_OK);


  // run get/set
	for (int i = 0; i < count; ++i)
	{
		while ((e = rtRemoteLocateObject(env, testId.c_str(), server)) != RT_OK)
		{
			rtLogInfo("failed to find %s:%s\n", testId.c_str(), rtStrError(e));
		}

		e = server.set("num", static_cast<uint32_t>(i));
    RT_ASSERT(e == RT_OK);

		// rtLogInfo("set:%d. %s", i, rtStrError(e));

		uint32_t n = server.get<uint32_t>("num");
    static_cast<void>(n);

		// rtLogInfo("get:%d", n);
	}

  // set callback
  e = server.set("onMessage", new rtFunctionCallback(messageHandler));
  if (e != RT_OK)
    rtLogError("failed to set message handler: %s", rtStrError(e));
  else
    rtLogInfo("message handler is set.");


  // signal server to flood our callback
  e = server.send("startCallbackTest");
  RT_ASSERT(e == RT_OK);

  e = server.send("shutdown");
  rtLogInfo("shutting down server: %s/%s", rtStrError(e), testId.c_str());

  time_t startTime = time(nullptr);
  while (true)
  {
    e = remoteRunUntil(env, 1000, false);
    rtLogInfo("[%s] rtRemoteRun:%s", testId.c_str(), rtStrError(e));

    if (time(nullptr) - startTime > 10)
      break;
  }

  if (logFile)
    fclose(logFile);

	return 0;
}
