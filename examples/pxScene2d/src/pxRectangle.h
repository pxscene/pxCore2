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
#include "pxPixel.h"


#include "pxColorNames.h"


class pxRectangle: public pxObject 
{
public:
  rtDeclareObject(pxRectangle, pxObject);
  rtProperty(fillColor, fillColor, setFillColor, rtValue);
  rtProperty(lineColor, lineColor, setLineColor, rtValue);
  rtProperty(lineWidth, lineWidth, setLineWidth, float);

  pxRectangle(pxScene2d* scene):pxObject(scene),mLineWidth(0) 
  {
    float f[4] = {0,0,0,1};
    float l[4] = {1,1,1,1};
    setFillColor(f);
    setLineColor(l);
    //mReady.send("resolve",this);
  }

  virtual void onInit();

  rtError fillColorInternal(uint32_t& c) const
  {
#ifdef PX_LITTLEENDIAN_PIXELS

    c = ((uint8_t) (mFillColor[PX_RED  ] * 255.0f) << 24) |  // R
        ((uint8_t) (mFillColor[PX_GREEN] * 255.0f) << 16) |  // G
        ((uint8_t) (mFillColor[PX_BLUE ] * 255.0f) <<  8) |  // B
        ((uint8_t) (mFillColor[PX_ALPHA] * 255.0f) <<  0);   // A
#else

    c = ((uint8_t) (mFillColor[PX_ALPHA] * 255.0f) << 24) |  // A
        ((uint8_t) (mFillColor[PX_BLUE ] * 255.0f) << 16) |  // B
        ((uint8_t) (mFillColor[PX_GREEN] * 255.0f) <<  8) |  // G
        ((uint8_t) (mFillColor[PX_RED  ] * 255.0f) <<  0);   // R

#endif
    return RT_OK;
  }

  rtError setFillColorInternal(uint32_t c)
  {
#ifdef PX_LITTLEENDIAN_PIXELS
    
    mFillColor[PX_RED  ] = (float)((c >>24) & 0xff) / 255.0f;  // R
    mFillColor[PX_GREEN] = (float)((c >>16) & 0xff) / 255.0f;  // G
    mFillColor[PX_BLUE ] = (float)((c >> 8) & 0xff) / 255.0f;  // B
    mFillColor[PX_ALPHA] = (float)((c >> 0) & 0xff) / 255.0f;  // A
#else
    
    mFillColor[PX_ALPHA] = (float)((c >>24) & 0xff) / 255.0f;  // A
    mFillColor[PX_BLUE ] = (float)((c >>16) & 0xff) / 255.0f;  // B
    mFillColor[PX_GREEN] = (float)((c >> 8) & 0xff) / 255.0f;  // G
    mFillColor[PX_RED  ] = (float)((c >> 0) & 0xff) / 255.0f;  // R
    
#endif

    return RT_OK;
  }


  rtError fillColor(rtValue &c) const
  {
    uint32_t cc = 0;
    rtError err = fillColorInternal(cc);
    
    c = cc;
    
    return err;
  }
  
  rtError setFillColor(rtValue c)
  {
    // Set via STRING...
    if( c.getType() == 's')
    {
      rtString str = c.toString();
      
      uint8_t r,g,b, a;
      if( web2rgb( str, r, g, b, a) == RT_OK)
      {
        mFillColor[PX_RED  ] = (float) r / 255.0f;  // R
        mFillColor[PX_GREEN] = (float) g / 255.0f;  // G
        mFillColor[PX_BLUE ] = (float) b / 255.0f;  // B
        mFillColor[PX_ALPHA] = (float) a / 255.0f;  // A
        
        return RT_OK;
      }
      
      return RT_FAIL;
    }
    
    // Set via UINT32...
    uint32_t clr = c.toUInt32();
    
    return setFillColorInternal(clr); 
  }
  
  rtError setLineColorInternal(uint32_t& c) const
  {
#ifdef PX_LITTLEENDIAN_PIXELS

    c = ((uint8_t) (mLineColor[PX_RED  ] * 255.0f) << 24) |  // R
        ((uint8_t) (mLineColor[PX_GREEN] * 255.0f) << 16) |  // G
        ((uint8_t) (mLineColor[PX_BLUE ] * 255.0f) <<  8) |  // B
        ((uint8_t) (mLineColor[PX_ALPHA] * 255.0f) <<  0);   // A
#else

    c = ((uint8_t) (mLineColor[PX_ALPHA] * 255.0f) << 24) |  // A
        ((uint8_t) (mLineColor[PX_BLUE ] * 255.0f) << 16) |  // B
        ((uint8_t) (mLineColor[PX_GREEN] * 255.0f) <<  8) |  // G
        ((uint8_t) (mLineColor[PX_RED  ] * 255.0f) <<  0);   // R

#endif

    return RT_OK;
  }


  rtError setLineColorInternal(uint32_t c)
  {
#ifdef PX_LITTLEENDIAN_PIXELS
    
    mLineColor[PX_RED  ] = (float)((c >> 24) & 0xff) / 255.0f;  // R
    mLineColor[PX_GREEN] = (float)((c >> 16) & 0xff) / 255.0f;  // G
    mLineColor[PX_BLUE ] = (float)((c >>  8) & 0xff) / 255.0f;  // B
    mLineColor[PX_ALPHA] = (float)((c >>  0) & 0xff) / 255.0f;  // A

#else
    
    mLineColor[PX_ALPHA] = (float)((c >> 24) & 0xff) / 255.0f;  // A
    mLineColor[PX_BLUE ] = (float)((c >> 16) & 0xff) / 255.0f;  // B
    mLineColor[PX_GREEN] = (float)((c >>  8) & 0xff) / 255.0f;  // G
    mLineColor[PX_RED  ] = (float)((c >>  0) & 0xff) / 255.0f;  // R

#endif
    return RT_OK;
  }
  
  rtError lineColor(rtValue &c) const
  {
    uint32_t cc = 0;
    rtError err = setLineColorInternal(cc);
    
    c = cc;
    
    return err;
  }

  rtError setLineColor(rtValue c)
  {
    // Set via STRING...
    if( c.getType() == 's')
    {
      rtString str = c.toString();
      
      uint8_t r,g,b, a;
      if( web2rgb( str, r, g, b, a) == RT_OK)
      {
        mLineColor[PX_RED  ] = (float) r / 255.0f;  // R
        mLineColor[PX_GREEN] = (float) g / 255.0f;  // G
        mLineColor[PX_BLUE ] = (float) b / 255.0f;  // B
        mLineColor[PX_ALPHA] = (float) a / 255.0f;  // A
        
        return RT_OK;
      }
      
      return RT_FAIL;
    }

    // Set via UINT32...
    uint32_t clr = c.toUInt32();
    
    return setLineColorInternal(clr);
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
    mFillColor[PX_RED  ] = c[PX_RED  ];
    mFillColor[PX_GREEN] = c[PX_GREEN];
    mFillColor[PX_BLUE ] = c[PX_BLUE ];
    mFillColor[PX_ALPHA] = c[PX_ALPHA];
  }
  
  // c is assumed to not be premultiplied
  void setLineColor(float* c) 
  {
    mLineColor[PX_RED  ] = c[PX_RED  ];
    mLineColor[PX_GREEN] = c[PX_GREEN];
    mLineColor[PX_BLUE ] = c[PX_BLUE ];
    mLineColor[PX_ALPHA] = c[PX_ALPHA];
  }
  
  virtual void draw();
  
private:
  float mFillColor[4];
  float mLineColor[4];
  float mLineWidth;
};

#endif // PX_RECTANGLE_H