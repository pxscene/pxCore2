/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to
 * test message parsing and message response
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtTestIncludes.h"
/**
 * used to test Object
 */
class rtRpcUnitTestObject : public rtObject
{
  rtDeclareObject(rtRpcUnitTestObject, rtObject);
public:
  rtProperty(count, getCount, setCount, int);
  rtError getCount(int& n) const { n = m_count; return RT_OK; }
  rtError setCount(int  n) { m_count = n; return RT_OK; }
private:
  int m_count;
};
rtDefineObject(rtRpcUnitTestObject, rtObject);
rtDefineProperty(rtRpcUnitTestObject, count);
/**
 * used to test function
 */
rtError
testFunc(int argc, rtValue const* argv, rtValue* result, void* argp )
{
  printf("testFunc\n");
  printf("argc:%d\n", argc);
  printf("argv[0]:%s\n", argv[0].toString().cString());

  (void) result;
  (void) argp;

  return RT_OK;
}

/**
 * rtRpcUnitTest class
 */
class rtRpcUnitTest : public ::testing::Test
{
protected:
  /**
   * init environment and get hostObject
   */
  virtual void SetUp() {
   rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
   EXPECT_EQ(RT_OK, rtRemoteInit(env));
   EXPECT_EQ(RT_OK, rtRemoteLocateObject(env, hostObject, objectRef));
 }

 /**
  * clear environment
  */
 virtual void TearDown() {
   EXPECT_EQ(RT_OK, rtRemoteShutdown());
 }

 /**
  * do type test
  * @param type the test value's type
  * @param value the test value
  */
 void typeTest(const char* type, rtValue value){
   rtValue newVal;
   EXPECT_EQ(RT_OK, objectRef.set(type, value));
   EXPECT_EQ(RT_OK, objectRef.get(type, newVal));
   if(type == "vptr"){
     EXPECT_EQ(value.toVoidPtr(), newVal.toVoidPtr());
   }
   else if(type == "ffloat"){
     EXPECT_FLOAT_EQ(value.toFloat(), newVal.toFloat());
   }
   else if(type == "ddouble"){
     EXPECT_DOUBLE_EQ(value.toDouble(), newVal.toDouble());
     bool equalFlag = (value.toDouble() == newVal.toDouble());
     if(!equalFlag){
      ADD_FAILURE() << "This is a bug in pxCore, send double value set to remote server, But it return float value";
     }
   }
   else {
     EXPECT_EQ(value, newVal);
   }
 }
 // objectRef is used to set and get value
 rtObjectRef objectRef;
 rtString hostObject = "host_object";
};

/**
 * test name: method0AndReturn10Test
 * test message: method.call.request
 * test method: method0AndReturn10
 * function: send value to remote and get 10
 */
TEST_F(rtRpcUnitTest, method0AndReturn10Test)
{
  rtValue methodVal[1];
  rtValue result;
  methodVal[0] = 168;
  EXPECT_EQ(RT_OK, objectRef.SendReturns("method0AndReturn10", 1, methodVal, result));
  EXPECT_EQ(10, result.toInt32());
}

/**
 * test name: method1AndReturnTest
 * test message: method.call.request
 * test method: method1AndReturn
 * function: send value to remote and the server
 * will return cast int of 2*value.
 */
TEST_F(rtRpcUnitTest, method1AndReturnTest)
{
  rtValue methodVal[1];
  rtValue result;
  methodVal[0] = 168;
  int32_t expectedVal;
  expectedVal = 2 * 168;
  EXPECT_EQ(RT_OK, objectRef.SendReturns("method1AndReturn", 1, methodVal, result));
  EXPECT_EQ(expectedVal, result.toInt32());
}

/**
 * test name: twoIntNumberSumTest
 * test message: method.call.request
 * test method: twoIntNumberSum
 * function: send two values to remote and the server will
 * return cast int of sum of this two values.
 */
TEST_F(rtRpcUnitTest, twoIntNumberSumTest)
{
  int32_t expectedVal;
  rtValue methodVal[2];
  rtValue result;
  methodVal[0] = 168;
  methodVal[1] = 668;
  expectedVal = 168 + 668;
  EXPECT_EQ(RT_OK, objectRef.SendReturns("twoIntNumberSum", 2, methodVal, result));
  EXPECT_EQ(expectedVal, result.toInt32());
}

/**
 * test name: twoFloatNumberSumTest
 * test message: method.call.request
 * test method: twoFloatNumberSum
 * function: send  two value to remote and get result (that is cast float from sum of them) from response of remote.
 */
