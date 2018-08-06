//-----------------------------------------------------------------------------------
//  pxBenchmark.h
//  Benchmark
//
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------

#ifndef pxBenchmark_h
#define pxBenchmark_h

#include <stdio.h>
#include <pxIView.h>

#include <stdlib.h>
#include "rtPathUtils.h"
#include "pxCore.h"
#include "pxTimer.h"
#include "pxWindow.h"

#define ANIMATION_ROTATE_XYZ
#include "rtUrlUtils.h"
#include "rtScript.h"
#include <stdlib.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <stdlib.h>
#include <fstream>

#include <stdint.h>    // for PRId64
#include <inttypes.h>  // for PRId6

#include <celero/Celero.h>
using namespace celero;

//-----------------------------------------------------------------------------------
//  class pxBenchmarkExperimentValue
//  Notes: This is class is degined to test the performance of graphics API
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------
class pxBenchmarkExperimentValue : public TestFixture::ExperimentValue
{
public:
    uint64_t         mTotalTime;
    //std::string      mName;
    
    pxBenchmarkExperimentValue()
    : mTotalTime (0)
    , ExperimentValue()
    {
        
    }
    pxBenchmarkExperimentValue(int64_t v)
    : mTotalTime (0)
    , ExperimentValue (v)
    {
    }
    
    pxBenchmarkExperimentValue(int64_t v, uint64_t totTime)
    : ExperimentValue (v)
    , mTotalTime (totTime)
    {
    }
    
    pxBenchmarkExperimentValue(int64_t v, int64_t i, uint64_t totTime)
    : ExperimentValue (v, i)
    , mTotalTime (totTime)
    {
    };
    
    ~pxBenchmarkExperimentValue()
    {
    }
    
};

//-----------------------------------------------------------------------------------
//  class pxApiFixture
//  Notes: This is class is degined to test the performance of API based on times recoded in pxBenchmark
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------
class pxApiFixture : public celero::TestFixture
{
    
    pxBenchmarkExperimentValue                mExperimentValue;
    uint64_t                                  mIterationCounter;
    
    void TestDrawRect ();
    void TestDrawDiagLine ();
    void TestDrawDiagRect ();
    void TestDrawImage ();
    void TestDrawImage9 ();
    void TestDrawImage9Border ();
    void TestDrawAll ();
    void TestDrawImageMasked ();
    void TestDrawTextureQuads ();
    void TestDrawOffscreen ();
    
public:
    enum type
    {
        xDrawRect,
        xDrawDiagLine,
        xDrawDiagRect,
        xDrawImage,
        xDrawImage9,
        xDrawImageBorder9,
        xDrawImageMasked,
        xDrawTextureQuads,
        //xDrawOffscreen,
        xDrawAll
    };
    
    float               mCurrentX;
    float               mCurrentY;
    float               mUnitWidth;
    float               mUnitHeight;
    
    pxApiFixture()
    : mIterationCounter(0)
    , mCurrentX (0)
    , mCurrentY (0)
    , mUnitWidth (50) // TODO 20->50x50 image size 512x512
    , mUnitHeight (50)
    {
    }
    
    ~pxApiFixture()
    {
    }
    
    virtual void onExperimentStart(const celero::TestFixture::ExperimentValue&) override;
    
    virtual void onExperimentEnd() override;
    
    virtual std::vector<celero::TestFixture::ExperimentValue> getExperimentValues() const override;
    
    /// Before each run, build a vector of random integers.
    virtual void setUp(const celero::TestFixture::ExperimentValue& x) override;
    
    virtual void tearDown() override;
    
    virtual void UserBenchmark() override;
    
    pxBenchmarkExperimentValue& popExperimentValue();
    
    static pxApiFixture& Instance();
    
    virtual uint64_t run(const uint64_t threads, const uint64_t iterations, const celero::TestFixture::ExperimentValue& experimentValue) override;
    
    uint64_t getIterationCounter() const;
    
    void setIterationCounter(uint64_t val);
};

