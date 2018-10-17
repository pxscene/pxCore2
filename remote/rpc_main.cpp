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
#include "rtRemoteConfig.h"
#include <rtObject.h>
#include <functional>

#include <unistd.h>
#include <iostream>
#include <map>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

rtRemoteEnvironment* env = nullptr;

class rtLcd : public rtObject
{

  rtDeclareObject(rtLcd, rtObject);
  rtProperty(text, text, setText, rtString);
  rtProperty(height, height, setHeight, uint32_t);
  rtProperty(width, width, setWidth, uint32_t);
  rtProperty(connections, connections, setConnections, rtObjectRef);

public:
  rtLcd()
    : rtObject()
    , m_connections(new rtArrayObject())
  {
  }

  rtString text() const { return m_text; }
  rtError  text(rtString& s) const { s = m_text; return RT_OK; }
  rtError  setText(rtString const& s) { m_text = s; return RT_OK; }

  uint32_t height() const { return m_height; }
  rtError  height(uint32_t& s) const { s = m_height; return RT_OK; }
  rtError  setHeight(uint32_t s) { m_height = s; return RT_OK; }

  uint32_t width() const { return m_width; }
  rtError  width(uint32_t& s) const { s = m_width; return RT_OK; }
  rtError  setWidth(uint32_t s) { m_width = s; return RT_OK; }

  rtObjectRef   connections() const { return m_connections; }
  rtError       connections(rtObjectRef& arr) const { arr = m_connections; return RT_OK; }
  rtError       setConnections(rtObjectRef const& arr) { m_connections = arr; return RT_OK; }

private:
  rtString      m_text;
  uint32_t      m_width;
  uint32_t      m_height;
  rtObjectRef   m_connections;
};

class rtEcho : public rtObject
{
  rtDeclareObject(rtEcho, rtEcho);
  rtProperty(message, getMessage, setMessage, rtString);
  rtProperty(onMessageChanged, getOnMessageChanged, setOnMessageChanged, rtFunctionRef);

public:
  rtError getMessage(rtString& s) const
    { s = m_msg; return RT_OK; }

  rtError setMessage(rtString const& s)
  {
    rtError e = RT_OK;
    m_msg = s;
    if (m_func)
    {
      e = m_func.send(s);
      if (e != RT_OK)
        rtLogError("failed to notify of message changed. %s", rtStrError(e));
    }
    return RT_OK;
  }

  rtError getOnMessageChanged(rtFunctionRef& func) const
    { func = m_func; return RT_OK; }

  rtError setOnMessageChanged(rtFunctionRef const& func)
    { m_func = func; return RT_OK; }

  static rtError handleMessageChanged(int /*argc*/, rtValue const* argv, rtValue* /*result*/, void* /*argp*/)
  {
    rtString s = argv[0].toString();
    rtLogInfo("message has changed: %s", s.cString());
    return RT_OK;
  }

private:
  rtString m_msg;
  rtFunctionRef m_func;
};


class rtThermostat : public rtObject
{
public:
  rtDeclareObject(rtThermostat, rtObject);
  rtProperty(lcd, lcd, setLcd, rtObjectRef);
  rtProperty(onTempChanged, onTempChanged, setOnTempChanged, rtFunctionRef);

  // rtMethodNoArgAndNoReturn("hello", hello);
  rtMethod2ArgAndReturn("add", add, int32_t, int32_t, int32_t);

  rtError add(int32_t x, int32_t y, int32_t& result)
  {
    result = x + y;
    return RT_OK;
  }

  rtObjectRef lcd() const { return m_lcd; }
  rtError     lcd(rtObjectRef& ref) const { ref = m_lcd; return RT_OK; }
  rtError     setLcd(rtObjectRef lcd) { m_lcd = lcd; return RT_OK; }

  rtFunctionRef onTempChanged() const { return m_onTempChanged; } 
  rtError onTempChanged(rtFunctionRef& ref) const { ref = m_onTempChanged; return RT_OK; } 
  rtError setOnTempChanged(rtFunctionRef ref) { m_onTempChanged = ref; return RT_OK; } 

  rtError hello()
  {
    rtLogInfo("hello");
    return RT_OK;
  }

private:
  rtObjectRef 		m_lcd;
  rtFunctionRef 	m_onTempChanged;
};

rtDefineObject(rtLcd, 	rtObject);
rtDefineProperty(rtLcd, text);
rtDefineProperty(rtLcd, width);
rtDefineProperty(rtLcd, height);
rtDefineProperty(rtLcd, connections);

