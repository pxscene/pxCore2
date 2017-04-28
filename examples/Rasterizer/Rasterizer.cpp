#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxKeycodes.h"
#include "pxOffscreen.h"
#include "pxTimer.h"
#include "rtData.h"

#include "xs_StringUtil.h"

#include "stdio.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifndef PX_PLATFORM_WIN
#include <unistd.h>
#else
#include <io.h>
#define open _open
#define close _close
#define read _read
#define fstat _fstat
#define stat _stat
#endif

#define JOHNS

#ifdef JOHNS
#include "pxCanvas.h"
#endif

//#endif

pxEventLoop eventLoop;


void mandel(pxBuffer& b,                       
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
      cx = xmin + ix * ( xmax - xmin ) / ( nx - 1 );
      x = y = x2 = y2 = 0.0;
      iter = 0;
      while( iter < maxiter && ( x2 + y2 ) <= 9.0 /* 10000.0 ??? */ )
      { 
        temp = x2 - y2 + cx;
        y = 2 * x * y + cy;
        x = temp;
        x2 = x * x;
        y2 = y * y;
        iter++;
      }
            	      
      double v;
                        
      if (iter >= maxiter) 
        v = 1.0;
      else 
        v = (double)iter/(double)maxiter;
            
      pxPixel* p = b.pixel(ix, iy);

      if (v >= 1.0)
      {
        p->r = p->b = p->g = 0;
        p->a = 32;
      }
      else
      {
        p->r = (uint8_t)(255*v);
        p->b = (uint8_t)(80*(1.0-v));
        p->g = (uint8_t)(255*(1.0-v));
        p->a = 255;
      }
    }
  }
}

void drawBackground(pxBuffer& b)
{
  int w = b.width();
  int h = b.height();

  for (int y = 0; y < h; y++)
  {
    pxPixel* p = b.scanline(y);
    for (int x = 0; x < w; x++)
    {
      p->r = pxClamp<int>(x + y, 0, 255);
      p->g = pxClamp<int>(y,     0, 255);
      p->b = pxClamp<int>(x,     0, 255);
      p++;
    }
  }
}

bool rtLoadFileUnix(const char* file, rtData& data)
{
  bool e = false;
    
  struct stat st;
    
  int f = open(file, O_RDONLY);
  if (f != -1)
  {
    fstat(f, &st);
        
    unsigned long sizeToUse = st.st_size; // space for null terminstor
        
    data.initWithLength(sizeToUse+1);
    if (data.bytes())
    {
      unsigned long bytesRead = read(f, data.bytes(), sizeToUse);
      unsigned char* bytes = (unsigned char*)data.bytes();
      bytes[bytesRead] = 0; // NULL terminate it for convenience
      e = true;
    }
    close(f);
  }
  return e;
}


class myWindow: public pxWindow
{
public:
    
  myWindow(): mShapeIndex(6)
  {
    gQuality = 1;
    gTextureRotate = 0;
    gAnimating = false;        
    gRotateDelta = 1 * 3.1415 / 180;

    // Init Texture
    textureOffscreen.init(128,96);
    mandel(textureOffscreen, -2, 1, -1.5, 1.5, 16);

    // Init Offscreen
    offscreen.init(800, 800);

#ifdef JOHNS
    canvasPtr = new pxCanvas;
    pxCanvas& canvas = *canvasPtr;

    canvas.initWithBuffer(&offscreen);
    canvas.setAlpha(1.0);
    canvas.setAlphaTexture(true);
    canvas.setTexture(&textureOffscreen);
#endif

		loadShape();
  }

  ~myWindow()
  {
#ifdef JOHNS
    delete canvasPtr;
#endif
  }


  void loadShape()
	{
    if (mShapeIndex >= 1 && mShapeIndex <= 4)
    {
      readFile(mShapeIndex);
    }
    else if (mShapeIndex == 5)
    {
      testLargeFill();
    }
#if 0
		else
		{
			testText();
		}
#endif
	}
	
