#ifndef RT_THREAD_TASK_H
#define RT_THREAD_TASK_H

class rtThreadTask
{  
public:
    rtThreadTask(void (*functionPointer)(void*), void* data);
    ~rtThreadTask();
    void execute();
    
private:
    void (*mFunctionPointer)(void*);
    void* mData;
};

#endif //RT_THREAD_TASK_H