//-----------------------------------------------------------------------------------
//  pxBenchmark.cpp
//  pxBenchmark
//
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------
#include "pxBenchmark.h"

#include "pxScene2d.h"
#include "pxContext.h"
#include "rtSettings.h"
#include "pxEventLoop.h"

//-----------------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------------

#ifndef PX_BENCHMARK_VERSION
#define PX_BENCHMARK_VERSION "dev"
#endif

benchmarkWindow win;
using namespace std;
pxEventLoop  eventLoop;
pxEventLoop* gLoop = &eventLoop;

pxContext context;


#ifdef ENABLE_DEBUG_MODE
extern int          g_argc;
extern char**       g_argv;
char** g_origArgv = NULL;
#endif

//-----------------------------------------------------------------------------------
// class pxbenchmarkWindow
//-----------------------------------------------------------------------------------

void benchmarkWindow::init(const int32_t& x, const int32_t& y, const int32_t& w, const int32_t& h, const char* url/* = NULL*/)
    {
        mApiFixture = std::shared_ptr<pxApiFixture>(&pxApiFixture::Instance());
        
        pxWindow::init(x,y,w,h);
        
        mExperimentFactory = std::shared_ptr<pxBenchmarkFactory>(&pxBenchmarkFactory::Instance());
        
        mBm = celero::RegisterBaseline(mGroupName.c_str(), mBenchmarkName.c_str(), mSamples,
                                                                    mIterations, mThreads,
                                                                    mExperimentFactory);
        
        g_argv[g_argc++] = (char*)mGroupName.c_str();
        g_argv[g_argc++] = (char*)("--archive=pxBenchmark_archive.csv");
        g_argv[g_argc++] = (char*)("--outputTable=pxBenchmark_outputTable.csv");
        /*g_argv[g_argc++] = (char*)mBenchmarkName.c_str();
        g_argv[g_argc++] = (char*)mSamples;
        g_argv[g_argc++] = (char*)mIterations;
        g_argv[g_argc++] = (char*)mThreads;*/
    }
    
void* benchmarkWindow::getInterface(const char* /*name*/)
    {
        return NULL;
    }
    
rtError benchmarkWindow::setView(pxIView* v)
    {
        return RT_OK;
    }

void benchmarkWindow::invalidateRect(pxRect* r)
    {
        pxWindow::invalidateRect(r);
    }
    
void benchmarkWindow::close()
    {
        onCloseRequest();
    }

void benchmarkWindow::onSize(/*const */int32_t/*&*/ w, /*const */int32_t/*&*/ h)
    {
        if (mWidth != w || mHeight != h)
        {
            mWidth  = w;
            mHeight = h;
        }
    }

void benchmarkWindow::onMouseDown(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y, /*const */uint32_t/*&*/ flags)
    {
    }
    
void benchmarkWindow::onCloseRequest()
    {
        if (mClosed)
        return;
        mClosed = true;
        rtLogInfo(__FUNCTION__);
        context.term();
       
        ENTERSCENELOCK()
        eventLoop.exit();
        EXITSCENELOCK()
        
    }
    
void benchmarkWindow::onMouseUp(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y, /*const */uint32_t/*&*/ flags)
    {
    }
    
void benchmarkWindow::onMouseLeave()
    {
    }
    
void benchmarkWindow::onMouseMove(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y)
    {
    }
    
void benchmarkWindow::onFocus()
    {
    }

void benchmarkWindow::onBlur()
    {
    }
    
void benchmarkWindow::onKeyDown(/*const */uint32_t/*&*/ keycode, /*const */uint32_t/*&*/ flags)
    {
    }
    
void benchmarkWindow::onKeyUp(/*const */uint32_t/*&*/ keycode, /*const */uint32_t/*&*/ flags)
    {
    }
    
void benchmarkWindow::onChar(/*const */uint32_t/*&*/ c)
    {
    }
    
void benchmarkWindow::onDraw(pxSurfaceNative/*&*/ sn)
    {
        celero::Run(g_argc, g_argv);
    }
    
void benchmarkWindow::onAnimationTimer()
    {
        
    }

void benchmarkWindow::StartTimer()
    {
   mTimer = celero::timer::GetSystemTime();
    }

void benchmarkWindow::StopTimer()
    {
    mTimer = celero::timer::GetSystemTime() - mTimer;
    rtLogInfo("Timer = %d", (int)mTimer);
    }


std::shared_ptr<pxApiFixture> benchmarkWindow::getPxApiFixture () const
    {
    return mApiFixture;
    }

//-----------------------------------------------------------------------------------
//  pxApiFixture:
//-----------------------------------------------------------------------------------
std::shared_ptr<TestFixture> pxBenchmarkFactory::Create()
    {
    return win.getPxApiFixture();
    }

