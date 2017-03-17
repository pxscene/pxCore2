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

class rtEcho : public rtObject
{
	rtDeclareObject(rtEcho, rtEcho);
	rtProperty(message, getMessage, setMessage, rtString);
	rtProperty(onMessageChanged, getOnMessageChanged, setOnMessageChanged, rtFunctionRef);

public:
	rtError getMessage(rtString& s) const
		{ s = m_msg; return RT_OK; }

	rtError setMessage(rtString const& s)
	{
		rtError e = RT_OK;
		m_msg = s;
		if (m_func)
		{
			e = m_func.send(s);
			if (e != RT_OK)
				rtLogError("failed to notify of message changed. %s", rtStrError(e));
		}
		return RT_OK;
	}

	rtError getOnMessageChanged(rtFunctionRef& func) const
		{ func = m_func; return RT_OK; }

	rtError setOnMessageChanged(rtFunctionRef const& func)
		{ m_func = func; return RT_OK; }

	static rtError handleMessageChanged(int /*argc*/, rtValue const* argv, rtValue* /*result*/, void* /*argp*/)
	{
		rtString s = argv[0].toString();
		rtLogInfo("message has changed: %s", s.cString());
		return RT_OK;
	}

private:
	rtString m_msg;
	rtFunctionRef m_func;
};



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
	unsigned int count=100;

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
				count = atoi(optarg);
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

	e = server.set("onMessageChanged", new rtFunctionCallback(rtEcho::handleMessageChanged));
		if (e != RT_OK)
			rtLogError("failed to set message handler: %s", rtStrError(e));


	for (unsigned int j = 0; j < count; ++j)
	{
		while ((e = rtRemoteLocateObject(env, testId.c_str(), server)) != RT_OK)
		{
			rtLogInfo("failed to find %s:%s\n",testId.c_str(), rtStrError(e));
		}
		e = server.set("num", j);
		rtLogInfo("set:%d", j);

		uint32_t n = server.get<uint32_t>("num");
		rtLogInfo("get:%d", n);
		// RT_ASSERT(n == static_cast<uint32_t>(j));
	}

  e = server.send("shutdown");
  rtLogInfo("shutting down server: %s/%s", rtStrError(e), testId.c_str());

  if (logFile)
    fclose(logFile);

	return 0;
}
