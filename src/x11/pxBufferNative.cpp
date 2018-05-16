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

// pxBufferNative.cpp

#include "../pxCore.h"
#include "pxBufferNative.h"
#include "../pxRect.h"

#include "../pxOffscreen.h"

void pxBuffer::blit(pxSurfaceNative s, int dstLeft, int dstTop, 
                    int dstWidth, int dstHeight,
                    int srcLeft, int srcTop)
{
  XImage* image = ::XCreateImage(s->display,
                                 XDefaultVisual(s->display,
                                                XDefaultScreen(s->display)),
                                 24,ZPixmap, 0, (char*)base(),
                                 width(), height(), 32, stride());
  
  if (image)
  {
    ::XPutImage(s->display, s->drawable, s->gc, image, srcLeft, srcTop,
                dstLeft, dstTop, dstWidth, dstHeight);

    // If we don't NULL this out XDestroyImage will damage
    // the heap by trying to free it internally
    image->data = NULL;
    XDestroyImage(image);
  }
}


