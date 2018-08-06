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

#include <cassert>
#include <cmath>

#include <celero/Archive.h>
#include <celero/Benchmark.h>
#include <celero/Callbacks.h>
#include <celero/Celero.h>
#include <celero/CommandLine.h>
#include <celero/Console.h>
#include <celero/Distribution.h>
#include <celero/Exceptions.h>
#include <celero/Executor.h>
#include <celero/JUnit.h>
#include <celero/Print.h>
#include <celero/ResultTable.h>
#include <celero/TestVector.h>
#include <celero/Utilities.h>
#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>


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

void benchmarkWindow::init(const int32_t& x, const int32_t& y, const int32_t& w, const int32_t& h, const int32_t& mw, const int32_t& mh)
{
    mApiFixture = std::shared_ptr<pxApiFixture>(&pxApiFixture::Instance());
    
    std::cout << "Writing results to: " << mOutputTableCSV << std::endl;
    celero::ResultTable::Instance().setFileName(mOutputTableCSV);
    
    celero::AddExperimentResultCompleteFunction([](std::shared_ptr<celero::ExperimentResult> p) { celero::ResultTable::Instance().add(p); });
    
    std::cout << "Archiving results to: " << mArchiveCSV << std::endl;
    celero::Archive::Instance().setFileName(mArchiveCSV);
    
    celero::AddExperimentResultCompleteFunction([](std::shared_ptr<celero::ExperimentResult> p) { celero::Archive::Instance().add(p); });
    
    print::TableBanner();
    
    pxWindow::init(x,y,w,h);
    
    mApiFixture->mUnitWidth = mw;
    
    mApiFixture->mUnitHeight = mh;
    
    mApiFixture->popExperimentValue().Value = pxApiFixture::type::xDrawRect;
    
    mApiFixture->popExperimentValue().Iterations = 0;
    
    mApiFixture->popExperimentValue().mTotalTime = 0;
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
    celero::ResultTable::Instance().closeFile();
    onCloseRequest();
}

