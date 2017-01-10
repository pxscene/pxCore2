#ifndef PX_WINDOW_UTIL_H
#define PX_WINDOW_UTIL_H

#include <inttypes.h>

uint32_t keycodeFromNative(uint32_t nativeKeycode);

// WARNING this utility function should be avoided.
// This function provides a very very simple way to map from keycodes
// to ascii values and assumes a US keyboard among other things.
// A better platform aligned mechanism should almost always be used.
uint32_t keycodeToAscii(uint32_t keycode, uint32_t flags);

#endif //PX_WINDOW_UTIL_H
