/*
 * load5apng.cpp
 *
 * loads APNG file, saves all frames as PNG.
 * no frames composition.
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

unsigned char header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
unsigned char footer[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};
char szOut[512];

void recalc_crc(unsigned char * p, unsigned int size)
{
  unsigned int crc = crc32(0, Z_NULL, 0);
  crc = crc32(crc, p + 4, size - 8);
  png_save_uint_32(p + size - 4, crc);
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

FILE * processing_start(int num, bool hasInfo, CHUNK & chunkIHDR, std::vector<CHUNK>& chunksInfo)
{
  FILE * f;

  sprintf(szOut, "test_load5_%03d.png", num);

  if ((f = fopen(szOut, "wb")) != 0)
  {
    fwrite(&header, 1, 8, f);
    fwrite(chunkIHDR.p, 1, chunkIHDR.size, f);

    if (hasInfo)
      for (unsigned int i=0; i<chunksInfo.size(); i++)
        fwrite(chunksInfo[i].p, 1, chunksInfo[i].size, f);
  }
  return f;
}

void processing_data(FILE * f, unsigned char * p, unsigned int size)
{
  fwrite(p, 1, size, f);
}

void processing_finish(FILE * f, unsigned int w, unsigned int h)
{
  fwrite(&footer, 1, 12, f);
  fclose(f);

  printf("  [libpng");
#ifdef PNG_APNG_SUPPORTED
  printf("+apng");
#endif
  printf(" %s]:  ", PNG_LIBPNG_VER_STRING);
  printf("%s : %dx%d\n", szOut, w, h);
}

int load_apng(char * szIn)
{
  FILE * f;
  FILE * f2;
  unsigned int id, i, w, h, w0, h0, x0, y0;
  unsigned int delay_num, delay_den, dop, bop;
  unsigned char sig[8];
  CHUNK chunk;
  CHUNK chunkIHDR;
  std::vector<CHUNK> chunksInfo;
  bool isAnimated = false;
  bool skipFirst = false;
  bool hasInfo = false;
  int num = 0;
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

        f2 = processing_start(num++, hasInfo, chunkIHDR, chunksInfo);

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
              processing_finish(f2, w0, h0);

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
              recalc_crc(chunkIHDR.p, chunkIHDR.size);
              f2 = processing_start(num++, hasInfo, chunkIHDR, chunksInfo);
            }
            else
              skipFirst = false;
          }
          else
          if (id == id_IDAT)
          {
            hasInfo = true;
            processing_data(f2, chunk.p, chunk.size);
          }
          else
          if (id == id_fdAT && isAnimated)
          {
            png_save_uint_32(chunk.p + 4, chunk.size - 16);
            memcpy(chunk.p + 8, "IDAT", 4);
            recalc_crc(chunk.p + 4, chunk.size - 4);
            processing_data(f2, chunk.p + 4, chunk.size - 4);
          }
          else
          if (id == id_IEND)
          {
            if (hasInfo)
              processing_finish(f2, w0, h0);

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
            processing_data(f2, chunk.p, chunk.size);
            chunksInfo.push_back(chunk);
            continue;
          }
          delete[] chunk.p;
        }
        if (num != 0)
          res = 0;
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

int main(int argc, char** argv)
{
  if (argc <= 1)
  {
    printf("Usage : load5apng input.png\n");
    return 1;
  }

  int res = load_apng(argv[1]);

  printf("\n");
  return res;
}
