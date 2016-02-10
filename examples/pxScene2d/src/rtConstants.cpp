#include "rtConstants.h"

rtConstantsAnimation rtConstants::animationConstants;
rtConstantsStretch rtConstants::stretchConstants;
rtConstantsAlignVertical rtConstants::alignVerticalConstants;
rtConstantsAlignHorizontal rtConstants::alignHorizontalConstants;
rtConstantsTruncation rtConstants::truncationConstants; 
  
rtError rtConstantsAnimation::interpolators(rtObjectRef& v) const
{
  rtRefT<rtArrayObject> keys = new rtArrayObject;
  {
    rtMethodMap* m = getMap();      

    rtPropertyEntry* e = m->getFirstProperty();
    while(e) 
    {
      // exclude anything that doesn't start with "TWEEN_" or "EASE_"
      if(!strncmp(e->mPropertyName,"TWEEN_",6) || !strncmp(e->mPropertyName,"EASE_",5)) {
        keys->pushBack(e->mPropertyName);
      }
      e = e->mNext;
    }
  }

  v = keys;
  return RT_OK;
}

//rtDefineObject(rtConstants, rtObject);
// Constants for Animation
rtDefineObject(rtConstantsAnimation, rtObject);
rtDefineProperty(rtConstantsAnimation, TWEEN_LINEAR);
rtDefineProperty(rtConstantsAnimation, TWEEN_EXP1);
rtDefineProperty(rtConstantsAnimation, TWEEN_EXP2);
rtDefineProperty(rtConstantsAnimation, TWEEN_EXP3);
rtDefineProperty(rtConstantsAnimation, TWEEN_STOP);
rtDefineProperty(rtConstantsAnimation, EASE_IN_QUAD);
rtDefineProperty(rtConstantsAnimation, EASE_IN_CUBIC);
rtDefineProperty(rtConstantsAnimation, EASE_IN_BACK);
rtDefineProperty(rtConstantsAnimation, EASE_IN_ELASTIC);
rtDefineProperty(rtConstantsAnimation, EASE_OUT_ELASTIC);
rtDefineProperty(rtConstantsAnimation, EASE_OUT_BOUNCE);
//rtDefineProperty(rtConstantsAnimation, EASE_INOUT_BOUNCE);
rtDefineProperty(rtConstantsAnimation, OPTION_END);
rtDefineProperty(rtConstantsAnimation, OPTION_OSCILLATE);
rtDefineProperty(rtConstantsAnimation, OPTION_LOOP);
rtDefineProperty(rtConstantsAnimation, OPTION_FASTFORWARD);
rtDefineProperty(rtConstantsAnimation, COUNT_FOREVER);
rtDefineProperty(rtConstantsAnimation, interpolators);
// Constants for Stretch
rtDefineObject(rtConstantsStretch, rtObject);
rtDefineProperty(rtConstantsStretch,NONE);
rtDefineProperty(rtConstantsStretch,STRETCH);
rtDefineProperty(rtConstantsStretch,REPEAT);
// Constants for Vertical Alignment
rtDefineObject(rtConstantsAlignVertical, rtObject);
rtDefineProperty(rtConstantsAlignVertical, TOP);
rtDefineProperty(rtConstantsAlignVertical, CENTER);
rtDefineProperty(rtConstantsAlignVertical, BOTTOM);
// Constants for Horizontal Alignment
rtDefineObject(rtConstantsAlignHorizontal, rtObject);
rtDefineProperty(rtConstantsAlignHorizontal, LEFT);
rtDefineProperty(rtConstantsAlignHorizontal, CENTER);
rtDefineProperty(rtConstantsAlignHorizontal, RIGHT);
// Constants for Truncation
rtDefineObject(rtConstantsTruncation, rtObject);
rtDefineProperty(rtConstantsTruncation, NONE);
rtDefineProperty(rtConstantsTruncation, TRUNCATE);
rtDefineProperty(rtConstantsTruncation, TRUNCATE_AT_WORD);
