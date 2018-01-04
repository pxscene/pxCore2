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

#include "rtPathUtils.h"
#include <stdlib.h>
#if defined WIN32
#include <direct.h>
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

  return RT_OK;
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