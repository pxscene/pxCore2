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

  float iw = ( w() == 0 ) ? mImage.width()  : w();
  float ih = ( h() == 0 ) ? mImage.height() : h();
  
  rtError ret = pxLoadSVGImage(s, len, mImage, iw, ih);
  
  if(ret == RT_OK)
  {
    setW( iw ); // Use SVG size - of not set
    setH( ih ); // Use SVG size - of not set
    
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
  if(mInitialized)
  {
    static pxTextureRef nullMaskRef;

    context.drawImage(0, 0,  mTexture->width(), mTexture->height(),
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
