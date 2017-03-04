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

// pxConstants.h

#ifndef _PX_CONSTANTS
#define _PX_CONSTANTS

#include "rtCore.h"
#include "rtRef.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "pxInterpolators.h"



class pxConstantsAnimation : public rtObject 
{

public:
  enum animationInterpolators {
    TWEEN_LINEAR = 0, 
    TWEEN_EXP1 = 1,
    TWEEN_EXP2,
    TWEEN_EXP3,
    TWEEN_STOP, 
    EASE_IN_QUAD,
    EASE_IN_CUBIC,
    EASE_IN_BACK,
    EASE_IN_ELASTIC,
    EASE_OUT_ELASTIC,
    EASE_OUT_BOUNCE, 
    EASE_INOUT_BOUNCE 
  };
  enum animationOptions {
    // Options  
    OPTION_OSCILLATE = 1,
    OPTION_LOOP,
    OPTION_FASTFORWARD = 8,
    OPTION_REWIND = 16  
  };
  enum animationCounts {  
    // Count
    COUNT_FOREVER = -1
  };

  rtDeclareObject(pxConstantsAnimation, rtObject);
  
  rtConstantProperty(TWEEN_LINEAR, TWEEN_LINEAR, uint32_t);
  rtConstantProperty(TWEEN_EXP1, TWEEN_EXP1, uint32_t);
  rtConstantProperty(TWEEN_EXP2, TWEEN_EXP2, uint32_t);
  rtConstantProperty(TWEEN_EXP3, TWEEN_EXP3, uint32_t);
  rtConstantProperty(TWEEN_STOP, TWEEN_STOP, uint32_t);
  rtConstantProperty(EASE_IN_QUAD, EASE_IN_QUAD, uint32_t);
  rtConstantProperty(EASE_IN_CUBIC, EASE_IN_CUBIC, uint32_t);
  rtConstantProperty(EASE_IN_BACK, EASE_IN_BACK, uint32_t);
  rtConstantProperty(EASE_IN_ELASTIC, EASE_IN_ELASTIC, uint32_t);
  rtConstantProperty(EASE_OUT_ELASTIC, EASE_OUT_ELASTIC, uint32_t);
  rtConstantProperty(EASE_OUT_BOUNCE, EASE_OUT_BOUNCE, uint32_t);
  // EASE_INOUT_BOUNCE not yet enabled
  // rtConstantProperty(EASE_INOUT_BOUNCE, EASE_INOUT_BOUNCE, int32_t);
  // OPTIONS
  rtConstantProperty(OPTION_OSCILLATE, OPTION_OSCILLATE, uint32_t);
  rtConstantProperty(OPTION_LOOP, OPTION_LOOP, uint32_t);
  rtConstantProperty(OPTION_FASTFORWARD, OPTION_FASTFORWARD, uint32_t);
  rtConstantProperty(OPTION_REWIND, OPTION_REWIND, uint32_t);
  rtConstantProperty(COUNT_FOREVER, COUNT_FOREVER, int32_t);

  rtReadOnlyProperty(interpolators, interpolators, rtObjectRef);
  
  rtError interpolators(rtObjectRef& v) const;
  
  pxInterp getInterpFunc(uint32_t c) 
  {
    switch((animationInterpolators)c)
    {
      case TWEEN_LINEAR: return pxInterpLinear; break;
      case TWEEN_EXP1: return pxExp1; break;
      case TWEEN_EXP2: return pxExp2; break;
      case TWEEN_EXP3: return pxExp3; break;
      case TWEEN_STOP: return pxStop; break;
      case EASE_IN_QUAD: return pxInQuad; break;
      case EASE_IN_CUBIC: return pxInCubic; break;
      case EASE_IN_BACK: return pxInBack; break;
      case EASE_IN_ELASTIC: return pxEaseInElastic; break;
      case EASE_OUT_ELASTIC: return pxEaseOutElastic; break;
      case EASE_OUT_BOUNCE: return pxEaseOutBounce; break;
      // case EASE_INOUT_BOUNCE: return pxEaseInOutBounce; break;
      default: return pxInterpLinear;
    };
  }

};



class pxConstantsStretch : public rtObject
{
public:
  enum constants {
    NONE = 0, 
    STRETCH,
    REPEAT,
  };
  rtDeclareObject(pxConstantsStretch, rtObject);
  
  rtConstantProperty(NONE, NONE, uint32_t);
  rtConstantProperty(STRETCH, STRETCH, uint32_t);
  rtConstantProperty(REPEAT, REPEAT, uint32_t);
};


class pxConstantsAlignVertical : public rtObject
{
public:
  enum constants {
    TOP = 0, 
    CENTER,
    BOTTOM,
  };
  rtDeclareObject(pxConstantsAlignVertical, rtObject);
  
  rtConstantProperty(TOP, TOP, uint32_t);
  rtConstantProperty(CENTER, CENTER, uint32_t);
  rtConstantProperty(BOTTOM, BOTTOM, uint32_t);
};

class pxConstantsAlignHorizontal : public rtObject
{
public:
  enum constants {
    LEFT = 0, 
    CENTER,
    RIGHT,
  };
  rtDeclareObject(pxConstantsAlignHorizontal, rtObject);
  
  rtConstantProperty(LEFT, LEFT, uint32_t);
  rtConstantProperty(CENTER, CENTER, uint32_t);
  rtConstantProperty(RIGHT, RIGHT, uint32_t);
};

class pxConstantsTruncation : public rtObject
{
public:
  enum constants {
    NONE = 0, 
    TRUNCATE,
    TRUNCATE_AT_WORD,
  };
  rtDeclareObject(pxConstantsTruncation, rtObject);
  
  rtConstantProperty(NONE, NONE, uint32_t);
  rtConstantProperty(TRUNCATE, TRUNCATE, uint32_t);
  rtConstantProperty(TRUNCATE_AT_WORD, TRUNCATE_AT_WORD, uint32_t);
};


/* Class for access to constants */
class pxConstants : public rtObject
{
  
public: 

  static rtRef<pxConstantsAnimation> animationConstants;
  static rtRef<pxConstantsStretch> stretchConstants;
  static rtRef<pxConstantsAlignVertical> alignVerticalConstants;
  static rtRef<pxConstantsAlignHorizontal> alignHorizontalConstants;
  static rtRef<pxConstantsTruncation> truncationConstants;  
  
};

#endif // PX_CONSTANTS
