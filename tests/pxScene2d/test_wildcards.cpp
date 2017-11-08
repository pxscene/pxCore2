#include <sstream>

#include "pxScene2d.h"
#include "test_includes.h" // Needs to be included last

class wildcardTest : public testing::Test
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
    permissionsMap_t map;
    map["*"] = true;
    map["qwerty"] = true;
    map["https://comcast.com"] = true;
    map["https://comcast.com            "] = true;
    map["            https://comcast.com"] = true;

    EXPECT_EQ (-1,test(map, "", ""));
    EXPECT_EQ (0, test(map, "asdfgh", "*"));
    EXPECT_EQ (0, test(map, "qwerty", "qwerty"));
    EXPECT_EQ (0, test(map, "*", "*"));
    EXPECT_EQ (0, test(map, "***", "*"));
    EXPECT_EQ (0, test(map, "https://comcast.com", "https://comcast.com"));
    EXPECT_EQ (0, test(map, "https://comcast.com            ", "https://comcast.com            "));
    EXPECT_EQ (0, test(map, "            https://comcast.com", "            https://comcast.com"));
  }

  void test2()
  {
    permissionsMap_t map;
    map["https://"] = true;
    map["https://*"] = true;
    map["*://*"] = true;
    map["https://comcast.com"] = true;
    map["https://comcast.com*"] = true;
    map["https://*comcast.com"] = true;
    map["*://comcast.com"] = true;
    map["https://*.comcast.com/*"] = true;
    map["https://*.comcast.com/*/index"] = true;
    map["https://*.comcast.com/*/index?*"] = true;
    map["https://comcast.com/index/*"] = true;
    map["https://*.comcast.com/*/index?p1=*"] = true;
    map["http://localhost:*"] = true;
    map["http://127.0.0.1:*"] = true;
    map["http://[::1]:*"] = true;
    map["http://[0:0:0:0:0:0:0:1]:*"] = true;
    map["http://::1:*"] = true;
    map["http://0:0:0:0:0:0:0:1:*"] = true;
    map["http://*:*@www.my_site.com"] = true;
    map["http://*/index*"] = true;

    EXPECT_EQ (0, test(map, "http://", "*://*"));
    EXPECT_EQ (0, test(map, "https://", "https://*"));
    EXPECT_EQ (0, test(map, "https://comcast.com", "https://comcast.com*"));
    EXPECT_EQ (0, test(map, "https://m.xfinity.comcast.com/index", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(map, "https://m.xfinity.comcast.com/blabla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test(map, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*/index?*"));
    EXPECT_EQ (0, test(map, "https://m.comcast.com/", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(map, "https://comcast.com/index/blahblah", "https://comcast.com/index/*"));
    EXPECT_EQ (0, test(map, "https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test(map, "http://localhost:50050/authService/getDeviceId", "http://localhost:*"));
    EXPECT_EQ (0, test(map, "http://127.0.0.1:50050/authService/getDeviceId", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test(map, "http://[::1]:50050/authService/getDeviceId", "http://[::1]:*"));
    EXPECT_EQ (0, test(map, "http://[0:0:0:0:0:0:0:1]:50050/authService/getDeviceId", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(map, "http://::1:50050/authService/getDeviceId", "http://::1:*"));
    EXPECT_EQ (0, test(map, "http://0:0:0:0:0:0:0:1:50050/authService/getDeviceId", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(map, "http://my_email%40gmail.com:password@www.my_site.com", "http://*:*@www.my_site.com"));
    EXPECT_EQ (0, test(map, "http://敷リオワニ内前ヲルホ/index.js", "http://*/index*"));
  }

  void test3()
  {
    permissionsMap_t map;
    map["https://*"] = true;
    map["https://*.comcast.com"] = true;
    map["https://comcast.com/*"] = true;
    map["HTTPS://COMCAST.COM"] = true;
    map["https://*.comcast.com/*"] = true;
    map["https://*.comcast.com/*/index?p1=*"] = true;
    map["https://*.comcast.com/*/index"] = true;
    map["*://github.com"] = true;
    map["*://github.com/*"] = true;
    map["*://github.com:*"] = true;
    map["*://github.com:*/*"] = true;
    map["http://localhost:*"] = true;
    map["http://localhost:*/*"] = true;
    map["http://127.0.0.1"] = true;
    map["http://127.0.0.1:*"] = true;
    map["http://127.0.0.1:*/*"] = true;
    map["http://[::1]"] = true;
    map["http://[::1]:*"] = true;
    map["http://[::1]:*/*"] = true;
    map["http://[0:0:0:0:0:0:0:1]"] = true;
    map["http://[0:0:0:0:0:0:0:1]:*"] = true;
    map["http://[0:0:0:0:0:0:0:1]:*/*"] = true;
    map["http://::1"] = true;
    map["http://::1:*"] = true;
    map["http://::1:*/*"] = true;
    map["http://0:0:0:0:0:0:0:1"] = true;
    map["http://0:0:0:0:0:0:0:1:*"] = true;
    map["http://0:0:0:0:0:0:0:1:*/*"] = true;
    map["https://localhost"] = true;
    map["https://localhost:*"] = true;
    map["https://localhost:*/*"] = true;
    map["https://127.0.0.1"] = true;
    map["https://127.0.0.1:*"] = true;
    map["https://127.0.0.1:*/*"] = true;
    map["https://[::1]"] = true;
    map["https://[::1]:*"] = true;
    map["https://[::1]:*/*"] = true;
    map["https://[0:0:0:0:0:0:0:1]"] = true;
    map["https://[0:0:0:0:0:0:0:1]:*"] = true;
    map["https://[0:0:0:0:0:0:0:1]:*/*"] = true;
    map["https://::1"] = true;
    map["https://::1:*"] = true;
    map["https://::1:*/*"] = true;
    map["https://0:0:0:0:0:0:0:1"] = true;
    map["https://0:0:0:0:0:0:0:1:*"] = true;
    map["https://0:0:0:0:0:0:0:1:*/*"] = true;
    map["serviceManager://*"] = true;
    map["feature://*"] = true;
    map["serviceManager://com.comcast.application"] = true;
    map["serviceManager://com.comcast.stateObserver"] = true;
    map["serviceManager://com.comcast.FrameRate"] = true;

    EXPECT_EQ (-1,test(map, "http://", ""));
    EXPECT_EQ (0, test(map, "https://comcast.com", "https://*"));
    EXPECT_EQ (0, test(map, "https://m.comcast.com", "https://*.comcast.com"));
    EXPECT_EQ (0, test(map, "https://comcast.com/", "https://comcast.com/*"));
    EXPECT_EQ (0, test(map, "https://m.comcast.com/a/index?p2=1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(map, "https://m.comcast.com/a/index?p1=1", "https://*.comcast.com/*/index?p1=*"));
    EXPECT_EQ (0, test(map, "HTTPS://COMCAST.COM", "HTTPS://COMCAST.COM"));
    EXPECT_EQ (0, test(map, "https://m.xfinity.comcast.com/blabla/index?some=some1&parm=value#p1", "https://*.comcast.com/*"));
    EXPECT_EQ (0, test(map, "https://m.xfinity.comcast.com/bla/bla/index", "https://*.comcast.com/*/index"));
    EXPECT_EQ (0, test(map, "https://github.com", "*://github.com"));
    EXPECT_EQ (0, test(map, "https://github.com/", "*://github.com/*"));
    EXPECT_EQ (0, test(map, "https://github.com:443", "*://github.com:*"));
    EXPECT_EQ (0, test(map, "https://github.com:443/", "*://github.com:*/*"));
    EXPECT_EQ (-1,test(map, "http://localhost", ""));
    EXPECT_EQ (0, test(map, "http://localhost:50050", "http://localhost:*"));
    EXPECT_EQ (0, test(map, "http://localhost:50050/", "http://localhost:*/*"));
    EXPECT_EQ (0, test(map, "http://127.0.0.1", "http://127.0.0.1"));
    EXPECT_EQ (0, test(map, "http://127.0.0.1:50050", "http://127.0.0.1:*"));
    EXPECT_EQ (0, test(map, "http://127.0.0.1:50050/", "http://127.0.0.1:*/*"));
    EXPECT_EQ (0, test(map, "http://[::1]", "http://[::1]"));
    EXPECT_EQ (0, test(map, "http://[::1]:50050", "http://[::1]:*"));
    EXPECT_EQ (0, test(map, "http://[::1]:50050/", "http://[::1]:*/*"));
    EXPECT_EQ (0, test(map, "http://[0:0:0:0:0:0:0:1]", "http://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test(map, "http://[0:0:0:0:0:0:0:1]:50050", "http://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(map, "http://[0:0:0:0:0:0:0:1]:50050/", "http://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test(map, "http://::1", "http://::1"));
    EXPECT_EQ (0, test(map, "http://::1:50050", "http://::1:*"));
    EXPECT_EQ (0, test(map, "http://::1:50050/", "http://::1:*/*"));
    EXPECT_EQ (0, test(map, "http://0:0:0:0:0:0:0:1", "http://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test(map, "http://0:0:0:0:0:0:0:1:50050", "http://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(map, "http://0:0:0:0:0:0:0:1:50050/", "http://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test(map, "https://localhost", "https://localhost"));
    EXPECT_EQ (0, test(map, "https://localhost:50050", "https://localhost:*"));
    EXPECT_EQ (0, test(map, "https://localhost:50050/", "https://localhost:*/*"));
    EXPECT_EQ (0, test(map, "https://127.0.0.1", "https://127.0.0.1"));
    EXPECT_EQ (0, test(map, "https://127.0.0.1:50050", "https://127.0.0.1:*"));
    EXPECT_EQ (0, test(map, "https://127.0.0.1:50050/", "https://127.0.0.1:*/*"));
    EXPECT_EQ (0, test(map, "https://[::1]", "https://[::1]"));
    EXPECT_EQ (0, test(map, "https://[::1]:50050", "https://[::1]:*"));
    EXPECT_EQ (0, test(map, "https://[::1]:50050/", "https://[::1]:*/*"));
    EXPECT_EQ (0, test(map, "https://[0:0:0:0:0:0:0:1]", "https://[0:0:0:0:0:0:0:1]"));
    EXPECT_EQ (0, test(map, "https://[0:0:0:0:0:0:0:1]:50050", "https://[0:0:0:0:0:0:0:1]:*"));
    EXPECT_EQ (0, test(map, "https://[0:0:0:0:0:0:0:1]:50050/", "https://[0:0:0:0:0:0:0:1]:*/*"));
    EXPECT_EQ (0, test(map, "https://::1", "https://::1"));
    EXPECT_EQ (0, test(map, "https://::1:50050", "https://::1:*"));
    EXPECT_EQ (0, test(map, "https://::1:50050/", "https://::1:*/*"));
    EXPECT_EQ (0, test(map, "https://0:0:0:0:0:0:0:1", "https://0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, test(map, "https://0:0:0:0:0:0:0:1:50050", "https://0:0:0:0:0:0:0:1:*"));
    EXPECT_EQ (0, test(map, "https://0:0:0:0:0:0:0:1:50050/", "https://0:0:0:0:0:0:0:1:*/*"));
    EXPECT_EQ (0, test(map, "serviceManager://api1", "serviceManager://*"));
    EXPECT_EQ (0, test(map, "feature://screenshot", "feature://*"));
    EXPECT_EQ (0, test(map, "serviceManager://com.comcast.application", "serviceManager://com.comcast.application"));
    EXPECT_EQ (0, test(map, "serviceManager://com.comcast.stateObserver", "serviceManager://com.comcast.stateObserver"));
    EXPECT_EQ (0, test(map, "serviceManager://com.comcast.FrameRate", "serviceManager://com.comcast.FrameRate"));
  }

  void test4()
  {
    permissionsMap_t map;
    map["*"] = true;
    map["*://*"] = true;
    map["*://github.com"] = true;
    map["*://github.com/*"] = true;
    map["*://github.com:*"] = true;
    map["*://github.com:*/*"] = true;
    map["https://github.com/*"] = true;
    map["https://github.com/pxscene/*"] = true;
    map["*://github.com/pxscene/*"] = true;

    EXPECT_EQ (0, test(map, "https://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "https://github.com/pxscene/*"));
    EXPECT_EQ (0, test(map, "https://github.com:443/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com:*/*"));
    EXPECT_EQ (0, test(map, "http://github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*://github.com/pxscene/*"));
    EXPECT_EQ (0, test(map, "https://github.com/features", "https://github.com/*"));
    EXPECT_EQ (0, test(map, "https://github.com:443/features", "*://github.com:*/*"));
    EXPECT_EQ (0, test(map, "http://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test(map, "https://github.com/pxscene", "https://github.com/*"));
    EXPECT_EQ (0, test(map, "https://github.com:443/pxscene", "*://github.com:*/*"));
    EXPECT_EQ (0, test(map, "http://github.com/pxscene", "*://github.com/*"));
    EXPECT_EQ (0, test(map, "blablahttps://github.com/features", "*://github.com/*"));
    EXPECT_EQ (0, test(map, "blablah", "*"));
    EXPECT_EQ (0, test(map, "blablah:/", "*"));
    EXPECT_EQ (0, test(map, "blablah://", "*://*"));
    EXPECT_EQ (0, test(map, "blablah:///", "*://*"));
    EXPECT_EQ (0, test(map, "blablah://abc", "*://*"));
    EXPECT_EQ (0, test(map, "blablah://github.com", "*://github.com"));
    EXPECT_EQ (0, test(map, "blablah://github.com:8080", "*://github.com:*"));
    EXPECT_EQ (0, test(map, "https://google.com", "*://*"));
    EXPECT_EQ (0, test(map, "http://google.com", "*://*"));
    EXPECT_EQ (0, test(map, "github.com", "*"));
    EXPECT_EQ (0, test(map, "github.com://github.com", "*://github.com"));
    EXPECT_EQ (0, test(map, "github.com/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test(map, "github.com:80/pxscene/pxCore/blob/master/examples/pxScene2d/README.md", "*"));
    EXPECT_EQ (0, test(map, "ftp://github.com", "*://github.com"));
    EXPECT_EQ (0, test(map, "ftp://github.com:21", "*://github.com:*"));
    EXPECT_EQ (0, test(map, "ftp://github.com:21/", "*://github.com:*/*"));
    EXPECT_EQ (0, test(map, "ftp://github.com:21/pxscene/blabla", "*://github.com:*/*"));
    EXPECT_EQ (0, test(map, "ftp://github.com/pxscene/blabla", "*://github.com/pxscene/*"));
  }

  void test5()
  {
    permissionsMap_t map;
    map["*"] = true;
    map["http://localhost:50050/authService/getDeviceId"] = true;
    map["http://localhost*"] = true;
    map["serviceManager://*"] = true;
    map["feature://*"] = true;

    EXPECT_EQ (0, test(map, "http://localhost/", "http://localhost*"));
  }

private:
  // -1 if not found, 0 if 'expectedResult' matches, 1 if 'expectedResult' doesn't match
  int test(const permissionsMap_t& map, const char* input, const char* expectedResult)
  {
    permissionsMap_t::const_iterator it = find_best_wildcard_match(map, input);
    if (it != map.end())
    {
      if (0 == it->first.compare(expectedResult))
        return 0;
      rtLogError("differs: actual '%s' expected '%s'", it->first.c_str(), expectedResult);
      return 1;
    }
    return -1;
  }
};

TEST_F(wildcardTest, wildcardTests)
{
  test1();
  test2();
  test3();
  test4();
  test5();
}
