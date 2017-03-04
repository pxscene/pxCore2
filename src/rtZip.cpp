/*

 pxCore Copyright 2005-2017 John Robinson

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

//https://www.iana.org/assignments/uri-schemes/prov/jar

#include "rtZip.h"
#include "string.h"

extern "C"
{
void fill_memory_filefunc64 (zlib_filefunc64_def* pzlib_filefunc_def);
}

rtZip::rtZip(): mUnzFile(NULL) {}
rtZip::~rtZip() { term(); }

rtError rtZip::initFromBuffer(const void* buffer, size_t bufferSize)
{
  char path[64] = {0};
  zlib_filefunc64_def memory_file;

  mData.init((uint8_t*)buffer, bufferSize);
  
  sprintf(path, "%p+%x", mData.data(), mData.length());
  
  fill_memory_filefunc64(&memory_file);
  mUnzFile = unzOpen2_64(path, &memory_file);

  mFilePaths.empty();

  return mUnzFile?RT_OK:RT_FAIL;
} 

rtError rtZip::initFromFile(const char* fileName)
{
  mUnzFile = unzOpen64(fileName);
  mFilePaths.empty();
  return mUnzFile?RT_OK:RT_FAIL;
}

rtError rtZip::term()
{
  if (mUnzFile)
  {
    unzClose(mUnzFile);
  }
  return RT_OK;
}

uint32_t rtZip::fileCount() const
{
  uint32_t count = 0;
  unz_global_info64 info;
  if (unzGetGlobalInfo64(mUnzFile, &info) == UNZ_OK)
  {
    count = info.number_entry;
  }
  return count;
}

rtError rtZip::getFilePathAtIndex(uint32_t i,rtString& filePath) const
{
  rtError e = RT_FAIL;
 
  if (mFilePaths.size() == 0)
  {
    int u = unzGoToFirstFile(mUnzFile);
    while (u == UNZ_OK)
    {
      char buffer[1024];
      if (unzGetCurrentFileInfo64(mUnzFile,NULL,buffer,sizeof(buffer)-1,
                                  NULL,0,NULL,0) == UNZ_OK)
      {
        mFilePaths.push_back(buffer);
      }

      u = unzGoToNextFile(mUnzFile);
    }
  }

  if (i < mFilePaths.size())
  {
    filePath = mFilePaths[i];
    e = RT_OK;
  }

  return e;
}

rtError rtZip::getFileData(const char* filePath, rtData& d) const
{
  rtError e = RT_FAIL;
  int err = UNZ_OK;
  if (unzLocateFile(mUnzFile, filePath, 0)==UNZ_OK)
  {
    unz_file_info64 fileInfo;
    char filename[256];
    err = unzGetCurrentFileInfo64(mUnzFile, &fileInfo, filename, 
                                  sizeof(filename),NULL,0,NULL,0);
    if (err == UNZ_OK)
    {
      // TODO warning truncating size
      if (d.init((uint32_t)fileInfo.uncompressed_size) == RT_OK)
      {
        err = unzOpenCurrentFilePassword(mUnzFile, NULL);
        if (err == UNZ_OK)
        {
          int amount = unzReadCurrentFile(mUnzFile,d.data(),d.length());
          if ((uint32_t)amount == d.length())
          {
            e = RT_OK;
          }
          err = unzCloseCurrentFile(mUnzFile);
        }
      }
    }
  }
  return e;
}

bool rtZip::isZip(const void* buffer, size_t bufferSize)
{
  const char zipMagic[] = "\x50\x4b\x03\x04";

  bool result = false;
  if (buffer)
  {
    if (bufferSize > strlen(zipMagic))
      result = (strncmp(zipMagic, (const char*)buffer, strlen(zipMagic)) == 0);    
  }
  return result;
}