void benchmarkWindow::onSize(/*const */int32_t/*&*/ w, /*const */int32_t/*&*/ h)
{
    if (mWidth != w || mHeight != h)
    {
        mWidth  = w;
        mHeight = h;
        reset();
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

void benchmarkWindow::RegisterTest (const string& groupName, const string& benchmarkName, const uint64_t samples,
                                    const uint64_t iterations, const uint64_t threads)
{
    if (NULL == mExperimentFactory)
        mExperimentFactory = std::shared_ptr<pxBenchmarkFactory>(&pxBenchmarkFactory::Instance());
    
    // TODO celero::TestVector::Instance().clear();
    
    mBaselineBm = celero::RegisterBaseline(groupName.c_str(), benchmarkName.c_str(), samples,
                                           iterations, threads,
                                           mExperimentFactory);
    
    /*mBms.push_back (celero::RegisterTest(groupName.c_str(), benchmarkName.c_str(), samples,
     iterations, threads,
     mExperimentFactory));*/
    /* g_argc = 0;
     g_argv[g_argc++] = (char*)groupName.c_str();
     g_argv[g_argc++] = (char*)("--archive=pxBenchmark_archive.csv");
     g_argv[g_argc++] = (char*)("--outputTable=pxBenchmark_outputTable.csv");
     
     string tag = "--group=" + groupName;
     g_argv[g_argc++] = (char*)tag.c_str();*/
}

void benchmarkWindow::reset()
{
    if (mExperimentFactory == NULL)
        mExperimentFactory = std::shared_ptr<pxBenchmarkFactory>(&pxBenchmarkFactory::Instance());
    
    switch ((int)mApiFixture->popExperimentValue().Value) {
        case pxApiFixture::type::xDrawRect:
            mGroupName = "DrawRect";
            break;
        case pxApiFixture::type::xDrawDiagLine:
            mGroupName = "DrawDiagLine";
            break;
        case pxApiFixture::type::xDrawDiagRect:
            mGroupName = "DrawDiagRect";
            break;
        case pxApiFixture::type::xDrawImage9:
            mGroupName = "DrawImage9";
            break;
        case pxApiFixture::type::xDrawImage:
            mGroupName = "DrawImage";
            break;
        case pxApiFixture::type::xDrawImageBorder9:
            mGroupName = "DrawImageBorder9";
            break;
        case pxApiFixture::type::xDrawImageMasked:
            mGroupName = "DrawImageMasked";
            break;
        case pxApiFixture::type::xDrawTextureQuads:
            mGroupName = "DrawTextureQuads";
            break;
        //case pxApiFixture::type::xDrawOffscreen:
        //    mGroupName = "DrawOffscreen";
        //    break;
        case pxApiFixture::type::xDrawAll:
            mGroupName = "DrawAll";
            break;
        default:
            break;
    }
    
    RegisterTest(mGroupName, mBenchmarkName, mSamples, 1, mThreads);
    
    mApiFixture->popExperimentValue().Iterations = 0;
    mApiFixture->popExperimentValue().mTotalTime = 0;
    mApiFixture->mCurrentY = 0;
    mApiFixture->mCurrentX = 0;
    
    pxWindow::invalidateRect(NULL);
}

void benchmarkWindow::onDraw(pxSurfaceNative/*&*/ sn)
{
    if (mApiFixture->getIterationCounter() == 0)
    {
        context.setSize(win.GetWidth(), win.GetHeight());
        float fillColor[] = {1.0, 1.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
        //context.clear(1280, 720);
    }
    
    if (mApiFixture->getIterationCounter() <= mIterations)
        celero::executor::Run(mGroupName);
    else if (mApiFixture->popExperimentValue().Value == pxApiFixture::type::xDrawAll)
    {
        celero::ResultTable::Instance().closeFile();
        return;
    }
    else
    {
        context.setSize(win.GetWidth(), win.GetHeight());
        float fillColor[] = {1.0, 1.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
        //context.clear(1280, 720);
        
        mApiFixture->popExperimentValue().Value++;
        
        mApiFixture->setIterationCounter(0);
        
        reset ();
    }
    
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
//  pxBenchmarkFactory:
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
        
        mExperimentValue.mTotalTime += totalTime;
        mExperimentValue.Iterations++;
        totalTime = mExperimentValue.mTotalTime;// / mExperimentValue[i].Iterations;
        mIterationCounter++;
        
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
    //context.clear(1280, 720);
    context.drawRect(mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, 1, NULL, color);
}

void pxApiFixture::TestDrawDiagLine ()
{
    static float color[4] = {0., 0.0, 1.0, 1.0};
    //context.clear(1280, 720);
    context.drawDiagLine(mCurrentX, mCurrentY, mCurrentX+mUnitWidth, mCurrentY+mUnitHeight, color);
}

void pxApiFixture::TestDrawDiagRect ()
{
    static float color[4] = {0., 1.0, 0.0, 1.0};
    //context.clear(1280, 720);
    context.drawDiagRect(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, color);
}

void pxApiFixture::TestDrawImage ()
{
    //context.clear(1280, 720);
    rtString settingsPath;
    string url = "Resources/" + to_string((mExperimentValue.Iterations%15)+1) + ".jpg";
    if (RT_OK == rtGetHomeDirectory(settingsPath))
        url = settingsPath.cString() + url;
    
    //if (!rtFileExists((char*)url.c_str()))
    //    return;
    
    rtRef<rtImageResource> resource = pxImageManager::getImage((char*)url.c_str());
    pxTextureRef texture = resource->getTexture();
    if (texture == NULL)
        return;
    
    //pxTimedOffscreenSequence& imageSequence = resource->getTimedOffscreenSequence();
    
    //pxOffscreen &o = imageSequence.getFrameBuffer(mCurFrame);
    // context.createTexture(o)
    //static float color[4] = {1., 0.0, 0.0, 1.0};
    
    //pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    
   // pxContextFramebufferRef maskSnapshot = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    
    //
    context.drawImage(mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, resource->getTexture(), nullptr, true, NULL, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : pxConstantsStretch::REPEAT, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : ((int)mCurrentY) % 2 == 0 ? pxConstantsStretch::REPEAT : pxConstantsStretch::NONE, true, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::NORMAL : pxConstantsMaskOperation::INVERT);
}

void pxApiFixture::TestDrawImage9 ()
{
    //pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    //context.clear(1280, 720);
    
    rtString settingsPath;
    string url = "Resources/" + to_string((mExperimentValue.Iterations%15)+1) + ".jpg";
    if (RT_OK == rtGetHomeDirectory(settingsPath))
        url = settingsPath.cString() + url;
    
    //if (!rtFileExists((char*)url.c_str()))
    //    return;
    
    rtRef<rtImageResource> resource = pxImageManager::getImage((char*)url.c_str());
    pxTextureRef texture = resource->getTexture();
    if (texture == NULL)
        return;
    
    context.drawImage9(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, texture);
}

void pxApiFixture::TestDrawImage9Border ()
{
    //pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    
    static float color[4] = {0., 1.0, 0.0, 1.0};
    rtString settingsPath;
    string url = "Resources/" + to_string((mExperimentValue.Iterations%15)+1) + ".jpg";
    if (RT_OK == rtGetHomeDirectory(settingsPath))
        url = settingsPath.cString() + url;
    
    //if (!rtFileExists((char*)url.c_str()))
    //    return;
    
    rtRef<rtImageResource> resource = pxImageManager::getImage((char*)url.c_str());
    pxTextureRef texture = resource->getTexture();
    if (texture == NULL)
        return;
    
    context.drawImage9Border(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, ((int)mCurrentX) % 2 == 0 ? true : false, color, texture);
}

void pxApiFixture::TestDrawImageMasked ()
{
    //pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    
    //pxContextFramebufferRef maskSnapshot = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    //context.clear(1280, 720);
    
    rtString settingsPath;
    string url = "Resources/" + to_string((mExperimentValue.Iterations%15)+1) + ".jpg";
    if (RT_OK == rtGetHomeDirectory(settingsPath))
        url = settingsPath.cString() + url;
    
    rtRef<rtImageResource> resource = pxImageManager::getImage((char*)url.c_str());
    pxTextureRef texture = resource->getTexture();
    if (texture == NULL)
        return;
    
    
    url = "Resources/" + to_string((mExperimentValue.Iterations%15)+2) + ".jpg";
    
    url = settingsPath.cString() + url;
    
    resource = pxImageManager::getImage((char*)url.c_str());
   
    context.drawImageMasked(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::constants::NORMAL : pxConstantsMaskOperation::constants::INVERT, texture, resource->getTexture());
}

void pxApiFixture::TestDrawTextureQuads()
{
    pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    
    pxTextureRef textureRef = drawableSnapshotForMask->getTexture();/* context.createTexture(static_cast<float>(mUnitWidth), static_cast<float>(mUnitHeight),
                                                    static_cast<float>(mUnitWidth), static_cast<float>(mUnitHeight),
                                                    NULL);*/
    
    static float color[4] = {1., 0.0, 0.0, 1.0};
    const float verts[6][2] =
    {
        { mCurrentX,     mCurrentY },
        { mCurrentX + mUnitWidth,   mCurrentY },
        { mCurrentX,   mCurrentY + mUnitHeight },
        { mCurrentX + mUnitWidth,   mCurrentY },
        { mCurrentX,   mCurrentY + mUnitHeight },
        { mCurrentX + mUnitWidth, mCurrentY + mUnitHeight }
    };
    float u1 = 0;//((int)mCurrentX) % 2 == 0 ? 0 : 1;
    float v1 = 1;//((int)mCurrentY) % 2 == 0 ? 1 : 0;
    float u2 = 1;//((int)mCurrentX) % 2 == 0 ? 1 : 0;
    float v2 = 0;//((int)mCurrentY) % 2 == 0 ? 0 : 1;
    const float uvs[6][2] =
    {
        { u1, v1  },
        { u2, v1  },
        { u1, v2 },
        { u2, v1  },
        { u1, v2 },
        { u2, v2 }
    };
    
    context.drawTexturedQuads(1, verts, uvs, textureRef, color);
}

void pxApiFixture::TestDrawOffscreen()
{
    /*pxContextFramebufferRef drawableSnapshotForMask = context.createFramebuffer(static_cast<int>(floor(mUnitWidth)), static_cast<int>(floor(mUnitHeight)));
    pxOffscreen offscreen;
    context.drawOffscreen(mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mUnitWidth, mUnitHeight, offscreen);*/
}

void pxApiFixture::TestDrawAll ()
{
    TestDrawRect();
    
    TestDrawDiagLine();
    
    TestDrawDiagRect();
    
    TestDrawImage9Border();
    
    TestDrawImage();
    
    TestDrawImage9();
    
    TestDrawImageMasked();
    
    TestDrawTextureQuads();
    
    TestDrawOffscreen();
}

void pxApiFixture::onExperimentStart(const celero::TestFixture::ExperimentValue& exp)
{
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    if (mCurrentX >= win.GetWidth() && mCurrentY >= win.GetHeight())
    {
        mCurrentX = 0.0;
        mCurrentY = 0.0;
        
        context.setSize(win.GetWidth(), win.GetHeight());
        
        float fillColor[] = {1.0, 1.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
    }
    else if (mCurrentX < win.GetWidth())
        mCurrentX += mUnitWidth;
    else
    {
        mCurrentX = 0;
        mCurrentY += mUnitHeight;
    }
    
    switch ((int)mExperimentValue.Value) {
        case xDrawRect:
            TestDrawRect();
            break;
        case xDrawDiagLine:
            TestDrawDiagLine();
            break;
        case xDrawDiagRect:
            TestDrawDiagRect();
            break;
        case xDrawImage9:
            TestDrawImage9();
            break;
        case xDrawImage:
            TestDrawImage();
            break;
        case xDrawImageBorder9:
            TestDrawImage9Border();
            break;
        case xDrawImageMasked:
            TestDrawImageMasked();
            break;
        case xDrawTextureQuads:
            TestDrawTextureQuads();
            break;
        //case xDrawOffscreen:
        //    TestDrawOffscreen();
        //    break;
        case xDrawAll:
            TestDrawAll();
            break;
        default:
            break;
    }
}

void pxApiFixture::onExperimentEnd()
{
}

std::vector<celero::TestFixture::ExperimentValue> pxApiFixture::getExperimentValues() const
{
    std::vector<celero::TestFixture::ExperimentValue> problemSpace;
    problemSpace.push_back(mExperimentValue);
    return problemSpace;
}

pxBenchmarkExperimentValue& pxApiFixture::popExperimentValue()
{
    return mExperimentValue;
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
    g_origArgv = argv;
    
    char buffer[256];
    sprintf(buffer, "pxbenchmark: %s", xstr(PX_BENCHMARK_VERSION));
    
    int32_t windowWidth = rtGetEnvAsValue("PXBENCHMARK_WINDOW_WIDTH","1280").toInt32();
    
    int32_t windowHeight = rtGetEnvAsValue("PXBENCHMARK_WINDOW_HEIGHT","720").toInt32();
    
    rtValue screenWidth, screenHeight;
    if (RT_OK == rtSettings::instance()->value("screenWidth", screenWidth))
        windowWidth = screenWidth.toInt32();
    
    if (RT_OK == rtSettings::instance()->value("screenHeight", screenHeight))
        windowHeight = screenHeight.toInt32();
    
    
    int32_t unitW = 25;
    int32_t unitH = 25;
    if (argc > 3)
    {
        unitW = stoi(argv[3]);
        unitH = stoi(argv[4]);
    }
    
    if (argc > 5)
    {
        windowWidth = stoi(argv[5]);
        windowHeight = stoi(argv[6]);
    }
    // OSX likes to pass us some weird parameter on first launch after internet install
    rtLogInfo("window width = %d height = %d", windowWidth, windowHeight);
    
    win.init(0, 0, windowWidth, windowHeight, unitW, unitH);
    
    win.setTitle(buffer);
    
    // JRJR TODO Why aren't these necessary for glut... pxCore bug
    win.setVisibility(true);
    
    uint32_t animationFPS = 60;
    win.setAnimationFPS(animationFPS);
    
    context.init();
    
    eventLoop.run();
    
    return 0;
}

