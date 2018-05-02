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
