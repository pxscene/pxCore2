#ifndef PX_CONTEXT_DESC_H
#define PX_CONTEXT_DESC_H

#ifdef PX_PLATFORM_WAYLAND_EGL
#include <GLES2/gl2.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#endif

typedef struct
{
  GLuint framebuffer;
  GLuint texture;
  int width;
  int height;
} pxContextSurfaceNativeDesc;

#endif //PX_CONTEXT_DESC_H