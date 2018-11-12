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
#include "pxTimer.h"


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

int MAX_IMAGES_CNT = 42;
int gFPS = 60;
uint64_t gGPU = 0;
uint64_t gCPU = 0;
uint64_t gTotal = 0;
uint64_t gOther = 0;
uint64_t gOtherStart = 0;
string   gFirmware = "";
string   gDeviceType = "";

const int gDuration = 2;

#ifdef ENABLE_DEBUG_MODE
extern int          g_argc;
extern char**       g_argv;
char** g_origArgv = NULL;
#endif

//-----------------------------------------------------------------------------------
// class pxbenchmarkWindow
//-----------------------------------------------------------------------------------

void benchmarkWindow::init(const int32_t& x, const int32_t& y, const int32_t& w, const int32_t& h, const int32_t& mw, const int32_t& mh, const bool doArchive/* = false*/, bool doCreateTexture /*= true*/)
{
    mApiFixture = std::shared_ptr<pxApiFixture>(new pxApiFixture());
    
    std::cout << "Writing results to: " << GetOutPath() + mOutputTableCSV << std::endl;
    celero::ResultTable::Instance().setFileName(GetOutPath() + mOutputTableCSV);
    
    celero::AddExperimentResultCompleteFunction([](std::shared_ptr<celero::ExperimentResult> p) { celero::ResultTable::Instance().add(p); });
    
    if (doArchive)
    {
        std::cout << "Archiving results to: " << GetOutPath() + mArchiveCSV << std::endl;
        celero::Archive::Instance().setFileName(GetOutPath() + mArchiveCSV);
        
        celero::AddExperimentResultCompleteFunction([](std::shared_ptr<celero::ExperimentResult> p) { celero::Archive::Instance().add(p); });
    }
    
    print::TableBanner();
    
    mApiFixture->popExperimentValue().Value = pxApiFixture::type::xDrawRect;
    
    pxWindow::init(x,y,w,h);
    
    mApiFixture->mUnitWidth = mw;
    
    mApiFixture->mUnitHeight = mh;
    
    mApiFixture->popExperimentValue().Iterations = 0;
    
    mApiFixture->popExperimentValue().mTotalTime = 0;
    
    mApiFixture->SetDoCreateTexture(doCreateTexture);
    
    mTexture.init(mWidth, mHeight);
    
   // drawBackground(mTexture);
}

void benchmarkWindow::drawBackground(pxBuffer& b)
{
    // Fill the buffer with a simple pattern as a function of f(x,y)
    int w = b.width();
    int h = b.height();
    
    for (int y = 0; y < h; y++)
    {
        pxPixel* p = b.scanline(y);
        for (int x = 0; x < w; x++)
        {
            p->r = pxClamp<int>(x+y, 255);
            p->g = pxClamp<int>(y,   255);
            p->b = pxClamp<int>(x,   255);
            p++;
        }
    }
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
    
    celero::ResultTable::Instance().closeFile();
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
        mExperimentFactory = std::shared_ptr<pxBenchmarkFactory>(new pxBenchmarkFactory());
    
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
        mExperimentFactory = std::shared_ptr<pxBenchmarkFactory>(new pxBenchmarkFactory());
    
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
        case pxApiFixture::type::xDrawImageJPG:
            mGroupName = "DrawImageJPG";
            break;
        case pxApiFixture::type::xDrawImagePNG:
            mGroupName = "DrawImagePNG";
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
        /*case pxApiFixture::type::xDrawImage9Ran:
            mGroupName = "DrawImage9Ran";
            break;
        case pxApiFixture::type::xDrawImageRan:
            mGroupName = "DrawImageRan";
            break;
        case pxApiFixture::type::xDrawImageBorder9Ran:
            mGroupName = "DrawImageBorder9Ran";
            break;
        case pxApiFixture::type::xDrawImageMaskedRan:
            mGroupName = "DrawImageMaskedRan";
            break;
        case pxApiFixture::type::xDrawTextureQuadsRan:
            mGroupName = "DrawTextureQuadsRan";
            break;
        //case pxApiFixture::type::xDrawOffscreen:
        //    mGroupName = "DrawOffscreen";
        //    break;
         */
        case pxApiFixture::type::xDrawAll:
            mGroupName = "DrawAll";
            break;
        default:
            break;
    }
    
    string experimentName = to_string((int)mApiFixture->mUnitWidth) + "x" + to_string((int)mApiFixture->mUnitHeight);
    RegisterTest(mGroupName, experimentName, mSamples, 1, mThreads);
    
    mApiFixture->popExperimentValue().Iterations = 0;
    //mApiFixture->popExperimentValue().mTotalTime = 0;
    mApiFixture->mCurrentY = 0;
    mApiFixture->mCurrentX = 0;
    
    pxWindow::invalidateRect(NULL);
}

