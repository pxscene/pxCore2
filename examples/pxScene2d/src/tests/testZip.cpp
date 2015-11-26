// testSTring.cpp Copyright 2005-2015 John Robinson
// rtCore

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
