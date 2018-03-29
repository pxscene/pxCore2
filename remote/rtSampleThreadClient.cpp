/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains four object test examples
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtRemote.h"
#include "rtRemoteClient.h"
#include "rtRemoteObject.h"
#include <unistd.h>


rtRemoteEnvironment* env = rtEnvironmentGetGlobal();

// test basic type
void
doBasicTest(const rtObjectRef& remoteObj, const rtValue& value, const char* propName)
{
  auto const* obj = dynamic_cast<rtRemoteObject const*>(remoteObj.getPtr());
  remoteObj->Set(propName, &value);
  rtValue newVal;
  remoteObj->Get(propName, &newVal);
  char buff[512];
  sprintf(buff, "[%s]:%s test => %s, rpc val = %s, passed=[%s]",
          obj->getId().c_str(),
          value.getTypeStr(),
          value.toString().cString(),
          newVal.toString().cString(),
          (value == newVal ? "true" : "false"));
  rtLogInfo("%s", buff);
}

void
doTest(rtObjectRef remoteObj, uint32_t delayTime)
{
  std::this_thread::sleep_for(std::chrono::seconds(delayTime));
  auto const* obj = dynamic_cast<rtRemoteObject const*>(remoteObj.getPtr());
  const char* id = obj->getId().c_str();
  while (true)
  {
    doBasicTest(remoteObj, rtValue(rand()), "int32");
    doBasicTest(remoteObj, rtValue((int8_t) rand()), "int8");
    doBasicTest(remoteObj, rtValue((int64_t) rand()), "int64");
    doBasicTest(remoteObj, rtValue("SampleString"), "string");

    // test method passed two int and return the sum, rtMethod2ArgAndReturn
    rtValue testValue2;
    int32_t destV;
    remoteObj.sendReturns("twoIntNumberSum", rtValue((int32_t) 123), rtValue((int32_t) 12), testValue2);
    testValue2.getInt32(destV);
    rtLogInfo("[%s] test method %s, passed = [%s]", id, "twoIntNumberSum", destV == 123 + 12 ? "true" : "false");

    rtLogInfo("[%s] test done, next test at 10s ...", id);
    sleep(10);
  }
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

  rtObjectRef host_object;
  // find object host_object
  while ((e = rtRemoteLocateObject(env, "host_object", host_object)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  rtObjectRef obj2;
  // find object obj2
  while ((e = rtRemoteLocateObject(env, "obj2", obj2)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  rtObjectRef obj3;
  // find object obj3
  while ((e = rtRemoteLocateObject(env, "obj3", obj3)) != RT_OK)
  {
    rtLogInfo("still looking:%s", rtStrError(e));
  }

  rtObjectRef obj4;
  // find object obj4
  while ((e = rtRemoteLocateObject(env, "obj4", obj4)) != RT_OK)
  {
    rtLogInfo("still looking : %s", rtStrError(e));
  }

  std::thread t1(doTest, host_object, 0);
  std::thread t2(doTest, obj2, 1);
  std::thread t3(doTest, obj3, 2);
  std::thread t4(doTest, obj4, 3);

  while (true)
  {
    sleep(10);
  }
  return 0;
}