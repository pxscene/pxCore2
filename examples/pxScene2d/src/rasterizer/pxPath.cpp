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

#include "pxScene2d.h"
#include "pxContext.h"

#include "pxPath.h"


extern pxContext context;


void pxPath::onInit()
{
  char *s = (char *)mPath.cString();
  
  if(!s)
  {
    rtLogError("Error creating pxPath - path string is NULL");
    return;
  }
  
  size_t  len = mPath.length();

  if(!len)
  {
    rtLogError("Error creating pxPath - path string is EMPTY");
    return;
  }

  // If pxObject dimensions ARE set ... relate to SVG
  //
  float iw = ( w() <= 0 ) ? mImage.width()  : w();
  float ih = ( h() <= 0 ) ? mImage.height() : h();
  
  rtError ret = pxLoadSVGImage(s, len, mImage, static_cast<int>(iw), static_cast<int>(ih));
  
  // If pxObject dimensions NOT set yet ... infer from SVG
  //
  if(iw <=0) iw = static_cast<float>(mImage.width());
  if(ih <=0) ih = static_cast<float>(mImage.height());
  
  if(ret == RT_OK)
  {
    setW( iw ); // Use SVG size - of not set
    setH( ih ); // Use SVG size - of not set
  
    // premultiply
    for (int y = 0; y < mImage.height(); y++)
    {
      pxPixel* d = mImage.scanline(y);
      pxPixel* de = d + mImage.width();
      while (d < de)
      {
        d->r = (d->r * d->a)/255;
        d->g = (d->g * d->a)/255;
        d->b = (d->b * d->a)/255;
        d++;
      }
    }
    
    mTexture = context.createTexture(mImage);
    
    mInitialized = true;
  }
  
  sendPromise();
}

pxPath::~pxPath()
{

}

void pxPath::sendPromise()
{
    mReady.send("resolve",this);
}

void pxPath::draw()
{
  if(mInitialized && mTexture)
  {
    static pxTextureRef nullMaskRef;

    context.drawImage(0, 0,  static_cast<float>(mTexture->width()), static_cast<float>(mTexture->height()),
                      mTexture, nullMaskRef,  false, NULL,
                      pxConstantsStretch::NONE,
                      pxConstantsStretch::NONE,
                      false, pxConstantsMaskOperation::NORMAL);
  }
}

rtError pxPath::setPath(const rtString d)
{
  mPath = d;

  return RT_OK;
}

rtDefineObject(pxPath, pxObject);

rtDefineProperty(pxPath, d);

//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
