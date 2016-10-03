#include "gtest/gtest.h"

int exampleTest(int value)
{
  return value * 2;
}
 
TEST(pxScene2dTests, exampleTest) { 
    EXPECT_EQ (2, exampleTest(1));
    EXPECT_EQ (4, exampleTest(2));
    EXPECT_EQ (6, exampleTest(3));
}

