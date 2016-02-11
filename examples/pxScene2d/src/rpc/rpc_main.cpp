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


int main(int argc, char* /*argv*/[])
{
  char const* objectName = "com.xfinity.xsmart.Thermostat/JakesHouse";

  rtRemoteObjectLocator locator;
  locator.open("224.10.10.12", 10004, "eth0");
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

    int i = 10;

    while (true)
    {
      rtString desc;
      rtError err = RT_FAIL;
     
      #if 0 // this works
      err = obj.sendReturns<rtString>("description", desc);
      RT_ASSERT(err);
      #endif

      err = obj.set("prop1", i++);
      RT_ASSERT(err);

      uint32_t n = obj.get<uint32_t>("prop1");
      printf("fillColor: %d\n", n);

      sleep(1);
    }
  }
  else
  {
    rtObjectRef obj(new rtThermostat());
    locator.registerObject(objectName, obj);

    while (1)
      sleep(10);
  }

  return 0;
}
