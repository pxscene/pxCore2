#include <sstream>

#include "rtPermissions.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions5Test : public testing::Test
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
    // This test uses default Bootstrap config (./pxscenepermissions.conf)

    // "*" : "default"
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://localhost", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://localhost:8080", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://127.0.0.1", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://127.0.0.1:8080", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://[::1]", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://[::1]:8080", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("file:///afile", rtPermissions::DEFAULT, "http://default.web.site"));
    EXPECT_FALSE(allows("anything", rtPermissions::SERVICE, "http://default.web.site"));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE, "http://default.web.site"));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND, "http://default.web.site"));

    // "*://localhost" : "local",
    // "*://localhost:*" : "local",
    // "*://127.0.0.1" : "local",
    // "*://127.0.0.1:*" : "local",
    // "*://[::1]" : "local",
    // "*://[::1]:*" : "local",
    // "*://[0:0:0:0:0:0:0:1]" : "local",
    // "*://[0:0:0:0:0:0:0:1]:*" : "local",
    // "file://*" : "local",
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://localhost:8080", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://127.0.0.1", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://127.0.0.1:8080", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://[::1]", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://[::1]:8080", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("file:///afile", rtPermissions::DEFAULT, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE, "ftp://127.0.0.1:9999"));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND, "ftp://127.0.0.1:9999"));

    // "*://*.comcast.com" : "comcast",
    // "*://*.comcast.com:*" : "comcast",
    // "*://*.comcast.net" : "comcast",
    // "*://*.comcast.net:*" : "comcast",
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://localhost:8080", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://127.0.0.1", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://127.0.0.1:8080", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://[::1]", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://[::1]:8080", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("http://[0:0:0:0:0:0:0:1]:8080", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_FALSE(allows("file:///afile", rtPermissions::DEFAULT, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE, "http://blabla.comcast.net:80"));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND, "http://blabla.comcast.net:80"));

    // "*://www.pxscene.org" : "pxscene.org",
    // "*://www.pxscene.org:*" : "pxscene.org",
    // "*://pxscene.org" : "pxscene.org",
    // "*://pxscene.org:*" : "pxscene.org",
    // NOTE all URL origins/services/apps present here are actually used by pxscene.org
    // NOTE use something similar to "grep -hrIiEo "([a-z]+)://[a-zA-Z0-9:.?=_-]*" <pxscene repo> | sort | uniq"
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://54.146.54.142", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://abc.abc.abc", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://aescripts.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://c1.staticflickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://c2.staticflickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://codepen.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://edge.myriad-xcr.xcr.comcast.net", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://farm3.static.flickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://farm4.static.flickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://farm6.static.flickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://farm9.staticflickr.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://fontawesome.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://github.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://google.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://gsgd.co.uk", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://johnrobinsn.github.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://json-schema.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://localhost", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://localhost:8090", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://aescripts.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://cdnjs.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://codepen.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://creative.adobe.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://diatrophy.github.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://fonts.googleapis.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://guides.github.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://help.github.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://helpx.adobe.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://jakearchibald.github.io", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://registry.npmjs.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://updates.aescripts.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://www.apple.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("https://www.youtube.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.apache.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.apple.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.freetype.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.pxscene.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.storminthecastle.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.w3.org", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("http://www.w3schools.com", rtPermissions::DEFAULT, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE, "http://www.pxscene.org"));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND, "http://www.pxscene.org"));

    // "*://xre2-apps.cvs-a.ula.comcast.net" : "pxscene-samples",
    // "*://xre2-apps.cvs-a.ula.comcast.net:*" : "pxscene-samples",
    // "*://px-apps.sys.comcast.net" : "pxscene-samples",
    // "*://px-apps.sys.comcast.net:*" : "pxscene-samples"
    // NOTE all URL origins/services/apps present here are actually used by pxscene-samples
    // NOTE use something similar to "grep -hrIiEo "([a-z]+)://[a-zA-Z0-9:.?=_-]*" <pxscene-samples repo> | sort | uniq"
    EXPECT_TRUE (allows("http://any.web.site", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("dvr://*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://10.0.2.2", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://10.253.86.167", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://127.0.0.1:8080", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://54.146.54.142", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://96.118.11.20", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://96.118.6.151", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://a-96-118-11-20.sys.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://access.auth.po.ccp.cable.comcast.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccpmer-po-v003-p.po.ccp.cable.comcast.com:9002", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccpmer-po-v003-p.po.ccp.cable.comcast.com:9003", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccr.cdvr-vin2.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccr.linear-fre-pil.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccr.linear-nat-pil-red.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ccr.linear-nat-pil.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://cdn.online-convert.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://design.staging.xfinity.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.ccp-img.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.ip-eas-dns.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.myriad-gn-xcr.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.myriad-xcr.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.x1-app.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://edge.xre-app-icons.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://farm3.static.flickr.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://farm4.static.flickr.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://farm5.staticflickr.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://farm6.static.flickr.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://google.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://infinity.x1.xcal.tv", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ips-prod.apps.xcal.tv", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://ips-qa.apps.xcal.tv", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://irac-fl.s-akacast.iheart.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://legacy.myriad-next.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://localhost:8080", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://localhost:8081", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://localhost:8090", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://merclient-ch2-1p-vip.cable.comcast.com:10119", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://merclient-ch2-1p-vip.cable.comcast.com:9002", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://merclient-ch2-1p-vip.cable.comcast.com:9003", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://my.xfinity.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://por-img.cimcontent.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://px-apps.sys.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://pxscene.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://quarterback-ci-restapi-teamcity.xreapps.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://api.twitter.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://ccp.sys.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://cfry002.github.io", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://conniefry.github.io", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://developer.github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://developer.mozilla.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://diatrophy.github.io", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://edge.myriad-xcr.xcr.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://server.test-cors.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://fonts.googleapis.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://github.comcast.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://guides.github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://help.github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://identity.auth.po.ccp.cable.comcast.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://images.njck.co", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://pages.github.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://PATH_TO_THIS_FILE.js", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://px-apps.sys.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://s3.dualstack.us-west-1.amazonaws.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://scontent.xx.fbcdn.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://unsplash.it", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://userstream.twitter.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://vod-api-ci.corevui.net:443", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://websockets.g1.app.cloud.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://www.teamccp.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://www.w3.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://x1.ozlo.ai", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("https://youtu.be", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://tvxcts-c5-c00001-b.ch.tvx.comcast.com:10004", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://udbbrs.g.comcast.net:9096", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://vod-api-ci.corevui.net:8080", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://wp.storminthecastle.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.apache.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.nbcnews.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.nbcolympics.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.pxscene.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.sample-videos.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.schaik.com", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://www.w3.org", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("http://xre2-apps.cvs-a.ula.comcast.net", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("native:///*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("ocap://*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("scene://*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("wikiimg://Amy_Adams_2016.jpg", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("ws://*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("ws://listening-devices-websocket.corevui.net:8083", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("wss://px-wss.sys.comcast.net:8443", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("xre:///*", rtPermissions::DEFAULT, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("anything", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.soundPlayer_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("systemService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.DisplaySettings", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("FrontPanelService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("homeNetworkingService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("screenCaptureService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("MSOPairingService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.stateObserver", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.StoragemanagerService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.TsbSettings", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("Warehouse", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("WebSocketService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("BrowserSettings", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("AVInput", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("MemoryInfo", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.hdmiCec_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.hdmiInput_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.wifiManager", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.dataCapture_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.traceroute_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.bluetoothSettings_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.copilot_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.ControlService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.RemoteActionMappingService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.application_3", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.ping_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.HDCPProfile", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.VREXmanagerService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.videoApplicationEvents_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.XreFrameRateService", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("rdk.linearSegmentedAdvertising_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.hdcpCompliance_1s", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.XREReceiverDiagnostics", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("com.comcast.webcamera", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.intrusionDetection_1d", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.openrdk.ETVIntegratedSignaling_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.IPDirectUnicast_1", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("org.rdk.LoggingPreferences", rtPermissions::SERVICE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("anything", rtPermissions::FEATURE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("screenshot", rtPermissions::FEATURE, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("anything", rtPermissions::WAYLAND, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("netflix", rtPermissions::WAYLAND, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("rmfplayer", rtPermissions::WAYLAND, "https://px-apps.sys.comcast.net"));
    EXPECT_TRUE (allows("rdkmediaplayer", rtPermissions::WAYLAND, "https://px-apps.sys.comcast.net"));
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

TEST_F(rtPermissions5Test, rtPermissionsTests)
{
  test();
}