	void drawFrame()
  {
    // Draw the current shape
    drawBackground(offscreen);
#ifdef JOHNS

    pxMatrix m;
    m.identity();
    m.rotate(gTextureRotate);
    canvasPtr->setTextureMatrix(m);

		if (mShapeIndex != 6)
			canvasPtr->fill();
		else
		{
#if 1
//			canvasPtr->setFont(L"Arial");
//			canvasPtr->setFontSize(10);
//			canvasPtr->drawText(L"Hello john", 100, 100);
#else
			pxBuffer* b = canvasPtr->texture();
			canvasPtr->setTexture(NULL);
			//canvasPtr->setYOversample(4);
			canvasPtr->setFillColor(0, 255, 0);
#if 0
			canvasPtr->moveTo(1, 1);
			canvasPtr->lineTo(2, 1);
			canvasPtr->lineTo(2, 2.68);
			canvasPtr->lineTo(1, 2.68);
			canvasPtr->lineTo(1,1);
#endif
			for (int i = 0; i <= 10; i++)
			{
        canvasPtr->newPath();
        double d = (double)2 + ((double)i * 0.1);
        canvasPtr->rectangle(2+ (i*1)+ 1, 1, 2 + (i*1) + 2, d);
//			canvasPtr->rectangle(2+ (i*1)+ 1, 1, 2 + (i*1) + 2, 2.3);
//			printf("bottom: %f\n", d);
        canvasPtr->closePath();
        canvasPtr->fill();
			}

			for (int i = 0; i <= 10; i++)
			{
        canvasPtr->newPath();
        double d = (double)6 + ((double)i * 0.1);
        canvasPtr->rectangle(2+ (i*1)+ 1, d, 2 + (i*1) + 2, 8);
//			printf("bottom: %f\n", d);
        canvasPtr->closePath();
        canvasPtr->fill();
			}
			canvasPtr->setStrokeColor(255, 0, 0);
			canvasPtr->setStrokeWidth(1);
			//canvasPtr->stroke();
			canvasPtr->setTexture(b);
#endif
		}
#endif
    invalidateRect();

  }

  void onCreate()
  {
    drawFrame();
  }

  void onAnimationTimer()
  {
		gTextureRotate += gRotateDelta;
		drawFrame();
  }

  void onSize(int32_t w, int32_t h)
  {
    offscreen.init(w, h);
    drawFrame();
  }

  void onKeyDown(uint32_t keyCode, uint32_t flags)
  {
//      printf("onKeyDown\n");
    switch(keyCode)
    {
    case PX_KEY_LEFT:
    {
      // Rotate Texture
#ifdef JOHNS
      pxMatrix m;
      m.identity();
      m.rotate(gTextureRotate+=gRotateDelta);
      canvasPtr->setTextureMatrix(m);
      canvasPtr->clear();
#endif
    }
    break;
    case PX_KEY_RIGHT:
    {
      // Rotate Texture
#ifdef JOHNS
      pxMatrix m;
      m.identity();
      m.rotate(gTextureRotate-=gRotateDelta);
      canvasPtr->setTextureMatrix(m);
      canvasPtr->clear();
#endif
    }
    break;
    case PX_KEY_UP:
      // Increase Quality
      gQuality++;
      if (gQuality > 4) gQuality = 4;
#ifdef JOHNS
      canvasPtr->setYOversample(1<<gQuality);
      printf("Oversample => %d\n", 1 << gQuality);
#endif
      break;    
    case PX_KEY_DOWN:
      // Decrease Quality
      gQuality--;
      if (gQuality < 0) gQuality = 0;
#ifdef JOHNS
      canvasPtr->setYOversample(1<<gQuality);
      printf("Oversample => %d\n", 1 << gQuality);
#endif
      break;
    case PX_KEY_ONE:
    case PX_KEY_TWO:
    {
//          printf("cycle\n");
      // Cycle throught Shapes
      mShapeIndex = (keyCode == PX_KEY_ONE)?mShapeIndex-1:
        mShapeIndex+1;
      if (mShapeIndex < 1) mShapeIndex = 6;
      if (mShapeIndex > 6) mShapeIndex = 1;
      loadShape();
      drawFrame();
    }
    break;
    case PX_KEY_A:
      // Toggle Texture Animation
      gAnimating = !gAnimating;
      setAnimationFPS(gAnimating?70:0);
      break;
    case PX_KEY_T:
    {
      // Toogle Filtering
#ifdef JOHNS
      if (!canvasPtr->texture())
      {
        canvasPtr->setTexture(&textureOffscreen);
      }
      else canvasPtr->setTexture(NULL);
#endif
    }
    break;
    case PX_KEY_F:
    {
      // Time Filling Shape
      //double start = pxMilliseconds();
      // drawFrame
#ifdef JOHNS
      canvasPtr->fill(true);
#endif
      // double end = pxMilliseconds();
      // printf("Elapsed Time %gms FPS: %g\n", end-start, 1000/(end-start));
    }
    break;
    case PX_KEY_C:
      // Toggle Texture Clamp
#ifdef JOHNS
      canvasPtr->setTextureClamp(!canvasPtr->textureClamp());
#endif
      break;
    case PX_KEY_B:
      // Toggle Bilerp
#ifdef JOHNS
      canvasPtr->setBiLerp(!canvasPtr->biLerp());
#endif
      break;
    case PX_KEY_M:
      // Toggle Alpha Texture
#ifdef JOHNS
      canvasPtr->setAlphaTexture(!canvasPtr->alphaTexture());
#endif
      break;
    default:
      break;
    }
    drawFrame();
  }

