// pxCore CopyRight 2007 John Robinson
// Portable Framebuffer and Windowing Library
// pxOffscreenNative.h

#ifndef PX_OFFSCREEN_NATIVE_H
#define PX_OFFSCREEN_NATIVE_H

#include "../pxBuffer.h"

class pxOffscreenNative: public pxBuffer
{
public:
	pxOffscreenNative(): gworld(NULL), data(NULL) {}
protected:
	GWorldPtr gworld;
	char* data;
};

#endif