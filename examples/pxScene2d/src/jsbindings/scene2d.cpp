#include "px.h"
#include <pxScene2d.h>

using namespace v8;

namespace
{
  const char* kClassName = "Scene2d";
}

namespace px 
{ 
namespace scene
{
  Persistent<Function> Scene2d::m_ctor;

  void Scene2d::Export(Handle<v8::Object> exports)
  {
    Local<FunctionTemplate> tmpl = FunctionTemplate::New(New);
    tmpl->SetClassName(String::NewSymbol(kClassName));

    Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
    inst->SetInternalFieldCount(1);

    rt::Object::Inherit(tmpl);

    m_ctor = Persistent<Function>::New(tmpl->GetFunction());
    exports->Set(String::NewSymbol(kClassName), m_ctor);
  }

  Handle<v8::Object> Scene2d::New(const pxScene2dRef& scene)
  {
    HandleScope scope;
    Local<Value> argv[1] = { External::New(scene.getPtr()) };
    Local<v8::Object> obj = m_ctor->NewInstance(1, argv);
    return scope.Close(obj);
  }

  Handle<Value> Scene2d::New(const Arguments& args)
  { 
    if (args.IsConstructCall())
    {
      pxScene2d* s = reinterpret_cast<pxScene2d *>(Local<External>::Cast(args[0])->Value());
      Scene2d* obj = new Scene2d(s);
      obj->Wrap(args.This());
      return args.This();
    }
    else
    {
      // invoked as Scene2d()
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

