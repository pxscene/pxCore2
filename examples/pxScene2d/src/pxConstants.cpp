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

// pxConstants.cpp

#include "pxConstants.h"

rtRef<pxConstantsAnimation> pxConstants::animationConstants = new pxConstantsAnimation();
rtRef<pxConstantsStretch> pxConstants::stretchConstants = new pxConstantsStretch();
rtRef<pxConstantsAlignVertical> pxConstants::alignVerticalConstants = new pxConstantsAlignVertical();
rtRef<pxConstantsAlignHorizontal> pxConstants::alignHorizontalConstants = new pxConstantsAlignHorizontal();
rtRef<pxConstantsTruncation> pxConstants::truncationConstants = new pxConstantsTruncation(); 
  
rtError pxConstantsAnimation::interpolators(rtObjectRef& v) const
{
  rtRef<rtArrayObject> keys = new rtArrayObject;
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

//rtDefineObject(pxConstants, rtObject);
// Constants for Animation
rtDefineObject(pxConstantsAnimation, rtObject);
rtDefineProperty(pxConstantsAnimation, TWEEN_LINEAR);
rtDefineProperty(pxConstantsAnimation, TWEEN_EXP1);
rtDefineProperty(pxConstantsAnimation, TWEEN_EXP2);
rtDefineProperty(pxConstantsAnimation, TWEEN_EXP3);
rtDefineProperty(pxConstantsAnimation, TWEEN_STOP);
rtDefineProperty(pxConstantsAnimation, EASE_IN_QUAD);
rtDefineProperty(pxConstantsAnimation, EASE_IN_CUBIC);
rtDefineProperty(pxConstantsAnimation, EASE_IN_BACK);
rtDefineProperty(pxConstantsAnimation, EASE_IN_ELASTIC);
rtDefineProperty(pxConstantsAnimation, EASE_OUT_ELASTIC);
rtDefineProperty(pxConstantsAnimation, EASE_OUT_BOUNCE);
//rtDefineProperty(pxConstantsAnimation, EASE_INOUT_BOUNCE);
rtDefineProperty(pxConstantsAnimation, OPTION_OSCILLATE);
rtDefineProperty(pxConstantsAnimation, OPTION_LOOP);
rtDefineProperty(pxConstantsAnimation, OPTION_FASTFORWARD);
rtDefineProperty(pxConstantsAnimation, OPTION_REWIND);
rtDefineProperty(pxConstantsAnimation, COUNT_FOREVER);
rtDefineProperty(pxConstantsAnimation, interpolators);
rtDefineProperty(pxConstantsAnimation, STATUS_IDLE);
rtDefineProperty(pxConstantsAnimation, STATUS_INPROGRESS);
rtDefineProperty(pxConstantsAnimation, STATUS_CANCELLED);
rtDefineProperty(pxConstantsAnimation, STATUS_ENDED);
// Constants for Stretch
rtDefineObject(pxConstantsStretch, rtObject);
rtDefineProperty(pxConstantsStretch,NONE);
rtDefineProperty(pxConstantsStretch,STRETCH);
rtDefineProperty(pxConstantsStretch,REPEAT);
// Constants for Vertical Alignment
rtDefineObject(pxConstantsAlignVertical, rtObject);
rtDefineProperty(pxConstantsAlignVertical, TOP);
rtDefineProperty(pxConstantsAlignVertical, CENTER);
rtDefineProperty(pxConstantsAlignVertical, BOTTOM);
// Constants for Horizontal Alignment
rtDefineObject(pxConstantsAlignHorizontal, rtObject);
rtDefineProperty(pxConstantsAlignHorizontal, LEFT);
rtDefineProperty(pxConstantsAlignHorizontal, CENTER);
rtDefineProperty(pxConstantsAlignHorizontal, RIGHT);
// Constants for Truncation
rtDefineObject(pxConstantsTruncation, rtObject);
rtDefineProperty(pxConstantsTruncation, NONE);
rtDefineProperty(pxConstantsTruncation, TRUNCATE);
rtDefineProperty(pxConstantsTruncation, TRUNCATE_AT_WORD);
