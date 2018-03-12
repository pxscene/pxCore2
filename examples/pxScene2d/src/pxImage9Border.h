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
  rtDeclareObject(pxImage9Border, pxImage9);
  rtProperty(borderLeft, borderLeft, setBorderLeft, float);
  rtProperty(borderTop, borderTop, setBorderTop, float);
  rtProperty(borderRight, borderRight, setBorderRight, float);
  rtProperty(borderBottom, borderBottom, setBorderBottom, float);
  rtProperty(maskColor, maskColor, setMaskColor, uint32_t);
  rtProperty(drawCenter, drawCenter, setDrawCenter, bool);

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

  rtError maskColor(uint32_t& c) const
  {
    c = 0;
    rtLogWarn("maskColor get not available");
    return RT_OK;
  }

  rtError setMaskColor(uint32_t c)
  {
    mMaskColor[0] = (float)((c>>24)&0xff)/255.0f;
    mMaskColor[1] = (float)((c>>16)&0xff)/255.0f;
    mMaskColor[2] = (float)((c>>8)&0xff)/255.0f;
    mMaskColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  bool drawCenter()            const { return mDrawCenter;}
  rtError drawCenter(bool& v)  const { v = mDrawCenter; return RT_OK;  }
  rtError setDrawCenter(bool v) { mDrawCenter = v; return RT_OK; }
  
protected:
  virtual void draw();
  
  float mBorderLeft, mBorderTop, mBorderRight, mBorderBottom;
  float mMaskColor[4];
  bool mDrawCenter;
};

#endif //PX_IMAGE9BORDER_H