rtDefineObject(rtEcho, rtObject);
rtDefineProperty(rtEcho, message);
rtDefineProperty(rtEcho, onMessageChanged);

rtDefineObject(rtThermostat, rtObject);
rtDefineProperty(rtThermostat, lcd);
rtDefineProperty(rtThermostat, onTempChanged);
rtDefineMethod(rtThermostat, add);

static rtError my_callback(int /*argc*/, rtValue const* /*argv*/, rtValue* result, void* /*argp*/)
{
  static int i = 10;

  char buff[256];
  snprintf(buff, sizeof(buff), "Hello World again (%d) from:%d", i++, (int) getpid());
  if (result)
  {
    *result = rtValue(buff);
  }
  return RT_OK;
}


static char const* objectName = "com.xfinity.xsmart.Thermostat.JakesHouse";

void Test_Echo_Client()
{
  rtObjectRef obj;
  rtError e = rtRemoteLocateObject(env, "echo.object", obj);
  RT_ASSERT(e == RT_OK);

  e = obj.set("onMessageChanged", new rtFunctionCallback(rtEcho::handleMessageChanged));
  if (e != RT_OK)
    rtLogError("failed to set message handler: %s", rtStrError(e));

  int i = 0;
  char buff[256];

  while (true)
  {
    memset(buff, 0, sizeof(buff));
    sprintf(buff, "hello:%06d", i++);
    e = obj.set("message", buff);
    rtLogInfo("set:%s", rtStrError(e));

    e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }
}

void Test_Echo_Server()
{
  rtObjectRef obj(new rtEcho());
  rtError e = rtRemoteRegisterObject(env, "echo.object", obj);
  RT_ASSERT(e == RT_OK);
  while (true)
  {
    e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }
}

void Test_SetProperty_Basic_Client()
{
  while (true)
  {
    rtObjectRef objectRef;

    rtLogInfo("looking for test.lcd");
    rtError e = RT_OK;

    while ((e = rtRemoteLocateObject(env, "test.lcd", objectRef)) != RT_OK)
    {
      rtLogInfo("failed to find test.lcd:%s\n", rtStrError(e));
    }

    RT_ASSERT(e == RT_OK);

    {
      rtLogInfo("getting connections");
      rtObjectRef cons = objectRef.get<rtObjectRef>("connections");
      if (cons)
      {
        rtLogInfo("setting 1");
        cons.set(1, "hello");

        rtLogInfo("setting 2");
        cons.set(2, "world");
      }
      else
      {
        rtLogInfo("remote object doesn't have property");
      }
    }

    {
      rtLogInfo("getting connections");
      rtObjectRef cons = objectRef.get<rtObjectRef>("connections");
      if (cons)
      {
        rtLogInfo("getting 1");
        rtString s1 = cons.get<rtString>(1);
        rtLogInfo("cons[1]:%s", s1.cString());

        rtLogInfo("getting 2");
        rtString s2 = cons.get<rtString>(2);
        rtLogInfo("cons[2]:%s", s2.cString());
      }
      else
      {
        rtLogInfo("remote object doesn't have property");
      }
    }

    #if 0
    for (int i = 0, j = 10; i < 5; ++i)
    {
      e = objectRef.set("height", j);
      // RT_ASSERT(e == RT_OK);

      //  if (i % 1000 == 0)
      rtLogInfo("set:%d", j);

      uint32_t n = objectRef.get<uint32_t>("height");

      //   if (i % 1000 == 0)
      rtLogInfo("get:%d", n);

      // RT_ASSERT(n == static_cast<uint32_t>(j));

      j++;

      rtLogInfo("sleeping for 1");
      sleep(1);
    }
    #endif

    rtLogInfo("sleeping for one second");
    sleep(1);
  }
}

void Test_SetProperty_Basic_Server()
{
  rtObjectRef obj(new rtLcd());
  rtError e = rtRemoteRegisterObject(env, "test.lcd", obj);
  RT_ASSERT(e == RT_OK);

  // int n = 0;
  while (true)
  {
    rtError e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
#if 0
    if (n++ == 4)
    {
      rtLogInfo("removing registration");
      rtError e = rtRemoteUnregisterObject(env, "test.lcd");
      RT_ASSERT(e == RT_OK);
    }
#endif
  }
}

