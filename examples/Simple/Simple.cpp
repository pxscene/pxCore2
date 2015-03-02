// Simple Example CopyRight 2007-2009 John Robinson
// Demonstrates filling a pxBuffer with a simple pattern
// and displaying it in a window.

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"

#include <stdio.h>

pxEventLoop eventLoop;

void drawBackground(pxBuffer& b)
{
  // Fill the buffer with a simple pattern as a function of f(x,y)
  int w = b.width();
  int h = b.height();

  for (int y = 0; y < h; y++)
  {
    pxPixel* p = b.scanline(y);
    for (int x = 0; x < w; x++)
    {
      p->r = pxClamp<int>(x+y, 255);
      p->g = pxClamp<int>(y,   255);
      p->b = pxClamp<int>(x,   255);
      p++;
    }
  }
}

class myWindow: public pxWindow
{
private:
  // Event Handlers - Look in pxWindow.h for more
  void onCloseRequest()
  {
    // When someone clicks the close box no policy is predefined.
    // so we need to explicitly tell the event loop to exit
    eventLoop.exit();
  }

  void onSize(int newWidth, int newHeight)
  {
    // When ever the window resizes (re)allocate a buffer big
    // enough for the entire client area and draw our pattern into it
    mTexture.init(newWidth, newHeight);
    drawBackground(mTexture);
  }

  void onDraw(pxSurfaceNative s)
  {
    // Draw the texture into this window
    mTexture.blit(s);
  }

  pxOffscreen mTexture;
};

int pxMain()
{
  myWindow win;

  win.init(10, 64, 640, 480);
  win.setTitle("Simple");
  win.setVisibility(true);

  eventLoop.run();

  return 0;
}


