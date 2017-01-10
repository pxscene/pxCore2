#ifndef __RT_REMOTE_CALLBACK_H__
#define __RT_REMOTE_CALLBACK_H__

template<class TFunc>
struct rtRemoteCallback
{
  rtRemoteCallback() 
    : Func(nullptr)
    , Arg(nullptr) { }

  rtRemoteCallback(TFunc func, void* arg) 
    : Func(func),
    Arg(arg) { }

  TFunc Func;
  void* Arg;
};

#endif
