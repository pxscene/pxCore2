#include <stdio.h>
#include <ctime>
#include <sys/time.h>

#include "pxTimer.h"
#include "pxCore.h"
#include "pxEventLoop.h"
#include "pxWindow.h"
#include "pxOffscreen.h"
#include "pxCanvas.h"

pxEventLoop eventLoop;


//############ CPU Load #########

static struct timeval  wall_last;
static struct timeval utime_last;
static struct timeval stime_last;

static double avgRaster_count = 0.0;
static double avgRaster_sigma = 0.0;

//##############################

extern int  gFrameNumber;
extern bool gDirectRender;

extern IDirectFB         *dfb;
extern IDirectFBSurface  *dfbSurface; // aka 'primary'


int  gIterations   = 0;
int  gFpsRate      = 30; // default:  30 frames per second
int  gFpsCount     = 0;
int  gFpsWindow    = 60; // default:  60 frames wide
int  gOversampleY  = 1;
int  gUseSquares   = -1; // default:  (unset)

float  gSquareSize   = 32.0f;

float  size_w = gSquareSize, size_h = gSquareSize; //(default)

int  screen_w = 1280,        screen_h = 720;

bool gDragRace         = false;

bool gFormatCSV        = false;
bool gUseMandel        = false;
bool gUseFilledTex     = false;
bool gUseRotation      = false;
bool gUseAnimation     = false;
bool gShowWork         = false;
bool gSkipBlending     = false;
bool gSkipAllDrawing   = false;
bool gSkipClearscreen  = false;
bool gSkipMatrixMath   = false;
bool gVerbose          = false;

bool gPerfTiming       = false;

int  gLoopInProcessMax = 1; // default

double gAppStart_ms    = 0.0;
double gExitTimeout_ms = -1; // default - unset
double gExitIterations = 0;

double fData_FPS_value    = 0.0;
double fData_LastDraw_ms  = 0.0;
double fData_CPU_load     = 0.0;

double fData_AvgRaster_ms = 0.0;
double fData_MaxRaster_ms = 0.0;
double fData_MinRaster_ms = 9999.0;

bool   gLoop = true;

