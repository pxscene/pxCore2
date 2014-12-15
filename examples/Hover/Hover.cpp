// Hover Example CopyRight 2007-2009 John Robinson
// Demonstrates detecting when the mouse has entered the window
// as well as detecting when it has left the window using the
// new onMouseLeave event
//
// Useful for reliably clearing hover effects within
// the view.

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"

#include <stdio.h>

pxEventLoop eventLoop;

void drawBackground(pxBuffer& b, bool hovered)
{
    // Fill the buffer with a simple pattern as a function of f(x,y)
    int w = b.width();
    int h = b.height();

    for (int y = 0; y < h; y++)
    {
        pxPixel* p = b.scanline(y);
        if (hovered)
        {
            for (int x = 0; x < w; x++)
            {
                p->r = pxClamp<int>(x+y, 255);
                p->g = pxClamp<int>(y,   255);
                p->b = pxClamp<int>(x,   255);
                p++;
            }
        }
        else
        {
            for (int x = 0; x < w; x++)
            {
                p->b = pxClamp<int>(x+y, 255);
                p->r = pxClamp<int>(y,   255);
                p->g = pxClamp<int>(x,   255);
                p++;
            }
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

    void onCreate()
    {
        hovered = false;
    }

    void onSize(int newWidth, int newHeight)
    {
        // When ever the window resizes (re)allocate a buffer big 
	    // enough for the entire client area and draw our pattern into it
	    mTexture.init(newWidth, newHeight);
	    drawBackground(mTexture, hovered);
    }

    void onDraw(pxSurfaceNative s)
    {
        drawBackground(mTexture, hovered);
        // Draw the texture into this window
	    mTexture.blit(s);
    }

    void onMouseMove(int x, int y)
    {
        if (!hovered)
        {
            hovered = true;
            invalidateRect();
        }
    }

    void onMouseLeave()
    {
        if (hovered)
        {
            hovered = false;
            invalidateRect();
        }
    }

    pxOffscreen mTexture;

    bool hovered;
};

int pxMain()
{
    myWindow win;

    win.init(10, 64, 640, 480);
    win.setTitle("Hover");
    win.setVisibility(true);

    eventLoop.run();

    return 0;
}


