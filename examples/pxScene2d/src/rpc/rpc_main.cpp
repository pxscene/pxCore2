
#include "rtRpc.h"
#include <rtObject.h>

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
  rtProperty(onTempChanged, onTempChanged, onTempChanged, rtFunctionRef);
  rtMethodNoArgAndNoReturn("hello", hello);
  rtMethod2ArgAndReturn("add", add, int32_t, int32_t, int32_t);

  rtError add(int32_t x, int32_t y, int32_t& result)
  {
    result = x + y;
    return RT_OK;
  }

  float prop1() const
    { return m_prop1; }

  rtError prop1(uint32_t& v) const
    { v = m_prop1; return RT_OK; }

  rtError setProp1(uint32_t v)
    { m_prop1 = v; return RT_OK; }



  rtFunctionRef onTempChanged() const
    { return m_onTempChanged; }

  rtError onTempChanged(rtFunctionRef& ref) const
    { ref = m_onTempChanged; return RT_OK; }

  rtError onTempChanged(rtFunctionRef ref)
    { m_onTempChanged = ref; return RT_OK; }



  rtError hello()
  {
    printf("hello!\n");
    return RT_OK;
  }

private:
  uint32_t m_prop1;
  rtFunctionRef m_onTempChanged;
};


rtDefineObject(rtThermostat, rtObject);
rtDefineProperty(rtThermostat, prop1);
rtDefineProperty(rtThermostat, onTempChanged);
rtDefineMethod(rtThermostat, add);

static rtError my_callback(int argc, rtValue const* argv, rtValue* result, void* /*argp*/)
{
  static int i = 10;

  char buff[256];
  snprintf(buff, sizeof(buff), "The temp has change by: %d", i++);
  if (result)
  {
    *result = rtValue(buff);
  }
  return RT_OK;
}



int main(int argc, char* /*argv*/[])
{
  char const* objectName = "com.xfinity.xsmart.Thermostat/JakesHouse";

  rtError e = rtRpcInit();
  assert(e == RT_OK);

  if (argc == 2)
  {
    rtObjectRef obj;
    e = rtRpcLocateObject(objectName, obj);
    if (e != RT_OK)
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
      e = obj.sendReturns<rtString>("description", desc);
      RT_ASSERT(e);
      #endif

      #if 0 // this works
      e = obj.set("prop1", i);
      if (e != RT_OK)
      {
	printf("failed to set prop\n");
	return -1;
      }

      uint32_t n = obj.get<uint32_t>("prop1");
      printf("fillColor: %d\n", n);
      #endif

      #if 0// this works
      int32_t ret = 0;
      e = obj.sendReturns<int32_t>("add", i, i, ret);
      printf("HERE (%d): %d + %d = %d\n", ret, i, i, ret);
      #endif

      e = obj.set("onTempChanged", new rtFunctionCallback(my_callback));
      sleep(1000);
    }
  }
  else
  {
    rtObjectRef obj(new rtThermostat());
    e = rtRpcRegisterObject(objectName, obj);

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
	if (e == RT_OK)
	{
	  printf("s:%s\n", s.cString());
	}
	else
	{
	  printf("error: %d\n", (int) e);
	}
      }
      else
      {
	printf("onTempChanged is null\n");
      }

      sleep(1);
    }
  }

  return 0;
}
