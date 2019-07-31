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

#ifndef _RT_JSON_UTILS
#define _RT_JSON_UTILS

#include "rtValue.h"

rtError json2rtValue(const char* json, rtValue& val);
rtError jsonFile2rtValue(const char* path, rtValue& val);

rtError rtValue2jsonFile(const rtValue& val, const char* path);

#endif // _RT_JSON_UTILS
