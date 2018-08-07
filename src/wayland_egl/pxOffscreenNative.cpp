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

#include "../pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>

pxError pxOffscreen::init(int width, int height)
{
    term();

    pxError e = PX_FAIL;

    data = (char*) new unsigned char[width * height * 4];

    if (data)
    {
	setBase(data);
	setWidth(width);
	setHeight(height);
	setStride(width*4);
	setUpsideDown(false);
	e = PX_OK;
    }

    //code

    return e;
}

pxError pxOffscreen::term()
{
    return pxOffscreenNative::term();
}

pxError pxOffscreenNative::term()
{
    delete [] data;
    data = NULL;
    
    return PX_OK;
}



