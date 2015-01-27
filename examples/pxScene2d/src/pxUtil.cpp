// pxCore CopyRight 2007-2015 John Robinson
// pxUtil.cpp

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <png.h>

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"

rt_error pxLoadImage(const char* filename, pxOffscreen& b) {
  return pxLoadPNGImage(filename, b);
}

rt_error pxStoreImage(const char* filename, pxBuffer& b) {
  return pxStorePNGImage(filename, b);
}

rt_error pxLoadPNGImage(const char* filename, pxOffscreen& o)
{
  rt_error e = RT_FAIL;
  
  png_structp png_ptr;
  png_infop info_ptr;
  //  int number_of_passes;
  png_bytep * row_pointers;
  
  /* open file and test for it being a png */
  FILE *fp = fopen(filename, "rb");
  if (fp) {
    unsigned char header[8];    // 8 is the maximum size that can be checked

    fread(header, 1, 8, fp);
    if (png_sig_cmp(header, 0, 8) == 0) {
      
      /* initialize stuff */
      png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      
      if (png_ptr) {
	
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr) {
	  
	  if (!setjmp(png_jmpbuf(png_ptr))) {
	    
	    png_init_io(png_ptr, fp);
	    png_set_sig_bytes(png_ptr, 8);
	    
	    png_read_info(png_ptr, info_ptr);
	    
	    int width = png_get_image_width(png_ptr, info_ptr);
	    int height = png_get_image_height(png_ptr, info_ptr);
	    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
	    //png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

	    if (color_type == PNG_COLOR_TYPE_PALETTE)
	      png_set_palette_to_rgb(png_ptr);

#if 0
	    if (info_ptr->bit_depth < 8) {
	      png_set_packing(png_ptr);
#endif

#if 0		
	      if (color_type == PNG_COLOR_TYPE_GRAY)
		png_set_gray_1_2_4_to_8(png_ptr);
	    }
#endif	    
	    if (color_type == PNG_COLOR_TYPE_GRAY || 
		color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
	      png_set_gray_to_rgb(png_ptr);
	    
	    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
	      png_set_tRNS_to_alpha(png_ptr);

#if 0
	    if (info_ptr->bit_depth == 16)
	      png_set_strip_16(png_ptr);
#endif
	    png_set_bgr(png_ptr);
	    png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
	    
	    o.init(width, height);

	    //	    number_of_passes = png_set_interlace_handling(png_ptr);
	    png_read_update_info(png_ptr, info_ptr);
	    
	    /* read file */
	    if (!setjmp(png_jmpbuf(png_ptr))) {
	      
	      row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * height);
	      if (row_pointers) {

#if 0
		for (y=0; y<height; y++)
		  row_pointers[y] = (png_byte*) malloc(png_get_rowbytes(png_ptr,info_ptr));
#endif
#if 1
		for (int y = 0; y < height; y++) {
		  row_pointers[y] = (png_byte*)o.scanline(y);
		}
#endif
	      
		png_read_image(png_ptr, row_pointers);
		free(row_pointers);
	      }
	      e = RT_OK;
	    }
	  }
	}
      }
    }
    fclose(fp);
  }
  return e;
}

rt_error pxStorePNGImage(const char* filename, pxBuffer& b, bool /*grayscale*/, bool /*alpha*/)
{
  rt_error e = RT_FAIL;

  png_structp png_ptr;
  png_infop info_ptr;
  png_bytep * row_pointers;

  /* create file */
  FILE *fp = fopen(filename, "wb");
  if (fp) {
    
    /* initialize stuff */
    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

    if (png_ptr) {

      info_ptr = png_create_info_struct(png_ptr);

      if (info_ptr) {

        if (!setjmp(png_jmpbuf(png_ptr))) {

	  png_init_io(png_ptr, fp);

	  /* write header */
	  if (!setjmp(png_jmpbuf(png_ptr))) {

#if 0
	    png_byte color_type = (grayscale?PNG_COLOR_MASK_ALPHA:0) |
	      (alpha?PNG_COLOR_MASK_ALPHA:0);
#endif
	    png_set_bgr(png_ptr);

	    png_set_IHDR(png_ptr, info_ptr, b.width(), b.height(),
			 8, PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
			 PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
	    
	    png_write_info(png_ptr, info_ptr);


	    /* write bytes */
	    if (!setjmp(png_jmpbuf(png_ptr))) {

	      row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * b.height());
	      if (row_pointers) {

		for (int y = 0; y < b.height(); y++) {
		  row_pointers[y] = (png_byte*)b.scanline(y);
		}
		
		png_write_image(png_ptr, row_pointers);

		/* end write */
		if (!setjmp(png_jmpbuf(png_ptr))) {
		  png_write_end(png_ptr, NULL);
		  e = RT_OK;
		}

		free(row_pointers);
	      }
	    }
	  }
	}
      }
    }
#if 0
		/* cleanup heap allocation */
		for (y=0; y<height; y++)
		  free(row_pointers[y]);
#endif
		//		free(row_pointers);

		fclose(fp);
  }
  return e;
}

rt_error pxLoadJPGImage(char* /*filename*/, pxOffscreen& /*o*/) {
  return RT_FAIL;
}

rt_error pxStoreJPGImage(char* /*filename*/, pxBuffer& /*b*/) {
  return RT_FAIL;
}
