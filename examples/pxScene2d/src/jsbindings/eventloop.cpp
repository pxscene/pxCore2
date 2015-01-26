#include "px.h"
#include <pxEventLoop.h>
#include <pthread.h>

using namespace v8;

namespace
{
  const char* kClassName = "EventLoop";

  pthread_t eventLoopThread;

  void* runEventLoop(void* argp)
  {
    pxEventLoop* eventLoop = reinterpret_cast<pxEventLoop *>(argp);
    eventLoop->run();
    return 0;
  }
}

namespace px
{
  template<typename TWrapper, typename TPXObject> 
  Persistent<Function> WrapperObject<TWrapper, TPXObject>::m_ctor;

  void EventLoop::Export(Handle<Object> exports)
  {
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol(kClassName));
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Local<Template> proto = t->PrototypeTemplate();
    proto->Set(String::NewSymbol("run"), FunctionTemplate::New(Run)->GetFunction());
    proto->Set(String::NewSymbol("exit"), FunctionTemplate::New(Exit)->GetFunction());

    m_ctor = Persistent<Function>::New(t->GetFunction());
    exports->Set(String::NewSymbol(kClassName), m_ctor);
  }

  Handle<Value> EventLoop::Exit(const Arguments& args)
  {
    HandleScope scope;
    unwrap(args)->exit();
    return scope.Close(Undefined());
  }

  Handle<Value> EventLoop::Run(const Arguments& args)
  {
    HandleScope scope;
    pthread_create(&eventLoopThread, NULL, &runEventLoop, unwrap(args));
    return scope.Close(Undefined());
  }

  Handle<Value> EventLoop::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      EventLoop* e = new EventLoop();
      PXPTR(e) = new pxEventLoop();
      e->Wrap(args.This());
      return args.This();
    }
    else
    {
      const int argc = 1;

      HandleScope scope;
      Local<Value> argv[argc] = { args[0] };
      return scope.Close(m_ctor->NewInstance(argc, argv));
    }
  }
}
