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
#include "test_includes.h" // Needs to be included last

class rtPermissions1Test : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void testFullAccess()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtObjectRef applications = new rtMapObject();
      p.set("applications", applications);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* applications_allow = new rtArrayObject();
      applications_allow->pushBack("browser");
      rtObjectRef applications_allow_ref = applications_allow;
      applications.set("allow", applications_allow_ref);
      permissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(NULL));

    // FullTrust
    // Allow everything
    // NOTE: these blocks of checks have identical URLs across thew whole file. If you change one, change the rest too!
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    rtObjectRef childPermissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("com.comcast.application");
      serviceManager_block->pushBack("com.comcast.stateObserver");
      serviceManager_block->pushBack("com.comcast.FrameRate");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      childPermissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(&mParentPermissions));

    // FullTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 given serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));
  }

  void testUntrusted()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtObjectRef applications = new rtMapObject();
      p.set("applications", applications);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("*");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("*");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("screenshot");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      rtArrayObject* applications_allow = new rtArrayObject();
      applications_allow->pushBack("*");
      rtObjectRef applications_allow_ref = applications_allow;
      applications.set("allow", applications_allow_ref);
      rtArrayObject* applications_block = new rtArrayObject();
      applications_block->pushBack("browser");
      rtObjectRef applications_block_ref = applications_block;
      applications.set("block", applications_block_ref);
      permissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(NULL));

    // Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    // + Block application browser
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("screenshot", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("browser", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    rtObjectRef childPermissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("com.comcast.application");
      serviceManager_block->pushBack("com.comcast.stateObserver");
      serviceManager_block->pushBack("com.comcast.FrameRate");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      childPermissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(&mParentPermissions));

    // Untrusted -> Child
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("browser", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));
  }

  void testLimitedTrust()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtObjectRef applications = new rtMapObject();
      p.set("applications", applications);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("com.comcast.application");
      serviceManager_block->pushBack("com.comcast.stateObserver");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      rtArrayObject* applications_allow = new rtArrayObject();
      applications_allow->pushBack("browser");
      rtObjectRef applications_allow_ref = applications_allow;
      applications.set("allow", applications_allow_ref);
      rtArrayObject* applications_block = new rtArrayObject();
      applications_block->pushBack("*");
      rtObjectRef applications_block_ref = applications_block;
      applications.set("block", applications_block_ref);
      permissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(NULL));

    // LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    // + Block all applications except browser
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("unknown", rtPermissions::WAYLAND));

    rtObjectRef childPermissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("com.comcast.application");
      serviceManager_block->pushBack("com.comcast.stateObserver");
      serviceManager_block->pushBack("com.comcast.FrameRate");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      childPermissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(&mParentPermissions));

    // LimitedTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("unknown", rtPermissions::WAYLAND));
  }

  void testRealWorldExample()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtObjectRef applications = new rtMapObject();
      p.set("applications", applications);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("http://*");
      url_allow->pushBack("https://*");
      url_allow->pushBack("http://localhost:50050");
      url_allow->pushBack("http://127.0.0.1:50050");
      url_allow->pushBack("http://[::1]:50050");
      url_allow->pushBack("http://[0:0:0:0:0:0:0:1]:50050");
      url_allow->pushBack("http://::1:50050");
      url_allow->pushBack("http://0:0:0:0:0:0:0:1:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("*");
      url_block->pushBack("*://*");
      url_block->pushBack("*://github.com");
      url_block->pushBack("*://github.com:*");
      url_block->pushBack("http://localhost");
      url_block->pushBack("http://localhost:*");
      url_block->pushBack("http://127.0.0.1");
      url_block->pushBack("http://127.0.0.1:*");
      url_block->pushBack("http://[::1]");
      url_block->pushBack("http://[::1]:*");
      url_block->pushBack("http://[0:0:0:0:0:0:0:1]");
      url_block->pushBack("http://[0:0:0:0:0:0:0:1]:*");
      url_block->pushBack("http://::1");
      url_block->pushBack("http://::1:*");
      url_block->pushBack("http://0:0:0:0:0:0:0:1");
      url_block->pushBack("http://0:0:0:0:0:0:0:1:*");
      url_block->pushBack("https://localhost");
      url_block->pushBack("https://localhost:*");
      url_block->pushBack("https://127.0.0.1");
      url_block->pushBack("https://127.0.0.1:*");
      url_block->pushBack("https://[::1]");
      url_block->pushBack("https://[::1]:*");
      url_block->pushBack("https://[0:0:0:0:0:0:0:1]");
      url_block->pushBack("https://[0:0:0:0:0:0:0:1]:*");
      url_block->pushBack("https://::1");
      url_block->pushBack("https://::1:*");
      url_block->pushBack("https://0:0:0:0:0:0:0:1");
      url_block->pushBack("https://0:0:0:0:0:0:0:1:*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("com.comcast.application");
      serviceManager_allow->pushBack("com.comcast.stateObserver");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("*");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      rtArrayObject* applications_allow = new rtArrayObject();
      applications_allow->pushBack("browser");
      rtObjectRef applications_allow_ref = applications_allow;
      applications.set("allow", applications_allow_ref);
      rtArrayObject* applications_block = new rtArrayObject();
      applications_block->pushBack("*");
      rtObjectRef applications_block_ref = applications_block;
      applications.set("block", applications_block_ref);
      permissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(NULL));

    // RealWorldExample
    // Block everything
    // + Allow all http://... and https://... except github.com and localhost UNLESS localhost port is 50050 and scheme is http
    // + Allow 2 serviceManager-s
    // + Allow feature screenshot
    // + Allow application browser
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("unknown", rtPermissions::WAYLAND));

    rtObjectRef childPermissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      p.set("serviceManager", serviceManager);
      rtObjectRef features = new rtMapObject();
      p.set("features", features);
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      rtArrayObject* url_block = new rtArrayObject();
      url_block->pushBack("http://localhost*");
      rtObjectRef url_block_ref = url_block;
      url.set("block", url_block_ref);
      rtArrayObject* serviceManager_allow = new rtArrayObject();
      serviceManager_allow->pushBack("*");
      rtObjectRef serviceManager_allow_ref = serviceManager_allow;
      serviceManager.set("allow", serviceManager_allow_ref);
      rtArrayObject* serviceManager_block = new rtArrayObject();
      serviceManager_block->pushBack("com.comcast.application");
      serviceManager_block->pushBack("com.comcast.stateObserver");
      serviceManager_block->pushBack("com.comcast.FrameRate");
      rtObjectRef serviceManager_block_ref = serviceManager_block;
      serviceManager.set("block", serviceManager_block_ref);
      rtArrayObject* features_allow = new rtArrayObject();
      features_allow->pushBack("screenshot");
      rtObjectRef features_allow_ref = features_allow;
      features.set("allow", features_allow_ref);
      rtArrayObject* features_block = new rtArrayObject();
      features_block->pushBack("*");
      rtObjectRef features_block_ref = features_block;
      features.set("block", features_block_ref);
      childPermissions = p;
    }

    EXPECT_TRUE (RT_OK == mPermissions.set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions.set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions.setParent(&mParentPermissions));

    // RealWorldExample -> Child
    // Block everything
    // + Allow http://localhost:50050
    // + Allow feature screenshot
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://example.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://github.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("foo://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com:80", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://github.com:80/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[::1]:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://::1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost/", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost:10004", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("https://localhost:50050", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE));
    EXPECT_FALSE(allows("", rtPermissions::WAYLAND));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND));
    EXPECT_FALSE(allows("unknown", rtPermissions::WAYLAND));
  }

private:
  bool allows(const char* url, rtPermissions::Type type)
  {
    bool a;
    EXPECT_TRUE (RT_OK == mPermissions.allows(url, type, a));
    return a;
  }

  rtPermissions mPermissions;
  rtPermissions mParentPermissions;
};

TEST_F(rtPermissions1Test, rtPermissionsTests)
{
  testFullAccess();
  testUntrusted();
  testLimitedTrust();
  testRealWorldExample();
}
