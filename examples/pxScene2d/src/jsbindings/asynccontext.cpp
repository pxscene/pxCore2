#include "px.h"
#include <rtObjectMacros.h>

using namespace v8;

namespace px
{
  JavaScriptCallback::JavaScriptCallback()
  {
    m_req.data = this;
    m_functionLookup = NULL;
  }

  JavaScriptCallback::~JavaScriptCallback()
  {
    delete m_functionLookup;
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
      args[i] = rt::rt2js(m_args[i]);
    return args;
  }

  void JavaScriptCallback::DoCallback(uv_work_t* req, int status)
  {
    JavaScriptCallback* ctx = reinterpret_cast<JavaScriptCallback *>(req->data);

    Handle<Value>* args = ctx->MakeArgs();

    // TODO: Should this be Local<Function>? 
    Persistent<Function> callbackFunction = ctx->m_functionLookup->Lookup();
    if (!callbackFunction.IsEmpty())
      callbackFunction->Call(Context::GetCurrent()->Global(), static_cast<int>(ctx->m_args.size()), args);

    delete ctx;
    delete [] args;
  }
}

