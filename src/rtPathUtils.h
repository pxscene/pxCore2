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

// rtPathUtils.h

#ifndef _RT_PATH_UTILS
#define _RT_PATH_UTILS

#include <string>
#include <vector>

#include "rtCore.h"
#include "rtString.h"
#include "rtValue.h"

rtError rtEnsureTrailingPathSeparator(rtString& d);
rtError rtGetCurrentDirectory(rtString& d);
rtError rtGetHomeDirectory(rtString& d);

rtError rtGetEnv(const char* e, rtString& v);
rtString rtGetEnvAsString(const char* name, const char* defaultValue = "");
rtValue rtGetEnvAsValue(const char* name, const char* defaultValue = "");

bool rtFileExists(const char* f);

bool rtIsPathAbsolute(const char *path);
bool rtIsPathAbsolute(const rtString &path);

const char *rtModuleDirSeparator();

rtError rtPathUtilPutEnv(const char *name, const char * value);

std::string rtConcatenatePath(const std::string &dir, const std::string &file);

std::string rtGetRootModulePath(const char *file = "");

class rtModuleDirs {

public:
  static rtModuleDirs* instance(const char *env_name = "NODE_PATH");
  static void destroy();
  typedef std::pair<std::vector<std::string>::iterator,
                    std::vector<std::string>::iterator> iter;
  iter iterator();

private:
  rtModuleDirs(const char *env_name);
  std::vector<std::string> mModuleDirs;
  static rtModuleDirs *mModuleInstance;
};

void rtPathUtilsInit();

#endif
