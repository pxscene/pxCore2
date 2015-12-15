#include "rtTest.h"
#include "testString.h"
#include "testValue.h"
#include "testObject.h"
#include "testZip.h"

int main()
{
  testString();
  testValue();
  testObject();
  testZip();
  rtDumpTestCounters();
}
