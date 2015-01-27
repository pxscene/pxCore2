#include "px.h"

#define PX_PLATFORM_X11
#include <pxScene2d.h>

using namespace v8;

namespace px 
{ 
namespace scene
{
  Persistent<Function> Scene2d::m_ctor;

  void Scene2d::Export(Handle<Object> exports)
  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(New);
    tmpl->SetClassName(String::NewSymbol("Scene2d"));
    tmpl->InstanceTemplate()->SetInternalFieldCount(1);

    // t->ProtoTypeTemplate()->Set(String::NewSymbol(

    BaseObject::Inherit(tmpl);

    m_ctor = Persistent<Function>::New(tmpl->GetFunction());
    exports->Set(String::NewSymbol("Scene2d"), m_ctor);
  }

  Handle<Value> Scene2d::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      Scene2d* obj = new Scene2d();
      obj->Wrap(args.This());
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

  #if 0
  Handle<Value> Scene2d::GetRoot(const Arguments& args)
  {
  }
  #endif

} // namespac scene
} // namespace px

