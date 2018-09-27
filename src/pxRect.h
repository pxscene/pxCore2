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

// pxRect.h

#ifndef PX_RECT_H
#define PX_RECT_H

#include "pxCore.h"
#include <stdint.h>

// A class used to describe a rectangle using integer coordinates
class pxRect
{
public:
  inline pxRect(): mLeft(0), mTop(0), mRight(0), mBottom(0) {}
  inline pxRect(int32_t l, int32_t t, int32_t r, int32_t b)
  {
    mLeft = l;
    mTop = t;
    mRight = r;
    mBottom = b;
  }
  
  inline int32_t width() const { return mRight-mLeft; }
  inline void setWidth(int32_t w) { mRight = mLeft + w; }

  inline int32_t height() const { return mBottom-mTop; }
  inline void setHeight(int32_t h) { mBottom = mBottom + h; }
  
  inline int32_t left() const { return mLeft; }
  inline void setLeft(int32_t l) { mLeft = l; }
  
  inline int32_t top() const { return mTop; }
  inline void setTop(int32_t t) { mTop = t; }

  inline int32_t bottom() const { return mBottom; }
  inline void setBottom(int32_t b) { mBottom = b; }

  inline int32_t right() const { return mRight; }
  inline void setRight(int32_t r) { mRight = r; }
  
  inline void setLTRB(int32_t l, int32_t t, int32_t r, int32_t b)
  {
    mLeft = l;
    mTop = t;
    mRight = r;
    mBottom = b;
  }
  
  inline void setLTWH(int32_t l, int32_t t, int32_t w, int32_t h)
  {
    mLeft = l;
    mTop = t;
    mRight = l + w;
    mBottom = t + h;
  }
  
  inline void set(int32_t l, int32_t t, int32_t r, int32_t b) { setLTRB(l, t, r, b); }
  
  inline void setEmpty() { setLTRB(0, 0, 0, 0); }
  
  void intersect(const pxRect& r)
  {
    mLeft = pxMax<int32_t>(mLeft, r.mLeft);
    mTop = pxMax<int32_t>(mTop, r.mTop);
    mRight = pxMin<int32_t>(mRight, r.mRight);
    mBottom = pxMin<int32_t>(mBottom, r.mBottom);
  }
  
  void unionRect(const pxRect& r)
  {
    if (!r.isEmpty())
    {
      if (!isEmpty())
      {
        mLeft = pxMin<int32_t>(mLeft, r.mLeft);
        mTop = pxMin<int32_t>(mTop, r.mTop);
        mRight = pxMax<int32_t>(mRight, r.mRight);
        mBottom = pxMax<int32_t>(mBottom, r.mBottom);
      }
      else
        *this = r;
    }
  }
  
  bool hitTest(int32_t x, int32_t y)
  {
    return (x >= mLeft && x <= mRight && y >= mTop && y <= mBottom);
  }
  
  bool isEmpty() const { return (mLeft >= mRight) || (mTop >= mBottom); }
  
  int isEqual(const pxRect& r) { return (mLeft != r.left() || mTop != r.top() || mRight != r.right() || mBottom != r.bottom()) ? false : true; }
private:
    int32_t mLeft, mTop, mRight, mBottom;
};


#endif

