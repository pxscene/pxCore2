/*

pxCore Copyright 2005-2018 John Robinson

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

// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxCore.h"
#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
pxOffscreenNative(): data(NULL) {}
  virtual ~pxOffscreenNative() {}

  pxError term();

  void swizzleTo(rtPixelFmt /*fmt*/) {};

protected:
  char* data;
};

#endif
