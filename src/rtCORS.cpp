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

#include "rtCORS.h"

#include "rtUrlUtils.h"
#include "rtHttpResponse.h"
#include "rtFileDownloader.h"

#include <ctype.h>
#include <curl/curl.h>

const char* rtCORS::ENV_NAME_ENABLED = "SPARK_CORS_ENABLED";
const char* rtCORS::HTTPHeaderName_Origin = "origin";
const char* rtCORS::HTTPHeaderName_Authorization = "authorization";
const char* rtCORS::HTTPHeaderName_Cookie = "cookie";
const char* rtCORS::HTTPHeaderName_AccessControlRequestMethod = "access-control-request-method";
const char* rtCORS::HTTPHeaderName_AccessControlRequestHeaders = "access-control-request-headers";
const char* rtCORS::HTTPHeaderName_AccessControlAllowOrigin = "access-control-allow-origin";
const char* rtCORS::HTTPHeaderName_AccessControlAllowCredentials = "access-control-allow-credentials";

bool rtCORS::mEnabled = true; // default

rtCORS::rtCORS(const rtString& origin)
  : mOrigin(origin)
{
  rtLogDebug("%s : origin '%s'", __FUNCTION__, mOrigin.cString());

  static bool didCheck = false;
  if (!didCheck)
  {
    didCheck = true;
    const char* s = getenv(ENV_NAME_ENABLED);
    if (s != NULL)
    {
      rtString envVal(s);
      mEnabled = 0 == envVal.compare("true") || 0 == envVal.compare("1");
      rtLogWarn("%s : %s", __FUNCTION__, mEnabled ? "enabled" : "disabled");
    }
  }
}

rtCORS::~rtCORS()
{
  rtLogDebug("%s : origin '%s'", __FUNCTION__, mOrigin.cString());
}

// TODO: preflight mode

rtError rtCORS::updateRequestForAccessControl(struct curl_slist** headerList) const
{
  if (!mEnabled)
     return RT_OK;

  rtLogDebug("%s", __FUNCTION__);

  // Do not check is origin header already exists

  if (!mOrigin.isEmpty())
  {
    rtString headerOrigin("Origin: ");
    headerOrigin.append(mOrigin.cString());
    rtLogDebug("%s : append header '%s'", __FUNCTION__, headerOrigin.cString());
    struct curl_slist* newHeaderList = curl_slist_append(*headerList, headerOrigin.cString());
    if (!newHeaderList)
    {
      rtLogError("%s : failed to add header '%s'", __FUNCTION__, headerOrigin.cString());
      return RT_ERROR;
    }
    *headerList = newHeaderList;
  }

  return RT_OK;
}

rtError rtCORS::updateResponseForAccessControl(rtFileDownloadRequest* request) const
{
  if (!mEnabled)
     return RT_OK;

  rtLogDebug("%s", __FUNCTION__);

  const rtString& url = request->fileUrl();
  const rtString& origin = rtUrlGetOrigin(url.cString());

  // Cannot make requests to an empty URL
  if (origin.isEmpty())
  {
    rtLogError("%s : origin is empty for URL '%s'", __FUNCTION__, url.cString());
    return RT_ERROR;
  }

  // Same-origin
  if (0 == origin.compare(mOrigin.cString()))
  {
    rtLogDebug("%s : same-origin '%s'", __FUNCTION__, origin.cString());
    return RT_OK;
  }

  rtString errorDescription;
  rtString rawHeaders(request->headerData(), request->headerDataSize());
  std::map<rtString, rtString> headerMap;
  rtHttpResponse::parseHeaders(rawHeaders, headerMap);
  rtLogDebug("%s : check access to '%s' from origin '%s'", __FUNCTION__, origin.cString(), mOrigin.cString());
  if (passesAccessControlCheck(headerMap, false, origin, errorDescription))
  {
    rtLogDebug("%s : allow '%s'", __FUNCTION__, url.cString());
    return RT_OK;
  }

  rtLogWarn("%s : block '%s' from origin '%s' because: %s",
    __FUNCTION__, origin.cString(), mOrigin.cString(), errorDescription.cString());

  // Disallow access to the resource's contents.
  if (request->downloadedData() != NULL)
    free(request->downloadedData());
  request->setDownloadedData(NULL, 0);

  request->setDownloadStatusCode(RT_ERROR_NOT_ALLOWED);
  request->setErrorString(errorDescription.cString());

  return RT_OK;
}

