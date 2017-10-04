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

  void testOriginMalformed()
  {
    EXPECT_TRUE (rtUrlGetOrigin(NULL).isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("h").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("1234567890").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("://").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("#").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("?").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("/").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("ftp://").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("httphttps").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("https::").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("htttp://").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http:blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http:/blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("http//blahblah").isEmpty());
    EXPECT_TRUE (rtUrlGetOrigin("htp://blahblah").isEmpty());
  }

  void testOriginNormal()
  {
    EXPECT_TRUE (rtUrlGetOrigin("http://").compare("http://") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("https://").compare("https://") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("https:///").compare("https://") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah").compare("http://blahblah") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah:8888").compare("http://blahblah:8888") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah/index.js?some=some1&parm=value").compare("http://blahblah") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah.com:100/index.js?some=some1&parm=value").compare("http://blahblah.com:100") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah.com:100?some=some1&parm=value").compare("http://blahblah.com:100") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah.com:100#parm=value").compare("http://blahblah.com:100") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://blahblah.com:100?x=http://blahblah.com").compare("http://blahblah.com:100") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("HTTPS://BLAHBLAH/INDEX.JS?SOME=SOME1&PARM=VALUE").compare("HTTPS://BLAHBLAH") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://[2001:db8:1f70::999:de8:7648:6e8]:100/").compare("http://[2001:db8:1f70::999:de8:7648:6e8]:100") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://my_email%40gmail.com:password@www.my_site.com").compare("http://my_email%40gmail.com:password@www.my_site.com") == 0);
    EXPECT_TRUE (rtUrlGetOrigin("http://敷リオワニ内前ヲルホ/index.js").compare("http://敷リオワニ内前ヲルホ") == 0);
  }
};

TEST_F(rtUrlUtilsTest, rtUrlUtilsTests)
{
  testOriginMalformed();
  testOriginNormal();
}
