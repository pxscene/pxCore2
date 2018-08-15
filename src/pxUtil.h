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

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wconversion"
#endif

// pxUtil.h
#ifndef PX_UTIL_H
#define PX_UTIL_H
#include "rtFile.h"

#include <vector>


rtString md5sum(rtString &d); //fwd

void    base64_cleanup();

rtError base64_encode(rtData &d, rtString &s);
rtError base64_encode(const unsigned char *data, size_t input_length, rtString &s);

rtError base64_decode(rtString &s, rtData &d);
rtError base64_decode(const unsigned char *data, size_t input_length, rtData &d);

class pxTimedOffscreenSequence
{
public:
  pxTimedOffscreenSequence():mTotalTime(0),mNumPlays(0) {}
  ~pxTimedOffscreenSequence() {}

  void init();
  void addBuffer(pxBuffer &b, double duration);

  uint32_t numFrames()
  {
    return mSequence.size();
  }

  uint32_t numPlays()
  {
    return mNumPlays;
  }

  void setNumPlays(uint32_t numPlays)
  {
    mNumPlays = numPlays;
  }

  pxOffscreen &getFrameBuffer(int frameNum)
  {
    return mSequence[frameNum].mOffscreen;
  }

  double getDuration(int frameNum)
  {
    return mSequence[frameNum].mDuration;
  }

  double totalTime()
  {
    return mTotalTime;
  }

private:
  struct entry
  {
    pxOffscreen mOffscreen;
    double mDuration;
  };

  std::vector<entry> mSequence;
  double mTotalTime;
  uint32_t mNumPlays;

}; // CLASS - pxTimedOffscreenSequence


typedef enum pxImageType_
{
  PX_IMAGE_JPG,      // Joint Photographic Experts Group - .jpeg or .jpg
  PX_IMAGE_PNG,      // Portable Network Graphics
  PX_IMAGE_GIF,      // Graphics Interchange Format
  PX_IMAGE_TIFF,     // Tagged Image File Format
  PX_IMAGE_BMP,      // Microsoft Bitmap format
  PX_IMAGE_WEBP,     // Google WebP format, a type of .riff file
  PX_IMAGE_ICO,      // Microsoft icon format
  PX_IMAGE_SVG,      // Scalable Vector Graphics
  PX_IMAGE_INVALID,  // unidentified image types.
}
pxImageType;


pxImageType getImageType(const uint8_t* data, size_t len);
rtString imageType2str(pxImageType t);

rtError pxLoadImage( const char* imageData, size_t imageDataSize, pxOffscreen& o, int32_t w = 0, int32_t h = 0, float sx = 1.0f, float sy = 1.0f);
rtError pxLoadImage( const char* filename,                        pxOffscreen& b, int32_t w = 0, int32_t h = 0, float sx = 1.0f, float sy = 1.0f);
rtError pxStoreImage(const char* filename, pxOffscreen& b);

bool pxIsPNGImage(rtData d);
bool pxIsPNGImage(const char* imageData, size_t imageDataSize);

rtError pxLoadAImage(const char* imageData, size_t imageDataSize,
  pxTimedOffscreenSequence &s);
rtError pxLoadAPNGImage(const char *imageData, size_t imageDataSize,
  pxTimedOffscreenSequence &s);

rtError pxLoadPNGImage(const char* imageData, size_t imageDataSize, 
                       pxOffscreen& o);
rtError pxLoadPNGImage(const char* filename, pxOffscreen& o);
rtError pxStorePNGImage(const char* filename, pxOffscreen& b,
                        bool grayscale = false, bool alpha=true);

rtError pxStorePNGImage(pxOffscreen& b, rtData& pngData);

#if 0
bool pxIsJPGImage(const char* imageData, size_t imageDataSize);
rtError pxStoreJPGImage(const char* filename, pxBuffer& b);
#endif

#ifdef ENABLE_LIBJPEG_TURBO
rtError pxLoadJPGImageTurbo(const char* buf, size_t buflen, pxOffscreen& o);
#endif //ENABLE_LIBJPEG_TURBO

rtError pxLoadJPGImage(const char* imageData, size_t imageDataSize, pxOffscreen& o);
rtError pxLoadJPGImage(const char* filename, pxOffscreen& o);


rtError pxLoadSVGImage(const char* buf, size_t buflen, pxOffscreen& o, int w = 0, int h = 0, float sx = 1.0f, float sy = 1.0f);
rtError pxLoadSVGImage(const char* filename,           pxOffscreen& o, int w = 0, int h = 0, float sx = 1.0f, float sy = 1.0f);
rtError pxStoreSVGImage(const char* filename, pxBuffer& b); // NOT SUPPORTED


#endif //PX_UTIL_H

#ifdef PX_PLATFORM_MAC
#pragma clang diagnostic pop
#endif
