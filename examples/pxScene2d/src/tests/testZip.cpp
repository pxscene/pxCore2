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


#include "testZip.h"

#include <stdio.h>
#include "rtZip.h"

void fileTest()
{
  rtZip z;

  printf("File Test\n");
  if (z.initFromFile("/home/johnrobinson/public_html/gallery.zip") == RT_OK)
  {
    rtData d;
    if (z.getFileData("gallery.js", d) == RT_OK)
    {
      for (uint32_t i = 0; i < d.length(); i++)
      {
        printf("%c", d.data()[i]);
      }
    }
  }
}

void bufferTest()
{
  rtZip z;

  printf("Buffer Test\n");
  rtData d;
  if (rtLoadFile("/home/johnrobinson/public_html/gallery.zip", d) == RT_OK)
  {
    if (z.initFromBuffer(d.data(), d.length()) == RT_OK)
    {
      rtData d;
      if (z.getFileData("fancy.js", d) == RT_OK)
      {
        for (uint32_t i = 0; i < d.length(); i++)
        {
          printf("%c", d.data()[i]);
        }
      }
      uint32_t fileCount = z.fileCount();
      printf("fileCount: %d\n", fileCount);
      for (uint32_t i = 0; i < fileCount; i++)
      {
        rtString filePath;
        if (z.getFilePathAtIndex(i, filePath) == RT_OK)
          printf("filePath[%d]: %s\n",i,filePath.cString());
        else
          printf("Failed\n");
      }
    }
  }
}

void testZip()
{
  printf("testZip");
  fileTest();
  bufferTest();
}
