//-----------------------------------------------------------------------------------
//  pxBenchmark.h
//  Benchmark
//
//  Created by Kalokhe, Ashwini on 7/11/18.
//  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
//-----------------------------------------------------------------------------------

// pxContext.h

#ifndef PX_CONTEXT_H
#define PX_CONTEXT_H

#include "rtCore.h"
#include "rtRef.h"

#include "pxCore.h"
#include "pxOffscreen.h"
#include "pxMatrix4T.h"
#include "pxConstants.h"
#include "pxTexture.h"
#include "pxContextFramebuffer.h"

#ifdef ENABLE_DFB
#include "pxContextDescDFB.h"
#else
#include "pxContextDescGL.h"
#endif //ENABLE_DFB

//-----------------------------------------------------------------------------------
// class shaderProgram
// This is shader base class
//-----------------------------------------------------------------------------------
class shaderBase
{
protected:
    GLuint  mProgram;
    GLuint  mFragShader;
    GLuint  mVertShader;
    
public:
    shaderBase ()
    : mProgram (0)
    , mFragShader (0)
    , mVertShader (0)
    {
    }
    
    virtual ~shaderBase();
    
    virtual void init(const char* v, const char* f);
    
    int getUniformLocation(const char* name);
    
    void use();
    
protected:
    // Override to do uniform lookups
    virtual void prelink() {}
    virtual void postlink() {}
    
}; // CLASS - shader


//-----------------------------------------------------------------------------------
// class solidShader
// This defines solidShader for benchmark application
//-----------------------------------------------------------------------------------
class solidShader : public shaderBase
{
    GLint mResolutionLoc;
    GLint mMatrixLoc;
    
    GLint mPosLoc;
    GLint mUVLoc;
    
    GLint mColorLoc;
    GLint mAlphaLoc;
    
protected:
    
    virtual void prelink();
    
    virtual void postlink();
    
public:
    solidShader ()
        : mResolutionLoc (0)
        , mMatrixLoc (0)
        , mPosLoc (0)
        , mUVLoc (0)
        , mColorLoc (0)
        , mAlphaLoc (0)
        {
        }
    
    pxError draw(int resW, int resH, float* matrix, float alpha,
                 GLenum mode,
                 const void* pos,
                 int count,
                 const float* color);
    
}; //CLASS - solidShaderProgram

//-----------------------------------------------------------------------------------
// struct glShaderProgDetails
// glShaderProg Details
//-----------------------------------------------------------------------------------
struct glShaderProgDetails
{
    GLuint program;
    GLuint fragShader;
    GLuint vertShader;
};

//-----------------------------------------------------------------------------------
// class pxContext
// This defines context view for benchmark application
//-----------------------------------------------------------------------------------
class pxContext {
    
    bool        mGContextInit;
    solidShader *mgSolidShader;
    
public:
    pxContext()
        : mGContextInit (false)
        , mgSolidShader (NULL)
        {}
    
    ~pxContext();

    void apiName();
    
    void init();
    
    static glShaderProgDetails createShaderProgram(const char* vShaderTxt, const char* fShaderTxt);
    static void linkShaderProgram(GLuint program);
};


#endif
