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
                                                                    1/*mIterations*/, mThreads,
                                                                    mExperimentFactory);
        
        g_argv[g_argc++] = (char*)mGroupName.c_str();
        g_argv[g_argc++] = (char*)("--archive=pxBenchmark_archive.csv");
        g_argv[g_argc++] = (char*)("--outputTable=pxBenchmark_outputTable.csv");
        /*g_argv[g_argc++] = (char*)mBenchmarkName.c_str();
        g_argv[g_argc++] = (char*)mSamples;
        g_argv[g_argc++] = (char*)mIterations;
        g_argv[g_argc++] = (char*)mThreads;*/
        
        // We will run some total number of sets of tests all together.
        // Each one growing by a power of 2.
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawRect)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawDiagLine)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawDiagRect)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawImage)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawImage9)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawImageBorder9)});
        mApiFixture->popExperimentValues().push_back({int64_t(pxApiFixture::type::xDrawAll)});
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
        
        if (mApiFixture->getIterationCounter() == 0)
        {
            context.setSize(win.GetWidth(), win.GetHeight());
            float fillColor[] = {1.0, 1.0, 0.0, 1.0};
            context.clear(0, 0, fillColor);
            // context.clear(1280, 720);
            //celero::Run();
        }
        
        if (mApiFixture->getIterationCounter() == mIterations)
            return;
        
        if (mApiFixture->getExperimentValues()[0].Iterations >= mIterations)
        {
            celero::Run(g_argc, g_argv);
            mApiFixture->setIterationCounter(mIterations);
            
        }
        else if (mApiFixture->getExperimentValues()[0].Iterations < mIterations)
            celero::Run();
    }
    
void benchmarkWindow::onAnimationTimer()
    {
        pxWindow::invalidateRect(NULL);
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

uint64_t benchmarkWindow::GetCurrentTimeElapsed() const
{
    return celero::timer::GetSystemTime() - mTimer;
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

pxApiFixture& pxApiFixture::Instance()
{
    static pxApiFixture singleton;
    return singleton;
}

uint64_t pxApiFixture::getIterationCounter() const
{
    return mIterationCounter;
}

void pxApiFixture::setIterationCounter(uint64_t val)
{
    mIterationCounter = val;
}

//-----------------------------------------------------------------------------------
/// \param threads The number of working threads.
/// \param iterations The number of times to loop over the UserBenchmark function.
/// \param experimentValue The experiment value to pass in setUp function.
///
/// \return Returns the number of microseconds the run took.
//-----------------------------------------------------------------------------------
uint64_t pxApiFixture::run(const uint64_t threads, const uint64_t iterations, const celero::TestFixture::ExperimentValue& experimentValue)
{
    // This function constitutes one sample consisting of several iterations for a single experiment value.
    
    if(this->HardCodedMeasurement() == 0)
    {
        uint64_t totalTime = 0;
        
        // Set up the testing fixture.
        this->setUp(experimentValue);
        
        // Run the test body for each iterations.
       // if (!mIterationCounter)
          //  mIterationCounter = iterations;
        
        // Get the starting time.
        const auto startTime = celero::timer::GetSystemTime();
        
        // Count down to zero
        // Iterations are used when the benchmarks are very fast.
        // Do not start/stop the timer inside this loop.
        // The purpose of the loop is to help counter timer quantization/errors.
        //while(mIterationCounter--)
        {
            this->onExperimentStart(experimentValue);
            
            this->UserBenchmark();
            
            this->onExperimentEnd();
            
            
          //  if (win.GetCurrentTimeElapsed() > 16000)
            //    return totalTime;
        }
        // See how long it took.
        totalTime += celero::timer::GetSystemTime() - startTime;
        
        for (int i = 0; i <= xDrawAll; ++i)
            if (mExperimentValue[i].Value == experimentValue.Value)
            {
                mExperimentValue[i].mTotalTime += totalTime;
                mExperimentValue[i].Iterations++;
                totalTime = mExperimentValue[i].mTotalTime;// / mExperimentValue[i].Iterations;
                mIterationCounter = mExperimentValue[i].Iterations;
                break;
            }
        
        // Tear down the testing fixture.
        this->tearDown();
        
        // Return the duration in microseconds for the given problem size.
        return totalTime;
    }
    
    return this->HardCodedMeasurement();
}

void pxApiFixture::TestDrawRect ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    
    context.drawRect(mUnitWidth, mUnitHeight, mCurrentX+mCurrentY, color, color);
}

void pxApiFixture::TestDrawDiagLine ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    context.drawDiagLine(mCurrentX, mCurrentY, mCurrentX+mUnitWidth, mCurrentY+mUnitHeight, color);
}

void pxApiFixture::TestDrawDiagRect ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    
    //float x = mExperimentValue[xDrawDiagRect].Iterations;
    //float y = mExperimentValue[xDrawDiagRect].Iterations;
    
    context.drawDiagRect(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, color);
}

