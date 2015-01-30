#ifndef PX_CONTEXT_DESC_H
#define PX_CONTEXT_DESC_H

#include <directfb.h>

typedef struct
{
  DFBSurfaceDescription dsc;

  IDirectFBSurface      *surface;

//  GLuint framebuffer;
//  GLuint texture;

  int width;
  int height;
}
pxContextSurfaceNativeDesc;

#endif //PX_CONTEXT_DESC_H
