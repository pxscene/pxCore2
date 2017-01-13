// pxCore CopyRight 2007-2015 John Robinson
// pxUtil.h

#include "rtFile.h"

rtError pxLoadImage(const char* imageData, size_t imageDataSize, 
                    pxOffscreen& o);
rtError pxLoadImage(const char* filename, pxOffscreen& b);
rtError pxStoreImage(const char* filename, pxOffscreen& b);

//bool pxIsPNGImage(const char* imageData, size_t imageDataSize);
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
rtError pxInitializeJPGImageTurbo();
rtError pxCleanupJPGImageTurbo();
rtError pxLoadJPGImageTurbo(const char* buf, size_t buflen, pxOffscreen& o);
#endif //ENABLE_LIBJPEG_TURBO
rtError pxLoadJPGImage(const char* imageData, size_t imageDataSize, 
                       pxOffscreen& o);
rtError pxLoadJPGImage(const char* filename, pxOffscreen& o);

