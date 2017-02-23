/*
 * resave2rgba.c
 *
 * converts any APNG to 32bpp APNG.
 * loads and saves animation frame by frame.
 *
 * needs apng-patched libpng.
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

void load_resave_apng(char * szIn, char * szOut)
{
  FILE * f1;
  FILE * f2;

  if ((f1 = fopen(szIn,  "rb")) != 0)
  if ((f2 = fopen(szOut, "wb")) != 0)
  {
    unsigned int    width, height, depth, coltype, rowbytes, size, i, j;
    png_bytepp      rows;
    unsigned char * p_frame;
    unsigned char   sig[8];

    if (fread(sig, 1, 8, f1) == 8 && png_sig_cmp(sig, 0, 8) == 0)
    {
      png_structp read_png_ptr   = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      png_infop   read_info_ptr  = png_create_info_struct(read_png_ptr);
      png_structp write_png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      png_infop   write_info_ptr = png_create_info_struct(write_png_ptr);
      if (read_png_ptr && read_info_ptr && write_png_ptr && write_info_ptr)
      {
        if (setjmp(png_jmpbuf(read_png_ptr)))
        {
          png_destroy_read_struct(&read_png_ptr, &read_info_ptr, NULL);
          fclose(f1);
          fclose(f2);
          return;
        }
        if (setjmp(png_jmpbuf(write_png_ptr)))
        {
          png_destroy_write_struct(&write_png_ptr, &write_info_ptr);
          fclose(f1);
          fclose(f2);
          return;
        }
        png_init_io(read_png_ptr, f1);
        png_set_sig_bytes(read_png_ptr, 8);
        png_read_info(read_png_ptr, read_info_ptr);
        png_set_expand(read_png_ptr);
        png_set_strip_16(read_png_ptr);
        png_set_gray_to_rgb(read_png_ptr);                        /* make it RGB */
        png_set_add_alpha(read_png_ptr, 0xff, PNG_FILLER_AFTER);  /* make it RGBA */
        (void)png_set_interlace_handling(read_png_ptr);
        png_read_update_info(read_png_ptr, read_info_ptr);
        width    = png_get_image_width(read_png_ptr, read_info_ptr);
        height   = png_get_image_height(read_png_ptr, read_info_ptr);
        depth    = png_get_bit_depth(read_png_ptr, read_info_ptr);
        coltype  = png_get_color_type(read_png_ptr, read_info_ptr);
        rowbytes = png_get_rowbytes(read_png_ptr, read_info_ptr);

        png_init_io(write_png_ptr, f2);
        png_set_compression_level(write_png_ptr, 9);
        png_set_IHDR(write_png_ptr, write_info_ptr, width, height, depth, coltype, 0, 0, 0);

        size = height*rowbytes;
        p_frame = (unsigned char *)malloc(size);
        rows = (png_bytepp)malloc(height*sizeof(png_bytep));
        if (p_frame && rows)
        {
          png_uint_32     frames = 1;
#ifdef PNG_APNG_SUPPORTED
          png_uint_32     x0 = 0;
          png_uint_32     y0 = 0;
          png_uint_32     w0 = width;
          png_uint_32     h0 = height;
          png_uint_32     plays = 0;
          unsigned short  delay_num = 1;
          unsigned short  delay_den = 10;
          unsigned char   dop = 0;
          unsigned char   bop = 0;
          unsigned int    hidden;
          if (png_get_valid(read_png_ptr, read_info_ptr, PNG_INFO_acTL))
          {
            png_get_acTL(read_png_ptr, read_info_ptr, &frames, &plays);
            png_set_acTL(write_png_ptr, write_info_ptr, frames, plays);
          }
          hidden = png_get_first_frame_is_hidden(read_png_ptr, read_info_ptr);
          png_set_first_frame_is_hidden(write_png_ptr, write_info_ptr, hidden);
#endif
          png_write_info(write_png_ptr, write_info_ptr);

          for (j=0; j<height; j++)
            rows[j] = p_frame + j*rowbytes;

          for (i=0; i<frames; i++)
          {
#ifdef PNG_APNG_SUPPORTED
            if (png_get_valid(read_png_ptr, read_info_ptr, PNG_INFO_acTL))
            {
              png_read_frame_head(read_png_ptr, read_info_ptr);
              png_get_next_frame_fcTL(read_png_ptr, read_info_ptr, &w0, &h0, &x0, &y0, &delay_num, &delay_den, &dop, &bop);
              png_write_frame_head(write_png_ptr, write_info_ptr, NULL, w0, h0, x0, y0, delay_num, delay_den, dop, bop);
            }
#endif
            png_read_image(read_png_ptr, rows);
            png_write_image(write_png_ptr, rows);
#ifdef PNG_APNG_SUPPORTED
            if (frames > 1)
              png_write_frame_tail(write_png_ptr, write_info_ptr);
#endif
          }
          png_read_end(read_png_ptr, read_info_ptr);
          png_write_end(write_png_ptr, write_info_ptr);
          free(rows);
          free(p_frame);
        }
        printf("  [libpng");
#ifdef PNG_APNG_SUPPORTED
        printf("+apng");
#endif
        printf(" %s]:  ", PNG_LIBPNG_VER_STRING);
        printf("%s : %dx%d\n", szOut, width, height);
      }
      png_destroy_read_struct(&read_png_ptr, &read_info_ptr, NULL);
      png_destroy_write_struct(&write_png_ptr, &write_info_ptr);
    }
    fclose(f1);
    fclose(f2);
  }
}

int main(int argc, char** argv)
{
  if (argc <= 1)
    printf("Usage : resave2rgba input.png\n");
  else
    load_resave_apng(argv[1], "test_resave2rgba.png");

  printf("\n");
  return (0);
}
