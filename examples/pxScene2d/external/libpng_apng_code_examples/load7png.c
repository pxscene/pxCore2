/*
 * load7png.c
 *
 * loads static PNG file, saves it as TGA (32bpp).
 * uses progressive loading.
 *
 * needs regular, unpatched libpng.
 *
 * Copyright (c) 2012-2014 Max Stepin
 * maxst at users.sourceforge.net
 *
 * zlib license
 * ------------
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include "png.h"

unsigned int    width, height, channels, rowbytes;
unsigned char * p_frame = NULL;
png_bytepp      rows = NULL;

void save_tga(unsigned char ** rows, unsigned int w, unsigned int h, unsigned int channels)
{
  char szOut[512];
  FILE * f2;
  if (channels == 4)
  {
    unsigned short tgah[9] = {0,2,0,0,0,0,(unsigned short)w,(unsigned short)h,0x0820};
    sprintf(szOut, "test_load7.tga");
    if ((f2 = fopen(szOut, "wb")) != 0)
    {
      unsigned int j;
      if (fwrite(&tgah, 1, 18, f2) != 18) return;
      for (j=0; j<h; j++)
        if (fwrite(rows[h-1-j], channels, w, f2) != w) return;
      fclose(f2);
    }
    printf("  [libpng");
#ifdef PNG_APNG_SUPPORTED
    printf("+apng");
#endif
    printf(" %s]:  ", PNG_LIBPNG_VER_STRING);
    printf("%s : %dx%d\n", szOut, w, h);
  }
}

void info_fn(png_structp png_ptr, png_infop info_ptr)
{
  png_set_expand(png_ptr);
  png_set_strip_16(png_ptr);
  png_set_gray_to_rgb(png_ptr);
  png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
  png_set_bgr(png_ptr);
  (void)png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
  width    = png_get_image_width(png_ptr, info_ptr);
  height   = png_get_image_height(png_ptr, info_ptr);
  channels = png_get_channels(png_ptr, info_ptr);
  rowbytes = png_get_rowbytes(png_ptr, info_ptr);

  p_frame = (unsigned char *)malloc(height*rowbytes);
  rows = (png_bytepp)malloc(height*sizeof(png_bytep));

  if (p_frame && rows)
  {
    unsigned int j;

    for (j=0; j<height; j++)
      rows[j] = p_frame + j*rowbytes;
  }
}

void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
  png_progressive_combine_row(png_ptr, rows[row_num], new_row);
}

void end_fn(png_structp png_ptr, png_infop info_ptr)
{
  if (p_frame && rows)
  {
    save_tga(rows, width, height, channels);
    free(rows);
    free(p_frame);
  }
}

void load_png(char * szImage)
{
  FILE * f1;

  png_structp png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop   info_ptr = png_create_info_struct(png_ptr);
  if (png_ptr && info_ptr)
  {
    if (setjmp(png_jmpbuf(png_ptr)))
    {
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
      return;
    }
    png_set_progressive_read_fn(png_ptr, NULL, info_fn, row_fn, end_fn);

    if ((f1 = fopen(szImage, "rb")) != 0)
    {
      unsigned char smallbuf[1024];
      int len;
      do
      {
        len = fread(smallbuf, 1, 1024, f1);
        if (len > 0)
          png_process_data(png_ptr, info_ptr, smallbuf, len);
      }
      while (len == 1024 && !feof(f1));
      fclose(f1);
    }

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
  }
}

int main(int argc, char** argv)
{
  if (argc <= 1)
    printf("Usage : load7png input.png\n");
  else
    load_png(argv[1]);

  printf("\n");
  return (0);
}
