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

// pxUtil.cpp

#ifndef WIN32
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>

#include "rtLog.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"

#include <openssl/md5.h>

#define SUPPORT_PNG
#define SUPPORT_JPG

#include "rtRef.h"
#include "rtObject.h"

  #include <stdio.h>
  #include <string.h>
  #include <float.h>

  //#define STB_IMAGE_WRITE_IMPLEMENTATION
  //#include "stb_image_write.h"
  #define NANOSVG_ALL_COLOR_KEYWORDS  // Include full list of color keywords.
  #define NANOSVG_IMPLEMENTATION      // Expands implementation
  #include "nanosvg.h"
  #define NANOSVGRAST_IMPLEMENTATION
  #include "nanosvgrast.h"

#ifdef SAFE_FREE
#undef SAFE_FREE
#endif
#define SAFE_FREE(x)  do { free(x); (x) = NULL; } while(0)

pxImageType getImageType( const uint8_t* data, size_t len ); //fwd

class NSVGrasterizerEx
{
  public:
       NSVGrasterizerEx()  {  rast = nsvgCreateRasterizer(); } // ctor
      ~NSVGrasterizerEx()  {  nsvgDeleteRasterizer(rast);    } // dtor

    NSVGrasterizer *getPtr() { return rast; };

  private:
    NSVGrasterizer *rast;

}; // CLASS;

static NSVGrasterizerEx rast;
static rtMutex          rastMutex;


// Assume alpha is not premultiplied
rtError pxLoadImage(const char *imageData, size_t imageDataSize,  pxOffscreen &o,
                        int32_t w /* = 0    */, int32_t h /* = 0    */,
                         float sx /* = 1.0f */,  float sy /* = 1.0f */)
{
  pxImageType imgType = getImageType( (const uint8_t*) imageData, imageDataSize);
  rtError retVal = RT_FAIL;

  switch(imgType)
  {
    case PX_IMAGE_PNG:
         {
           retVal = pxLoadPNGImage(imageData, imageDataSize, o);
         }
         break;

    case PX_IMAGE_JPG:
         {
#ifdef ENABLE_LIBJPEG_TURBO
           retVal = pxLoadJPGImageTurbo(imageData, imageDataSize, o);
           if (retVal != RT_OK)
           {
             retVal = pxLoadJPGImage(imageData, imageDataSize, o);
           }
#else
        retVal = pxLoadJPGImage(imageData, imageDataSize, o);
#endif //ENABLE_LIBJPEG_TURBO
         }
         break;

    case PX_IMAGE_SVG:
    default:
         {
           retVal = pxLoadSVGImage(imageData, imageDataSize, o, w, h, sx, sy);
         }
         break;
  }//SWITCH

  if (retVal != RT_OK)
  {
    rtLogError("ERROR:  pxLoadImage() - failed" );
    return retVal;
  }


  // TODO more sane image type detection and flow

  if (o.mPixelFormat != RT_DEFAULT_PIX)
  {
    o.swizzleTo(RT_DEFAULT_PIX);
  }

  return retVal;
}

// APNG looks like a PNG with extra chunks ... can fallback to display static PNG

rtError pxLoadAImage(const char* imageData, size_t imageDataSize,
  pxTimedOffscreenSequence &s)
{
  // Load as PNG...
  rtError retVal = pxLoadAPNGImage(imageData, imageDataSize, s);

  if (retVal != RT_OK) // Failed ... trying as JPG (why?)
  {
#if 0
    rtLogError("ERROR:  pxLoadAPNGImage() - failed to load APNG ... " );

    return retVal;
#else
    pxOffscreen o;
#ifdef ENABLE_LIBJPEG_TURBO
    retVal = pxLoadJPGImageTurbo(imageData, imageDataSize, o);
    if (retVal != RT_OK)
    {
      retVal = pxLoadJPGImage(imageData, imageDataSize, o);
    }
#else
    retVal = pxLoadJPGImage(imageData, imageDataSize, o);
#endif //ENABLE_LIBJPEG_TURBO
    s.init();
    s.addBuffer(o,0);
#endif // 0
  }

  return retVal;
}

// TODO Detection needs to be improved...
// Handling jpeg as fallback now
rtError pxLoadImage(const char *filename, pxOffscreen &b,
                        int32_t w /* = 0    */, int32_t h /* = 0    */,
                         float sx /* = 1.0f */,  float sy /* = 1.0f */)
{
  rtData d;
  rtError e = rtLoadFile(filename, d);
  if (e == RT_OK)
    return pxLoadImage((const char *)d.data(), d.length(), b);
  else
  {
    e = RT_RESOURCE_NOT_FOUND;
    rtLogError("Could not load image file %s.", filename);
  }
  return e;
}

rtError pxStoreImage(const char *filename, pxOffscreen &b)
{
  return pxStorePNGImage(filename, b);
}

bool pxIsPNGImage(rtData d)
{
  return (getImageType( (const uint8_t*) d.data(), d.length()) == PX_IMAGE_PNG);
}

bool pxIsPNGImage(const char *imageData, size_t imageDataSize)
{
  return (getImageType( (const uint8_t*) imageData, imageDataSize) == PX_IMAGE_PNG);
}

rtError pxLoadPNGImage(const char *filename, pxOffscreen &o)
{
  rtData d;
  rtError e = rtLoadFile(filename, d);
  if (e == RT_OK)
  {
    // TODO get rid of the cast
    e = pxLoadPNGImage((const char *)d.data(), d.length(), o);
  }
  else
  {
    rtLogError("Failed to load image file, %s.", filename);
  }

  return e;
}

/* structure to store PNG image bytes */
struct mem_encode
{
  char *buffer;
  size_t size;
};

// TODO change this to using rtData more directly
void my_png_write_data(png_structp png_ptr, png_bytep data, png_size_t length)
{
  /* with libpng15 next line causes pointer deference error; use libpng12 */
  struct mem_encode *p = (struct mem_encode *)png_get_io_ptr(png_ptr); /* was png_ptr->io_ptr */
  size_t nsize = p->size + length;

  /* allocate or grow buffer */
  if (p->buffer)
    p->buffer = (char *)realloc(p->buffer, nsize);
  else
    p->buffer = (char *)malloc(nsize);

  if (!p->buffer)
    png_error(png_ptr, "Write Error");

  /* copy new bytes to end of buffer */
  memcpy(p->buffer + p->size, data, length);
  p->size += length;
}