//-----------------------------------------------------------------------------------
//  class pxBenchmarkFactory
//  Notes: This is class is degined to test the performance of graphics API
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------
class pxBenchmarkFactory : public celero::Factory
{
public:
    pxBenchmarkFactory()
    {
    }
    
    ~pxBenchmarkFactory()
    {
    }
    
    virtual std::shared_ptr<TestFixture> Create() override;
    
    static pxBenchmarkFactory& Instance();
};

//-----------------------------------------------------------------------------------
//  class BenchmarkWindow
//  Notes: This is class is degined to test the performance of graphics API
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------
class benchmarkWindow : public pxWindow, public pxIViewContainer, public Benchmark
{
    int32_t             mWidth;
    int32_t             mHeight;
    bool                mClosed;
    std::string         mApiName;
    uint64_t            mTimer;
    std::string         mGroupName;
    std::string         mBenchmarkName;
    uint64_t            mSamples;
    uint64_t            mIterations;
    uint64_t            mThreads;
    
    int32_t             mWindowWidth;
    int32_t             mWindowHeight;
    std::string         mArchiveCSV;
    std::string         mOutputTableCSV;
    
    std::shared_ptr<pxApiFixture>                   mApiFixture;
    std::shared_ptr<celero::Benchmark>              mBaselineBm;
    std::vector<std::shared_ptr<celero::Benchmark>> mBms;
    std::shared_ptr<pxBenchmarkFactory>             mExperimentFactory;
    
public:
    benchmarkWindow ()
    : mWidth (-1)
    , mHeight (-1)
    , mClosed (false)
    , mTimer (0)
    , mGroupName ("pxApiTest")
    , mBenchmarkName ("pxApiTest")
    , mSamples (1)
    , mIterations (1056)
    , mThreads (1)
    , mArchiveCSV ("pxBenchmark_archive.csv")
    , mOutputTableCSV ("pxBenchmark_outputTable.csv")
    {
        
    }
    
    virtual ~benchmarkWindow()
    {
    }
    
    void init(const int32_t& x, const int32_t& y, const int32_t& w, const int32_t& h, const char* url = NULL);
    
    void* getInterface(const char* /*name*/);
    
    rtError setView(pxIView* v);
    
    virtual void invalidateRect(pxRect* r);
    
    void close();
    
    std::shared_ptr<pxApiFixture> getPxApiFixture () const;
    
    int32_t GetWidth () const { return mWidth; }
    
    int32_t GetHeight () const { return mHeight; }
    
    uint64_t GetCurrentTimeElapsed () const;
    //float GetCurrentX () { return mCurrentX; }
    //float GetCurrentY () { return mCurrentY; }
    
    void reset();
protected:
    
    virtual void onSize(/*const */int32_t/*&*/ w, /*const */int32_t/*&*/ h);
    
    virtual void onMouseDown(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y, /*const */uint32_t/*&*/ flags);
    
    virtual void onCloseRequest();
    
    virtual void onMouseUp(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y, /*const */uint32_t/*&*/ flags);
    
    virtual void onMouseLeave();
    
    virtual void onMouseMove(/*const */int32_t/*&*/ x, /*const */int32_t/*&*/ y);
    
    virtual void onFocus();
    
    virtual void onBlur();
    
    virtual void onKeyDown(/*const */uint32_t/*&*/ keycode, /*const */uint32_t/*&*/ flags);
    
    virtual void onKeyUp(/*const */uint32_t/*&*/ keycode, /*const */uint32_t/*&*/ flags);
    
    virtual void onChar(/*const */uint32_t/*&*/ c);
    
    virtual void onDraw(pxSurfaceNative/*&*/);
    
    virtual void onAnimationTimer();
    
    // Benchmark
    void StartTimer();
    
    void StopTimer();
    
    void RegisterTest (const std::string& groupName, const std::string& benchmarkName, const uint64_t samples,
                       const uint64_t iterations, const uint64_t threads);
};

#endif /* pxBenchmark_h */


