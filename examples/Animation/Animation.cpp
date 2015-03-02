// Animation Example CopyRight 2007-2009 John Robinson
// Demonstrates using the simple animation support in pxWindow

#include "pxCore.h"

#include "pxEventLoop.h"
#include "pxWindow.h"

#include "pxOffscreen.h"

pxEventLoop eventLoop;

int gStep = 0;
int gDirection = 1;
int gOffset = 0;

const int gFPS = 15;
const int gDuration = 2;

void drawBackground(pxBuffer& b)
{
    // Fill the buffer with a simple pattern as a function of f(x,y)
    int w = b.width();
    int h = b.height();

    gOffset += (gStep * gDirection);

    if (gOffset >= pxMin<int>(w, h)) gDirection = -1;
    else if (gOffset < 0) gDirection = 1;

    for (int y = 0; y < h; y++)
    {
        pxPixel* p = b.scanline(y);
        for (int x = 0; x < w; x++)
        {
            p->r = pxClamp<int>(128-gOffset+x+y, 255);
            p->g = pxClamp<int>(128-gOffset+y,   255);
            p->b = pxClamp<int>(128-gOffset+x,   255);
            p++;
        }
    }
}

class myWindow: public pxWindow
{
private:
    void onCreate()
    {
        // Kick off the animation
        setAnimationFPS(gFPS);
    }

    // Event Handlers - Look in pxWindow.h for more
    void onCloseRequest()
    {
        // When someone clicks the close box no policy is predefined.
        // so we need to explicitly tell the event loop to exit
        eventLoop.exit();
    }

    void onSize(int w, int h)
    {
        // When ever the window resizes we (re)allocate a buffer 
        // big enough for the entire client area and draw our 
        // pattern into it
        mTexture.init(w, h);

        // recalculate how far to step each frame
        gStep = (int)((pxMin<int>(w, h) / (double)gFPS) / (double)gDuration);
        drawBackground(mTexture);
    }

    void onDraw(pxSurfaceNative s)
    {
        // Draw the texture into this window
        mTexture.blit(s);
    }

    void onAnimationTimer()
    {
	    // The background changes each time we call drawBackground
	    // so just call it whenever the animation time goes off.
        drawBackground(mTexture);
        invalidateRect();
    }

    pxOffscreen mTexture;
};

int pxMain()
{
    myWindow win;

    win.init(10, 64, 640, 480);
    win.setTitle("Animation");
    win.setVisibility(true);

    eventLoop.run();

    return 0;
}


