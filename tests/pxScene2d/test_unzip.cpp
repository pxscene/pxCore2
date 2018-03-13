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

#include <sstream>

#define private public
#define protected public

#include "unzip.h"
#include <string.h>
#include <unistd.h>


#include "test_includes.h" // Needs to be included last

using namespace std;

ZPOS64_T tell64_file  OF((voidpf opaque, voidpf stream))
{
  return 0;
}

long seek64_file    OF((voidpf opaque, voidpf stream, ZPOS64_T offset, int origin))
{
  return 0;
}

voidpf open64_file OF((voidpf opaque, const void* filename, int mode))
{
  return NULL;
}

voidpf open_file OF((voidpf opaque, const char* filename, int mode))
{
  return NULL;
}

uLong read_file OF((voidpf opaque, voidpf stream, void* buf, uLong size))
{
  return 0;
}

uLong write_file OF((voidpf opaque, voidpf stream, const void* buf, uLong size))
{
  return 0;
}

long tell_file OF((voidpf opaque, voidpf stream))
{
  return 0;
}

long seek_file OF((voidpf opaque, voidpf stream, uLong offset, int origin))
{
  return 0;
}

int close_file OF((voidpf opaque, voidpf stream))
{
  return 0;
}

int testerror_file OF((voidpf opaque, voidpf stream))
{
  return 0;
}

zlib_filefunc_def customdef = {&open_file, &read_file, &write_file, &tell_file, &seek_file, &close_file, &testerror_file, NULL};
zlib_filefunc64_def custom64def = {&open64_file, &read_file, &write_file, &tell64_file, &seek64_file, &close_file, &testerror_file, NULL};

class unzipTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void zipOpenTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE(NULL != mFileHandle);
      unzClose(mFileHandle);
    }

    void zipOpenFailedTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest1.zip");
      EXPECT_TRUE(NULL == mFileHandle);
    }

    void zipOpen64Test()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE(NULL != mFileHandle);
      unzClose(mFileHandle);
    }

    void zipOpen2NoFileHandlerTest()
    {
      mFileHandle = unzOpen2("supportfiles/ziptest.zip",NULL);
      EXPECT_TRUE(NULL != mFileHandle);
      unzClose(mFileHandle);
    }

    void zipOpen2FileHandlerTest()
    {
      mFileHandle = unzOpen2("supportfiles/ziptest.zip",&customdef);
      EXPECT_TRUE(NULL == mFileHandle);
    }

    void zipOpen2_64NoFileHandlerTest()
    {
      mFileHandle = unzOpen2_64("supportfiles/ziptest.zip",NULL);
      EXPECT_TRUE(NULL != mFileHandle);
      unzClose(mFileHandle);
    }

    void zipOpen2_64FileHandlerTest()
    {
      mFileHandle = unzOpen2_64("supportfiles/ziptest.zip",&custom64def);
      EXPECT_TRUE(NULL == mFileHandle);
    }

    void zipCloseTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE(UNZ_OK == unzClose(mFileHandle));
    }

    void zipCloseFailedTest()
    {
      EXPECT_TRUE(UNZ_OK != unzClose(NULL));
    }

    void zipCompareCaseSensitiveTest()
    {
      EXPECT_TRUE (0 == unzStringFileNameCompare("supportfiles/ziptest.zip","supportfiles/ziptest.zip",1));
    }

    void zipCompareCaseInsensitiveTest()
    {
      EXPECT_TRUE (0 == unzStringFileNameCompare("supportfiles/ziptest.zip","SUPPORTFILES/ziptest.zip",2));
    }

    void zipCompareCaseSensitiveDefaultValTest()
    {
      EXPECT_TRUE (0 == unzStringFileNameCompare("supportfiles/ziptest.zip","supportfiles/ziptest.zip",0));
    }

    void zipGetGlobalInfoTest()
    {
      unz_global_info info;
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzGetGlobalInfo(mFileHandle, &info));
      EXPECT_TRUE(info.number_entry == 3); 
      unzClose(mFileHandle);
    }

    void zipGetGlobalInfoFailedTest()
    {
      unz_global_info info;
      EXPECT_TRUE (UNZ_OK != unzGetGlobalInfo(NULL, &info));
    }

    void zipGetGlobalInfo64FailedTest()
    {
      unz_global_info64 info;
      EXPECT_TRUE (UNZ_OK != unzGetGlobalInfo64(NULL, &info));
    }

    void zipGetGlobalInfo64Test()
    {
      unz_global_info64 info;
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzGetGlobalInfo64(mFileHandle, &info));
      EXPECT_TRUE(info.number_entry == 3); 
      unzClose(mFileHandle);
    }

    void zipGetGlobalCommentTest()
    {
      char buffer[1000];
      memset(buffer, 0, sizeof(buffer));
      int nbytes = 0;
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (0 == unzGetGlobalComment(mFileHandle, buffer, sizeof(buffer)));
      unzClose(mFileHandle);
    }

    void zipGoToFirstFileTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzGoToFirstFile(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGoToFirstFileFailureTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGoToFirstFile(NULL));
    }

    void zipGoToNextFileTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzGoToNextFile(mFileHandle));
      EXPECT_TRUE (UNZ_OK == unzGoToNextFile(mFileHandle));
      EXPECT_TRUE (UNZ_END_OF_LIST_OF_FILE == unzGoToNextFile(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGoToNextFileFailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGoToNextFile(NULL));
    }

    void zipLocateFilePresentTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzLocateFile(mFileHandle,"ziptest/",0));
      EXPECT_TRUE (UNZ_OK == unzLocateFile(mFileHandle,"ziptest/file1",1));
      EXPECT_TRUE (UNZ_OK == unzLocateFile(mFileHandle,"ziptest/file2",2));
      unzClose(mFileHandle);
    }

    void zipLocateFileNotPresentTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_END_OF_LIST_OF_FILE == unzLocateFile(mFileHandle,"filenotthere",0));
      unzClose(mFileHandle);
    }

    void zipLocateFileNullTest()
    {
      mFileHandle = unzOpen("supportfiles/filenotthere.zip");
      EXPECT_TRUE (UNZ_PARAMERROR == unzLocateFile(mFileHandle,"ziptest/",0));
    }

    void zipLocateFileMoreLengthTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      char searchFile[300];
      for (int i=0;i<298;i++)
      {
        searchFile[i] = 'a';
      }
      searchFile[299] = '\0';
      EXPECT_TRUE (UNZ_PARAMERROR == unzLocateFile(mFileHandle,searchFile,0));
      unzClose(mFileHandle);
    }

    void zipGetCurrentFileInfoTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      unz_file_info fileinfo;
      char filename[1000];
      memset(filename, 0, sizeof(filename));
      char extrafield[1000];
      memset(extrafield, 0, sizeof(extrafield));
      char comment[1000];
      memset(comment, 0, sizeof(comment));
      EXPECT_TRUE (UNZ_OK == unzGetCurrentFileInfo(mFileHandle,&fileinfo, filename, sizeof(filename), extrafield, sizeof(extrafield), comment, sizeof(comment)));
      EXPECT_TRUE(filename[0] != '\0');
      unzClose(mFileHandle);
    }

    void zipGetCurrentFileInfo64Test()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      unz_file_info64 fileinfo;
      char filename[1000];
      memset(filename, 0, sizeof(filename));
      char extrafield[1000];
      memset(extrafield, 0, sizeof(extrafield));
      char comment[1000];
      memset(comment, 0, sizeof(comment));
      EXPECT_TRUE (UNZ_OK == unzGetCurrentFileInfo64(mFileHandle,&fileinfo, filename, sizeof(filename), extrafield, sizeof(extrafield), comment, sizeof(comment)));
      EXPECT_TRUE(filename[0] != '\0');
      unzClose(mFileHandle);
    }

    void zipOpenCurrentFileTest()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzOpenCurrentFile(mFileHandle));
      unzCloseCurrentFile(mFileHandle);
      unzClose(mFileHandle);
    }

    void zipReadCurrentFileTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      char* files[] = {"ziptest/file1", "ziptest/file2" };
      unz_global_info global_info;
      EXPECT_TRUE (UNZ_OK == unzGetGlobalInfo( mFileHandle, &global_info ));
      char read_buffer[2048];
      uLong i;
      for ( i = 0; i < global_info.number_entry; ++i )
      {
          // Get info about current file.
          unz_file_info file_info;
          char filename[100];
          unzGetCurrentFileInfo(mFileHandle,&file_info,filename,100,NULL,0,NULL,0);
          // Check if this entry is a directory or file.
          const size_t filename_length = strlen( filename );
          if ( filename[ filename_length-1 ] == '/' )
          {
              printf( "dir:%s\n", filename );
              fflush(stdout);
          }
          else
          {
              // Entry is a file, so extract it.
              printf( "file:%s\n", filename );
              fflush(stdout);
              if ( unzOpenCurrentFile( mFileHandle ) != UNZ_OK )
              {
                  printf( "could not open file\n" );
                  fflush(stdout);
                  unzClose(mFileHandle);
                  break;
              }
 	      EXPECT_TRUE(0 < unzReadCurrentFile( mFileHandle, read_buffer, 2048 )); 
          }
  
          unzCloseCurrentFile( mFileHandle );
  
          // Go the the next entry listed in the zip file.
          if ( ( i+1 ) < global_info.number_entry )
          {
              if ( unzGoToNextFile( mFileHandle ) != UNZ_OK )
              {
                  printf( "cound not read next file\n" );
                  unzClose( mFileHandle );
                  break;
              }
          }
      }
      unzClose( mFileHandle );
    }

    void zipOpenCurrentFileWithPwdTest()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest_comp.zip");
      EXPECT_TRUE (NULL != mFileHandle);
      EXPECT_TRUE (UNZ_OK != unzOpenCurrentFilePassword(mFileHandle,"password"));
      unzClose(mFileHandle);
    }
  
    void zipCloseCurrentFileTest()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzOpenCurrentFile(mFileHandle));
      EXPECT_TRUE (UNZ_OK == unzCloseCurrentFile(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGetCurrentStreamFilePosTest()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_OK == unzOpenCurrentFile(mFileHandle));
      EXPECT_TRUE (0 != unzGetCurrentFileZStreamPos64(mFileHandle));
      EXPECT_TRUE (UNZ_OK == unzCloseCurrentFile(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGetCurrentStreamFilePosFailTest()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      EXPECT_TRUE (0 == unzGetCurrentFileZStreamPos64(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGetOffsetFailedTest()
    {
      uLong offset = unzGetOffset(NULL);
      EXPECT_TRUE (offset == 0);
    }

    void zipGetOffsetTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      uLong offset = unzGetOffset(mFileHandle);
      EXPECT_TRUE (offset > 0);
      unzClose(mFileHandle);
    }

    void zipSetOffsetTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE ( UNZ_OK != unzSetOffset(mFileHandle, 0));
      unzClose(mFileHandle);
    }

    void zipSetOffset64FailedTest()
    {
      EXPECT_TRUE ( UNZ_OK != unzSetOffset64(NULL,0));
    }

    void zipGetOffset64Test()
    {
      mFileHandle = unzOpen64("supportfiles/ziptest.zip");
      uLong offset = unzGetOffset64(mFileHandle);
      EXPECT_TRUE (offset > 0);
      unzClose(mFileHandle);
    }

    void zipGetFilePosTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      unz_file_pos file_pos;
      EXPECT_TRUE (UNZ_OK == unzGetFilePos(mFileHandle, &file_pos));
      unzClose(mFileHandle);
    }

    void zipGetFilePos64FailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGetFilePos64(NULL, NULL));
    }

    void zipGetLocalExtrafieldTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      unzOpenCurrentFile(mFileHandle);
      char buffer[2048];
      EXPECT_TRUE (0 < unzGetLocalExtrafield(mFileHandle, buffer, 2048));
      EXPECT_TRUE (UNZ_OK == unzCloseCurrentFile(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipGoToFilePosTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      unz_file_pos file_pos;
      file_pos.pos_in_zip_directory = 0;
      file_pos.num_of_file = 0;
      EXPECT_TRUE (UNZ_OK != unzGoToFilePos(mFileHandle, &file_pos));
      unzClose(mFileHandle);
    }

    void zipTellTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_PARAMERROR == unztell(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipTellFailTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unztell(NULL));
    }

    void zipTell64Test()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_PARAMERROR != unztell64(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipTell64FailTest()
    {
      EXPECT_TRUE (-1 == unztell64(NULL));
    }

    void zipEofTest()
    {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (UNZ_PARAMERROR == unzeof(mFileHandle));
      unzClose(mFileHandle);
    }

    void zipEofFailTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzeof(NULL));
    }

    void zipunzGoToFilePos64FailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGoToFilePos64(NULL, NULL));
    }

    void zipunzOpenCurrentFile3FailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzOpenCurrentFile3(NULL, NULL, NULL, 0, NULL));
    }

    void zipunzOpenCurrentFile2FailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzOpenCurrentFile2(NULL, NULL, NULL, 0));
    }

    void zipunzGetCurrentFileZStreamPos64FailedTest()
    {
      EXPECT_TRUE (0 == unzGetCurrentFileZStreamPos64(NULL));
    }

    void zipunzGetLocalExtrafieldFailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGetLocalExtrafield(NULL, NULL, 0));
    }

    void zipunzCloseCurrentFileFailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzCloseCurrentFile(NULL));
    }

    void zipunzGetGlobalCommentFailedTest()
    {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGetGlobalComment(NULL, NULL, 0));
    }

    void zipunzGetOffset64FailedTest()
    {
      EXPECT_TRUE (0 == unzGetOffset64(NULL));
    }

   void zipzipunzGoToFilePosNullPosFailedTest()
   {
      EXPECT_TRUE (UNZ_PARAMERROR == unzGoToFilePos(NULL,NULL));
   }

   void zipunzReadCurrentFileFailedTest()
   {
      EXPECT_TRUE (UNZ_PARAMERROR == unzReadCurrentFile(NULL,NULL,0));
   }

   void zipunzGetLocalExtrafieldNullBufferTest()
   {
      mFileHandle = unzOpen("supportfiles/ziptest.zip");
      EXPECT_TRUE (0 > unzGetLocalExtrafield(mFileHandle, NULL, 0));
      unzClose(mFileHandle);
   }

   void zipunzReadCurrentFileWrongZipTest()
   {
      mFileHandle = unzOpen("supportfiles/nothing.zip");
      char buf[1000];
      memset(buf, 0, 1000);
      unzReadCurrentFile(mFileHandle, buf,1000);
      unzClose(mFileHandle);
   }

   void zipunzeofTrueTest()
   {
      mFileHandle = unzOpen("supportfiles/empty.zip");
      EXPECT_TRUE (1 != unzeof(mFileHandle)); 
      unzClose(mFileHandle);
   }

   private:
     unzFile mFileHandle;
};

