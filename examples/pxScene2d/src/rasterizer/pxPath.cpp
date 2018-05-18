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


pxPath::pxPath(pxScene2d* scene): pxObject(scene)

                           //       mStrokeColor(pxClear),
                           //       mStrokeType(pxCanvas2d::StrokeType::inside),
                           //       mFillColor(pxClear)
{
  mx = 0;
  my = 0;
 // canvas
}


void pxPath::onInit()
{
  mInitialized = true;
  
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
  context.drawOffscreen(0, 0, 0, 0, mImage.width(), mImage.height(), mImage);
}

rtError pxPath::setPath(const rtString d)
{
  mPath = d;
  char *s = (char *)d.cString();
  
  if(!d || !s)
  {
    return RT_ERROR;
  }
    
  size_t  len = strlen(d);
  
  rtError ret = pxLoadSVGImage(s, len, mImage, w(), h() );

  sendPromise();

  return ret;
}

rtDefineObject(pxPath, pxObject);

rtDefineProperty(pxPath, d);

//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
//====================================================================================================================================
