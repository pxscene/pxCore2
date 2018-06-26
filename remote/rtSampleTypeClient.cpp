/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains all type test examples
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtRemote.h"
#include "rtRemoteObject.h"
#include "rtRemoteClient.h"
#include <unistd.h>
#include <memory>
#include <cmath>
#include <sys/socket.h>


// the total examples
int totalExamplesCount = 0;


// the suceed examples
int succeedExamplesCount = 0;
rtRemoteEnvironment* env = rtEnvironmentGetGlobal();

// check result
void
checkResult(rtValue const& value, rtValue const& newVal)
{
  bool result = false;
  char buffer[1024];
  switch (newVal.getType())
  {
    case RT_floatType:
    {
      float v1, v2;
      value.getFloat(v1);
      newVal.getFloat(v2);
      sprintf(buffer, "set val = %f, rpc result = %f", v1, v2);
      result = abs(v1 - v2) < 0.001f;
      break;
    }
    case RT_doubleType:
    {
      double v1, v2;
      value.getDouble(v1);
      newVal.getDouble(v2);
      sprintf(buffer, "set val = %lf, rpc result = %lf", v1, v2);
      result = abs(v1 - v2) < 0.0001;
      break;
    }
    case RT_boolType:
    {
      bool v1, v2;
      value.getBool(v1);
      newVal.getBool(v2);
      sprintf(buffer, "set val = %s, rpc result = %s", v1 ? "true" : "false", v2 ? "true" : "false");
      result = v1 == v2;
      break;
    }
    case RT_rtStringType:
    {
      rtString v1, v2;
      value.getString(v1);
      newVal.getString(v2);
      sprintf(buffer, "set val = %s, rpc result = %s", v1.cString(), v2.cString());
      result = v1.compare(v2.cString()) == 0;
      break;
    }
    case RT_voidPtrType:
    {
      voidPtr v1, v2;
      value.getVoidPtr(v1);
      newVal.getVoidPtr(v2);
      sprintf(buffer, "set val = %p, rpc result = %p", v1, v2);
      result = v1 == v2;
      break;
    }

    case RT_int8_tType:
    case RT_uint8_tType:
    case RT_int32_tType:
    case RT_uint32_tType:
    case RT_int64_tType: // use uint64 for print
    {
      int64_t v1, v2;
      value.getInt64(v1);
      newVal.getInt64(v2);
      sprintf(buffer, "set val = %ld, rpc result = %ld", v1, v2);
      result = v1 == v2;
      break;
    }
    case RT_objectType:
    {
      rtObjectRef v1, v2;
      value.getObject(v1);
      newVal.getObject(v2);
      auto const* obj1 = dynamic_cast<rtRemoteObject const*>(v1.getPtr());
      auto const* obj2 = dynamic_cast<rtRemoteObject const*>(v2.getPtr());
      sprintf(buffer, "set val = %s, rpc result = %s", obj1->getId().c_str(), obj2->getId().c_str());
      result = obj1->getId().compare(obj2->getId()) == 0;
      break;
    }
    case RT_uint64_tType:
    {
      uint64_t v1, v2;
      value.getUInt64(v1);
      newVal.getUInt64(v2);
      sprintf(buffer, "set val = %lu, rpc result = %lu", v1, v2);
      result = v1 == v2;
      break;
    }
    case RT_functionType:
    {
      rtFunctionRef v1, v2;
      value.getFunction(v1);
      newVal.getFunction(v2);
      v2.send();
      sprintf(buffer, "set val = function, rpc result = function");
      result = v1 == v2;
      break;
    }
    default:
    {
    }
  }
  succeedExamplesCount += (result ? 1 : 0);
  rtLogInfo("%s test => %s, passed=[%s]", value.getTypeStr(), buffer, result ? "true" : "false");
}

// test basic type
void
doBasicTest(const rtObjectRef& remoteObj, const rtValue& value, const char* propName)
{

  totalExamplesCount += 1;
  rtError e = remoteObj->Set(propName, &value);
  if (e != RT_OK)
  {
    rtLogInfo("%s , passed=[%s], err=%s", value.getTypeStr(), "false", rtStrError(e));
    return;
  }

  rtValue newVal;
  e = remoteObj->Get(propName, &newVal);
  if (e != RT_OK)
  {
    rtLogInfo("%s , passed=[%s], err=%s", value.getTypeStr(), "false", rtStrError(e));
    return;
  }
  checkResult(value, newVal);
}

// test basic type with index
void
doBasicTestWithIndex(const rtObjectRef& remoteObj, const rtValue& value, uint32_t index)
{
  totalExamplesCount += 1;
  rtValue rtArrayVal;
  rtError e = remoteObj->Get("arr", &rtArrayVal);  // get arr object first
  if (e != RT_OK)
  {
    rtLogInfo("Get arr object failed, passed = [%s]", "false");
  }
  rtObjectRef arrRef;
  rtArrayVal.getObject(arrRef);

  e = arrRef->Set(index, &value);
  if (e != RT_OK)
  {
    rtLogInfo("%s , passed=[%s], err=%s", value.getTypeStr(), "false", rtStrError(e));
    return;
  }
  rtValue newVal;
  e = arrRef->Get(index, &newVal);
  if (e != RT_OK)
  {
    rtLogInfo("%s , passed=[%s], err=%s", value.getTypeStr(), "false", rtStrError(e));
    return;
  }
  checkResult(value, newVal);
}

