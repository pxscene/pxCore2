// Simple Example CopyRight 2007-2009 John Robinson
// Demonstrates utilzing native drawing primitives in
// a pxWindow

// Notes
// Click to draw a circle

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

pxEventLoop eventLoop;

#ifdef PX_PLATFORM_WIN
// Since this is a native drawing examnple
// need some random windows specific things
#include <tchar.h>
#elif defined(PX_PLATFORM_MAC)
#include <CoreGraphics/CoreGraphics.h>
#endif

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

    // Now do some native Drawing

#if defined(PX_PLATFORM_WIN)
    // On Windows pxSurfaceNative is a HDC
    TCHAR *t = _T("Some Native Windows Text");
    ::SetBkMode(s, TRANSPARENT);
    ::ExtTextOut(s, 100, 100, 0, NULL, t, (UINT)_tcslen(t), NULL);
    ::MoveToEx(s, 100, 100, NULL);
    ::LineTo(s, 200, 200);
#elif defined(PX_PLATFORM_MAC)
    // On Mac pxSurfaceNative is a CGContextRef
    CGContextBeginPath((CGContextRef)s);
    CGContextMoveToPoint((CGContextRef)s,100,100);
    CGContextAddLineToPoint((CGContextRef)s, 200, 200);
    CGContextStrokePath((CGContextRef)s);
#elif defined(PX_PLATFORM_X11)
    // On X11 pxSurfaceNative is a ptr to a structure
    // Please see pxBufferNative.h for its definition
    XTextItem item;
    item.chars = "Some Native X11 Text";
    item.nchars = strlen(item.chars);
    item.delta = 0;
    item.font = None;

    XDrawText(s->display, s->drawable, s->gc,
              100, 100, &item, 1);
    XDrawLine(s->display, s->drawable, s->gc,
              100, 100, 200, 200);

#endif
  }

  void onMouseUp(int x, int y, unsigned long flags)
  {
    // Demonstrate doing some native drawing outside of the paint loop
    pxSurfaceNative s;
    if (PX_OK == beginNativeDrawing(s))
    {
#if defined(PX_PLATFORM_WIN)
      HGDIOBJ oldBrush = SelectObject(s, ::GetStockObject(NULL_BRUSH));
      Ellipse(s, x-50, y-50, x+50, y+50);
      SelectObject(s, oldBrush);
#elif defined(PX_PLATFORM_MAC)
      // On Mac pxSurfaceNative is a CGContextRef
      // TODO do drawing outside of onDraw event
#elif defined(PX_PLATFORM_X11)
      XDrawArc(s->display, s->drawable, s->gc,
               x-50, y-50, 100, 100, 0, 360*64);
#endif
      endNativeDrawing(s);
    }
  }

  pxOffscreen mTexture;
};

int pxMain()
{
  myWindow win;

  win.init(10, 64, 640, 480);
  win.setTitle("NativeDrawing");
  win.setVisibility(true);

  eventLoop.run();

  return 0;
}


