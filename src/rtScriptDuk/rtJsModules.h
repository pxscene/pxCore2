#ifndef RT_JS_MODULES_H
#define RT_JS_MODULES_H

#include <rtError.h>
#include <rtObject.h>
#include <rtString.h>
#include <rtValue.h>

#include <stdarg.h>
#include <string>
#include <map>
#include <memory>

extern "C" {
#include "duv.h"

}
void rtSetupJsModuleBindings(duk_context *ctx);

#endif