pxBenchmarkFactory& pxBenchmarkFactory::Instance()
{
    static pxBenchmarkFactory singleton;
    return singleton;
}

//-----------------------------------------------------------------------------------
//  pxApiFixture:
//-----------------------------------------------------------------------------------
pxApiFixture::pxApiFixture()
{
}

pxApiFixture::~pxApiFixture()
{
}

pxApiFixture& pxApiFixture::Instance()
{
    static pxApiFixture singleton;
    return singleton;
}

void pxApiFixture::onExperimentStart(const celero::TestFixture::ExperimentValue& exp)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
   // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};

    context.drawRect(100, 100, 400, color, color);
    
    context.drawDiagLine(400, 400, 500, 500, color);
    
    context.drawDiagRect(200, 200, 400, 400, color);
    
    pxTextureRef nullMaskRef;
    pxTextureRef textureRef = context.createTexture(200, 200, 400, 400);
    
    context.drawImage(400, 400, 200, 200, textureRef, nullMaskRef, true, color, pxConstantsStretch::STRETCH, pxConstantsStretch::STRETCH, true, pxConstantsMaskOperation::NORMAL);
    
    pxTextureRef textureRef9 = context.createTexture(100, 100, 200, 200);
    context.drawImage9(100, 100, 400, 400, 500, 500, textureRef9);
}

void pxApiFixture::onExperimentEnd()
{
    mProblemSpace.clear();
}

std::vector<celero::TestFixture::ExperimentValue> pxApiFixture::getExperimentValues() const
{
    return mProblemSpace;
}

std::vector<celero::TestFixture::ExperimentValue>& pxApiFixture::popExperimentValues()
{
    return mProblemSpace;
}

void pxApiFixture::setUp(const celero::TestFixture::ExperimentValue& experimentValue)
{
}

void pxApiFixture::tearDown()
{
}

void pxApiFixture::UserBenchmark()
{
}

BASELINE(_DrawRect, baseline_rsp, pxApiFixture, 30, 10000, 100)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};
    
    context.drawRect(100, 100, 400, color, color);
}

BASELINE(_DrawDiagLine, baseline_rsp, pxApiFixture, 30, 10000, 100)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};
    
    context.drawDiagLine(400, 400, 500, 500, color);
}

BASELINE(_DrawDiagRect, baseline_rsp, pxApiFixture, 30, 10000, 100)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};
    
    context.drawDiagRect(200, 200, 400, 400, color);
}

BASELINE(_DrawImage, baseline_rsp, pxApiFixture, 30, 10000, 100)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};

    pxTextureRef nullMaskRef;
    pxTextureRef textureRef = context.createTexture(200, 200, 400, 400);
    
    context.drawImage(400, 400, 200, 200, textureRef, nullMaskRef, true, color, pxConstantsStretch::STRETCH, pxConstantsStretch::STRETCH, true, pxConstantsMaskOperation::NORMAL);
}

BASELINE_FIXED_F(_DrawImage9, baseline_rsp, pxApiFixture, 30, 10000, 100)
{
    context.setSize(1280, 720);
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    pxTextureRef textureRef9 = context.createTexture(100, 100, 200, 200);
    context.drawImage9(100, 100, 400, 400, 500, 500, textureRef9);
}
//-----------------------------------------------------------------------------------
//  pxMain:
//-----------------------------------------------------------------------------------
#define xstr(s) str(s)
#define str(s) #s

int pxMain(int argc, char* argv[])
{
    g_argv = (char**)malloc((argc+2) * sizeof(char*));
    g_origArgv = g_argv;
    
    char buffer[256];
    sprintf(buffer, "pxbenchmark: %s", xstr(PX_BENCHMARK_VERSION));
    
    int32_t windowWidth = rtGetEnvAsValue("PXBENCHMARK_WINDOW_WIDTH","1280").toInt32();
    int32_t windowHeight = rtGetEnvAsValue("PXBENCHMARK_WINDOW_HEIGHT","720").toInt32();
    
    rtValue screenWidth, screenHeight;
    if (RT_OK == rtSettings::instance()->value("screenWidth", screenWidth))
    windowWidth = screenWidth.toInt32();
    if (RT_OK == rtSettings::instance()->value("screenHeight", screenHeight))
    windowHeight = screenHeight.toInt32();
    
    if (argc > 3)
    {
        windowWidth = stoi(argv[3]);
        windowHeight = stoi(argv[4]);
    }
    // OSX likes to pass us some weird parameter on first launch after internet install
    rtLogInfo("window width = %d height = %d", windowWidth, windowHeight);
    win.init(10, 10, windowWidth, windowHeight, NULL);//, url);
    win.setTitle(buffer);
    
    // JRJR TODO Why aren't these necessary for glut... pxCore bug
    win.setVisibility(true);
    
    context.init();
    
    eventLoop.run();
    
    return 0;
}

