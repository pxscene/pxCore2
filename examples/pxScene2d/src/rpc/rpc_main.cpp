#include "rtRemoteObject.h"
#include "rtRemoteObjectLocator.h"

#include <unistd.h>
#include <iostream>
#include <map>

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
    {
      printf("not ok\n");
      return 0;
    }
    else
    {
      printf("ok\n");
    }
    sleep(2);
  }
  else
  {
    rtObjectRef thermo(new rtObject());
    thermo.set("description", "hello from your thermostat");

    locator.registerObject(objectName, thermo);

    while (1)
      sleep(10);
  }

  return 0;
}
