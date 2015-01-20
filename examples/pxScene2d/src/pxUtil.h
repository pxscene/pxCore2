// pxCore CopyRight 2007-2015 John Robinson
// pxUtil.h

#include "rtFile.h"
//#include "pxCore.h"

rt_error pxLoadImage(const char* filename, pxOffscreen& b);
rt_error pxStoreImage(const char* filename, pxBuffer& b);

rt_error pxLoadPNGImage(const char* filename, pxOffscreen& o);
rt_error pxStorePNGImage(const char* filename, pxBuffer& b, bool grayscale = false, bool alpha=true);

rt_error pxLoadJPGImage(const char* filename, pxOffscreen& o);
rt_error pxStoreJPGImage(const char* filename, pxBuffer& b);


