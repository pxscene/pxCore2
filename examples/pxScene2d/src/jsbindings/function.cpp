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

    v8::Local<v8::ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);

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
