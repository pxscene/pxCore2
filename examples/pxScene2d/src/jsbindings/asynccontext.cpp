#include "px.h"

using namespace v8;

namespace px
{
  AsyncContext::AsyncContext()
  {
    Request.data = this;
  }

  AsyncContext::~AsyncContext()
  {
  }

  void AsyncContext::EnqueueCallback()
  {
    if (!Callback) return;
    if (Callback->IsEmpty()) return;
    // TODO: Extensive error checking here
    uv_queue_work(uv_default_loop(), &Request, &Work, &DoCallback);
  }

  void AsyncContext::Work(uv_work_t* req)
  {
    // empty
  }

  Handle<Value>* AsyncContext::MakeArgs()
  {
    Handle<Value>* args = new Handle<Value>[Args.size()];

    for (size_t i = 0; i < Args.size(); ++i)
      args[i] = Args[i].ToJavaScript();
    return args;
  }

  void AsyncContext::DoCallback(uv_work_t* req, int status)
  {
    AsyncContext* ctx = reinterpret_cast<AsyncContext *>(req->data);

    Handle<Value>* args = ctx->MakeArgs();

    TryCatch tryCatch;
    (*ctx->Callback)->Call(Context::GetCurrent()->Global(), 
      static_cast<int>(ctx->Args.size()), args);

    delete ctx;
    delete [] args;
  }

  Handle<Value> AsyncContext::Argument::ToJavaScript()
  {
    switch (Type)
    {
      case AT_I4:
        return Number::New(Data.iVal);
        break;

      case AT_UL:
        return Integer::NewFromUnsigned(Data.ulVal);
        break;

      default:
        // TODO: FAIL
        assert(false);
        break;
    }
  }
}

