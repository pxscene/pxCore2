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

// rtThreadQueue.h
#ifndef RT_THREAD_QUEUE_H
#define RT_THREAD_QUEUE_H

#include "rtError.h"
#include "rtMutex.h"

#include <deque>

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

  rtError removeAllTasksForObject(void* context);

  // Invoke this method periodically on the dispatching (owning) thread
  // maxSeconds=0 means process until empty
  rtError process(double maxSeconds = 0);

private:
  std::deque<ThreadQueueEntry> mTasks;
  rtMutex mTaskMutex;
};
#endif //RT_THREAD_QUEUE_H