void pxApiFixture::TestDrawImage ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    pxTextureRef nullMaskRef;
    pxTextureRef textureRef = context.createTexture(200, 200, 400, 400);
    context.drawImage(400, 400, 200, 200, textureRef, nullMaskRef, true, color, pxConstantsStretch::STRETCH, pxConstantsStretch::STRETCH, true, pxConstantsMaskOperation::NORMAL);
}

void pxApiFixture::TestDrawImage9 ()
{
    pxTextureRef textureRef9 = context.createTexture(100, 100, 200, 200);
    context.drawImage9(100, 100, 400, 400, 500, 500, textureRef9);
}

void pxApiFixture::TestDrawImage9Border ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    pxTextureRef textureRef9Border = context.createTexture(100, 100, 200, 200);
    context.drawImage9Border(100, 100, 400, 400, 500, 500, 0, 0, 0, 0, true, color, textureRef9Border);
}

void pxApiFixture::TestDrawAll ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    context.drawRect(100, 100, 400, color, color);
    
    pxTextureRef textureRef = context.createTexture(200, 200, 400, 400);
    context.drawDiagLine(400, 400, 500, 500, color);
    
    context.drawDiagRect(200, 200, 400, 400, color);
    
    pxTextureRef nullMaskRef;
    context.drawImage(400, 400, 200, 200, textureRef, nullMaskRef, true, color, pxConstantsStretch::STRETCH, pxConstantsStretch::STRETCH, true, pxConstantsMaskOperation::NORMAL);
    
    pxTextureRef textureRef9 = context.createTexture(100, 100, 200, 200);
    context.drawImage9(100, 100, 400, 400, 500, 500, textureRef9);
    
    pxTextureRef textureRef9Border = context.createTexture(100, 100, 200, 200);
    context.drawImage9Border(100, 100, 400, 400, 500, 500, 0, 0, 0, 0, true, color, textureRef9Border);
}

void pxApiFixture::onExperimentStart(const celero::TestFixture::ExperimentValue& exp)
{
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    if (mCurrentX > win.GetWidth() && mCurrentY > win.GetHeight())
    {
        mCurrentX = 0.0;
        mCurrentY = 0.0;
    }
    else if (mCurrentX < win.GetWidth())
        mCurrentX += mUnitWidth;
    else
    {
        mCurrentX = 0;
        mCurrentY += mUnitHeight;
    }
    
     switch ((int)exp.Value) {
        case xDrawRect:
            break;
        case xDrawDiagLine:
             TestDrawDiagLine();
             break;
        case xDrawDiagRect:
            TestDrawDiagRect();
            break;
        case xDrawImage9:
            break;
        case xDrawImage:
            break;
        case xDrawImageBorder9:
            break;
        case xDrawAll:
            break;
        default:
            break;
    }
    
}

void pxApiFixture::onExperimentEnd()
{
   // mProblemSpace.clear();
}

std::vector<celero::TestFixture::ExperimentValue> pxApiFixture::getExperimentValues() const
{
    std::vector<celero::TestFixture::ExperimentValue> problemSpace;
    for (int i = 0; i <= xDrawAll; ++i)
    {
        problemSpace.push_back(mExperimentValue[i]);
        //problemSpace[i].Iterations = 1;
    }
    return problemSpace;
}

std::vector<pxBenchmarkExperimentValue>& pxApiFixture::popExperimentValues()
{
    return mExperimentValue;
}

void pxApiFixture::setUp(const celero::TestFixture::ExperimentValue& experimentValue)
{
}

void pxApiFixture::tearDown()
{
    //mExperimentValue.clear();
}

void pxApiFixture::UserBenchmark()
{
}

/*BASELINE_F(_DrawRect, baseline, pxApiFixture, 30, 10000)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BENCHMARK_F(_DrawRect, baseline_rsp, pxApiFixture, 30, 10000)
{
    context.setSize(win.GetWidth(), win.GetHeight());
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};
    
    context.drawRect(100, 100, 400, color, color);
}*/

/*
BASELINE_FIXED_F(_DrawRect, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BASELINE_FIXED_F(_DrawDiagLine, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BASELINE_FIXED_F(_DrawDiagRect, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BASELINE_FIXED_F(_DrawImage, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BASELINE_FIXED_F(_DrawImage9, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
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

BASELINE_FIXED_F(_DrawImage9Border, baseline_rsp, pxApiFixture, 30, 10000, 1)
{
    context.setSize(win.GetWidth(), win.GetHeight());
    float fillColor[] = {1.0, 1.0, 0.0, 1.0};
    context.clear(0, 0, fillColor);
    // context.clear(1280, 720);
    
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
    static float color[4] = {1.0, 0.0, 0.0, 1.0};
    pxTextureRef textureRef9Border = context.createTexture(100, 100, 200, 200);
    context.drawImage9Border(100, 100, 400, 400, 500, 500, 0, 0, 0, 0, true, color, textureRef9Border);
}
*/
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
    
    uint32_t animationFPS = 60;
    win.setAnimationFPS(animationFPS);
    
    context.init();
    
    eventLoop.run();
    
    return 0;
}

