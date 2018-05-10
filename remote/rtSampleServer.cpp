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

#include "rtRemote.h"
#include "rtObject.h"

/**
 * this is a sample remote object,
 * this object contains full type and method.
 */
class HostObject : public rtObject
{
  rtDeclareObject(HostObject, rtObject);
public:
  HostObject() : m_count(0), m_arr(new rtArrayObject)
  {
    auto arr = (rtArrayObject*) m_arr.getPtr();
    arr->pushBack(rtValue(10));
    arr->pushBack(rtValue(12.3f));
    arr->pushBack(rtValue("Hello world"));
  }

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

  // define array
  rtProperty(arr, getArr, setArr, rtObjectRef);

  rtError getCount(int& c) const
  {
    c = m_count;
    return RT_OK;
  }

  rtError setCount(int c)
  {
    m_count = c;
    return RT_OK;
  }

  rtError setFloat(float& c) const
  {
    c = m_ffloat;
    return RT_OK;
  }

  rtError getFloat(float c)
  {
    m_ffloat = c;
    return RT_OK;
  }

  rtError setDouble(double& c) const
  {
    c = m_double;
    return RT_OK;
  }

  rtError getDouble(double c)
  {
    m_double = c;
    return RT_OK;
  }

  rtError setBool(bool& c) const
  {
    c = m_bbool;
    return RT_OK;
  }

  rtError getBool(bool c)
  {
    m_bbool = c;
    return RT_OK;
  }

  rtError setInt8(int8_t& c) const
  {
    c = m_short;
    return RT_OK;
  }

  rtError getInt8(int8_t c)
  {
    m_short = c;
    return RT_OK;
  }

  rtError setUInt8(uint8_t& c) const
  {
    c = m_ushort;
    return RT_OK;
  }

  rtError getUInt8(uint8_t c)
  {
    m_ushort = c;
    return RT_OK;
  }

  rtError setInt32(int32_t& c) const
  {
    c = m_iint;
    return RT_OK;
  }

  rtError getInt32(int32_t c)
  {
    m_iint = c;
    return RT_OK;
  }

  rtError setUInt32(uint32_t& c) const
  {
    c = m_uint32;
    return RT_OK;
  }

  rtError getUInt32(uint32_t c)
  {
    m_uint32 = c;
    return RT_OK;
  }

  rtError setInt64(int64_t& c) const
  {
    c = m_int64;
    return RT_OK;
  }

  rtError getInt64(int64_t c)
  {
    m_int64 = c;
    return RT_OK;
  }

  rtError setUInt64(uint64_t& c) const
  {
    c = m_uint64;
    return RT_OK;
  }

  rtError getUInt64(uint64_t c)
  {
    m_uint64 = c;
    return RT_OK;
  }

  rtError setString(rtString& c) const
  {
    c = m_string;
    return RT_OK;
  }

  rtError getString(rtString c)
  {
    m_string = c;
    return RT_OK;
  }

  rtError setVptr(voidPtr& c) const
  {
    c = m_ptr;
    return RT_OK;
  }

  rtError getVptr(voidPtr c)
  {
    m_ptr = c;
    return RT_OK;
  }

  rtError getObjVar(rtObjectRef& obj) const
  {
    obj = m_obj;
    return RT_OK;
  }

  rtError setObjVar(const rtObjectRef& obj)
  {
    m_obj = obj;
    return RT_OK;
  }

  rtError getOnTickCallback(rtFunctionRef& func) const
  {
    func = m_callback;
    return RT_OK;
  }

  rtError setOnTickCallback(const rtFunctionRef& func)
  {
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

  rtError method1AndReturn(int32_t in, int32_t& o)
  {
    o = in * 2;
    return RT_OK;
  }

  rtError method0AndReturn10(int32_t& out)
  {
    out = 10;
    return RT_OK;
  }

  rtError method1IntAndNoReturn(int32_t in)
  {
    m_method_value = in;
    return RT_OK;
  }

  rtError twoFloatNumberSum(float a, float b, float& out)
  {
    out = a + b;
    return RT_OK;
  }

  rtError twoIntNumberSum(int32_t a, int32_t b, int32_t& out)
  {
    out = a + b;
    return RT_OK;
  }

  rtError method2FunctionAndNoReturn(rtFunctionRef callback, int32_t v)
  {
    if (callback)
    {
      char buffer[1024];
      sprintf(buffer, "method1FunctionAndReturn10 invoke callback , v = %d", v);
      rtString s(buffer);
      rtError e = callback.send(s);
      if (e != RT_OK)
      {
        rtLogInfo("send:%s", rtStrError(e));
      }
    }
  }

  rtError getArr(rtObjectRef& obj) const
  {
    obj = m_arr;
    return RT_OK;
  }

  rtError setArr(const rtObjectRef& obj)
  {
    m_arr = obj;
    return RT_OK;
  }
private:
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

  rtObjectRef m_arr;
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
rtDefineProperty(HostObject, arr);

// ---- export methods
rtDefineMethod(HostObject, method1AndReturn);
rtDefineMethod(HostObject, method0AndReturn10);
rtDefineMethod(HostObject, twoIntNumberSum);
rtDefineMethod(HostObject, twoFloatNumberSum);
rtDefineMethod(HostObject, method1IntAndNoReturn);
rtDefineMethod(HostObject, method2FunctionAndNoReturn);


/*
 server-side:
 1. create server host classes
 2. register them with rtRemoteRegisterObject() with its name as argument
 3. invoke rtRemoteRunUntil() or rtRemoteRun()
 */
int
main(int /*argc*/, char* /*argv*/ [])
{
  rtLogSetLevel(RT_LOG_INFO);

  rtError e;
  rtRemoteEnvironment* env = rtEnvironmentGetGlobal();
  e = rtRemoteInit(env);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteInit:%s", rtStrError(e));
    exit(1);
  }

  // create 4 objects
  rtObjectRef obj(new HostObject());
  rtObjectRef obj2(new HostObject());
  rtObjectRef obj3(new HostObject());
  rtObjectRef obj4(new HostObject());

  // register 4 objects
  e = rtRemoteRegisterObject(env, "host_object", obj);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject host_object:%s", rtStrError(e));
    exit(2);
  }

  e = rtRemoteRegisterObject(env, "obj2", obj2);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject obj2:%s", rtStrError(e));
    exit(2);
  }

  e = rtRemoteRegisterObject(env, "obj3", obj3);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject obj3:%s", rtStrError(e));
    exit(2);
  }
  e = rtRemoteRegisterObject(env, "obj4", obj4);
  if (e != RT_OK)
  {
    rtLogError("rtRemoteRegisterObject obj4:%s", rtStrError(e));
    exit(2);
  }

  while (true)
  {
    rtRemoteRunUntil(env, 1000);
  }
  return 0;
}
