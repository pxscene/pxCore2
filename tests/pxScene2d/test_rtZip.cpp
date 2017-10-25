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

const char  testArray[256] = {};
const void* testBuffer = (void*) &testArray[0];

class rtZipTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void initFromBufferTest()
    {
      rtData  buffer;
      rtError ret = rtLoadFile("supportfiles/sample.zip", buffer);
      
      ret = mData.initFromBuffer(buffer.data(), buffer.length());
      EXPECT_TRUE(ret == RT_OK);
    }

    void initFromFileTest()
    {
      rtError ret = mData.initFromFile("supportfiles/sample.zip");

      EXPECT_TRUE(ret == RT_OK);
    }

    void termTest()
    {
      rtData  buffer;
      rtError ret;
      
      ret = rtLoadFile("supportfiles/sample.zip", buffer);
      EXPECT_TRUE(ret == RT_OK);
      
      ret = mData.initFromBuffer(buffer.data(), buffer.length());
      EXPECT_TRUE(ret == RT_OK);

      ret = mData.term();
      EXPECT_TRUE(ret == RT_OK);
    }

    void fileCountTest()
    {
      rtData  buffer;
      rtError ret;

      ret = rtLoadFile("supportfiles/sample.zip", buffer);
      EXPECT_TRUE(ret == RT_OK);

      ret = mData.initFromBuffer(buffer.data(), buffer.length());
      EXPECT_TRUE(ret == RT_OK);

      uint32_t count = mData.fileCount();

      EXPECT_FALSE(count == 0);
      EXPECT_TRUE( count == 1);
    }

    void getFilePathAtIndexTest()
    {
      rtError ret = mData.initFromFile("supportfiles/sample.zip");
      EXPECT_TRUE(ret == RT_OK);

      rtString path;

      ret = mData.getFilePathAtIndex(-1, path);
      EXPECT_TRUE(ret == RT_FAIL);

      ret = mData.getFilePathAtIndex(0, path);
      EXPECT_TRUE(ret == RT_OK);

      EXPECT_TRUE ( path == "test.html");  // << 'test.html' is file within 'sample.zip'
    }

    void getFileDataTest()
    {
      rtError ret = mData.initFromFile("supportfiles/sample.zip");
      rtData data;

      ret = mData.getFileData("does_not_exist.html", data);
      EXPECT_TRUE(ret == RT_FAIL);

      ret = mData.getFileData("test.html", data);
      EXPECT_TRUE(ret == RT_OK);

      EXPECT_TRUE(data.length() == 36);
    }

    void isZipTest()
    {
      rtData  buffer;
      rtError ret = rtLoadFile("supportfiles/test.html", buffer);

      EXPECT_FALSE( rtZip::isZip(buffer.data(), buffer.length()) );

      ret = rtLoadFile("supportfiles/sample.zip", buffer);

      EXPECT_TRUE( rtZip::isZip(buffer.data(), buffer.length()) );
    }

    private:
      rtZip mData;
};

TEST_F(rtZipTest, rtZipTests)
{
  initFromBufferTest();
  initFromFileTest();
  termTest();

  fileCountTest();

  getFilePathAtIndexTest();
  getFileDataTest();

  isZipTest();
}

