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

// rtUrlUtils.h

#include "rtUrlUtils.h"
#include <ctype.h>


#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#endif

#include <curl/curl.h>

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif

/*
 * rtUrlEncodeParameters: Takes an url in the form of
 *  "http://blahblah/index.js?some=some1&parm=value" and
 *  returns an rtString with the query parameters url-encoded.
 */
rtString rtUrlEncodeParameters(const char* url)
{
  rtString retVal;

/*
  char * pch;
  pch = strtok ((char *)url,"?");
  if (pch != NULL) {
    retVal = pch;
    // Get remainder of url after '?' and encode it
    pch = strtok (NULL, "?");
    if( pch != NULL) {
      retVal.append("?");
      CURL *curl = curl_easy_init();
      if(curl) {
        char *output = curl_easy_escape(curl, pch, 0);
        if(output) {
          retVal.append(output);
        }
        curl_free(output);
      }
    }
  }
*/
  rtString tempStr = url;
  int32_t pos = tempStr.find(0,"?");
  if( pos != -1) {
    retVal = tempStr.substring(0, pos+1);
    tempStr = tempStr.substring(pos+1,0);
    CURL *curl = curl_easy_init();
    if(curl && tempStr.length() > 0) {
      char *output = curl_easy_escape(curl, tempStr.cString(), 0);
      if(output) {
        retVal.append(output);
      }
      curl_free(output);
    }
    if (curl)
      curl_easy_cleanup(curl);
  }
  else {
    retVal = url;
  }

  // printf("Returning %s\n",retVal.cString());
  return retVal;
}

/*
 * rtUrlGetOrigin: Takes an url in the form of
 *  "http://blahblah/index.js?some=some1&parm=value" and
 *  returns an rtString in form scheme://host or
 *  an empty rtString if url is not valid or is the local file system
 */
rtString rtUrlGetOrigin(const char* url)
{
  if (url != NULL)
  {
    // See https://tools.ietf.org/html/rfc8089
    const char* u = url;
    const char* f = "file:";
    for (; *u && *f && tolower(*u) == *f; u++, f++);
    if (*f == 0)
    {
      return rtString();
    }

    // See http://www.ietf.org/rfc/rfc3986.txt.
    // URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
    // scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    // hier-part     = "//" authority path-abempty / path-absolute / path-rootless / path-empty
    // authority     = [ userinfo "@" ] host [ ":" port ]
    // userinfo      = *( unreserved / pct-encoded / sub-delims / ":" )
    // unreserved    = ALPHA / DIGIT / "-" / "." / "_" / "~"
    // pct-encoded   = "%" HEXDIG HEXDIG
    // sub-delims    = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="
    // host          = IP-literal / IPv4address / reg-name
    // path-abempty  = *( "/" segment )
    // path-absolute = "/" [ segment-nz *( "/" segment ) ]
    // path-rootless = segment-nz *( "/" segment )
    // path-empty    = 0<pchar>
    // segment       = *pchar
    // segment-nz    = 1*pchar
    // pchar         = unreserved / pct-encoded / sub-delims / ":" / "@"
    //
    // See https://tools.ietf.org/html/rfc6454#section-7.
    //    serialized-origin   = scheme "://" host [ ":" port ]
    //                        ; <scheme>, <host>, <port> from RFC 3986

    u = url;

    // scheme        = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )
    for (; *u && (*u == '+' || *u == '-' || *u == '.' || isalpha(*u) || isdigit(*u)); u++);
    // URI           = scheme ":" ...
    if (*u == ':' && u != url)
    {
      u++;
      // hier-part     = "//" ...
      if (*u == '/' && *(u + 1) == '/')
      {
        const char* hier_part = u;
        u += 2;
        rtString scheme(url, (uint32_t) (u - 3 - url));

        // URI           = scheme ":" hier-part [ "?" query ] [ "#" fragment ]
        for (; *u && *u != '/' && *u != '?' && *u != '#'; u++);

        // authority     = [ userinfo "@" ] host [ ":" port ]
        const char* host = u - 1;
        for (; host >= hier_part + 2 && *host != '@'; host--);
        host++;

        //    serialized-origin   = scheme "://" host [ ":" port ]
        rtString host_port;
        if (u > host)
        {
          host_port = rtString(host, (uint32_t) (u - host));
        }

        return scheme + rtString("://") + host_port;
      }
    }
  }

  return rtString();
}
