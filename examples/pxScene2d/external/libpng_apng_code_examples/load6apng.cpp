/*
 * load6apng.cpp
 *
 * loads APNG file, saves all frames as PNG (32bpp).
 * including frames composition.
 *
 * needs regular, unpatched libpng.
 *
 * Copyright (c) 2014 Max Stepin
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
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include "png.h"     /* original (unpatched) libpng is ok */
#include "zlib.h"

#define notabc(c) ((c) < 65 || (c) > 122 || ((c) > 90 && (c) < 97))

#define id_IHDR 0x52444849
#define id_acTL 0x4C546361
#define id_fcTL 0x4C546366
#define id_IDAT 0x54414449
#define id_fdAT 0x54416466
#define id_IEND 0x444E4549

struct CHUNK { unsigned char * p; unsigned int size; };
struct APNGFrame { unsigned char * p, ** rows; unsigned int w, h, delay_num, delay_den; };

void info_fn(png_structp png_ptr, png_infop info_ptr)
{
  png_set_expand(png_ptr);
  png_set_strip_16(png_ptr);
  png_set_gray_to_rgb(png_ptr);
  png_set_add_alpha(png_ptr, 0xff, PNG_FILLER_AFTER);
  (void)png_set_interlace_handling(png_ptr);
  png_read_update_info(png_ptr, info_ptr);
}

void row_fn(png_structp png_ptr, png_bytep new_row, png_uint_32 row_num, int pass)
{
  APNGFrame * frame = (APNGFrame *)png_get_progressive_ptr(png_ptr);
  png_progressive_combine_row(png_ptr, frame->rows[row_num], new_row);
}

void compose_frame(unsigned char ** rows_dst, unsigned char ** rows_src, unsigned char bop, unsigned int x, unsigned int y, unsigned int w, unsigned int h)
{
  unsigned int  i, j;
  int u, v, al;

  for (j=0; j<h; j++)
  {
    unsigned char * sp = rows_src[j];
    unsigned char * dp = rows_dst[j+y] + x*4;

    if (bop == 0)
      memcpy(dp, sp, w*4);
    else
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

unsigned int read_chunk(FILE * f, CHUNK * pChunk)
{
  unsigned char len[4];
  if (fread(&len, 4, 1, f) == 1)
  {
    pChunk->size = png_get_uint_32(len) + 12;
    pChunk->p = new unsigned char[pChunk->size];
    memcpy(pChunk->p, len, 4);
    if (fread(pChunk->p + 4, pChunk->size - 4, 1, f) == 1)
      return *(unsigned int *)(pChunk->p + 4);
  }
  return 0;
}

void processing_start(png_structp & png_ptr, png_infop & info_ptr, void * frame_ptr, bool hasInfo, CHUNK & chunkIHDR, std::vector<CHUNK>& chunksInfo)
{
  unsigned char header[8] = {137, 80, 78, 71, 13, 10, 26, 10};

  png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  info_ptr = png_create_info_struct(png_ptr);
  if (!png_ptr || !info_ptr)
    return;

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return;
  }

  png_set_crc_action(png_ptr, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
  png_set_progressive_read_fn(png_ptr, frame_ptr, info_fn, row_fn, NULL);
          
  png_process_data(png_ptr, info_ptr, header, 8);
  png_process_data(png_ptr, info_ptr, chunkIHDR.p, chunkIHDR.size);

  if (hasInfo)
    for (unsigned int i=0; i<chunksInfo.size(); i++)
      png_process_data(png_ptr, info_ptr, chunksInfo[i].p, chunksInfo[i].size);
}

void processing_data(png_structp png_ptr, png_infop info_ptr, unsigned char * p, unsigned int size)
{
  if (!png_ptr || !info_ptr)
    return;

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return;
  }

  png_process_data(png_ptr, info_ptr, p, size);
}

int processing_finish(png_structp png_ptr, png_infop info_ptr)
{
  unsigned char footer[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};

  if (!png_ptr || !info_ptr)
    return 1;

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return 1;
  }

  png_process_data(png_ptr, info_ptr, footer, 12);
  png_destroy_read_struct(&png_ptr, &info_ptr, 0);

  return 0;
}

