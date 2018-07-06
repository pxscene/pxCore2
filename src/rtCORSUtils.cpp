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

#include "rtCORSUtils.h"

#include "rtUrlUtils.h"

#include <ctype.h>

namespace
{
  static const char* ENV_NAME_ENABLED = "SPARK_CORS_ENABLED";
}

rtError rtCORSUtilsCheckOrigin(const rtString& origin, const rtString& url, const rtString& headers)
{
  static bool enable = true; // default
  static bool didCheck = false;
  if (!didCheck)
  {
    didCheck = true;
    const char* s = getenv(ENV_NAME_ENABLED);
    if (s != NULL)
    {
      rtString envVal(s);
      enable = 0 == envVal.compare("true") || 0 == envVal.compare("1");
    }
  }

  if (!enable)
    return RT_OK;

  // Do not apply restrictions to local files
  if (origin.isEmpty())
    return RT_OK;

  // Do not apply restrictions for same-origin requests
  const rtString& reqUrlOrigin = rtUrlGetOrigin(url.cString());
  if (!reqUrlOrigin.isEmpty() && 0 == reqUrlOrigin.compare(origin.cString()))
    return RT_OK;

  rtString allowOrigin;
  const char* allowOriginFieldStart = "access-control-allow-origin:";
  if (!headers.isEmpty())
  {
    // Case-insensitive search.
    const char* h = headers.cString();
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

  // If the response includes zero Access-Control-Allow-Origin header values, return fail
  if (allowOrigin.isEmpty())
    return RT_ERROR_CORS_NO_HEADER;

  if (allowOrigin.compare("*") == 0 || allowOrigin.compare(origin) == 0 || allowOrigin.compare("null") == 0)
    return RT_OK;

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

  return RT_ERROR_CORS_ORIGIN_MISMATCH;
}
