#include "px.h"
#include "pxScene2d.h"

namespace
{
  const char* kClassName = "Function";
}

namespace rt
{
  v8::Persistent<v8::Function> Function::m_ctor;

  void rt::Function::Export(v8::Handle<v8::Object> exports)
  {
    v8::Local<v8::FunctionTemplate> tmpl = v8::FunctionTemplate::New(New);
    tmpl->SetClassName(v8::String::NewSymbol(kClassName));

    v8::Local<v8::Template> proto = tmpl->PrototypeTemplate();
    proto->Set(v8::String::NewSymbol("call"), v8::FunctionTemplate::New(Invoke)->GetFunction());

    v8::Local<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);
    inst->SetCallAsFunctionHandler(Invoke);

    m_ctor = v8::Persistent<v8::Function>::New(tmpl->GetFunction());
    exports->Set(v8::String::NewSymbol(kClassName), m_ctor);
  }

  v8::Handle<v8::Object> rt::Function::New(const rtFunctionRef& function)
  {
    v8::HandleScope scope;
    v8::Local<v8::Value> argv[1] = { v8::External::New(function.getPtr()) };
    v8::Local<v8::Object> obj = m_ctor->NewInstance(1, argv);
    return scope.Close(obj);
  }

  v8::Handle<v8::Value> rt::Function::Invoke(const v8::Arguments& args)
  {
    v8::HandleScope scope;

    std::vector<rtValue> argList;
    for (int i = 0; i < args.Length(); ++i)
      argList.push_back(js2rt(args[i]));

    rtLogDebug("Invoking function");

    rtValue result;
    rtError err = unwrap(args)->Send(args.Length(), &argList[0], &result);
    if (err != RT_OK)
    {
      rtLogFatal("failed to invoke function: %d", err);
      abort();
    }

    return scope.Close(rt2js(result));
  }

  v8::Handle<v8::Value> rt::Function::New(const v8::Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      rtIFunction* p = reinterpret_cast<rtIFunction *>(v8::Local<v8::External>::Cast(args[0])->Value());
      rt::Function* func = new rt::Function(p);
      func->Wrap(args.This());
      return args.This();
    }
    else
    {
      const int argc = 1;

      v8::HandleScope scope;
      v8::Local<v8::Value> argv[argc] = { args[0] };
      return scope.Close(m_ctor->NewInstance(argc, argv));
    }
  }
}
