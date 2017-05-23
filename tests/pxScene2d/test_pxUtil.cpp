#include <list>

#define private public
#define protected public

#include "pxResource.h"
#include "rtFileDownloader.h"
#include "rtString.h"
#include <string.h>
#include <unistd.h>
#include <pxOffscreen.h>
#include <pxUtil.h>
#include <pxCore.h>
#include <dlfcn.h>
#include <png.h>

#include "test_includes.h" // Needs to be included last

using namespace std;
bool failPngCreateWriteStruct = false;
bool failPngCreateReadStruct = false;
bool failPngCreateInfoStruct = false;

extern bool pxIsPNGImage(const char*, size_t);
extern bool pxIsJPGImage(const char*, size_t);
extern rtError pxStoreJPGImage(char * , pxBuffer &);

typedef png_structp (*png_create_write_struct_t)(png_const_charp , png_voidp , png_error_ptr, png_error_ptr);
typedef png_structp (*png_create_read_struct_t)(png_const_charp , png_voidp , png_error_ptr, png_error_ptr);
typedef png_infop (*png_create_info_struct_t)(png_structp);

png_structp png_create_write_struct(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn)
{
  static png_create_write_struct_t png_create_write_struct_p = (png_create_write_struct_t) dlsym(RTLD_NEXT, "png_create_write_struct");
  if (true == failPngCreateWriteStruct)
  {
    return NULL;
  }
  else
  {
    if (NULL != png_create_write_struct_p)
    {
      return png_create_write_struct_p(user_png_ver,error_ptr,error_fn,warn_fn);
    }
  }
}

png_structp png_create_read_struct(png_const_charp user_png_ver, png_voidp error_ptr, png_error_ptr error_fn, png_error_ptr warn_fn)
{
  static png_create_read_struct_t png_create_read_struct_p = (png_create_read_struct_t) dlsym(RTLD_NEXT, "png_create_read_struct");
  if (true == failPngCreateReadStruct)
  {
    return NULL;
  }
  else
  {
    if (NULL != png_create_read_struct_p)
    {
      return png_create_read_struct_p(user_png_ver,error_ptr,error_fn,warn_fn);
    }
  }
}

png_infop png_create_info_struct(png_structp png_ptr)
{
  static png_create_info_struct_t png_create_info_struct_p = (png_create_info_struct_t) dlsym(RTLD_NEXT, "png_create_info_struct");
  if (true == failPngCreateInfoStruct)
  {
    return NULL;
  }
  else
  {
    if (NULL != png_create_info_struct_p)
    {
      return png_create_info_struct_p(png_ptr);
    }
  }
}

class pxUtilTest : public testing::Test
{
  public:
    virtual void SetUp()
    {
      bool sysRet = system("wget http://apng.onevcat.com/assets/elephant.png");
      mDownloadImageFailed = sysRet;
      if (false == mDownloadImageFailed)
        sysRet = system("mv elephant.png supportfiles/apngimage.png");
      else
      {
        printf("WARNING !!!! apngimage not downloaded, so some test cases may not execute\n");
        fflush(stdout);
      }
    }

    virtual void TearDown()
    {
      bool sysRet;
      if (false == mDownloadImageFailed)
      {
        sysRet = system("rm -rf supportfiles/apngimage.png");
      }
      sysRet = system("rm -rf supportfiles/wrong.png supportfiles/generatedflower.png");
      mDownloadImageFailed = true;
    }

