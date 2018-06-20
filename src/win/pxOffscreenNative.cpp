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

// pxOffscreenNative.cpp

#define PX_NATIVE

#include "../pxCore.h"
#include "pxOffscreenNative.h"
#include "../pxOffscreen.h"
#include "../pxRect.h"

pxError pxOffscreen::init(int width, int height)
{
    // Optimization
    if (bitmap)
    {
        if (width == pxBuffer::width() && height == pxBuffer::height())
            return PX_OK;
    }

    term();  // release all resources if this object is reinitialized

    void *base;

#ifdef WINCE
    BITMAPINFO bi = { { sizeof(BITMAPINFOHEADER), width, height, 1, 32 } };            
    bitmap = CreateDIBSection(NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS, 
                                (void **)&base, NULL, NULL);
#else
	
	BITMAPV5HEADER bi;
    ZeroMemory(&bi,sizeof(BITMAPV5HEADER));
    bi.bV5Size           = sizeof(BITMAPV5HEADER);
    bi.bV5Width           = width;
    bi.bV5Height          = height;
    bi.bV5Planes = 1;
    bi.bV5BitCount = 32;
    bi.bV5Compression = BI_BITFIELDS;
    // The following mask specification specifies a supported 32 BPP
    // alpha format for Windows XP.
    bi.bV5RedMask   =  0x00FF0000;
    bi.bV5GreenMask =  0x0000FF00;
    bi.bV5BlueMask  =  0x000000FF;
    bi.bV5AlphaMask =  0xFF000000; 

    bitmap = CreateDIBSection(NULL, (BITMAPINFO*)&bi, DIB_RGB_COLORS, 
                                (void **)&base, NULL, NULL);
#endif
    
    setBase(base);
    setWidth(width);
    setHeight(height);
    setStride(width*4);
    setUpsideDown(true);
    
    return bitmap?PX_OK:PX_FAIL;
}

pxError pxOffscreen::term()
{
    if (bitmap)
    {
		DeleteObject(bitmap);
        setBase(NULL);
		setWidth(0);
		setHeight(0);
		setStride(0);
    }

	return PX_OK;
}

