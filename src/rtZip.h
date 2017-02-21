// rtCore CopyRight 2007-2015 John Robinson
// rtZip.h

#ifndef _RT_ZIP_H
#define _RT_ZIP_H

#include "rtError.h"
#include "rtFile.h"
#include "rtString.h"

extern "C"
{
#include "zlib.h"
#include "unzip.h"
}

#include <vector>

class rtZip
{
public:
  rtZip();
  ~rtZip();

  rtError initFromBuffer(const void* buffer,size_t bufferSize);
  rtError initFromFile(const char* fileName);
  rtError term();

  uint32_t fileCount() const;
  rtError getFilePathAtIndex(uint32_t i,rtString& filePath) const;

  rtError getFileData(const char* filePath,rtData& d) const;

  static bool isZip(const void* buffer, size_t bufferSize);

private:
  unzFile mUnzFile;
  rtData mData;

  mutable std::vector<rtString> mFilePaths;
};

#endif