// TODO rewrite this...
rtError pxStorePNGImage(pxOffscreen &b, rtData &pngData)
{
  if (b.mPixelFormat != RT_PIX_RGBA)
  {
    // TODO should not need swizzle... libpng should have
    // have options to deal with this
    b.swizzleTo(RT_PIX_RGBA); // needed for PNG encoder
  }

  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;

  struct mem_encode state;
  state.buffer = NULL;
  state.size = 0;

  {
    // initialize stuff
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
      rtLogError("FATAL: png_create_write_struct() - FAILED");
      return RT_FAIL;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, NULL);
      rtLogError("FATAL: png_create_info_struct() - FAILED");
      return RT_FAIL;
    }

    if (!setjmp(png_jmpbuf(png_ptr)))
    {
      png_set_write_fn(png_ptr, &state, my_png_write_data, NULL);
      // write header
      if (!setjmp(png_jmpbuf(png_ptr)))
      {
#if 0
        png_byte color_type = (grayscale?PNG_COLOR_MASK_ALPHA:0) |
            (alpha?PNG_COLOR_MASK_ALPHA:0);
#endif

        png_set_IHDR(png_ptr, info_ptr, b.width(), b.height(),
                     8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        // setjmp() ... needed for 'libpng' error handling...

        // write bytes
        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * b.height());

          if (row_pointers)
          {
            for (int y = 0; y < b.height(); y++)
            {
              row_pointers[y] = (png_byte *)b.scanline(y);
            }

            png_write_image(png_ptr, row_pointers);

            // end write
            if (!setjmp(png_jmpbuf(png_ptr)))
            {
              png_write_end(png_ptr, NULL);
            }

            free(row_pointers);

          } //ENDIF - row_pointers
        }
      }
    }

    png_destroy_write_struct(&png_ptr, &info_ptr);
  }

  pngData.init((uint8_t *)state.buffer, state.size);

  if (state.buffer)
    free(state.buffer);

  return RT_OK;
}

rtError pxStorePNGImage(const char *filename, pxOffscreen &b, bool /*grayscale*/,
                        bool /*alpha*/)
{
  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep *row_pointers;

  // create file
  rtFilePointer fp(fopen(filename, "wb"));

  if (fp)
  {
    // initialize stuff
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (!png_ptr)
    {
      rtLogError("FATAL: png_create_write_struct() - FAILED");
      return RT_FAIL;
    }

    info_ptr = png_create_info_struct(png_ptr);

    if (!info_ptr)
    {
      png_destroy_write_struct(&png_ptr, NULL);
      rtLogError("FATAL: png_create_info_struct() - FAILED");
      return RT_FAIL;
    }

    if (!setjmp(png_jmpbuf(png_ptr)))
    {
      png_init_io(png_ptr, fp.getPtr());

      // write header
      if (!setjmp(png_jmpbuf(png_ptr)))
      {
#if 0
        png_byte color_type = (grayscale?PNG_COLOR_MASK_ALPHA:0) |
            (alpha?PNG_COLOR_MASK_ALPHA:0);
#endif
        //        png_set_bgr(png_ptr);

        png_set_IHDR(png_ptr, info_ptr, b.width(), b.height(),
                     8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
                     PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

        png_write_info(png_ptr, info_ptr);

        // setjmp() ... needed for 'libpng' error handling...

        // write bytes
        if (!setjmp(png_jmpbuf(png_ptr)))
        {
          row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * b.height());

          if (row_pointers)
          {
            for (int y = 0; y < b.height(); y++)
            {
              row_pointers[y] = (png_byte *)b.scanline(y);
            }

            png_write_image(png_ptr, row_pointers);

            // end write
            if (!setjmp(png_jmpbuf(png_ptr)))
            {
              png_write_end(png_ptr, NULL);
            }

            free(row_pointers);

          } //ENDIF - row_pointers
        }
      }
    }

    png_destroy_write_struct(&png_ptr, &info_ptr);

#if 0
    // cleanup heap allocation
    for (y=0; y<height; y++)
    {
      free(row_pointers[y]);
    }
#endif
    //		free(row_pointers);
  }
  return RT_OK;
}

// JPEG Support

/*
 * example.c
 *
 * This file illustrates how to use the IJG code as a subroutine library
 * to read or write JPEG image files.  You should look at this code in
 * conjunction with the documentation file libjpeg.txt.
 *
 * This code will not do anything useful as-is, but it may be helpful as a
 * skeleton for constructing routines that call the JPEG library.
 *
 * We present these routines in the same coding style used in the JPEG code
 * (ANSI function definitions, etc); but you are of course free to code your
 * routines in a different style if you prefer.
 */

#include <stdio.h>

/*
 * Include file for users of JPEG library.
 * You will need to have included system headers that define at least
 * the typedefs FILE and size_t before you can include jpeglib.h.
 * (stdio.h is sufficient on ANSI-conforming systems.)
 * You may also wish to include "jerror.h".
 */

#include "jpeglib.h"

/*
 * <setjmp.h> is used for the optional error recovery mechanism shown in
 * the second part of the example.
 */

#include <setjmp.h>

#if 0

/******************** JPEG COMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to feed data into the JPEG compressor.
 * We present a minimal version that does not worry about refinements such
 * as error recovery (the JPEG code will just exit() if it gets an error).
 */


/*
 * IMAGE DATA FORMATS:
 *
 * The standard input image format is a rectangular array of pixels, with
 * each pixel having the same number of "component" values (color channels).
 * Each pixel row is an array of JSAMPLEs (which typically are unsigned chars).
 * If you are working with color data, then the color values for each pixel
 * must be adjacent in the row; for example, R,G,B,R,G,B,R,G,B,... for 24-bit
 * RGB color.
 *
 * For this example, we'll assume that this data structure matches the way
 * our application has stored the image in memory, so we can just pass a
 * pointer to our image buffer.  In particular, let's say that the image is
 * RGB color and is described by:
 */

extern JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
extern int image_height;	/* Number of rows in image */
extern int image_width;		/* Number of columns in image */


/*
 * Sample routine for JPEG compression.  We assume that the target file name
 * and a compression quality factor are passed in.
 */

