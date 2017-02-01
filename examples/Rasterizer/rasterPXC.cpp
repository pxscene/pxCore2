#include <stdio.h>
#include <ctime>
#include <sys/time.h>

#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"
#include "pxTimer.h"
#include "pxCanvas.h"

pxEventLoop eventLoop;


//############ CPU Load #########

static struct timeval  wall_start;
static struct timeval utime_start;
static struct timeval stime_start;

static double avgRaster_count = 0.0;
static double avgRaster_sigma = 0.0;

//##############################

extern int  gFrameNumber;
extern bool gDirectRender;

int gCount60 = 0;
int  gFpsCount     = 0;
int  gOversampleY  = 1;
int  gUseSquares   = -1; // default - unset
int  gSquareSize   = 32;

int  size_w = gSquareSize, size_h = gSquareSize; //(default)
int  screen_w = 1280,        screen_h = 720;

bool gFormatCSV       = false;
bool gUseMandel       = false;
bool gUseRotation     = false;
bool gUseAnimation    = false;
bool gShowWork        = false;
bool gSkipBlending    = false;
bool gSkipClearscreen = false;

bool gPerfTiming      = false;

double gAppStart_ms    = 0.0;
double gExitTimeout_ms = -1; // default - unset
double gExitIterations = 0;

pxColor gFgColor = pxBlue;
pxColor gBgColor = pxGray;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void mandel(pxBuffer& b,
            long double xmin, long double xmax,
            long double ymin, long double ymax,
            unsigned maxiter)
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

      double v = (iter >= maxiter) ? 1.0 : (double)iter/(double)maxiter;

      pxPixel* p = b.pixel(ix, iy);

      if (v >= 1.0)
      {
        p->r = p->b = p->g = 0;
        p->a = 32;
      }
      else
      {
        p->r = (uint8_t)(255 * v);
        p->b = (uint8_t)( 80 * (1.0-v));
        p->g = (uint8_t)(255 * (1.0-v));
        p->a = 255;
      }
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

class myWindow: public pxWindow
{
public:

