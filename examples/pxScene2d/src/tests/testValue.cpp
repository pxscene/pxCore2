#include "testValue.h"

#include "rtValue.h"

#include "rtTest.h"
#include "rtObject.h"

void testValue()
{
  printf("testValue\n");
  rtValue vEmpty;
  rtValue v1;
  rtValue v2;
  rtValue v3;

  // Test default values
  RT_TEST(v1.getType() == RT_voidType);
  RT_TEST(v1 == v2);

  RT_TEST(v1.toBool() == false);
  RT_TEST(v1.toInt8() == 0);
  RT_TEST(v1.toInt32() == 0);
  RT_TEST(v1.toUInt32() == 0);
  RT_TEST(v1.toInt64() == 0);
  RT_TEST(v1.toUInt64() == 0);
  RT_TEST(v1.toFloat() == 0.0);
  RT_TEST(v1.toDouble() == 0.0);
  RT_TEST(v1.toString() == "");
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test boolean true
  v1 = true;
  v2 = true;
  v3 = false;
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_boolType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 1);
  RT_TEST(v1.toInt32() == 1);
  RT_TEST(v1.toUInt32() == 1);
  RT_TEST(v1.toInt64() == 1);
  RT_TEST(v1.toUInt64() == 1);
  RT_TEST(v1.toFloat() == 1.0);
  RT_TEST(v1.toDouble() == 1.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "true");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test boolean false
  v1 = false;
  v2 = false;
  v3 = true;
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_boolType);
  RT_TEST(v1.toBool() == false);
  RT_TEST(v1.toInt8() == 0);
  RT_TEST(v1.toInt32() == 0);
  RT_TEST(v1.toUInt32() == 0);
  RT_TEST(v1.toInt64() == 0);
  RT_TEST(v1.toUInt64() == 0);
  RT_TEST(v1.toFloat() == 0.0);
  RT_TEST(v1.toDouble() == 0.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "false");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test Int8
  v1.setInt8(64);
  v2.setInt8(64);
  v3.setInt8(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_int8_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test UInt8
  v1.setUInt8(64);
  v2.setUInt8(64);
  v3.setUInt8(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_uint8_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test Int32
  v1.setInt32(64);
  v2.setInt32(64);
  v3.setInt32(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_int32_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test UInt32
  v1.setUInt32(64);
  v2.setUInt32(64);
  v3.setUInt32(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_uint32_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test Int64
  v1.setInt64(64);
  v2.setInt64(64);
  v3.setInt64(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_int64_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test UInt64
  v1.setUInt64(64);
  v2.setUInt64(64);
  v3.setUInt64(65);
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_uint64_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test int
  v1 = 64;
  v2 = 64;
  v3 = 65;
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_int32_tType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 64);
  RT_TEST(v1.toInt32() == 64);
  RT_TEST(v1.toUInt32() == 64);
  RT_TEST(v1.toInt64() == 64);
  RT_TEST(v1.toUInt64() == 64);
  RT_TEST(v1.toFloat() == 64.0);
  RT_TEST(v1.toDouble() == 64.0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "64");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test double value
  v1 = 3.14;
  v2 = 3.14;
  v3 = 3.15;
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_doubleType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 3);
  RT_TEST(v1.toInt32() == 3);
  RT_TEST(v1.toUInt32() == 3);
  RT_TEST(v1.toInt64() == 3);
  RT_TEST(v1.toUInt64() == 3);
  RT_TEST(v1.toFloat() == (float)3.14);
  RT_TEST(v1.toDouble() == 3.14);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "3.14");
  RT_TEST(v1.toVoidPtr() == NULL);

  // Test negative double value
  v1 = -3.14;
  v2 = -3.14;
  v3 = -3.15;
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_doubleType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == -3);
  RT_TEST(v1.toInt32() == -3);
  RT_TEST(v1.toUInt32() == (uint32_t)-3);
  RT_TEST(v1.toInt64() == -3);
  RT_TEST(v1.toUInt64() == (uint64_t)-3);
  RT_TEST(v1.toFloat() == (float)-3.14);
  RT_TEST(v1.toDouble() == -3.14);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "-3.14");
  RT_TEST(v1.toVoidPtr() == NULL);

  v1 = "3.14";
  RT_TEST(v1.getType() == RT_stringType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 3);
  RT_TEST(v1.toInt32() == 3);
  RT_TEST(v1.toUInt32() == 3);
  RT_TEST(v1.toInt64() == 3);
  RT_TEST(v1.toUInt64() == 3);
  RT_TEST(v1.toFloat() == (float)3.14);
  RT_TEST(v1.toDouble() == 3.14);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "3.14");
  RT_TEST(v1.toVoidPtr() == NULL);

  v1.setString("3.14");
  RT_TEST(v1.getType() == RT_stringType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 3);
  RT_TEST(v1.toInt32() == 3);
  RT_TEST(v1.toUInt32() == 3);
  RT_TEST(v1.toInt64() == 3);
  RT_TEST(v1.toUInt64() == 3);
  RT_TEST(v1.toFloat() == (float)3.14);
  RT_TEST(v1.toDouble() == 3.14);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "3.14");
  RT_TEST(v1.toVoidPtr() == NULL);

  v1 = "Hello";
  v2 = "Hello";
  v3 = "hello";
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_stringType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 0);
  RT_TEST(v1.toInt32() == 0);
  RT_TEST(v1.toUInt32() == 0);
  RT_TEST(v1.toInt64() == 0);
  RT_TEST(v1.toUInt64() == 0);
  RT_TEST(v1.toFloat() == 0);
  RT_TEST(v1.toDouble() == 0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "Hello");
  RT_TEST(v1.toVoidPtr() == NULL);

  v1.setString("Hello");
  v2.setString("Hello");
  v3.setString("hello");
  RT_TEST(v1 != vEmpty);
  RT_TEST(v1 == v2);
  RT_TEST(v1 != v3);
  RT_TEST(v1.getType() == RT_stringType);
  RT_TEST(v1.toBool() == true);
  RT_TEST(v1.toInt8() == 0);
  RT_TEST(v1.toInt32() == 0);
  RT_TEST(v1.toUInt32() == 0);
  RT_TEST(v1.toInt64() == 0);
  RT_TEST(v1.toUInt64() == 0);
  RT_TEST(v1.toFloat() == 0);
  RT_TEST(v1.toDouble() == 0);
  RT_TEST(v1.toObject() == NULL);
  RT_TEST(v1.toFunction() == NULL);
  RT_TEST(v1.toString() == "Hello");
  RT_TEST(v1.toVoidPtr() == NULL);

}
