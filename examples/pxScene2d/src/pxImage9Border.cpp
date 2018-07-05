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

// pxImage9.cpp

#include "rtString.h"
#include "rtRef.h"
#include "pxScene2d.h"
#include "pxImage9Border.h"
#include "pxContext.h"

extern pxContext context;

pxImage9Border::pxImage9Border(pxScene2d* scene) : pxImage9(scene), mBorderLeft(0), mBorderTop(0), mBorderRight(0), mBorderBottom(0),
                                                   mMaskColor(), mDrawCenter(false)
{
  mMaskColor[0] = 1.0;
  mMaskColor[1] = 1.0;
  mMaskColor[2] = 1.0;
  mMaskColor[3] = 1.0;
}

pxImage9Border::~pxImage9Border()
{
}

void pxImage9Border::draw() {
  if (getImageResource() != NULL && getImageResource()->isInitialized() && !mSceneSuspended)
  {
    context.drawImage9Border(mw, mh, mBorderLeft, mBorderTop, mBorderRight, mBorderBottom, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom,
                             mDrawCenter, mMaskColor, getImageResource()->getTexture());
  }
}

rtDefineObject(pxImage9Border, pxImage9);
rtDefineProperty(pxImage9Border, borderLeft);
rtDefineProperty(pxImage9Border, borderTop);
rtDefineProperty(pxImage9Border, borderRight);
rtDefineProperty(pxImage9Border, borderBottom);
rtDefineProperty(pxImage9Border, maskColor);
rtDefineProperty(pxImage9Border, drawCenter);