GLOBAL(void)
write_JPEG_file (char * filename, int quality)
{
  /* This struct contains the JPEG compression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   * It is possible to have several such structures, representing multiple
   * compression/decompression processes, in existence at once.  We refer
   * to any one struct (and its associated working data) as a "JPEG object".
   */
  struct jpeg_compress_struct cinfo;
  /* This struct represents a JPEG error handler.  It is declared separately
   * because applications often want to supply a specialized error handler
   * (see the second half of this file for an example).  But here we just
   * take the easy way out and use the standard error handler, which will
   * print a message on stderr and call exit() if compression fails.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct jpeg_error_mgr jerr;
  /* More stuff */
  FILE * outfile;		/* target file */
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  /* Step 1: allocate and initialize JPEG compression object */

  /* We have to set up the error handler first, in case the initialization
   * step fails.  (Unlikely, but it could happen if you are out of memory.)
   * This routine fills in the contents of struct jerr, and returns jerr's
   * address which we place into the link field in cinfo.
   */
  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  /* Step 2: specify data destination (eg, a file) */
  /* Note: steps 2 and 3 can be done in either order. */

  /* Here we use the library-supplied code to send compressed data to a
   * stdio stream.  You can also write your own code to do something else.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to write binary files.
   */
  if ((outfile = fopen(filename, "wb")) == NULL) {
    fprintf(stderr, "can't open %s\n", filename);
    exit(1);
  }
  jpeg_stdio_dest(&cinfo, outfile);

  /* Step 3: set parameters for compression */

  /* First we supply a description of the input image.
   * Four fields of the cinfo struct must be filled in:
   */
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 3;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
  /* Now use the library's routine to set default compression parameters.
   * (You must set at least cinfo.in_color_space before calling this,
   * since the defaults depend on the source color space.)
   */
  jpeg_set_defaults(&cinfo);
  /* Now you can set any non-default parameters you wish to.
   * Here we just illustrate the use of quality (quantization table) scaling:
   */
  jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

  /* Step 4: Start compressor */

  /* TRUE ensures that we will write a complete interchange-JPEG file.
   * Pass TRUE unless you are very sure of what you're doing.
   */
  jpeg_start_compress(&cinfo, TRUE);

  /* Step 5: while (scan lines remain to be written) */
  /*           jpeg_write_scanlines(...); */

  /* Here we use the library's state variable cinfo.next_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   * To keep things simple, we pass one scanline per call; you can pass
   * more if you wish, though.
   */
  row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }

  /* Step 6: Finish compression */

  jpeg_finish_compress(&cinfo);
  /* After finish_compress, we can close the output file. */
  fclose(outfile);

  /* Step 7: release JPEG compression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_compress(&cinfo);

  /* And we're done! */
}


/*
 * SOME FINE POINTS:
 *
 * In the above loop, we ignored the return value of jpeg_write_scanlines,
 * which is the number of scanlines actually written.  We could get away
 * with this because we were only relying on the value of cinfo.next_scanline,
 * which will be incremented correctly.  If you maintain additional loop
 * variables then you should be careful to increment them properly.
 * Actually, for output to a stdio stream you needn't worry, because
 * then jpeg_write_scanlines will write all the lines passed (or else exit
 * with a fatal error).  Partial writes can only occur if you use a data
 * destination module that can demand suspension of the compressor.
 * (If you don't know what that's for, you don't need it.)
 *
 * If the compressor requires full-image buffers (for entropy-coding
 * optimization or a multi-scan JPEG file), it will create temporary
 * files for anything that doesn't fit within the maximum-memory setting.
 * (Note that temp files are NOT needed if you use the default parameters.)
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 *
 * Scanlines MUST be supplied in top-to-bottom order if you want your JPEG
 * files to be compatible with everyone else's.  If you cannot readily read
 * your data in that order, you'll need an intermediate array to hold the
 * image.  See rdtarga.c or rdbmp.c for examples of handling bottom-to-top
 * source data using the JPEG code's internal virtual-array mechanisms.
 */

#endif

/******************** JPEG DECOMPRESSION SAMPLE INTERFACE *******************/

/* This half of the example shows how to read data from the JPEG decompressor.
 * It's a bit more refined than the above, in that we show:
 *   (a) how to modify the JPEG library's standard error-reporting behavior;
 *   (b) how to allocate workspace using the library's memory manager.
 *
 * Just to make this example a little different from the first one, we'll
 * assume that we do not intend to put the whole image into an in-memory
 * buffer, but to send it line-by-line someplace else.  We need a one-
 * scanline-high JSAMPLE array as a work buffer, and we will let the JPEG
 * memory manager allocate it for us.  This approach is actually quite useful
 * because we don't need to remember to deallocate the buffer separately: it
 * will go away automatically when the JPEG object is cleaned up.
 */

/*
 * ERROR HANDLING:
 *
 * The JPEG library's standard error handler (jerror.c) is divided into
 * several "methods" which you can override individually.  This lets you
 * adjust the behavior without duplicating a lot of code, which you might
 * have to update with each future release.
 *
 * Our example here shows how to override the "error_exit" method so that
 * control is returned to the library's caller when a fatal error occurs,
 * rather than calling exit() as the standard error_exit method does.
 *
 * We use C's setjmp/longjmp facility to return control.  This means that the
 * routine which calls the JPEG library must first execute a setjmp() call to
 * establish the return point.  We want the replacement error_exit to do a
 * longjmp().  But we need to make the setjmp buffer accessible to the
 * error_exit routine.  To do this, we make a private extension of the
 * standard JPEG error handler object.  (If we were using C++, we'd say we
 * were making a subclass of the regular error handler.)
 *
 * Here's the extended error handler struct:
 */

struct my_error_mgr
{
  struct jpeg_error_mgr pub; /* "public" fields */

  jmp_buf setjmp_buffer; /* for return to caller */
};