pxColor gFgColor = pxBlue;
pxColor gBgColor = pxGray;


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void draw_mandel(pxBuffer& b,
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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

    if(gUseMandel || gUseFilledTex)
    {
      // Mandel Texture
      mandelTex.init(size_w, size_w);

      if(gUseMandel)
      {
         draw_mandel(mandelTex, -2, 1, -1.5, 1.5, 16);
      }
      else
      {
         mandelTex.fill(pxBlue);
      }
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
      canvasPtr->setAlpha(1.0);
      canvasPtr->setYOversample(gOversampleY);
      canvasPtr->setBiLerp(false);
//      canvasPtr->setBiLerp(!gSkipBlending);

      canvasPtr->rectangle(0,0,size_w,size_w);

    }// ONCE
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

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

      if(!gSkipMatrixMath)
      {
        if(gUseRotation)
        {
          const  float one_degree = 0.0174533; // in radians
          static float angle      = 0.2617990; // 15 degrees in radians

          if(gUseAnimation)
          {
            angle += one_degree;
          }

          m.translate(-size_w/2, -size_h/2);
          m.rotate(angle);
        } // FOR

        if(gUseSquares == 1) // Single Square... center on screen
        {
          if(!gUseRotation) m.translate(-size_w/2, -size_h/2);
          m.translate(screen_w/2, screen_h/2);
        }
        else
        {
          m.translate(dx, dy);
        }
      }// SKIP MATRIX MATH
      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
      // RENDER

      canvasPtr->setMatrix(m);
      canvasPtr->fill(gPerfTiming); // <<<< RASTERIZE IT !!

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    }//FOR

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    extern bool needsFlip;
    needsFlip = true;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  struct timeval start, end;

  inline void updateFPS()
  {
    gettimeofday(&end, NULL);

    long seconds  = end.tv_sec  - start.tv_sec;
    long useconds = end.tv_usec - start.tv_usec;

    long total_ms      = ((seconds) * 1000.0 + useconds/1000.0) + 0.5;
    float ms_per_frame = (float) total_ms / (float) gFpsCount;

    fData_FPS_value = 1.0f / (ms_per_frame / 1000.0f);

    // Reset
    gettimeofday(&start, NULL);
  }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  inline void updateCPU()
  {
    // ### CPU Load
    int who = RUSAGE_SELF;
    struct rusage ru;

    int ret = getrusage(who, &ru);

    struct timeval utime_now = ru.ru_utime;
    struct timeval stime_now = ru.ru_stime;

    struct timeval wall_now;
    gettimeofday(&wall_now, NULL);

    // WALL Time ...
    double wall_start = wall_last.tv_sec + (wall_last.tv_usec / 1000000.0);
    double wall_stop  =  wall_now.tv_sec + ( wall_now.tv_usec / 1000000.0);
    double wall_time  = wall_stop - wall_start;

    // KERNEL Time ... (this process)
    double sys_start  =  stime_last.tv_sec + (stime_last.tv_usec / 1000000.0);
    double sys_stop   =   stime_now.tv_sec + ( stime_now.tv_usec / 1000000.0);
    double sys_time   = sys_stop - sys_start;

    // USER Time ...  (this process)
    double user_start = utime_last.tv_sec + (utime_last.tv_usec / 1000000.0);
    double user_stop  =  utime_now.tv_sec + ( utime_now.tv_usec / 1000000.0);
    double user_time  = user_stop - user_start;

    fData_CPU_load     = ( (user_time + sys_time) / wall_time ) * 100.0;
    fData_AvgRaster_ms = avgRaster_count > 0 ? (avgRaster_sigma / avgRaster_count) : 0.0;

    if( fData_AvgRaster_ms > fData_MaxRaster_ms)
    {
        fData_MaxRaster_ms = fData_AvgRaster_ms;
    }

    if( fData_AvgRaster_ms < fData_MinRaster_ms)
    {
        fData_MinRaster_ms = fData_AvgRaster_ms;
    }

    // Reset
    wall_last  = wall_now;
    utime_last = ru.ru_utime;
    stime_last = ru.ru_stime;

    avgRaster_sigma = 0;
    avgRaster_count = 0;
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onCloseRequest()
  {
    if(gDragRace)
    {
      gLoop = false;
    }
    else
    {
      eventLoop.exit();
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

  void onDraw2(pxSurfaceNative s)
  {
      if(!gSkipClearscreen)
      {
          dfbSurface->Clear(dfbSurface, gBgColor.r, gBgColor.g, gBgColor.b, gBgColor.a); //CLEAR
      }

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      if(!gSkipAllDrawing)
      {
        beginNativeDrawing(s); // <<< LOCKS - Primary Surface ... and Fills in WxH, Base, Stride etc.

        drawFrame(s);

        endNativeDrawing(s); // <<< UNLOCKS - Primary Surface
      }

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      gIterations++;

      // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

      if( gIterations >= gExitIterations )
      {
        onCloseRequest();
      }
  }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  void onDraw(pxSurfaceNative s)
  {
    if(!gSkipClearscreen)
    {
      clearScreen(gBgColor);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    beginNativeDrawing(s); // <<< LOCKS  ... and Fills in WxH, Base, Stride etc.

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    double now_ms = pxMilliseconds();

    if(!gSkipAllDrawing)
    {
      drawFrame(s); // DO WORK !

      // RASTER timing...
      double raster_ms = pxMilliseconds() - now_ms;

      avgRaster_sigma += raster_ms;
      avgRaster_count++;

      if(gShowWork)
      {
          printf("\n [%d] ############# onDraw() >>> work: %f ms", gFrameNumber, raster_ms );
      }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if(gFpsCount == 0)
    {
      gettimeofday(&start, NULL);
    }
    else
    if(gFpsCount == gFpsWindow) // MAX
    {
        updateFPS();

        updateCPU();

        if(gFormatCSV)
        {
            printf("\nINFO  FPS:, %3.6f, frames, draw()  Avg:, %4.6f, ms, Max:, %4.6f, ms, Min:, %4.6f, ms,   load:, %3.1f%%",
                fData_FPS_value, fData_AvgRaster_ms, fData_MaxRaster_ms, fData_MinRaster_ms, fData_CPU_load);
        }
        else
        {
            printf("\nElapsed  FPS: %3.6f frames  ... draw()  Avg: %4.6f ms  Max: %4.6f ms  Min: %4.6f ms   Load: %3.1f%%",
                fData_FPS_value, fData_AvgRaster_ms, fData_MaxRaster_ms, fData_MinRaster_ms, fData_CPU_load);
        }

        gFpsCount = 0;
        gIterations++; // every "gFpsWindow" frames .. increment iterations
    }
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    gFpsCount++;
    gFrameNumber++;

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    endNativeDrawing(s); // <<< UNLOCKS

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    if(  ( (gExitIterations > 0) && (gIterations >= gExitIterations)              ) ||
         ( (gExitTimeout_ms > 0) && (gExitTimeout_ms < (now_ms - gAppStart_ms))) )
    {
      printf("\n\n");
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
      printf("\n");
      printf("\nUSAGE:  rasterPXC <args> ");
      printf("\n");
      printf("\n        -w       Show \"work:\"  per frame timings");
      printf("\n        -r       Use rotation Static of 15 Degrees");
      printf("\n        -a       Use rotation Animation");
      printf("\n        -b       Skip Alpha Blending");
      printf("\n        -c       Skip clearscreen");
      printf("\n        -d       Skip *ALL* Drawing");
      printf("\n        -m       Skip *ALL* Matrix Math");
      printf("\n        -x       Drag race... Don't SLEEP !");
      printf("\n        -xl NN   Drag race... NN times in one process");
      printf("\n        -p       Enable Perf Timers");
      printf("\n        -csv     Output format in CSV");
      printf("\n        -s NN    Use NN for square dimension");
      printf("\n        -i NN    Exit after 'NN' Avg. Windows data points ");
      printf("\n        -t NN    Exit after 'NN ms' (Milliseconds) ");
      printf("\n        -n NN    Use NN squares in checherboard");
      printf("\n        -y NN    Use NN for OversampleY setting");
      printf("\n        -f NN    Use NN for FPS rate");
      printf("\n        -w NN    Use NN for FPS Avg. Window size");
      printf("\n");
      printf("\n        -fg CLR  Use Foreground color   (WHITE,BLACK,BLUE,RED,GREEN,GRAY)");
      printf("\n        -bg CLR  Use Background color   (WHITE,BLACK,BLUE,RED,GREEN,GRAY)");
      printf("\n");
      printf("\n        Use \"mandel\"  for Mandelbrot Set texture fill.");
      printf("\n        Use \"solid\"   for Solid Color    texture fill.");
      printf("\n");
      printf("\n");
      exit(0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void processArgs(int argc, char **argv)
{
  gDirectRender = true;

  for(int i=0; i< argc; i++)
  {
    gShowWork        = (gShowWork        == true) || (strcmp(argv[i], "-w"     ) == 0); // match ?
    gUseRotation     = (gUseRotation     == true) || (strcmp(argv[i], "-r"     ) == 0); // match ?
    gUseAnimation    = (gUseAnimation    == true) || (strcmp(argv[i], "-a"     ) == 0); // match ?
    gSkipBlending    = (gSkipBlending    == true) || (strcmp(argv[i], "-b"     ) == 0); // match ?
    gSkipClearscreen = (gSkipClearscreen == true) || (strcmp(argv[i], "-c"     ) == 0); // match ?
    gSkipAllDrawing  = (gSkipAllDrawing  == true) || (strcmp(argv[i], "-d"     ) == 0); // match ?
    gSkipMatrixMath  = (gSkipMatrixMath  == true) || (strcmp(argv[i], "-m"     ) == 0); // match ?
    gPerfTiming      = (gPerfTiming      == true) || (strcmp(argv[i], "-p"     ) == 0); // match ?
    gUseMandel       = (gUseMandel       == true) || (strcmp(argv[i], "mandel" ) == 0); // match ?
    gUseFilledTex    = (gUseFilledTex    == true) || (strcmp(argv[i], "solid"  ) == 0); // match ?
    gDirectRender    = (gDirectRender    == true) || (strcmp(argv[i], "direct" ) == 0); // match ?
    gDragRace        = (gDragRace        == true) || (strcmp(argv[i], "-x"     ) == 0); // match ?
    gVerbose         = (gVerbose         == true) || (strcmp(argv[i], "-v"     ) == 0); // match ?

    if(strcmp(argv[i], "-f") ==0)
    {
        gFpsRate = atof(argv[++i]);
    }

    if(strcmp(argv[i], "-i") ==0)
    {
        gExitIterations = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-n") ==0)
    {
        gUseSquares = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-xl") ==0)
    {
        gDragRace = true;
        gLoopInProcessMax = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-s") ==0)
    {
        gSquareSize = atoi(argv[++i]);

        size_w = size_h = gSquareSize;
    }

    if(strcmp(argv[i], "-t") ==0)
    {
        gExitTimeout_ms = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-w") ==0)
    {
        gFpsWindow = atoi(argv[++i]);
    }

    if(strcmp(argv[i], "-y") ==0)
    {
        gOversampleY = atoi(argv[++i]);
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
  gettimeofday(&wall_last, NULL);

  int who = RUSAGE_SELF;//RUSAGE_CHILDREN;//RUSAGE_SELF;
  struct rusage ru;
  int ret = getrusage(who, &ru);

  utime_last = ru.ru_utime;
  stime_last = ru.ru_stime;
  //###

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  gAppStart_ms = pxMilliseconds();

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // PROCESS ARGS

  processArgs(argc, argv);

  if(gUseSquares == -1)
  {
    gUseSquares =   ( ( ((float) screen_w / (float) size_w) ) + 1.0)
                  * ( ( ((float) screen_h / (float) size_h) ) + 0.5);
  }

  if(gVerbose)
  {
    printf("\n\n #########  PX %s %s RASTER !! \n",
      (gUseMandel ? "MANDEL" : gUseFilledTex ? "SOLID TEXTURE" : "FILL"),
      (gDirectRender ? "DIRECT" : "MEMCPY"));

    char string1[14] = {'\0'};
    char string2[14] = {'\0'};

    if(gExitIterations >0 || gExitTimeout_ms > 0)
    {
      sprintf(string1, "%s",   (gExitIterations > 0) ? " - ITERATIONS: "  : (gExitTimeout_ms > 0) ? " - TIMEOUT: " : "");
      sprintf(string2, "%.1f", (gExitIterations > 0) ? gExitIterations : (gExitTimeout_ms > 0) ? gExitTimeout_ms : 0.0);
    }

    printf("\n NOTE: Screen WxH: %d x %d   @  %d FPS   (Avg. Window: %d) ", screen_w, screen_h, gFpsRate, gFpsWindow);
    printf("\n NOTE:  Using %d square%s  WxH: %d x %d   - %s ROTATION (%s) %s %s %s %s %s%s  %s\n",
        gUseSquares, (gUseSquares > 1) ?  "s" : "", // plural ?
        gSquareSize, gSquareSize,      // dimensions
        gUseRotation                   ? "WITH"             : "WITHOUT",
        (gUseAnimation & gUseRotation) ? "Animated"         : "Static",
        (gSkipClearscreen)             ? " - SKIP Clear"    : "",
        (gSkipBlending)                ? " - SKIP Blending" : "",
        (gSkipAllDrawing)              ? " - SKIP Drawing"  : "",
        (gSkipMatrixMath)              ? " - SKIP Math"     : "",
        string1, string2,
        gPerfTiming ? "TIMERS" : ""); // Perf Timers in pxRasterizer...
  }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if 0 // Benchmark MATRIX math
    pxMatrix mmm;

    double ss = pxMilliseconds();

    unsigned long int count = 100000;
    for(unsigned long int i = 0; i < count; i++)
    {
         mmm.identity();
         mmm.translate(33, 44);
         mmm.rotate(0.261799); // 15 degrees in radians
    }

    double ee = pxMilliseconds() - ss;
    printf("\nMATRIX - translate = %3.12f" , ee / (double) count);
#endif

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  myWindow win;

  win.init(0, 0, screen_w, screen_h);
  win.setTitle("Rasterizer");
  win.setVisibility(true);
  win.setAnimationFPS(gFpsRate);

  if(gDragRace)
  {
      pxSurfaceNativeDesc d;

      d.dfb          = dfb;
      d.surface      = dfbSurface; // primary surface
      d.windowWidth  = screen_w;
      d.windowHeight = screen_h;

      if(gVerbose)
      {
        printf("\n\n DRAG RACE !!! ...");
      }

      for(int i=0; i< gLoopInProcessMax; i++)
      {
          double start_ms = pxMilliseconds();

          while(gLoop)
          {
              win.onDraw2(&d);

              dfbSurface->Flip(dfbSurface, NULL, DSFLIP_NONE ); // DSFLIP_NONE   DSFLIP_WAITFORSYNC

          }//WHILE

          // Reset
          gLoop = true;
          gIterations = 0;

          pxSleepMS(100);

          double elapsed_ms    = pxMilliseconds() - start_ms;
          double frametime_sec = (elapsed_ms / 1000.0f) / gExitIterations;

          printf("\nPXC DRAG RACE >> %d frames in %3.2f ms .... %3.2f FPS ",
                (int) gExitIterations, elapsed_ms, (1.0/frametime_sec) );
      }//FOR
  }
  else
  {
      eventLoop.run();
  }

  pxSleepMS(1000); // allow DFB to cleanup  ?

  return 0;
}
