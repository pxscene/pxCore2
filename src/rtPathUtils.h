/*

 rtCore Copyright 2005-2017 John Robinson

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

// rtPathUtils.h

#ifndef _RT_PATH_UTILS
#define _RT_PATH_UTILS

#include "rtCore.h"
#include "rtString.h"
#include "rtValue.h"

rtError rtGetCurrentDirectory(rtString& d);
rtError rtGetHomeDirectory(rtString& d);

rtError rtGetEnv(const char* e, rtString& v);
rtString rtGetEnvAsString(const char* name, const char* defaultValue = "");
rtValue rtGetEnvAsValue(const char* name, const char* defaultValue = "");

bool rtFileExists(const char* f);

#endif
