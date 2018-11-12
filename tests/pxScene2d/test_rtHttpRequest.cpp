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

class rtHttpRequestTest : public testing::Test
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

  void ctor_url_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    req = new rtHttpRequest("https://example.com");
    ref = req;
    EXPECT_EQ ((int)0, (int)req->url().compare("https://example.com"));
    EXPECT_EQ ((int)0, (int)req->headers().size());
    EXPECT_TRUE (req->method().isEmpty());
    EXPECT_TRUE (req->writeData().isEmpty());
    EXPECT_FALSE (req->inQueue());
  }

  void ctor_options_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    rtObjectRef options = new rtMapObject;
    options.set("protocol", "http:");
    options.set("hostname", "httpbin.org");
    options.set("path", "/post");
    options.set("method", "POST");
    rtObjectRef headers = new rtMapObject;
    headers.set("Content-Type", "application/json");
    headers.set("Authorization", "token");
    options.set("headers", headers);

    req = new rtHttpRequest(options);
    req->write("{\"postKey1\":\"postValue1\"}");
    ref = req;
    EXPECT_EQ ((int)0, (int)req->url().compare("http://httpbin.org/post"));
    EXPECT_EQ ((int)2, (int)req->headers().size());
    EXPECT_EQ ((int)0, (int)req->headers()[0].compare("Content-Type: application/json"));
    EXPECT_EQ ((int)0, (int)req->headers()[1].compare("Authorization: token"));
    EXPECT_EQ ((int)0, (int)req->method().compare("POST"));
    EXPECT_EQ ((int)0, (int)req->writeData().compare("{\"postKey1\":\"postValue1\"}"));
    EXPECT_FALSE (req->inQueue());
  }

  void setHeader_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    req = new rtHttpRequest("https://example.com");
    req->setHeader("Authorization", "token");
    ref = req;
    EXPECT_EQ ((int)0, (int)req->url().compare("https://example.com"));
    EXPECT_EQ ((int)1, (int)req->headers().size());
    EXPECT_EQ ((int)0, (int)req->headers()[0].compare("Authorization: token"));
    EXPECT_TRUE (req->method().isEmpty());
    EXPECT_TRUE (req->writeData().isEmpty());
    EXPECT_FALSE (req->inQueue());
  }

  static rtError end_test_callback(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    EXPECT_EQ ((int)1, (int)numArgs);

    rtHttpResponse* resp = (rtHttpResponse*)args[0].toObject().getPtr();
    int32_t statusCode;
    EXPECT_EQ ((int)RT_OK, (int)resp->statusCode(statusCode));
    EXPECT_EQ ((int)statusCode, (int)200);
    rtString errorMessage;
    EXPECT_EQ ((int)RT_OK, (int)resp->errorMessage(errorMessage));
    EXPECT_TRUE (errorMessage.isEmpty());

    UNUSED_PARAM(result);

    sem_post((sem_t*)context);
    return RT_OK;
  }

  void end_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    rtObjectRef options = new rtMapObject;
    options.set("protocol", "http:");
    options.set("hostname", "httpbin.org");
    options.set("path", "/post");
    options.set("method", "POST");
    rtObjectRef headers = new rtMapObject;
    headers.set("Content-Type", "application/json");
    headers.set("Authorization", "token");
    options.set("headers", headers);

    rtFunctionRef fn;

    req = new rtHttpRequest(options);
    req->addListener("response", new rtFunctionCallback(end_test_callback, testSem));
    req->write("{\"postKey1\":\"postValue1\"}");
    req->end();
    ref = req;
    EXPECT_EQ ((int)0, (int)req->url().compare("http://httpbin.org/post"));
    EXPECT_EQ ((int)2, (int)req->headers().size());
    EXPECT_EQ ((int)0, (int)req->headers()[0].compare("Content-Type: application/json"));
    EXPECT_EQ ((int)0, (int)req->headers()[1].compare("Authorization: token"));
    EXPECT_EQ ((int)0, (int)req->method().compare("POST"));
    EXPECT_EQ ((int)0, (int)req->writeData().compare("{\"postKey1\":\"postValue1\"}"));
    EXPECT_TRUE (req->inQueue());

    // already in queue
    EXPECT_EQ ((int)RT_FAIL, (int)req->end());
    EXPECT_EQ ((int)RT_FAIL, (int)req->write("123"));
    EXPECT_EQ ((int)RT_FAIL, (int)req->setHeader("Key", "Value"));

    sem_wait(testSem);
  }

  sem_t* testSem;
};

TEST_F(rtHttpRequestTest, rtHttpRequestTests)
{
  ctor_url_test();
  ctor_options_test();
  setHeader_test();
  end_test();
}
