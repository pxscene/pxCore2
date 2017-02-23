/*
 * load4apng.c
 *
 * loads APNG file, saves all frames as TGA (32bpp).
 * including frames composition.
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
#include <string.h>
#include "png.h"

void save_tga(unsigned char ** rows, unsigned int w, unsigned int h, unsigned int channels, unsigned int frame)
{
  char szOut[512];
  FILE * f2;
  if (channels == 4)
  {
    unsigned short tgah[9] = {0,2,0,0,0,0,(unsigned short)w,(unsigned short)h,0x0820};
    sprintf(szOut, "test_load4_%03d.tga", frame);
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
    printf("%s : %dx%d   %c\n", szOut, w, h, frame>0 ? '*' : ' ');
  }
}

#ifdef PNG_APNG_SUPPORTED
void BlendOver(unsigned char ** rows_dst, unsigned char ** rows_src, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int  i, j;
  int u, v, al;

  for (j=0; j<h; j++)
  {
    unsigned char * sp = rows_src[j];
    unsigned char * dp = rows_dst[j+y] + x*4;

    for (i=0; i<w; i++, sp+=4, dp+=4)
    {
      if (sp[3] == 255)
        memcpy(dp, sp, 4);
      else
      if (sp[3] != 0)
      {
        if (dp[3] != 0)
        {
          u = sp[3]*255;
          v = (255-sp[3])*dp[3];
          al = u + v;
          dp[0] = (sp[0]*u + dp[0]*v)/al;
          dp[1] = (sp[1]*u + dp[1]*v)/al;
          dp[2] = (sp[2]*u + dp[2]*v)/al;
          dp[3] = al/255;
        }
        else
          memcpy(dp, sp, 4);
      }
    }  
  }
}
#endif

void load_png(char * szImage)
{
  FILE * f1;

  if ((f1 = fopen(szImage, "rb")) != 0)
  {
    unsigned int    width, height, channels, rowbytes, size, i, j;
    png_bytepp      rows_image;
    png_bytepp      rows_frame;
    unsigned char * p_image;
    unsigned char * p_frame;
    unsigned char * p_temp;
    unsigned char   sig[8];

    if (fread(sig, 1, 8, f1) == 8 && png_sig_cmp(sig, 0, 8) == 0)
    {
      png_structp png_ptr  = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
      png_infop   info_ptr = png_create_info_struct(png_ptr);
      if (png_ptr && info_ptr)
      {
        if (setjmp(png_jmpbuf(png_ptr)))
        {
          png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
          fclose(f1);
          return;
        }
        png_init_io(png_ptr, f1);
        png_set_sig_bytes(png_ptr, 8);
        png_read_info(png_ptr, info_ptr);
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
        size = height*rowbytes;
        p_image = (unsigned char *)malloc(size);
        p_frame = (unsigned char *)malloc(size);
        p_temp  = (unsigned char *)malloc(size);
        rows_image = (png_bytepp)malloc(height*sizeof(png_bytep));
        rows_frame = (png_bytepp)malloc(height*sizeof(png_bytep));
        if (p_image && p_frame && p_temp && rows_image && rows_frame)
        {
          png_uint_32     frames = 1;
          png_uint_32     x0 = 0;
          png_uint_32     y0 = 0;
          png_uint_32     w0 = width;
          png_uint_32     h0 = height;
#ifdef PNG_APNG_SUPPORTED
          png_uint_32     plays = 0;
          unsigned short  delay_num = 1;
          unsigned short  delay_den = 10;
          unsigned char   dop = 0;
          unsigned char   bop = 0;
          unsigned int    first = (png_get_first_frame_is_hidden(png_ptr, info_ptr) != 0) ? 1 : 0;
          if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
            png_get_acTL(png_ptr, info_ptr, &frames, &plays);
#endif
          for (j=0; j<height; j++)
            rows_image[j] = p_image + j*rowbytes;

          for (j=0; j<height; j++)
            rows_frame[j] = p_frame + j*rowbytes;

          for (i=0; i<frames; i++)
          {
#ifdef PNG_APNG_SUPPORTED
            if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
            {
              png_read_frame_head(png_ptr, info_ptr);
              png_get_next_frame_fcTL(png_ptr, info_ptr, &w0, &h0, &x0, &y0, &delay_num, &delay_den, &dop, &bop);
              printf("Frame n=%d delay_num=%d delay_den=%d\n", i, delay_num, delay_den);
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
            for (j=0; j<h0; j++)
              memcpy(rows_image[j+y0] + x0*4, rows_frame[j], w0*4);

            save_tga(rows_image, width, height, channels, i);

#ifdef PNG_APNG_SUPPORTED
            if (dop == PNG_DISPOSE_OP_PREVIOUS)
              memcpy(p_image, p_temp, size);
            else
            if (dop == PNG_DISPOSE_OP_BACKGROUND)
              for (j=0; j<h0; j++)
                memset(rows_image[j+y0] + x0*4, 0, w0*4);
#endif
          }
          png_read_end(png_ptr, info_ptr);
          free(rows_frame);
          free(rows_image);
          free(p_temp);
          free(p_frame);
          free(p_image);
        }
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }
    fclose(f1);
  }
}

int main(int argc, char** argv)
{
  if (argc <= 1)
    printf("Usage : load4apng input.png\n");
  else
    load_png(argv[1]);

  printf("\n");
  return (0);
}
