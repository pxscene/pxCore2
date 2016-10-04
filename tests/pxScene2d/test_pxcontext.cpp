#include "gtest/gtest.h"
#include "pxContext.h"

bool outlinesTest(bool value)
{
  pxContext a;
  bool showOutlines = a.showOutlines();
  return value * 2;
}
 
TEST(pxScene2dTests, pxContextOutlinesTest)
{ 
    EXPECT_TRUE (outlinesTest(true) == true);
    EXPECT_TRUE (outlinesTest(false) == false);
}

