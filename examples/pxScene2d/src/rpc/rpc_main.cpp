#include "rtRemoteObject.h"
#include "rtRemoteObjectLocator.h"
#include "rtRpcTransport.h"

#include <unistd.h>
#include <iostream>
#include <map>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>

int main(int argc, char* /*argv*/[])
{
  char const* objectName = "com.xfinity.xsmart.Thermostat/JakesHouse";

  rtRemoteObjectLocator locator;
  locator.open("224.10.10.12", 10004, "eth0");
  locator.start();

  if (argc == 2)
  {
    rtObjectRef thermo;
    rtError err = locator.findObject(objectName, thermo);
    if (err != RT_OK)
      assert(false);

    while (true)
    {
      rtString desc;
      rtError err = thermo.sendReturns<rtString>("description", desc);
      printf("desc: %s\n", desc.cString());
      sleep(1);
    }
  }
  else
  {
    rtObjectRef thermo(new rtObject());
    locator.registerObject(objectName, thermo);

    while (1)
      sleep(10);
  }

  return 0;
}
