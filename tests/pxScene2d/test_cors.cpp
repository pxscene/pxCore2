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

  void isCORSRequestHeader_test()
  {
    rtError e;
    bool result;
    rtCORS cors("http://example.com");

    e = cors.isCORSRequestHeader("", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (result);

    e = cors.isCORSRequestHeader("Access-Control-Allow-Origin", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (result);

    e = cors.isCORSRequestHeader("Origin", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (result);

    e = cors.isCORSRequestHeader("access-control-request-headers", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (result);
  }

  void isCredentialsRequestHeader_test()
  {
    rtError e;
    bool result;
    rtCORS cors("http://example.com");

    e = cors.isCredentialsRequestHeader("", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (result);

    e = cors.isCredentialsRequestHeader("Access-Control-Allow-Origin", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_FALSE (result);

    e = cors.isCredentialsRequestHeader("Cookie", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (result);

    e = cors.isCredentialsRequestHeader("authorization", result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_TRUE (result);
  }

  void origin_test()
  {
    rtError e;
    rtString result;
    rtCORS cors("http://example.com");

    e = cors.origin(result);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)0, (int)result.compare("http://example.com"));
  }
};

TEST_F(corsTest, corsTests)
{
  updateRequestForAccessControl_test();
  updateResponseForAccessControl_test();
  passesAccessControlCheck_test();
  isCORSRequestHeader_test();
  isCredentialsRequestHeader_test();
  origin_test();
}