typedef struct my_error_mgr *my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit(j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr)cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message)(cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

/*
 * SOME FINE POINTS:
 *
 * In the above code, we ignored the return value of jpeg_read_scanlines,
 * which is the number of scanlines actually read.  We could get away with
 * this because we asked for only one line at a time and we weren't using
 * a suspending data source.  See libjpeg.txt for more info.
 *
 * We cheated a bit by calling alloc_sarray() after jpeg_start_decompress();
 * we should have done it beforehand to ensure that the space would be
 * counted against the JPEG max_memory setting.  In some systems the above
 * code would risk an out-of-memory error.  However in general we don't
 * know the output image dimensions before jpeg_start_decompress(), unless we
 * call jpeg_calc_output_dimensions().  See libjpeg.txt for more about this.
 *
 * Scanlines are returned in the same order as they appear in the JPEG file,
 * which is standardly top-to-bottom.  If you must emit data bottom-to-top,
 * you can use one of the virtual arrays provided by the JPEG memory manager
 * to invert the data.  See wrbmp.c for an example.
 *
 * As with compression, some operating modes may require temporary files.
 * On some systems you may need to set up a signal handler to ensure that
 * temporary files are deleted if the program is interrupted.  See libjpeg.txt.
 */

bool pxIsJPGImage(const char *imageData, size_t imageDataSize)
{
  return (getImageType( (const uint8_t*) imageData, imageDataSize) == PX_IMAGE_JPG);
}

rtError pxLoadJPGImage(const char *filename, pxOffscreen &o)
{
  rtData d;
  rtError e = rtLoadFile(filename, d);
  if (e == RT_OK)
  {
#ifdef ENABLE_LIBJPEG_TURBO
    rtError retVal = pxLoadJPGImageTurbo((const char *)d.data(), d.length(), o);
    if (retVal != RT_OK)
    {
      retVal = pxLoadJPGImage((const char *)d.data(), d.length(), o);
    }
    return retVal;
#else
    return pxLoadJPGImage((const char *)d.data(), d.length(), o);
#endif //ENABLE_LIBJPEG_TURBO
  }
  else
    rtLogError("Could not load JPG file: %s", filename);

  return e;
}

#ifdef ENABLE_LIBJPEG_TURBO
extern "C" {
#include <turbojpeg.h>
}

rtError pxLoadJPGImageTurbo(const char *buf, size_t buflen, pxOffscreen &o)
{
  rtLogDebug("using pxLoadJPGImageTurbo");
  if (!buf)
  {
    rtLogError("NULL buffer passed into pxLoadJPGImageTurbo");
    return RT_FAIL;
  }

  tjhandle jpegDecompressor = tjInitDecompress();

  int width, height, jpegSubsamp, jpegColorspace;

  tjDecompressHeader3(jpegDecompressor, (unsigned char *)buf, buflen, &width, &height, &jpegSubsamp, &jpegColorspace);

  //int colorComponent = 3;

  if (jpegColorspace == TJCS_GRAY)
  {
    //colorComponent = 1;
    tjDestroy(jpegDecompressor);
    return RT_FAIL;// TODO : add grayscale support for libjpeg turbo.  falling back to libjpeg for now
  }

  // limit memory usage to resolution 4096x4096
  if (((size_t)width * height) > ((size_t)4096 * 4096))
  {
    rtLogError("Error libjpeg-turbo: image too large");
    tjDestroy(jpegDecompressor);
    return RT_FAIL;
  }

  unsigned char *imageBuffer = tjAlloc(width * height * 3);

  if (!imageBuffer)
  {
    rtLogError("Error allocating libjpeg-turbo buffer");
    tjDestroy(jpegDecompressor);
    return RT_FAIL;
  }

  int result = tjDecompress2(jpegDecompressor, (unsigned char *)buf, buflen, imageBuffer, width, 0, height, TJPF_RGB /*(colorComponent == 3) ? TJPF_RGB : jpegColorspace*/, TJFLAG_FASTDCT);

  if (result != 0)
  {
    rtLogError("Error decompressing using libjpeg turbo");
    tjFree(imageBuffer);
    tjDestroy(jpegDecompressor);
    return RT_FAIL;
  }

  o.init(width, height);

  int scanlinen = 0;
  unsigned int bufferIndex = 0;
  while (scanlinen < height)
  {
    pxPixel *p = o.scanline(scanlinen++);
    {
      char *b = (char *)&imageBuffer[bufferIndex];
      char *bend = b + (width * 3);
      while (b < bend)
      {
        p->r = b[0];
        p->g = b[1];
        p->b = b[2];
        p->a = 255;
        b += 3; // next pixel
        bufferIndex += 3;
        p++;
      }
    }
  }

  o.mPixelFormat = RT_PIX_ARGB;

  tjFree(imageBuffer);
  tjDestroy(jpegDecompressor);

  /* And we're done! */
  return RT_OK;
}
#endif //ENABLE_LIBJPEG_TURBO

rtError pxLoadJPGImage(const char *buf, size_t buflen, pxOffscreen &o)
{
  if (!buf)
  {
    rtLogError("NULL buffer passed into pxLoadJPGImage");
    return RT_FAIL;
  }

  /* This struct contains the JPEG decompression parameters and pointers to
   * working space (which is allocated as needed by the JPEG library).
   */
  struct jpeg_decompress_struct cinfo;
  /* We use our private extension JPEG error handler.
   * Note that this struct must live as long as the main JPEG parameter
   * struct, to avoid dangling-pointer problems.
   */
  struct my_error_mgr jerr;
  /* More stuff */
  //  FILE * infile;		/* source file */
  JSAMPARRAY buffer; /* Output row buffer */
  int row_stride;    /* physical row width in output buffer */

  /* In this example we want to open the input file before doing anything else,
   * so that the setjmp() error recovery below can assume the file is open.
   * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
   * requires it in order to read binary files.
   */

  /* Step 1: allocate and initialize JPEG decompression object */

  /* We set up the normal JPEG error routines, then override error_exit. */
  cinfo.err = jpeg_std_error(&jerr.pub);
  jerr.pub.error_exit = my_error_exit;
  /* Establish the setjmp return context for my_error_exit to use. */
  if (setjmp(jerr.setjmp_buffer))
  {
    /* If we get here, the JPEG code has signaled an error.
     * We need to clean up the JPEG object, close the input file, and return.
     */
    jpeg_destroy_decompress(&cinfo);
    //    fclose(infile);
    return RT_FAIL;
  }
  /* Now we can initialize the JPEG decompression object. */
  jpeg_create_decompress(&cinfo);

  /* Step 2: specify data source (eg, a file) */

  jpeg_mem_src(&cinfo, (unsigned char *)buf, buflen);

  /* Step 3: read file parameters with jpeg_read_header() */

  (void)jpeg_read_header(&cinfo, TRUE);
  cinfo.out_color_space = JCS_RGB;

  /* We can ignore the return value from jpeg_read_header since
   *   (a) suspension is not possible with the stdio data source, and
   *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
   * See libjpeg.txt for more info.
   */

  /* Step 4: set parameters for decompression */

  /* In this example, we don't need to change any of the defaults set by
   * jpeg_read_header(), so we do nothing here.
   */

  /* Step 5: Start decompressor */

  (void)jpeg_start_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* We may need to do some setup of our own at this point before reading
   * the data.  After jpeg_start_decompress() we have the correct scaled
   * output image dimensions available, as well as the output colormap
   * if we asked for color quantization.
   * In this example, we need to make an output work buffer of the right size.
   */
  /* JSAMPLEs per row in output buffer */
  row_stride = cinfo.output_width * cinfo.output_components;
  /* Make a one-row-high sample array that will go away when done with image */
  buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr)&cinfo, JPOOL_IMAGE, row_stride, 1);

  o.init(cinfo.output_width, cinfo.output_height);

  /* Step 6: while (scan lines remain to be read) */
  /*           jpeg_read_scanlines(...); */

  /* Here we use the library's state variable cinfo.output_scanline as the
   * loop counter, so that we don't have to keep track ourselves.
   */
  int scanlinen = 0;
  while (cinfo.output_scanline < cinfo.output_height)
  {
    /* jpeg_read_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could ask for
     * more than one scanline at a time if that's more convenient.
     */
    (void)jpeg_read_scanlines(&cinfo, buffer, 1);
    /* Assume put_scanline_someplace wants a pointer and sample count. */

    pxPixel *p = o.scanline(scanlinen++);
    {
      char *b = (char *)buffer[0];
      char *bend = b + (cinfo.output_width * 3);
      while (b < bend)
      {
        p->r = b[0];
        p->g = b[1];
        p->b = b[2];
        p->a = 255;
        b += 3; // next pixel
        p++;
      }
    }
  }

  o.mPixelFormat = RT_PIX_ARGB;

  /* Step 7: Finish decompression */

  (void)jpeg_finish_decompress(&cinfo);
  /* We can ignore the return value since suspension is not possible
   * with the stdio data source.
   */

  /* Step 8: Release JPEG decompression object */

  /* This is an important step since it will release a good deal of memory. */
  jpeg_destroy_decompress(&cinfo);

  /* After finish_decompress, we can close the input file.
   * Here we postpone it until after no more JPEG errors are possible,
   * so as to simplify the setjmp error logic above.  (Actually, I don't
   * think that jpeg_destroy can do an error exit, but why assume anything...)
   */
  //fclose(infile);

  /* At this point you may want to check to see whether any corrupt-data
   * warnings occurred (test whether jerr.pub.num_warnings is nonzero).
   */

  /* And we're done! */
  return RT_OK;
}