rtError rtCORS::passesAccessControlCheck(const rtString& rawHeaderData, bool withCredentials, const rtString& origin, bool& passes) const
{
  if (!mEnabled)
  {
    passes = true;
    return RT_OK;
  }

  rtLogDebug("%s", __FUNCTION__);

  // Cannot make requests to an empty URL
  if (origin.isEmpty())
  {
    rtLogError("%s : request origin is empty", __FUNCTION__);
    return RT_ERROR;
  }

  // Same-origin
  if (0 == origin.compare(mOrigin.cString()))
  {
    rtLogDebug("%s : same-origin '%s'", __FUNCTION__, origin.cString());
    passes = true;
    return RT_OK;
  }

  rtString errorDescription;
  std::map<rtString, rtString> headerMap;
  rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
  rtLogDebug("%s : check access to '%s' from origin '%s'. withCredentials: '%s'",
    __FUNCTION__, origin.cString(), mOrigin.cString(), withCredentials ? "true" : "false");
  passes = passesAccessControlCheck(headerMap, withCredentials, origin, errorDescription);
  if (!passes)
    rtLogWarn("%s : block '%s' from origin '%s' because: %s",
      __FUNCTION__, origin.cString(), mOrigin.cString(), errorDescription.cString());
  else
    rtLogDebug("%s : allow '%s'", __FUNCTION__, origin.cString());

  return RT_OK;
}

bool rtCORS::passesAccessControlCheck(const std::map<rtString, rtString>& headerMap, bool withCredentials, const rtString& origin, rtString& errorDescription) const
{
  if (!mEnabled)
    return true;

  // A wildcard Access-Control-Allow-Origin can not be used if credentials are to be sent,
  // even with Access-Control-Allow-Credentials set to true.
  std::map<rtString, rtString>::const_iterator it = headerMap.find(HTTPHeaderName_AccessControlAllowOrigin);
  rtString accessControlOriginString = it != headerMap.end() ? it->second : rtString("");
  if (0 == accessControlOriginString.compare("*") && !withCredentials)
    return true;

  // https://www.w3.org/TR/cors/#access-control-allow-origin-response-header
  // the value of the Origin request header, "*", or "null"
  if (0 == accessControlOriginString.compare("null") && !withCredentials)
    return true;

  if (0 != accessControlOriginString.compare(mOrigin.cString()) && !mOrigin.isEmpty())
  {
    if (0 == accessControlOriginString.compare("*"))
      errorDescription = "Cannot use wildcard in Access-Control-Allow-Origin when credentials flag is true.";
    else if (-1 != accessControlOriginString.find(0, ','))
      errorDescription = "Access-Control-Allow-Origin cannot contain more than one origin.";
    else
    {
      rtString str = "Origin ";
      str.append(origin);
      str.append(" is not allowed by Access-Control-Allow-Origin.");
      errorDescription = str;
    }
    return false;
  }

  if (withCredentials)
  {
    std::map<rtString, rtString>::const_iterator accessControlCredentialsString = headerMap.find(HTTPHeaderName_AccessControlAllowCredentials);
    if (accessControlCredentialsString == headerMap.end() || 0 != accessControlCredentialsString->second.compare("true"))
    {
      errorDescription = "Credentials flag is true, but Access-Control-Allow-Credentials is not \"true\".";
      return false;
    }
  }

  return true;
}

rtError rtCORS::isCORSRequestHeader(const rtString& headerName, bool& isCORSHeader) const
{
  bool result = false;
  rtString str = rtHttpResponse::toLowercaseStr(headerName);
  if (str.compare(HTTPHeaderName_Origin) == 0 ||
    str.compare(HTTPHeaderName_AccessControlRequestMethod) == 0 ||
    str.compare(HTTPHeaderName_AccessControlRequestHeaders) == 0)
  {
    result = true;
  }
  isCORSHeader = result;
  return RT_OK;
}

rtError rtCORS::isCredentialsRequestHeader(const rtString& headerName, bool& isCredentialsHeader) const
{
  bool result = false;
  rtString str = rtHttpResponse::toLowercaseStr(headerName);
  if (str.compare(HTTPHeaderName_Authorization) == 0 ||
    str.compare(HTTPHeaderName_Cookie) == 0)
  {
    result = true;
  }
  isCredentialsHeader = result;
  return RT_OK;
}

rtDefineObject(rtCORS, rtObject);
rtDefineMethod(rtCORS, passesAccessControlCheck);
rtDefineMethod(rtCORS, isCORSRequestHeader);
rtDefineMethod(rtCORS, isCredentialsRequestHeader);
rtDefineProperty(rtCORS, isEnabled);
rtDefineProperty(rtCORS, origin);
