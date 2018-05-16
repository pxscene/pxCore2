/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the test cases in this file are used to test function
 * of rtRemote.cpp
 *
 * @author      TCSCODER
 * @version     1.0
 */
#include "rtRemoteConfig.h"
#include "rtRemoteEnvironment.h"
#include "rtTestIncludes.h"
/**
 * used to test Object
 */
class rtRemoteTestObject : public rtObject
{
  rtDeclareObject(rtRemoteTestObject, rtObject);
public:
  rtProperty(count, getCount, setCount, int);
  rtError getCount(int& n) const { n = m_count; return RT_OK; }
  rtError setCount(int  n) { m_count = n; return RT_OK; }
private:
  int m_count;
};
rtDefineObject(rtRemoteTestObject, rtObject);
rtDefineProperty(rtRemoteTestObject, count);

/**
 * rtRemoteTest class
 */
class rtRemoteTest : public ::testing::Test
{
protected:
 virtual void SetUp() {
 }
 virtual void TearDown() {
 }
};

/**
 * test function rtRemoteInit()
 */
TEST(rtRemoteTest, rtRemoteInitNoParamTest)
{
  EXPECT_EQ(RT_OK, rtRemoteInit());
}

/**
 * test function rtRemoteInit(rtRemoteEnvironment* env)
 */
TEST(rtRemoteTest, rtRemoteInitTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
}

/**
 * test function rtRemoteShutdown()
 */
TEST(rtRemoteTest, rtRemoteShutdownNoParamTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  EXPECT_EQ(RT_OK, rtRemoteShutdown());
}

/**
 * test function rtRemoteShutdown, immediate shut down
 */
TEST(rtRemoteTest, rtRemoteShutdownImmediateTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env, true));
}


/**
 * test function rtRemoteShutdown, not immediate shut down
 */
TEST(rtRemoteTest, rtRemoteShutdownNotImmediateTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env, false));
}

/**
 * test function rtRemoteRegisterObject
 */
TEST(rtRemoteTest, rtRemoteRegisterObjectTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  rtObjectRef serverObj(new rtRemoteTestObject());
  rtObjectRef serverObj2(new rtRemoteTestObject());
  rtObjectRef objectRef;
  char const* objectName = "rtRemoteTest";
  // test function rtRemoteRegisterObject(rtRemoteEnvironment* env, char const* id, rtObjectRef const& obj)
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(env, objectName, serverObj));
  // test function rtRemoteRegisterObject(char const* id, rtObjectRef const& obj)
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(objectName, serverObj2));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test invalid args to rtRemoteRegisterObject(rtRemoteEnvironment* env,
 * char const* id, rtObjectRef const& obj)
 */
TEST(rtRemoteTest, rtRemoteRegisterObjectInvalidParamsTest)
{
  rtObjectRef obj(new rtRemoteTestObject());
  char const *id =  "rtRemoteTest";
  rtRemoteEnvironment *env = NULL;
  // test invalid env
  EXPECT_EQ(RT_FAIL, rtRemoteRegisterObject(env, id, obj));
  env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  // test invalid id
  id = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteRegisterObject(env, id, obj));
  id = "rtRemoteTest";
  // test invlaid obj
  obj = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteRegisterObject(env, id, obj));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test function rtRemoteUnregisterObject
 */
TEST(rtRemoteTest, rtRemoteUnregisterObjectTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  rtObjectRef serverObj(new rtRemoteTestObject());
  rtObjectRef objectRef;
  char const* objectName = "rtRemoteTest";
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(env, objectName, serverObj));
  // test rtRemoteUnregisterObject(rtRemoteEnvironment* env, char const* id)
  EXPECT_EQ(RT_OK, rtRemoteUnregisterObject(env, objectName));
  EXPECT_EQ(RT_OK, rtRemoteRegisterObject(env, objectName, serverObj));
  // test rtRemoteUnregisterObject(char const* id)
  EXPECT_EQ(RT_OK, rtRemoteUnregisterObject(objectName));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test invalid args to rtRemoteRegisterObject
 */
TEST(rtRemoteTest, rtRemoteUnRegisterObjectInvalidParamsTest)
{
  char const *id =  "rtRemoteTest";
  // test invalid env
  rtRemoteEnvironment *env = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteUnregisterObject(env, id));
  env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  // test invalid id
  id = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteUnregisterObject(env, id));
  id = "rtRemoteTest";
  // test use not register id to unregister object, it will return RT_ERROR_OBJECT_NOT_FOUND
  EXPECT_EQ(RT_ERROR_OBJECT_NOT_FOUND, rtRemoteUnregisterObject(env, id));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test invalid args to rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id,
 * rtObjectRef& obj, int timeout, remoteDisconnectedCallback cb, void *cbdata)
 */
TEST(rtRemoteTest, rtRemoteLocateObjectInvalidParamsTest)
{
  rtObjectRef objectRef;
  char const* id = "host_object";

  // test invalid env
  rtRemoteEnvironment *env = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteLocateObject(env, id, objectRef));
  env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));

  // test invalid id
  id = NULL;
  EXPECT_EQ(RT_ERROR_INVALID_ARG, rtRemoteLocateObject(env, id, objectRef));

  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test function rtRemoteRunUntil
 */
TEST(rtRemoteTest, rtRemoteRunUntilTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  const char * remoteObjectName = "host_object";
  rtObjectRef objectRef;
  int32_t value = 168;
  int timeout = 5000;
  rtValue newVal;
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  EXPECT_EQ(RT_OK, rtRemoteLocateObject(env, remoteObjectName, objectRef));
  EXPECT_EQ(RT_OK, objectRef.set("count", value));
  // test wait 5000ms
  rtRemoteRunUntil(env, timeout);
  EXPECT_EQ(RT_OK, objectRef.get("count", newVal));
  EXPECT_EQ(value, newVal.toInt32());
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}

/**
 * test rtEnvironmentFromFile
 */
TEST(rtRemoteEnvTest, rtEnvironmentFromFileTest)
{
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  const char *fileName = "rtremote.conf";
  rtRemoteEnvironment *newEnv = rtEnvironmentFromFile(fileName);
  const rtRemoteConfig *config = env->Config;
  const rtRemoteConfig *configNew = newEnv->Config;
  EXPECT_EQ(config->resolver_multicast_address6(), configNew->resolver_multicast_address6());
}

/**
 * test rtRemoteRegisterQueueReadyHandler
 */
TEST(rtRemoteEnvTest, rtRemoteRegisterQueueReadyHandlerTest)
{
  rtRemoteQueueReady handler = NULL;
  void* argp = NULL;
  rtRemoteEnvironment *env = rtEnvironmentGetGlobal();
  EXPECT_EQ(RT_OK, rtRemoteInit(env));
  EXPECT_EQ(RT_OK, rtRemoteRegisterQueueReadyHandler(env, handler, argp));
  EXPECT_EQ(RT_OK, rtRemoteShutdown(env));
}