rtError pxStoreSVGImage(const char* /*filename*/, pxBuffer& /*b*/)  { return RT_FAIL; } // NOT SUPPORTED


rtError pxLoadSVGImage(const char* buf, size_t buflen, pxOffscreen& o, int  w /* = 0    */,      int h /* = 0    */,
                                                                     float sx /* = 1.0f */,   float sy /* = 1.0f */)
{
  rtMutexLockGuard  autoLock(rastMutex);

  if (rast.getPtr() == NULL)
  {
    rtLogError("SVG:  No rasterizer available \n");
    return RT_FAIL;
  }

  if (buf == NULL || buflen == 0 )
  {
    rtLogError("SVG:  Bad args.\n");
    return RT_FAIL;
  }

  if (sx <= 0.0f || sy <= 0.0f)
  {
    rtLogError("SVG:  Bad image scale  sx: %f  sy: %f\n", sx, sy);
    return RT_FAIL;
  }

  // NOTE:  'nanosvg' is *destructive* to the SVG source buffer
  //
  //        Pass it a copy !
  //
  void *buf_copy = malloc(buflen);
  if (NULL == buf_copy)
  {
    rtLogError("SVG:  Could not create memory for SVG data .\n");
    return RT_FAIL;
  }
  memcpy(buf_copy, buf, buflen);
  
  NSVGimage *image = nsvgParse( (char *) buf_copy, "px", 96.0f); // 96 dpi (suggested default)
  free(buf_copy); // clean-up
  if (image == NULL)
  {
    rtLogError("SVG:  Could not init decode SVG.\n");
    return RT_FAIL;
  }

  int image_w = (int)image->width;  // parsed SVG image dimensions
  int image_h = (int)image->height; // parsed SVG image dimensions

  if (image_w == 0 || image_h == 0)
  {
    nsvgDelete(image);

    rtLogError("SVG:  Bad image dimensions  WxH: %d x %d\n", image_w, image_h);
    return RT_FAIL;
  }

  // Dimensions WxH ... *only* if no Scale XY
  if( (sx == 1.0f && sy == 1.0f) && (w > 0 && h > 0) ) // <<< Use WxH only if no scale. Scale takes precedence
  {
    float ratioW = (float) w / (float) image_w;
    float ratioH = (float) h / (float) image_h;

    sx = sy = (ratioW < ratioH) ? ratioW : ratioH; // MIN()
  }

  o.initWithColor( (image_w * sx), (image_h * sy), pxClear); // default sized

  nsvgRasterizeFull(rast.getPtr(), image, 0, 0, sx, sy,
                    (unsigned char*) o.base(), o.width(), o.height(), o.width() *4);

  nsvgDelete(image);

  return RT_OK;
}


rtError pxLoadSVGImage(const char *filename, pxOffscreen &o, int  w /* = 0    */,      int h /* = 0    */,
                                                           float sx /* = 1.0f */,   float sy /* = 1.0f */)
{
  rtData d;
  rtError e = rtLoadFile(filename, d);
  if (e == RT_OK)
  {
    // TODO get rid of the cast
    e = pxLoadSVGImage((const char *)d.data(), d.length(), o, w, h, sx, sy);
  }
  else
  {
    rtLogError("Failed to load image file, %s.", filename);
  }

  return e;
}

