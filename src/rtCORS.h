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

#ifndef _RT_CORS
#define _RT_CORS

#include "rtString.h"
#include "rtError.h"
#include "rtObject.h"
#include "rtRef.h"

#include <string>
#include <map>

struct curl_slist;
class rtFileDownloadRequest;
class rtCORS;

typedef rtRef<rtCORS> rtCORSRef;

class rtCORS : public rtObject
{
public:
  rtCORS(const rtString& origin = rtString());
  virtual ~rtCORS();

  rtDeclareObject(rtCORS, rtObject);
  rtMethod3ArgAndReturn("passesAccessControlCheck", passesAccessControlCheck, rtString, bool, rtString, bool);
  rtReadOnlyProperty(isEnabled, isEnabled, bool);

  rtError passesAccessControlCheck(const rtString& rawHeaderData, bool withCredentials, const rtString& origin, bool& passes) const;
  rtError isEnabled(bool& v) const { v = mEnabled; return RT_OK; }

  rtError updateRequestForAccessControl(struct curl_slist** headerList) const;
  rtError updateResponseForAccessControl(rtFileDownloadRequest* request) const;

  static rtError parseHeaders(const rtString& rawHeaderData, std::map<std::string, rtString>& headerMap);
  static std::string toLowercaseStr(const rtString& str);

protected:
  bool passesAccessControlCheck(const std::map<std::string, rtString>& headerMap, bool withCredentials, const rtString& origin, rtString& errorDescription) const;

  static const char* ENV_NAME_ENABLED;
  static const char* HTTPHeaderName_AccessControlAllowOrigin;
  static const char* HTTPHeaderName_AccessControlAllowCredentials;

  static bool mEnabled;

  rtString mOrigin;
};

#endif
