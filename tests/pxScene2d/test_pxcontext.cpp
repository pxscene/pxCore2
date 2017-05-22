#include "pxContext.h"

#include "test_includes.h" // Needs to be included last

bool outlinesTest(bool value)
{
  pxContext a;
  a.setShowOutlines(value);
  return a.showOutlines();
}

TEST(pxScene2dTests, pxContextOutlinesTest)
{
    EXPECT_TRUE (outlinesTest(true) == true);
    EXPECT_TRUE (outlinesTest(false) == false);
}

