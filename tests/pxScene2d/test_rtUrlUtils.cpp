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
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://my_email%40gmail.com:password@www.my_site.com").compare("http://my_email%40gmail.com:password@www.my_site.com"));
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
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://cnn.example.com&story=breaking_news@10.0.0.1/top_story.htm").compare("ftp://cnn.example.com&story=breaking_news@10.0.0.1"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ics.uci.edu/pub/ietf/uri/#Related").compare("http://www.ics.uci.edu"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.w3.org/Addressing/").compare("http://www.w3.org"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://foo.example.com/rfc/").compare("ftp://foo.example.com"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ics.uci.edu/pub/ietf/uri/historical.html#WARNING").compare("http://www.ics.uci.edu"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("file:///etc/hosts").compare("file://"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ftp://ftp.is.co.za/rfc/rfc1808.txt").compare("ftp://ftp.is.co.za"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("http://www.ietf.org/rfc/rfc2396.txt").compare("http://www.ietf.org"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("ldap://[2001:db8::7]/c=GB?objectClass?one").compare("ldap://[2001:db8::7]"));
    EXPECT_EQ (0, (int)rtUrlGetOrigin("telnet://192.0.2.16:80/").compare("telnet://192.0.2.16:80"));
  }
};

TEST_F(rtUrlUtilsTest, rtUrlUtilsTests)
{
  testUrlGetOriginMalformed();
  testUrlGetOriginNormal();
  testUrlGetOriginRfc3986();
}
