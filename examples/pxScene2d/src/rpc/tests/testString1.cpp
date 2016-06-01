#include <gtest/gtest.h>
#include "../../rtString.h"
#include <string.h>

class rtStringTest : public ::testing::Test 
{
protected:
virtual void SetUp() {
}

virtual void TearDown() {
}

};

TEST(rtStringTest,NullTest)
{
   rtString s;

   EXPECT_TRUE(s==NULL);
   EXPECT_TRUE(s == "");
   EXPECT_TRUE(s.length() == 0);
   EXPECT_TRUE(s.byteLength() == 0);
   EXPECT_STREQ("",s.cString());
}

TEST(rtStringTest,StringLengthTest)
{
   rtString s;

   const char *ch="";
   EXPECT_TRUE(s.length() == 0);
   EXPECT_TRUE(s.byteLength() == 0);
   EXPECT_STREQ("",s.cString());
   EXPECT_TRUE(s.isEmpty());
}

TEST(rtStringTest,StringComparisonTest)
{
   rtString s;
   rtString s2;

   s = "hello";
   s2 = "ABC";

   EXPECT_FALSE(s < s2);
   EXPECT_TRUE(s > s2);

   EXPECT_FALSE(s <= s2);
   EXPECT_TRUE(s >= s2);

   EXPECT_TRUE(s <= s);
   EXPECT_TRUE(s >= s);

   EXPECT_STRNE(s,s2);
   s.term();
   s2.term(); 
   EXPECT_TRUE(s.byteLength() == 0);
   EXPECT_TRUE(s2.byteLength() == 0);
   EXPECT_STREQ(s,s2);
}

TEST(rtStringTest,FindStringTest)
{
   rtString s;
   rtString s2;

   s = "hello";
   s2 = "ll";
   EXPECT_TRUE(s.find(0,s2));
   
   s2="o";
   EXPECT_TRUE(s.find(0,s2));
}

TEST(rtStringTest,AppendStringTest)
{
   rtString s = "Hello";
   rtString s2 = "Hello World";
   const char *chAppend = " World";
   s.append(chAppend);
   
   EXPECT_STREQ(s2,s);
   printf("\n string s :: %s",s.cString());
   printf("\n string s2 :: %s \n",s2.cString());
}

TEST(rtStringTest,SubStringTest)
{
   rtString s;
   rtString s2;

   s = "hello";
   s2 = "he";
   EXPECT_FALSE(s2.compare(s.substring(0,2)));
   const char *test = "hello";
   EXPECT_STREQ(test,s.cString());   
}

int main(int argc, char **argv) {
 ::testing::InitGoogleTest(&argc,argv);
return RUN_ALL_TESTS();
}

