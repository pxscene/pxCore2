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

#include "rtSettings.h"
#include "rtPathUtils.h"
#include <pxScene2d.h>
#include "test_includes.h" // Needs to be included last

class rtSettingsTest : public testing::Test
{
public:
  virtual void SetUp()
  {
  }

  virtual void TearDown()
  {
  }

  void testWriteRead()
  {
    rtSettings s;

    // populate
    EXPECT_EQ((int)RT_OK, (int)s.setValue("string", "abc"));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("true", true));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("false", false));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("null", rtValue()));
    int8_t int8_tVal = -128;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("int8_t", int8_tVal));
    uint8_t uint8_tVal = 255;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("uint8_t", uint8_tVal));
    int32_t int32_tVal = -2147483648;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("int32_t", int32_tVal));
    uint32_t uint32_tVal = 4294967295;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("uint32_t", uint32_tVal));
    int64_t int64_tVal = -9223372036854775808ull;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("int64_t", int64_tVal));
    uint64_t uint64_tVal = 18446744073709551615ull;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("uint64_t", uint64_tVal));
    float floatVal = 12.666f;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("float", floatVal));
    double doubleVal = 12.6664287277627762;
    EXPECT_EQ((int)RT_OK, (int)s.setValue("double", doubleVal));

    // verify keys
    std::vector<rtString> k;
    k.push_back("string");
    k.push_back("true");
    k.push_back("false");
    k.push_back("null");
    k.push_back("int8_t");
    k.push_back("uint8_t");
    k.push_back("int32_t");
    k.push_back("uint32_t");
    k.push_back("int64_t");
    k.push_back("uint64_t");
    k.push_back("float");
    k.push_back("double");
    std::sort(k.begin(), k.end());
    std::vector<rtString> kVal;
    EXPECT_EQ((int)RT_OK, (int)s.keys(kVal));
    EXPECT_TRUE(k == kVal);

    // write
    rtString filePath;
    rtGetCurrentDirectory(filePath);
    filePath.append("/test_rtSettings.json");
    EXPECT_EQ((int)RT_OK, (int)s.save(filePath));

    // clear
    EXPECT_EQ((int)RT_OK, (int)s.clear());
    EXPECT_EQ((int)RT_OK, (int)s.keys(k));
    EXPECT_EQ((int)0, (int)k.size());

    // read
    EXPECT_EQ((int)RT_OK, (int)s.loadFromFile(filePath));
    rtString rmCmd("rm ");
    rmCmd.append(filePath.cString());
    bool sysRet = system(rmCmd.cString());
    EXPECT_TRUE(sysRet == 0);

    // verify values
    rtValue value;
    EXPECT_EQ((int)RT_OK, (int)s.value("string", value));
    EXPECT_EQ((int)0, strcmp("abc", value.toString().cString()));
    EXPECT_EQ((int)RT_OK, (int)s.value("true", value));
    EXPECT_TRUE(value.toBool());
    EXPECT_EQ((int)RT_OK, (int)s.value("false", value));
    EXPECT_FALSE(value.toBool());
    EXPECT_EQ((int)RT_OK, (int)s.value("null", value));
    EXPECT_TRUE(value.isEmpty());
    EXPECT_EQ((int)RT_OK, (int)s.value("int8_t", value));
    EXPECT_EQ(int8_tVal, value.toInt8());
    EXPECT_EQ((int)RT_OK, (int)s.value("uint8_t", value));
    EXPECT_EQ(uint8_tVal, value.toUInt8());
    EXPECT_EQ((int)RT_OK, (int)s.value("int32_t", value));
    EXPECT_EQ(int32_tVal, value.toInt32());
    EXPECT_EQ((int)RT_OK, (int)s.value("uint32_t", value));
    EXPECT_EQ(uint32_tVal, value.toUInt32());
    EXPECT_EQ((int)RT_OK, (int)s.value("int64_t", value));
    EXPECT_EQ(int64_tVal, value.toInt64());
    EXPECT_EQ((int)RT_OK, (int)s.value("uint64_t", value));
    EXPECT_EQ(uint64_tVal, value.toUInt64());
    EXPECT_EQ((int)RT_OK, (int)s.value("float", value));
    EXPECT_EQ(floatVal, value.toFloat());
    EXPECT_EQ((int)RT_OK, (int)s.value("double", value));
    EXPECT_EQ(doubleVal, value.toDouble());
  }

  void testFromArgs()
  {
    rtSettings s;

    // read from args
    int argc = 7;
    char argv[7][32] = {"process\0", "-keyX\0", "-key1\0", "value1\0", "-key2=true\0", "--keyZ\0", "valueZ\0"};
    char* _argv[] = {argv[0], argv[1], argv[2], argv[3], argv[4], argv[5], argv[6]};
    EXPECT_EQ((int)RT_OK, (int)s.loadFromArgs(argc, _argv));

    // verify keys
    std::vector<rtString> k;
    k.push_back("key1");
    k.push_back("key2");
    std::sort(k.begin(), k.end());
    std::vector<rtString> kVal;
    EXPECT_EQ((int)RT_OK, (int)s.keys(kVal));
    EXPECT_TRUE(k == kVal);

    // verify values
    rtValue value;
    EXPECT_EQ((int)RT_OK, (int)s.value("key1", value));
    EXPECT_EQ((int)0, strcmp("value1", value.toString().cString()));
    EXPECT_EQ((int)RT_OK, (int)s.value("key2", value));
    EXPECT_EQ((int)0, strcmp("true", value.toString().cString()));
  }

  void testNotFound()
  {
    rtSettings s;

    // populate
    EXPECT_EQ((int)RT_OK, (int)s.setValue("a", "a"));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("b", "b"));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("c", "c"));

    // verify not found & remove
    rtValue value;
    EXPECT_EQ((int)RT_ERROR, (int)s.value("y", value));
    EXPECT_EQ((int)RT_ERROR, (int)s.remove("x"));
    EXPECT_EQ((int)RT_OK, (int)s.remove("b"));

    // verify keys
    std::vector<rtString> k;
    k.push_back("a");
    k.push_back("c");
    std::sort(k.begin(), k.end());
    std::vector<rtString> kVal;
    EXPECT_EQ((int)RT_OK, (int)s.keys(kVal));
    EXPECT_TRUE(k == kVal);
  }

  void testOverwrite()
  {
    rtSettings s;

    // populate
    EXPECT_EQ((int)RT_OK, (int)s.setValue("a", "a"));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("b", "b"));
    EXPECT_EQ((int)RT_OK, (int)s.setValue("c", "c"));

    // read from args
    int argc = 5;
    char argv[5][32] = {"process\0", "-a\0", "x\0", "-d\0", "y\0"};
    char* _argv[] = {argv[0], argv[1], argv[2], argv[3], argv[4]};
    EXPECT_EQ((int)RT_OK, (int)s.loadFromArgs(argc, _argv));

    // verify keys
    std::vector<rtString> k;
    k.push_back("a");
    k.push_back("b");
    k.push_back("c");
    k.push_back("d");
    std::sort(k.begin(), k.end());
    std::vector<rtString> kVal;
    EXPECT_EQ((int)RT_OK, (int)s.keys(kVal));
    EXPECT_TRUE(k == kVal);

    // verify values
    rtValue value;
    EXPECT_EQ((int)RT_OK, (int)s.value("a", value));
    EXPECT_EQ((int)0, strcmp("x", value.toString().cString()));
    EXPECT_EQ((int)RT_OK, (int)s.value("b", value));
    EXPECT_EQ((int)0, strcmp("b", value.toString().cString()));
    EXPECT_EQ((int)RT_OK, (int)s.value("c", value));
    EXPECT_EQ((int)0, strcmp("c", value.toString().cString()));
    EXPECT_EQ((int)RT_OK, (int)s.value("d", value));
    EXPECT_EQ((int)0, strcmp("y", value.toString().cString()));
  }

  void testPermissionPresentRead()
   {
     rtSettings::instance()->setValue("disableFilePermissionCheck",true);
     pxScene2d* scene = new pxScene2d();
     rtValue val;
     EXPECT_TRUE(scene->sparkSetting("disableFilePermissionCheck",val) == RT_OK);
     EXPECT_TRUE(val.toBool() == true);
     delete scene;
     rtSettings::instance()->remove("disableFilePermissionCheck");
   }

   void testPermissionAbsentRead()
   {
     pxScene2d* scene = new pxScene2d();
     rtValue val;
     EXPECT_TRUE(scene->sparkSetting("disableFilePermissionCheck",val) == RT_OK);
     EXPECT_TRUE(true == val.isEmpty());
     delete scene;
   }
};

TEST_F(rtSettingsTest, rtSettingsTests)
{
  testWriteRead();
  testFromArgs();
  testNotFound();
  testOverwrite();
  // read permission value from scene
  testPermissionPresentRead();
  testPermissionAbsentRead();
}