TEST_F(unzipTest, unzipTests)
{
  zipOpenTest();
  zipOpenFailedTest();
  zipOpen64Test();
  zipOpen2NoFileHandlerTest();
  zipOpen2FileHandlerTest();
  zipOpen2_64NoFileHandlerTest();
  zipOpen2_64FileHandlerTest();
  zipCloseTest();
  zipCloseFailedTest();
  zipCompareCaseSensitiveTest();
  zipCompareCaseInsensitiveTest();
  zipCompareCaseSensitiveDefaultValTest();
  zipGetGlobalInfoTest();
  zipGetGlobalInfo64Test();
  zipGetGlobalCommentTest();
  zipGoToFirstFileTest();
  zipGoToFirstFileFailureTest();
  zipGoToNextFileTest();
  zipGoToNextFileFailedTest();
  zipLocateFilePresentTest();
  zipLocateFileNotPresentTest();
  zipLocateFileNullTest();
  zipLocateFileMoreLengthTest();
  zipGetCurrentFileInfoTest();
  zipGetCurrentFileInfo64Test();
  zipOpenCurrentFileTest();
  zipReadCurrentFileTest();
  zipOpenCurrentFileWithPwdTest();
  zipCloseCurrentFileTest();
  zipGetCurrentStreamFilePosTest();
  zipGetCurrentStreamFilePosFailTest();
  zipGetOffsetTest();
  zipGetOffsetFailedTest();
  zipSetOffsetTest();
  zipSetOffset64FailedTest();
  zipGetOffset64Test();
  zipGetFilePosTest();
  zipGetLocalExtrafieldTest();
  zipGoToFilePosTest();
  zipGetGlobalInfoFailedTest();
  zipGetGlobalInfo64FailedTest();
  zipTellTest();
  zipTellFailTest();
  zipTell64Test();
  zipTell64FailTest();
  zipEofTest();
  zipEofFailTest();
  zipGetFilePos64FailedTest();
  zipunzGoToFilePos64FailedTest();
  zipunzOpenCurrentFile3FailedTest();
  zipunzOpenCurrentFile2FailedTest();
  zipunzGetCurrentFileZStreamPos64FailedTest();
  zipunzGetLocalExtrafieldFailedTest();
  zipunzCloseCurrentFileFailedTest();
  zipunzGetGlobalCommentFailedTest();
  zipunzGetOffset64FailedTest();
  zipzipunzGoToFilePosNullPosFailedTest();
  zipunzReadCurrentFileFailedTest();
  zipunzGetLocalExtrafieldNullBufferTest();
  //zipunzReadCurrentFileWrongZipTest();
  zipunzeofTrueTest();
}
