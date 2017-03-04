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

// pxColor.h

#ifndef PX_COLOR_H
#define PX_COLOR_H

#include "pxPixel.h"

const pxColor pxClear(0, 0, 0, 0); // RGBA

const pxColor pxWhite(255, 255, 255, 255);
const pxColor pxBlack(0, 0, 0, 255);
const pxColor pxRed(255, 0, 0, 255);
const pxColor pxGreen(0, 255, 0, 255);
const pxColor pxBlue(0, 0, 255, 255);
const pxColor pxGray(128, 128, 128, 255);

#endif
