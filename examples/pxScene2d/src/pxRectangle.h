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

// pxRectangle.h

#ifndef PX_RECTANGLE_H
#define PX_RECTANGLE_H

#include "pxScene2d.h"

class pxRectangle: public pxObject 
{
public:
  rtDeclareObject(pxRectangle, pxObject);
  rtProperty(fillColor, fillColor, setFillColor, uint32_t);
  rtProperty(lineColor, lineColor, setLineColor, uint32_t);
  rtProperty(lineWidth, lineWidth, setLineWidth, float);

  pxRectangle(pxScene2d* scene):pxObject(scene),mLineWidth(0) 
  {
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
    //mReady.send("resolve",this);
  }

  virtual void onInit() {mReady.send("resolve",this);}

  rtError fillColor(uint32_t& c) const
  {
#ifdef PX_LITTLEENDIAN_PIXELS

    c = ((uint8_t) (mFillColor[0] * 255.0f) << 24) |  // R
        ((uint8_t) (mFillColor[1] * 255.0f) << 16) |  // G
        ((uint8_t) (mFillColor[2] * 255.0f) <<  8) |  // B
        ((uint8_t) (mFillColor[3] * 255.0f) <<  0);   // A
#else

    c = ((uint8_t) (mFillColor[3] * 255.0f) << 24) |  // A
        ((uint8_t) (mFillColor[2] * 255.0f) << 16) |  // B
        ((uint8_t) (mFillColor[1] * 255.0f) <<  8) |  // G
        ((uint8_t) (mFillColor[0] * 255.0f) <<  0);   // R

#endif
    return RT_OK;
  }

  rtError setFillColor(uint32_t c) 
  {
    mFillColor[0] = (float)((c>>24)&0xff)/255.0f;
    mFillColor[1] = (float)((c>>16)&0xff)/255.0f;
    mFillColor[2] = (float)((c>>8)&0xff)/255.0f;
    mFillColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineColor(uint32_t& c) const
  {
#ifdef PX_LITTLEENDIAN_PIXELS

    c = ((uint8_t) (mLineColor[0] * 255.0f) << 24) |  // R
        ((uint8_t) (mLineColor[1] * 255.0f) << 16) |  // G
        ((uint8_t) (mLineColor[2] * 255.0f) <<  8) |  // B
        ((uint8_t) (mLineColor[3] * 255.0f) <<  0);   // A
#else

    c = ((uint8_t) (mLineColor[3] * 255.0f) << 24) |  // A
        ((uint8_t) (mLineColor[2] * 255.0f) << 16) |  // B
        ((uint8_t) (mLineColor[1] * 255.0f) <<  8) |  // G
        ((uint8_t) (mLineColor[0] * 255.0f) <<  0);   // R

#endif

    return RT_OK;
  }

  rtError setLineColor(uint32_t c) 
  {
    mLineColor[0] = (float)((c>>24)&0xff)/255.0f;
    mLineColor[1] = (float)((c>>16)&0xff)/255.0f;
    mLineColor[2] = (float)((c>>8)&0xff)/255.0f;
    mLineColor[3] = (float)((c>>0)&0xff)/255.0f;
    return RT_OK;
  }

  rtError lineWidth(float& w) const 
  {
    w = mLineWidth;
    return RT_OK;
  }
  
  rtError setLineWidth(float w) 
  {
    mLineWidth = w;
    return RT_OK;
  }

  // c is assumed to not be premultiplied
  void setFillColor(float* c) 
  {
    mFillColor[0] = c[0];
    mFillColor[1] = c[1];
    mFillColor[2] = c[2];
    mFillColor[3] = c[3];
  }
  
  // c is assumed to not be premultiplied
  void setLineColor(float* c) 
  {
    mLineColor[0] = c[0];
    mLineColor[1] = c[1];
    mLineColor[2] = c[2];
    mLineColor[3] = c[3];
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};

#endif
