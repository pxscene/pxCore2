#ifndef PX_CONTEXT_DESC_H
#define PX_CONTEXT_DESC_H

#include <stddef.h>

#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)

#include <GLES2/gl2.h>

#else

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

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
