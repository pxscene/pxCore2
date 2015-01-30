#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtWindowWrapper.h"

void ModuleInit(v8::Handle<v8::Object> exports) 
{
  rtFunctionWrapper::exportPrototype(exports);
  rtObjectWrapper::exportPrototype(exports);
  rtWindowWrapper::exportPrototype(exports);
}

NODE_MODULE(px, ModuleInit)

