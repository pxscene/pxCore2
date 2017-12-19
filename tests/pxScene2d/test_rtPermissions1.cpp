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
      permissions = p;
    }

    // FullTrust
    // Allow everything
    // NOTE: these blocks of checks have identical URLs across thew whole file. If you change one, change the rest too!
    EXPECT_TRUE (allows(NULL, permissions, NULL));
    EXPECT_TRUE (allows("", permissions, NULL));
    EXPECT_TRUE (allows("foo", permissions, NULL));
    EXPECT_TRUE (allows("foo://", permissions, NULL));
    EXPECT_TRUE (allows("foo://example.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://github.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://localhost", permissions, NULL));
    EXPECT_TRUE (allows("http://", permissions, NULL));
    EXPECT_TRUE (allows("https://", permissions, NULL));
    EXPECT_TRUE (allows("http://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80/", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost/", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:50050", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.application", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.stateObserver", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.FrameRate", permissions, NULL));
    EXPECT_TRUE (allows("feature://", permissions, NULL));
    EXPECT_TRUE (allows("feature://screenshot", permissions, NULL));
    EXPECT_TRUE (allows("feature://unknown", permissions, NULL));

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

    // FullTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 given serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, childPermissions, permissions));
    EXPECT_TRUE (allows("", childPermissions, permissions));
    EXPECT_TRUE (allows("foo", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("http://", childPermissions, permissions));
    EXPECT_TRUE (allows("https://", childPermissions, permissions));
    EXPECT_TRUE (allows("http://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:50050", childPermissions, permissions));
    EXPECT_TRUE (allows("serviceManager://", childPermissions, permissions));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://", childPermissions, permissions));
    EXPECT_TRUE (allows("feature://screenshot", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://unknown", childPermissions, permissions));
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
      permissions = p;
    }

    // Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    EXPECT_TRUE (allows(NULL, permissions, NULL));
    EXPECT_TRUE (allows("", permissions, NULL));
    EXPECT_TRUE (allows("foo", permissions, NULL));
    EXPECT_TRUE (allows("foo://", permissions, NULL));
    EXPECT_TRUE (allows("foo://example.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://github.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://localhost", permissions, NULL));
    EXPECT_TRUE (allows("http://", permissions, NULL));
    EXPECT_TRUE (allows("https://", permissions, NULL));
    EXPECT_TRUE (allows("http://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:10004", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050/authService", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:50050", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://org.rdk.soundPlayer_1", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", permissions, NULL));
    EXPECT_TRUE (allows("feature://", permissions, NULL));
    EXPECT_FALSE(allows("feature://screenshot", permissions, NULL));
    EXPECT_TRUE (allows("feature://unknown", permissions, NULL));

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

    // Untrusted -> Child
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block all features
    EXPECT_TRUE (allows(NULL, childPermissions, permissions));
    EXPECT_TRUE (allows("", childPermissions, permissions));
    EXPECT_TRUE (allows("foo", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("http://", childPermissions, permissions));
    EXPECT_TRUE (allows("https://", childPermissions, permissions));
    EXPECT_TRUE (allows("http://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:10004", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050/authService", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:50050", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://org.rdk.soundPlayer_1", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://screenshot", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://unknown", childPermissions, permissions));
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
      permissions = p;
    }

    // LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, permissions, NULL));
    EXPECT_TRUE (allows("", permissions, NULL));
    EXPECT_TRUE (allows("foo", permissions, NULL));
    EXPECT_TRUE (allows("foo://", permissions, NULL));
    EXPECT_TRUE (allows("foo://example.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://github.com", permissions, NULL));
    EXPECT_TRUE (allows("foo://localhost", permissions, NULL));
    EXPECT_TRUE (allows("http://", permissions, NULL));
    EXPECT_TRUE (allows("https://", permissions, NULL));
    EXPECT_TRUE (allows("http://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80", permissions, NULL));
    EXPECT_TRUE (allows("https://github.com:80/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost/", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("https://localhost:50050", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.FrameRate", permissions, NULL));
    EXPECT_FALSE(allows("feature://", permissions, NULL));
    EXPECT_TRUE (allows("feature://screenshot", permissions, NULL));
    EXPECT_FALSE(allows("feature://unknown", permissions, NULL));

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

    // LimitedTrust -> Child
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 3 serviceManager-s
    // + Block all features except screenshot
    EXPECT_TRUE (allows(NULL, childPermissions, permissions));
    EXPECT_TRUE (allows("", childPermissions, permissions));
    EXPECT_TRUE (allows("foo", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("foo://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("http://", childPermissions, permissions));
    EXPECT_TRUE (allows("https://", childPermissions, permissions));
    EXPECT_TRUE (allows("http://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80", childPermissions, permissions));
    EXPECT_TRUE (allows("https://github.com:80/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost/", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("https://localhost:50050", childPermissions, permissions));
    EXPECT_TRUE (allows("serviceManager://", childPermissions, permissions));
    EXPECT_TRUE (allows("serviceManager://org.rdk.soundPlayer_1", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://", childPermissions, permissions));
    EXPECT_TRUE (allows("feature://screenshot", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://unknown", childPermissions, permissions));
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
      permissions = p;
    }

    // RealWorldExample
    // Block everything
    // + Allow all http://... and https://... except github.com and localhost UNLESS localhost port is 50050 and scheme is http
    // + Allow 2 serviceManager-s
    // + Allow feature screenshot
    EXPECT_TRUE (allows(NULL, permissions, NULL));
    EXPECT_TRUE (allows("", permissions, NULL));
    EXPECT_TRUE (allows("foo", permissions, NULL));
    EXPECT_FALSE(allows("foo://", permissions, NULL));
    EXPECT_FALSE(allows("foo://example.com", permissions, NULL));
    EXPECT_FALSE(allows("foo://github.com", permissions, NULL));
    EXPECT_FALSE(allows("foo://localhost", permissions, NULL));
    EXPECT_TRUE (allows("http://", permissions, NULL));
    EXPECT_TRUE (allows("https://", permissions, NULL));
    EXPECT_TRUE (allows("http://example.com", permissions, NULL));
    EXPECT_TRUE (allows("https://example.com", permissions, NULL));
    EXPECT_FALSE(allows("https://github.com", permissions, NULL));
    EXPECT_FALSE(allows("https://github.com:80", permissions, NULL));
    EXPECT_FALSE(allows("https://github.com:80/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost/", permissions, NULL));
    EXPECT_FALSE(allows("http://localhost:10004", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", permissions, NULL));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", permissions, NULL));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[::1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://::1:50050/", permissions, NULL));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", permissions, NULL));
    EXPECT_FALSE(allows("http://127.0.0.1:1001/", permissions, NULL));
    EXPECT_FALSE(allows("http://[::1]:1001/", permissions, NULL));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:1001/", permissions, NULL));
    EXPECT_FALSE(allows("http://::1:1001/", permissions, NULL));
    EXPECT_FALSE(allows("http://0:0:0:0:0:0:0:1:1001/", permissions, NULL));
    EXPECT_FALSE(allows("https://localhost", permissions, NULL));
    EXPECT_FALSE(allows("https://localhost/", permissions, NULL));
    EXPECT_FALSE(allows("https://localhost:10004", permissions, NULL));
    EXPECT_FALSE(allows("https://localhost:50050", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://org.rdk.soundPlayer_1", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.application", permissions, NULL));
    EXPECT_TRUE (allows("serviceManager://com.comcast.stateObserver", permissions, NULL));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", permissions, NULL));
    EXPECT_FALSE(allows("feature://", permissions, NULL));
    EXPECT_TRUE (allows("feature://screenshot", permissions, NULL));
    EXPECT_FALSE(allows("feature://unknown", permissions, NULL));

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

    // RealWorldExample -> Child
    // Block everything
    // + Allow http://localhost:50050
    // + Allow feature screenshot
    EXPECT_TRUE (allows(NULL, childPermissions, permissions));
    EXPECT_TRUE (allows("", childPermissions, permissions));
    EXPECT_TRUE (allows("foo", childPermissions, permissions));
    EXPECT_FALSE(allows("foo://", childPermissions, permissions));
    EXPECT_FALSE(allows("foo://example.com", childPermissions, permissions));
    EXPECT_FALSE(allows("foo://github.com", childPermissions, permissions));
    EXPECT_FALSE(allows("foo://localhost", childPermissions, permissions));
    EXPECT_TRUE (allows("http://", childPermissions, permissions));
    EXPECT_TRUE (allows("https://", childPermissions, permissions));
    EXPECT_TRUE (allows("http://example.com", childPermissions, permissions));
    EXPECT_TRUE (allows("https://example.com", childPermissions, permissions));
    EXPECT_FALSE(allows("https://github.com", childPermissions, permissions));
    EXPECT_FALSE(allows("https://github.com:80", childPermissions, permissions));
    EXPECT_FALSE(allows("https://github.com:80/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://localhost:10004", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", childPermissions, permissions));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[::1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://::1:50050/", childPermissions, permissions));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://127.0.0.1:1001/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://[::1]:1001/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:1001/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://::1:1001/", childPermissions, permissions));
    EXPECT_FALSE(allows("http://0:0:0:0:0:0:0:1:1001/", childPermissions, permissions));
    EXPECT_FALSE(allows("https://localhost", childPermissions, permissions));
    EXPECT_FALSE(allows("https://localhost/", childPermissions, permissions));
    EXPECT_FALSE(allows("https://localhost:10004", childPermissions, permissions));
    EXPECT_FALSE(allows("https://localhost:50050", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://org.rdk.soundPlayer_1", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.application", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.stateObserver", childPermissions, permissions));
    EXPECT_FALSE(allows("serviceManager://com.comcast.FrameRate", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://", childPermissions, permissions));
    EXPECT_TRUE (allows("feature://screenshot", childPermissions, permissions));
    EXPECT_FALSE(allows("feature://unknown", childPermissions, permissions));
  }

private:
  bool allows(const char* url, const rtObjectRef& permissions, const rtObjectRef& parentPermissions)
  {
    rtPermissions* p = new rtPermissions();
    rtPermissions* parent = new rtPermissions();
    if (permissions)
    {
      EXPECT_TRUE (RT_OK == p->set(permissions));
    }
    if (parentPermissions)
    {
      EXPECT_TRUE (RT_OK == parent->set(parentPermissions));
      EXPECT_TRUE (RT_OK == p->setParent(parent));
    }

    bool allows;
    EXPECT_TRUE (RT_OK == p->allows(url, allows));
    delete p;
    return allows;
  }
};

TEST_F(rtPermissions1Test, rtPermissionsTests)
{
  testFullAccess();
  testUntrusted();
  testLimitedTrust();
  testRealWorldExample();
}