rtError pxStoreJPGImage(char * /*filename*/, pxBuffer & /*b*/)
{
  return RT_FAIL; // NOT SUPPORTED
}

struct PngStruct
{
  PngStruct(char *data, size_t dataSize)
      : imageData(data), imageDataSize(dataSize), readPosition(0)
  {
  }

  char *imageData;
  size_t imageDataSize;
  int readPosition;
};

void readPngData(png_structp pngPtr, png_bytep data, png_size_t length)
{
  png_voidp a = png_get_io_ptr(pngPtr);
  PngStruct *pngStruct = (PngStruct *)a;

  memcpy((char *)data, pngStruct->imageData + pngStruct->readPosition, length);
  pngStruct->readPosition += length;
}

rtError pxLoadPNGImage(const char *imageData, size_t imageDataSize,
                       pxOffscreen &o)
{
  rtError e = RT_FAIL;

  png_structp png_ptr;
  png_infop info_ptr;
  //  int number_of_passes;
  png_bytep *row_pointers;
  PngStruct pngStruct((char *)imageData, imageDataSize);

  if (!imageData)
  {
    rtLogError("FATAL: Invalid arguments - imageData = NULL");
    return e;
  }

  if (imageDataSize < 8)
  {
    rtLogError("FATAL: Invalid arguments - imageDataSize < 8");
    return e;
  }

  unsigned char header[8]; // 8 is the maximum size that can be checked

  // open file and test for it being a png
  memcpy(header, imageData, 8);
  pngStruct.readPosition += 8;

  // test PNG header
  if (png_sig_cmp(header, 0, 8) != 0)
  {
    // TODO Improve Detection of different image types
    // Fail quietly so we fallback to JPG
    //    rtLogError("FATAL: Invalid PNG header");
    return RT_FAIL;
  }

  // initialize stuff
  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

  if (!png_ptr)
  {
    rtLogError("FATAL: png_create_read_struct() - failed !");
    return e;
  }

  info_ptr = png_create_info_struct(png_ptr);

  if (!info_ptr)
  {
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    rtLogError("FATAL: png_create_info_struct() - failed !");
    return e;
  }

  if (!setjmp(png_jmpbuf(png_ptr)))
  {
    png_set_read_fn(png_ptr, (png_voidp)&pngStruct, readPngData);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);

    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    if (bit_depth == 16)
    {
        png_set_strip_16(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_PALETTE)
    {
      png_set_palette_to_rgb(png_ptr);
    }

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
      png_set_gray_to_rgb(png_ptr);
    }

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    {
      png_set_tRNS_to_alpha(png_ptr);
    }

    //png_set_bgr(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);

    o.init(width, height);

    //	    number_of_passes = png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);

    // read file
    if (!setjmp(png_jmpbuf(png_ptr)))
    {
      row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);

      if (row_pointers)
      {
        for (int y = 0; y < height; y++)
        {
          row_pointers[y] = (png_byte *)o.scanline(y);
        }

        png_read_image(png_ptr, row_pointers);
        free(row_pointers);
      }
      e = RT_OK;
    }
    else
    {
      e = RT_FAIL;
    }
  }
  else
  {
    e = RT_FAIL;
  }


  if (e == RT_OK)
  {
    o.mPixelFormat = RT_PIX_RGBA;
  }

  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  return e;
}

void pxTimedOffscreenSequence::init()
{
  mTotalTime = 0;
  mNumPlays = 0;
  mSequence.clear();
}

