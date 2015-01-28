#ifndef PX_THREAD_TASK_H
#define PX_THREAD_TASK_H

class pxThreadTask
{  
public:
    pxThreadTask(void (*functionPointer)(void*), void* data);
    ~pxThreadTask();
    void execute();
    
private:
    void (*mFunctionPointer)(void*);
    void* mData;
};

#endif //PX_THREAD_TASK_H