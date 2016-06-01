// Mandelbrot Example CopyRight 2007-2009 John Robinson
// Demonstrates filling a pxBuffer with the mandelbrot set
// and drawing it into a window.

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"

#include "pxOffscreen.h"

pxEventLoop eventLoop;

void mandel( pxBuffer& b,
	     long double xmin, long double xmax,
	     long double ymin, long double ymax,
	     unsigned maxiter
	   )
{ 
    int nx = b.width();
    int ny = b.height();
    
    short ix, iy;
    unsigned iter;
    long double cx, cy;
    long double x, y, x2, y2, temp;

    for( iy = 0; iy < ny; iy ++ )
    { 
        cy = ymin + iy * ( ymax - ymin ) / ( ny - 1 );
 
        for( ix = 0; ix < nx; ix ++ )
    	{ 

            // for a given pixel calculate if that point is in
            // the mandelbrot set
            cx = xmin + ix * ( xmax - xmin ) / ( nx - 1 );
            x = y = x2 = y2 = 0.0;
            iter = 0;

            // use an escape radius of 9.0
            while( iter < maxiter && ( x2 + y2 ) <= 9.0 )
	        { 
                temp = x2 - y2 + cx;
                y = 2 * x * y + cy;
                x = temp;
                x2 = x * x;
                y2 = y * y;
                iter++;
	        }

            pxPixel* p = b.pixel(ix, iy);


            // Select a color based on how many iterations it took to escape
            // the mandelbrot set
            if (iter >= maxiter)
            {
                p->r = p->b = p->g = 0;
                p->a = 32;
            }
            else
            {
                double v = (double)iter/(double)maxiter;
		        p->r = (unsigned char)(255*v);
		        p->b = (unsigned char)(80*(1.0-v));
		        p->g = (unsigned char)(255*(1.0-v));
		        p->a = 255;
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
        eventLoop.exit();
    }

    void onSize(int w, int h)
    {
        // Fill an offscreen buffer (pxOffscreen) with the Mandelbrot pattern
        mTexture.init(w, h);  // Initialize the offscreen to be the desired size
        mandel(mTexture, -2, 1, -1.5, 1.5, 16);
    }

    void onDraw(pxSurfaceNative s)
    {
        // Draw the texture into this window
        mTexture.blit(s);
    }

    pxOffscreen mTexture;
};

int pxMain(int argc, char* argv[])
{
    myWindow win;

    win.init(10, 64, 640, 480);
    win.setTitle("Mandelbrot");
    win.setVisibility(true);

    eventLoop.run();

    return 0;
}