int load_apng(char * szIn, std::vector<APNGFrame>& frames)
{
  FILE * f;
  unsigned int id, i, j, w, h, w0, h0, x0, y0;
  unsigned int delay_num, delay_den, dop, bop, rowbytes, imagesize;
  unsigned char sig[8];
  png_structp png_ptr;
  png_infop info_ptr;
  CHUNK chunk;
  CHUNK chunkIHDR;
  std::vector<CHUNK> chunksInfo;
  bool isAnimated = false;
  bool skipFirst = false;
  bool hasInfo = false;
  APNGFrame frameRaw = {0};
  APNGFrame frameCur = {0};
  APNGFrame frameNext = {0};
  int res = -1;

  if ((f = fopen(szIn, "rb")) != 0)
  {
    if (fread(sig, 1, 8, f) == 8 && png_sig_cmp(sig, 0, 8) == 0)
    {
      id = read_chunk(f, &chunkIHDR);

      if (id == id_IHDR && chunkIHDR.size == 25)
      {
        w0 = w = png_get_uint_32(chunkIHDR.p + 8);
        h0 = h = png_get_uint_32(chunkIHDR.p + 12);
        x0 = 0;
        y0 = 0;
        delay_num = 1;
        delay_den = 10;
        dop = 0;
        bop = 0;
        rowbytes = w * 4;
        imagesize = h * rowbytes;

        frameRaw.p = new unsigned char[imagesize];
        frameRaw.rows = new png_bytep[h * sizeof(png_bytep)];
        for (j=0; j<h; j++)
          frameRaw.rows[j] = frameRaw.p + j * rowbytes;

        frameCur.w = w;
        frameCur.h = h;
        frameCur.p = new unsigned char[imagesize];
        frameCur.rows = new png_bytep[h * sizeof(png_bytep)];
        for (j=0; j<h; j++)
          frameCur.rows[j] = frameCur.p + j * rowbytes;

        processing_start(png_ptr, info_ptr, (void *)&frameRaw, hasInfo, chunkIHDR, chunksInfo);

        while ( !feof(f) )
        {
          id = read_chunk(f, &chunk);

          if (id == id_acTL && !hasInfo && !isAnimated)
          {
            isAnimated = true;
            skipFirst = true;
          }
          else
          if (id == id_fcTL && (!hasInfo || isAnimated))
          {
            if (hasInfo)
            {
              if (!processing_finish(png_ptr, info_ptr))
              {
                frameNext.p = new unsigned char[imagesize];
                frameNext.rows = new png_bytep[h * sizeof(png_bytep)];
                for (j=0; j<h; j++)
                  frameNext.rows[j] = frameNext.p + j * rowbytes;

                if (dop == 2)
                  memcpy(frameNext.p, frameCur.p, imagesize);

                compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
                frameCur.delay_num = delay_num;
                frameCur.delay_den = delay_den;

                frames.push_back(frameCur);

                if (dop != 2)
                {
                  memcpy(frameNext.p, frameCur.p, imagesize);
                  if (dop == 1)
                    for (j=0; j<h0; j++)
                      memset(frameNext.rows[y0 + j] + x0*4, 0, w0*4);
                }
                frameCur.p = frameNext.p;
                frameCur.rows = frameNext.rows;
              }
              else
              {
                delete[] frameCur.rows;
                delete[] frameCur.p;
                delete[] chunk.p;
                break;
              }
            }

            // At this point the old frame is done. Let's start a new one.
            w0 = png_get_uint_32(chunk.p + 12);
            h0 = png_get_uint_32(chunk.p + 16);
            x0 = png_get_uint_32(chunk.p + 20);
            y0 = png_get_uint_32(chunk.p + 24);
            delay_num = png_get_uint_16(chunk.p + 28);
            delay_den = png_get_uint_16(chunk.p + 30);
            dop = chunk.p[32];
            bop = chunk.p[33];

            if (hasInfo)
            {
              memcpy(chunkIHDR.p + 8, chunk.p + 12, 8);
              processing_start(png_ptr, info_ptr, (void *)&frameRaw, hasInfo, chunkIHDR, chunksInfo);
            }
            else
              skipFirst = false;

            if (frames.size() == (skipFirst ? 1 : 0))
            {
              bop = 0;
              if (dop == 2)
                dop = 1;
            }
          }
          else
          if (id == id_IDAT)
          {
            hasInfo = true;
            processing_data(png_ptr, info_ptr, chunk.p, chunk.size);
          }
          else
          if (id == id_fdAT && isAnimated)
          {
            png_save_uint_32(chunk.p + 4, chunk.size - 16);
            memcpy(chunk.p + 8, "IDAT", 4);
            processing_data(png_ptr, info_ptr, chunk.p + 4, chunk.size - 4);
          }
          else
          if (id == id_IEND)
          {
            if (hasInfo && !processing_finish(png_ptr, info_ptr))
            {
              compose_frame(frameCur.rows, frameRaw.rows, bop, x0, y0, w0, h0);
              frameCur.delay_num = delay_num;
              frameCur.delay_den = delay_den;
              frames.push_back(frameCur);
            }
            else
            {
              delete[] frameCur.rows;
              delete[] frameCur.p;
            }
            delete[] chunk.p;
            break;
          }
          else
          if (notabc(chunk.p[4]) || notabc(chunk.p[5]) || notabc(chunk.p[6]) || notabc(chunk.p[7]))
          {
            delete[] chunk.p;
            break;
          }
          else
          if (!hasInfo)
          {
            processing_data(png_ptr, info_ptr, chunk.p, chunk.size);
            chunksInfo.push_back(chunk);
            continue;
          }
          delete[] chunk.p;
        }
        delete[] frameRaw.rows;
        delete[] frameRaw.p;

        if (!frames.empty())
          res = (skipFirst) ? 0 : 1;
      }
    }
    fclose(f);

    for (i=0; i<chunksInfo.size(); i++)
      delete[] chunksInfo[i].p;

    chunksInfo.clear();
    delete[] chunkIHDR.p;
  }

  return res;
}

