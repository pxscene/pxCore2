#include <sstream>

#include "rtPermissions.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions2Test : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test()
  {
    EXPECT_TRUE (RT_OK == rtPermissions::clearBootstrapConfig());
    EXPECT_TRUE (RT_OK == rtPermissions::loadBootstrapConfig("supportfiles/pxscenepermissions.sample.conf"));

    // Bootstrap -> FullTrust
    // Allow everything
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:10004", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("com.comcast.application", rtPermissions::SERVICE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("com.comcast.stateObserver", rtPermissions::SERVICE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("com.comcast.FrameRate", rtPermissions::SERVICE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("", rtPermissions::FEATURE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND, "https://xfinity.comcast.com"));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND, "https://xfinity.comcast.com"));

    // Bootstrap -> LimitedTrust
    // Allow everything
    // + Block all http://localhost... URL-s except http://localhost:50050
    // + Block 2 serviceManager-s
    // + Block all features except screenshot
    // + Block all applications except browser
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("", rtPermissions::SERVICE, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("com.comcast.FrameRate", rtPermissions::SERVICE, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("", rtPermissions::FEATURE, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("unknown", rtPermissions::FEATURE, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("", rtPermissions::WAYLAND, "https://foo.partner2.com"));
    EXPECT_TRUE (allows("browser", rtPermissions::WAYLAND, "https://foo.partner2.com"));
    EXPECT_FALSE(allows("unknown", rtPermissions::WAYLAND, "https://foo.partner2.com"));

    // Bootstrap -> Untrusted
    // Allow everything
    // + Block all http://localhost... URL-s
    // + Block all serviceManager-s
    // + Block feature screenshot
    // + Block application browser
    EXPECT_TRUE (allows(NULL, rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("foo://", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("foo://example.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("foo://github.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("foo://localhost", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://example.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://example.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://github.com:80", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://github.com:80/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:10004", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getDeviceId", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getAuthToken", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getSessionToken", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("http://localhost:50050/authService/getServiceAccessToken", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:50050/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://[::1]:50050/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:50050/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://::1:50050/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:50050/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://127.0.0.1:1001/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://[::1]:1001/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:1001/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://::1:1001/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("http://0:0:0:0:0:0:0:1:1001/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://localhost", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://localhost/", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://localhost:10004", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_TRUE (allows("https://localhost:50050", rtPermissions::DEFAULT, "http://example.com"));
    EXPECT_FALSE(allows("", rtPermissions::SERVICE, "http://example.com"));
    EXPECT_FALSE(allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE, "http://example.com"));
    EXPECT_FALSE(allows("com.comcast.application", rtPermissions::SERVICE, "http://example.com"));
    EXPECT_FALSE(allows("com.comcast.stateObserver", rtPermissions::SERVICE, "http://example.com"));
    EXPECT_FALSE(allows("com.comcast.FrameRate", rtPermissions::SERVICE, "http://example.com"));
    EXPECT_TRUE (allows("", rtPermissions::FEATURE, "http://example.com"));
    EXPECT_FALSE(allows("screenshot", rtPermissions::FEATURE, "http://example.com"));
    EXPECT_TRUE (allows("unknown", rtPermissions::FEATURE, "http://example.com"));
    EXPECT_TRUE (allows("", rtPermissions::WAYLAND, "http://example.com"));
    EXPECT_FALSE(allows("browser", rtPermissions::WAYLAND, "http://example.com"));
    EXPECT_TRUE (allows("unknown", rtPermissions::WAYLAND, "http://example.com"));

    EXPECT_TRUE (RT_OK == rtPermissions::clearBootstrapConfig());
  }

private:
  bool allows(const char* url, rtPermissions::Type type, const char* origin)
  {
    EXPECT_TRUE (RT_OK == mPermissions.setOrigin(origin));
    bool allows;
    EXPECT_TRUE (RT_OK == mPermissions.allows(url, type, allows));
    return allows;
  }

  rtPermissions mPermissions;
};

TEST_F(rtPermissions2Test, rtPermissionsTests)
{
  test();
}
