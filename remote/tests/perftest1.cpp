#include <rtRemote.h>
#include <rtRemoteConfig.h>
#include <rtRemoteEnvironment.h>
#include <rtGuid.h>

#include <list>
#include <sstream>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>

struct Settings
{
  int NumIterations;
  std::string TestId;
  std::string PathToExe;
};

class rtPerformanceCounter
{
  using ResultMap = std::map< std::string, std::list<timeval>* >;

public:
  rtPerformanceCounter(char const* s)
    : m_name(s)
  {
    gettimeofday(&m_start, 0);
  }

  ~rtPerformanceCounter()
  {
    timeval end;
    gettimeofday(&end, 0);

    timeval duration;
    memset(&duration, 0, sizeof(timeval));

    timeval_subtract(&duration, &m_start, &end);

    std::list<timeval>* results = nullptr;

    auto itr = m_results.find(m_name);
    if (itr == m_results.end())
    {
      results = new std::list<timeval>();
      m_results.insert(ResultMap::value_type(m_name, results));
    }
    else
    {
      results = itr->second;
    }

    results->push_back(duration);
  }

  static void dumpResults()
  {
    // TODO
  }

private:
  // https://www.gnu.org/software/libc/manual/html_node/Elapsed-Time.html
  int
  timeval_subtract (struct timeval *result, struct timeval *x, struct timeval *y)
  {
    /* Perform the carry for the later subtraction by updating y. */
    if (x->tv_usec < y->tv_usec) {
      int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
      y->tv_usec -= 1000000 * nsec;
      y->tv_sec += nsec;
    }
    if (x->tv_usec - y->tv_usec > 1000000) {
      int nsec = (x->tv_usec - y->tv_usec) / 1000000;
      y->tv_usec += 1000000 * nsec;
      y->tv_sec -= nsec;
    }

    /* Compute the time remaining to wait.
       tv_usec is certainly positive. */
    result->tv_sec = x->tv_sec - y->tv_sec;
    result->tv_usec = x->tv_usec - y->tv_usec;

    /* Return 1 if result is negative. */
    return x->tv_sec < y->tv_sec;
  }

private:
  std::string m_name;
  timeval m_start;
  static ResultMap m_results;
};


rtPerformanceCounter::ResultMap rtPerformanceCounter::m_results;

bool testOver = false;
std::mutex shutdownMutex;

std::string
GetNextTestId()
{
  rtGuid testId = rtGuid::newRandom();
  std::stringstream buff;
  buff << "test.object.";
  buff << testId.toString();
  return buff.str();
}

std::string
GetPathToExecutable(char const* p)
{
  RT_ASSERT(p != nullptr);
  RT_ASSERT(strlen(p) > 0);

  char* s = getcwd(nullptr, 0);

  std::stringstream buff;
  buff << s; 
  free(s);

  buff << '/';
  if (strncmp(p, "./", 2) == 0)
    p += 2;

  buff << p;
  return buff.str();
}

rtError
rtRemoteRunUntil(rtRemoteEnvironment* env, uint32_t millisecondsFromNow)
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
        if (testOver)
          return RT_OK;
      }

      e = rtRemoteRun(env, 16);
      if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
        return e;
    }
  }
  return e;
}



void
RunServer(rtRemoteEnvironment* env, Settings* settings)
{
  rtLogInfo("%s running server", settings->TestId.c_str());

  rtObjectRef obj(new rtTestObject());
  rtError e = rtRemoteRegisterObject(env, settings->TestId.c_str(), obj);
  RT_ASSERT(e == RT_OK);

  while (true)
  {
    {
      std::unique_lock<std::mutex> lock(shutdownMutex);
      if (testOver)
      {
        rtLogInfo("server shutting down");
        return;
      }
    }

    e = rtRemoteRunUntil(env, 1000);
    rtLogInfo("[%s] rtRemoteRun:%s", settings->TestId.c_str(), rtStrError(e));
  }
}

pid_t
StartChildProcess(Settings* settings, bool isClient)
{
  pid_t pid = fork();
  if (pid == -1)
  {
    rtLogError("failed to invoke fork(). %s", rtStrError(errno));
    abort();
  }

  if (pid == 0)
  {
    if (isClient)
    {
      execl(settings->PathToExe.c_str(), settings->PathToExe.c_str(), "--client", "-i",
        settings->TestId.c_str(), 0);
      rtLogInfo("failed to execute client. %s", rtStrError(errno));
    }
    else
    {
      execl(settings->PathToExe.c_str(), settings->PathToExe.c_str(), "--server", "-i",
        settings->TestId.c_str(), 0);

      int e = errno;
      rtLogInfo("failed to execute server. (%d) %s", e, rtStrError(e));
    }
    abort();
  }

  return pid;
}

