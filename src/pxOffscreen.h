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

// pxColors.h

#ifndef PXOFFSCREEN_H
#define PXOFFSCREEN_H

#include "pxCore.h"
#include "pxBuffer.h"

// Class used to create and manage offscreen pixmaps
// This class subclasses pxBuffer (pxBuffer.h)
// Please refer to pxBuffer.h for additional methods
class pxOffscreen: public pxOffscreenNative
{
public:
  
  pxOffscreen();
  virtual ~pxOffscreen();

  pxOffscreen(const pxOffscreen& o)
  {
    init(o.width(),o.height());
    o.blit(*this);
  }
  
  pxOffscreen& operator=(const pxOffscreen& rhs)
  {
    init(rhs.width(),rhs.height());
    rhs.blit(*this);
    return *this;
  }

  // This will initialize the offscreen for the given height and width 
  // but will not clear it.
  pxError init(int width, int height);
    
  // This will initialize the offscreen for the given height and width and
  // will clear it with the provided color.
  pxError initWithColor(int width, int height, const pxColor& color);
  
  pxError term();
  
  void swizzleTo(rtPixelFmt fmt);

};

#endif // PXOFFSCREEN_H
