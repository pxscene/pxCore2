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

#include "rtHttpRequest.h"
#include "rtHttpResponse.h"

#include <semaphore.h>
#include <fcntl.h>

#include "test_includes.h" // Needs to be included last

class rtHttpResponseTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    testSem = sem_open("/semaphore", O_CREAT, 0644, 0);
  }

  virtual void TearDown()
  {
    int ret = sem_close(testSem);
    ret = sem_unlink("/semaphore");
    UNUSED_PARAM(ret);
  }

  void toLowercaseStr_test()
  {
    rtString s;
    s = rtHttpResponse::toLowercaseStr("Access-Control-Allow-Origin");
    EXPECT_EQ (std::string(s.cString()), "access-control-allow-origin");
    s = rtHttpResponse::toLowercaseStr("Access-Control-Allow-Credentials");
    EXPECT_EQ (std::string(s.cString()), "access-control-allow-credentials");
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
    EXPECT_EQ (std::string(headerMap["content-type"].cString()), "application/json");
    EXPECT_EQ (std::string(headerMap["set-cookie"].cString()), "cookie-from-server=noop");
    EXPECT_EQ (std::string(headerMap["access-control-allow-origin"].cString()), "");
    EXPECT_EQ (std::string(headerMap["x-cloud-trace-context"].cString()), "209bc1a00c7409bf54f1642316d9fe6f;o=1");
    EXPECT_EQ (std::string(headerMap["date"].cString()), "Tue, 10 Jul 2018 14:33:54 GMT");
    EXPECT_EQ (std::string(headerMap["server"].cString()), "Google Frontend");
    EXPECT_EQ (std::string(headerMap["content-length"].cString()), "845");
    EXPECT_EQ (std::string(headerMap["expires"].cString()), "Tue, 10 Jul 2018 14:33:54 GMT");
    EXPECT_EQ (std::string(headerMap["connection"].cString()), "close");

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
    EXPECT_EQ (std::string(headerMap["date"].cString()), "Tue, 02 Oct 2018 13:24:44 GMT");
    EXPECT_EQ (std::string(headerMap["content-type"].cString()), "text/html");
    EXPECT_EQ (std::string(headerMap["transfer-encoding"].cString()), "chunked");
    EXPECT_EQ (std::string(headerMap["connection"].cString()), "keep-alive");
    EXPECT_EQ (std::string(headerMap["set-cookie"].cString()), "__cfduid=d17b30f71dcb858abe7f0067525cbb0721538486684; expires=Wed, 02-Oct-19 13:24:44 GMT; path=/; domain=.nodejs.org; HttpOnly");
    EXPECT_EQ (std::string(headerMap["last-modified"].cString()), "Sat, 29 Sep 2018 11:08:33 GMT");
    EXPECT_EQ (std::string(headerMap["cf-cache-status"].cString()), "HIT");
    EXPECT_EQ (std::string(headerMap["expires"].cString()), "Tue, 02 Oct 2018 17:24:44 GMT");
    EXPECT_EQ (std::string(headerMap["cache-control"].cString()), "public, max-age=14400");
    EXPECT_EQ (std::string(headerMap["expect-ct"].cString()), "max-age=604800, report-uri=\"https://report-uri.cloudflare.com/cdn-cgi/beacon/expect-ct\"");
    EXPECT_EQ (std::string(headerMap["server"].cString()), "cloudflare");
    EXPECT_EQ (std::string(headerMap["cf-ray"].cString()), "46377db4dc4d8267-KBP");

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
    EXPECT_EQ (std::string(headerMap["server"].cString()), "Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips");
    EXPECT_EQ (std::string(headerMap["location"].cString()), "https://example.com ");
    EXPECT_EQ (std::string(headerMap["content-length"].cString()), "284");
    EXPECT_EQ (std::string(headerMap["connection"].cString()), "");
    EXPECT_EQ (std::string(headerMap["content-type"].cString()), "text/html; charset=iso-8859-1");

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
    EXPECT_EQ (std::string(headerMap["server"].cString()), "Apache/2.4.6 (CentOS) OpenSSL/1.0.1e-fips");
    EXPECT_EQ (std::string(headerMap["x-cloud-trace-context"].cString()), "敷リオワニ内前ヲルホ");
    EXPECT_EQ (std::string(headerMap["access-control-allow-origin"].cString()), "http://localhost:8888");
    EXPECT_EQ (std::string(headerMap["connection"].cString()), "CLOSE");

    rawHeaderData =
      "content-type: application/json"
    ;

    e = rtHttpResponse::parseHeaders(rawHeaderData, headerMap);
    EXPECT_EQ ((int)RT_OK, (int)e);
    EXPECT_EQ ((int)1, (int)headerMap.size());
    EXPECT_EQ (std::string(headerMap["content-type"].cString()), "application/json");

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

  static rtError response_test_callback1(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    EXPECT_EQ ((int)1, (int)numArgs);

    rtHttpResponse* resp = (rtHttpResponse*)args[0].toObject().getPtr();
    int32_t statusCode;
    EXPECT_EQ ((int)RT_OK, (int)resp->statusCode(statusCode));
    EXPECT_EQ ((int)statusCode, (int)200);
    rtString errorMessage;
    EXPECT_EQ ((int)RT_OK, (int)resp->errorMessage(errorMessage));
    EXPECT_TRUE (errorMessage.isEmpty());

    rtObjectRef headers;
    EXPECT_EQ ((int)RT_OK, (int)resp->headers(headers));
    rtString h1 = headers.get<rtString>("access-control-allow-credentials");
    rtString h2 = headers.get<rtString>("access-control-allow-origin");
    EXPECT_EQ (std::string(h1.cString()), "true");
    EXPECT_EQ (std::string(h2.cString()), "*");

    rtFunctionRef fn1 = new rtFunctionCallback(response_test_callback3, (sem_t*)context);
    rtFunctionRef fn2 = new rtFunctionCallback(response_test_callback4, (sem_t*)context);
    EXPECT_EQ ((int)RT_OK, resp->addListener("data", fn1));
    EXPECT_EQ ((int)RT_OK, resp->addListener("end", fn2));

    UNUSED_PARAM(result);

    return RT_OK;
  }

  static rtError response_test_callback2(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    UNUSED_PARAM(numArgs);
    UNUSED_PARAM(args);
    UNUSED_PARAM(result);

    ADD_FAILURE();

    sem_post((sem_t*)context);
    return RT_OK;
  }

  static rtError response_test_callback3(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    EXPECT_EQ ((int)1, (int)numArgs);

    responseData += args[0].toString();

    UNUSED_PARAM(result);
    UNUSED_PARAM(context);

    return RT_OK;
  }

  static rtError response_test_callback4(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    UNUSED_PARAM(numArgs);
    UNUSED_PARAM(args);
    UNUSED_PARAM(result);

    EXPECT_FALSE (responseData.isEmpty());
    EXPECT_TRUE (responseData.beginsWith("{\n  \"args\": {}"));
    EXPECT_TRUE (responseData.endsWith("\n}\n"));

    sem_post((sem_t*)context);
    return RT_OK;
  }

  void response_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    rtObjectRef options = new rtMapObject;
    options.set("protocol", "https:");
    options.set("hostname", "httpbin.org");
    options.set("path", "/get");
    options.set("method", "GET");

    rtFunctionRef fn1 = new rtFunctionCallback(response_test_callback1, testSem);
    rtFunctionRef fn2 = new rtFunctionCallback(response_test_callback2, testSem);

    req = new rtHttpRequest(options);
    EXPECT_EQ ((int)RT_OK, req->addListener("response", fn1));
    EXPECT_EQ ((int)RT_OK, req->addListener("error", fn2));
    EXPECT_EQ ((int)RT_OK, req->end());
    ref = req;
    EXPECT_TRUE (req->inQueue());

    sem_wait(testSem);
  }

  static rtError addListener_test_callback(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    UNUSED_PARAM(numArgs);
    UNUSED_PARAM(args);
    UNUSED_PARAM(result);

    int* times_fn_called = (int*)context;
    (*times_fn_called)++;

    return RT_OK;
  }

  void addListener_test()
  {
    rtObjectRef ref;
    rtHttpResponse* resp;

    int times_fn1_called = 0;
    int times_fn2_called = 0;
    int times_fn3_called = 0;
    rtFunctionRef fn1 = new rtFunctionCallback(addListener_test_callback, &times_fn1_called);
    rtFunctionRef fn2 = new rtFunctionCallback(addListener_test_callback, &times_fn2_called);
    rtFunctionRef fn3 = new rtFunctionCallback(addListener_test_callback, &times_fn3_called);

    resp = new rtHttpResponse();
    EXPECT_EQ ((int)RT_OK, resp->addListener("data", fn1));
    EXPECT_EQ ((int)RT_OK, resp->once("data", fn2));
    EXPECT_EQ ((int)RT_OK, resp->addListener("end", fn3));
    ref = resp;

    resp->onData();
    EXPECT_EQ (1, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    resp->onData();
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    EXPECT_EQ ((int)RT_OK, resp->removeAllListenersByName("data"));

    resp->onData();
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    resp->onEnd();
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (1, times_fn3_called);

    resp->onEnd();
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (2, times_fn3_called);

    EXPECT_EQ ((int)RT_OK, resp->removeAllListeners());

    resp->onEnd();
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (2, times_fn3_called);
  }

  sem_t* testSem;
  static rtString responseData;
};

rtString rtHttpResponseTest::responseData = rtString();

TEST_F(rtHttpResponseTest, rtHttpResponseTests)
{
  toLowercaseStr_test();
  parseHeaders_test();
  response_test();
  addListener_test();
}
