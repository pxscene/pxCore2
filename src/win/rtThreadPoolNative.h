#ifndef RT_THREAD_POOL_NATIVE_H
#define RT_THREAD_POOL_NATIVE_H

#include "../rtMutex.h"
#include "../rtThreadTask.h"

#include <vector>
#include <deque>

class rtThreadPoolNative
{
public:
  rtThreadPoolNative(int numberOfThreads);
  ~rtThreadPoolNative();

  void executeTask(rtThreadTask* threadTask);
  void startThread();

protected:

  bool initialize();
  void destroy();

  int mNumberOfThreads;
  bool mRunning;
  rtMutex mThreadTaskMutex;
  rtThreadCondition mThreadTaskCondition;
  std::vector<HANDLE> mThreads;
  std::deque<rtThreadTask*> mThreadTasks;
};

#endif //RT_THREAD_POOL_H