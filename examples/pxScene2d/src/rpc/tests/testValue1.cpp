#include <gtest/gtest.h>
#include "../../rtValue.h"
#include "../../rtObject.h"

class rtValueTest : public ::testing::Test
{
protected:
virtual void SetUp()
{

}

virtual void TearDown()
{

}
};

TEST(rtValueTest,DefaultValuesTest)
{
  rtValue v1;
  rtValue v2;
  
  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
 // EXPECT_EQ(RT_voidType,val1.getType());
  EXPECT_EQ(v1,v2);

  EXPECT_FALSE(v1.toBool());
  EXPECT_EQ(0,v1.toInt8());
  EXPECT_EQ(0,v1.toInt32());
  EXPECT_EQ(0,v1.toUInt32());
  EXPECT_EQ(0,v1.toInt64());
  EXPECT_EQ(0,v1.toUInt64());
  EXPECT_NEAR(0.0,v1.toFloat(),2);
  EXPECT_EQ(0.0,v1.toDouble());
  EXPECT_STREQ("",v1.toString());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_EQ(v,v1.toVoidPtr());
}

TEST(rtValueTest,NullrtValueTest)
{
 rtValue v1 = 0;
 rtValue v2 = 0;
 // Test default values
  //EXPECT_EQ(RT_voidType,v1.getType());
  EXPECT_EQ(v1,v2);

  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
  EXPECT_FALSE(v1.toBool());
  EXPECT_EQ(0,v1.toInt8());
  EXPECT_EQ(0,v1.toInt32());
  EXPECT_EQ(0,v1.toUInt32());
  EXPECT_EQ(0,v1.toInt64());
  EXPECT_EQ(0,v1.toUInt64());
  EXPECT_NEAR(0.0,v1.toFloat(),2);
  EXPECT_EQ(0.0,v1.toDouble());
  EXPECT_STREQ("0",v1.toString());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_EQ(v,v1.toVoidPtr());
}