void
testAllTypes(const rtObjectRef& remoteObj)
{
  rtLogInfo("======== start test rtValue type =======");
  //float last 7 digit
  doBasicTest(remoteObj, rtValue(12.0123123f), "ffloat");
  doBasicTest(remoteObj, rtValue(-123.8818f), "ffloat");
  doBasicTest(remoteObj, rtValue(199123123.91f), "ffloat");

  //test bool
  doBasicTest(remoteObj, rtValue(true), "bbool");
  doBasicTest(remoteObj, rtValue(false), "bbool");

  // int8 range [-128,127]
  doBasicTest(remoteObj, rtValue((int8_t) -128), "int8");
  doBasicTest(remoteObj, rtValue((int8_t) 0), "int8");
  doBasicTest(remoteObj, rtValue((int8_t) 127), "int8");

  //test uint8, the data range is[0,255]
  doBasicTest(remoteObj, rtValue((uint8_t) 0), "uint8");
  doBasicTest(remoteObj, rtValue((uint8_t) 255), "uint8");

  //test int32, range is 	[–2147483648 , 2147483647]
  doBasicTest(remoteObj, rtValue((int32_t) -2147483648), "int32");
  doBasicTest(remoteObj, rtValue((int32_t) 0), "int32");
  doBasicTest(remoteObj, rtValue((int32_t) 123), "int32");
  doBasicTest(remoteObj, rtValue((int32_t) 2147483647), "int32");

  // test uint32, range is [0 - 4,294,967,295]
  doBasicTest(remoteObj, rtValue((uint32_t) 0), "uint32");
  doBasicTest(remoteObj, rtValue((uint32_t) 4294967295), "uint32");
  doBasicTest(remoteObj, rtValue((uint32_t) 123123), "uint32");

  // test int64, range is [–9223372036854775808  9223372036854775807]
  doBasicTest(remoteObj, rtValue((int64_t) -9223372036854775808U), "int64");
  doBasicTest(remoteObj, rtValue((int64_t) 9223372036854775807L), "int64");
  doBasicTest(remoteObj, rtValue((int64_t) 0), "int64");
  doBasicTest(remoteObj, rtValue((int64_t) 123123), "int64");

  // test uint64, range is [0 - 18446744073709551615]
  doBasicTest(remoteObj, rtValue((uint64_t) 18446744073709551615U), "uint64");
  doBasicTest(remoteObj, rtValue((uint64_t) 0), "uint64");
  doBasicTest(remoteObj, rtValue((uint64_t) 123123123), "uint64");
  doBasicTest(remoteObj, rtValue((uint64_t) 123), "uint64");

  // test double
  doBasicTest(remoteObj, rtValue(1231.12312312312), "ddouble");
  doBasicTest(remoteObj, rtValue(-1231.12312312312), "ddouble");
  doBasicTest(remoteObj, rtValue(-0.12), "ddouble");

  // test string
  doBasicTest(remoteObj, rtValue(
      "implemented in both /Library/Java/JavaVirtualMachines/jdk1.8.0_40.jdk/Conten"), "string");
  doBasicTest(remoteObj, rtValue("{\"jsonKey\":\"values\"}"), "string");
  doBasicTest(remoteObj, rtValue("1"), "string");

  // void ptr, java/node void ptr use int64 to store
  doBasicTest(remoteObj, rtValue((voidPtr) 723123231L), "vptr");
  doBasicTest(remoteObj, rtValue((voidPtr) 789892349L), "vptr");
  rtLogInfo("========= %d of %d example succeed, %d failed.", succeedExamplesCount,
            totalExamplesCount, totalExamplesCount - succeedExamplesCount);
}


// index test
void
doIndexTest(const rtObjectRef& rtObject)
{
  doBasicTestWithIndex(rtObject, rtValue(102), 0);
  doBasicTestWithIndex(rtObject, rtValue(122.123f), 1);
  doBasicTestWithIndex(rtObject, rtValue("Hello world !!!"), 2);
}

// object test
void
doObjectTest(const rtObjectRef& rtObject, const char* propertyName)
{
  std::shared_ptr<rtRemoteClient> client(new rtRemoteClient(env, sockaddr_storage()));
  rtValue oldVal = rtValue(new rtRemoteObject("test_id", client));
  doBasicTest(rtObject, oldVal, propertyName);
}


rtError
functionTest(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogInfo("function name = %s", "functionTest");
  rtLogInfo("argc:%d", argc);
  return RT_OK;
}
void
doFunctionTest(const rtObjectRef& rtObject, const char* propertyName)
{
  rtFunctionRef callback(new rtFunctionCallback(functionTest));
  doBasicTest(rtObject, rtValue(callback), propertyName);
}

int
main(int /*argc*/, char* /*argv*/ [])
{
  rtError e;
  rtLogSetLevel(RT_LOG_INFO);
  e = rtRemoteInit(env);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  rtObjectRef obj;
  // find object
  while ((e = rtRemoteLocateObject(env, "host_object", obj)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  while (true)
  {
    totalExamplesCount = 0;
    succeedExamplesCount = 0;
    doFunctionTest(obj, "onTick");
    doObjectTest(obj, "objvar");
    doIndexTest(obj);
    testAllTypes(obj);
    rtLogInfo("test completed, next test will at %ds ...", 10);
    sleep(10);
  }
  return 0;
}