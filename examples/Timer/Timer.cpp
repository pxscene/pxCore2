// Timer Example CopyRight 2007-2009 John Robinson
// Simple example showing how to measure elapsed time

#include "pxCore.h"
#include "pxTimer.h"

#include <stdio.h>

int pxMain()
{
   double start, end;
   
   printf("Timing back to back in microseconds...\n");
   start = pxMicroseconds();
   end = pxMicroseconds();

   printf("\telapsed %gus\n\n", end-start);

   printf("Timing pxSleepMS(5) in microseconds...\n");
   start = pxMicroseconds();
   pxSleepMS(5);  // Sleep for approximately 5ms
   end = pxMicroseconds();

   printf("\telapsed %gus\n\n", end-start);

   printf("Timing pxSleepMS(5) in milliseconds...\n");
   start = pxMilliseconds();
   pxSleepMS(5);  // Sleep for approximately 1ms
   end = pxMilliseconds();

   printf("\telapsed %gms\n\n", end-start);

   printf("Timing pxSleep(5000) in seconds...\n");
   start = pxSeconds();
   pxSleepMS(5000);  // Sleep for approximately 5 seconds
   end = pxSeconds();

   printf("\telapsed %gs\n\n", end-start);

   return 0;
}