TEST(rtValueTest,DoublertValueTest)
{
  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
// Test double value
  rtValue v1 = 3.14;
  rtValue v2 = 3.14;
  rtValue v3 = 3.15;
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_doubleType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(3,v1.toInt8());
  EXPECT_EQ(3,v1.toInt32());
  EXPECT_EQ(3,v1.toUInt32());
  EXPECT_EQ(3,v1.toInt64());
  EXPECT_EQ(3,v1.toUInt64());
  EXPECT_NEAR(3.14,v1.toFloat(),2);
  EXPECT_EQ(3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("3.14",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test negative double value
  v1 = -3.14;
  v2 = -3.14;
  v3 = -3.15;
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_doubleType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(-3,v1.toInt8());
  EXPECT_EQ(-3,v1.toInt32());
  EXPECT_EQ((uint32_t)-3,v1.toUInt32());
  EXPECT_EQ(-3,v1.toInt64());
  EXPECT_EQ((uint64_t)-3,v1.toUInt64());
  EXPECT_NEAR((float)-3.14,v1.toFloat(),2);
  EXPECT_EQ(-3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("-3.14",v1.toString());
  EXPECT_EQ(RT_OK, v1.toVoidPtr());

  v1 = "3.14";
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(3,v1.toInt8());
  EXPECT_EQ(3,v1.toInt32());
  EXPECT_EQ(3,v1.toUInt32());
  EXPECT_EQ(3,v1.toInt64());
  EXPECT_EQ(3,v1.toUInt64());
  EXPECT_NEAR(3.14,v1.toFloat(),2);
  EXPECT_EQ(3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("3.14",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  v1.setString("3.14");
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(3,v1.toInt8());
  EXPECT_EQ(3,v1.toInt32());
  EXPECT_EQ(3,v1.toUInt32());
  EXPECT_EQ(3,v1.toInt64());
  EXPECT_EQ(3,v1.toUInt64());
  printf("\nHERE: %.9f\n", v1.toFloat());
  EXPECT_NEAR(3.14,v1.toFloat(),2);  
  EXPECT_EQ(3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("3.14",v1.toString()); 
  EXPECT_EQ(RT_OK,v1.toVoidPtr());
}

TEST(rtValueTest,IntrtValueTest)
{
// Test Int8
  rtValue v1;
  v1.setInt8(64);
  rtValue v2;
  v2.setInt8(64);
  rtValue v3;
  v3.setInt8(65);
  
  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_int8_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test UInt8
  v1.setUInt8(64);
  v2.setUInt8(64);
  v3.setUInt8(65);
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_uint8_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test Int32
  v1.setInt32(64);
  v2.setInt32(64);
  v3.setInt32(65);
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_int32_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test UInt32
  v1.setUInt32(64);
  v2.setUInt32(64);
  v3.setUInt32(65);
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_uint32tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test Int64
  v1.setInt64(64);
  v2.setInt64(64);
  v3.setInt64(65);
  //EXPECT_NE(v1, vEmpty);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
  //EXPECT_EQ(RT_int64_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test UInt64
  v1.setUInt64(64);
  v2.setUInt64(64);
  v3.setUInt64(65);
  //EXPECT_NE(v1, vEmpty);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
  //EXPECT_EQ(RT_uint64_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  // Test int
  v1 = 64;
  v2 = 64;
  v3 = 65;
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1, v3);
  //EXPECT_EQ(RT_int32_tType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(64,v1.toInt8());
  EXPECT_EQ(64,v1.toInt32());
  EXPECT_EQ(64,v1.toUInt32());
  EXPECT_EQ(64,v1.toInt64());
  EXPECT_EQ(64,v1.toUInt64());
  EXPECT_NEAR(64.0,v1.toFloat(),2);
  EXPECT_EQ(64.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("64",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());
}

TEST(rtValueTest,StringrtValueTest)
{
  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
  rtValue v1 = "3.14";
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(3,v1.toInt8());
  EXPECT_EQ(3,v1.toInt32());
  EXPECT_EQ(3,v1.toUInt32());
  EXPECT_EQ(3,v1.toInt64());
  EXPECT_EQ(3,v1.toUInt64());
  EXPECT_NEAR(3.14,v1.toFloat(),2);  
  EXPECT_EQ(3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("3.14",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  v1.setString("3.14");
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(3,v1.toInt8());
  EXPECT_EQ(3,v1.toInt32());
  EXPECT_EQ(3,v1.toUInt32());
  EXPECT_EQ(3,v1.toInt64());
  EXPECT_EQ(3,v1.toUInt64());
  EXPECT_NEAR(3.14,v1.toFloat(),2); 
  EXPECT_EQ(3.14,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("3.14",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  v1 = "Hello";
  rtValue v2 = "Hello";
  rtValue v3 = "hello";
  //EXPECT_NE(v1, vEmpty);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(0,v1.toInt8());
  EXPECT_EQ(0,v1.toInt32());
  EXPECT_EQ(0,v1.toUInt32());
  EXPECT_EQ(0,v1.toInt64());
  EXPECT_EQ(0,v1.toUInt64());
  EXPECT_NEAR(0,v1.toFloat(),2);
  EXPECT_EQ(0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("Hello",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

  v1.setString("Hello");
  v2.setString("Hello");
  v3.setString("hello");
  //EXPECT_NE(v1, vEmpty);
  EXPECT_EQ(v1, v2);
  EXPECT_NE(v1, v3);
  //EXPECT_EQ(RT_stringType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(0,v1.toInt8());
  EXPECT_EQ(0,v1.toInt32());
  EXPECT_EQ(0,v1.toUInt32());
  EXPECT_EQ(0,v1.toInt64());
  EXPECT_EQ(0,v1.toUInt64());
  EXPECT_NEAR(0,v1.toFloat(),2);
  EXPECT_EQ(0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("Hello",v1.toString());
  EXPECT_EQ(RT_OK,v1.toVoidPtr());

}

TEST(rtValueTest,BoolrtValueTest)
{
  rtObjectRef obj;
  rtFunctionRef fun;
  voidPtr v;
  // Test default values
// Test boolean true
  rtValue v1 = true;
  rtValue v2 = true;
  rtValue v3 = false;
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_boolType,v1.getType());
  EXPECT_TRUE(v1.toBool());
  EXPECT_EQ(1,v1.toInt8());
  EXPECT_EQ(1,v1.toInt32());
  EXPECT_EQ(1,v1.toUInt32());
  EXPECT_EQ(1,v1.toInt64());
  EXPECT_EQ(1,v1.toUInt64());
  EXPECT_NEAR(1.0,v1.toFloat(),2);
  EXPECT_EQ(1.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_NE("true",v1.toString());  
  EXPECT_EQ(v,v1.toVoidPtr());

  // Test boolean false
  v1 = false;
  v2 = false;
  v3 = true;
  //EXPECT_NE(v1,vEmpty);
  EXPECT_EQ(v1,v2);
  EXPECT_NE(v1,v3);
  //EXPECT_EQ(RT_boolType,v1.getType());
  EXPECT_FALSE(v1.toBool());
  EXPECT_EQ(0,v1.toInt8());
  EXPECT_EQ(0,v1.toInt32());
  EXPECT_EQ(0,v1.toUInt32());
  EXPECT_EQ(0,v1.toInt64());
  EXPECT_EQ(0,v1.toUInt64());
  EXPECT_EQ(0.0,v1.toFloat());
  EXPECT_EQ(0.0,v1.toDouble());
  EXPECT_EQ(obj,v1.toObject());
  EXPECT_EQ(fun,v1.toFunction());
  EXPECT_STREQ("false",v1.toString());
  EXPECT_EQ(v,v1.toVoidPtr());

}

int main(int argc,char **argv) 
{
::testing::InitGoogleTest(&argc,argv);
return RUN_ALL_TESTS();
}

