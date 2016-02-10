// rtCore Copyright 2007-2015 John Robinson
// rtConstants.h

#ifndef _RT_CONSTANTS
#define _RT_CONSTANTS

#include <stdint.h>

#include "rtRefT.h"

// TODO rtDefs vs rtCore.h
#include "rtDefs.h"
#include "rtError.h"
#include "rtValue.h"
#include "rtObject.h"
#include "rtObjectMacros.h"
#include "pxInterpolators.h"

class rtConstantsAnimation;
class rtConstantsStretch;
class rtConstantsAlignVertical;
class rtConstantsAlignHorizontal;
class rtConstantsTruncation;

/* Class for access to constants */
class rtConstants : public rtObject
{
  
public:  
  static rtConstantsAnimation animationConstants;
  static rtConstantsStretch stretchConstants;
  static rtConstantsAlignVertical alignVerticalConstants;
  static rtConstantsAlignHorizontal alignHorizontalConstants;
  static rtConstantsTruncation truncationConstants;  
  
};

class rtConstantsAnimation : public rtObject 
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
    OPTION_END= 0,
    OPTION_OSCILLATE,
    OPTION_LOOP,
    OPTION_FASTFORWARD = 8 
  };
  enum animationCounts {  
    // Count
    COUNT_FOREVER = -1
  };

  rtDeclareObject(rtConstantsAnimation, rtObject);
  
  rtConstantProperty(TWEEN_LINEAR, TWEEN_LINEAR, int32_t);
  rtConstantProperty(TWEEN_EXP1, TWEEN_EXP1, int32_t);
  rtConstantProperty(TWEEN_EXP2, TWEEN_EXP2, int32_t);
  rtConstantProperty(TWEEN_EXP3, TWEEN_EXP3, int32_t);
  rtConstantProperty(TWEEN_STOP, TWEEN_STOP, int32_t);
  rtConstantProperty(EASE_IN_QUAD, EASE_IN_QUAD, int32_t);
  rtConstantProperty(EASE_IN_CUBIC, EASE_IN_CUBIC, int32_t);
  rtConstantProperty(EASE_IN_BACK, EASE_IN_BACK, int32_t);
  rtConstantProperty(EASE_IN_ELASTIC, EASE_IN_ELASTIC, int32_t);
  rtConstantProperty(EASE_OUT_ELASTIC, EASE_OUT_ELASTIC, int32_t);
  rtConstantProperty(EASE_OUT_BOUNCE, EASE_OUT_BOUNCE, int32_t);
  // EASE_INOUT_BOUNCE not yet enabled
  // rtConstantProperty(EASE_INOUT_BOUNCE, EASE_INOUT_BOUNCE, int32_t);
  rtConstantProperty(OPTION_END, OPTION_END, int32_t);
  rtConstantProperty(OPTION_OSCILLATE, OPTION_OSCILLATE, int32_t);
  rtConstantProperty(OPTION_LOOP, OPTION_LOOP, int32_t);
  rtConstantProperty(OPTION_FASTFORWARD, OPTION_FASTFORWARD, int32_t);
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



class rtConstantsStretch : public rtObject
{
public:
  enum constants {
    NONE = 0, 
    STRETCH,
    REPEAT,
  };
  rtDeclareObject(rtConstantsStretch, rtObject);
  
  rtConstantProperty(NONE, NONE, int32_t);
  rtConstantProperty(STRETCH, STRETCH, int32_t);
  rtConstantProperty(REPEAT, REPEAT, int32_t);
};


class rtConstantsAlignVertical : public rtObject
{
public:
  enum constants {
    TOP = 0, 
    CENTER,
    BOTTOM,
  };
  rtDeclareObject(rtConstantsAlignVertical, rtObject);
  
  rtConstantProperty(TOP, TOP, int32_t);
  rtConstantProperty(CENTER, CENTER, int32_t);
  rtConstantProperty(BOTTOM, BOTTOM, int32_t);
};

class rtConstantsAlignHorizontal : public rtObject
{
public:
  enum constants {
    LEFT = 0, 
    CENTER,
    RIGHT,
  };
  rtDeclareObject(rtConstantsAlignHorizontal, rtObject);
  
  rtConstantProperty(LEFT, LEFT, int32_t);
  rtConstantProperty(CENTER, CENTER, int32_t);
  rtConstantProperty(RIGHT, RIGHT, int32_t);
};

class rtConstantsTruncation : public rtObject
{
public:
  enum constants {
    NONE = 0, 
    TRUNCATE,
    TRUNCATE_AT_WORD,
  };
  rtDeclareObject(rtConstantsTruncation, rtObject);
  
  rtConstantProperty(NONE, NONE, int32_t);
  rtConstantProperty(TRUNCATE, TRUNCATE, int32_t);
  rtConstantProperty(TRUNCATE_AT_WORD, TRUNCATE_AT_WORD, int32_t);
};



#endif
