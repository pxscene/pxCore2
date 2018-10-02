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

#include "rtCORS.h"
#include "rtFileDownloader.h"

#include <curl/curl.h>

#include "test_includes.h" // Needs to be included last

class corsTest : public testing::Test
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
    std::string s;
    s = rtCORS::toLowercaseStr("Access-Control-Allow-Origin");
    EXPECT_EQ ((int)0, (int)s.compare("access-control-allow-origin"));
    s = rtCORS::toLowercaseStr("Access-Control-Allow-Credentials");
    EXPECT_EQ ((int)0, (int)s.compare("access-control-allow-credentials"));
  }

  void parseHeaders_test()
  {
    std::map<std::string, rtString> headerMap;
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

    rtError e = rtCORS::parseHeaders(rawHeaderData, headerMap);
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
      "Server:\t\t\tApache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips\n"
      "Location:\t\t\thttps://example.com \n"
      "Content-Length:\t\t\t284\n"
      "Connection:\n"
      "Content-Type:\t\t\ttext/html; charset=iso-8859-1"
    ;

    e = rtCORS::parseHeaders(rawHeaderData, headerMap);
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

    e = rtCORS::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)5, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["server"].compare("Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips"));
    EXPECT_EQ ((int)0, (int)headerMap["x-cloud-trace-context"].compare("敷リオワニ内前ヲルホ"));
    EXPECT_EQ ((int)0, (int)headerMap["access-control-allow-origin"].compare("http://localhost:8888"));
    EXPECT_EQ ((int)0, (int)headerMap["connection"].compare("CLOSE"));
    EXPECT_EQ ((int)0, (int)headerMap["expires"].compare(""));

    rawHeaderData =
      "content-type: application/json"
    ;

    e = rtCORS::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)1, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["content-type"].compare("application/json"));

    rawHeaderData =
      "x"
    ;

    e = rtCORS::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)1, (int)headerMap.size());
    EXPECT_EQ ((int)0, (int)headerMap["x"].compare(""));

    rawHeaderData =
      ""
    ;

    e = rtCORS::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)0, (int)headerMap.size());
  }

  void updateRequestForAccessControl_test()
  {
    rtCORS cors("https://example.com");
    struct curl_slist *list = NULL;

    cors.updateRequestForAccessControl(&list);
    EXPECT_EQ ((int)0, (int)std::string("Origin: https://example.com").compare(list->data));
    EXPECT_TRUE (NULL == list->next);
    curl_slist_free_all(list);
    list = NULL;

    list = curl_slist_append(list, "Content-Type: application/json");
    cors.updateRequestForAccessControl(&list);
    EXPECT_EQ ((int)0, (int)std::string("Content-Type: application/json").compare(list->data));
    EXPECT_EQ ((int)0, (int)std::string("Origin: https://example.com").compare(list->next->data));
    EXPECT_TRUE (NULL == list->next->next);
    curl_slist_free_all(list);
    list = NULL;
  }

  void updateResponseForAccessControl_test()
  {
    rtCORS cors("https://example.com");
    rtFileDownloadRequest request("https://bank.com", NULL);
    rtString rawHeaderData;
    rawHeaderData =
      "content-type: application/json\r\n"
      "set-cookie: cookie-from-server=noop\r\n"
      "access-control-allow-origin: https://mobile.bank.com\r\n"
      "x-cloud-trace-context: 209bc1a00c7409bf54f1642316d9fe6f;o=1\r\n"
      "date: Tue, 10 Jul 2018 14:33:54 GMT\r\n"
      "server: Google Frontend\r\n"
      "content-length: 4\r\n"
      "expires: Tue, 10 Jul 2018 14:33:54 GMT\r\n"
      "connection: close"
    ;
    char* rawHeaderData_str = (char*)malloc(rawHeaderData.byteLength()+1);
    memset(rawHeaderData_str, 0, rawHeaderData.byteLength()+1);
    strcpy(rawHeaderData_str, rawHeaderData.cString());
    request.setHeaderData(rawHeaderData_str, rawHeaderData.byteLength());
    rtString downloadedData;
    downloadedData =
      "data"
    ;
    char* downloadedData_str = (char*)malloc(downloadedData.byteLength()+1);
    memset(downloadedData_str, 0, downloadedData.byteLength()+1);
    strcpy(downloadedData_str, downloadedData.cString());
    request.setDownloadedData(downloadedData_str, downloadedData.byteLength());
    EXPECT_FALSE (NULL == request.downloadedData());
    EXPECT_EQ ((int)downloadedData.byteLength(), (int)request.downloadedDataSize());
    cors.updateResponseForAccessControl(&request);

    int statusCode = request.downloadStatusCode();
    rtString error = request.errorString();
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, statusCode);
    EXPECT_TRUE (NULL == request.downloadedData());
    EXPECT_EQ ((int)0, (int)request.downloadedDataSize());
    EXPECT_EQ ((int)0, (int)error.compare("Origin https://bank.com is not allowed by Access-Control-Allow-Origin."));
  }

  void passesAccessControlCheck_test()
  {
    rtError e;
    bool passes;
    rtString rawHeaderData;

    rtCORS cors("http://example.com");
    // "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://example.com"

    // case-sensitive Origin
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: HTTP://example.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // empty origin
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: ", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // typical allow
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://example.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // same-origin
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: no", false, "http://example.com", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // *
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: *", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // null
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: null", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // list
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://www.example.com http://example.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // no response headers
    e = cors.passesAccessControlCheck("", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // mismatch (scheme)
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: https://example.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // mismatch (port)
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://example.com:80", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // mismatch
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://blahblah.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // with credentials, typical allow
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://example.com\r\nAccess-Control-Allow-Credentials: true", true, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // with credentials, no credentials header
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://example.com", true, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // with credentials, credentials header "false"
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://example.com\r\nAccess-Control-Allow-Credentials: false", true, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);
    // with credentials, anonymous
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: *\r\nAccess-Control-Allow-Credentials: true", true, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (passes);

    cors = rtCORS("");

    // empty origin (request from filesystem)
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: http://blahblah.com", false, "https://w3c-test.org", passes);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (passes);
    // empty origin, empty request
    e = cors.passesAccessControlCheck("Access-Control-Allow-Origin: *", false, "", passes);
    EXPECT_EQ ((int)RT_ERROR, (int)e);
  }
};

TEST_F(corsTest, corsTests)
{
  toLowercaseStr_test();
  parseHeaders_test();
  updateRequestForAccessControl_test();
  updateResponseForAccessControl_test();
  passesAccessControlCheck_test();
}
