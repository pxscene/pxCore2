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
#include <string>
#include <stdlib.h>

#include "rtCORSUtils.h"
#include "pxArchive.h"
#include "rtFileDownloader.h"
#include "test_includes.h" // Needs to be included last

class pxArchiveMockForCORS : public pxArchive
{
public:
  inline rtFileDownloadRequest* getDownloadRequest() const
  {
    return mDownloadRequest;
  }
};

class corsTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    // Enable...
    const char* envVal = getenv(USE_ACCESS_CONTROL_CHECK_ENV_NAME);
    if (envVal != NULL)
    {
      useAccessControlCheckEnv = envVal;
    }
    setenv(USE_ACCESS_CONTROL_CHECK_ENV_NAME,"1",1);
    rtLogWarn("env set...");
  }

  virtual void TearDown()
  {
    // Clean up...
    if (!useAccessControlCheckEnv.empty())
    {
      setenv(USE_ACCESS_CONTROL_CHECK_ENV_NAME,useAccessControlCheckEnv.c_str(),1);
      rtLogWarn("env revert...");
    }
    else
    {
      unsetenv(USE_ACCESS_CONTROL_CHECK_ENV_NAME);
      rtLogWarn("env unset...");
    }
  }

  void test0()
  {
    // nulls
    EXPECT_FALSE(testFileDownloadRequest("http://localhost:8888", NULL, NULL));
    EXPECT_TRUE (testFileDownloadRequest("http://localhost:8888", "http://localhost:8888", NULL));
    EXPECT_FALSE(testFileDownloadRequest("http://localhost:9999", "http://localhost:8888", NULL));
    EXPECT_TRUE (testFileDownloadRequest(NULL, NULL, NULL));
    EXPECT_TRUE (testFileDownloadRequest(NULL, "http://localhost:8888", NULL));
  }

  void test1()
  {
    // case-insensitive headers
    EXPECT_TRUE (testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\naccess-control-allow-origin: http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_TRUE (testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_TRUE (testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:          \t\thttp://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // empty origin
    EXPECT_FALSE(testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // empty origin
    EXPECT_TRUE (testFileDownloadRequest(
        "",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // minimal response
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:100",
        "Access-Control-Allow-Origin: http://blahblah.com:100"));
    // utf8
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nOmg: 敷リオワニ内前ヲルホ\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // utf8
    EXPECT_TRUE (testFileDownloadRequest(
        "敷リオワニ内前ヲルホ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: 敷リオワニ内前ヲルホ\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // utf8 (list)
    EXPECT_TRUE (testFileDownloadRequest(
        "敷リオワニ内前ヲルホ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com 敷リオワニ内前ヲルホ http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // whitespace
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com   ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com   ",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com   \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void test2()
  {
    // case-sensitive Origin
    EXPECT_FALSE (testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=HTTP://LOCALHOST:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: HTTP://LOCALHOST:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // LF instead of CRLF
    EXPECT_FALSE (testFileDownloadRequest(
        "http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: http://localhost:8888\nServer: BaseHTTP/0.3 Python/2.7.13\nDate: Wed, 27 Sep 2017 17:27:57 GMT\nContent-Length: 87\n\n"));
  }

  void test3()
  {
    // no origin
    EXPECT_TRUE (testFileDownloadRequest(
        NULL,
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // typical allow
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // same-origin
    EXPECT_TRUE (testFileDownloadRequest(
        "https://w3c-test.org",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // *
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=*",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // null
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: null\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // list
    EXPECT_TRUE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://www.example.com http://blahblah.com:100 http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void test4()
  {
    // no response
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        ""));
    // no CORS headers in response
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        "HTTP/1.1 200 OK\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (scheme)
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=https://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: https://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (empty)
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:1000",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com:1000\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch (list)
    EXPECT_FALSE (testFileDownloadRequest(
        "http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=null",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://www.example.com http://blahblah.com:10004 http://localhost\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));

  }

  void test5()
  {
    EXPECT_TRUE (testArchiveOrigin(NULL, ""));
    EXPECT_TRUE (testArchiveOrigin("", ""));
    EXPECT_TRUE (testArchiveOrigin("http://", "http://"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com:500", "http://www.example.com:500"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com:80/", "http://www.example.com:80/"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com", "http://www.example.com"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com/", "http://www.example.com/"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com/foo", "http://www.example.com/foo"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com?bar", "http://www.example.com?bar"));
    EXPECT_TRUE (testArchiveOrigin("http://www.example.com/index?foo=#bar", "http://www.example.com/index?foo=#bar"));
    EXPECT_TRUE (testArchiveOrigin("a b cd ", "a b cd "));
    EXPECT_TRUE (testArchiveOrigin(" a b cd ", " a b cd "));
    EXPECT_TRUE (testArchiveOrigin(" ", " "));
    EXPECT_TRUE (testArchiveOrigin("        ", "        "));
    EXPECT_TRUE (testArchiveOrigin(" X", " X"));
    EXPECT_TRUE (testArchiveOrigin("        X", "        X"));
    EXPECT_TRUE (testArchiveOrigin("Y ", "Y "));
    EXPECT_TRUE (testArchiveOrigin("Y        ", "Y        "));
    EXPECT_TRUE (testArchiveOrigin("\t", "\t"));
    EXPECT_TRUE (testArchiveOrigin("\t\t\t", "\t\t\t"));
    EXPECT_TRUE (testArchiveOrigin("\tX", "\tX"));
    EXPECT_TRUE (testArchiveOrigin("\t\t\tX", "\t\t\tX"));
    EXPECT_TRUE (testArchiveOrigin("Y\t", "Y\t"));
    EXPECT_TRUE (testArchiveOrigin("Y\t\t\t", "Y\t\t\t"));
    EXPECT_TRUE (testArchiveOrigin("敷リオワニ内前ヲルホ", "敷リオワニ内前ヲルホ"));
  }

  void test6()
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
  std::string useAccessControlCheckEnv;

  bool testFileDownloadRequest(const char* origin, const char* url, const char* responseHeaders)
  {
    rtString originStr(origin);
    rtString reqUrl(url);
    rtString rawHeaders(responseHeaders);
    return RT_OK == rtCORSUtilsCheckOrigin(origin, reqUrl, rawHeaders);
  }

  bool testArchiveOrigin(const char* originIn, const char* originOut)
  {
    pxArchiveMockForCORS* a = new pxArchiveMockForCORS;
    a->initFromUrl(rtString("http://foo.bar/"), originIn ? rtString(originIn) : rtString());
    rtFileDownloadRequest* downloadRequest = a->getDownloadRequest();
    EXPECT_FALSE (downloadRequest == NULL);
    const rtString& o = downloadRequest->origin();
    const char* parsedOrigin = o.cString();

    bool ret;
    if (0 == strcmp(originOut, parsedOrigin))
      ret = true;
    else
    {
      ret = false;
      rtLogError("mismatch: '%s' instead of '%s'", parsedOrigin, originOut);
    }

    // Clean up...
    downloadRequest->setCallbackFunctionThreadSafe(NULL);
    rtFileDownloader::instance()->removeDownloadRequest(downloadRequest);
    delete a;

    return ret;
  }

  bool testFileDownloadRequestDoesNotReturnAnything(const char* originIn, const char* url)
  {
    rtFileDownloadRequest* r = new rtFileDownloadRequest(url, NULL);
    r->setOrigin(originIn);
    bool ok = rtFileDownloader::instance()->downloadFromNetwork(r);

    bool ret;
    if (ok)
    {
      ret =
        -1 == r->downloadStatusCode() &&
        NULL == r->downloadedData() &&
        0 == r->downloadedDataSize();
    }
    else
    {
      ret = true;
      rtLogError("connectionError: '%s'", r->errorString().cString());
    }

    // Clean up...
    delete r;

    return ret;
  }
};

TEST_F(corsTest, corsTests)
{
  test0();
  test1();
  test2();
  test3();
  test4();
  test5();
  test6();
}
