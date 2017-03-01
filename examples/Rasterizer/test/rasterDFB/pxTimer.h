// pxCore CopyRight 2007-2015 John Robinson
// Portable Framebuffer and Windowing Library
// pxTimer.h

#ifndef PX_TIMER_H
#define PX_TIMER_H

#include <inttypes.h>

double pxSeconds();
double pxMilliseconds();
double pxMicroseconds();

void pxSleepMS(uint32_t msToSleep);

#endif //PX_TIMER_H