  void onCloseRequest()
  {
    eventLoop.exit();
  }

  void onDraw(pxSurfaceNative s)
  {
    offscreen.blit(s);
  }


  void testLargeFill()
  {
    // Draw a filled 640x480
#ifdef JOHNS
    pxCanvas& canvas = *canvasPtr;

    canvas.newPath();
    canvas.moveTo(0, 0);
    canvas.lineTo(640, 0);
    canvas.lineTo(640, 480);
    canvas.lineTo(0, 480);
    canvas.lineTo(0, 0);
    canvas.fill();
#endif
  }

  void testText()
  {
    // Draw a filled 640x480
#ifdef JOHNS
    pxCanvas& canvas = *canvasPtr;

//		canvas.drawText(L"Hello John", 100, 100);
#endif
  }


  void readFile(int test)
  {
#ifdef JOHNS
    pxCanvas& canvas = *canvasPtr;
#endif

    uint32_t len = 0;

    char *f;

    int pathNum = 0;
    rtData d;
    {
      {

#if defined(PX_PLATFORM_WIN)
        if (rtLoadFileUnix("c:\\_data\\complex.data", d))
#elif defined(PX_PLATFORM_MAC)
          if (rtLoadFileUnix("/Users/Shared/complex.data", d))
#elif defined(PX_PLATFORM_X11)
            if (rtLoadFileUnix("./complex.data", d))
#endif
            {
              f = (char*)d.bytes();
              len = d.length();
                
              // char *of = f;
              char *ef = f + len;
              if (f)
              {
                while (f && f<ef && *f)
                {
                  pathNum++;
                        
                  if (pathNum == test) 
                  {
#ifdef JOHNS
                    canvas.newPath();
#endif
                  }

                  bool first = true;
                  while (f && *f!=0 && *f!='\r' && *f!='\n')
                  {
                    real64 x, y;
                    char str1[256], str2[256];
                    f = xs_NextToken(f, str1, 256, ','); 
                    if (f&&f[0]==',') f++;
                    f = xs_NextToken(f, str2, 256, ','); 
                    if (f&&f[0]==',') f++;
                    xs_atod(str1, &x, 256);
                    xs_atod(str2, &y, 256);
                    if (pathNum == test) 
                    {
                      if (first) 
                      {
#ifdef JOHNS
                        canvas.moveTo(x,y);
#endif
                        first = false;
                      }
                      else
                      {
#ifdef JOHNS
                        canvas.lineTo(x,y);
#endif
                      }
                    }
                  }
                        
                  if (f)
                    while (f && *f && (*f=='\r' || *f=='\n')) 	f++;
                }
                    
              }
            }
      }
    }
  }

private:
  int mShapeIndex;
  int gQuality;
  double gTextureRotate;
  bool gAnimating;

  double gRotateDelta;

  pxOffscreen textureOffscreen;
  pxOffscreen offscreen;

#ifdef JOHNS
  pxCanvas* canvasPtr;
#endif
};

void usage()
{
  printf("Keys\n");
  printf("----\n");
  printf("1  - Previous Shape\n");
  printf("2  - Next Shape\n");
  printf("T  - Toggle Texture Map\n");
  printf("M  - Toggle Alpha Texture\n");
  printf("C  - Toggle Texture Clamp\n");
  printf("B  - Toggle BiLerp\n");
  printf("A  - Toggle Animation\n");
  printf("F  - Time Fill\n");
  printf("Up - Increase Oversampling\n");
  printf("Dn - Decrease Oversampling\n");
  printf("\n");
}

int pxMain(int, char **)
{
  usage();

  myWindow win;

  win.init(10, 64, 800, 600);
  win.setTitle("Rasterizer");
  win.setVisibility(true);

  eventLoop.run();

  return 0;
}


