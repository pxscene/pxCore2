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

  void testUrlGetHostname()
  {
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://localhost").compare("localhost"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://127.0.0.1").compare("127.0.0.1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[::1]").compare("::1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[0:0:0:0:0:0:0:1]").compare("0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://localhost:8080").compare("localhost"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://127.0.0.1:8080").compare("127.0.0.1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[::1]:8080").compare("::1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[0:0:0:0:0:0:0:1]:8080").compare("0:0:0:0:0:0:0:1"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[fe80::95ef:2fd0:cc0d:e94b]").compare("fe80::95ef:2fd0:cc0d:e94b"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://[fe00::0]").compare("fe00::0"));
    EXPECT_EQ (0, (int)rtUrlGetHostname("http://ip6-localnet").compare("ip6-localnet"));

    EXPECT_TRUE (rtUrlGetHostname("http://::1").isEmpty());
    EXPECT_TRUE (rtUrlGetHostname("http://").isEmpty());
    EXPECT_TRUE (rtUrlGetHostname("").isEmpty());
    EXPECT_TRUE (rtUrlGetHostname(NULL).isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("://111").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("127.0.0.1").isEmpty());
  }

  void testUrlEscape()
  {
    EXPECT_EQ (std::string(""), rtUrlEscape(NULL).cString());
    EXPECT_EQ (std::string(""), rtUrlEscape("").cString());
    EXPECT_EQ (
      std::string("file%3A%2F%2F"),
      rtUrlEscape("file://").cString());
    EXPECT_EQ (
      std::string("This%20isn%27t%20a%20URL."),
      rtUrlEscape("This isn't a URL.").cString());
    EXPECT_EQ (
      std::string("http%3A%2F%2FMVSXX.COMPANY.COM%3A04445%2FCICSPLEXSM%2F%2FJSMITH%2FVIEW%2FOURLOCTRAN%3FA_TRANID%3DP%2A%26O_TRANID%3DNE"),
      rtUrlEscape("http://MVSXX.COMPANY.COM:04445/CICSPLEXSM//JSMITH/VIEW/OURLOCTRAN?A_TRANID=P*&O_TRANID=NE").cString());
    EXPECT_EQ (
      std::string("https%3A%2F%2Fapi.fullcontact.com%2Fv2%2Fperson.json%3Femail%3Dbart%40fullcontact.com%26apiKey%3Dyour_api_key_here"),
      rtUrlEscape("https://api.fullcontact.com/v2/person.json?email=bart@fullcontact.com&apiKey=your_api_key_here").cString());
    EXPECT_EQ (
      std::string("http%3A%2F%2Fsite%2Fgwturl%23user%3A45%2Fcomments"),
      rtUrlEscape("http://site/gwturl#user:45/comments").cString());
    EXPECT_EQ (
      std::string("http%3A%2F%2Fmy_email%2540gmail.com%3Apassword%40www.my_site.com"),
      rtUrlEscape("http://my_email%40gmail.com:password@www.my_site.com").cString());
    EXPECT_EQ (
      std::string("http%3A%2F%2F%5B2001%3Adb8%3A85a3%3A%3A8a2e%3A370%3A7334%5D%2Ffoo%2Fbar"),
      rtUrlEscape("http://[2001:db8:85a3::8a2e:370:7334]/foo/bar").cString());
    EXPECT_EQ (
      std::string("ftp%3A%2F%2Fuser%3Apass%2521word%40example.com%2Fpath%2Fto%2Finput.mp3"),
      rtUrlEscape("ftp://user:pass%21word@example.com/path/to/input.mp3").cString());
    EXPECT_EQ (
      std::string("sftp%3A%2F%2Fuser%2540example.com%3Apass%2521word%40example.com%2Fpath%2Fto%2Finput.3gp"),
      rtUrlEscape("sftp://user%40example.com:pass%21word@example.com/path/to/input.3gp").cString());
    EXPECT_EQ (
      std::string("http%3A%2F%2Fexample.com%2Ffile%5B%2F%5D.html"),
      rtUrlEscape("http://example.com/file[/].html").cString());
  }
};

TEST_F(rtUrlUtilsTest, rtUrlUtilsTests)
{
  testUrlGetOriginMalformed();
  testUrlGetOriginNormal();
  testUrlGetOriginRfc3986();
  testUrlGetOriginHack();
  testUrlGetHostname();
  testUrlEscape();
}
