#ifndef PX_SNAPSHOT_H
#define PX_SNAPSHOT_H

#include "rtCore.h"

class pxSnapshot : public pxSnapshotNative
{
public:
    pxSnapshot() {}
    ~pxSnapshot() {}
    
  int getWidth() { return mWidth; }
  int getHeight() { return mHeight; }
  bool isInitialized() { return mInitialized; }
};

#endif //PX_SNAPSHOT_H