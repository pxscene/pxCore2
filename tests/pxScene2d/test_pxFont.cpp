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

#include <list>
#include <sstream>

#define private public
#define protected public

#include "pxWindow.h"
#include "pxScene2d.h"
#include "pxFont.h"
#include <string.h>
#include <sstream>

#include "test_includes.h" // Needs to be included last

//pxFontManager fontManager;

uint32_t font1Id, font2Id, font3Id;

TEST(pxFontTest, testAddFont)
{

  rtRef<pxFont> font1 = pxFontManager::getFont("http://pxscene.org/examples/px-reference/fonts/DejaVuSans.ttf");
  font1Id = ((pxFont*)font1.getPtr())->getFontId();

  rtRef<pxFont> font2 = pxFontManager::getFont("http://pxscene.org/examples/px-reference/fonts/IndieFlower.ttf");
  font2Id = ((pxFont*)font2.getPtr())->getFontId();

  EXPECT_NE(font1Id, font2Id);

  rtRef<pxFont> font3 = pxFontManager::getFont("http://pxscene.org/examples/px-reference/fonts/DejaVuSans.ttf");
  font3Id = ((pxFont*)font3.getPtr())->getFontId();
  EXPECT_EQ(font1Id, font3Id);
  EXPECT_NE(font3Id, font2Id);

}

TEST(pxFontTest, testRemoveFont)
{
  pxFontManager::removeFont(font1Id);

  rtRef<pxFont> font4 = pxFontManager::getFont("http://pxscene.org/examples/px-reference/fonts/DejaVuSans.ttf");
  EXPECT_EQ(font1Id, ((pxFont*)font4.getPtr())->getFontId());
  

}

TEST(pxFontTest, loadResourceFromArchiveSuccessTest)
{
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxFont* font = new pxFont("", 0, "");
      font->setUrl("XFINITYSansTTCond-Medium.ttf");
      font->loadResourceFromArchive(scene->getArchive());
      rtObjectRef loadStatus = new rtMapObject;
      font->loadStatus(loadStatus);
      rtValue status;
      loadStatus.Get("statusCode",&status);
      EXPECT_TRUE(status == PX_RESOURCE_STATUS_OK);
      delete scene;
}

TEST(pxFontTest, loadResourceFromArchiveFailureTest)
{
      pxScene2d* scene = new pxScene2d();
      rtObjectRef archive;
      EXPECT_TRUE(RT_OK == scene->loadArchive("supportfiles/test_arc_resources.jar", archive));
      pxFont* font = new pxFont("", 0, "");
      font->setUrl("XFINITYSansTTCond-Medium1.ttf");
      font->loadResourceFromArchive(scene->getArchive());
      rtObjectRef loadStatus = new rtMapObject;
      font->loadStatus(loadStatus);
      rtValue status;
      loadStatus.Get("statusCode",&status);
      EXPECT_TRUE(status == PX_RESOURCE_STATUS_FILE_NOT_FOUND);
      delete scene;
}

