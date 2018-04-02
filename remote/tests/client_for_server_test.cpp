/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * This is a sample client for testing rtRemoteServerTest
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtRemote.h"
#include "rtObject.h"

#include <cmath>
#include <limits>

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
 * do type test
 * @param objectRef Object reference
 * @param type the test value's type
 * @param value the test value
 */
void typeTest(rtObjectRef objectRef, const char* type, rtValue value)
{
  rtValue newVal;
  if (RT_OK != objectRef.set(type, value))
  {
    rtLogError("[%s][%d]Failed to set value\n", __FUNCTION__, __LINE__);
    return;
  }
  
  if (RT_OK != objectRef.get(type, newVal))
  {
    rtLogError("[%s][%d]Failed to get value\n", __FUNCTION__, __LINE__);
    return;
  }
  if (type == "vptr")
  {
    if (value.toVoidPtr() != newVal.toVoidPtr())
    {
      rtLogError("[%s][%d]Failed to check vptr equality\n", __FUNCTION__, __LINE__);
      return;
    }
  }
  else if (type == "ffloat")
  {
    if (std::fabs(value.toFloat() - newVal.toFloat()) > std::numeric_limits<float>::epsilon())
    {
      rtLogError("[%s][%d]Failed to check float equality\n", __FUNCTION__, __LINE__);
      return;
    }
  }
  else if(type == "ddouble")
  {
    if (std::fabs(value.toDouble() - newVal.toDouble()) > std::numeric_limits<double>::epsilon())
    {
      rtLogError("[%s][%d]Failed to check double equality: This is a bug in pxCore, send double value set to remote server,"
        "But it return float value\n", __FUNCTION__, __LINE__);
      return;
    }
  }
  else
  {
    if (value != newVal)
    {
      rtLogError("[%s][%d]Failed to check equality\n", __FUNCTION__, __LINE__);
    }
  }
}

/**
 * test name: method0AndReturn10Test
 * test message: method.call.request
 * test method: method0AndReturn10
 * function: send value to remote and get 10
 */
