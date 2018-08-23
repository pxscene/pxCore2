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

#define private public
#define protected public

#include "pxText.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

class pxTextTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
      
    }

    void getFontFromJarTest()
    {
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxText* text = new pxText(scene);
      pxFont* font = (pxFont*)text->mFont.getPtr();
      EXPECT_TRUE(font->isFontLoaded() == true);
    }
};

TEST_F(pxTextTest, pxTextTests)
{
  getFontFromJarTest();
}
