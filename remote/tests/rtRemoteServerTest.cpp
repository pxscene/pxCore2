/**
 * Copyright (C) 2018 TopCoder Inc., All Rights Reserved.
 */

/**
 * the tests in this file are used to
 * test rtRemoteServer.cpp
 * 
 * Note, almost other unit tests are running in the client mode.
 * But this test will run in the server mode.
 *
 * @author      TCSCODER
 * @version     1.0
 */

#include "rtTestIncludes.h"
#include "rtRemote.h"
#include "rtObject.h"

/**
 * this is a sample remote object,
 * this object contains full type and method.
 */
class HostObject : public rtObject {
  rtDeclareObject(HostObject, rtObject);
public:
  HostObject() : m_count(0) {}

  // define float property
  rtProperty(ffloat, setFloat, getFloat, float);

  // define double property
  rtProperty(ddouble, setDouble, getDouble, double);

  // define bool property
  rtProperty(bbool, setBool, getBool, bool);

  // define int8 property
  rtProperty(int8, setInt8, getInt8, int8_t);

  // define uint8 property
  rtProperty(uint8, setUInt8, getUInt8, uint8_t);

  // define int32 property
  rtProperty(int32, setInt32, getInt32, int32_t);

  // define uint32 property
  rtProperty(uint32, setUInt32, getUInt32, uint32_t);

  // define int64 property
  rtProperty(int64, setInt64, getInt64, int64_t);

  // define uint64 property
  rtProperty(uint64, setUInt64, getUInt64, uint64_t);

  // define string property
  rtProperty(string, setString, getString, rtString);

  // define voidPtr property
  rtProperty(vptr, setVptr, getVptr, voidPtr);

  rtProperty(count, getCount, setCount, int);

  // define rtObjectRef property
  rtProperty(objvar, getObjVar, setObjVar, rtObjectRef);

  // define rtFunctionRef property
  rtProperty(onTick, getOnTickCallback, setOnTickCallback, rtFunctionRef);

  // Define getters and setters

  rtError getCount(int &c) const {
    c = m_count;
    return RT_OK;
  }

  rtError setCount(int c) {
    m_count = c;
    return RT_OK;
  }

  rtError setFloat(float &c) const {
    c = m_ffloat;
    return RT_OK;
  }

  rtError getFloat(float c) {
    m_ffloat = c;
    return RT_OK;
  }

  rtError setDouble(double &c) const {
    c = m_double;
    return RT_OK;
  }

  rtError getDouble(double c) {
    m_double = c;
    return RT_OK;
  }

  rtError setBool(bool &c) const {
    c = m_bbool;
    return RT_OK;
  }

  rtError getBool(bool c) {
    m_bbool = c;
    return RT_OK;
  }

  rtError setInt8(int8_t &c) const {
    c = m_short;
    return RT_OK;
  }

  rtError getInt8(int8_t c) {
    m_short = c;
    return RT_OK;
  }

  rtError setUInt8(uint8_t &c) const {
    c = m_ushort;
    return RT_OK;
  }

  rtError getUInt8(uint8_t c) {
    m_ushort = c;
    return RT_OK;
  }

  rtError setInt32(int32_t &c) const {
    c = m_iint;
    return RT_OK;
  }

  rtError getInt32(int32_t c) {
    m_iint = c;
    return RT_OK;
  }

  rtError setUInt32(uint32_t &c) const {
    c = m_uint32;
    return RT_OK;
  }

  rtError getUInt32(uint32_t c) {
    m_uint32 = c;
    return RT_OK;
  }

  rtError setInt64(int64_t &c) const {
    c = m_int64;
    return RT_OK;
  }

  rtError getInt64(int64_t c) {
    m_int64 = c;
    return RT_OK;
  }

  rtError setUInt64(uint64_t &c) const {
    c = m_uint64;
    return RT_OK;
  }

  rtError getUInt64(uint64_t c) {
    m_uint64 = c;
    return RT_OK;
  }

  rtError setString(rtString &c) const {
    c = m_string;
    return RT_OK;
  }

  rtError getString(rtString c) {
    m_string = c;
    return RT_OK;
  }

  rtError setVptr(voidPtr &c) const {
    c = m_ptr;
    return RT_OK;
  }

  rtError getVptr(voidPtr c) {
    m_ptr = c;
    return RT_OK;
  }

  rtError getObjVar(rtObjectRef &obj) const {
    obj = m_obj;
    return RT_OK;
  }

  rtError setObjVar(const rtObjectRef &obj) {
    m_obj = obj;
    return RT_OK;
  }

  rtError getOnTickCallback(rtFunctionRef &func) const {
    func = m_callback;
    return RT_OK;
  }

  rtError setOnTickCallback(const rtFunctionRef &func) {
    m_callback = func;
    return RT_OK;
  }

