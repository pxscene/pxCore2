#include "ioapi.h"

#include "test_includes.h" // Needs to be included last

using namespace std;

class fileTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      fill_fopen_filefunc(&mFuncDef);
    }

    virtual void TearDown()
    {
    }

    void fileOpenTest()
    {
      mFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_CREATE);
      EXPECT_TRUE (NULL != mFileHandle);
    }

    void fileOpenExistingTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_EXISTING);
      EXPECT_TRUE (NULL != lFileHandle);
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void fileOpenReadWriteFilterTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (NULL != lFileHandle);
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void fileOpenFailedTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfilefail", -10);
      EXPECT_TRUE (NULL == lFileHandle);
      lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, NULL, ZLIB_FILEFUNC_MODE_CREATE);
      EXPECT_TRUE (NULL == lFileHandle);
    }

    void fileErrorTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfileerror", ZLIB_FILEFUNC_MODE_CREATE);
      EXPECT_TRUE (0 == mFuncDef.zerror_file(NULL, lFileHandle));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
      EXPECT_TRUE (0 > mFuncDef.zerror_file(NULL, NULL));
    }

    void fileCloseTest()
    {
      EXPECT_TRUE (0 == mFuncDef.zclose_file(NULL, mFileHandle));
    }

    void fileCloseFailedTest()
    {
      EXPECT_TRUE (0 != mFuncDef.zclose_file(NULL, NULL));
    }

    void fileReadTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (NULL != lFileHandle);
      char buffer[100];
      memset(buffer, 0, sizeof(buffer));
      EXPECT_TRUE (0 < mFuncDef.zread_file(NULL, lFileHandle, &buffer, sizeof(buffer)-1));
      EXPECT_TRUE (0 == mFuncDef.zread_file(NULL, NULL, &buffer, sizeof(buffer)-1));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void fileWriteTest()
    {
      char buffer[100];
      memset(buffer, 0, sizeof(buffer));
      strcpy(buffer,"Testing");
      EXPECT_TRUE (0 < mFuncDef.zwrite_file(NULL, mFileHandle, &buffer, sizeof(buffer)-1));
      EXPECT_TRUE (0 == mFuncDef.zwrite_file(NULL, NULL, &buffer, sizeof(buffer)-1));
    }

    void fileTellTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      char buffer[100];
      memset(buffer, 0, sizeof(buffer));
      EXPECT_TRUE (0 < mFuncDef.zread_file(NULL, lFileHandle, &buffer, sizeof(buffer)-1));
      EXPECT_TRUE(0 < mFuncDef.ztell_file(NULL, lFileHandle));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
      EXPECT_TRUE(-1 == mFuncDef.ztell_file(NULL, NULL));
    }

    void fileSeekTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (0 == mFuncDef.zseek_file(NULL, lFileHandle, 1, ZLIB_FILEFUNC_SEEK_SET));
      EXPECT_TRUE (0 > mFuncDef.zseek_file(NULL, lFileHandle, -1, ZLIB_FILEFUNC_SEEK_SET));
      EXPECT_TRUE (0 == mFuncDef.zseek_file(NULL, lFileHandle, 2, ZLIB_FILEFUNC_SEEK_END));
      EXPECT_TRUE (0 == mFuncDef.zseek_file(NULL, lFileHandle, 1, ZLIB_FILEFUNC_SEEK_CUR));
      EXPECT_TRUE (-1 == mFuncDef.zseek_file(NULL, lFileHandle, 1, -1));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
      EXPECT_TRUE (-1 == mFuncDef.zseek_file(NULL, NULL, 1, ZLIB_FILEFUNC_SEEK_SET));
    }

   private:
     zlib_filefunc_def mFuncDef;
     FILE*  mFileHandle;
};

TEST_F(fileTest, fileTests)
{
  fileOpenTest();
  fileOpenExistingTest();
  fileOpenReadWriteFilterTest();
  fileOpenFailedTest();
  fileErrorTest();
  fileWriteTest();
  fileCloseTest();
  fileCloseFailedTest();
  fileReadTest();
  fileTellTest();
  fileSeekTest();
}

