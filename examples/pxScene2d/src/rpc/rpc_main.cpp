#include "rtRemoteObject.h"
#include "rtRemoteObjectLocator.h"
#include "rtRpcClient.h"

#include <unistd.h>
#include <iostream>
#include <map>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

#define RT_ASSERT(E) if ((E) != RT_OK) { printf("failed: %d, %d\n", (E), __LINE__); assert(false); }

class rtThermostat : public rtObject
{
public:
  rtDeclareObject(rtThermostat, rtObject);
  rtProperty(prop1, prop1, setProp1, uint32_t);
  rtMethodNoArgAndNoReturn("hello", hello);
  rtMethod2ArgAndReturn("add", add, int32_t, int32_t, int32_t);

  rtError add(int32_t x, int32_t y, int32_t& result)
  {
    result = x + y;
    return RT_OK;
  }

  float prop1()               const { return m_prop1;}
  rtError prop1(uint32_t& v)     const { v = m_prop1; return RT_OK; }
  rtError setProp1(uint32_t v)         { m_prop1 = v; return RT_OK; }

  rtError hello()
  {
    printf("hello!\n");
    return RT_OK;
  }

private:
  uint32_t m_prop1;
};


rtDefineObject(rtThermostat, rtObject);
rtDefineProperty(rtThermostat, prop1);
rtDefineMethod(rtThermostat, add);


int main(int argc, char* /*argv*/[])
{
  char const* objectName = "com.xfinity.xsmart.Thermostat/JakesHouse";

  rtRemoteObjectLocator locator;
  locator.open(); // "224.10.10.12", 10004, "en0");
  locator.start();

  if (argc == 2)
  {
    rtObjectRef obj;
    rtError err = locator.findObject(objectName, obj);
    if (err != RT_OK)
    {
      printf("failed to find object: %s\n", objectName);
      exit(0);
    }

    int i = 1;

    while (true)
    {
      i++;

      rtString desc;
     
      #if 0 // this works
      err = obj.sendReturns<rtString>("description", desc);
      RT_ASSERT(err);
      #endif

      #if 0 // this works
      err = obj.set("prop1", i++);
      RT_ASSERT(err);

      uint32_t n = obj.get<uint32_t>("prop1");
      printf("fillColor: %d\n", n);
      #endif

      #if 0 // this works
      int32_t ret = 0;
      err = obj.sendReturns<int32_t>("add", i, i, ret);
      printf("HERE (%d): %d + %d = %d\n", ret, i, i, ret);
      #endif

      sleep(30); // test keep-alive
      sleep(1);
    }
  }
  else
  {
    rtObjectRef obj(new rtThermostat());
    locator.registerObject(objectName, obj);

    printf("sleeping for 5\n");
    sleep(5);
    locator.removeObject(objectName);
    while (1)
      sleep(10);
  }

  return 0;
}
