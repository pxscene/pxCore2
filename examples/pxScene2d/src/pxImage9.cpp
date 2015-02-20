// pxCore CopyRight 2007-2015 John Robinson
// pxImage9.cpp

#include "rtString.h"
#include "rtRefT.h"
#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxUtil.h"
#include "pxScene2d.h"
#include "pxImage9.h"
#include "pxContext.h"
#include "pxFileDownloader.h"

extern pxContext context;
extern int gFileDownloadsPending;

rtError pxImage9::url(rtString& s) const { s = mURL; return RT_OK; }
rtError pxImage9::setURL(const char* s) { 
  mURL = s;
  loadImage(mURL);
  return RT_OK;
}

void pxImage9::draw() {
  context.drawImage9(mw, mh, mx1, my1, mx2, my2, mOffscreen);
}

void pxImage9::loadImage(rtString url)
{
  char* s = url.cString();
  const char *result = strstr(s, "http");
  int position = result - s;
  if (position == 0 && strlen(s) > 0)
  {
    pxFileDownloadRequest* downloadRequest = 
      new pxFileDownloadRequest(s, this);
    gFileDownloadsPending++;
    pxFileDownloader::getInstance()->addToDownloadQueue(downloadRequest);
  }
  else 
  {
    if (pxLoadImage(s, mOffscreen) != RT_OK)
      rtLogWarn("image load failed");
    else
      rtLogDebug("image %d, %d", mOffscreen.width(), mOffscreen.height());
    mw = mOffscreen.width();
    mh = mOffscreen.height();
  }
}

void pxImage9::onFileDownloadComplete(pxFileDownloadRequest* downloadRequest)
{
  if (pxLoadImage(downloadRequest->getDownloadedData(),
                  downloadRequest->getDownloadedDataSize(), 
                  mOffscreen) != RT_OK)
  {
    rtLogError("Image Decode Failed: %s", downloadRequest->getFileURL().cString());
  }
  else
  {
    mw = mOffscreen.width();
    mh = mOffscreen.height();
  }
}

rtDefineObject(pxImage9, pxObject);
rtDefineProperty(pxImage9, url);
rtDefineProperty(pxImage9, x1);
rtDefineProperty(pxImage9, y1);
rtDefineProperty(pxImage9, x2);
rtDefineProperty(pxImage9, y2);
