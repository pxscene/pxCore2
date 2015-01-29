#include "pxSnapshotOpenGL.h"

#include <iostream>

using namespace std;

pxSnapshotNative::pxSnapshotNative() : 
  mWidth(0), mHeight(0), mInitialized(false),mFramebuffer(0), mTexture(0)
{
}

pxSnapshotNative::~pxSnapshotNative()
{
  destroySnapshot();
}

void pxSnapshotNative::destroySnapshot()
{
  if (mFramebuffer != 0)
  {
    glDeleteFramebuffers(1, &mFramebuffer);
    mFramebuffer = 0;
  }
  if (mTexture != 0)
  {
    glDeleteTextures(1, &mTexture);
    mTexture = 0;
  }
  mInitialized = false;
}

void pxSnapshotNative::initialize(int width, int height)
{
  destroySnapshot();
  mWidth = width;
  mHeight = height;
  
  glGenFramebuffers(1, &mFramebuffer);
  glGenTextures(1, &mTexture);
  glBindTexture(GL_TEXTURE_2D, mTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
	       mWidth, mHeight, 0, GL_BGRA_EXT,
	       GL_UNSIGNED_BYTE, NULL);
  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  mInitialized = true;
}

pxError pxSnapshotNative::beginSnapshotPainting()
{
  if (!mInitialized)
  {
    cout << "snapshot is not initialized\n";
    return PX_NOTINITIALIZED;
  }
  glBindFramebuffer(GL_FRAMEBUFFER, mFramebuffer);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, 
                          GL_TEXTURE_2D, mTexture, 0);
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    cout << "error starting snapshot capture\n";
    return PX_FAIL;
  }
  
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  return PX_OK;
}
void pxSnapshotNative::endSnapshotPainting()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void pxSnapshotNative::prepareForDrawing()
{
  glBindTexture(GL_TEXTURE_2D, mTexture);
}