void save_png(char * szOut, APNGFrame * frame)
{
  FILE * f;
  png_structp  png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  png_infop    info_ptr = png_create_info_struct(png_ptr);

  if (!png_ptr || !info_ptr)
    return;

  if (setjmp(png_jmpbuf(png_ptr)))
  {
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);
    return;
  }

  if ((f = fopen(szOut, "wb")) != 0)
  {
    png_init_io(png_ptr, f);
    png_set_compression_level(png_ptr, 9);
    png_set_IHDR(png_ptr, info_ptr, frame->w, frame->h, 8, 6, 0, 0, 0);
    png_write_info(png_ptr, info_ptr);
    png_write_image(png_ptr, frame->rows);
    png_write_end(png_ptr, info_ptr);
    fclose(f);
  }
  png_destroy_write_struct(&png_ptr, &info_ptr);
  printf("  [libpng");
#ifdef PNG_APNG_SUPPORTED
  printf("+apng");
#endif
  printf(" %s]:  ", PNG_LIBPNG_VER_STRING);
  printf("%s : %dx%d\n", szOut, frame->w, frame->h);
}

int main(int argc, char** argv)
{
  unsigned int i;
  int res;
  char szOut[512];
  std::vector<APNGFrame> frames;

  if (argc <= 1)
  {
    printf("Usage : load6apng input.png\n");
    return 1;
  }

  res = load_apng(argv[1], frames);
  if (res < 0)
    return 1;

  for (i=0; i<frames.size(); i++)
  {
    sprintf(szOut, "test_load6_%03d.png", i+res);
    save_png(szOut, &frames[i]);
    delete[] frames[i].rows;
    delete[] frames[i].p;
  }

  frames.clear();

  printf("\n");
  return 0;
}
