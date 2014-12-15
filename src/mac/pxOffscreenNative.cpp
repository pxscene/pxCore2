// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.cpp

#include "pxOffscreen.h"

pxError pxOffscreen::init(int width, int height)
{
	term();

	Rect pr;
	MacSetRect(&pr, 0, 0, width, height);	

	data = new char[4 * width * height];
	
	if (data)
	{
		NewGWorldFromPtr (&gworld, 32, &pr, NULL, NULL, 0, (char*)data, 4*width);

		setBase(data);
		setWidth(width);
		setHeight(height);
		setStride(width*4);
		setUpsideDown(false);
	}

	return (gworld && data)?PX_OK:PX_FAIL;
}

pxError pxOffscreen::term()
{
	delete [] data; 
	data = NULL;
		
	if (gworld)
	{
		DisposeGWorld(gworld);
		gworld = NULL;
	}
	return PX_OK;
}


