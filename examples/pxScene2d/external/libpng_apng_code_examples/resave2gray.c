/*
 * resave2gray.c
 *
 * converts any APNG to grayscale APNG.
 * loads the whole animation, then saves it all.
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

typedef struct 
{
  png_uint_32    w0, h0, x0, y0;
  unsigned short delay_num, delay_den;
  unsigned char  dop, bop;
  unsigned char * frame;
  unsigned int size;
  png_bytepp rows;
} image_info;

void load_apng(char * szImage, image_info ** img, unsigned int * w, unsigned int * h, unsigned int * d, unsigned int * t, png_uint_32 * frames, png_uint_32 * plays, unsigned int * hidden)
{
  FILE * f1;

  if ((f1 = fopen(szImage, "rb")) != 0)
  {
    unsigned int    rowbytes, i, j;
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
        png_set_rgb_to_gray(png_ptr, 1, -1, -1);       /* make it grayscale */
        (void)png_set_interlace_handling(png_ptr);
        png_read_update_info(png_ptr, info_ptr);
        *w = png_get_image_width(png_ptr, info_ptr);
        *h = png_get_image_height(png_ptr, info_ptr);
        *d = png_get_bit_depth(png_ptr, info_ptr);
        *t = png_get_color_type(png_ptr, info_ptr);
        *frames = 1;
        *plays  = 0;
#ifdef PNG_APNG_SUPPORTED
        *hidden = png_get_first_frame_is_hidden(png_ptr, info_ptr);
        if (png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
          png_get_acTL(png_ptr, info_ptr, frames, plays);
#endif
        *img = (image_info *)malloc(*frames*sizeof(image_info));
        if (*img != NULL)
        {
          image_info * pimg = *img;
          for (i=0; i<*frames; i++)
          {
#ifdef PNG_APNG_SUPPORTED
            if (i>=*hidden && png_get_valid(png_ptr, info_ptr, PNG_INFO_acTL))
            {
              png_read_frame_head(png_ptr, info_ptr);
              png_get_next_frame_fcTL(png_ptr, info_ptr, &pimg->w0, &pimg->h0, &pimg->x0, &pimg->y0, &pimg->delay_num, &pimg->delay_den, &pimg->dop, &pimg->bop);
            }
            else
#endif
            {
              pimg->x0 = 0;
              pimg->y0 = 0;
              pimg->w0 = *w;
              pimg->h0 = *h;
              pimg->delay_num = 1;
              pimg->delay_den = 10;
              pimg->bop = 0;
              pimg->dop = 0;
            }
            rowbytes = png_get_rowbytes(png_ptr, info_ptr);
            pimg->size = pimg->h0*rowbytes;
            pimg->frame = (unsigned char *)malloc(pimg->size);
            pimg->rows = (png_bytepp)malloc(pimg->h0*sizeof(png_bytep));
            if (pimg->frame && pimg->rows)
            {
              for (j=0; j<pimg->h0; j++)
                pimg->rows[j] = pimg->frame + j*rowbytes;

              png_read_image(png_ptr, pimg->rows);
            }
            pimg++;
          }
          png_read_end(png_ptr, info_ptr);
        }
      }
      png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    }
    fclose(f1);
  }
}

void save_apng(char * szImage, image_info * img, unsigned int w, unsigned int h, unsigned int d, unsigned int t, png_uint_32 frames, png_uint_32 plays, unsigned int hidden)
{
  FILE * f1;
  unsigned int i;

  if ((f1 = fopen(szImage, "wb")) != 0)
  {
    png_structp png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop   info_ptr = png_create_info_struct(png_ptr);
    if (png_ptr && info_ptr)
    {
      if (setjmp(png_jmpbuf(png_ptr)))
      {
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(f1);
        return;
      }
      image_info * pimg = img;

      png_init_io(png_ptr, f1);
      png_set_compression_level(png_ptr, 9);
      png_set_IHDR(png_ptr, info_ptr, w, h, d, t, 0, 0, 0);
#ifdef PNG_APNG_SUPPORTED
      if (frames > 1)
      {
        png_set_acTL(png_ptr, info_ptr, frames, plays);
        png_set_first_frame_is_hidden(png_ptr, info_ptr, hidden);
      }
#endif
      png_write_info(png_ptr, info_ptr);
      for (i=0; i<frames; i++)
      {
#ifdef PNG_APNG_SUPPORTED
        if (frames > 1)
          png_write_frame_head(png_ptr, info_ptr, NULL, pimg->w0, pimg->h0, pimg->x0, pimg->y0, pimg->delay_num, pimg->delay_den, pimg->dop, pimg->bop);
#endif
        png_write_image(png_ptr, pimg->rows);
#ifdef PNG_APNG_SUPPORTED
        if (frames > 1)
          png_write_frame_tail(png_ptr, info_ptr);
#endif
        pimg++;
      }
      png_write_end(png_ptr, info_ptr);
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
  image_info * img;
  unsigned int w, h, d, t, hidden, i;
  png_uint_32 frames, plays;

  if (argc <= 1)
    printf("Usage : resave2gray input.png\n");
  else
  {
    load_apng(argv[1], &img, &w, &h, &d, &t, &frames, &plays, &hidden);
    save_apng("test_resave2gray.png", img, w, h, d, t, frames, plays, hidden);

    for (i=0; i<frames; i++)
    {
      if (img[i].frame != NULL)
        free(img[i].frame);
      if (img[i].rows != NULL)
        free(img[i].rows);
    }
    free(img);
  }

  printf("\n");
  return (0);
}
