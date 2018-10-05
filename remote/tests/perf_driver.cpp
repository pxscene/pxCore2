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

#include <list>
#include <sstream>
#include <unistd.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

#define TEST_NAME "test"

static bool testOver = false;
static std::mutex shutdownMutex;
static FILE* logFile = nullptr;


struct Settings
{
  int NumIterations;
  std::string TestId;
  std::string PathToExe;
  rtLogLevel LogLevel;
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

    timeval_subtract(&duration, &end, &m_start);

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
    dumpResults();
  }

  static void dumpResults()
  {
    rtLogInfo("Test results:" );
    std::list<timeval>* result = m_results[TEST_NAME];
    for (std::list<timeval>::iterator it = result->begin(); it != result->end(); it++)
      rtLogInfo("sec:%ld:  usec:%ld: ", (*it).tv_sec ,(*it).tv_usec);
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
  char* t = strdup(p);
  char* v = strchr(t, '/');
  if (v)
    *v = '\0';

  std::stringstream buff;
  buff << s; 
  free(s);

  buff << '/';
  if (strcmp(t, ".") != 0)
    buff << t;
  free(t);

  return buff.str();
}

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
        if (testOver)
          return RT_OK;
      }

      e = rtRemoteRun(env, wait ? millisecondsFromNow : 16, wait);
      if (e != RT_OK && e != RT_ERROR_QUEUE_EMPTY)
        return e;
    }
  }
  return e;
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
    std::string path = settings->PathToExe;
    if (path[path.size()-1] != '/')
      path += "/";
    if (isClient)
      path += "perf_client";
    else
      path += "perf_server";

    char const* logLevel = rtLogLevelToString(settings->LogLevel);

    rtLogInfo("starting: %s", path.c_str());
    if (isClient)
    {
      char num[8];
      memset(num, 0, sizeof(num));
      snprintf(num, sizeof(num), "%d", settings->NumIterations);

      execl(path.c_str(), path.c_str(), "-i", settings->TestId.c_str(), "-n", num,
        "-l", logLevel , (char *) nullptr);
    }
    else
    {
      execl(path.c_str(), path.c_str(), "-i", settings->TestId.c_str(),
        "-l", logLevel , (char *) nullptr);
    }

    rtLogError("failed to exec:%s. %s", path.c_str(), strerror(errno));
    abort();
  }

  return pid;
}

struct option longOptions[] = 
{
  { "num", required_argument, 0, 'n' },
  { "help", no_argument, 0, 'h' },
  { "log-level", required_argument, 0, 'l' },
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

  rtLogLevel logLevel = RT_LOG_INFO;

  while (true)
  {
    int c = getopt_long(argc, argv, "n:h:l:", longOptions, &optionIndex);
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

      case 'l':
        logLevel = rtLogLevelFromString(optarg);
        break;

      default:
        rtLogError("invalid argument");
        abort();
        break;
    }
  }

  int status;

  settings.PathToExe = GetPathToExecutable(argv[0]);
  settings.TestId = GetNextTestId();
  settings.LogLevel = logLevel;

  char logFileName[256];
  snprintf(logFileName, sizeof(logFileName), "%s.driver.log", settings.TestId.c_str());
  logFile = fopen(logFileName, "w");

  rtLogSetLevel(logLevel);
  rtLogSetLogHandler(logFileWriter);

  rtPerformanceCounter  testrun(TEST_NAME);
  pid_t serverPid = StartChildProcess(&settings, false);
  pid_t clientPid = StartChildProcess(&settings, true);

  rtLogInfo("waiting for server:%d", static_cast<int>(serverPid));
  waitpid(serverPid, &status, 0);
  rtLogInfo("server pid has exited:%d", static_cast<int>(serverPid));

  rtLogInfo("waiting for client:%d", static_cast<int>(clientPid));
  waitpid(clientPid, &status, 0);
  rtLogInfo("client pid has exited:%d", static_cast<int>(clientPid));

  rtLogInfo("returning from main");

  if (logFile)
    fclose(logFile);

  return 0;
}