void pxTimedOffscreenSequence::addBuffer(pxBuffer &b, double d)
{
  entry e;
  e.mOffscreen.init(b.width(), b.height());

  b.blit(e.mOffscreen);

  e.mDuration = d;

  mSequence.push_back(e);
  mTotalTime += d;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "png.h"

//#define PNG_APNG_SUPPORTED

#ifdef PNG_APNG_SUPPORTED
void BlendOver(unsigned char **rows_dst, unsigned char **rows_src, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int i, j;
  int u, v, al;

  for (j = 0; j < h; j++)
  {
    unsigned char *sp = rows_src[j];
    unsigned char *dp = rows_dst[j + y] + x * 4;

    for (i = 0; i < w; i++, sp += 4, dp += 4)
    {
      if (sp[3] == 255)
        memcpy(dp, sp, 4);
      else if (sp[3] != 0)
      {
        if (dp[3] != 0)
        {
          u = sp[3] * 255;
          v = (255 - sp[3]) * dp[3];
          al = u + v;
          dp[0] = (sp[0] * u + dp[0] * v) / al;
          dp[1] = (sp[1] * u + dp[1] * v) / al;
          dp[2] = (sp[2] * u + dp[2] * v) / al;
          dp[3] = al / 255;
        }
        else
          memcpy(dp, sp, 4);
      }
    }
  }
}
#endif

rtError pxLoadAPNGImage(const char *imageData, size_t imageDataSize,
                        pxTimedOffscreenSequence &s)
{
  if (!imageData)
  {
    rtLogError("FATAL: Invalid arguments - imageData = NULL");
    return RT_FAIL;
  }

  if (imageDataSize < 8)
  {
    rtLogError("FATAL: Invalid arguments - imageDataSize < 8");
    return RT_FAIL;
  }

  s.init();

  PngStruct pngStruct((char *)imageData, imageDataSize);

  unsigned char header[8]; // 8 is the maximum size that can be checked

  // test for it being a png
  memcpy(header, imageData, 8);
  pngStruct.readPosition += 8;

  // test PNG header
  if (png_sig_cmp(header, 0, 8) != 0)
  {
    // TODO Improve Detection of different image types
    //    rtLogError("FATAL: Invalid PNG header");
    return RT_FAIL;
  }

  //unsigned int width, height, channels, rowbytes, size, i, j;
  unsigned int width, height, i, j;

  unsigned long size, rowbytes;

  png_bytepp rows_image;
  png_bytepp rows_frame;
  unsigned char *p_image;
  unsigned char *p_frame;
  unsigned char *p_temp;

  png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (png_ptr && info_ptr)
  {
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return RT_FAIL;
    }

    png_set_read_fn(png_ptr, (png_voidp)&pngStruct, readPngData);
    png_set_sig_bytes(png_ptr, 8);
    png_read_info(png_ptr, info_ptr);
    png_set_expand(png_ptr);
    png_set_strip_16(png_ptr);
    png_set_palette_to_rgb(png_ptr);
    png_set_gray_to_rgb(png_ptr);
    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
    (void)png_set_interlace_handling(png_ptr);
    png_read_update_info(png_ptr, info_ptr);
    width = png_get_image_width(png_ptr, info_ptr);
    height = png_get_image_height(png_ptr, info_ptr);
    //channels = png_get_channels(png_ptr, info_ptr);
    rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    size = height * rowbytes;
    p_image = (unsigned char *)malloc(size);
    p_frame = (unsigned char *)malloc(size);
    p_temp = (unsigned char *)malloc(size);
    rows_image = (png_bytepp)malloc(height * sizeof(png_bytep));
    rows_frame = (png_bytepp)malloc(height * sizeof(png_bytep));
    if (p_image && p_frame && p_temp && rows_image && rows_frame)
    {
      png_uint_32 frames = 1;
      png_uint_32 x0 = 0;
      png_uint_32 y0 = 0;
      png_uint_32 w0 = width;
      png_uint_32 h0 = height;
      unsigned int first = 0;
      unsigned short delay_num = 1;
      unsigned short delay_den = 10;

#ifdef PNG_APNG_SUPPORTED
      png_uint_32 plays = 0;
      unsigned char dop = 0;
      unsigned char bop = 0;

      first = (png_get_first_frame_is_hidden(png_ptr, info_ptr) != 0) ? 1 : 0;
      if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
        png_get_acTL(png_ptr, info_ptr, &frames, &plays);

      s.setNumPlays(plays);
#endif
      for (j = 0; j < height; j++)
        rows_image[j] = p_image + j * rowbytes;

      for (j = 0; j < height; j++)
        rows_frame[j] = p_frame + j * rowbytes;

      for (i = 0; i < frames; i++)
      {
#ifdef PNG_APNG_SUPPORTED
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
        {
          png_read_frame_head(png_ptr, info_ptr);
          png_get_next_frame_fcTL(png_ptr, info_ptr, &w0, &h0, &x0, &y0, &delay_num, &delay_den, &dop, &bop);

          if (!delay_den)
            delay_den = 100;
        }
        if (i == first)
        {
          bop = PNG_BLEND_OP_SOURCE;
          if (dop == PNG_DISPOSE_OP_PREVIOUS)
            dop = PNG_DISPOSE_OP_BACKGROUND;
        }
#endif
        png_read_image(png_ptr, rows_frame);

#ifdef PNG_APNG_SUPPORTED
        if (dop == PNG_DISPOSE_OP_PREVIOUS)
          memcpy(p_temp, p_image, size);

        if (bop == PNG_BLEND_OP_OVER)
          BlendOver(rows_image, rows_frame, x0, y0, w0, h0);
        else
#endif
          for (j = 0; j < h0; j++)
            memcpy(rows_image[j + y0] + x0 * 4, rows_frame[j], w0 * 4);

        // TODO Extra copy of frame going on here
        if (i >= first)
        {
          pxOffscreen o;
          o.init(width, height);
          for (uint32_t i = 0; i < height; i++)
          {
            memcpy(o.scanline(i), rows_image[i], width * 4);
          }
          s.addBuffer(o, (double)delay_num / (double)delay_den);
        }

#ifdef PNG_APNG_SUPPORTED
        if (dop == PNG_DISPOSE_OP_PREVIOUS)
          memcpy(p_image, p_temp, size);
        else if (dop == PNG_DISPOSE_OP_BACKGROUND)
          for (j = 0; j < h0; j++)
            memset(rows_image[j + y0] + x0 * 4, 0, w0 * 4);
#endif
      }
      png_read_end(png_ptr, info_ptr);

    }

    free(rows_frame);
    free(rows_image);
    free(p_temp);
    free(p_frame);
    free(p_image);
  }
  png_destroy_read_struct(&png_ptr, &info_ptr, NULL);

  return RT_OK;
}

rtString imageType2str(pxImageType t)
{
  switch(t)
  {
    case PX_IMAGE_JPG:      return rtString("PX_IMAGE_JPG");
    case PX_IMAGE_PNG:      return rtString("PX_IMAGE_PNG");
    case PX_IMAGE_GIF:      return rtString("PX_IMAGE_GIF");
    case PX_IMAGE_TIFF:     return rtString("PX_IMAGE_TIFF");
    case PX_IMAGE_BMP:      return rtString("PX_IMAGE_BMP");
    case PX_IMAGE_WEBP:     return rtString("PX_IMAGE_WEBP");
    case PX_IMAGE_ICO:      return rtString("PX_IMAGE_ICO");
    case PX_IMAGE_SVG:      return rtString("PX_IMAGE_SVG");
    default:
    case PX_IMAGE_INVALID:  return rtString("PX_IMAGE_INVALID");
  }
}

