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

#include "rtPathUtils.h"
#include <stdlib.h>
#if defined WIN32
#include <direct.h>
#include <shlwapi.h>
#else
#include <unistd.h>
#endif

#include <sys/stat.h>

rtError rtGetCurrentDirectory(rtString& d)
{
  char* p = getcwd(NULL, 0);

  if (p)
  {
    d = p;
    free(p);
  }
  return RT_OK;
}

rtError rtEnsureTrailingPathSeparator(rtString& d)
{
  if (!d.endsWith("/"))
  {
#if defined(WIN32)
    if (!d.endsWith("\\"))
#endif
    {
      d.append("/");
    }
  }
  return RT_OK;
}

rtError rtGetHomeDirectory(rtString& d)
{
  rtError e = RT_FAIL;

  #if defined(WIN32)
  const char* homeDir = "USERPROFILE";
  #else
  const char* homeDir = "HOME";
  #endif

  if (rtGetEnv(homeDir, d) == RT_OK)
    e = rtEnsureTrailingPathSeparator(d);

  return e;
}

bool rtFileExists(const char* f)
{
  struct stat buffer;
  return (stat (f, &buffer) == 0);
}

rtError rtGetEnv(const char* e, rtString& v)
{
  v = getenv(e);
  return RT_OK;
}

rtString rtGetEnvAsString(const char* name, const char* defaultValue)
{
  rtString v;
  rtGetEnv(name, v);
  if (v.isEmpty())
    v = defaultValue;
  return v;
}

rtValue rtGetEnvAsValue(const char* name, const char* defaultValue)
{
  return rtGetEnvAsString(name, defaultValue);
}

bool rtIsPathAbsolute(const char *path)
{
  if (!path)
    return false;

# ifdef WIN32
    return !PathIsRelativeA(path);
# else
    return path[0] == '/';
# endif
}

bool rtIsPathAbsolute(const rtString &path)
{
  if (path.isEmpty())
    return false;

 const char *str = path.cString();

 return rtIsPathAbsolute(str);
}

const char *rtModuleDirSeparator()
{
# ifdef WIN32
    return ";";
# else
    return ":";
# endif
}

rtError rtPathUtilPutEnv(const char *name, const char * value)
{
# ifdef WIN32
    return _putenv_s(name, value != NULL ? value : "") == 0 ? RT_OK : RT_ERROR;
# else
    if (value != NULL)
      return setenv(name, value, 1) == 0 ? RT_OK : RT_ERROR;
    else
      return unsetenv(name) == 0 ? RT_OK : RT_ERROR;
# endif
}

std::string rtConcatenatePath(const std::string &dir, const std::string &file)
{
  rtString path = dir.c_str();
  rtEnsureTrailingPathSeparator(path);
  path.append(file.c_str());

  return std::string(path.cString());
}

std::string rtGetRootModulePath(const char *file)
{
  rtModuleDirs *dirs = rtModuleDirs::instance();
  const std::string rootDir = *dirs->iterator().first;
  return rtConcatenatePath(rootDir, file != NULL ? file : "");
}

rtModuleDirs::rtModuleDirs(const char *env_name) {
  rtString cwd;
  rtGetCurrentDirectory(cwd);

  rtString rt_env = rtGetEnvAsString(env_name, cwd.cString());
  std::string env = rt_env.cString();

  const std::string sep = rtModuleDirSeparator();

  while (env.size())
  {
    const size_t index = env.find(sep);

    if (index != std::string::npos)
    {
      mModuleDirs.push_back(env.substr(0, index));
      env = env.substr(index + sep.size());

      if (env.size() == 0)
        mModuleDirs.push_back(env);
    } else
    {
      mModuleDirs.push_back(env);
      env = "";
    }
  }
}

rtModuleDirs *rtModuleDirs::instance(const char *env_name)
{
  if (NULL == mModuleInstance)
    mModuleInstance = new rtModuleDirs(env_name);

  return mModuleInstance;
}

void rtModuleDirs::destroy()
{
  if (NULL != mModuleInstance)
    delete mModuleInstance;

  mModuleInstance = NULL;
}

rtModuleDirs::iter rtModuleDirs::iterator()
{
  return std::make_pair(mModuleDirs.begin(), mModuleDirs.end());
}

rtModuleDirs *rtModuleDirs::mModuleInstance = NULL;