void benchmarkWindow::onDraw(pxSurfaceNative/*&*/ sn)
{
    
    if (mApiFixture->getIterationCounter() == 0)
    {
        context.setSize(win.GetWidth(), win.GetHeight());
        //float fillColor[] = {1.0, 1.0, 0.0, 1.0};
        //context.clear(0, 0, fillColor);
        context.clear(win.GetWidth(), win.GetHeight());
        
        float fillColor[] = {0.0, 0.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
        
    }
    
    if (mApiFixture->getIterationCounter() <= mIterations)
    {
        celero::executor::Run(mGroupName);
    }
    else if (mApiFixture->popExperimentValue().Value == pxApiFixture::type::xEnd || mApiFixture->popExperimentValue().Value == pxApiFixture::type::xDrawAll)
    {
        if (mApiFixture->popExperimentValue().Value == pxApiFixture::type::xDrawAll)
        {
            mApiFixture->popExperimentValue().Value++;
            gTotal = (celero::timer::GetSystemTime() - gTotal);
            gOther += (celero::timer::GetSystemTime() - gOtherStart);
            vector<string> list(6);
            
            list[0] = "Device Type";
            list[1] = "Firmware";
            list[2] = "Date";
            list[3] = "GPU(ms)";
            list[4] = "CPU(ms)";
            list[5] = "NOTES";
            celero::ResultTable::Instance().add(list);
            
            list[0] = gDeviceType;
            list[1] = gFirmware;
            
            time_t rawtime;
            struct tm * timeinfo;
            char buffer[80];
            
            time (&rawtime);
            timeinfo = localtime(&rawtime);
            
            strftime(buffer,sizeof(buffer),"%m\/%d\/%Y",timeinfo);
            std::string str(buffer);
            
            
            list[2] = str;
            list[3] = to_string((int)(gGPU*0.001));
            list[4] = to_string((int)((gCPU+gOther)*0.001));
            list[5] = "Total(ms)=" + to_string((int)(gTotal*0.001)) + "FPS:=" + to_string((int)(gFPS));
            celero::ResultTable::Instance().add(list);
            
            celero::ResultTable::Instance().closeFile();
            system("/bin/bash -c ./automation.sh &");
#if PX_PLATFORM_GENERIC_EGL
            //string cmnd = "libreoffice --calc " + mOutPath + mOutputTableCSV;
            //system(cmnd.c_str());
#else
            string cmnd = "open " + mOutPath + mOutputTableCSV;
            system(cmnd.c_str());
#endif
        }
        std::cout << "Results logged to " << mOutPath + mOutputTableCSV << std::endl;
        exit(0);
        return;
    }
    else
    {
        context.setSize(win.GetWidth(), win.GetHeight());
        //float fillColor[] = {1.0, 1.0, 0.0, 1.0};
        //context.clear(0, 0, fillColor);
        context.clear(win.GetWidth(), win.GetHeight());
        
        float fillColor[] = {0.0, 0.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
        
        mApiFixture->popExperimentValue().Value++;
        
        mApiFixture->setIterationCounter(0);
        
        reset ();
    }
    
    // Draw the texture into this window
   // mTexture.blit(sn);
    
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

/*pxBenchmarkFactory& pxBenchmarkFactory::Instance()
{
    static pxBenchmarkFactory singleton;
    return singleton;
}*/

//-----------------------------------------------------------------------------------
//  pxApiFixture:
//-----------------------------------------------------------------------------------

/*pxApiFixture& pxApiFixture::Instance()
{
    static pxApiFixture singleton;
    return singleton;
}*/

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
        
        gOther += (celero::timer::GetSystemTime() - gOtherStart);
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
        totalTime = (celero::timer::GetSystemTime() - startTime);
        
        gOtherStart = celero::timer::GetSystemTime();
        
        if (mExperimentValue.Value == xDrawImageJPG || mExperimentValue.Value == xDrawImagePNG)
            gCPU += totalTime;
        else
            gGPU += totalTime;
       
        //std::chrono::microseconds ms(totalTime);
        
        //std::chrono::seconds secs = std::chrono::duration_cast<std::chrono::seconds>(ms);
        
        //mExperimentValue.mFPS = 60 * (secs.count() + 1);
        
        //gFPS += 60 / totalTime;
        
        mExperimentValue.mTotalTime += totalTime;
        mExperimentValue.Iterations++;
        //totalTime = mExperimentValue.mTotalTime;// / mExperimentValue[i].Iterations;
        mIterationCounter++;
        
        if (mExp != nullptr)
        {
            string experimentName = to_string((int)mExperimentValue.mTotalTime);
        
            mExp->setName (experimentName);
            //win.popBaselineBm()->setBaseline(exp);
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
    context.drawRect(mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, 1, NULL, color);
}

void pxApiFixture::TestDrawDiagLine ()
{
    static float color[4] = {0., 0.0, 1.0, 1.0};
    context.drawDiagLine(mCurrentX, mCurrentY, mCurrentX+mUnitWidth, mCurrentY+mUnitHeight, color);
}

void pxApiFixture::TestDrawDiagRect ()
{
    static float color[4] = {0., 1.0, 0.0, 1.0};
    context.drawDiagRect(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, color);
}

pxTextureRef pxApiFixture::GetImageTexture (const string& format)
{
    string url = "/tmp/Resources/" + to_string((mExperimentValue.Iterations % MAX_IMAGES_CNT) + 1) + format;
    
    //url = win.GetOutPath() + url;
    
    if (!rtFileExists((char*)url.c_str()))
        return NULL;
    
    rtRef<rtImageResource> resource = pxImageManager::getImage((char*)url.c_str());
    pxTextureRef texture = resource->getTexture();
    if (texture == NULL)
        return NULL;
    resource->clearDownloadRequest();
    return texture;
}

void pxApiFixture::TestDrawImage ()
{
    static float color[4] = {0., 0.0, 1.0, 1.0};
    
    context.drawImage(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, mTextureRef, mTextureMaskRef, true, color, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : pxConstantsStretch::REPEAT, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : ((int)mCurrentY) % 2 == 0 ? pxConstantsStretch::REPEAT : pxConstantsStretch::NONE, true, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::NORMAL : pxConstantsMaskOperation::INVERT);
}

void pxApiFixture::TestDrawImage9 ()
{
    context.drawImage9(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mTextureRef);
}

void pxApiFixture::TestDrawImage9Border ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    
    context.drawImage9Border(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, false, color, mTextureRef);
}

void pxApiFixture::TestDrawImageMasked ()
{
    context.drawImageMasked(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::constants::NORMAL : pxConstantsMaskOperation::constants::INVERT, mTextureRef, mTextureMaskRef);
}

void pxApiFixture::TestDrawTextureQuads()
{
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
    float u1 = ((int)mCurrentX) % 2 == 0 ? 0 : 1;
    float v1 = ((int)mCurrentY) % 2 == 0 ? 1 : 0;
    float u2 = ((int)mCurrentX) % 2 == 0 ? 1 : 0;
    float v2 = ((int)mCurrentY) % 2 == 0 ? 0 : 1;
    const float uvs[6][2] =
    {
        { u1, v1 },
        { u2, v1 },
        { u1, v2 },
        { u2, v1 },
        { u1, v2 },
        { u2, v2 }
    };
    
    context.drawTexturedQuads(1, verts, uvs, mTextureRef, color);
}

void pxApiFixture::TestDrawOffscreen()
{
    /*pxOffscreen offscreen;
    context.drawOffscreen(mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mUnitWidth, mUnitHeight, offscreen);*/
}

void pxApiFixture::TestDrawImageJPG ()
{
    if (mTextureRef != NULL)
        mTextureRef->deleteTexture();
    
    mTextureRef = GetImageTexture (".jpg");
    
    mTextureMaskRef = NULL;
    
    context.drawImage(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, mTextureRef, mTextureMaskRef, false, NULL, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : pxConstantsStretch::REPEAT, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : ((int)mCurrentY) % 2 == 0 ? pxConstantsStretch::REPEAT : pxConstantsStretch::NONE, true, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::NORMAL : pxConstantsMaskOperation::INVERT);
}

void pxApiFixture::TestDrawImagePNG ()
{
    if (mTextureRef != NULL)
        mTextureRef->deleteTexture();
    
    mTextureRef = GetImageTexture (".png");
    
    mTextureMaskRef = NULL;
    
    context.drawImage(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, mTextureRef, mTextureMaskRef, false, NULL, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : pxConstantsStretch::REPEAT, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : ((int)mCurrentY) % 2 == 0 ? pxConstantsStretch::REPEAT : pxConstantsStretch::NONE, true, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::NORMAL : pxConstantsMaskOperation::INVERT);
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

void pxApiFixture::TestDrawImageRan ()
{
    static float color[4] = {0., 0.0, 1.0, 1.0};

    context.drawImage(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, mTextureRef, mTextureMaskRef, true, color, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : pxConstantsStretch::REPEAT, ((int)mCurrentX) % 2 == 0 ? pxConstantsStretch::STRETCH : ((int)mCurrentY) % 2 == 0 ? pxConstantsStretch::REPEAT : pxConstantsStretch::NONE, true, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::NORMAL : pxConstantsMaskOperation::INVERT);
}

void pxApiFixture::TestDrawImage9Ran ()
{
    context.drawImage9(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mTextureRef);
}

void pxApiFixture::TestDrawImage9BorderRan ()
{
    static float color[4] = {1., 0.0, 0.0, 1.0};
    
    context.drawImage9Border(mUnitWidth, mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, mCurrentX, mCurrentY, mCurrentX + mUnitWidth, mCurrentY + mUnitHeight, false, color, mTextureRef);
}

void pxApiFixture::TestDrawImageMaskedRan ()
{
    context.drawImageMasked(mCurrentX, mCurrentY, mUnitWidth, mUnitHeight, ((int)mCurrentX) % 2 == 0 ? pxConstantsMaskOperation::constants::NORMAL : pxConstantsMaskOperation::constants::INVERT, mTextureRef, mTextureMaskRef);
}

void pxApiFixture::TestDrawTextureQuadsRan()
{
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
    float u1 = ((int)mCurrentX) % 2 == 0 ? 0 : 1;
    float v1 = ((int)mCurrentY) % 2 == 0 ? 1 : 0;
    float u2 = ((int)mCurrentX) % 2 == 0 ? 1 : 0;
    float v2 = ((int)mCurrentY) % 2 == 0 ? 0 : 1;
    const float uvs[6][2] =
    {
        { u1, v1 },
        { u2, v1 },
        { u1, v2 },
        { u2, v1 },
        { u1, v2 },
        { u2, v2 }
    };
    
    context.drawTexturedQuads(1, verts, uvs, mTextureRef, color);
}

void pxApiFixture::onExperimentStart(const celero::TestFixture::ExperimentValue& exp)
{
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
        case xDrawImageJPG:
            TestDrawImageJPG();
            break;
        case xDrawImagePNG:
            TestDrawImagePNG();
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
        /*case xDrawImage9Ran:
            TestDrawImage9Ran();
            break;
        case xDrawImageRan:
            TestDrawImageRan();
            break;
        case xDrawImageBorder9Ran:
            TestDrawImage9BorderRan();
            break;
        case xDrawImageMaskedRan:
            TestDrawImageMaskedRan();
            break;
        case xDrawTextureQuadsRan:
            TestDrawTextureQuadsRan();
            break;*/
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

void random( pxBuffer& b, long double xmin, long double xmax, long double ymin, long double ymax, unsigned maxiter )
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

pxTextureRef pxApiFixture::CreateTexture ()
{
    pxOffscreen o;
    o.init(mUnitWidth, mUnitHeight);
    random(o, -3, 2, -2.5, 2.5, 18);
    // premultiply
    for (int y = 0; y < mUnitHeight; y++)
    {
        pxPixel* d = o.scanline(y);
        pxPixel* de = d + (int)mUnitWidth;
        //int x = 0;
        while (d < de)
        {
             d->r = rand() / 255;// (d->r * d->a)/255;
             d->g = rand() / 255;// (d->g * d->a)/255;
             d->b = rand() / 255; //(d->b * d->a)/255;
            
            
           /* double xyValue = x * 5.0 / mUnitWidth + y * 10.0 / mUnitHeight + 5.0 * rand() / 256.0;
            double sineValue = 226 * fabs(sin(xyValue * 3.14159));
            d->r = 30 + sineValue;
            d->g = 10 + sineValue;
            d->b = sineValue;*/
            
            d++;
            
        }
    }
    
    return context.createTexture(o);
}
void pxApiFixture::setUp(const celero::TestFixture::ExperimentValue& experimentValue)
{
    pxMatrix4f m;
    context.setMatrix(m);
    
    context.setAlpha(1.0);
    
    context.setShowOutlines(true);
    
   // context.clear(win.GetWidth(), win.GetHeight());
   /*
    switch ((int)mExperimentValue.Value) {
        case xDrawImage9Ran:
        case xDrawImageRan:
        case xDrawImageBorder9Ran:
        case xDrawImageMaskedRan:
        case xDrawTextureQuadsRan:
        case xDrawImageJPG:
        case xDrawImagePNG:
           mUnitWidth = (rand() % (int)win.GetWidth());// / (int)win.GetWidth();
            mUnitWidth = mUnitWidth < 50 ? 50 : mUnitWidth;
            mUnitHeight = (rand() % (int)win.GetHeight());// / (int)win.GetHeight();
            mUnitHeight = mUnitHeight < 50 ? 50 : mUnitHeight;
        
       // mUnitHeight = (mCurrentY + mUnitHeight*50) > win.GetHeight() ? 100 : (mUnitHeight + mUnitHeight * 50);
       // mUnitWidth = (mCurrentX + mUnitWidth*50) > win.GetWidth() ? 100 : (mUnitWidth + mUnitWidth * 50);
        
            context.clear(win.GetWidth(), win.GetHeight());
            //win.SetIterations (100); // TEMP DEMO
            break;
        default:
        //mUnitHeight = mCurrentY + mUnitHeight*2 > win.GetHeight() ? 100 : mUnitHeight + mUnitHeight * 2;
        //mUnitWidth = mCurrentX + mUnitWidth*2 > win.GetWidth() ? 100 : mUnitWidth + mUnitWidth * 2;
           // win.SetIterations (1056); // TEMP DEMO
            break;
    }*/
    
    
    if (NULL != mTextureRef)
    {
        mTextureRef->deleteTexture();
        mTextureRef = NULL;
    }
    
    if (mExperimentValue.Value != xDrawImagePNG && mExperimentValue.Value != xDrawImageJPG)
        mTextureRef = mDoCreateTexture ? CreateTexture() : GetImageTexture ("jpg");
    
    //mTextureRef = context.createTexture(win.GetTexture());
    mTextureMaskRef = NULL;// CreateTexture();//maskSnapshot->getTexture();
    
    //if (win.popBaselineBm()->getExperimentSize() > 0)
    {
        mExp = win.popBaselineBm()->getBaseline();
        //shared_ptr<Experiment> exp = win.popBaselineBm()->getBaseline();
        //string experimentName = to_string((int)mUnitWidth) + "x" + to_string((int)mUnitHeight);
        
        //exp->setName (experimentName);
        //win.popBaselineBm()->setBaseline(exp);
    }
}

void pxApiFixture::tearDown()
{
    if (mCurrentX >= win.GetWidth() && mCurrentY >= win.GetHeight())
    {
        mCurrentX = 0.0;
        mCurrentY = 0.0;
        
        context.setSize(win.GetWidth(), win.GetHeight());
        
        context.clear(win.GetWidth(), win.GetHeight());
        
        float fillColor[] = {0.0, 0.0, 0.0, 1.0};
        context.clear(0, 0, fillColor);
    }
    else if (mCurrentX < win.GetWidth())
        mCurrentX += mUnitWidth;
    else
    {
        mCurrentX = 0;
        mCurrentY += mUnitHeight;
    }
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
    gTotal = celero::timer::GetSystemTime();
    
    gOtherStart = celero::timer::GetSystemTime();
    
#ifdef ENABLE_DEBUG_MOD
    g_argv = (char**)malloc((argc+2) * sizeof(char*));
    g_origArgv = argv;
#endif //ENABLE_DEBUG_MOD
    
    char buffer[256];
    sprintf(buffer, "pxbenchmark: %s", xstr(PX_BENCHMARK_VERSION));
    
    int32_t windowWidth = rtGetEnvAsValue("PXBENCHMARK_WINDOW_WIDTH","1280").toInt32();
    
    int32_t windowHeight = rtGetEnvAsValue("PXBENCHMARK_WINDOW_HEIGHT","720").toInt32();
    
    rtValue screenWidth, screenHeight;
    if (RT_OK == rtSettings::instance()->value("screenWidth", screenWidth))
        windowWidth = screenWidth.toInt32();
    
    if (RT_OK == rtSettings::instance()->value("screenHeight", screenHeight))
        windowHeight = screenHeight.toInt32();
    
    cout<<argv;
    int32_t unitW = 25;
    int32_t unitH = 25;
    
    for(int i = 0; i < argc; i++){
        printf("Argument %i = %s\n", i, argv[i]);}
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
    
    bool doArchive = false;
    /*if (argc > 6)
    {
        gDeviceType = std::string(argv[7]);
        cout<<gDeviceType;
    }
    
    if (argc > 7)
    {
        gFirmware = std::string(argv[8]);
    }
    
    if (argc > 8)
        doArchive = stoi(argv[9]) == 1 ? true : false;
    
    if (argc > 9)
    {
        std::string path(argv[10]);
        win.SetOutPath(path);
    }
     */
    // OSX likes to pass us some weird parameter on first launch after internet install
    rtLogInfo("window width = %d height = %d", windowWidth, windowHeight);
    
    win.init(0, 0, windowWidth, windowHeight, unitW, unitH, doArchive, true);
    
    win.setTitle(buffer);
    
    // JRJR TODO Why aren't these necessary for glut... pxCore bug
    win.setVisibility(true);
    
    win.setAnimationFPS(gFPS);
    
    context.init();
    
    eventLoop.run();
    
    return 0;
}

