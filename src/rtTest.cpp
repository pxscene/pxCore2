// rtCore CopyRight 2005-2015 John Robinson
// rtTest.cpp

#include "rtTest.h"
#include "stdio.h"

long totalTestsRun = 0;
long totalTestsFailed = 0;

void rtResetTestCounters() 
{
    totalTestsRun = 0;
    totalTestsFailed = 0;
}

void rtDumpTestCounters() 
{
    printf("%ld Tests Run\n", totalTestsRun);
    printf("%ld Tests Failed\n", totalTestsFailed);
}

