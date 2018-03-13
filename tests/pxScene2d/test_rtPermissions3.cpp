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

#include "pxScene2d.h"
#include "test_includes.h" // Needs to be included last

class rtPermissions3Test : public testing::Test
{
public:
  virtual void SetUp()
  {
    mScene = new pxScene2d(true, NULL);
  }

  virtual void TearDown()
  {
    delete mScene;
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
    rtObjectRef applications = new rtMapObject();
    p.set("applications", applications);
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
    rtArrayObject* applications_allow = new rtArrayObject();
    applications_allow->pushBack("allowed_wayland_app");
    rtObjectRef applications_allow_ref = applications_allow;
    applications.set("allow", applications_allow_ref);
    permissions = p;

    EXPECT_TRUE (RT_OK == mScene->setPermissions(permissions));

    // Block everything
    // + Allow URLs "http://allowed.com", "http://*.allowed.com:80"
    // + Allow serviceManager-s "org.rdk.allowed_1", "org.rdk.allowed_2"
    // + Allow feature "screenshot"
    // + Allow application "browser"
    EXPECT_TRUE (allowsCreate("http://allowed.site.url"));
    EXPECT_TRUE (allowsCreate("http://another.allowed.site.url:80"));
    EXPECT_FALSE(allowsCreate("http://not.allowed.block.this"));
    EXPECT_TRUE (allowsService("org.rdk.allowed_1"));
    EXPECT_TRUE (allowsService("org.rdk.allowed_2"));
    EXPECT_FALSE(allowsService("not.allowed"));
    EXPECT_TRUE (allowsScreenshot());
    EXPECT_TRUE (allowsLoadArchive("http://allowed.site.url"));
    EXPECT_TRUE (allowsLoadArchive("http://another.allowed.site.url:80"));
    EXPECT_FALSE(allowsLoadArchive("http://not.allowed.block.this"));
    EXPECT_TRUE (allowsWayland("allowed_wayland_app"));

    // Same
    // + none features are allowed
    // + none applications are allowed
    features_allow->empty();
    applications_allow->empty();
    EXPECT_TRUE (RT_OK == mScene->setPermissions(permissions));

    EXPECT_FALSE(allowsScreenshot());
    EXPECT_FALSE(allowsWayland("allowed_wayland_app"));
  }

private:
  bool allowsCreate(const char* url)
  {
    rtObjectRef p = new rtMapObject();
    p.set("t", "object");
    p.set("url", url);
    rtObjectRef o;
    rtError e = mScene->create(p, o);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsScreenshot()
  {
    rtString type("ignore this");
    rtString pngData;
    rtError e = mScene->screenshot(type, pngData);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsService(const char* name)
  {
    rtString nameStr(name);
    rtObjectRef returnObject;
    rtError e = mScene->getService(nameStr, returnObject);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsWayland(const char* cmd)
  {
    rtObjectRef p = new rtMapObject();
    p.set("cmd", cmd);
    rtObjectRef returnObject;
    rtError e = mScene->createWayland(p, returnObject);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  bool allowsLoadArchive(const char* url)
  {
    rtString urlStr(url);
    rtObjectRef archive;
    rtError e = mScene->loadArchive(urlStr, archive);
    return RT_ERROR_NOT_ALLOWED != e;
  }

  pxScene2d* mScene;
};

TEST_F(rtPermissions3Test, rtPermissionsTests)
{
  test();
}
