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

// pxImage9.cpp

#include "rtString.h"
#include "rtRef.h"
#include "pxScene2d.h"
#include "pxImage9Border.h"
#include "pxContext.h"

extern pxContext context;

pxImage9Border::pxImage9Border(pxScene2d* scene) : pxImage9(scene), mBorderLeft(0), mBorderTop(0), mBorderRight(0), mBorderBottom(0)
{
}

pxImage9Border::~pxImage9Border()
{
}

void pxImage9Border::draw() {
  if (getImageResource() != NULL)
  {
    context.drawImage9Border(mw, mh, mBorderLeft, mBorderTop, mBorderRight, mBorderBottom, mInsetLeft, mInsetTop, mInsetRight, mInsetBottom, getImageResource()->getTexture());
  }
}

rtDefineObject(pxImage9Border, pxObject);
rtDefineProperty(pxImage9Border, borderLeft);
rtDefineProperty(pxImage9Border, borderTop);
rtDefineProperty(pxImage9Border, borderRight);
rtDefineProperty(pxImage9Border, borderBottom);