void
RunClient(rtRemoteEnvironment* env, Settings* settings)
{
  rtLogInfo("%s running client", settings->TestId.c_str());

  rtObjectRef obj;
  rtError e = RT_OK;

  rtLogInfo("locating remote object:%s", settings->TestId.c_str());
  {
    rtPerformanceCounter("locate");
    e = rtRemoteLocateObject(env, settings->TestId.c_str(), obj);
  }

  if (e != RT_OK)
    rtLogInfo("failed to locate object. %s", rtStrError(e));

  RT_ASSERT(e == RT_OK);
  RT_ASSERT(obj != nullptr);

  uint32_t num = 1000;
  for (int i = 0; i < settings->NumIterations; ++i)
  {
    {
      std::unique_lock<std::mutex> lock(shutdownMutex);
      if (testOver)
      {
        rtLogInfo("client shutting down");
        return;
      }
    }

    {
      rtPerformanceCounter("set");
      e = obj.set("num", num);
      RT_ASSERT(e == RT_OK);
    }

    {
      rtPerformanceCounter("get");
      uint32_t num2 = obj.get<uint32_t>("num");
      RT_ASSERT(num2 == num);
    }

    num++;
  }

  // signal server to shutdown
  rtLogInfo("sending shutdown signal to server");
  if (e != RT_OK)
    rtLogWarn("failed to send shutdown signal. %s", rtStrError(e));

  e = obj.send("shutdown");
  RT_ASSERT(e == RT_OK);
}


struct option longOptions[] = 
{
  { "num", required_argument, 0, 'n' },
  { "help", no_argument, 0, 'h' },
  { "test-id", required_argument, 0, 'i' },
  { "client", no_argument, 0, 'c' },
  { "server", no_argument, 0, 's' },
  { 0, 0, 0, 0 }
};


void PrintHelp()
{
  printf("pertest1 [args]\n");
  printf("   read the code\n");
  exit(0);
}


int main(int argc, char* argv[])
{
  Settings settings;
  int optionIndex = 0;
  bool isClient = false;
  bool isServer = false;

  while (true)
  {
    int c = getopt_long(argc, argv, "n:hcsi:", longOptions, &optionIndex);
    if (c == -1)
      break;

    switch (c)
    {
      case 'n':
        settings.NumIterations = static_cast<int>(strtol(optarg, nullptr, 10));
        break;

      case 'h':
        PrintHelp();
        exit(0);
        break;

      case 'c':
        isClient = true;
        break;

      case 's':
        isServer = true;
        break;

      case 'i':
        settings.TestId = optarg;
        break;

      default:
        rtLogError("invalid argument");
        abort();
        break;
    }
  }

  rtLogInfo("is_client:%d is_server:%d", isClient, isServer);

  if (!isClient && !isServer)
  {
    settings.PathToExe = GetPathToExecutable(argv[0]);

    rtLogInfo("current executable:%s", settings.PathToExe.c_str());
    settings.TestId = GetNextTestId();

    pid_t serverPid = StartChildProcess(&settings, false);

    rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
    RT_ASSERT(env != nullptr);

    rtError e = rtRemoteInit(env);
    RT_ASSERT(e == RT_OK);

    // running client in current process
    // not how I want it, but it's easy for now
    RunClient(env, &settings);

    rtLogInfo("waiting for server:%d", static_cast<int>(serverPid));
    int status;
    waitpid(serverPid, &status, 0);
    rtLogInfo("server pid has exited:%d", static_cast<int>(serverPid));
  }

  if (isClient || isServer)
  {
    RT_ASSERT(!settings.TestId.empty());

    rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
    RT_ASSERT(env != nullptr);

    rtError e = rtRemoteInit(env);
    RT_ASSERT(e == RT_OK);

    if (isClient)
    {
      RunClient(env, &settings);
    }

    if (isServer)
    {
      RunServer(env, &settings);
    }

    rtLogInfo("invoking rtRemoteShutdown");
    rtRemoteShutdown(env);
    rtLogInfo("it's done");
  }

  rtLogInfo("returning from main");
  return 0;
}
