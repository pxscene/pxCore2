#include "px.h"

using namespace v8;

namespace
{
  Handle<Value> toJs(px::JavaScriptCallback::Argument::ArgType type,
    px::JavaScriptCallback::Argument::ArgData data)
  {
    switch (type)
    {
      case px::JavaScriptCallback::Argument::AT_I2:
        return Integer::New(data.sVal);
        break;
      case px::JavaScriptCallback::Argument::AT_I4:
        return Integer::New(data.iVal);
        break;
      case px::JavaScriptCallback::Argument::AT_UL:
        return Integer::NewFromUnsigned(data.ulVal);
        break;
      default:
        // TODO: FAIL
        assert(false);
        break;
    }
  }
}


namespace px
{
  JavaScriptCallback::JavaScriptCallback()
  {
    m_req.data = this;
  }

  JavaScriptCallback::~JavaScriptCallback()
  {
  }

  void JavaScriptCallback::Enqueue()
  {
    // TODO: Extensive error checking here
    uv_queue_work(uv_default_loop(), &m_req, &Work, &DoCallback);
  }

  void JavaScriptCallback::Work(uv_work_t* req)
  {
    // empty
  }

  Handle<Value>* JavaScriptCallback::MakeArgs()
  {
    Handle<Value>* args = new Handle<Value>[m_args.size()];
    for (size_t i = 0; i < m_args.size(); ++i)
      args[i] = toJs(m_args[i].Type, m_args[i].Data);
    return args;
  }

  void JavaScriptCallback::DoCallback(uv_work_t* req, int status)
  {
    JavaScriptCallback* ctx = reinterpret_cast<JavaScriptCallback *>(req->data);

    Handle<Value>* args = ctx->MakeArgs();

    // TODO: Should this be Local<Function>? 
    //Persistent<Function> callbackFunction = ctx->m_functionLookup->Lookup();
    Persistent<Function>* callbackFunction = ctx->m_callback;
    if (!callbackFunction->IsEmpty())
    {
      TryCatch tryCatch;
      (*callbackFunction)->Call(Context::GetCurrent()->Global(), static_cast<int>(ctx->m_args.size()), args);
    }

    delete ctx;
    delete [] args;
  }
}

