// This is not part of Node... added to expose needed methods.

#include "v8.h"

/*MODIFIED CODE BEGIN*/

namespace node
{
  class Environment;
 
  v8::Handle<v8::Context> makeContext(v8::Isolate *isolate, v8::Handle<v8::Object> sandbox);
}
 
/*MODIFIED CODE END*/