  myWindow()
  {
    gTextureRotate = 0;
    gRotateDelta   = 1.0f * 3.1415f / 180.0f;

    // Init Offscreen
    offscreen.initWithColor(screen_w, screen_h, pxBlue);

    canvasPtr = new pxCanvas;
    pxCanvas& canvas = *canvasPtr;

    if(gDirectRender)
    {
      canvas.initWithBuffer(&offscreen);
      canvas.setAlpha(1.0);
    }

    if(gUseMandel)
    {
      // Mandel Texture
      mandelTex.init(size_w, size_w);
      mandel(mandelTex, -2, 1, -1.5, 1.5, 16);
    }
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  ~myWindow()
  {
    delete canvasPtr;
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  inline void drawFrame()
  {
    // required
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  inline void drawFrame(pxSurfaceNative s)
  {
    if(!s || !canvasPtr)
    {
      return; // not ready yet
    }

    double now = pxMilliseconds();

    static pxBuffer surf;

    surf.setBase(   s->windowBase   );
    surf.setWidth(  s->windowWidth  );
    surf.setHeight( s->windowHeight );
    surf.setStride( s->windowStride );

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // UPDATE / DRAW
    static bool once = true;
    static pxMatrix m;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if(once)
    {
      once = false;

      size_w = gSquareSize;
      size_h = size_w;

      canvasPtr->initWithBuffer(&surf);

      if(!gUseMandel)
      {
        canvasPtr->setTexture(NULL);       // SOLID fill
        canvasPtr->setFillColor(gFgColor); // Default is RED
      }
      else
      {
        if(!gSkipBlending)
        {
          canvasPtr->setAlphaTexture(true); // TEXTURE fill
        }
        canvasPtr->setTexture(&mandelTex);
      }
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      // canvasPtr->setOverdraw(false);   // DONT TOUCH - experimental
      // canvasPtr->initWithBuffer(&surf);
      canvasPtr->setAlpha(1.0);
      canvasPtr->setYOversample(gOversampleY);
      canvasPtr->setBiLerp(false);

      canvasPtr->rectangle(0,0,size_w,size_w);

    }// ONCE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    double now1 = pxMilliseconds();

    int dx = 0;
    int dy = 0;

    int index  = 0;
    bool blank_square = false;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // DRAW ALL SQUARES !
    for(int i=0; i< gUseSquares; i++)
    {
      dx = (size_w * index);

      if(dx > screen_w) // next line
      {
          dx = 0;
          dy += (size_h);

          index = 0;
      }

      index++;
      blank_square = (gUseSquares >1 ) ? !blank_square : false;

      if(blank_square)
      {
          continue; // Skip blanks squares... checkerboard !
      }

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      m.identity();

      if(gUseRotation)
      {
        const  float one_degree = 0.0174533; // in radians
        static float angle      = 0.261799; // 15 degrees in radians

        if(gUseAnimation)
        {
          angle += one_degree;
        }

        m.translate(-size_w/2, -size_h/2);
        m.rotate(angle);
      }

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      if(gUseSquares == 1) // Single Square... center on screen
      {
        if(!gUseRotation) m.translate(-size_w/2, -size_h/2);
        m.translate(screen_w/2, screen_h/2);
      }
      else
      {
        m.translate(dx, dy);
      }

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // RENDER
      canvasPtr->setMatrix(m);
      canvasPtr->fill(gPerfTiming);
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    }//FOR
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    // RASTER timing...
    double raster_ms = pxMilliseconds() - now;

    avgRaster_sigma += raster_ms;

    if(++avgRaster_count == 60.0)
    {
        updateCPU();

        avgRaster_count = 0.0;
        avgRaster_sigma = 0.0;

        gCount60++;
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    extern bool needsFlip;
    needsFlip = true;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  struct timeval start, end;

  inline void updateFPS()
  {
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Update FPS...

    if(gFpsCount++ == 0)
    {
      gettimeofday(&start, NULL);
    }
    else if(gFpsCount == 60)
    {
      gettimeofday(&end, NULL);

      long seconds  = end.tv_sec  - start.tv_sec;
      long useconds = end.tv_usec - start.tv_usec;

      long total_ms      = ((seconds) * 1000 + useconds/1000.0) + 0.5;
      float ms_per_frame = (float) total_ms / (float) gFpsCount;

      if(gFormatCSV)
      {
        printf("\nINFO  FPS:, %3.6f, frames ", 1.0f / (ms_per_frame / 1000.0f));
      }
      else
      {
        printf("\nElapsed  FPS: %3.6f frames ", 1.0f / (ms_per_frame / 1000.0f));
      }

      gFpsCount = 0;
      start = end;
    }
  }
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  inline void updateCPU()
  {
    // ### CPU Load
    int who = RUSAGE_SELF;//RUSAGE_CHILDREN;//RUSAGE_SELF;
    struct rusage ru;

    int ret = getrusage(who, &ru);

    struct timeval utime_now = ru.ru_utime;
    struct timeval stime_now = ru.ru_stime;

    struct timeval wall_now;
    gettimeofday(&wall_now, NULL);

    double start0    = wall_start.tv_sec + wall_start.tv_usec / 1000000;
    double stop0     = wall_now.tv_sec   + wall_now.tv_usec   / 1000000;
    double wall_time = stop0 - start0;

    double start1   = utime_start.tv_sec + utime_start.tv_usec / 1000000;
    double stop1    = utime_now.tv_sec   + utime_now.tv_usec   / 1000000;
    double cpu_time = stop1 - start1;

    double percentage = (cpu_time / wall_time) * 100.0;

    if(gFormatCSV)
    {
      printf(" draw()  %d pt Avg:, %4.6f, ms,   load:, %3.1f%%",
            (int)avgRaster_count, (avgRaster_sigma / avgRaster_count), percentage);
    }
    else
    {
      printf(" ############# draw() >>> %d pt Avg: %4.6f ms   load:  %3.1f%%",
            (int)avgRaster_count, (avgRaster_sigma / avgRaster_count), percentage);
    }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onCreate()
  {
    drawFrame();
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onAnimationTimer()
  {
    gTextureRotate += gRotateDelta;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onSize(int32_t w, int32_t h)
  {
    offscreen.initWithColor(w, h, pxGreen);
    drawFrame();
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onCloseRequest()
  {
    eventLoop.exit();
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onDraw(pxSurfaceNative s)
  {
    if(!gSkipClearscreen)
    {
      clearScreen(gBgColor);
    }

    beginNativeDrawing(s); // <<< LOCKS  ... and Fills in WxH, Base, Stride etc.

    double now_ms = pxMilliseconds();

    drawFrame(s);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if(gShowWork)
    {
      double work_ms = pxMilliseconds() - now_ms;
      printf("\n [%d] ############# onDraw() >>> work: %f ms", gFrameNumber, work_ms );
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    updateFPS();

    endNativeDrawing(s); // <<< UNLOCKS

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if(  ( (gExitIterations > 0) && (gCount60 >= gExitIterations)              ) ||
         ( (gExitTimeout_ms > 0) && (gExitTimeout_ms < (now_ms - gAppStart_ms))) )
    {
      fflush(stdout);

      onCloseRequest();
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:
  double gTextureRotate;
  double gRotateDelta;

  pxOffscreen offscreen;
  pxOffscreen mandelTex;

  pxCanvas* canvasPtr;
};

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void usage()
{
      printf("\n");
      printf("\nUSAGE:  RasterFill <args> ");
      printf("\n");
      printf("\n        -w       Show \"work:\"  per frame timings");
      printf("\n        -r       Use rotation Static of 15 Degrees");
      printf("\n        -a       Use rotation Animation");
      printf("\n        -b       Skip Alpha Blending");
      printf("\n        -c       Skip clearscreen");
      printf("\n        -p       Enable Perf Timers");
      printf("\n        -csv     Output format in CSV");
      printf("\n        -s NN    Use NN for square dimension");
      printf("\n        -i <II>  Exit after '<II>' 60pt data points ");
      printf("\n        -t <MM>  Exit after '<MM> ms' (Milliseconds) ");
      printf("\n        -n NN    Use NN squares in checherboard");
      printf("\n        -y NN    Use NN for OversampleY setting");
      printf("\n");
      printf("\n        -fg CLR  Use Foreground color   (WHITE,BLACK,BLUE,RED,GREEN,GRAY)");
      printf("\n        -bg CLR  Use Background color   (WHITE,BLACK,BLUE,RED,GREEN,GRAY)");
      printf("\n\n");
      exit(0);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void processArgs(int argc, char **argv)
{
  gShowWork     = false;
  gDirectRender = true;
  gUseMandel    = false;
  gUseRotation  = false;
  gUseAnimation = false;

  if(argc > 1)
  {
    gUseMandel = (strcmp(argv[1], "mandel") == 0); // match ?
  }

  // if(argc > 2)
  // {
  //   gDirectRender = (strcmp(argv[2], "direct") == 0); // match ?
  // }

  for(int i=0; i< argc; i++)
  {
    gShowWork        = (gShowWork        == true) || (strcmp(argv[i], "-w") == 0); // match ?
    gUseRotation     = (gUseRotation     == true) || (strcmp(argv[i], "-r") == 0); // match ?
    gUseAnimation    = (gUseAnimation    == true) || (strcmp(argv[i], "-a") == 0); // match ?
    gSkipBlending    = (gSkipBlending    == true) || (strcmp(argv[i], "-b") == 0); // match ?
    gSkipClearscreen = (gSkipClearscreen == true) || (strcmp(argv[i], "-c") == 0); // match ?
    gPerfTiming      = (gPerfTiming      == true) || (strcmp(argv[i], "-p") == 0); // match ?
    gUseMandel       = (gUseMandel       == true) || (strcmp(argv[i], "mandel") == 0); // match ?
    gDirectRender    = (gDirectRender    == true) || (strcmp(argv[i], "direct") == 0); // match ?

    if(strcmp(argv[i], "-n") ==0)
    {
        gUseSquares = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-y") ==0)
    {
        gOversampleY = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-s") ==0)
    {
        gSquareSize = atoi(argv[++i]);

        size_w = size_h = gSquareSize;
    }

    if(strcmp(argv[i], "-i") ==0)
    {
        gExitIterations = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-t") ==0)
    {
        gExitTimeout_ms = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-csv") ==0)
    {
      gFormatCSV = true;
    }

    if(strcmp(argv[i], "-fg") ==0)
    {
        i++;

              if(strcmp(argv[i], "WHITE") ==0)
        {
          gFgColor = pxWhite;
        }
        else if(strcmp(argv[i], "BLUE") ==0)
        {
          gFgColor = pxBlue;
        }
        else if(strcmp(argv[i], "RED") ==0)
        {
          gFgColor = pxRed;
        }
        else if(strcmp(argv[i], "GREEN") ==0)
        {
          gFgColor = pxGreen;
        }
        else if(strcmp(argv[i], "GRAY") ==0)
        {
          gFgColor = pxGray;
        }
    }

    if(strcmp(argv[i], "-bg") ==0)
    {
        i++;

            if(strcmp(argv[i], "WHITE") ==0)
        {
          gBgColor = pxWhite;
        }
        else if(strcmp(argv[i], "BLUE") ==0)
        {
          gBgColor = pxBlue;
        }
        else if(strcmp(argv[i], "RED") ==0)
        {
          gBgColor = pxRed;
        }
        else if(strcmp(argv[i], "GREEN") ==0)
        {
          gBgColor = pxGreen;
        }
        else if(strcmp(argv[i], "GRAY") ==0)
        {
          gBgColor = pxGray;
        }
    }

    if(strcmp(argv[i], "-?") ==0)
    {
      usage(); // exits...
    }
  }
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int pxMain(int argc, char **argv)
{
  //### CPU Load
  gettimeofday(&wall_start, NULL);

  int who = RUSAGE_SELF;//RUSAGE_CHILDREN;//RUSAGE_SELF;
  struct rusage ru;
  int ret = getrusage(who, &ru);

  utime_start = ru.ru_utime;
  stime_start = ru.ru_stime;
  //###

  gAppStart_ms = pxMilliseconds();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // PROCESS ARGS

  processArgs(argc, argv);

  if(gUseSquares == -1)
  {
    gUseSquares =   ( ( ((float) screen_w / (float) size_w) ) + 1.0)
                  * ( ( ((float) screen_h / (float) size_h) ) + 0.5);
  }

  printf("\n\n #########  PX %s %s RASTER !! \n",
    (gUseMandel    ? "MANDEL" : "FILL"),
    (gDirectRender ? "DIRECT" : "MEMCPY"));

  char string1[14] = {'\0'};
  char string2[14] = {'\0'};

  if(gExitIterations >0 || gExitTimeout_ms > 0)
  {
    sprintf(string1, "%s",   (gExitIterations > 0) ? " - ITERATIONS: "  : (gExitTimeout_ms > 0) ? " - TIMEOUT: " : "");
    sprintf(string2, "%.1f", (gExitIterations > 0) ? gExitIterations : (gExitTimeout_ms > 0) ? gExitTimeout_ms : 0.0);
  }

  printf("\n NOTE: Screen WxH: %d x %d ", screen_w, screen_h);
  printf("\n NOTE:  Using %d square%s  WxH: %d x %d   - %s ROTATION (%s) %s %s %s%s  %s\n",
      gUseSquares, (gUseSquares > 1) ?  "s" : "", // plural ?
      gSquareSize, gSquareSize,      // dimensions
      gUseRotation                   ? "WITH"          : "WITHOUT",
      (gUseAnimation & gUseRotation) ? "Animated"      : "Static",
      (gSkipClearscreen)             ? " - SKIP Clear"    : "",
      (gSkipBlending)                ? " - SKIP Blending" : "",
      string1, string2,
      gPerfTiming ? "TIMERS" : "");

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  myWindow win;

  win.init(0, 0, screen_w, screen_h);
  win.setTitle("Rasterizer");
  win.setVisibility(true);
  win.setAnimationFPS(30);

  eventLoop.run();

  return 0;
}


