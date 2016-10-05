#include "gtest/gtest.h"
#include "pxContext.h"

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

