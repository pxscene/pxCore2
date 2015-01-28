#include "px.h"
#include <pxOffscreen.h>
#include <string>

using namespace v8;

namespace
{
  const char* kClassName = "Offscreen";
}

namespace px
{
  Persistent<Function> Offscreen::m_ctor;

  void Offscreen::Export(Handle<Object> exports)
  {
    Local<FunctionTemplate> t = FunctionTemplate::New(New);
    t->SetClassName(String::NewSymbol(kClassName));
    t->InstanceTemplate()->SetInternalFieldCount(1);

    Local<Template> proto = t->PrototypeTemplate();
    proto->Set(String::NewSymbol("init"), FunctionTemplate::New(Init)->GetFunction());
    proto->Set(String::NewSymbol("initWithColor"), FunctionTemplate::New(InitWithColor)->GetFunction());

    m_ctor = Persistent<Function>::New(t->GetFunction());
    exports->Set(String::NewSymbol(kClassName), m_ctor);
  }

  Handle<Value> Offscreen::Init(const Arguments& args)
  {
    HandleScope scope;
    unwrap(args)->init(toInt32(args, 0), toInt32(args, 1));
    return scope.Close(Undefined());
  }

  Handle<Value> Offscreen::InitWithColor(const Arguments& args)
  {
    HandleScope scope;

    String::Utf8Value s(args[2]->ToString());
    std::string name(*s);

    // TODO: Color is a lot of work -- just hard code for now

    unwrap(args)->initWithColor(toInt32(args, 0), toInt32(args, 1),  pxGreen);
    return scope.Close(Undefined());
  }

  Handle<Value> Offscreen::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      Offscreen* w = new Offscreen();
      PXPTR(w) = new pxOffscreen();
      w->Wrap(args.This());
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

