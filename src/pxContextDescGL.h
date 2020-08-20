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

// pxContextDescGL.h

#ifndef PX_CONTEXT_DESC_H
#define PX_CONTEXT_DESC_H

//#include <stddef.h>

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
#ifdef PX_PLATFORM_GLUT
#include <GL/glut.h>
#endif
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
