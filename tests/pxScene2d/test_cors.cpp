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

#include "rtCORSUtils.h"
#include "rtFileDownloader.h"

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

  void testNullEmpty()
  {
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest("http://localhost:8888", NULL, NULL));
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest("http://localhost:8888", "http://localhost:8888", NULL));
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest("http://localhost:9999", "http://localhost:8888", NULL));
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(NULL, NULL, NULL));
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(NULL, "http://localhost:8888", NULL));
  }

  void testHeaders()
  {
    // case-insensitive headers
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\naccess-control-allow-origin: http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:          \t\thttp://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // empty origin
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // empty origin
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // minimal response
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:100",
        "Access-Control-Allow-Origin: http://blahblah.com:100"));
    // utf8
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nOmg: 敷リオワニ内前ヲルホ\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // utf8
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "敷リオワニ内前ヲルホ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: 敷リオワニ内前ヲルホ\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // utf8 (list)
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "敷リオワニ内前ヲルホ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com 敷リオワニ内前ヲルホ http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // whitespace
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com   ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com   ",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com   \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));

    // case-sensitive Origin
    EXPECT_EQ ((int)RT_ERROR_CORS_ORIGIN_MISMATCH, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=HTTP://LOCALHOST:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: HTTP://LOCALHOST:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // LF instead of CRLF
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: http://localhost:8888\nServer: BaseHTTP/0.3 Python/2.7.13\nDate: Wed, 27 Sep 2017 17:27:57 GMT\nContent-Length: 87\n\n"));

    // no origin
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        NULL,
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // typical allow
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // same-origin
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "https://w3c-test.org",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // *
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=*",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // null
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: null\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // list
    EXPECT_EQ ((int)RT_OK, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://www.example.com http://blahblah.com:100 http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));

    // no response
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        ""));
    // no CORS headers in response
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        "HTTP/1.1 200 OK\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (scheme)
    EXPECT_EQ ((int)RT_ERROR_CORS_ORIGIN_MISMATCH, (int)testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=https://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: https://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (empty)
    EXPECT_EQ ((int)RT_ERROR_CORS_NO_HEADER, (int)testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch
    EXPECT_EQ ((int)RT_ERROR_CORS_ORIGIN_MISMATCH, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:1000",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com:1000\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (list)
    EXPECT_EQ ((int)RT_ERROR_CORS_ORIGIN_MISMATCH, (int)testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://www.example.com http://blahblah.com:10004 http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void testEffect()
  {
    EXPECT_TRUE (testFileDownloadRequestDoesNotReturnAnything("http://foo.bar",
      "http://server.test-cors.org/server?id=7562468&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20foo.bar"));
    EXPECT_TRUE (testFileDownloadRequestDoesNotReturnAnything("http://foo.bar",
      "http://server.test-cors.org/server?id=7562468&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20%20"));
    EXPECT_TRUE (testFileDownloadRequestDoesNotReturnAnything("http://foo.bar",
      "http://server.test-cors.org/server?id=7562468&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20"));
    EXPECT_TRUE (testFileDownloadRequestDoesNotReturnAnything("http://foo.bar",
      "http://server.test-cors.org/server?id=7562468&enable=true&status=200&credentials=false&response_headers=Access-Control-Allow-Origin%3A%20%2A%2A%2A%2A%2A%2A%2A%2A%2A%2A%2A"));
  }

private:
  rtError testFileDownloadRequest(const char* origin, const char* url, const char* responseHeaders)
  {
    rtString originStr(origin);
    rtString reqUrl(url);
    rtString rawHeaders(responseHeaders);
    return rtCORSUtilsCheckOrigin(origin, reqUrl, rawHeaders);
  }

  bool testFileDownloadRequestDoesNotReturnAnything(const char* originIn, const char* url)
  {
    rtFileDownloadRequest* r = new rtFileDownloadRequest(url, NULL);
    r->setOrigin(originIn);
    bool ok = rtFileDownloader::instance()->downloadFromNetwork(r);
    bool ret = !ok ||
      ((RT_ERROR_CORS_ORIGIN_MISMATCH == r->downloadStatusCode() || RT_ERROR_CORS_NO_HEADER == r->downloadStatusCode()) &&
      NULL == r->downloadedData() &&
      0 == r->downloadedDataSize());

    // Clean up...
    delete r;
    return ret;
  }
};

TEST_F(corsTest, corsTests)
{
  testNullEmpty();
  testHeaders();
  testEffect();
}
