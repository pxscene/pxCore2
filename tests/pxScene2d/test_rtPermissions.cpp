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

#include "rtPermissions.h"
#include "rtUrlUtils.h"

#include "test_includes.h" // Needs to be included last

class rtPermissionsTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test_parentChild_1()
  {
    const char* example1 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ]\n"
      "  },\n"
      "  \"applications\": {\n"
      "    \"allow\": [\n"
      "      \"browser\"\n"
      "    ]\n"
      "  }\n"
      "}";

    const char* example2 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\",\n"
      "      \"http://localhost:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\",\n"
      "      \"com.comcast.FrameRate\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    rtPermissionsRef p1 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p1->set(example1));
    EXPECT_TRUE (RT_OK == p1->setParent(NULL));
    rtPermissionsRef p2 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p2->set(example2));
    EXPECT_TRUE (RT_OK == p2->setParent(p1));

    // FullTrust
    // Allow everything
    // NOTE: these blocks of checks have identical URLs across thew whole file. If you change one, change the rest too!
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // FullTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 given serviceManager-s
    // + Block all features except screenshot
    EXPECT_EQ ((int)RT_OK, (int)p2->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::WAYLAND));
  }

  void test_parentChild_2()
  {
    const char* example1 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"screenshot\"\n"
      "    ]\n"
      "  },\n"
      "  \"applications\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"browser\"\n"
      "    ]\n"
      "  }\n"
      "}";

    const char* example2 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\",\n"
      "      \"http://localhost:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\",\n"
      "      \"com.comcast.FrameRate\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    rtPermissionsRef p1 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p1->set(example1));
    EXPECT_TRUE (RT_OK == p1->setParent(NULL));
    rtPermissionsRef p2 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p2->set(example2));
    EXPECT_TRUE (RT_OK == p2->setParent(p1));

    // Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    // + Block application browser
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // Untrusted -> Child
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block all features except screenshot
    EXPECT_EQ ((int)RT_OK, (int)p2->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::WAYLAND));
  }

  void test_parentChild_3()
  {
    const char* example1 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\",\n"
      "      \"http://localhost:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"applications\": {\n"
      "    \"allow\": [\n"
      "      \"browser\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    const char* example2 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\",\n"
      "      \"http://localhost:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\",\n"
      "      \"com.comcast.FrameRate\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    rtPermissionsRef p1 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p1->set(example1));
    EXPECT_TRUE (RT_OK == p1->setParent(NULL));
    rtPermissionsRef p2 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p2->set(example2));
    EXPECT_TRUE (RT_OK == p2->setParent(p1));

    // LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    // + Block all applications except browser
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // LimitedTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 serviceManager-s
    // + Block all features except screenshot
    EXPECT_EQ ((int)RT_OK, (int)p2->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::WAYLAND));
  }

  void test_parentChild_4()
  {
    const char* example1 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"http://*\",\n"
      "      \"https://*\",\n"
      "      \"http://localhost:50050\",\n"
      "      \"http://127.0.0.1:50050\",\n"
      "      \"http://[::1]:50050\",\n"
      "      \"http://[0:0:0:0:0:0:0:1]:50050\",\n"
      "      \"http://::1:50050\",\n"
      "      \"http://0:0:0:0:0:0:0:1:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\",\n"
      "      \"*://*\",\n"
      "      \"*://github.com\",\n"
      "      \"*://github.com:*\",\n"
      "      \"http://localhost\",\n"
      "      \"http://localhost:*\",\n"
      "      \"http://127.0.0.1\",\n"
      "      \"http://127.0.0.1:*\",\n"
      "      \"http://[::1]\",\n"
      "      \"http://[::1]:*\",\n"
      "      \"http://[0:0:0:0:0:0:0:1]\",\n"
      "      \"http://[0:0:0:0:0:0:0:1]:*\",\n"
      "      \"http://::1\",\n"
      "      \"http://::1:*\",\n"
      "      \"http://0:0:0:0:0:0:0:1\",\n"
      "      \"http://0:0:0:0:0:0:0:1:*\",\n"
      "      \"https://localhost\",\n"
      "      \"https://localhost:*\",\n"
      "      \"https://127.0.0.1\",\n"
      "      \"https://127.0.0.1:*\",\n"
      "      \"https://[::1]\",\n"
      "      \"https://[::1]:*\",\n"
      "      \"https://[0:0:0:0:0:0:0:1]\",\n"
      "      \"https://[0:0:0:0:0:0:0:1]:*\",\n"
      "      \"https://::1\",\n"
      "      \"https://::1:*\",\n"
      "      \"https://0:0:0:0:0:0:0:1\",\n"
      "      \"https://0:0:0:0:0:0:0:1:*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  },\n"
      "  \"applications\": {\n"
      "    \"allow\": [\n"
      "      \"browser\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    const char* example2 =
      "{\n"
      "  \"url\": {\n"
      "    \"allow\": [\n"
      "      \"*\",\n"
      "      \"http://localhost:50050\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"http://localhost*\"\n"
      "    ]\n"
      "  },\n"
      "  \"serviceManager\": {\n"
      "    \"allow\": [\n"
      "      \"*\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"com.comcast.application\",\n"
      "      \"com.comcast.stateObserver\",\n"
      "      \"com.comcast.FrameRate\"\n"
      "    ]\n"
      "  },\n"
      "  \"features\": {\n"
      "    \"allow\": [\n"
      "      \"screenshot\"\n"
      "    ],\n"
      "    \"block\": [\n"
      "      \"*\"\n"
      "    ]\n"
      "  }\n"
      "}";

    rtPermissionsRef p1 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p1->set(example1));
    EXPECT_TRUE (RT_OK == p1->setParent(NULL));
    rtPermissionsRef p2 = new rtPermissions;
    EXPECT_TRUE (RT_OK == p2->set(example2));
    EXPECT_TRUE (RT_OK == p2->setParent(p1));

    // RealWorldExample
    // Block everything
    // + Allow all http://... and https://... except github.com and localhost UNLESS localhost port is 50050 and scheme is http
    // + Allow 2 serviceManager-s
    // + Allow feature screenshot
    // + Allow application browser
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // RealWorldExample -> Child
    // Block everything
    // + Allow http://localhost:50050
    // + Allow feature screenshot
    EXPECT_EQ ((int)RT_OK, (int)p2->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p2->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p2->allows("unknown", rtPermissions::WAYLAND));
  }

  void test_supportfilesBadConf()
  {
    // This bootstrap is valid json but has no correct items in roles
    // Should default to block everything
    rtPermissions::init("supportfiles/permissions.bad.conf");

    rtPermissionsRef p1 = new rtPermissions("http://1.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    p1 = new rtPermissions("http://2.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    p1 = new rtPermissions("http://3.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    p1 = new rtPermissions("http://4.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    p1 = new rtPermissions("http://0.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // reset
    rtPermissions::init();
  }

  void test_supportfilesSampleConf()
  {
    rtPermissions::init("supportfiles/permissions.sample.conf");

    // Bootstrap -> FullTrust
    // Allow everything
    rtPermissionsRef p1 = new rtPermissions("https://xfinity.comcast.com");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // Bootstrap -> LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    // + Block all applications except browser
    p1 = new rtPermissions("https://foo.partner2.com");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // Bootstrap -> Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    // + Block application browser
    p1 = new rtPermissions("http://example.com");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows(NULL, rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("screenshot", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("browser", rtPermissions::WAYLAND));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("unknown", rtPermissions::WAYLAND));

    // reset
    rtPermissions::init();
  }

  void test_supportfilesSample2Conf()
  {
    rtPermissions::init("supportfiles/permissions.sample2.conf");
    rtPermissionsRef p1;

    // "*://*.xreapps.net": "xreapps.net",
    // "*://*.xreapps.net:*": "xreapps.net"

    p1 = new rtPermissions("http://foo.com/testing/://123.xreapps.net/securityHole.js");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));
    p1 = new rtPermissions("http://123.xreapps.net/testing/://foo.com/securityHole.js");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));

    p1 = new rtPermissions("http://foo.com?x=http://123.xreapps.net/securityHole.js");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));
    p1 = new rtPermissions("http://123.xreapps.net?x=http://foo.com/securityHole.js");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));

    p1 = new rtPermissions("http://123.xreapps.net@foo.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));
    p1 = new rtPermissions("http://foo.com@123.xreapps.net");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));

    p1 = new rtPermissions("http://123.xreapps.net:80@foo.com:80");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));
    p1 = new rtPermissions("http://foo.com:80@123.xreapps.net:80");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));

    p1 = new rtPermissions("http://123.xreapps.net%40123.xreapps.net:123.xreapps.net@foo.com");
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));
    p1 = new rtPermissions("http://foo.com%40foo.com:foo.com@123.xreapps.net");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://456.xreapps.net", rtPermissions::DEFAULT));

    //    "allow": [
    //      "*://*.xreapps.net",
    //      "*://*.xreapps.net:*"
    //    ],

    p1 = new rtPermissions("http://123.xreapps.net");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://foo.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://123.xreapps.net/://foo.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://foo.com/://123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://123.xreapps.net?x=http://foo.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://foo.com?x=http://123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://foo.com@123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://123.xreapps.net@foo.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://foo.com:foo.com@123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://123.xreapps.net:123.xreapps.net@foo.com", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://foo.com%40foo.com:foo.com@123.xreapps.net", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://123.xreapps.net%40123.xreapps.net:123.xreapps.net@foo.com", rtPermissions::DEFAULT));

    // reset
    rtPermissions::init();
  }

  void test_defaultConf()
  {
    // This test uses default Bootstrap config
    rtPermissions::init();

    // "*" : "default"
    rtPermissionsRef p1 = new rtPermissions("http://default.web.site");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_ERROR_NOT_ALLOWED, (int)p1->allows("anything", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::WAYLAND));

    // "*://localhost" : "local",
    // "*://localhost:*" : "local",
    // "*://127.0.0.1" : "local",
    // "*://127.0.0.1:*" : "local",
    // "*://[::1]" : "local",
    // "*://[::1]:*" : "local",
    // "*://[0:0:0:0:0:0:0:1]" : "local",
    // "*://[0:0:0:0:0:0:0:1]:*" : "local",
    // "file://*" : "local",
    p1 = new rtPermissions("ftp://127.0.0.1:9999");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::WAYLAND));

    // "*://www.pxscene.org" : "pxscene.org",
    // "*://www.pxscene.org:*" : "pxscene.org",
    // "*://pxscene.org" : "pxscene.org",
    // "*://pxscene.org:*" : "pxscene.org",
    p1 = new rtPermissions("http://www.pxscene.org");
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::SERVICE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::FEATURE));
    EXPECT_EQ ((int)RT_OK, (int)p1->allows("anything", rtPermissions::WAYLAND));
  }

  void test_find_1()
  {
    const char* example =
      "{\n"
      "  \"*\": \"\",\n"
      "  \"qwerty\": \"\",\n"
      "  \"https://comcast.com\": \"\",\n"
      "  \"https://comcast.com            \": \"\",\n"
      "  \"            https://comcast.com\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "asdfgh", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "qwerty", s));
    EXPECT_EQ (std::string(s), "qwerty");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "*", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "***", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com", s));
    EXPECT_EQ (std::string(s), "https://comcast.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com            ", s));
    EXPECT_EQ (std::string(s), "https://comcast.com            ");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "            https://comcast.com", s));
    EXPECT_EQ (std::string(s), "            https://comcast.com");
  }

  void test_find_2()
  {
    const char* example =
      "{\n"
      "  \"https://\": \"\",\n"
      "  \"https://*\": \"\",\n"
      "  \"*://*\": \"\",\n"
      "  \"https://comcast.com\": \"\",\n"
      "  \"https://comcast.com*\": \"\",\n"
      "  \"https://*comcast.com\": \"\",\n"
      "  \"*://comcast.com\": \"\",\n"
      "  \"https://*.comcast.com/*\": \"\",\n"
      "  \"https://*.comcast.com/*/index\": \"\",\n"
      "  \"https://*.comcast.com/*/index?*\": \"\",\n"
      "  \"https://comcast.com/index/*\": \"\",\n"
      "  \"https://*.comcast.com/*/index?p1=*\": \"\",\n"
      "  \"http://localhost:*\": \"\",\n"
      "  \"http://127.0.0.1:*\": \"\",\n"
      "  \"http://[::1]:*\": \"\",\n"
      "  \"http://[0:0:0:0:0:0:0:1]:*\": \"\",\n"
      "  \"http://::1:*\": \"\",\n"
      "  \"http://0:0:0:0:0:0:0:1:*\": \"\",\n"
      "  \"http://*:*@www.my_site.com\": \"\",\n"
      "  \"http://*/index*\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://", s));
    EXPECT_EQ (std::string(s), "https://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com", s));
    EXPECT_EQ (std::string(s.cString()), "https://*comcast.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.xfinity.comcast.com/index", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.xfinity.comcast.com/blabla/index", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*/index");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*/index?*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.comcast.com/", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com/index/blahblah", s));
    EXPECT_EQ (std::string(s), "https://comcast.com/index/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.comcast.com/a/index?p1=1", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*/index?p1=*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://localhost:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://localhost:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://127.0.0.1:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://127.0.0.1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[::1]:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://[::1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[0:0:0:0:0:0:0:1]:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://[0:0:0:0:0:0:0:1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://::1:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://::1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://0:0:0:0:0:0:0:1:50050/authService/getDeviceId", s));
    EXPECT_EQ (std::string(s), "http://0:0:0:0:0:0:0:1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://my_email%40gmail.com:password@www.my_site.com", s));
    EXPECT_EQ (std::string(s), "http://*:*@www.my_site.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://敷リオワニ内前ヲルホ/index.js", s));
    EXPECT_EQ (std::string(s), "http://*/index*");
  }

  void test_find_3()
  {
    const char* example =
      "{\n"
      "  \"https://*\": \"\",\n"
      "  \"https://*.comcast.com\": \"\",\n"
      "  \"https://comcast.com/*\": \"\",\n"
      "  \"HTTPS://COMCAST.COM\": \"\",\n"
      "  \"https://*.comcast.com/*\": \"\",\n"
      "  \"https://*.comcast.com/*/index?p1=*\": \"\",\n"
      "  \"https://*.comcast.com/*/index\": \"\",\n"
      "  \"*://github.com\": \"\",\n"
      "  \"*://github.com/*\": \"\",\n"
      "  \"*://github.com:*\": \"\",\n"
      "  \"*://github.com:*/*\": \"\",\n"
      "  \"http://localhost:*\": \"\",\n"
      "  \"http://localhost:*/*\": \"\",\n"
      "  \"http://127.0.0.1\": \"\",\n"
      "  \"http://127.0.0.1:*\": \"\",\n"
      "  \"http://127.0.0.1:*/*\": \"\",\n"
      "  \"http://[::1]\": \"\",\n"
      "  \"http://[::1]:*\": \"\",\n"
      "  \"http://[::1]:*/*\": \"\",\n"
      "  \"http://[0:0:0:0:0:0:0:1]\": \"\",\n"
      "  \"http://[0:0:0:0:0:0:0:1]:*\": \"\",\n"
      "  \"http://[0:0:0:0:0:0:0:1]:*/*\": \"\",\n"
      "  \"http://::1\": \"\",\n"
      "  \"http://::1:*\": \"\",\n"
      "  \"http://::1:*/*\": \"\",\n"
      "  \"http://0:0:0:0:0:0:0:1\": \"\",\n"
      "  \"http://0:0:0:0:0:0:0:1:*\": \"\",\n"
      "  \"http://0:0:0:0:0:0:0:1:*/*\": \"\",\n"
      "  \"https://localhost\": \"\",\n"
      "  \"https://localhost:*\": \"\",\n"
      "  \"https://localhost:*/*\": \"\",\n"
      "  \"https://127.0.0.1\": \"\",\n"
      "  \"https://127.0.0.1:*\": \"\",\n"
      "  \"https://127.0.0.1:*/*\": \"\",\n"
      "  \"https://[::1]\": \"\",\n"
      "  \"https://[::1]:*\": \"\",\n"
      "  \"https://[::1]:*/*\": \"\",\n"
      "  \"https://[0:0:0:0:0:0:0:1]\": \"\",\n"
      "  \"https://[0:0:0:0:0:0:0:1]:*\": \"\",\n"
      "  \"https://[0:0:0:0:0:0:0:1]:*/*\": \"\",\n"
      "  \"https://::1\": \"\",\n"
      "  \"https://::1:*\": \"\",\n"
      "  \"https://::1:*/*\": \"\",\n"
      "  \"https://0:0:0:0:0:0:0:1\": \"\",\n"
      "  \"https://0:0:0:0:0:0:0:1:*\": \"\",\n"
      "  \"https://0:0:0:0:0:0:0:1:*/*\": \"\",\n"
      "  \"foo://*\": \"\",\n"
      "  \"bar://*\": \"\",\n"
      "  \"foo://com.comcast.application\": \"\",\n"
      "  \"foo://com.comcast.stateObserver\": \"\",\n"
      "  \"foo://com.comcast.FrameRate\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_PROP_NOT_FOUND, (int)rtPermissions::find(obj, "http://", s));
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com", s));
    EXPECT_EQ (std::string(s), "https://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.comcast.com", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://comcast.com/", s));
    EXPECT_EQ (std::string(s), "https://comcast.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.comcast.com/a/index?p2=1", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.comcast.com/a/index?p1=1", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*/index?p1=*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "HTTPS://COMCAST.COM", s));
    EXPECT_EQ (std::string(s), "HTTPS://COMCAST.COM");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://m.xfinity.comcast.com/bla/bla/index", s));
    EXPECT_EQ (std::string(s), "https://*.comcast.com/*/index");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com", s));
    EXPECT_EQ (std::string(s), "*://github.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com/", s));
    EXPECT_EQ (std::string(s), "*://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com:443", s));
    EXPECT_EQ (std::string(s), "*://github.com:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com:443/", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_PROP_NOT_FOUND, (int)rtPermissions::find(obj, "http://localhost", s));
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://localhost:50050", s));
    EXPECT_EQ (std::string(s), "http://localhost:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://localhost:50050/", s));
    EXPECT_EQ (std::string(s), "http://localhost:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://127.0.0.1", s));
    EXPECT_EQ (std::string(s), "http://127.0.0.1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://127.0.0.1:50050", s));
    EXPECT_EQ (std::string(s), "http://127.0.0.1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://127.0.0.1:50050/", s));
    EXPECT_EQ (std::string(s), "http://127.0.0.1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[::1]", s));
    EXPECT_EQ (std::string(s), "http://[::1]");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[::1]:50050", s));
    EXPECT_EQ (std::string(s), "http://[::1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[::1]:50050/", s));
    EXPECT_EQ (std::string(s), "http://[::1]:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[0:0:0:0:0:0:0:1]", s));
    EXPECT_EQ (std::string(s), "http://[0:0:0:0:0:0:0:1]");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[0:0:0:0:0:0:0:1]:50050", s));
    EXPECT_EQ (std::string(s), "http://[0:0:0:0:0:0:0:1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://[0:0:0:0:0:0:0:1]:50050/", s));
    EXPECT_EQ (std::string(s), "http://[0:0:0:0:0:0:0:1]:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://::1", s));
    EXPECT_EQ (std::string(s), "http://::1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://::1:50050", s));
    EXPECT_EQ (std::string(s), "http://::1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://::1:50050/", s));
    EXPECT_EQ (std::string(s), "http://::1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://0:0:0:0:0:0:0:1", s));
    EXPECT_EQ (std::string(s), "http://0:0:0:0:0:0:0:1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://0:0:0:0:0:0:0:1:50050", s));
    EXPECT_EQ (std::string(s), "http://0:0:0:0:0:0:0:1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://0:0:0:0:0:0:0:1:50050/", s));
    EXPECT_EQ (std::string(s), "http://0:0:0:0:0:0:0:1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://localhost", s));
    EXPECT_EQ (std::string(s), "https://localhost");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://localhost:50050", s));
    EXPECT_EQ (std::string(s), "https://localhost:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://localhost:50050/", s));
    EXPECT_EQ (std::string(s), "https://localhost:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://127.0.0.1", s));
    EXPECT_EQ (std::string(s), "https://127.0.0.1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://127.0.0.1:50050", s));
    EXPECT_EQ (std::string(s), "https://127.0.0.1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://127.0.0.1:50050/", s));
    EXPECT_EQ (std::string(s), "https://127.0.0.1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[::1]", s));
    EXPECT_EQ (std::string(s), "https://[::1]");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[::1]:50050", s));
    EXPECT_EQ (std::string(s), "https://[::1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[::1]:50050/", s));
    EXPECT_EQ (std::string(s), "https://[::1]:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[0:0:0:0:0:0:0:1]", s));
    EXPECT_EQ (std::string(s), "https://[0:0:0:0:0:0:0:1]");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[0:0:0:0:0:0:0:1]:50050", s));
    EXPECT_EQ (std::string(s), "https://[0:0:0:0:0:0:0:1]:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://[0:0:0:0:0:0:0:1]:50050/", s));
    EXPECT_EQ (std::string(s), "https://[0:0:0:0:0:0:0:1]:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://::1", s));
    EXPECT_EQ (std::string(s), "https://::1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://::1:50050", s));
    EXPECT_EQ (std::string(s), "https://::1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://::1:50050/", s));
    EXPECT_EQ (std::string(s), "https://::1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://0:0:0:0:0:0:0:1", s));
    EXPECT_EQ (std::string(s), "https://0:0:0:0:0:0:0:1");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://0:0:0:0:0:0:0:1:50050", s));
    EXPECT_EQ (std::string(s), "https://0:0:0:0:0:0:0:1:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://0:0:0:0:0:0:0:1:50050/", s));
    EXPECT_EQ (std::string(s), "https://0:0:0:0:0:0:0:1:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "foo://api1", s));
    EXPECT_EQ (std::string(s), "foo://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "bar://screenshot", s));
    EXPECT_EQ (std::string(s), "bar://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "foo://com.comcast.application", s));
    EXPECT_EQ (std::string(s), "foo://com.comcast.application");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "foo://com.comcast.stateObserver", s));
    EXPECT_EQ (std::string(s), "foo://com.comcast.stateObserver");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "foo://com.comcast.FrameRate", s));
    EXPECT_EQ (std::string(s), "foo://com.comcast.FrameRate");
  }

  void test_find_4()
  {
    const char* example =
      "{\n"
      "  \"*\": \"\",\n"
      "  \"*://*\": \"\",\n"
      "  \"*://github.com\": \"\",\n"
      "  \"*://github.com/*\": \"\",\n"
      "  \"*://github.com:*\": \"\",\n"
      "  \"*://github.com:*/*\": \"\",\n"
      "  \"https://github.com/*\": \"\",\n"
      "  \"https://github.com/pxscene/*\": \"\",\n"
      "  \"*://github.com/pxscene/*\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", s));
    EXPECT_EQ (std::string(s), "https://github.com/pxscene/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com:443/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", s));
    EXPECT_EQ (std::string(s), "*://github.com/pxscene/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com/features", s));
    EXPECT_EQ (std::string(s), "https://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com:443/features", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://github.com/features", s));
    EXPECT_EQ (std::string(s), "*://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com/pxscene", s));
    EXPECT_EQ (std::string(s), "https://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://github.com:443/pxscene", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://github.com/pxscene", s));
    EXPECT_EQ (std::string(s), "*://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablahttps://github.com/features", s));
    EXPECT_EQ (std::string(s), "*://github.com/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah:/", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah://", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah:///", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah://abc", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah://github.com", s));
    EXPECT_EQ (std::string(s), "*://github.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "blablah://github.com:8080", s));
    EXPECT_EQ (std::string(s), "*://github.com:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "https://google.com", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://google.com", s));
    EXPECT_EQ (std::string(s), "*://*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "github.com", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "github.com://github.com", s));
    EXPECT_EQ (std::string(s), "*://github.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "github.com:80/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", s));
    EXPECT_EQ (std::string(s), "*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "ftp://github.com", s));
    EXPECT_EQ (std::string(s), "*://github.com");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "ftp://github.com:21", s));
    EXPECT_EQ (std::string(s), "*://github.com:*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "ftp://github.com:21/", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "ftp://github.com:21/pxscene/blabla", s));
    EXPECT_EQ (std::string(s), "*://github.com:*/*");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "ftp://github.com/pxscene/blabla", s));
    EXPECT_EQ (std::string(s), "*://github.com/pxscene/*");
  }

  void test_find_5()
  {
    const char* example =
      "{\n"
      "  \"*\": \"\",\n"
      "  \"http://localhost:50050/authService/getDeviceId\": \"\",\n"
      "  \"http://localhost*\": \"\",\n"
      "  \"foo://*\": \"\",\n"
      "  \"bar://*\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "http://localhost/", s));
    EXPECT_EQ (std::string(s), "http://localhost*");
  }

  void test_find_6()
  {
    const char* example =
      "{\n"
      "  \"x\": \"\",\n"
      "  \"*\": \"\",\n"
      "  \"z\": \"\"\n"
      "}";

    rtObjectRef obj;
    EXPECT_TRUE (RT_OK == rtPermissions::json2obj(example, obj));

    rtString s;
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "x", s));
    EXPECT_EQ (std::string(s), "x");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "z", s));
    EXPECT_EQ (std::string(s), "z");
    EXPECT_EQ ((int)RT_OK, (int)rtPermissions::find(obj, "X", s));
    EXPECT_EQ (std::string(s), "*");
  }
};

TEST_F(rtPermissionsTest, rtPermissionsTests)
{
  test_parentChild_1();
  test_parentChild_2();
  test_parentChild_3();
  test_parentChild_4();
  test_supportfilesBadConf();
  test_supportfilesSampleConf();
  test_supportfilesSample2Conf();
  test_defaultConf();
  test_find_1();
  test_find_2();
  test_find_3();
  test_find_4();
  test_find_5();
  test_find_6();
}
