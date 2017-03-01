#include <stdio.h>
#include <ctime>
#include <sys/time.h>

#include <directfb.h>

#include "pxTimer.h"
#include "pxPixel.h"
#include "pxColor.h"
#include "pxBuffer.h"

#define PX_PLATFORM_GENERIC_DFB
#include "pxMatrix4T.h"

//############ CPU Load #########

static struct timeval  wall_last;
static struct timeval utime_last;
static struct timeval stime_last;

static double avgRaster_count = 0.0;
static double avgRaster_sigma = 0.0;

//##############################

static IDirectFB        *dfb     = NULL;
static IDirectFBSurface *primary = NULL;
static IDirectFBSurface *surface = NULL;
static IDirectFBSurface *mandel  = NULL;

int  gFrameNumber;
bool gDirectRender; // Not used

int  gIterations   = 0;
int  gFpsRate      = 30; // default:  30 frames per second
int  gFpsCount     = 0;
int  gFpsWindow    = 60; // default:  60 frames wide
int  gOversampleY  = 1;
int  gUseSquares   = -1; // default:  (unset)
int  gSquareSize   = 32;

int    size_w = gSquareSize,   size_h = gSquareSize; //(default)
int  screen_w = 1280,        screen_h = 720;

bool gDragRace        = false;

bool gUseVideoMem     = false; /// DFB only
bool gFormatCSV       = false;
bool gUseMandel       = false;
bool gUseFilledTex    = false;
bool gUseRotation     = false;
bool gUseAnimation    = false;
bool gShowWork        = false;
bool gSkipBlending    = false;
bool gSkipAllDrawing  = false;
bool gSkipClearscreen = false;
bool gSkipMatrixMath  = false;
bool gVerbose         = false;

bool needsFlip = true;

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

double gSleep_ms;
bool   gLoop = true;

pxColor gFgColor = pxRed;
pxColor gBgColor = pxGray;

void onTimer(uint32_t v); //fwd
void onDraw();

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

