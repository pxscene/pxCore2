#include <sstream>

#include "rtPermissions.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions4Test : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void test1()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("qwerty");
      url_allow->pushBack("https://comcast.com");
      url_allow->pushBack("https://comcast.com            ");
      url_allow->pushBack("            https://comcast.com");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      permissions = p;
    }

    EXPECT_EQ (-1,test(permissions, "", ""));
    EXPECT_EQ (0, test(permissions, "asdfgh", "*"));
    EXPECT_EQ (0, test(permissions, "qwerty", "qwerty"));
    EXPECT_EQ (0, test(permissions, "*", "*"));
    EXPECT_EQ (0, test(permissions, "***", "*"));
    EXPECT_EQ (0, test(permissions, "https://comcast.com", "https://comcast.com"));
    EXPECT_EQ (0, test(permissions, "https://comcast.com            ", "https://comcast.com            "));
    EXPECT_EQ (0, test(permissions, "            https://comcast.com", "            https://comcast.com"));
  }

  void test2()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("https://");
      url_allow->pushBack("https://*");
      url_allow->pushBack("*://*");
      url_allow->pushBack("https://comcast.com");
      url_allow->pushBack("https://comcast.com*");
      url_allow->pushBack("https://*comcast.com");
      url_allow->pushBack("*://comcast.com");
      url_allow->pushBack("https://*.comcast.com/*");
      url_allow->pushBack("https://*.comcast.com/*/index");
      url_allow->pushBack("https://*.comcast.com/*/index?*");
      url_allow->pushBack("https://comcast.com/index/*");
      url_allow->pushBack("https://*.comcast.com/*/index?p1=*");
      url_allow->pushBack("http://localhost:*");
      url_allow->pushBack("http://127.0.0.1:*");
      url_allow->pushBack("http://[::1]:*");
      url_allow->pushBack("http://[0:0:0:0:0:0:0:1]:*");
      url_allow->pushBack("http://::1:*");
      url_allow->pushBack("http://0:0:0:0:0:0:0:1:*");
      url_allow->pushBack("http://*:*@www.my_site.com");
      url_allow->pushBack("http://*/index*");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      permissions = p;
    }

    EXPECT_EQ (0, test(permissions, "http://", "*://*"));
    EXPECT_EQ (0, test(permissions, "https://", "https://*"));
    EXPECT_EQ (0, test(permissions, "https://comcast.com", "https://comcast.com*"));
    EXPECT_EQ (0, test(permissions, "https://m.xfinity.comcast.com/index", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(permissions, "https://m.xfinity.comcast.com/blabla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test(permissions, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*/index?*"));
    EXPECT_EQ (0, test(permissions, "https://m.comcast.com/", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(permissions, "https://comcast.com/index/blahblah", "https://comcast.com/index/*"));
    EXPECT_EQ (0, test(permissions, "https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test(permissions, "http://localhost:50050/authService/getDeviceId", "http://localhost:*"));
    EXPECT_EQ (0, test(permissions, "http://127.0.0.1:50050/authService/getDeviceId", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test(permissions, "http://[::1]:50050/authService/getDeviceId", "http://[::1]:*"));
    EXPECT_EQ (0, test(permissions, "http://[0:0:0:0:0:0:0:1]:50050/authService/getDeviceId", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(permissions, "http://::1:50050/authService/getDeviceId", "http://::1:*"));
    EXPECT_EQ (0, test(permissions, "http://0:0:0:0:0:0:0:1:50050/authService/getDeviceId", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(permissions, "http://my_email%40gmail.com:password@www.my_site.com", "http://*:*@www.my_site.com"));
    EXPECT_EQ (0, test(permissions, "http://敷リオワニ内前ヲルホ/index.js", "http://*/index*"));
  }

  void test3()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("https://*");
      url_allow->pushBack("https://*.comcast.com");
      url_allow->pushBack("https://comcast.com/*");
      url_allow->pushBack("HTTPS://COMCAST.COM");
      url_allow->pushBack("https://*.comcast.com/*");
      url_allow->pushBack("https://*.comcast.com/*/index?p1=*");
      url_allow->pushBack("https://*.comcast.com/*/index");
      url_allow->pushBack("*://github.com");
      url_allow->pushBack("*://github.com/*");
      url_allow->pushBack("*://github.com:*");
      url_allow->pushBack("*://github.com:*/*");
      url_allow->pushBack("http://localhost:*");
      url_allow->pushBack("http://localhost:*/*");
      url_allow->pushBack("http://127.0.0.1");
      url_allow->pushBack("http://127.0.0.1:*");
      url_allow->pushBack("http://127.0.0.1:*/*");
      url_allow->pushBack("http://[::1]");
      url_allow->pushBack("http://[::1]:*");
      url_allow->pushBack("http://[::1]:*/*");
      url_allow->pushBack("http://[0:0:0:0:0:0:0:1]");
      url_allow->pushBack("http://[0:0:0:0:0:0:0:1]:*");
      url_allow->pushBack("http://[0:0:0:0:0:0:0:1]:*/*");
      url_allow->pushBack("http://::1");
      url_allow->pushBack("http://::1:*");
      url_allow->pushBack("http://::1:*/*");
      url_allow->pushBack("http://0:0:0:0:0:0:0:1");
      url_allow->pushBack("http://0:0:0:0:0:0:0:1:*");
      url_allow->pushBack("http://0:0:0:0:0:0:0:1:*/*");
      url_allow->pushBack("https://localhost");
      url_allow->pushBack("https://localhost:*");
      url_allow->pushBack("https://localhost:*/*");
      url_allow->pushBack("https://127.0.0.1");
      url_allow->pushBack("https://127.0.0.1:*");
      url_allow->pushBack("https://127.0.0.1:*/*");
      url_allow->pushBack("https://[::1]");
      url_allow->pushBack("https://[::1]:*");
      url_allow->pushBack("https://[::1]:*/*");
      url_allow->pushBack("https://[0:0:0:0:0:0:0:1]");
      url_allow->pushBack("https://[0:0:0:0:0:0:0:1]:*");
      url_allow->pushBack("https://[0:0:0:0:0:0:0:1]:*/*");
      url_allow->pushBack("https://::1");
      url_allow->pushBack("https://::1:*");
      url_allow->pushBack("https://::1:*/*");
      url_allow->pushBack("https://0:0:0:0:0:0:0:1");
      url_allow->pushBack("https://0:0:0:0:0:0:0:1:*");
      url_allow->pushBack("https://0:0:0:0:0:0:0:1:*/*");
      url_allow->pushBack("serviceManager://*");
      url_allow->pushBack("feature://*");
      url_allow->pushBack("serviceManager://com.comcast.application");
      url_allow->pushBack("serviceManager://com.comcast.stateObserver");
      url_allow->pushBack("serviceManager://com.comcast.FrameRate");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      permissions = p;
    }

    EXPECT_EQ (-1,test(permissions, "http://", ""));
    EXPECT_EQ (0, test(permissions, "https://comcast.com", "https://*"));
    EXPECT_EQ (0, test(permissions, "https://m.comcast.com", "https://*.comcast.com"));
    EXPECT_EQ (0, test(permissions, "https://comcast.com/", "https://comcast.com/*"));
    EXPECT_EQ (0, test(permissions, "https://m.comcast.com/a/index?p2=1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(permissions, "https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test(permissions, "HTTPS://COMCAST.COM", "HTTPS://COMCAST.COM"));
    EXPECT_EQ (0, test(permissions, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(permissions, "https://m.xfinity.comcast.com/bla/bla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test(permissions, "https://github.com", "*://github.com"));
    EXPECT_EQ (0, test(permissions, "https://github.com/", "*://github.com/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com:443", "*://github.com:*"));
    EXPECT_EQ (0, test(permissions, "https://github.com:443/", "*://github.com:*/*"));
    EXPECT_EQ (-1,test(permissions, "http://localhost", ""));
    EXPECT_EQ (0, test(permissions, "http://localhost:50050", "http://localhost:*"));
    EXPECT_EQ (0, test(permissions, "http://localhost:50050/", "http://localhost:*/*"));
    EXPECT_EQ (0, test(permissions, "http://127.0.0.1", "http://127.0.0.1"));
    EXPECT_EQ (0, test(permissions, "http://127.0.0.1:50050", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test(permissions, "http://127.0.0.1:50050/", "http://127.0.0.1:*/*"));
    EXPECT_EQ (0, test(permissions, "http://[::1]", "http://[::1]"));
    EXPECT_EQ (0, test(permissions, "http://[::1]:50050", "http://[::1]:*"));
    EXPECT_EQ (0, test(permissions, "http://[::1]:50050/", "http://[::1]:*/*"));
    EXPECT_EQ (0, test(permissions, "http://[0:0:0:0:0:0:0:1]", "http://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test(permissions, "http://[0:0:0:0:0:0:0:1]:50050", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(permissions, "http://[0:0:0:0:0:0:0:1]:50050/", "http://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test(permissions, "http://::1", "http://::1"));
    EXPECT_EQ (0, test(permissions, "http://::1:50050", "http://::1:*"));
    EXPECT_EQ (0, test(permissions, "http://::1:50050/", "http://::1:*/*"));
    EXPECT_EQ (0, test(permissions, "http://0:0:0:0:0:0:0:1", "http://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test(permissions, "http://0:0:0:0:0:0:0:1:50050", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(permissions, "http://0:0:0:0:0:0:0:1:50050/", "http://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test(permissions, "https://localhost", "https://localhost"));
    EXPECT_EQ (0, test(permissions, "https://localhost:50050", "https://localhost:*"));
    EXPECT_EQ (0, test(permissions, "https://localhost:50050/", "https://localhost:*/*"));
    EXPECT_EQ (0, test(permissions, "https://127.0.0.1", "https://127.0.0.1"));
    EXPECT_EQ (0, test(permissions, "https://127.0.0.1:50050", "https://127.0.0.1:*"));
    EXPECT_EQ (0, test(permissions, "https://127.0.0.1:50050/", "https://127.0.0.1:*/*"));
    EXPECT_EQ (0, test(permissions, "https://[::1]", "https://[::1]"));
    EXPECT_EQ (0, test(permissions, "https://[::1]:50050", "https://[::1]:*"));
    EXPECT_EQ (0, test(permissions, "https://[::1]:50050/", "https://[::1]:*/*"));
    EXPECT_EQ (0, test(permissions, "https://[0:0:0:0:0:0:0:1]", "https://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test(permissions, "https://[0:0:0:0:0:0:0:1]:50050", "https://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(permissions, "https://[0:0:0:0:0:0:0:1]:50050/", "https://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test(permissions, "https://::1", "https://::1"));
    EXPECT_EQ (0, test(permissions, "https://::1:50050", "https://::1:*"));
    EXPECT_EQ (0, test(permissions, "https://::1:50050/", "https://::1:*/*"));
    EXPECT_EQ (0, test(permissions, "https://0:0:0:0:0:0:0:1", "https://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test(permissions, "https://0:0:0:0:0:0:0:1:50050", "https://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(permissions, "https://0:0:0:0:0:0:0:1:50050/", "https://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test(permissions, "serviceManager://api1", "serviceManager://*"));
    EXPECT_EQ (0, test(permissions, "feature://screenshot", "feature://*"));
    EXPECT_EQ (0, test(permissions, "serviceManager://com.comcast.application", "serviceManager://com.comcast.application"));
    EXPECT_EQ (0, test(permissions, "serviceManager://com.comcast.stateObserver", "serviceManager://com.comcast.stateObserver"));
    EXPECT_EQ (0, test(permissions, "serviceManager://com.comcast.FrameRate", "serviceManager://com.comcast.FrameRate"));
  }

  void test4()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("*://*");
      url_allow->pushBack("*://github.com");
      url_allow->pushBack("*://github.com/*");
      url_allow->pushBack("*://github.com:*");
      url_allow->pushBack("*://github.com:*/*");
      url_allow->pushBack("https://github.com/*");
      url_allow->pushBack("https://github.com/pxscene/*");
      url_allow->pushBack("*://github.com/pxscene/*");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      permissions = p;
    }

    EXPECT_EQ (0, test(permissions, "https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "https://github.com/pxscene/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com:443/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com:*/*"));
    EXPECT_EQ (0, test(permissions, "http://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com/pxscene/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com/features", "https://github.com/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com:443/features", "*://github.com:*/*"));
    EXPECT_EQ (0, test(permissions, "http://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com/pxscene", "https://github.com/*"));
    EXPECT_EQ (0, test(permissions, "https://github.com:443/pxscene", "*://github.com:*/*"));
    EXPECT_EQ (0, test(permissions, "http://github.com/pxscene", "*://github.com/*"));
    EXPECT_EQ (0, test(permissions, "blablahttps://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test(permissions, "blablah", "*"));
    EXPECT_EQ (0, test(permissions, "blablah:/", "*"));
    EXPECT_EQ (0, test(permissions, "blablah://", "*://*"));
    EXPECT_EQ (0, test(permissions, "blablah:///", "*://*"));
    EXPECT_EQ (0, test(permissions, "blablah://abc", "*://*"));
    EXPECT_EQ (0, test(permissions, "blablah://github.com", "*://github.com"));
    EXPECT_EQ (0, test(permissions, "blablah://github.com:8080", "*://github.com:*"));
    EXPECT_EQ (0, test(permissions, "https://google.com", "*://*"));
    EXPECT_EQ (0, test(permissions, "http://google.com", "*://*"));
    EXPECT_EQ (0, test(permissions, "github.com", "*"));
    EXPECT_EQ (0, test(permissions, "github.com://github.com", "*://github.com"));
    EXPECT_EQ (0, test(permissions, "github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test(permissions, "github.com:80/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test(permissions, "ftp://github.com", "*://github.com"));
    EXPECT_EQ (0, test(permissions, "ftp://github.com:21", "*://github.com:*"));
    EXPECT_EQ (0, test(permissions, "ftp://github.com:21/", "*://github.com:*/*"));
    EXPECT_EQ (0, test(permissions, "ftp://github.com:21/pxscene/blabla", "*://github.com:*/*"));
    EXPECT_EQ (0, test(permissions, "ftp://github.com/pxscene/blabla", "*://github.com/pxscene/*"));
  }

  void test5()
  {
    rtObjectRef permissions;
    {
      rtObjectRef p = new rtMapObject();
      rtObjectRef url = new rtMapObject();
      p.set("url", url);
      rtObjectRef serviceManager = new rtMapObject();
      rtArrayObject* url_allow = new rtArrayObject();
      url_allow->pushBack("*");
      url_allow->pushBack("http://localhost:50050/authService/getDeviceId");
      url_allow->pushBack("http://localhost*");
      url_allow->pushBack("serviceManager://*");
      url_allow->pushBack("feature://*");
      rtObjectRef url_allow_ref = url_allow;
      url.set("allow", url_allow_ref);
      permissions = p;
    }

    EXPECT_EQ (0, test(permissions, "http://localhost/", "http://localhost*"));
  }

private:
  // -1 if not found, 0 if 'expectedResult' matches, 1 if 'expectedResult' doesn't match
  int test(const rtObjectRef& permissions, const char* url, const char* expectedResult)
  {
    rtPermissions* p = new rtPermissions();
    EXPECT_TRUE (RT_OK == p->set(permissions));

    std::string match;
    int ret;
    if (RT_OK != p->findMatch(url, match))
    {
      ret = -1;
    }
    else
    {
      if (0 == match.compare(expectedResult))
        return 0;
      rtLogError("differs: actual '%s' expected '%s'", match.c_str(), expectedResult);
      return 1;
    }
    delete p;
    return ret;
  }
};

TEST_F(rtPermissions4Test, rtPermissionsTests)
{
  test1();
  test2();
  test3();
  test4();
  test5();
}
