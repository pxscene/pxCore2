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
#include "pxScene2d.h"
#include "test_includes.h" // Needs to be included last

class rtPermissionsTest : public testing::Test
{
public:
  virtual void SetUp()
  {
    mPermissions = new rtPermissions;
    mParentPermissions = new rtPermissions;
    mScene = new pxScene2d(true, NULL);
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

    EXPECT_TRUE (RT_OK == mPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(NULL));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(mParentPermissions));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(NULL));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(mParentPermissions));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(NULL));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(mParentPermissions));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(NULL));

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

    EXPECT_TRUE (RT_OK == mPermissions->set(childPermissions));
    EXPECT_TRUE (RT_OK == mParentPermissions->set(permissions));
    EXPECT_TRUE (RT_OK == mPermissions->setParent(mParentPermissions));

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

  void testMalformedConfig()
  {
    // This bootstrap has no correct items
    // Should default to allow everywhere
    mPermissions = new rtPermissions("http://1.com", "supportfiles/permissions.bad.conf");
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    mPermissions = new rtPermissions("http://2.com", "supportfiles/permissions.bad.conf");
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    mPermissions = new rtPermissions("http://3.com", "supportfiles/permissions.bad.conf");
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    mPermissions = new rtPermissions("http://4.com", "supportfiles/permissions.bad.conf");
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));

    mPermissions = new rtPermissions("http://0.com", "supportfiles/permissions.bad.conf");
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND));
  }

  void testSampleConfig()
  {
    // Bootstrap -> FullTrust
    // Allow everything
    mPermissions = new rtPermissions("https://xfinity.comcast.com", "supportfiles/permissions.sample.conf");
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

    // Bootstrap -> LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    // + Block all applications except browser
    mPermissions = new rtPermissions("https://foo.partner2.com", "supportfiles/permissions.sample.conf");
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

    // Bootstrap -> Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    // + Block application browser
    mPermissions = new rtPermissions("http://example.com", "supportfiles/permissions.sample.conf");
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT));
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
  }

  void testDefaultConfig()
  {
    // This test uses default Bootstrap config
    // "*" : "default"
    mPermissions = new rtPermissions("http://default.web.site");
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_FALSE(allows("anything", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND));

    // "*://localhost" : "local",
    // "*://localhost:*" : "local",
    // "*://127.0.0.1" : "local",
    // "*://127.0.0.1:*" : "local",
    // "*://[::1]" : "local",
    // "*://[::1]:*" : "local",
    // "*://[0:0:0:0:0:0:0:1]" : "local",
    // "*://[0:0:0:0:0:0:0:1]:*" : "local",
    // "file://*" : "local",
    mPermissions = new rtPermissions("ftp://127.0.0.1:9999");
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND));

    // "*://www.pxscene.org" : "pxscene.org",
    // "*://www.pxscene.org:*" : "pxscene.org",
    // "*://pxscene.org" : "pxscene.org",
    // "*://pxscene.org:*" : "pxscene.org",
    mPermissions = new rtPermissions("http://www.pxscene.org");
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://localhost:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://127.0.0.1:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[::1]:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("file:///afile", rtPermissions::DEFAULT));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND));
  }

  void testSceneAppliesPermissions()
  {
    rtObjectRef permissions;
    rtObjectRef p = new rtMapObject();
    rtObjectRef url = new rtMapObject();
    p.set("url", url);
    rtObjectRef serviceManager = new rtMapObject();
    p.set("serviceManager", serviceManager);
    rtObjectRef features = new rtMapObject();
    p.set("features", features);
    rtObjectRef applications = new rtMapObject();
    p.set("applications", applications);
    rtArrayObject* url_block = new rtArrayObject();
    url_block->pushBack("*");
    rtObjectRef url_block_ref = url_block;
    url.set("block", url_block_ref);
    rtArrayObject* url_allow = new rtArrayObject();
    url_allow->pushBack("http://allowed.site.url");
    url_allow->pushBack("http://*.allowed.site.url:80");
    rtObjectRef url_allow_ref = url_allow;
    url.set("allow", url_allow_ref);
    rtArrayObject* serviceManager_block = new rtArrayObject();
    serviceManager_block->pushBack("*");
    rtObjectRef serviceManager_block_ref = serviceManager_block;
    serviceManager.set("block", serviceManager_block_ref);
    rtArrayObject* serviceManager_allow = new rtArrayObject();
    serviceManager_allow->pushBack("org.rdk.allowed_1");
    serviceManager_allow->pushBack("org.rdk.allowed_2");
    rtObjectRef serviceManager_allow_ref = serviceManager_allow;
    serviceManager.set("allow", serviceManager_allow_ref);
    rtArrayObject* features_block = new rtArrayObject();
    features_block->pushBack("*");
    rtObjectRef features_block_ref = features_block;
    features.set("block", features_block_ref);
    rtArrayObject* features_allow = new rtArrayObject();
    features_allow->pushBack("screenshot");
    rtObjectRef features_allow_ref = features_allow;
    features.set("allow", features_allow_ref);
    rtArrayObject* applications_allow = new rtArrayObject();
    applications_allow->pushBack("allowed_wayland_app");
    rtObjectRef applications_allow_ref = applications_allow;
    applications.set("allow", applications_allow_ref);
    permissions = p;

    EXPECT_TRUE (RT_OK == mScene->setPermissions(permissions));

    // Block everything
    // + Allow URLs "http://allowed.com", "http://*.allowed.com:80"
    // + Allow serviceManager-s "org.rdk.allowed_1", "org.rdk.allowed_2"
    // + Allow feature "screenshot"
    // + Allow application "browser"
#ifdef PX_SERVICE_MANAGER
    EXPECT_TRUE (allowsService("org.rdk.allowed_1"));
    EXPECT_TRUE (allowsService("org.rdk.allowed_2"));
    EXPECT_FALSE(allowsService("not.allowed"));
#endif //PX_SERVICE_MANAGER
    EXPECT_TRUE (allowsScreenshot());
    EXPECT_TRUE (allowsLoadArchive("http://allowed.site.url"));
    EXPECT_TRUE (allowsLoadArchive("http://another.allowed.site.url:80"));
    EXPECT_FALSE(allowsLoadArchive("http://not.allowed.block.this"));

    // Same
    // + none features are allowed
    // + none applications are allowed
    features_allow->empty();
    applications_allow->empty();
    EXPECT_TRUE (RT_OK == mScene->setPermissions(permissions));

    EXPECT_FALSE(allowsScreenshot());
  }

  void testWildcardBadExamples()
  {
    mPermissionsMap.clear();
    mPermissionsMap[rtPermissions::wildcard_t("*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("qwerty", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com            ", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("            https://comcast.com", rtPermissions::DEFAULT)] = true;

    EXPECT_EQ (0, test("", "*"));
    EXPECT_EQ (0, test("asdfgh", "*"));
    EXPECT_EQ (0, test("qwerty", "qwerty"));
    EXPECT_EQ (0, test("*", "*"));
    EXPECT_EQ (0, test("***", "*"));
    EXPECT_EQ (0, test("https://comcast.com", "https://comcast.com"));
    EXPECT_EQ (0, test("https://comcast.com            ", "https://comcast.com            "));
    EXPECT_EQ (0, test("            https://comcast.com", "            https://comcast.com"));
  }

  void testWildcardRealExamples()
  {
    mPermissionsMap.clear();
    mPermissionsMap[rtPermissions::wildcard_t("https://", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*comcast.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://comcast.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*/index", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*/index?*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com/index/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*/index?p1=*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://localhost:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://127.0.0.1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[::1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[0:0:0:0:0:0:0:1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://::1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://0:0:0:0:0:0:0:1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://*:*@www.my_site.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://*/index*", rtPermissions::DEFAULT)] = true;

    EXPECT_EQ (0, test("http://", "*://*"));
    EXPECT_EQ (0, test("https://", "https://*"));
    EXPECT_EQ (0, test("https://comcast.com", "https://comcast.com*"));
    EXPECT_EQ (0, test("https://m.xfinity.comcast.com/index", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test("https://m.xfinity.comcast.com/blabla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test("https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*/index?*"));
    EXPECT_EQ (0, test("https://m.comcast.com/", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test("https://comcast.com/index/blahblah", "https://comcast.com/index/*"));
    EXPECT_EQ (0, test("https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test("http://localhost:50050/authService/getDeviceId", "http://localhost:*"));
    EXPECT_EQ (0, test("http://127.0.0.1:50050/authService/getDeviceId", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test("http://[::1]:50050/authService/getDeviceId", "http://[::1]:*"));
    EXPECT_EQ (0, test("http://[0:0:0:0:0:0:0:1]:50050/authService/getDeviceId", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test("http://::1:50050/authService/getDeviceId", "http://::1:*"));
    EXPECT_EQ (0, test("http://0:0:0:0:0:0:0:1:50050/authService/getDeviceId", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test("http://my_email%40gmail.com:password@www.my_site.com", "http://*:*@www.my_site.com"));
    EXPECT_EQ (0, test("http://敷リオワニ内前ヲルホ/index.js", "http://*/index*"));
  }

  void testWildcardRealExamplesExtended()
  {
    mPermissionsMap.clear();
    mPermissionsMap[rtPermissions::wildcard_t("https://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://comcast.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("HTTPS://COMCAST.COM", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*/index?p1=*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://*.comcast.com/*/index", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://localhost:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://localhost:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://127.0.0.1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://127.0.0.1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://127.0.0.1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[::1]", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[::1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[::1]:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[0:0:0:0:0:0:0:1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://[0:0:0:0:0:0:0:1]:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://::1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://::1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://::1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://0:0:0:0:0:0:0:1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://0:0:0:0:0:0:0:1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://0:0:0:0:0:0:0:1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://localhost", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://localhost:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://localhost:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://127.0.0.1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://127.0.0.1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://127.0.0.1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[::1]", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[::1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[::1]:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[0:0:0:0:0:0:0:1]:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://[0:0:0:0:0:0:0:1]:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://::1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://::1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://::1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://0:0:0:0:0:0:0:1", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://0:0:0:0:0:0:0:1:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://0:0:0:0:0:0:0:1:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("foo://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("bar://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("foo://com.comcast.application", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("foo://com.comcast.stateObserver", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("foo://com.comcast.FrameRate", rtPermissions::DEFAULT)] = true;

    EXPECT_EQ (-1,test("http://", ""));
    EXPECT_EQ (0, test("https://comcast.com", "https://*"));
    EXPECT_EQ (0, test("https://m.comcast.com", "https://*.comcast.com"));
    EXPECT_EQ (0, test("https://comcast.com/", "https://comcast.com/*"));
    EXPECT_EQ (0, test("https://m.comcast.com/a/index?p2=1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test("https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test("HTTPS://COMCAST.COM", "HTTPS://COMCAST.COM"));
    EXPECT_EQ (0, test("https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test("https://m.xfinity.comcast.com/bla/bla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test("https://github.com", "*://github.com"));
    EXPECT_EQ (0, test("https://github.com/", "*://github.com/*"));
    EXPECT_EQ (0, test("https://github.com:443", "*://github.com:*"));
    EXPECT_EQ (0, test("https://github.com:443/", "*://github.com:*/*"));
    EXPECT_EQ (-1,test("http://localhost", ""));
    EXPECT_EQ (0, test("http://localhost:50050", "http://localhost:*"));
    EXPECT_EQ (0, test("http://localhost:50050/", "http://localhost:*/*"));
    EXPECT_EQ (0, test("http://127.0.0.1", "http://127.0.0.1"));
    EXPECT_EQ (0, test("http://127.0.0.1:50050", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test("http://127.0.0.1:50050/", "http://127.0.0.1:*/*"));
    EXPECT_EQ (0, test("http://[::1]", "http://[::1]"));
    EXPECT_EQ (0, test("http://[::1]:50050", "http://[::1]:*"));
    EXPECT_EQ (0, test("http://[::1]:50050/", "http://[::1]:*/*"));
    EXPECT_EQ (0, test("http://[0:0:0:0:0:0:0:1]", "http://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test("http://[0:0:0:0:0:0:0:1]:50050", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test("http://[0:0:0:0:0:0:0:1]:50050/", "http://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test("http://::1", "http://::1"));
    EXPECT_EQ (0, test("http://::1:50050", "http://::1:*"));
    EXPECT_EQ (0, test("http://::1:50050/", "http://::1:*/*"));
    EXPECT_EQ (0, test("http://0:0:0:0:0:0:0:1", "http://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test("http://0:0:0:0:0:0:0:1:50050", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test("http://0:0:0:0:0:0:0:1:50050/", "http://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test("https://localhost", "https://localhost"));
    EXPECT_EQ (0, test("https://localhost:50050", "https://localhost:*"));
    EXPECT_EQ (0, test("https://localhost:50050/", "https://localhost:*/*"));
    EXPECT_EQ (0, test("https://127.0.0.1", "https://127.0.0.1"));
    EXPECT_EQ (0, test("https://127.0.0.1:50050", "https://127.0.0.1:*"));
    EXPECT_EQ (0, test("https://127.0.0.1:50050/", "https://127.0.0.1:*/*"));
    EXPECT_EQ (0, test("https://[::1]", "https://[::1]"));
    EXPECT_EQ (0, test("https://[::1]:50050", "https://[::1]:*"));
    EXPECT_EQ (0, test("https://[::1]:50050/", "https://[::1]:*/*"));
    EXPECT_EQ (0, test("https://[0:0:0:0:0:0:0:1]", "https://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test("https://[0:0:0:0:0:0:0:1]:50050", "https://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test("https://[0:0:0:0:0:0:0:1]:50050/", "https://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test("https://::1", "https://::1"));
    EXPECT_EQ (0, test("https://::1:50050", "https://::1:*"));
    EXPECT_EQ (0, test("https://::1:50050/", "https://::1:*/*"));
    EXPECT_EQ (0, test("https://0:0:0:0:0:0:0:1", "https://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test("https://0:0:0:0:0:0:0:1:50050", "https://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test("https://0:0:0:0:0:0:0:1:50050/", "https://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test("foo://api1", "foo://*"));
    EXPECT_EQ (0, test("bar://screenshot", "bar://*"));
    EXPECT_EQ (0, test("foo://com.comcast.application", "foo://com.comcast.application"));
    EXPECT_EQ (0, test("foo://com.comcast.stateObserver", "foo://com.comcast.stateObserver"));
    EXPECT_EQ (0, test("foo://com.comcast.FrameRate", "foo://com.comcast.FrameRate"));
  }

  void testWildcardGithubExamples()
  {
    mPermissionsMap.clear();
    mPermissionsMap[rtPermissions::wildcard_t("*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com:*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com:*/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://github.com/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("https://github.com/pxscene/*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("*://github.com/pxscene/*", rtPermissions::DEFAULT)] = true;

    EXPECT_EQ (0, test("https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "https://github.com/pxscene/*"));
    EXPECT_EQ (0, test("https://github.com:443/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com:*/*"));
    EXPECT_EQ (0, test("http://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com/pxscene/*"));
    EXPECT_EQ (0, test("https://github.com/features", "https://github.com/*"));
    EXPECT_EQ (0, test("https://github.com:443/features", "*://github.com:*/*"));
    EXPECT_EQ (0, test("http://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test("https://github.com/pxscene", "https://github.com/*"));
    EXPECT_EQ (0, test("https://github.com:443/pxscene", "*://github.com:*/*"));
    EXPECT_EQ (0, test("http://github.com/pxscene", "*://github.com/*"));
    EXPECT_EQ (0, test("blablahttps://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test("blablah", "*"));
    EXPECT_EQ (0, test("blablah:/", "*"));
    EXPECT_EQ (0, test("blablah://", "*://*"));
    EXPECT_EQ (0, test("blablah:///", "*://*"));
    EXPECT_EQ (0, test("blablah://abc", "*://*"));
    EXPECT_EQ (0, test("blablah://github.com", "*://github.com"));
    EXPECT_EQ (0, test("blablah://github.com:8080", "*://github.com:*"));
    EXPECT_EQ (0, test("https://google.com", "*://*"));
    EXPECT_EQ (0, test("http://google.com", "*://*"));
    EXPECT_EQ (0, test("github.com", "*"));
    EXPECT_EQ (0, test("github.com://github.com", "*://github.com"));
    EXPECT_EQ (0, test("github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test("github.com:80/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test("ftp://github.com", "*://github.com"));
    EXPECT_EQ (0, test("ftp://github.com:21", "*://github.com:*"));
    EXPECT_EQ (0, test("ftp://github.com:21/", "*://github.com:*/*"));
    EXPECT_EQ (0, test("ftp://github.com:21/pxscene/blabla", "*://github.com:*/*"));
    EXPECT_EQ (0, test("ftp://github.com/pxscene/blabla", "*://github.com/pxscene/*"));
  }

  void testWildcardLocalhostExamples()
  {
    mPermissionsMap.clear();
    mPermissionsMap[rtPermissions::wildcard_t("*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("http://localhost*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("foo://*", rtPermissions::DEFAULT)] = true;
    mPermissionsMap[rtPermissions::wildcard_t("bar://*", rtPermissions::DEFAULT)] = true;

    EXPECT_EQ (0, test("http://localhost/", "http://localhost*"));
  }

private:
  bool allows(const char* url, rtPermissions::Type type)
  {
    return RT_OK == mPermissions->allows(url, type);
  }

  bool allowsScreenshot()
  {
    rtString type("ignore this");
    rtString pngData;
    rtError e = mScene->screenshot(type, pngData);
    return RT_ERROR_NOT_ALLOWED != e;
  }

#ifdef PX_SERVICE_MANAGER
  bool allowsService(const char* name)
  {
    rtString nameStr(name);
    rtObjectRef returnObject;
    rtError e = mScene->getService(nameStr, returnObject);
    return RT_ERROR_NOT_ALLOWED != e;
  }
#endif //PX_SERVICE_MANAGER

  bool allowsLoadArchive(const char* url)
  {
    rtString urlStr(url);
    rtObjectRef archive;
    rtError e = mScene->loadArchive(urlStr, archive);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  // -1 if not found, 0 if 'expectedResult' matches, 1 if 'expectedResult' doesn't match
  int test(const char* url, const char* expectedResult)
  {
    rtPermissions::wildcard_t w;
    w.first = url;
    w.second = rtPermissions::DEFAULT;
    rtPermissions::permissionsMap_t::const_iterator it = rtPermissions::findWildcard(mPermissionsMap, w);
    if (it == mPermissionsMap.end()) {
      return -1;
    }
    if (0 == it->first.first.compare(expectedResult)) {
      return 0;
    }
    rtLogError("differs: actual '%s' expected '%s'", it->first.first.c_str(), expectedResult);
    return 1;
  }

  pxScene2dRef mScene;
  rtPermissionsRef mPermissions;
  rtPermissionsRef mParentPermissions;
  rtPermissions::permissionsMap_t mPermissionsMap;
};

TEST_F(rtPermissionsTest, rtPermissionsTests)
{
  // verify permissions
  testFullAccess();
  testUntrusted();
  testLimitedTrust();
  testRealWorldExample();
  // verify bootstrap config (JSON)
  testMalformedConfig();
  testSampleConfig();
  testDefaultConfig();
  // verify scene applies permissions
  testSceneAppliesPermissions();
  // verify wildcard
  testWildcardBadExamples();
  testWildcardRealExamples();
  testWildcardRealExamplesExtended();
  testWildcardGithubExamples();
  testWildcardLocalhostExamples();
}
