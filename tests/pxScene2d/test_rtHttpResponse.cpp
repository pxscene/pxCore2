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

#include <sstream>

#include "rtHttpResponse.h"

#include "test_includes.h" // Needs to be included last

class rtHttpResponseTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void toLowercaseStr_test()
  {
    rtString s;
    s = rtHttpResponse::toLowercaseStr("Access-Control-Allow-Origin");
    EXPECT_EQ ((int)0, (int)s.compare("access-control-allow-origin"));
    s = rtHttpResponse::toLowercaseStr("Access-Control-Allow-Credentials");
    EXPECT_EQ ((int)0, (int)s.compare("access-control-allow-credentials"));
  }

  void parseHeaders_test()
  {
    std::map<rtString, rtString> headerMap;
    rtString rawHeaderData;
    rawHeaderData =
      "content-type: application/json\r\n"
      "set-cookie: cookie-from-server=noop\r\n"
      "access-control-allow-origin: \r\n"
      "x-cloud-trace-context: 209bc1a00c7409bf54f1642316d9fe6f;o=1\r\n"
      "date: Tue, 10 Jul 2018 14:33:54 GMT\r\n"
      "server: Google Frontend\r\n"
      "content-length: 845\r\n"
      "expires: Tue, 10 Jul 2018 14:33:54 GMT\r\n"
      "connection: close"
    ;

    rtError e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)9, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["content-type"].compare("application/json"));
    EXPECT_EQ ((int)0, (int)headerMap["set-cookie"].compare("cookie-from-server=noop"));
    EXPECT_EQ ((int)0, (int)headerMap["access-control-allow-origin"].compare(""));
    EXPECT_EQ ((int)0, (int)headerMap["x-cloud-trace-context"].compare("209bc1a00c7409bf54f1642316d9fe6f;o=1"));
    EXPECT_EQ ((int)0, (int)headerMap["date"].compare("Tue, 10 Jul 2018 14:33:54 GMT"));
    EXPECT_EQ ((int)0, (int)headerMap["server"].compare("Google Frontend"));
    EXPECT_EQ ((int)0, (int)headerMap["content-length"].compare("845"));
    EXPECT_EQ ((int)0, (int)headerMap["expires"].compare("Tue, 10 Jul 2018 14:33:54 GMT"));
    EXPECT_EQ ((int)0, (int)headerMap["connection"].compare("close"));

    rawHeaderData =
     "HTTP/1.1 200 OK\r\n"
     "Date: Tue, 02 Oct 2018 13:24:44 GMT\r\n"
     "Content-Type: text/html\r\n"
     "Transfer-Encoding: chunked\r\n"
     "Connection: keep-alive\r\n"
     "Set-Cookie: __cfduid=d17b30f71dcb858abe7f0067525cbb0721538486684; expires=Wed, 02-Oct-19 13:24:44 GMT; path=/; domain=.nodejs.org; HttpOnly\r\n"
     "Last-Modified: Sat, 29 Sep 2018 11:08:33 GMT\r\n"
     "CF-Cache-Status: HIT\r\n"
     "Expires: Tue, 02 Oct 2018 17:24:44 GMT\r\n"
     "Cache-Control: public, max-age=14400\r\n"
     "Expect-CT: max-age=604800, report-uri=\"https://report-uri.cloudflare.com/cdn-cgi/beacon/expect-ct\"\r\n"
     "Server: cloudflare\r\n"
     "CF-RAY: 46377db4dc4d8267-KBP\r\n\r\n"
     ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)12, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["date"].compare("Tue, 02 Oct 2018 13:24:44 GMT"));
    EXPECT_EQ ((int)0, (int)headerMap["content-type"].compare("text/html"));
    EXPECT_EQ ((int)0, (int)headerMap["transfer-encoding"].compare("chunked"));
    EXPECT_EQ ((int)0, (int)headerMap["connection"].compare("keep-alive"));
    EXPECT_EQ ((int)0, (int)headerMap["set-cookie"].compare("__cfduid=d17b30f71dcb858abe7f0067525cbb0721538486684; expires=Wed, 02-Oct-19 13:24:44 GMT; path=/; domain=.nodejs.org; HttpOnly"));
    EXPECT_EQ ((int)0, (int)headerMap["last-modified"].compare("Sat, 29 Sep 2018 11:08:33 GMT"));
    EXPECT_EQ ((int)0, (int)headerMap["cf-cache-status"].compare("HIT"));
    EXPECT_EQ ((int)0, (int)headerMap["expires"].compare("Tue, 02 Oct 2018 17:24:44 GMT"));
    EXPECT_EQ ((int)0, (int)headerMap["cache-control"].compare("public, max-age=14400"));
    EXPECT_EQ ((int)0, (int)headerMap["expect-ct"].compare("max-age=604800, report-uri=\"https://report-uri.cloudflare.com/cdn-cgi/beacon/expect-ct\""));
    EXPECT_EQ ((int)0, (int)headerMap["server"].compare("cloudflare"));
    EXPECT_EQ ((int)0, (int)headerMap["cf-ray"].compare("46377db4dc4d8267-KBP"));

    rawHeaderData =
      "Server:\t\t\tApache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips\n"
      "Location:\t\t\thttps://example.com \n"
      "Content-Length:\t\t\t284\n"
      "Connection:\n"
      "Content-Type:\t\t\ttext/html; charset=iso-8859-1"
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)5, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["server"].compare("Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips"));
    EXPECT_EQ ((int)0, (int)headerMap["location"].compare("https://example.com "));
    EXPECT_EQ ((int)0, (int)headerMap["content-length"].compare("284"));
    EXPECT_EQ ((int)0, (int)headerMap["connection"].compare(""));
    EXPECT_EQ ((int)0, (int)headerMap["content-type"].compare("text/html; charset=iso-8859-1"));

    rawHeaderData =
      "Server:Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips\r\n"
      "x-cloud-trace-context: 敷リオワニ内前ヲルホ\r\n"
      "Access-Control-Allow-Origin:          \t\t   http://localhost:8888\r\n"
      "CONNECTION: CLOSE\n"
      "expires"
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)4, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["server"].compare("Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips"));
    EXPECT_EQ ((int)0, (int)headerMap["x-cloud-trace-context"].compare("敷リオワニ内前ヲルホ"));
    EXPECT_EQ ((int)0, (int)headerMap["access-control-allow-origin"].compare("http://localhost:8888"));
    EXPECT_EQ ((int)0, (int)headerMap["connection"].compare("CLOSE"));

    rawHeaderData =
      "content-type: application/json"
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)1, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["content-type"].compare("application/json"));

    rawHeaderData =
      "x"
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)0, (int)headerMap.size());

    rawHeaderData =
      ""
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)0, (int)headerMap.size());
  }
};

TEST_F(rtHttpResponseTest, rtHttpResponseTests)
{
  toLowercaseStr_test();
  parseHeaders_test();
}
