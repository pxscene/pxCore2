/*

 pxCore Copyright 2005-2017 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

// pxWindowUtil.h

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
