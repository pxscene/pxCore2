#include "rtTest.h"
#include "testString.h"
#include "testZip.h"

int main()
{
  testString();
  testZip();
  rtDumpTestCounters();
}