pxImageType getImageType( const uint8_t* data, size_t len )
{
  if ( data == NULL ) return PX_IMAGE_INVALID;
  if ( len < 16 ) return PX_IMAGE_INVALID;

  // .jpg:  FF D8 FF
  // .png:  89 50 4E 47 0D 0A 1A 0A
  // .gif:  GIF87a
  //        GIF89a
  // .tiff: 49 49 2A 00
  //        4D 4D 00 2A
  // .bmp:  BM
  // .webp: RIFF ???? WEBP
  // .ico   00 00 01 00
  //        00 00 02 00 ( cursor files )

  switch ( data[0] )
  {
    case (uint8_t)'\xFF':
      return ( !strncmp( (const char*)data, "\xFF\xD8\xFF", 3 )) ?
      PX_IMAGE_JPG : PX_IMAGE_INVALID;

    case (uint8_t)'\x89':
      return ( !strncmp( (const char*)data,
                        "\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8 )) ?
      PX_IMAGE_PNG : PX_IMAGE_INVALID;

    case 'G':
      return ( !strncmp( (const char*)data, "GIF87a", 6 ) ||
              !strncmp( (const char*)data, "GIF89a", 6 ) ) ?
      PX_IMAGE_GIF : PX_IMAGE_INVALID;

    case 'I':
      return ( !strncmp( (const char*)data, "\x49\x49\x2A\x00", 4 )) ?
      PX_IMAGE_TIFF : PX_IMAGE_INVALID;

    case 'M':
      return ( !strncmp( (const char*)data, "\x4D\x4D\x00\x2A", 4 )) ?
      PX_IMAGE_TIFF : PX_IMAGE_INVALID;

    case 'B':
      return (( data[1] == 'M' )) ?
      PX_IMAGE_BMP : PX_IMAGE_INVALID;

    case 'R':
      if ( strncmp( (const char*)data,     "RIFF", 4 ))
        return PX_IMAGE_INVALID;
      if ( strncmp( (const char*)(data+8), "WEBP", 4 ))
        return PX_IMAGE_INVALID;
      return PX_IMAGE_WEBP;

    case '\0':
      if ( !strncmp( (const char*)data, "\x00\x00\x01\x00", 4 ))
        return PX_IMAGE_ICO;
      if ( !strncmp( (const char*)data, "\x00\x00\x02\x00", 4 ))
        return PX_IMAGE_ICO;
      return PX_IMAGE_INVALID;

    default:
      return PX_IMAGE_INVALID;
  }
}


static char encoding_table[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
                                'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
                                'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
                                'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
                                'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
                                'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
                                'w', 'x', 'y', 'z', '0', '1', '2', '3',
                                '4', '5', '6', '7', '8', '9', '+', '/'};

static char *decoding_table = NULL;
static int mod_table[] = {0, 2, 1};

void build_decoding_table()
{
  decoding_table = (char*)malloc(256);

  for (int i = 0; i < 64; i++)
    decoding_table[(unsigned char) encoding_table[i]] = i;
}


void base64_cleanup()
{
  if(decoding_table)
  {
    free(decoding_table);
    decoding_table = NULL;
  }
}

// ENCODE
char *base64_encode(const unsigned char *data,
                    size_t input_length,
                    size_t *output_length)
{
  if(output_length == NULL)
  {
    return NULL;
  }

  if(data == NULL)
  {
    output_length = 0;
    return NULL;
  }

  if(input_length == 0)
  {
    output_length = 0;
    return NULL;
  }

  *output_length = 4 * ((input_length + 2) / 3);

  char *encoded_data = (char *)malloc(*output_length);
  if (encoded_data == NULL) return NULL;

  for (uint32_t i = 0, j = 0; i < input_length;)
  {

    uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
    uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

    uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

    encoded_data[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
    encoded_data[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
  }

  for (int i = 0; i < mod_table[input_length % 3]; i++)
  encoded_data[*output_length - 1 - i] = '=';

  return encoded_data;
}

rtError base64_encode(const unsigned char *data, size_t input_length, rtString& s)
{
  size_t output_length = 0;

  char *ans = base64_encode(data, input_length, &output_length);

  if(ans != NULL)
  {
    s.init( (const char*) ans, output_length);

    free(ans);

    return RT_OK;
  }
  else
  {
    return RT_FAIL;
  }
}

rtError base64_encode(rtData& d, rtString& s)
{
  return base64_encode( (const unsigned char *) d.data(), d.length(), s);
}


// DECODE
unsigned char *base64_decode(const unsigned char *data,
                             size_t input_length,
                             size_t *output_length)
{
  if (data == NULL)
  {
    return NULL;
  }

  if (output_length == NULL)
  {
    return NULL;
  }

  if ((input_length == 0) || (input_length % 4 != 0))
  {
    return NULL;
  }

  if (output_length)
    *output_length = input_length / 4 * 3;

  if (data[input_length - 1] == '=')
    (*output_length)--;
  if (data[input_length - 2] == '=')
    (*output_length)--;

  unsigned char *decoded_data = (unsigned char*)malloc(*output_length);

  if (decoded_data == NULL)
  {
    return NULL;
  }

  if (decoding_table == NULL)
  {
    build_decoding_table();
  }

  for (uint32_t i = 0, j = 0; i < input_length;)
  {
    uint32_t sextet_a = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_b = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_c = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];
    uint32_t sextet_d = data[i] == '=' ? 0 & i++ : decoding_table[data[i++]];

    uint32_t triple = (sextet_a << 3 * 6)
    + (sextet_b << 2 * 6)
    + (sextet_c << 1 * 6)
    + (sextet_d << 0 * 6);

    if (j < *output_length) decoded_data[j++] = (triple >> 2 * 8) & 0xFF;
    if (j < *output_length) decoded_data[j++] = (triple >> 1 * 8) & 0xFF;
    if (j < *output_length) decoded_data[j++] = (triple >> 0 * 8) & 0xFF;
  }

  return decoded_data;
}

rtError base64_decode(const unsigned char *data, size_t input_length, rtData& d)
{
  size_t output_length = 0;

  unsigned char *ans = base64_decode(data, input_length, &output_length);

  if(ans)
  {
    d.init((const uint8_t*) ans, output_length);

    free(ans);

    return RT_OK;
  }
  else
  {
    return RT_FAIL;
  }
}

//void testBase64()
//{
//    rtString  in1 = "VGVzdGluZzEyMw=="; // "Testing123" in Base64
//    rtData   out1;
//  
//    base64_decode( (const unsigned char *) in1.cString(), in1.length(), out1);
//  
//    rtString txt = "Testing123";
//    rtData   in2( (const uint8_t*) txt.cString(), txt.length());
//    rtString out2;
//  
//    base64_encode( in2, out2 );
//  
//    if(memcmp(out1.data(), in2.data(), out1.length()) == 0)
//    {
//      printf("OK");
//    }
//}

rtError base64_decode(rtString& s, rtData& d)
{
  // testBase64();
  return base64_decode( (const unsigned char *) s.cString(), s.length(), d);
}



rtString md5sum(rtString &d)
{
  unsigned char md5_result[MD5_DIGEST_LENGTH];        // binary
  unsigned char str_result[MD5_DIGEST_LENGTH*2 + 1];  // char string

  MD5( (const unsigned char *) d.cString(), d.length(), (unsigned char *) &md5_result[0] );

  for(int i=0; i < MD5_DIGEST_LENGTH; i++)
  {
    sprintf( (char *) &str_result[i*2], "%02X",  md5_result[i]); // sprintf() ... will null terminate
  }

  return rtString(  (char *) str_result);
}

