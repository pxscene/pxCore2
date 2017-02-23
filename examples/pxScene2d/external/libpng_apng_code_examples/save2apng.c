/*
 * save2apng.c
 *
 * saves animated PNG file.
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

void save_png(char * szImage, unsigned char * p_frame, unsigned int w, unsigned int h, unsigned int d, unsigned int t, unsigned int frames)
{
  FILE * f1;

  if ((f1 = fopen(szImage, "wb")) != 0)
  {
    png_structp png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop   info_ptr = png_create_info_struct(png_ptr);
    if (png_ptr != NULL && info_ptr != NULL && setjmp(png_jmpbuf(png_ptr)) == 0)
    {
      unsigned int rowbytes, n, j;
      png_bytepp   rows = (png_bytepp)malloc(h*sizeof(png_bytep));
      if (p_frame && rows)
      {
        png_init_io(png_ptr, f1);
        png_set_compression_level(png_ptr, 9);
        png_set_IHDR(png_ptr, info_ptr, w, h, d, t, 0, 0, 0);
#ifdef PNG_APNG_SUPPORTED
        png_set_acTL(png_ptr, info_ptr, frames, 0);
        /*png_set_first_frame_is_hidden(png_ptr, info_ptr, 1);*/
#endif
        png_write_info(png_ptr, info_ptr);
        rowbytes = png_get_rowbytes(png_ptr, info_ptr);
        for (n=0; n<frames; n++)
        {
          for (j=0; j<h; j++)
            rows[j] = p_frame + (n*h + j)*rowbytes;

#ifdef PNG_APNG_SUPPORTED
          png_write_frame_head(png_ptr, info_ptr, NULL, w, h, 0, 0, 12, 100, PNG_DISPOSE_OP_NONE, PNG_BLEND_OP_SOURCE);
#endif
          png_write_image(png_ptr, rows);
#ifdef PNG_APNG_SUPPORTED
          png_write_frame_tail(png_ptr, info_ptr);
#endif
        }
        png_write_end(png_ptr, info_ptr);
        free(rows);
      }
    }
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(f1);
    printf("  [libpng");
#ifdef PNG_APNG_SUPPORTED
    printf("+apng");
#endif
    printf(" %s]:  ", PNG_LIBPNG_VER_STRING);
    printf("%s : %dx%d\n", szImage, w, h);
  }
}

int main(int argc, char** argv)
{
  unsigned int  width, height, depth, coltype, frames, i, j, n;
  unsigned char data[5][52][86][4];

  width   = 86;
  height  = 52;
  depth   = 8;
  coltype = 6;
#ifdef PNG_APNG_SUPPORTED
  frames  = 5;
#else
  frames  = 1;
#endif

  for (n=0; n<frames; n++)
  for (j=0; j<height; j++)
  for (i=0; i<width; i++)
  {
    data[n][j][i][0] = ((i+j)%5==n) ? 0 : j*5;
    data[n][j][i][1] = ((i+j)%5==n) ? 0 : i*3;
    data[n][j][i][2] = ((i+j)%5==n) ? 0 : j*5;
    data[n][j][i][3] = 255;
  }

  save_png("test_save2.png", &data[0][0][0][0], width, height, depth, coltype, frames);

  printf("\n");
  return (0);
}
