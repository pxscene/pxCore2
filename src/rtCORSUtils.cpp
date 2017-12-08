/*

 pxCore Copyright 2005-2017 John Robinson

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

#include "rtPermissions.h"

#include "rtUrlUtils.h"

#include <sstream>

const char* USE_ACCESS_CONTROL_CHECK_ENV_NAME = "USE_ACCESS_CONTROL_CHECK";

rtError rtCORSUtilsCheckOrigin(const rtString& origin, const rtString& reqUrl, const rtString& rawHeaders, rtString* errorStr)
{
  bool enableCheck = getenv(USE_ACCESS_CONTROL_CHECK_ENV_NAME) != NULL;
  if (!enableCheck)
  {
    // not enabled
    return RT_OK;
  }

  if (origin.isEmpty())
  {
    // no origin
    return RT_OK;
  }

  const rtString& reqUrlOrigin = rtUrlGetOrigin(reqUrl.cString());
  if (!reqUrlOrigin.isEmpty() && !strcmp(origin, reqUrlOrigin.cString()))
  {
    // request is same-origin
    return RT_OK;
  }

  rtString allowOrigin;
  const char* allowOriginFieldStart = "access-control-allow-origin:";
  if (!rawHeaders.isEmpty())
  {
    // Case-insensitive search.
    const char* h = rawHeaders.cString();
    const char* o = allowOriginFieldStart;
    for (; *h && *o && tolower(*h) == *o; h++, o++);
    if (*o != 0)
    {
      allowOriginFieldStart = "\r\naccess-control-allow-origin:";
      o = allowOriginFieldStart;
      for (; *h && *o; h++)
        o = tolower(*h) == *o ? o + 1 : allowOriginFieldStart;
    }
    if (*o == 0)
    {
      // Trim left.
      for (; *h == ' ' || *h == '\t'; h++);
      if (*h != 0)
      {
        allowOrigin = h;
        int32_t end = allowOrigin.find(0, "\r\n");
        if (end > 0)
          allowOrigin = allowOrigin.substring(0, end);
        else if (end == 0)
          allowOrigin.term();
      }
    }
  }
  if (!allowOrigin.isEmpty())
  {
    // If the value of Access-Control-Allow-Origin is not a case-sensitive match, return fail.
    if (allowOrigin.compare("*") == 0 ||
      allowOrigin.compare(origin) == 0 ||
      allowOrigin.compare("null") == 0)
    {
      return RT_OK;
    }

    // Value can be a list of origins, SP separated
    const char* a = allowOrigin.cString();
    const char* o = origin.cString();
    for (; *a; a++)
    {
      o = *a == *o ? o + 1 : origin.cString();
      if (*o == 0)
      {
        o = origin.cString();
        if (*(a + 1) == 0 || *(a + 1) == ' ')
          return RT_OK;
      }
    }
  }

  if (errorStr != NULL)
  {
    std::stringstream errorStream;
    // If the response includes zero Access-Control-Allow-Origin header values, return fail.
    if (allowOrigin.isEmpty())
    {
      errorStream << "No 'Access-Control-Allow-Origin' header is present on the requested resource.";
      errorStream << " Origin '";
      errorStream << origin.cString();
      errorStream << "' is therefore not allowed access";
    }
    else
    {
      errorStream << "The 'Access-Control-Allow-Origin' header has a value '";
      errorStream << allowOrigin.cString();
      errorStream << "' that is not equal to the supplied origin.";
      errorStream << " Origin '";
      errorStream << origin.cString();
      errorStream << "' is therefore not allowed access";
    }
    *errorStr = errorStream.str().c_str();
  }

  return RT_ERROR_NOT_ALLOWED;
}
