#include <sstream>

#include "pxScene2d.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions3Test : public testing::Test
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
    rtObjectRef permissions;
    rtObjectRef p = new rtMapObject();
    rtObjectRef url = new rtMapObject();
    p.set("url", url);
    rtObjectRef serviceManager = new rtMapObject();
    p.set("serviceManager", serviceManager);
    rtObjectRef features = new rtMapObject();
    p.set("features", features);
    rtArrayObject* url_block = new rtArrayObject();
    url_block->pushBack("*");
    rtObjectRef url_block_ref = url_block;
    url.set("block", url_block_ref);
    rtArrayObject* url_allow = new rtArrayObject();
    url_allow->pushBack("http://allowed.site.url");
    url_allow->pushBack("http://*.allowed.site.url:80");
    rtObjectRef url_allow_ref = url_allow;
    url.set("allow", url_allow_ref);
    rtArrayObject* serviceManager_block = new rtArrayObject();
    serviceManager_block->pushBack("*");
    rtObjectRef serviceManager_block_ref = serviceManager_block;
    serviceManager.set("block", serviceManager_block_ref);
    rtArrayObject* serviceManager_allow = new rtArrayObject();
    serviceManager_allow->pushBack("org.rdk.allowed_1");
    serviceManager_allow->pushBack("org.rdk.allowed_2");
    rtObjectRef serviceManager_allow_ref = serviceManager_allow;
    serviceManager.set("allow", serviceManager_allow_ref);
    rtArrayObject* features_block = new rtArrayObject();
    features_block->pushBack("*");
    rtObjectRef features_block_ref = features_block;
    features.set("block", features_block_ref);
    rtArrayObject* features_allow = new rtArrayObject();
    features_allow->pushBack("screenshot");
    rtObjectRef features_allow_ref = features_allow;
    features.set("allow", features_allow_ref);
    permissions = p;

    // Block everything
    // + Allow URLs "http://allowed.com", "http://*.allowed.com:80"
    // + Allow serviceManager-s "org.rdk.allowed_1", "org.rdk.allowed_2"
    // + Allow feature "screenshot"
    EXPECT_TRUE (allowsCreate("http://allowed.site.url", permissions));
    EXPECT_TRUE (allowsCreate("http://another.allowed.site.url:80", permissions));
    EXPECT_FALSE(allowsCreate("http://not.allowed.block.this", permissions));
    EXPECT_TRUE (allowsServiceManager("org.rdk.allowed_1", permissions));
    EXPECT_TRUE (allowsServiceManager("org.rdk.allowed_2", permissions));
    EXPECT_FALSE(allowsServiceManager("not.allowed", permissions));
    EXPECT_TRUE (allowsScreenshot(permissions));
    EXPECT_TRUE (allowsLoadArchive("http://allowed.site.url", permissions));
    EXPECT_TRUE (allowsLoadArchive("http://another.allowed.site.url:80", permissions));
    EXPECT_FALSE(allowsLoadArchive("http://not.allowed.block.this", permissions));

    // Same
    // EXCEPT none features are allowed
    features_allow->empty();
    EXPECT_FALSE(allowsScreenshot(permissions));
  }

private:
  bool allowsCreate(const char* url, const rtObjectRef& permissions)
  {
    pxScene2d* scene = new pxScene2d(true, NULL);
    EXPECT_TRUE (RT_OK == scene->setPermissions(permissions));

    rtObjectRef p = new rtMapObject();
    p.set("t", "object");
    p.set("url", url);
    rtObjectRef o;
    rtError e = scene->create(p, o);

    delete scene;
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsScreenshot(const rtObjectRef& permissions)
  {
    pxScene2d* scene = new pxScene2d(true, NULL);
    EXPECT_TRUE (RT_OK == scene->setPermissions(permissions));

    rtString type("ignore this");
    rtString pngData;
    rtError e = scene->screenshot(type, pngData);

    delete scene;
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsServiceManager(const char* name, const rtObjectRef& permissions)
  {
    pxScene2d* scene = new pxScene2d(true, NULL);
    EXPECT_TRUE (RT_OK == scene->setPermissions(permissions));

    rtString nameStr(name);
    rtObjectRef returnObject;
    rtError e = scene->getService(nameStr, returnObject);

    delete scene;
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsLoadArchive(const char* url, const rtObjectRef& permissions)
  {
    pxScene2d* scene = new pxScene2d(true, NULL);
    EXPECT_TRUE (RT_OK == scene->setPermissions(permissions));

    rtString urlStr(url);
    rtObjectRef archive;
    rtError e = scene->loadArchive(urlStr, archive);

    delete scene;
    return RT_ERROR_NOT_ALLOWED != e;
  }
};

TEST_F(rtPermissions3Test, rtPermissionsTests)
{
  test();
}