TEST_F(rtRpcUnitTest, twoFloatNumberSumTest)
{
  rtValue methodVal[2];
  rtValue result;
  float expectedVal;
  methodVal[0] = 168.82312f;
  methodVal[1] = 628.812312f;
  expectedVal = 168.82312f + 628.812312f;
  EXPECT_EQ(RT_OK, objectRef.SendReturns("twoFloatNumberSum", 2, methodVal, result));
  EXPECT_EQ(expectedVal, result.toFloat());
}

/**
 * test name: method1IntAndNoReturnTest
 * test message: method.call.request
 * test method: method1IntAndNoReturn
 * function: send value to method1IntAndNoReturn in remote and get no response.
 */
TEST_F(rtRpcUnitTest, method1IntAndNoReturnTest)
{
  rtValue value = 10;
  EXPECT_EQ(RT_OK, objectRef.send("method1IntAndNoReturn", value));
}

/**
 * test name: method2FunctionAndNoReturnTest
 * test message: method.call.request
 * test method: method2FunctionAndNoReturn
 * function: send function to remote and get no response.
 */
TEST_F(rtRpcUnitTest, method2FunctionAndNoReturnTest)
{
  rtValue methodVal[2];
  rtFunctionRef callback(new rtFunctionCallback(testFunc));
  methodVal[0] = callback;
  methodVal[1] = 168;
  EXPECT_EQ(RT_OK, objectRef.send("method2FunctionAndNoReturn", methodVal));
}

/**
 * test message set.byindex.request
 */
TEST_F(rtRpcUnitTest, setByIndexTest)
{
  rtValue setVal = 10;
  /**
   * the returan value of objectRef.set(1, setVal) should be RT_OK,
   * But There are bugs that cause the return value not equal RT_OK in the code of pxCore
   */
  rtError retVal = objectRef.set(1, setVal);
  EXPECT_EQ(RT_OK, retVal);
  if(retVal != RT_OK){
    ADD_FAILURE() << "This is a bug in pxCore, when client send set.byindex.request to server, "
   <<"it will return set.byname.response not set.byindex.response";
  }
}

/**
 * test get.byindex.request
 */
TEST_F(rtRpcUnitTest, getByIndexTest)
{
  rtValue setVal = 88;
  /**
   * the returan value of objectRef.set(1, setVal) should be RT_OK,
   * But There are bugs that cause the return value not equal RT_OK in the code of pxCore
   */
  EXPECT_EQ(RT_OK, objectRef.set(1, setVal));
  rtValue newVal;
  /**
   * the returan value of objectRef.get(1, newVal) should be RT_OK,
   * But There are bugs that cause the return value not equal RT_OK in the code of pxCore
   */
  rtError retVal = objectRef.get(1, newVal);
  EXPECT_EQ(RT_OK, retVal);
  if(retVal != RT_OK){
    ADD_FAILURE() << "This is a bug in pxCore, rtRemote client doesn't suport message get.byindex.request";
  }
  // setval should equal newVal
  EXPECT_EQ(setVal, newVal);
}

/**
 * name: SetAndGetValueTest
 * message: set.byname.request, get.byname.request
 * function: test message set.byname.request and get.byname.request
 */
TEST_F(rtRpcUnitTest, setAndGetValueTest)
{
  rtValue value = 168;
  EXPECT_EQ(RT_OK, objectRef.set("count", value));
  rtValue newValue;
  EXPECT_EQ(RT_OK, objectRef.get("count", newValue));
  EXPECT_EQ(value, newValue.toInt32());
}

/**
 * test error value type
 * test fffloat type value set
 */
TEST_F(rtRpcUnitTest, errorValueTypeTest)
{
  float value = 168.8f;
  // test not set valut type in remote
  EXPECT_EQ(RT_PROP_NOT_FOUND, objectRef.set("fffloat", value));
}

/**
 * test not set string type, but get string type value
 */
TEST_F(rtRpcUnitTest, notSetButGetValueTest){
  rtValue value;
  rtString expectedVal = "";
  // string type value is not set, when get string type value, the remote should return ""
  EXPECT_EQ(RT_OK, objectRef.get("string", value));
  EXPECT_EQ(expectedVal, value.toString());
}

/**
 * test object type value
 */
TEST_F(rtRpcUnitTest, objectTypeTest){
  rtObjectRef object(new rtRpcUnitTestObject());
  const char* type = "objvar";
  typeTest(type, object);
}

/**
 * test function type value
 */
