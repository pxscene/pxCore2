/*

 pxCore Copyright 2005-2019 John Robinson

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

#ifndef PX_EFFECTS_H
#define PX_EFFECTS_H

typedef struct _pxTextEffects // meant to be copied using memcpy()
{
    bool shadowEnabled;
    float shadowColor[4];
    float shadowBlur;
    float shadowOffsetX;
    float shadowOffsetY;
    float shadowWidth;
    float shadowHeight;

    bool highlightEnabled;
    float highlightColor[4];
    float highlightOffset;
    float highlightWidth;
    float highlightHeight;
    float highlightPaddingLeft;
    float highlightPaddingRight;
    float highlightBlockHeight;

    // calculated later
    float shadowX;
    float shadowY;
    float shadowRadius;
    _pxTextEffects()
    : shadowEnabled(false)
    , shadowColor{0.0f, 0.0f, 0.0f, 0.0f}
    , shadowBlur(0.0f)
    , shadowOffsetX(0.0f)
    , shadowOffsetY(0.0f)
    , shadowWidth(0.0f)
    , shadowHeight(0.0f)
    , highlightEnabled(false)
    , highlightColor{0.0f, 0.0f, 0.0f, 0.0f}
    , highlightOffset(0.0f)
    , highlightWidth(0.0f)
    , highlightHeight(0.0f)
    , highlightPaddingLeft(0.0f)
    , highlightPaddingRight(0.0f)
    , highlightBlockHeight(0.0f)
    , shadowX(0.0f)
    , shadowY(0.0f)
    , shadowRadius(0.0f)
    {}
} pxTextEffects;

#endif //PX_EFFECTS_H
