// rtCore CopyRight 2007-2015 John Robinson
// rtThreadQueue.h
#ifndef RT_THREAD_QUEUE_H
#define RT_THREAD_QUEUE_H

#include "rtError.h"
#include "rtMutex.h"

#include <deque>
using namespace std;

typedef void (*rtThreadTaskCB)(void* context, void* data);

struct ThreadQueueEntry
{
  rtThreadTaskCB task;
  void* context;
  void* data;
};

class rtThreadQueue
{
public:
  rtThreadQueue();
  ~rtThreadQueue();

  // Queue a task for execution on a thread.
  // Thread safe
  rtError addTask(rtThreadTaskCB t, void* context, void* data);

  // Invoke this method periodically on the dispatching (owning) thread
  // maxSeconds=0 means process until empty
  rtError process(double maxSeconds = 0);

private:
  deque<ThreadQueueEntry> mTasks;
  rtMutex mTaskMutex;
};
#endif //RT_THREAD_QUEUE_H
