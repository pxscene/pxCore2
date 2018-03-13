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

#include "rtZip.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include "test_includes.h" // Needs to be included last

using namespace std;

class rtStringTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void isEmptyTest()
    {
      rtString  empty;
      rtString  non_empty("Hello World");

      EXPECT_TRUE(empty.isEmpty() == true);
    //  EXPECT_TRUE(non_empty.isEmpty() == false);  // TODO:  '==' operator not supported for rtString ?? 
    }

    void appendTest()
    {
      rtString  world(" World");
      rtString  hello_world("Hello World");

      // rtString  joined;    // TODO:  Seg Faults ... need empty string
      rtString  joined("");

      joined.append("Hello");
      joined.append(world);

      EXPECT_TRUE( std::string(joined.cString()) == "Hello World");
      // EXPECT_TRUE( std::string(joined == hello_world) );   // TODO:  '==' operator not supported for rtString ?? 
    }

    void compareTest()
    {
      mData = "123";

      EXPECT_TRUE( mData.compare("123") == 0 );
      EXPECT_FALSE(mData.compare("321") == 0);
    }

    void lengthTest()
    {
      mData = "123";

      EXPECT_TRUE(mData.length()     == 3); // UTF char count
      EXPECT_TRUE(mData.byteLength() == 3); // byte count
    }

    void cStringTest()
    {
      mData = "123";

      EXPECT_TRUE(mData.cString() != nullptr);

      // mData.term();    // TODO:  should it assign to NULL / nullptr

      // EXPECT_TRUE(mData.cString() == nullptr);
    }

    void operatorTests()
    {
    // finline bool operator== (const char* s) const { return compare(s) == 0; }
      mData = "123";  EXPECT_TRUE(mData == "123");
      
    // finline bool operator!= (const char* s) const { return compare(s) != 0; }
      mData = "123";  EXPECT_TRUE(mData != "321");

    // finline bool operator<  (const char* s) const { return compare(s) <  0; }
      mData = "A";  EXPECT_TRUE(mData < "B");

    // finline bool operator<= (const char* s) const { return compare(s) <= 0; }
      mData = "B";  EXPECT_TRUE(mData <= "B");

    // finline bool operator>  (const char* s) const { return compare(s) >  0; }
      mData = "C";  EXPECT_TRUE(mData > "B");

    // finline bool operator>= (const char* s) const { return compare(s) >= 0; }
      mData = "C";  EXPECT_TRUE(mData >= "C");
    }

    void beginsTest()
    {
       mData = "123";  EXPECT_TRUE(mData.beginsWith("12") );
    }
    
    
    void substringTest()
    {
//     rtString substring(size_t pos, size_t len = 0) const;

       mData = "123"; 
       
       EXPECT_TRUE(mData.substring(1,1)  == "2" );
       EXPECT_FALSE(mData.substring(1,3) == "2" );
       EXPECT_FALSE(mData.substring(3,3) == "2" );
    }

    void findTests()
    {
      // int32_t find(size_t pos, const char* s) const;
      // int32_t find(size_t pos, uint32_t codePoint) const;

       mData = "123"; 

       EXPECT_TRUE(mData.find(0,"2")   ==  1 );  // Good !
       EXPECT_TRUE(mData.find(0,"0")   == -1 );  // Bad !
       EXPECT_TRUE(mData.find(10,"2")  == -1 );  // Bad !
       EXPECT_TRUE(mData.find(0, 0x32) ==  1 );  // Good !  0x32 = "2"
       EXPECT_TRUE(mData.find(0, 0x34) == -1 );  // Bad !   0x34 = "4"
    }

    private:
      rtString mData;
};

TEST_F(rtStringTest, rtStringTests)
{
  isEmptyTest();
  appendTest();
  compareTest();
  lengthTest();
  cStringTest();

  operatorTests();
  beginsTest();
  substringTest();
  findTests();
}