#define DFB_CHECK(x...) x;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void applyMatrix(IDirectFBSurface  *surface, const float *mm)
{
  if(surface == NULL || mm == NULL)
  {
    fprintf(stderr,"\nERROR: cannot %s()  ... NULL params", __FUNCTION__);
    return;
  }

#define F2F_SHIFT (0x10000)   //  Floating-Point to Fixed-Point shift.

  s32 matrix[9];

  // Convert to fixed point for DFB...
  //
  matrix[0] = (s32)(mm[0]  * F2F_SHIFT);
  matrix[1] = (s32)(mm[4]  * F2F_SHIFT);
  matrix[2] = (s32)(mm[12] * F2F_SHIFT);
  matrix[3] = (s32)(mm[1]  * F2F_SHIFT);
  matrix[4] = (s32)(mm[5]  * F2F_SHIFT);
  matrix[5] = (s32)(mm[13] * F2F_SHIFT);

  matrix[6] = 0x00000;
  matrix[7] = 0x00000;
  matrix[8] = F2F_SHIFT;

  surface->SetMatrix(surface, matrix);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

inline void drawFrame()
{
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // UPDATE / DRAW
    static bool once = true;
    static pxMatrix4f m;

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
            continue;// Skip blanks squares... checkerboard !
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        m.identity();

        if(!gSkipMatrixMath)
        {
            if(gUseSquares == 1) // Single Square... center on screen
            {
                if(!gUseRotation) m.translate(-size_w/2, -size_h/2);
                m.translate(screen_w/2, screen_h/2);
            }
            else
            {
                m.translate(dx, dy);
            }

            if(gUseRotation)
            {
                const  float one_degree = 0.0174533; // in radians
                static float angle      = 0.261799; // 15 degrees in radians

                if(gUseAnimation)
                {
                    angle += one_degree;
                }

                m.rotateInRadians(angle);
                m.translate(-size_w/2, -size_h/2);
            }
        }// SKIP MATRIX MATH
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // RENDER

        applyMatrix(primary, m.data());

        if(gUseMandel)
        {
            primary->Blit(primary, mandel, NULL, 0,0);
        }
        else
        {
            primary->FillRectangle( primary, 0, 0, size_w, size_h);
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    }//FOR

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    //extern bool needsFlip;
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

    double frame_ms = (1.0/(double) gFpsRate) * 1000.0;

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void onCloseRequest()
    {
        gLoop = false;
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void onTimer(uint32_t v)
    {
         onDraw();
    }

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    void onDraw2()
    {
        if(!gSkipClearscreen)
        {
            primary->Clear(primary, gBgColor.r, gBgColor.g, gBgColor.b, gBgColor.a); //CLEAR
            needsFlip = true;
        }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        if(!gSkipAllDrawing)
        {
            drawFrame();
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

    void onDraw()
    {
        if(!gSkipClearscreen)
        {
            primary->Clear(primary, gBgColor.r, gBgColor.g, gBgColor.b, gBgColor.a); //CLEAR
            needsFlip = true;
        }

         // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        double now_ms = pxMilliseconds();

        if(!gSkipAllDrawing)
        {
            drawFrame(); // DO WORK !

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

        if( ( (gExitIterations > 0) && (gIterations >= gExitIterations)              ) ||
            ( (gExitTimeout_ms > 0) && (gExitTimeout_ms < (now_ms - gAppStart_ms))) )
        {
          printf("\n\n");
          fflush(stdout);

          onCloseRequest();
        }
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    }

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void usage()
{
      printf("\n");
      printf("\nUSAGE:  rasterDFB <args> ");
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
      printf("\n        -vm      Use VIDEO memory for images");
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
  for(int i=0; i< argc; i++)
  {
    gShowWork        = (gShowWork        == true) || (strcmp(argv[i], "-w"     ) == 0); // match ?
    gUseRotation     = (gUseRotation     == true) || (strcmp(argv[i], "-r"     ) == 0); // match ?
    gUseAnimation    = (gUseAnimation    == true) || (strcmp(argv[i], "-a"     ) == 0); // match ?
    gSkipBlending    = (gSkipBlending    == true) || (strcmp(argv[i], "-b"     ) == 0); // match ?
    gSkipClearscreen = (gSkipClearscreen == true) || (strcmp(argv[i], "-c"     ) == 0); // match ?
    gSkipAllDrawing  = (gSkipAllDrawing  == true) || (strcmp(argv[i], "-d"     ) == 0); // match ?
    gSkipMatrixMath  = (gSkipMatrixMath  == true) || (strcmp(argv[i], "-m"     ) == 0); // match ?
    gUseVideoMem     = (gUseVideoMem     == true) || (strcmp(argv[i], "-vm"    ) == 0); // match ?
    gUseMandel       = (gUseMandel       == true) || (strcmp(argv[i], "mandel" ) == 0); // match ?
    gUseFilledTex    = (gUseFilledTex    == true) || (strcmp(argv[i], "solid"  ) == 0); // match ?
    gDirectRender    = (gDirectRender    == true) || (strcmp(argv[i], "direct" ) == 0); // match ?
    gDragRace        = (gDragRace        == true) || (strcmp(argv[i], "-x"     ) == 0); // match ?
    gVerbose         = (gVerbose         == true) || (strcmp(argv[i], "-v"     ) == 0); // match ?

    if(strcmp(argv[i], "-f") ==0)
    {
        gFpsRate = atoi(argv[++i]);

        frame_ms = (1.0/(double) gFpsRate) * 1000.0;
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

int main(int argc, char *argv[])
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
        printf("\n\n#########  DFB %s RASTER !! \n",
        (gUseMandel ? "MANDEL" : gUseFilledTex ? "SOLID TEXTURE" : "FILL"));

        char string1[14] = {'\0'};
        char string2[14] = {'\0'};

        if(gExitIterations >0 || gExitTimeout_ms > 0)
        {
            sprintf(string1, "%s",   (gExitIterations > 0) ? " - ITERATIONS: "  : (gExitTimeout_ms > 0) ? " - TIMEOUT: " : "");
            sprintf(string2, "%.1f", (gExitIterations > 0) ? gExitIterations : (gExitTimeout_ms > 0) ? gExitTimeout_ms : 0.0);
        }

        printf("\n NOTE: Screen WxH: %d x %d   @  %d FPS   (Avg. Window: %d) ", screen_w, screen_h, gFpsRate, gFpsWindow);
        printf("\n NOTE:  Using %d square%s  WxH: %d x %d   - %s ROTATION (%s) %s %s %s %s %s %s%s \n",
            gUseSquares, (gUseSquares > 1) ?  "s" : "", // plural ?
            gSquareSize, gSquareSize,      // dimensions
            gUseRotation                   ? "WITH"             : "WITHOUT",
            (gUseAnimation & gUseRotation) ? "Animated"         : "Static",
            (gUseVideoMem)                 ? "Video Memory"     : "",
            (gSkipClearscreen)             ? " - SKIP Clear"    : "",
            (gSkipBlending)                ? " - SKIP Blending" : "",
            (gSkipAllDrawing)              ? " - SKIP Drawing"  : "",
            (gSkipMatrixMath)              ? " - SKIP Math"     : "",
            string1, string2);
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // SETUP DFB

    DFBSurfaceDescription screen;

    DFB_CHECK(DirectFBInit (&argc, &argv));

    DirectFBSetOption ("quiet", NULL); // Suppress banner

    DFB_CHECK(DirectFBCreate (&dfb));
//    DFB_CHECK(dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));
    DFB_CHECK(dfb->SetCooperativeLevel (dfb, DFSCL_NORMAL));

    screen.flags = (DFBSurfaceDescriptionFlags)(DSDESC_CAPS);// | DSDESC_WIDTH | DSDESC_HEIGHT);
    screen.caps  = (DFBSurfaceCapabilities)(DSCAPS_PRIMARY | DSCAPS_DOUBLE | DSCAPS_FLIPPING);

    DFB_CHECK(dfb->CreateSurface( dfb, &screen, &primary ));
    DFB_CHECK(primary->GetSize(primary, &screen_w, &screen_h));

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    if(gUseMandel || gUseFilledTex)
    {
        // DFBSurfaceDescription dsc;

        // dsc.width  = width();
        // dsc.height = height();
        // dsc.flags  = (DFBSurfaceDescriptionFlags) (DSDESC_HEIGHT | DSDESC_WIDTH | DSDESC_PREALLOCATED | DSDESC_PIXELFORMAT | DSDESC_CAPS);
        // dsc.caps   = DSCAPS_VIDEOONLY;

        // dsc.pixelformat           = DSPF_ARGB;
        // dsc.preallocated[0].data  = base();      // Buffer is your data
        // dsc.preallocated[0].pitch = width()*4;
        // dsc.preallocated[1].data  = NULL;
        // dsc.preallocated[1].pitch = 0;

        // DFB_CHECK (dfb->CreateSurface( dfb, &dsc, &mandel ));

        // if(mandel)
        // {
        //     mandel->Lock(mandel, (DFBSurfaceLockFlags) DSLF_WRITE, &s->windowBase, &s->windowStride);

        //     mandel->Unlock(mandel);
        // }

        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        // Create setting for 'mandel'
        //
        DFBSurfaceDescription dsc;

        dsc.width       = size_w;
        dsc.height      = size_h;
        dsc.caps        = DFBSurfaceCapabilities(gUseVideoMem ? DSCAPS_VIDEOONLY : DSCAPS_SYSTEMONLY); // DSCAPS_VIDEOONLY DSCAPS_SYSTEMONLY
        dsc.flags       = DFBSurfaceDescriptionFlags(DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT | DSDESC_CAPS);
        dsc.pixelformat = DSPF_ARGB;

        DFB_CHECK( dfb->CreateSurface( dfb, &dsc, &mandel ) );

        if(mandel) // ONCE !
        {
            void *pSurface;
            int   nStride;

            // Lock DFB surface  ... get pointer to backing buffer / pixels
            mandel->Lock(mandel, (DFBSurfaceLockFlags) DSLF_WRITE, &pSurface, &nStride);

            if(pSurface)
            {
                pxBuffer surf;

                surf.setBase(pSurface);
                surf.setWidth(size_w);
                surf.setHeight(size_h);
                surf.setStride(nStride);

                if(gUseMandel)
                {
                    // Render ... MANDELBROT SET ... directly to the buffer
                    draw_mandel(surf, -2, 1, -1.5, 1.5, 16);
                }
                else
                {
                    surf.fill(pxRed);
                }
            }

            mandel->Unlock(mandel);
        }
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    primary->SetRenderOptions(primary, DSRO_MATRIX);
    primary->SetColor( primary, gFgColor.r, gFgColor.g, gFgColor.b, 255.0);  //RGBA  .. RED

    if(!gSkipBlending)
    {
        primary->SetBlittingFlags(primary, DSBLIT_BLEND_ALPHACHANNEL);
    }

    size_w = gSquareSize;
    size_h = size_w;


    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
#if 0 // Benchmark MATRIX math
    pxMatrix4f mmm;

    double ss = pxMilliseconds();

    unsigned long int count = 100000;
    for(unsigned long int i = 0; i < count; i++)
    {
         mmm.identity();
         mmm.translate(33, 44);
         mmm.rotateInRadians(0.261799); // 15 degrees in radians
    }

    double ee = pxMilliseconds() - ss;
    printf("\nMATRIX - translate = %3.12f" , ee / (double) count);
#endif

    if(gDragRace)
    {
        if(gVerbose)
        {
            printf("\n\n DRAG RACE !!! ...");
        }

        for(int i=0; i< gLoopInProcessMax; i++)
        {
            double start_ms = pxMilliseconds();

            while(gLoop)
            {
                onDraw2();

                primary->Flip(primary, NULL, DSFLIP_NONE ); // DSFLIP_NONE   DSFLIP_WAITFORSYNC

            }//WHILE

            // Reset
            gLoop = true;
            gIterations = 0;

            pxSleepMS(100);

            double elapsed_ms    = pxMilliseconds() - start_ms;
            double frametime_sec = (elapsed_ms / 1000.0f) / gExitIterations;

            printf("\nDFB DRAG RACE >> %d frames in %3.2f ms .... %3.2f FPS ",
                (int) gExitIterations, elapsed_ms, (1.0/frametime_sec) );
        }//FOR
    }
    else
    {
        // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

        static double lastFrame_ms = pxMilliseconds();

        while(gLoop)
        {
            double start_ms   = pxMilliseconds();
            double elapsed_ms = start_ms - lastFrame_ms;  // Elapsed time sleeping

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            onTimer(0); // WORK   //    onTimer(0); // WORK   ... onTimer() >> display() >> animateAndRender() >>  drawFrame()

            double end_ms     = pxMilliseconds();
            double onTimer_ms = end_ms   - start_ms; // time spent in onTimer()
            double sleep_ms   = frame_ms - elapsed_ms - onTimer_ms;

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
            if(needsFlip)
            {
                needsFlip = false;
                DFB_CHECK(primary->Flip(primary, NULL, DSFLIP_NONE));// DSFLIP_NONE)); //DSFLIP_WAITFORSYNC));
            }
            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

            lastFrame_ms = end_ms;

            gFrameNumber++;

            if( (sleep_ms <      0.0) ||
                (sleep_ms > frame_ms) )
            {
                sleep_ms = frame_ms; // CLAMP
            }

            pxSleepMS( sleep_ms );

            // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
        }//WHILE
    }//ENDIF - gDragRace

    dfb->WaitIdle(dfb);

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
    // Clean Up
    if(primary) primary->Release(primary);
    if(surface) surface->Release(surface);
    if(mandel)   mandel->Release(mandel);
    if(dfb)         dfb->Release(dfb);
    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    return 0;
}