void Test_FunctionReferences_Client()
{
  rtObjectRef objectRef;
  rtError e = rtRemoteLocateObject(env, objectName, objectRef);
  RT_ASSERT(e == RT_OK);

  e = objectRef.set("onTempChanged", new rtFunctionCallback(my_callback));
  RT_ASSERT(e == RT_OK);

  while (true)
  {
    rtError e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }

  while (1) sleep(1);
}

void Test_FunctionReferences_Server()
{
  rtObjectRef obj(new rtThermostat());
  rtError e = rtRemoteRegisterObject(env, objectName, obj);
  RT_ASSERT(e == RT_OK);

  // locator.removeObject(objectName);
  int temp = 50;

  while (1)
  {
    rtFunctionRef ref;
    obj.get("onTempChanged", ref);

    if (ref)
    {
      rtValue arg1(temp++);

      rtString s;
      rtError e = ref.sendReturns<rtString>(arg1, s);
      if (e != RT_OK)
      {
        rtLogError("failed to send function: %s", rtStrError(e));
        return;
      }

      rtLogInfo("s: %s", s.cString());
    }
    else
    {
      rtLogInfo("onTempChanged is null");
    }

    rtError e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }
}

void
Test_MethodCall_Client()
{
  rtObjectRef objectRef;
  rtError e = rtRemoteLocateObject(env, objectName, objectRef);
  RT_ASSERT(e == RT_OK);

  int i = 1;
  while (true)
  {
    rtValue val(i);
    rtValue sum(0);
    rtError e = objectRef.sendReturns("add", val, val, sum);
    rtLogInfo("%d + %d == %d", i, i, sum.toInt32());
    RT_ASSERT(e == RT_OK);
    sleep(1);
    i += 1;
  }
}

void
Test_MethodCall_Server()
{
  rtObjectRef obj(new rtThermostat());
  rtError e = rtRemoteRegisterObject(env, objectName, obj);
  RT_ASSERT(e == RT_OK);
  while (true)
  {
    rtError e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }
}

void
Test_SetProperty_Object_Client()
{
  rtObjectRef objectRef;
  rtError e = rtRemoteLocateObject(env, objectName, objectRef);
  RT_ASSERT(e == RT_OK);

  int n = 10;
  char buff[256];

  while (true)
  {
    rtObjectRef lcd;
    e = objectRef.get("lcd", lcd);

    rtString s = lcd.get<rtString>("text");
    rtLogInfo("text: %s", s.cString());

    snprintf(buff, sizeof(buff), "hello from me:%d", n++);
    s = buff;
    lcd.set("text", s);

    RT_ASSERT(e == RT_OK);

    sleep(1);
  }
}

void
Test_SetProperty_Object_Server()
{
  rtObjectRef obj(new rtThermostat());

  rtObjectRef lcd(new rtLcd());
  lcd.set("text", "This is the lcd");
  lcd.set("width", 10);
  lcd.set("height", 11);

  obj.set("lcd", lcd);

  rtError e = rtRemoteRegisterObject(env, objectName, obj);
  RT_ASSERT(e == RT_OK);
  while (true)
  {
    rtError e = rtRemoteRunUntil(env, 1000, false);
    rtLogInfo("rtRemoteRun:%s", rtStrError(e));
  }
}

struct TestCase
{
  std::function<void ()> client;
  std::function<void ()> server;
};

std::map< int, TestCase > testCases;


int main(int argc, char* /*argv*/[])
{
  rtObjectRef list(new rtArrayObject());
  for (int i = 0; i < 10; ++i)
  {
    rtValue v("hello");
    rtError e = list.set(i, v);
    RT_ASSERT(e == RT_OK);
  }

  int n = list.get<int>("length");
  for (int i = 0; i < n + 2; ++i)
  {
    rtString s = list.get<rtString>(i);
    rtLogInfo("%d = '%s'.", i, s.cString());
  }

  env = rtEnvironmentGetGlobal();

  rtError e = rtRemoteInit(env);
  RT_ASSERT(e == RT_OK);

  if (argc == 1)
  {
    rtLogInfo("starting client");

    // Test_Echo_Client();
    // Test_FunctionReferences_Client();
    // Test_MethodCall_Client();
    // Test_SetProperty_Object_Client();
    Test_SetProperty_Basic_Client();
  }
  else
  {
    rtLogInfo("starting server");
    Test_SetProperty_Basic_Server();
    // Test_Echo_Server();
    // Test_FunctionReferences_Server();
    // Test_SetProperty_Object_Server();
    // Test_MethodCall_Server();
  }

  rtRemoteShutdown(env);
  return 0;
}