    void pxLoadImage3ArgsPngSuccessTest ()
    {
      rtData d;
      rtError loadImageSuccess = rtLoadFile("supportfiles/status_bg.png", d);
      rtError ret = pxLoadImage((const char*) d.data(), d.length(), mPngData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadImage3ArgsPngFailureJpegTurboSuccessTest ()
    {
      rtData d;
      rtError loadImageSuccess = rtLoadFile("supportfiles/sampleimage.jpeg", d);
      rtError ret = pxLoadImage((const char*) d.data(), d.length(), mJpegTurboData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadImage3ArgsFailureTest ()
    {
      pxOffscreen o;
      rtError ret = pxLoadImage(NULL, 0, o);
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxLoadImage3ArgsLessLengthFailureTest ()
    {
      pxOffscreen o;
      rtData d;
      d.init((uint8_t*)"abcde",5);
      rtError ret = pxLoadImage((const char*) d.data(), d.length(), mPngData);
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxLoadImage2ArgsPngSuccessTest ()
    {
      pxOffscreen o;
      rtError ret = pxLoadImage("supportfiles/status_bg.png", o);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadImage2ArgsSuccessTest ()
    {
      pxOffscreen o;
      rtError ret = pxLoadImage("supportfiles/sampleimage.jpeg", o);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadImage2ArgsFailureTest ()
    {
      pxOffscreen o;
      rtError ret = pxLoadImage("supportfiles1/sampleimage.jpeg", o);
      EXPECT_TRUE (ret != RT_OK);
    }

    void pxStoreImageSuccessTest ()
    {
      rtError ret = pxStoreImage("supportfiles/generatedflower.png", mPngData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxStoreImageFailureTest ()
    {
      pxOffscreen o;
      rtError ret = pxStoreImage("supportfiles/wrong.png", o);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxStoreJPGImageTest()
    {
      //todo when pxStoreJPGImage is implemented
      pxOffscreen b;
      EXPECT_TRUE(RT_FAIL == pxStoreJPGImage(NULL, b));
    }

    void pxStorePngImageToDataSuccessTest ()
    {
      rtData d;
      rtError ret = pxStorePNGImage(mPngData,d);
      EXPECT_TRUE (ret == RT_OK);
      EXPECT_TRUE (d.length() > 0);
    }

    void pxStorePngImageCreateWriteStructFailTest ()
    {
      pxOffscreen o;
      failPngCreateWriteStruct = true;
      rtError ret = pxStorePNGImage("supportfiles/wrong.png", o, false, false);
      failPngCreateWriteStruct = false;
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxStorePngImageCreateInfoStructFailTest ()
    {
      pxOffscreen o;
      failPngCreateInfoStruct = true;
      rtError ret = pxStorePNGImage("supportfiles/wrong.png", o, false, false);
      failPngCreateInfoStruct = false;
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxStorePngImagertdataCreateWriteStructFailTest ()
    {
      rtData d;
      failPngCreateWriteStruct = true;
      rtError ret = pxStorePNGImage(mPngData, d);
      failPngCreateWriteStruct = false;
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxStorePngImagertdataCreateInfoStructFailTest ()
    {
      rtData d;
      failPngCreateInfoStruct = true;
      rtError ret = pxStorePNGImage(mPngData, d);
      failPngCreateInfoStruct = false;
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxLoadPNGImage3ArgsCreateReadStructFailTest()
    {
      rtData d;
      rtError loadImageSuccess = rtLoadFile("supportfiles/status_bg.png", d);
      failPngCreateReadStruct = true;
      rtError ret = pxLoadPNGImage((const char*)d.data(), d.length(), mPngData);
      failPngCreateReadStruct = false;
      EXPECT_TRUE (ret == RT_FAIL);
    }

    void pxLoadPNGImage2ArgsSuccessTest()
    {
      rtError ret = pxLoadPNGImage("supportfiles/status_bg.png", mPngData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadPNGImage2ArgsFailureTest()
    {
      rtError ret = pxLoadPNGImage("supportfiles1/status_bg.png", mPngData);
      EXPECT_TRUE (ret != RT_OK);
    }

    void pxLoadJPGImage3ArgsSuccessTest()
    {
      rtData d;
      rtError loadImageSuccess = rtLoadFile("supportfiles/sampleimage.jpeg", d);
      rtError ret = pxLoadJPGImage((const char*)d.data(), d.length(), mJpegData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadJPGImage2ArgsSuccessTest()
    {
      rtError ret = pxLoadJPGImage("supportfiles/sampleimage.jpeg", mJpegTurboData);
      EXPECT_TRUE (ret == RT_OK);
    }

    void pxLoadJPGImage2ArgsFailureTest()
    {
      rtError ret = pxLoadJPGImage("supportfiles/status_bg", mJpegData);
      EXPECT_TRUE (ret != RT_OK);
    }

    void pxLoadAPngSuccessTest ()
    {
      if (false == mDownloadImageFailed)
      {
        rtData d;
        rtError loadImageSuccess = rtLoadFile("supportfiles/apngimage.png", d);
        rtError ret = pxLoadAImage((const char*) d.data(), d.length(), mAnimatedPngData);
        EXPECT_TRUE (ret == RT_OK);
      }
    }

    void pxLoadAPngFailureTest ()
    {
      pxTimedOffscreenSequence o;
      rtError ret = pxLoadAImage(NULL, 0, o);
      EXPECT_TRUE (ret != RT_OK);
    }

    void pxLoadAPNGImage2ArgsSmallImageLengthTest()
    {
      pxTimedOffscreenSequence o;
      rtError ret = pxLoadAPNGImage("abcde", 5, o);
      EXPECT_TRUE (ret != RT_OK);
    }

    void pxIsPngImageTest ()
    {
      // todo when pxIsPNGImage is implemented
      bool ret = pxIsPNGImage(NULL,0);
      EXPECT_TRUE (ret == true);
    }

    void pxIsJpgImageTest ()
    {
      // todo when pxIsJPGImage is implemented
      bool ret = pxIsJPGImage(NULL,0);
      EXPECT_TRUE (ret == false);
    }

    private:
      pxOffscreen mPngData;
      pxOffscreen mJpegTurboData;
      pxOffscreen mJpegData;
      pxTimedOffscreenSequence mAnimatedPngData;
      bool mDownloadImageFailed;
};

TEST_F(pxUtilTest, pxutilsTest)
{
    pxLoadImage3ArgsPngSuccessTest();
    pxLoadImage3ArgsPngFailureJpegTurboSuccessTest();
    pxLoadImage3ArgsFailureTest();
    pxLoadImage3ArgsLessLengthFailureTest();

    pxLoadImage2ArgsSuccessTest();
    pxLoadImage2ArgsFailureTest();

    pxLoadPNGImage2ArgsSuccessTest();
    pxLoadPNGImage2ArgsFailureTest();
    pxLoadPNGImage3ArgsCreateReadStructFailTest();

    pxLoadJPGImage3ArgsSuccessTest();

    pxLoadJPGImage2ArgsSuccessTest();
    pxLoadJPGImage2ArgsFailureTest();

    pxStoreImageSuccessTest();
    pxStoreImageFailureTest();
    pxStoreJPGImageTest();

    pxStorePngImageToDataSuccessTest();
    pxStorePngImageCreateWriteStructFailTest();
    //pxStorePngImageCreateInfoStructFailTest();

    pxStorePngImagertdataCreateWriteStructFailTest();
    //pxStorePngImagertdataCreateInfoStructFailTest();

    pxLoadAPngSuccessTest();
    pxLoadAPngFailureTest();

    pxLoadAPNGImage2ArgsSmallImageLengthTest();

    pxIsPngImageTest();
    pxIsJpgImageTest();
};
