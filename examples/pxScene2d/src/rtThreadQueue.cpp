// rtCore CopyRight 2007-2015 John Robinson
// rtThreadQueue.h

#include "rtThreadQueue.h"

#include "pxTimer.h"

rtThreadQueue::rtThreadQueue(){}
rtThreadQueue::~rtThreadQueue() {}

rtError rtThreadQueue::addTask(rtThreadTaskCB t, void* context, void* data)
{
  mTaskMutex.lock();
  ThreadQueueEntry entry;
  entry.task = t;
  entry.context = context;
  entry.data = data;
  mTasks.push_back(entry);
  mTaskMutex.unlock();

  return RT_OK;
}

rtError rtThreadQueue::removeAllTasksForObject(void* context)
{
  mTaskMutex.lock();
  for(deque<ThreadQueueEntry>::iterator it = mTasks.begin(); 
        it != mTasks.end(); ++it)
  {
    if ((it)->context == context)
    {
      it = mTasks.erase(it);
      break;
    }
  }
  mTaskMutex.unlock();

  return RT_OK;
}

rtError rtThreadQueue::process(double maxSeconds)
{
  bool done = false;
  double start = pxSeconds();
  do
  {
    ThreadQueueEntry entry;
    mTaskMutex.lock();
    if (!mTasks.empty())
    {
      entry = mTasks.front();
      mTasks.pop_front();
    }
    else done = true;
    mTaskMutex.unlock();
    
    if (!done)
    {
      entry.task(entry.context,entry.data);  
      if (maxSeconds > 0)
        done = (pxSeconds()-start) >= maxSeconds;
    }
  } while (!done);

  return RT_OK;
}
