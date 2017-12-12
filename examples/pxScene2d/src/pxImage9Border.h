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

// pxImage9Border.h

#ifndef PX_IMAGE9BORDER_H
#define PX_IMAGE9BORDER_H

#include "pxImage9.h"

class pxImage9Border: public pxImage9 {
public:
  rtDeclareObject(pxImage9Border, pxObject);
  rtProperty(borderLeft, borderLeft, setBorderLeft, float);
  rtProperty(borderTop, borderTop, setBorderTop, float);
  rtProperty(borderRight, borderRight, setBorderRight, float);
  rtProperty(borderBottom, borderBottom, setBorderBottom, float);

  pxImage9Border(pxScene2d* scene);
  virtual ~pxImage9Border();
  
  rtError borderLeft(float& v) const { v = mBorderLeft; return RT_OK; }
  rtError setBorderLeft(float v) { mBorderLeft = v; return RT_OK; }
  rtError borderTop(float& v) const { v = mBorderTop; return RT_OK; }
  rtError setBorderTop(float v) { mBorderTop = v; return RT_OK; }
  rtError borderRight(float& v) const { v = mBorderRight; return RT_OK; }
  rtError setBorderRight(float v) { mBorderRight = v; return RT_OK; }
  rtError borderBottom(float& v) const { v = mBorderBottom; return RT_OK; }
  rtError setBorderBottom(float v) { mBorderBottom = v; return RT_OK; }
  
protected:
  virtual void draw();
  
  float mBorderLeft, mBorderTop, mBorderRight, mBorderBottom;
};

#endif //PX_IMAGE9BORDER_H
