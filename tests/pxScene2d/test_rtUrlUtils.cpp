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

#include "rtUrlUtils.h"
#include "test_includes.h" // Needs to be included last

class rtUrlUtilsTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void testUrlGetOriginMalformed()
  {
    EXPECT_TRUE (rtUrlGetOrigin(NULL).isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("h").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("1234567890").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("://").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("#").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("?").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("/").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("httphttps").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("https::").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http:blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http:/blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http//blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("mailto:fred@example.com").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("news:comp.infosystems.www.servers.unix").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("tel:+1-816-555-1212").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("urn:oasis:names:specification:docbook:dtd:xml:4.1.2").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("file:///etc/hosts").isEmpty());
  }

  void testUrlGetOriginNormal()
  {
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://").compare("ftp://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("htttp://").compare("htttp://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("htp://blahblah").compare("htp://blahblah"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://").compare("http://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("https://").compare("https://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("https:///").compare("https://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah").compare("http://blahblah"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah:8888").compare("http://blahblah:8888"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah/index.js?some=some1&parm=value").compare("http://blahblah"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah.com:100/index.js?some=some1&parm=value").compare("http://blahblah.com:100"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah.com:100?some=some1&parm=value").compare("http://blahblah.com:100"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah.com:100#parm=value").compare("http://blahblah.com:100"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://blahblah.com:100?x=http://blahblah.com").compare("http://blahblah.com:100"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("HTTPS://BLAHBLAH/INDEX.JS?SOME=SOME1&PARM=VALUE").compare("HTTPS://BLAHBLAH"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://[2001:db8:1f70::999:de8:7648:6e8]:100/").compare("http://[2001:db8:1f70::999:de8:7648:6e8]:100"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://my_email%40gmail.com:password@www.my_site.com").compare("http://www.my_site.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://敷リオワニ内前ヲルホ/index.js").compare("http://敷リオワニ内前ヲルホ"));
  }

  void testUrlGetOriginRfc3986()
  {
    EXPECT_EQ (0, (int)rtUrlGetOrigin("foo://example.com:8042/over/there?name=ferret#nose").compare("foo://example.com:8042"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("foo://info.example.com?fred").compare("foo://info.example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://a/b/c/d;p?q").compare("http://a"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("example://a/b/c/%7Bfoo%7D").compare("example://a"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("eXAMPLE://a/./b/../b/%63/%7bfoo%7d").compare("eXAMPLE://a"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("HTTP://www.EXAMPLE.com/").compare("HTTP://www.EXAMPLE.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://example.com").compare("http://example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://example.com/").compare("http://example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://example.com:/").compare("http://example.com:"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://example.com:80/").compare("http://example.com:80"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://example.com/data/").compare("http://example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://cnn.example.com&story=breaking_news@10.0.0.1/top_story.htm").compare("ftp://10.0.0.1"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ics.uci.edu/pub/ietf/uri/#Related").compare("http://www.ics.uci.edu"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.w3.org/Addressing/").compare("http://www.w3.org"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://foo.example.com/rfc/").compare("ftp://foo.example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ics.uci.edu/pub/ietf/uri/historical.html#WARNING").compare("http://www.ics.uci.edu"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://ftp.is.co.za/rfc/rfc1808.txt").compare("ftp://ftp.is.co.za"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ietf.org/rfc/rfc2396.txt").compare("http://www.ietf.org"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ldap://[2001:db8::7]/c=GB?objectClass?one").compare("ldap://[2001:db8::7]"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("telnet://192.0.2.16:80/").compare("telnet://192.0.2.16:80"));
  }

  void testUrlGetOriginHack()
  {
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://foo.com/testing/://123.xreapps.net/securityHole.js").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://foo.com?x=http://123.xreapps.net").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://123.xreapps.net@foo.com").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://123.xreapps.net:80@foo.com").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://123.xreapps.net%40123.xreapps.net:123.xreapps.net@foo.com").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://@foo.com").compare("http://foo.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://:@[::1]:1/:@[::1]:1/:@[::1]:1").compare("http://[::1]:1"));
  }
};

TEST_F(rtUrlUtilsTest, rtUrlUtilsTests)
{
  testUrlGetOriginMalformed();
  testUrlGetOriginNormal();
  testUrlGetOriginRfc3986();
  testUrlGetOriginHack();
}
