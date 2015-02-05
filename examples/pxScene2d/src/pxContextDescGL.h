#ifndef PX_CONTEXT_DESC_H
#define PX_CONTEXT_DESC_H

#include <stddef.h>

#ifdef PX_PLATFORM_WAYLAND_EGL

#include <GLES2/gl2.h>

#else

#include <GL/glew.h>
#include <GL/gl.h>

#endif

typedef struct _pxContextSurfaceNativeDesc
{
    _pxContextSurfaceNativeDesc() : framebuffer(0), texture(0), 
            width(0), height(0),previousContextSurface(NULL) {}
  GLuint framebuffer;
  GLuint texture;
  int width;
  int height;
  _pxContextSurfaceNativeDesc* previousContextSurface;
}
pxContextSurfaceNativeDesc;

#endif //PX_CONTEXT_DESC_H
