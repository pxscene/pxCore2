#include "rtRemoteObject.h"
#include "rtRemoteObjectLocator.h"

#include <unistd.h>
#include <iostream>

int main(int argc, char* /*argv*/[])
{
  int ret = 0;
  char const* objectName = "com.xfinity.xsmart.Thermostat/JakesHouse";

  rtRemoteObjectLocator locator;
  ret = locator.open("224.10.10.12", 10004, "eth0");

  if (ret != 0)
    perror("failed to open");

  if (argc == 2)
  {
    locator.startListener(false);

    rtObjectRef thermo = locator.findObject(objectName);
    if (!thermo)
    {
      std::cout << "why didn't it work!" << std::endl;
    }
  }
  else
  {
    locator.startListener(true);

    rtObjectRef thermo(new rtObject());
    thermo.set("description", "hello from your thermostat");

    locator.registerObject(objectName, thermo);

    while (1)
      sleep(10);
  }

  return 0;
}
