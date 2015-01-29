#ifndef PX_SNAPSHOT_OPENGL_H
#define PX_SNAPSHOT_OPENGL_H

#ifdef PX_PLATFORM_WAYLAND_EGL
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glext.h>
#endif //PX_PLATFORM_WAYLAND_EGL

#include "pxCore.h"

class pxSnapshotNative
{
public:
  pxSnapshotNative();
  ~pxSnapshotNative();

  void initialize(int width, int height);
  pxError beginSnapshotPainting();
  void endSnapshotPainting();
  void prepareForDrawing();
  
protected:
  int mWidth, mHeight;
  bool mInitialized;
    
private:
  void destroySnapshot();
  GLuint mFramebuffer;
  GLuint mTexture;
};

#endif //PX_SNAPSHOT_OPENGL_H