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
