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

#include "rtFile.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>
#include <dlfcn.h>

#include "test_includes.h" // Needs to be included last

using namespace std;
bool failAlloc = false;
//typedef void* (*new_t)(size_t);
/*
void* operator new (size_t size)
{
  static new_t newp = (new_t) dlsym(RTLD_NEXT, "new");
  if (!failAlloc)
    return newp(size);
  return NULL;
}
*/
class rtDataTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void initLengthOnlySuccessTest()
    {
      rtError ret = mData.init(5);
      EXPECT_TRUE(mData.length() == 5);
      EXPECT_TRUE(ret == RT_OK);
    }

    void initLengthOnlyFailedTest()
    {
      rtError ret = mData.init(2147483698);
      EXPECT_TRUE(ret == RT_FAIL);
      EXPECT_TRUE(mData.length() == 0);
    }

    void initDataLengthSuccessTest()
    {
      char data[] = "test";
      rtError ret = mData.init((uint8_t*) &data, 4);
      EXPECT_TRUE(mData.length() == 4);
      EXPECT_TRUE( strcmp( (char *)mData.data(), "test") == 0);
      EXPECT_TRUE(ret == RT_OK);
    }

    void initDataLengthFailureTest()
    {
      char data[] = "test";
      rtError ret = mData.init((uint8_t*) &data, 500000000);
      EXPECT_TRUE(mData.length() == 0);
      EXPECT_TRUE( (char *)mData.data() == NULL);
      EXPECT_TRUE(ret == RT_FAIL);
    }

    void termTest()
    {
      char data[] = "test";
      rtError ret = mData.init((uint8_t*) &data, 4);

      mData.term();

      EXPECT_TRUE(mData.length() == 0);
      EXPECT_TRUE( (char *)mData.data() == NULL);
      EXPECT_TRUE(ret == RT_OK);
    }

    void storeDataSuccessValidLengthTest()
    {
      char data[] = "test";
      rtError ret = mData.init((uint8_t*) &data, 4);

      EXPECT_TRUE (rtStoreFile("supportfiles/storedata.txt",mData) == RT_OK);
      EXPECT_TRUE(ret == RT_OK);
    }

    void storeDataSuccessZeroLengthTest()
    {
      rtError ret = mData.init(0);
      EXPECT_TRUE (rtStoreFile("supportfiles/storedata.txt",mData) == RT_OK);
      EXPECT_TRUE(ret == RT_OK);
    }

    void storeDataFailedTest()
    {
      char data[] = "test";
      rtError ret = mData.init((uint8_t*) &data, 4);
      EXPECT_TRUE (rtStoreFile("supportfiles1/storedata.txt",mData) == RT_FAIL);
      EXPECT_TRUE(ret == RT_OK);
    }

    void loadDataSuccessTest()
    {
      int sysret = system("echo \"Hello\" > supportfiles/storedata.txt");
      EXPECT_TRUE (rtLoadFile("supportfiles/storedata.txt",mData) == RT_OK);
      printf("Queried data [%s] \n",mData.data());
      fflush(stdout);
      EXPECT_TRUE (mData.length() == 6);
      EXPECT_TRUE (strcmp("Hello\n", (char*) mData.data()) == 0);
      sysret = system("rm -rf supportfiles/storedata.txt");
      EXPECT_TRUE(sysret == RT_OK);
    }

    void loadDataFailureTest()
    {
      mData.init(0);
      EXPECT_TRUE (rtLoadFile("supportfiles1/storedata.txt",mData) == RT_FAIL);
      EXPECT_TRUE (mData.length() == 0);
    }
    private:
      rtData mData;
};

TEST_F(rtDataTest, rtDataTests)
{
  initLengthOnlySuccessTest();
//  initLengthOnlyFailedTest();
  initDataLengthSuccessTest();
//  initDataLengthFailureTest();
  termTest();
  storeDataSuccessValidLengthTest();
  storeDataSuccessZeroLengthTest();
  storeDataFailedTest();
  loadDataSuccessTest();
  loadDataFailureTest();
}

class rtFilePointerTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void initTest()
    {
      FILE* fptr = fopen("supportfiles/file.txt","w");
      rtFilePointer fp(fptr);
      EXPECT_TRUE (fp.getPtr() != NULL);
    }
};

TEST_F(rtFilePointerTest, rtFilePointerTests)
{
  initTest();
}