class file64Test : public testing::Test
{
  public:
    virtual void SetUp()
    {
      fill_fopen64_filefunc(&mFuncDef);
    }

    virtual void TearDown()
    {
    }

    void file64OpenTest()
    {
      mFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_CREATE);
      EXPECT_TRUE (NULL != mFileHandle);
    }

    void file64OpenExistingTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_EXISTING);
      EXPECT_TRUE (NULL != lFileHandle);
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void file64OpenReadWriteFilterTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (NULL != lFileHandle);
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void file64OpenFailedTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfilefail", -10);
      EXPECT_TRUE (NULL == lFileHandle);
    }

    void file64CloseTest()
    {
      EXPECT_TRUE (0 == mFuncDef.zclose_file(NULL, mFileHandle));
    }

    void file64ReadTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (NULL != lFileHandle);
      char buffer[100];
      memset(buffer, 0, sizeof(buffer));
      EXPECT_TRUE (0 < mFuncDef.zread_file(NULL, lFileHandle, &buffer, sizeof(buffer)-1));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void file64WriteTest()
    {
      char buffer[100];
      memset(buffer, 0, sizeof(buffer));
      strcpy(buffer,"Testing64");
      EXPECT_TRUE (0 < mFuncDef.zwrite_file(NULL, mFileHandle, &buffer, sizeof(buffer)-1));
    }

    void file64TellTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE(0 == mFuncDef.ztell64_file(NULL, lFileHandle));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
      EXPECT_TRUE((ZPOS64_T)-1 == mFuncDef.ztell64_file(NULL, NULL));
    }

    void file64SeekTest()
    {
      FILE* lFileHandle = (FILE*) mFuncDef.zopen64_file(NULL, "testfile", ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (0 == mFuncDef.zseek64_file(NULL, lFileHandle, 1, ZLIB_FILEFUNC_SEEK_SET));
      EXPECT_TRUE (0 > mFuncDef.zseek64_file(NULL, lFileHandle, -1, ZLIB_FILEFUNC_SEEK_SET));
      EXPECT_TRUE (0 == mFuncDef.zseek64_file(NULL, lFileHandle, 2, ZLIB_FILEFUNC_SEEK_END));
      EXPECT_TRUE (0 == mFuncDef.zseek64_file(NULL, lFileHandle, 1, ZLIB_FILEFUNC_SEEK_CUR));
      EXPECT_TRUE (-1 == mFuncDef.zseek64_file(NULL, lFileHandle, 1, -1));
      int closeret = mFuncDef.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
      EXPECT_TRUE (-1 == mFuncDef.zseek64_file(NULL, NULL, 1, ZLIB_FILEFUNC_SEEK_SET));
    }

   private:
     zlib_filefunc64_def mFuncDef;
     FILE*  mFileHandle;
};

TEST_F(file64Test, file64Tests)
{
  file64OpenTest();
  file64OpenExistingTest();
  file64OpenReadWriteFilterTest();
  file64OpenFailedTest();
  file64WriteTest();
  file64CloseTest();
  file64ReadTest();
  file64TellTest();
  file64SeekTest();
}

class fillFunctionTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
    }

    virtual void TearDown()
    {
    }

    void fillFunTest()
    {
      fill_fopen_filefunc(&mFuncDef);
      EXPECT_TRUE (NULL != mFuncDef.zopen_file);
      EXPECT_TRUE (NULL != mFuncDef.zread_file);
      EXPECT_TRUE (NULL != mFuncDef.zwrite_file);
      EXPECT_TRUE (NULL != mFuncDef.zseek_file);
      EXPECT_TRUE (NULL != mFuncDef.ztell_file);
      EXPECT_TRUE (NULL != mFuncDef.zclose_file);
      EXPECT_TRUE (NULL != mFuncDef.zerror_file);
      EXPECT_TRUE (NULL == mFuncDef.opaque);
    }

    void fill64FunTest()
    {
      fill_fopen64_filefunc(&mFunc64Def);
      EXPECT_TRUE (NULL != mFunc64Def.zopen64_file);
      EXPECT_TRUE (NULL != mFunc64Def.zread_file);
      EXPECT_TRUE (NULL != mFunc64Def.zwrite_file);
      EXPECT_TRUE (NULL != mFunc64Def.zseek64_file);
      EXPECT_TRUE (NULL != mFunc64Def.ztell64_file);
      EXPECT_TRUE (NULL != mFunc64Def.zclose_file);
      EXPECT_TRUE (NULL != mFunc64Def.zerror_file);
      EXPECT_TRUE (NULL == mFunc64Def.opaque);
    }

    void fill64_32_FunFrom32Test()
    {
      fill_zlib_filefunc64_32_def_from_filefunc32(&mFunc32Def,&mFuncDef);
      EXPECT_TRUE (NULL == mFunc32Def.zfile_func64.zopen64_file);
      EXPECT_TRUE (NULL != mFunc32Def.zfile_func64.zread_file);
      EXPECT_TRUE (NULL != mFunc32Def.zfile_func64.zwrite_file);
      EXPECT_TRUE (NULL == mFunc32Def.zfile_func64.zseek64_file);
      EXPECT_TRUE (NULL == mFunc32Def.zfile_func64.ztell64_file);
      EXPECT_TRUE (NULL != mFunc32Def.zfile_func64.zclose_file);
      EXPECT_TRUE (NULL != mFunc32Def.zfile_func64.zerror_file);
      EXPECT_TRUE (NULL == mFunc32Def.zfile_func64.opaque);
      EXPECT_TRUE (NULL != mFunc32Def.zseek32_file);
      EXPECT_TRUE (NULL != mFunc32Def.ztell32_file);
      EXPECT_TRUE (NULL != mFunc32Def.zopen32_file);
    }
   private:
     zlib_filefunc_def mFuncDef;
     zlib_filefunc64_def mFunc64Def;
     zlib_filefunc64_32_def mFunc32Def;
};

TEST_F(fillFunctionTest, fillFunctionTests)
{
  fillFunTest();
  fill64FunTest();
  fill64_32_FunFrom32Test();
}

class globalFunctionsTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      fill_fopen_filefunc(&mFuncDef);
      fill_zlib_filefunc64_32_def_from_filefunc32(&mFunc32Def,&mFuncDef);
    }

    virtual void TearDown()
    {
    }

    void openTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile", ZLIB_FILEFUNC_MODE_CREATE);
      EXPECT_TRUE (NULL != lFileHandle);
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void seekSuccessTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile",  ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (0 == call_zseek64(&mFunc32Def, lFileHandle, 1, ZLIB_FILEFUNC_SEEK_SET));
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void seekOffsetFailTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile",  ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (-1 == call_zseek64(&mFunc32Def, lFileHandle, -1, ZLIB_FILEFUNC_SEEK_SET));
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void seekFnUnavailableTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile",  ZLIB_FILEFUNC_MODE_READ);
      mFunc32Def.zseek32_file = NULL;
      EXPECT_TRUE (-1 == call_zseek64(&mFunc32Def, lFileHandle, -1, ZLIB_FILEFUNC_SEEK_SET));
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void tellSuccessTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile",  ZLIB_FILEFUNC_MODE_READ);
      EXPECT_TRUE (0 == call_ztell64(&mFunc32Def, lFileHandle));
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

    void tellFnUnavailableTest()
    {
      FILE* lFileHandle = (FILE*) call_zopen64(&mFunc32Def, "testfile",  ZLIB_FILEFUNC_MODE_READ);
      mFunc32Def.ztell32_file = NULL;
      EXPECT_TRUE ((unsigned long)-1 == call_ztell64(&mFunc32Def, lFileHandle));
      int closeret = mFunc32Def.zfile_func64.zclose_file(NULL, lFileHandle);
      EXPECT_TRUE (0 == closeret);
    }

   private:
     zlib_filefunc_def mFuncDef;
     zlib_filefunc64_32_def mFunc32Def;
};

TEST_F(globalFunctionsTest, globalFunctionsTests)
{
  openTest();
  seekSuccessTest();
  seekOffsetFailTest();
  seekFnUnavailableTest();
  tellSuccessTest();
  tellFnUnavailableTest();
}