  // define methods
  rtMethod1ArgAndReturn("method1AndReturn", method1AndReturn, int32_t, int32_t);

  rtMethodNoArgAndReturn("method0AndReturn10", method0AndReturn10, int32_t);

  rtMethod2ArgAndReturn("twoIntNumberSum", twoIntNumberSum, int32_t, int32_t, int32_t);

  rtMethod2ArgAndReturn("twoFloatNumberSum", twoFloatNumberSum, float, float, float);

  rtMethod1ArgAndNoReturn("method1IntAndNoReturn", method1IntAndNoReturn, int32_t);

  rtMethod2ArgAndNoReturn("method2FunctionAndNoReturn", method2FunctionAndNoReturn, rtFunctionRef, int32_t);

  rtError method1AndReturn(int32_t in, int32_t &o) {
    o = in * 2;
    return RT_OK;
  }

  rtError method0AndReturn10(int32_t &out) {
    out = 10;
    return RT_OK;
  }

  rtError method1IntAndNoReturn(int32_t in) {
    m_method_value = in;
    return RT_OK;
  }

  rtError twoFloatNumberSum(float a, float b, float &out) {
    out = a + b;
    return RT_OK;
  }

  rtError twoIntNumberSum(int32_t a, int32_t b, int32_t &out) {
    out = a + b;
    return RT_OK;
  }

  rtError method2FunctionAndNoReturn(rtFunctionRef callback, int32_t v) {
    if (callback) {
      char buffer[1024];
      sprintf(buffer, "method1FunctionAndReturn10 invoke callback , v = %d", v);
      rtString s(buffer);
      rtError e = callback.send(s);
      if (e != RT_OK) {
        rtLogInfo("send:%s", rtStrError(e));
      }
    }
  }
private:
  // Define members in different types

  int m_count;
  rtFunctionRef m_callback;
  rtObjectRef m_obj;
  float m_ffloat;
  bool m_bbool;

  int8_t m_short;
  uint8_t m_ushort;

  int32_t m_iint;
  uint32_t m_uint32;

  int64_t m_int64;
  uint64_t m_uint64;
  double m_double;

  int32_t m_method_value;

  rtString m_string;
  voidPtr m_ptr;
};


rtDefineObject(HostObject, rtObject);

// ---- export properties
rtDefineProperty(HostObject, ffloat);
rtDefineProperty(HostObject, bbool);
rtDefineProperty(HostObject, int8);
rtDefineProperty(HostObject, uint8);
rtDefineProperty(HostObject, int32);
rtDefineProperty(HostObject, uint32);
rtDefineProperty(HostObject, int64);
rtDefineProperty(HostObject, uint64);
rtDefineProperty(HostObject, ddouble);
rtDefineProperty(HostObject, string);
rtDefineProperty(HostObject, vptr);
rtDefineProperty(HostObject, count);
rtDefineProperty(HostObject, objvar);
rtDefineProperty(HostObject, onTick);

// ---- export methods
rtDefineMethod(HostObject, method1AndReturn);
rtDefineMethod(HostObject, method0AndReturn10);
rtDefineMethod(HostObject, twoIntNumberSum);
rtDefineMethod(HostObject, twoFloatNumberSum);
rtDefineMethod(HostObject, method1IntAndNoReturn);
rtDefineMethod(HostObject, method2FunctionAndNoReturn);

/**
 * rtRpcRemoteServerTest class
 */
class rtRpcRemoteServerTest : public ::testing::Test
{
protected:
  /**
   * init environment and register HostObject
   */
  virtual void SetUp() {
    EXPECT_EQ(RT_OK, rtRemoteInit(env));

    // register the remote object
    EXPECT_EQ(RT_OK, rtRemoteRegisterObject(env, hostObjectName, obj));
  }

  /**
  * clear environment
  */
  virtual void TearDown() {
    rtRemoteUnregisterDisconnectedCallback(env);
    EXPECT_EQ(RT_OK, rtRemoteShutdown());
  }
  
  // create a remote object
  rtObjectRef obj = new HostObject();
  // Host object name
  rtString hostObjectName = "host_object";
  // Environment
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
};

/**
 * Run the server and wait for the client's test
 */
TEST_F(rtRpcRemoteServerTest, serverTest)
{
  // Run the server and listen up to 60 seconds.
  const int MAX_LOOP_COUNT = 60;
  int loopCount = 0;

  while (true) {
    // Listen...
    rtError e = rtRemoteRunUntil(env, 1000);
    EXPECT_TRUE(RT_OK == e || RT_ERROR_QUEUE_EMPTY == e);

    rtLogInfo("Current loop count: %d, max loop count: %d\n", loopCount, MAX_LOOP_COUNT);

    // Exit if it has been listening enough time.
    if (loopCount >= MAX_LOOP_COUNT) {
      rtLogInfo("Exit\n");
      break;
    }
    loopCount++;
  }
}
