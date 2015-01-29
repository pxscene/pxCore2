#ifndef PX_SNAPSHOT_NO_GRAPHICS_H
#define PX_SNAPSHOT_NO_GRAPHICS_H

#include "pxCore.h"

class pxSnapshotNative
{
public:
  pxSnapshotNative() : mWidth(0), mHeight(0), mInitialized(false) {}
  ~pxSnapshotNative() {}

  void initialize(int width, int height) {(void)width; (void)height;}
  pxError beginSnapshotPainting() { return PX_FAIL; }
  void endSnapshotPainting() { }
  void prepareForDrawing() { }
  
protected:
  int mWidth, mHeight;
  bool mInitialized;
};

#endif //PX_SNAPSHOT_NO_GRAPHICS_H