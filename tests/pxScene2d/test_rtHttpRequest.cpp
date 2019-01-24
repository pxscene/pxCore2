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
    EXPECT_EQ (std::string(req->url().cString()), "https://example.com");
    EXPECT_EQ ((int)0, (int)req->headers().size());
    EXPECT_TRUE (req->method().isEmpty());
    EXPECT_EQ ((int)0, (int)req->writeDataSize());
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
    EXPECT_EQ (std::string(req->url().cString()), "http://httpbin.org/post");
    EXPECT_EQ ((int)2, (int)req->headers().size());
    EXPECT_EQ (std::string(req->headers()[0].cString()), "Content-Type: application/json");
    EXPECT_EQ (std::string(req->headers()[1].cString()), "Authorization: token");
    EXPECT_EQ (std::string(req->method().cString()), "POST");
    EXPECT_EQ (std::string((const char*)req->writeData(), req->writeDataSize()), "{\"postKey1\":\"postValue1\"}");
    EXPECT_FALSE (req->inQueue());
  }

  void setHeader_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    req = new rtHttpRequest("https://example.com");
    req->setHeader("Authorization", "token");
    ref = req;
    EXPECT_EQ (std::string(req->url().cString()), "https://example.com");
    EXPECT_EQ ((int)1, (int)req->headers().size());
    EXPECT_EQ (std::string(req->headers()[0].cString()), "Authorization: token");
    EXPECT_TRUE (req->method().isEmpty());
    EXPECT_EQ ((int)0, (int)req->writeDataSize());
    EXPECT_FALSE (req->inQueue());
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

    UNUSED_PARAM(result);

    sem_post((sem_t*)context);
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

  void response_test()
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

    rtFunctionRef fn1 = new rtFunctionCallback(response_test_callback1, testSem);
    rtFunctionRef fn2 = new rtFunctionCallback(response_test_callback2, testSem);

    req = new rtHttpRequest(options);
    EXPECT_EQ ((int)RT_OK, req->addListener("response", fn1));
    EXPECT_EQ ((int)RT_OK, req->addListener("error", fn2));
    EXPECT_EQ ((int)RT_OK, req->write("{\"postKey1\":\"postValue1\"}"));
    EXPECT_EQ ((int)RT_OK, req->end());
    ref = req;
    EXPECT_EQ (std::string(req->url().cString()), "http://httpbin.org/post");
    EXPECT_EQ ((int)2, (int)req->headers().size());
    EXPECT_EQ (std::string(req->headers()[0].cString()), "Content-Type: application/json");
    EXPECT_EQ (std::string(req->headers()[1].cString()), "Authorization: token");
    EXPECT_EQ (std::string(req->method().cString()), "POST");
    EXPECT_EQ (std::string((const char*)req->writeData(), req->writeDataSize()), "{\"postKey1\":\"postValue1\"}");
    EXPECT_TRUE (req->inQueue());

    // already in queue
    EXPECT_EQ ((int)RT_FAIL, (int)req->end());
    EXPECT_EQ ((int)RT_FAIL, (int)req->write("123"));
    EXPECT_EQ ((int)RT_FAIL, (int)req->setHeader("Key", "Value"));

    sem_wait(testSem);
  }

  static rtError error_test_callback1(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    UNUSED_PARAM(numArgs);
    UNUSED_PARAM(args);
    UNUSED_PARAM(result);

    ADD_FAILURE();

    sem_post((sem_t*)context);
    return RT_OK;
  }

  static rtError error_test_callback2(int numArgs, const rtValue* args, rtValue* result, void* context)
  {
    EXPECT_EQ ((int)1, (int)numArgs);

    rtString error = args[0].toString();
    EXPECT_EQ (std::string(error.cString()), "Download error for:https://this.url.does.not.exist. Error code:6. Using proxy:false ");

    UNUSED_PARAM(result);

    sem_post((sem_t*)context);
    return RT_OK;
  }

  void error_test()
  {
    rtObjectRef ref;
    rtHttpRequest* req;

    rtFunctionRef fn1 = new rtFunctionCallback(error_test_callback1, testSem);
    rtFunctionRef fn2 = new rtFunctionCallback(error_test_callback2, testSem);

    req = new rtHttpRequest("https://this.url.does.not.exist");
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
    rtHttpRequest* req;

    int times_fn1_called = 0;
    int times_fn2_called = 0;
    int times_fn3_called = 0;
    rtFunctionRef fn1 = new rtFunctionCallback(addListener_test_callback, &times_fn1_called);
    rtFunctionRef fn2 = new rtFunctionCallback(addListener_test_callback, &times_fn2_called);
    rtFunctionRef fn3 = new rtFunctionCallback(addListener_test_callback, &times_fn3_called);

    req = new rtHttpRequest("https://example.com");
    EXPECT_EQ ((int)RT_OK, req->addListener("response", fn1));
    EXPECT_EQ ((int)RT_OK, req->once("response", fn2));
    EXPECT_EQ ((int)RT_OK, req->addListener("error", fn3));
    ref = req;

    rtFileDownloadRequest* dwnl_OK = new rtFileDownloadRequest("https://example.com", req, rtHttpRequest::onDownloadComplete);

    rtHttpRequest::onDownloadComplete(dwnl_OK);
    EXPECT_EQ (1, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    rtHttpRequest::onDownloadComplete(dwnl_OK);
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    EXPECT_EQ ((int)RT_OK, req->removeAllListenersByName("response"));

    rtHttpRequest::onDownloadComplete(dwnl_OK);
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (0, times_fn3_called);

    rtFileDownloadRequest* dwnl_ERR = new rtFileDownloadRequest("https://example.com", req, rtHttpRequest::onDownloadComplete);
    dwnl_ERR->setErrorString("ERR test");

    rtHttpRequest::onDownloadComplete(dwnl_ERR);
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (1, times_fn3_called);

    rtHttpRequest::onDownloadComplete(dwnl_ERR);
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (2, times_fn3_called);

    EXPECT_EQ ((int)RT_OK, req->removeAllListeners());

    rtHttpRequest::onDownloadComplete(dwnl_ERR);
    EXPECT_EQ (2, times_fn1_called);
    EXPECT_EQ (1, times_fn2_called);
    EXPECT_EQ (2, times_fn3_called);

    delete dwnl_OK;
    delete dwnl_ERR;
  }

  sem_t* testSem;
};

TEST_F(rtHttpRequestTest, rtHttpRequestTests)
{
  ctor_url_test();
  ctor_options_test();
  setHeader_test();
  response_test();
  error_test();
  addListener_test();
}