void method0AndReturn10Test(rtObjectRef objectRef)
{
  rtValue methodVal[1];
  rtValue result;
  methodVal[0] = 168;
  if (RT_OK != objectRef.SendReturns("method0AndReturn10", 1, methodVal, result))
  {
    rtLogError("[%s][%d]Failed to send RPC method0AndReturn10\n", __FUNCTION__, __LINE__);
    return;
  }
  if (10 != result.toInt32())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test name: method1AndReturnTest
 * test message: method.call.request
 * test method: method1AndReturn
 * function: send value to remote and the server
 * will return cast int of 2*value.
 */
void method1AndReturnTest(rtObjectRef objectRef)
{
  rtValue methodVal[1];
  rtValue result;
  methodVal[0] = 168;
  int32_t expectedVal;
  expectedVal = 2 * 168;
  if (RT_OK != objectRef.SendReturns("method1AndReturn", 1, methodVal, result))
  {
    rtLogError("[%s][%d]Failed to send RPC method1AndReturn\n", __FUNCTION__, __LINE__);
    return;
  }
  if (expectedVal != result.toInt32())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test name: twoIntNumberSumTest
 * test message: method.call.request
 * test method: twoIntNumberSum
 * function: send two values to remote and the server will
 * return cast int of sum of this two values.
 */
void twoIntNumberSumTest(rtObjectRef objectRef)
{
  int32_t expectedVal;
  rtValue methodVal[2];
  rtValue result;
  methodVal[0] = 168;
  methodVal[1] = 668;
  expectedVal = 168 + 668;
  if (RT_OK != objectRef.SendReturns("twoIntNumberSum", 2, methodVal, result))
  {
    rtLogError("[%s][%d]Failed to send RPC twoIntNumberSum\n", __FUNCTION__, __LINE__);
    return;
  }
  if (expectedVal != result.toInt32())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test name: twoFloatNumberSumTest
 * test message: method.call.request
 * test method: twoFloatNumberSum
 * function: send  two value to remote and get result (that is cast float from sum of them) from response of remote.
 */
void twoFloatNumberSumTest(rtObjectRef objectRef)
{
  rtValue methodVal[2];
  rtValue result;
  float expectedVal;
  methodVal[0] = 168.82312f;
  methodVal[1] = 628.812312f;
  expectedVal = 168.82312f + 628.812312f;
  if (RT_OK != objectRef.SendReturns("twoFloatNumberSum", 2, methodVal, result))
  {
    rtLogError("[%s][%d]Failed to send RPC twoFloatNumberSum\n", __FUNCTION__, __LINE__);
    return;
  }
  if (expectedVal != result.toFloat())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test name: method1IntAndNoReturnTest
 * test message: method.call.request
 * test method: method1IntAndNoReturn
 * function: send value to method1IntAndNoReturn in remote and get no response.
 */
void method1IntAndNoReturnTest(rtObjectRef objectRef)
{
  rtValue value = 10;
  if (RT_OK != objectRef.send("method1IntAndNoReturn", value))
  {
    rtLogError("[%s][%d]Failed to send RPC method1IntAndNoReturn\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test name: method2FunctionAndNoReturnTest
 * test message: method.call.request
 * test method: method2FunctionAndNoReturn
 * function: send function to remote and get no response.
 */
void method2FunctionAndNoReturnTest(rtObjectRef objectRef)
{
  rtValue methodVal[2];
  rtFunctionRef callback(new rtFunctionCallback(testFunc));
  methodVal[0] = callback;
  methodVal[1] = 168;
  if (RT_OK != objectRef.send("method2FunctionAndNoReturn", methodVal))
  {
    rtLogError("[%s][%d]Failed to send RPC method1IntAndNoReturn\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test message set.byindex.request
 */
void setByIndexTest(rtObjectRef objectRef)
{
  rtValue setVal = 10;
  // the returan value of objectRef.set(1, setVal) should be RT_OK,
  // But There are bugs that cause the return value not equal RT_OK in the code of pxCore
  rtError retVal = objectRef.set(1, setVal);
  if (retVal != RT_OK)
  {
    rtLogError("[%s][%d]This is a bug in pxCore, when client send set.byindex.request to server, "
    "it will return set.byname.response not set.byindex.response", __FUNCTION__, __LINE__);
  }
}

/**
 * test get.byindex.request
 */
void getByIndexTest(rtObjectRef objectRef)
{
  rtValue setVal = 88;
  // the return value of objectRef.set(1, setVal) should be RT_OK,
  // But There are bugs that cause the return value not equal RT_OK in the code of pxCore
  if (RT_OK != objectRef.set(1, setVal))
  {
    rtLogError("[%s][%d]Failed to set value", __FUNCTION__, __LINE__);
    return;
  }
  rtValue newVal;
  // the return value of objectRef.get(1, newVal) should be RT_OK,
  // But There are bugs that cause the return value not equal RT_OK in the code of pxCore
  rtError retVal = objectRef.get(1, newVal);
  if (retVal != RT_OK)
  {
    rtLogError("[%s][%d]This is a bug in pxCore, rtRemote client doesn't suport message get.byindex.request", 
      __FUNCTION__, __LINE__);
  }
  // setval should equal newVal
  if (setVal != newVal)
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * name: SetAndGetValueTest
 * message: set.byname.request, get.byname.request
 * function: test message set.byname.request and get.byname.request
 */
void setAndGetValueTest(rtObjectRef objectRef)
{
  rtValue value = 168;
  if (RT_OK != objectRef.set("count", value))
  {
    rtLogError("[%s][%d]Failed to set count", __FUNCTION__, __LINE__);
    return;
  }
  rtValue newValue;
  if (RT_OK != objectRef.get("count", newValue))
  {
    rtLogError("[%s][%d]Failed to get count", __FUNCTION__, __LINE__);
    return;
  }
  if (value != newValue.toInt32())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test error value type
 * test fffloat type value set
 */
void errorValueTypeTest(rtObjectRef objectRef)
{
  float value = 168.8f;
  // test not set valut type in remote
  if (RT_PROP_NOT_FOUND != objectRef.set("fffloat", value))
  {
    rtLogError("[%s][%d]Failed to set fffloat", __FUNCTION__, __LINE__);
  }
}

/**
 * test not set string type, but get string type value
 */
void notSetButGetValueTest(rtObjectRef objectRef)
{
  rtValue value;
  rtString expectedVal = "";
  // string type value is not set, when get string type value, the remote should return ""
  if (RT_OK != objectRef.get("string", value))
  {
    rtLogError("[%s][%d]Failed to get string", __FUNCTION__, __LINE__);
    return;
  }
  if (expectedVal != value.toString())
  {
    rtLogError("[%s][%d]Result check failed\n", __FUNCTION__, __LINE__);
  }
}

/**
 * test object type value
 */
void objectTypeTest(rtObjectRef objectRef)
{
  rtObjectRef object(new rtRpcUnitTestObject());
  const char* type = "objvar";
  typeTest(objectRef, type, object);
}

/**
 * test function type value
 */
void funcTypeTestt(rtObjectRef objectRef)
{
  rtFunctionRef func(new rtFunctionCallback(testFunc));
  const char* type = "onTick";
  typeTest(objectRef, type, func);
}

/**
 * test float type value
 */
void floatTypeTest(rtObjectRef objectRef)
{
  const char* type = "ffloat";
  // test positive float value
  float value = 12.0123123f;
  typeTest(objectRef, type, value);

  // test negative float value
  value = -12.0123123f;
  typeTest(objectRef, type, value);

  // test more floating-point value
  value = 199123123.9188686828f;
  typeTest(objectRef, type, value);
}

/**
 * test bool type value
 * range is [false, true]
 */
void boolTypeTest(rtObjectRef objectRef)
{
  const char* type = "bbool";
  // test true
  bool value = true;
  typeTest(objectRef, type, value);

  // test false
  value = false;
  typeTest(objectRef, type, value);
}

/**
 * test int8 type value
 * range is [INT8_MIN, INT8_MAX]
 */
void int8TypeTest(rtObjectRef objectRef)
{
  const char* type = "int8";
  // test INT8_MIN
  int8_t value = INT8_MIN;
  typeTest(objectRef, type, value);

  // test a num between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test INT8_MAX
  value = INT8_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test uint8 type value
 * range is [0, UINT8_MAX]
 */
void uint8TypeTest(rtObjectRef objectRef)
{
  const char* type = "uint8";
  // test min value 0
  uint8_t value = 0;
  typeTest(objectRef, type, value);

  // test a value between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test UINT8_MAX
  value = UINT8_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test int32 type value
 * range is [0, INT32_MAX]
 */
void int32TypeTest(rtObjectRef objectRef)
{
  const char* type = "int32";
  // test INT32_MIN
  int32_t value = INT32_MIN;
  typeTest(objectRef, type, value);

  // test a value between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test INT32_MAX
  value = INT32_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test uint32 type value
 * range is [0, UINT32_MAX]
 */
void uint32TypeTest(rtObjectRef objectRef)
{
  const char* type = "uint32";
  // test min value 0
  uint32_t value = 0;
  typeTest(objectRef, type, value);

  // test a value between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test UINT32_MAX
  value = UINT32_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test int64 type value
 * range is [INT64_MIN, INT64_MAX]
 */
void int64TypeTest(rtObjectRef objectRef)
{
  const char* type = "int64";
  // test INT64_MIN
  int64_t value = INT64_MIN;
  typeTest(objectRef, type, value);

  // test a value between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test INT64_MAX
  value = INT64_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test uint64 type value
 * range is [0, UINT64_MAX]
 */
void uint64TypeTest(rtObjectRef objectRef)
{
  const char* type = "uint64";
  // test min value 0
  uint64_t value = 0;
  typeTest(objectRef, type, value);

  // test a value between min and max
  value = 8;
  typeTest(objectRef, type, value);

  // test UINT64_MAX
  value = UINT64_MAX;
  typeTest(objectRef, type, value);
}

/**
 * test double type value
 */
void doubleTypeTest(rtObjectRef objectRef)
{
  const char* type = "ddouble";
  // test negative double value
  double value = -168.8;
  typeTest(objectRef, type, value);

  // test positive double value
  value = 168.8;
  typeTest(objectRef, type, value);

  // test more floating-point value
  value = 1231.12312312312;
  typeTest(objectRef, type, value);
}

/**
 * test string type value
 */
void stringTypeTest(rtObjectRef objectRef)
{
  const char* type = "string";
  // test long string
  rtValue value = "- In the C++ server, when writing to the \"ddouble\" property, \
  which has a DOUBLE type, it seems the property has the float precision, even though \
  it seems it has declared the right type. Well, more or less, it is strange. It is like\
   it doesn't support numbers with several integer digits";
  typeTest(objectRef, type, value);
  // test Null character string
  value = "";
  typeTest(objectRef, type, value);
}

/**
 * test voidPtr type value
 */
void voidPtrTypeTest(rtObjectRef objectRef)
{
  const char* type = "vptr";
  // test uint64 value cast to voidPtr
  voidPtr value = (voidPtr)789892389L;
  typeTest(objectRef, type, value);
}

/**
 * Run all tests
 */
void runTests(rtObjectRef objectRef)
{
  // Call test functions
  method0AndReturn10Test(objectRef);
  method1AndReturnTest(objectRef);
  twoIntNumberSumTest(objectRef);
  twoFloatNumberSumTest(objectRef);
  method1IntAndNoReturnTest(objectRef);
  method2FunctionAndNoReturnTest(objectRef);
  setByIndexTest(objectRef);
  getByIndexTest(objectRef);
  setAndGetValueTest(objectRef);
  errorValueTypeTest(objectRef);
  notSetButGetValueTest(objectRef);
  objectTypeTest(objectRef);
  funcTypeTestt(objectRef);
  floatTypeTest(objectRef);
  boolTypeTest(objectRef);
  int8TypeTest(objectRef);
  uint8TypeTest(objectRef);
  int32TypeTest(objectRef);
  uint32TypeTest(objectRef);
  int64TypeTest(objectRef);
  uint64TypeTest(objectRef);
  doubleTypeTest(objectRef);
  stringTypeTest(objectRef);
  voidPtrTypeTest(objectRef);
}

int main()
{
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  // objectRef is used to set and get value
  rtObjectRef objectRef;
  rtString hostObject = "host_object";

  if (RT_OK != rtRemoteInit(env))
  {
    rtLogError("Failed to initialize environtment.\n");
    return 1;
  }
  rtError e;
  while ((e = rtRemoteLocateObject(env, hostObject, objectRef)) !=  RT_OK)
  {
    rtLogError("Still looking for remote object: %s\n", rtStrError(e));
  }

  // Client only runs 30 seconds, so to ensure the server is still running when closing
  // this client.
  const int MAX_LOOP_COUNT = 30;
  int loopCount = 0;
  while (true)
  {
    rtRemoteRunUntil(env, 1000);
    runTests(objectRef);

     // Exit if it has been listening enough time.
    if (loopCount >= MAX_LOOP_COUNT)
    {
      rtLogInfo("Exit\n");
      break;
    }
    loopCount++;
  }

  // Try to locate the same object again for covering findObject in rtRemoteServer.cpp
  while ((e = rtRemoteLocateObject(env, hostObject, objectRef)) !=  RT_OK)
  {
    rtLogError("Still looking for remote object: %s\n", rtStrError(e));
  }

  if (RT_OK != rtRemoteShutdown(env))
  {
    rtLogError("Failed to shutdown the client.\n");
    return 1;
  }
  return 0;
}