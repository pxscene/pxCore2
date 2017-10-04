#include <sstream>

#include "rtFileDownloader.h"
#include "test_includes.h" // Needs to be included last

class accessControlTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void testOriginMalformedAllow()
  {
    // case-insensitive headers
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "ORIGIN: http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\naccess-control-allow-origin: http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin:http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:http://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // optional whitespace
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin:  \thttp://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin:          \t\thttp://localhost:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // unexpected whitespace after Origin
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin : http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // empty origin
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // malformed origin
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // minimal response
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:100",
        "Access-Control-Allow-Origin: http://blahblah.com:100"));
    // utf8
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nOmg: 敷リオワニ内前ヲルホ\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // utf8
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: 敷リオワニ内前ヲルホ",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: 敷リオワニ内前ヲルホ\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void testOriginMalformedDisallow()
  {
    // case-sensitive Origin
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=HTTP://LOCALHOST:8888",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: HTTP://LOCALHOST:8888\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // LF instead of CRLF
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://localhost:8888",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://localhost:8888",
        "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: http://localhost:8888\nServer: BaseHTTP/0.3 Python/2.7.13\nDate: Wed, 27 Sep 2017 17:27:57 GMT\nContent-Length: 87\n\n"));
  }

  void testOriginAllow()
  {
    // no origin
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        NULL,
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // typical allow
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // same-origin
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: https://w3c-test.org",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // *
    EXPECT_TRUE (ALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=*",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: *\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void testOriginDisallow()
  {
    // no response
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        ""));
    // no CORS headers in response
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=none",
        "HTTP/1.1 200 OK\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=https://blahblah.com",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: https://blahblah.com\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: \r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
    // mismatch
    EXPECT_TRUE (DISALLOW == testFileDownloadRequest(
        "Origin: http://blahblah.com:100",
        "https://w3c-test.org/cors/resources/cors-makeheader.py?origin=http://blahblah.com:1000",
        "HTTP/1.1 200 OK\r\nAccess-Control-Allow-Origin: http://blahblah.com:1000\r\nServer: BaseHTTP/0.3 Python/2.7.13\r\nDate: Wed, 27 Sep 2017 17:27:57 GMT\r\nContent-Length: 87\r\n\r\n"));
  }

  void testNoContentWhenOriginDisallow()
  {
    rtFileDownloadRequest req("http://blahblah.com", NULL);
    rtString headerOrigin("Origin: http://blahblah.com:100");
    req.additionalHttpHeaders().push_back(headerOrigin);
    req.setHeaderData(NULL, 0);
    const char* content = "content";
    req.setDownloadedData(strdup(content), strlen(content));
    rtFileDownloader::checkAccessControlHeaders(&req);
    EXPECT_TRUE (DISALLOW == req.downloadStatusCode());
    EXPECT_TRUE (NULL == req.downloadedData());
    EXPECT_TRUE (0 == req.downloadedDataSize());
  }

private:
  static const int ALLOW = 0;
  static const int DISALLOW = -1;

  int testFileDownloadRequest(const char* origin, const char* url, const char* responseHeaders)
  {
    rtFileDownloadRequest req(url, NULL);
    if (origin)
    {
      rtString headerOrigin(origin);
      req.additionalHttpHeaders().push_back(headerOrigin);
    }
    req.setHeaderData(strdup(responseHeaders), strlen(responseHeaders));
    rtFileDownloader::checkAccessControlHeaders(&req);
    return req.downloadStatusCode();
  }
};

TEST_F(accessControlTest, accessControlTests)
{
  testOriginMalformedAllow();
  testOriginMalformedDisallow();
  testOriginAllow();
  testOriginDisallow();
  testNoContentWhenOriginDisallow();
}
