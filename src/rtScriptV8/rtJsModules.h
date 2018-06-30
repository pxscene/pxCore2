/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef RT_JS_MODULES
#define RT_JS_MODULES

#include "v8_headers.h"

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include "rtScript.h"
#include "rtWrapperUtils.h"

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

#include <assert.h>

#define RT_V8_TEST_BINDINGS

namespace rtScriptV8Utils
{

  extern rtV8FunctionItem v8ModuleBindings[];

} // namespace

#endif

