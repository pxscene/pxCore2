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

  void test2()
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

  void test3()
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

  void test4()
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

  void test5()
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
  // -1 if not found, 0 if 'expectedResult' matches, 1 if 'expectedResult' doesn't match
  int test(const char* url, const char* expectedResult)
  {
    rtPermissions::wildcard_t w;
    w.first = url;
    w.second = rtPermissions::DEFAULT;
    rtPermissions::permissionsMap_t::const_iterator it = rtPermissions::findWildcard(mPermissionsMap, w);
    if (it == mPermissionsMap.end())
      return -1;
    if (0 == it->first.first.compare(expectedResult))
      return 0;
    rtLogError("differs: actual '%s' expected '%s'", it->first.first.c_str(), expectedResult);
    return 1;
  }

  rtPermissions::permissionsMap_t mPermissionsMap;
};

TEST_F(rtPermissions4Test, rtPermissionsTests)
{
  test1();
  test2();
  test3();
  test4();
  test5();
}
