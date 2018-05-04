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

// pxOffscreen.cpp

#include "pxCore.h"
#include "pxOffscreen.h"

pxOffscreen::pxOffscreen()
{
}

pxOffscreen::~pxOffscreen()
{
  term();
}

void pxOffscreen::swizzleTo(rtPixelFmt fmt)
{
    pxOffscreenNative::swizzleTo(fmt);
}

pxError pxOffscreen::initWithColor(int32_t width, int32_t height, const pxColor& color)
{
  pxError e = init(width, height);
  fill(color);
  return e;
}

