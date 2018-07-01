/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * this file contains all type method examples
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


void
checkReturnValue(const char* methodName, const rtValue* rpcValue, const rtValue* expectedValue)
{
  bool result = false;
  totalExamplesCount += 1;
  if (expectedValue == nullptr)
  {
    result = true;
    succeedExamplesCount += 1;
  }
  else
  {
    if (*rpcValue == *expectedValue)
    {
      result = true;
      succeedExamplesCount += 1;
    }
  }
  rtLogInfo("test %s, result = [%s]", methodName, result ? "true" : "false");
}

rtError
functionTest(int argc, rtValue const* argv, rtValue* result, void* argp)
{
  // this matches the signature of the server's function
  rtLogInfo("functionTest invoked by remote");
  rtLogInfo("argc:%d", argc);
  for (int i = 0; i < argc; ++i)
  {
    rtLogInfo("argv[%d]:%s", i, argv[i].toString().cString());
  }
  return RT_OK;
}

void
doMethodTest(rtObjectRef remoteObj)
{
  // test no args passed, and return 10 , rtMethod1ArgAndReturn
  rtValue value;
  remoteObj.sendReturns("method0AndReturn10", value);
  checkReturnValue("method0AndReturn10", &value, new rtValue((int32_t) 10));

  // test method passed two int and return the sum, rtMethod2ArgAndReturn
  rtValue testValue2;
  remoteObj.sendReturns("twoIntNumberSum", rtValue((int32_t) 123), rtValue((int32_t) 12), testValue2);
  checkReturnValue("twoIntNumberSum", &testValue2, new rtValue((int32_t) (123 + 12)));

  // test method passed two float and return the sum, rtMethod2ArgAndReturn
  rtValue testValue3;
  remoteObj.sendReturns("twoFloatNumberSum", rtValue(123.3f), rtValue(12.3f), testValue3);
  checkReturnValue("twoFloatNumberSum", &testValue3, new rtValue(123.3f + 12.3f));

  // test method that passed 1 arg and no return, rtMethod1ArgAndNoReturn
  remoteObj.send("method1IntAndNoReturn", rtValue(11));
  checkReturnValue("method1IntAndNoReturn", nullptr, nullptr);

  // test method that passed RtFunction and invoke this function , rtMethod2ArgAndNoReturn
  rtFunctionRef callback(new rtFunctionCallback(functionTest));
  remoteObj.send("method2FunctionAndNoReturn", rtValue(callback), rtValue(10));
  checkReturnValue("method2FunctionAndNoReturn", nullptr, nullptr);

  rtLogInfo("========= %d of %d example succeed, %d failed.", succeedExamplesCount,
            totalExamplesCount, totalExamplesCount - succeedExamplesCount);
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
    doMethodTest(obj);
    rtLogInfo("test completed, next test will at %ds ...", 10);
    sleep(10);
  }
  return 0;
}