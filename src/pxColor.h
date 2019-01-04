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

// pxColor.h

#ifndef PX_COLOR_H
#define PX_COLOR_H


#include "rtString.h"
#include "rtError.h"
#include "pxPixel.h"

#include "pxColorNames.h"

const pxColor pxClear(0, 0, 0, 0); // RGBA

const pxColor pxWhite(255, 255, 255, 255);
const pxColor pxBlack(0, 0, 0, 255);
const pxColor pxRed(255, 0, 0, 255);
const pxColor pxGreen(0, 255, 0, 255);
const pxColor pxBlue(0, 0, 255, 255);
const pxColor pxGray(128, 128, 128, 255);


static rtError web2rgb(rtString &input, uint8_t &r, uint8_t &g, uint8_t &b)
{
  rtError retVal = RT_OK;

  char clr6[7];
  unsigned int rr, gg, bb;
  
  if (input.length() == 0)
  {
    return RT_FAIL; // FAIL
  }

  char* clr = (char *) input.cString();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  // Try Named Colors
  //
  if(clr[0] != '#')
  {
    input.toLower(); // in-place;

    uint32_t c = colorNames[ input ];
//    uint32_t c = nsvg__parseColor( input );
    
#ifdef PX_LITTLEENDIAN_PIXELS
    
    rr = (uint8_t)((c >> 24) & 0xff);  // R
    gg = (uint8_t)((c >> 16) & 0xff);  // G
    bb = (uint8_t)((c >>  8) & 0xff);  // B
    
#else
    
    bb = (uint8_t)((c >> 16) & 0xff);  // B
    gg = (uint8_t)((c >>  8) & 0xff);  // G
    rr = (uint8_t)((c >>  0) & 0xff);  // R
    
#endif
    
    r = rr;   g = gg;  b = bb;

    return RT_OK;
  }
  
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  //
  //  Set by WebColor   "#rgb" or "#RRGGBB"  strings
  //
  if(clr[0] != '#')
  {
    // rtLogDebug("web2rgb: %s  <<< Unknown Input \n", input);
    return RT_FAIL;
  }

  clr++; // skip '#'
  
  // Promote RGB to RRGGBB format
  if(strlen(clr) == 3)
  {
    clr6[0] = clr[0];  clr6[1] = clr[0];  // 01
    clr6[2] = clr[1];  clr6[3] = clr[1];  // 23
    clr6[4] = clr[2];  clr6[5] = clr[2];  // 45
    clr6[6] = '\0';
    
    clr = clr6; // point here now !
  }
  
  // Read "RRGGBB" formatted string
  if( sscanf(clr, "%02x%02x%02x", &rr, &gg, &bb) == 3)
  {
    r = rr;   g = gg;  b = bb;
  }
  else
  {
    retVal = RT_FAIL;
  }
  
  return retVal;
}


#endif // PX_COLOR_H