TEST_F(rtRpcUnitTest, funcTypeTest){
  rtFunctionRef func(new rtFunctionCallback(testFunc));
  const char* type = "onTick";
  typeTest(type, func);
}

/**
 * test float type value
 */
TEST_F(rtRpcUnitTest, floatTypeTest){
  const char* type = "ffloat";
  // test positive float value
  float value = 12.0123123f;
  typeTest(type, value);

  // test negative float value
  value = -12.0123123f;
  typeTest(type, value);

  // test more floating-point value
  value = 199123123.9188686828f;
  typeTest(type, value);
}

/**
 * test bool type value
 * range is [false, true]
 */
TEST_F(rtRpcUnitTest, boolTypeTest){
  const char* type = "bbool";
  // test true
  bool value = true;
  typeTest(type, value);

  // test false
  value = false;
  typeTest(type, value);
}

/**
 * test int8 type value
 * range is [INT8_MIN, INT8_MAX]
 */
TEST_F(rtRpcUnitTest, int8TypeTest){
  const char* type = "int8";
  // test INT8_MIN
  int8_t value = INT8_MIN;
  typeTest(type, value);

  // test a num between min and max
  value = 8;
  typeTest(type, value);

  // test INT8_MAX
  value = INT8_MAX;
  typeTest(type, value);
}

/**
 * test uint8 type value
 * range is [0, UINT8_MAX]
 */
TEST_F(rtRpcUnitTest, uint8TypeTest){
  const char* type = "uint8";
  // test min value 0
  uint8_t value = 0;
  typeTest(type, value);

  // test a value between min and max
  value = 8;
  typeTest(type, value);

  // test UINT8_MAX
  value = UINT8_MAX;
  typeTest(type, value);
}

/**
 * test int32 type value
 * range is [0, INT32_MAX]
 */
TEST_F(rtRpcUnitTest, int32TypeTest){
  const char* type = "int32";
  // test INT32_MIN
  int32_t value = INT32_MIN;
  typeTest(type, value);

  // test a value between min and max
  value = 8;
  typeTest(type, value);

  // test INT32_MAX
  value = INT32_MAX;
  typeTest(type, value);
}

/**
 * test uint32 type value
 * range is [0, UINT32_MAX]
 */
TEST_F(rtRpcUnitTest, uint32TypeTest){
  const char* type = "uint32";
  // test min value 0
  uint32_t value = 0;
  typeTest(type, value);

  // test a value between min and max
  value = 8;
  typeTest(type, value);

  // test UINT32_MAX
  value = UINT32_MAX;
  typeTest(type, value);
}

/**
 * test int64 type value
 * range is [INT64_MIN, INT64_MAX]
 */
TEST_F(rtRpcUnitTest, int64TypeTest){
  const char* type = "int64";
  // test INT64_MIN
  int64_t value = INT64_MIN;
  typeTest(type, value);

  // test a value between min and max
  value = 8;
  typeTest(type, value);

  // test INT64_MAX
  value = INT64_MAX;
  typeTest(type, value);
}

/**
 * test uint64 type value
 * range is [0, UINT64_MAX]
 */
TEST_F(rtRpcUnitTest, uint64TypeTest){
  const char* type = "uint64";
  // test min value 0
  uint64_t value = 0;
  typeTest(type, value);

  // test a value between min and max
  value = 8;
  typeTest(type, value);

  // test UINT64_MAX
  value = UINT64_MAX;
  typeTest(type, value);
}

/**
 * test double type value
 */
TEST_F(rtRpcUnitTest, doubleTypeTest){
  const char* type = "ddouble";
  // test negative double value
  double value = -168.8;
  typeTest(type, value);

  // test positive double value
  value = 168.8;
  typeTest(type, value);

  // test more floating-point value
  value = 1231.12312312312;
  typeTest(type, value);
}

/**
 * test string type value
 */
TEST_F(rtRpcUnitTest, stringTypeTest){
  const char* type = "string";
  // test long string
  rtValue value = "- In the C++ server, when writing to the \"ddouble\" property, \
  which has a DOUBLE type, it seems the property has the float precision, even though \
  it seems it has declared the right type. Well, more or less, it is strange. It is like\
   it doesn't support numbers with several integer digits";
  typeTest(type, value);
  // test Null character string
  value = "";
  typeTest(type, value);
}

/**
 * test voidPtr type value
 */
TEST_F(rtRpcUnitTest, voidPtrTypeTest){
  const char* type = "vptr";
  // test uint64 value cast to voidPtr
  voidPtr value = (voidPtr)789892389L;
  typeTest(type, value);
}
