 //-----------------------------------------------------------------------------------
 //  pxContext.cpp
 //  Benchmark
 //
 //  Created by Kalokhe, Ashwini on 7/11/18.
 //  Copyright © 2018 Kalokhe, Ashwini. All rights reserved.
 //-----------------------------------------------------------------------------------

// pxContextGL.cpp
#include "rtCore.h"
#include "rtLog.h"
#include "rtThreadTask.h"
#include "rtThreadPool.h"
#include "rtThreadQueue.h"
#include "rtMutex.h"
#include "rtScript.h"
#include "rtSettings.h"

#include "pxContext.h"
#include "pxUtil.h"
#include <algorithm>
#include <ctime>
#include <cstdlib>

#ifdef __APPLE__
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#if defined(PX_PLATFORM_WAYLAND_EGL) || defined(PX_PLATFORM_GENERIC_EGL)
#include <GLES2/gl2.h>
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2ext.h>
#else
#include <GL/glew.h>
#ifdef WIN32
#include <GL/wglew.h>
#endif // WIN32
#ifdef PX_PLATFORM_GLUT
#include <GL/glut.h>
#endif
#include <GL/gl.h>
#endif //PX_PLATFORM_WAYLAND_EGL
#endif

//-----------------------------------------------------------------------------------
// class globals
//-----------------------------------------------------------------------------------

enum pxCurrentGLProgram { PROGRAM_UNKNOWN = 0, PROGRAM_SOLID_SHADER,  PROGRAM_A_TEXTURE_SHADER, PROGRAM_TEXTURE_SHADER,
    PROGRAM_TEXTURE_MASKED_SHADER, PROGRAM_TEXTURE_BORDER_SHADER};


pxCurrentGLProgram currentGLProgram = PROGRAM_UNKNOWN;

#define SAFE_DELETE(p)  if(p) { delete p; p = NULL; };

//-----------------------------------------------------------------------------------
// class shaderBase
//-----------------------------------------------------------------------------------
shaderBase::~shaderBase() {
    glDetachShader(mProgram, mFragShader);
    glDetachShader(mProgram, mVertShader);
    glDeleteShader(mFragShader);
    glDeleteShader(mVertShader);
    glDeleteProgram(mProgram);
}

void shaderBase::init(const char* v, const char* f)
{
    /*glShaderProgDetails details = createShaderProgram(v, f);
    mProgram    = details.program;
    mFragShader = details.fragShader;
    mVertShader = details.vertShader;
    prelink();
    linkShaderProgram(mProgram);
    postlink();*/
}

int shaderBase::getUniformLocation(const char* name)
{
    int l = glGetUniformLocation(mProgram, name);
    if (l == -1)
        rtLogError("Shader does not define uniform %s.\n", name);
    return l;
}

void shaderBase::use()
{
    currentGLProgram = PROGRAM_UNKNOWN;
    glUseProgram(mProgram);
}

//-----------------------------------------------------------------------------------
// class solidShader
//-----------------------------------------------------------------------------------
void solidShader::prelink()
{
    mPosLoc = 0;
    mUVLoc = 1;
    glBindAttribLocation(mProgram, mPosLoc, "pos");
    glBindAttribLocation(mProgram, mUVLoc,  "uv");
}

void solidShader::postlink()
{
    mResolutionLoc = getUniformLocation("u_resolution");
    mMatrixLoc     = getUniformLocation("amymatrix");
    mColorLoc      = getUniformLocation("a_color");
    mAlphaLoc      = getUniformLocation("u_alpha");
}

pxError solidShader::draw(int resW, int resH, float* matrix, float alpha,
             GLenum mode,
             const void* pos,
             int count,
             const float* color)
{
    if (currentGLProgram != PROGRAM_SOLID_SHADER)
    {
        use();
        currentGLProgram = PROGRAM_SOLID_SHADER;
    }
    glUniform2f(mResolutionLoc, static_cast<GLfloat>(resW), static_cast<GLfloat>(resH));
    glUniformMatrix4fv(mMatrixLoc, 1, GL_FALSE, matrix);
    glUniform1f(mAlphaLoc, alpha);
    glUniform4fv(mColorLoc, 1, color);
    
    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, GL_FALSE, 0, pos);
    glEnableVertexAttribArray(mPosLoc);
    glDrawArrays(mode, 0, count);  ;
    glDisableVertexAttribArray(mPosLoc);
    
    return PX_OK;
}

//-----------------------------------------------------------------------------------
// class pxContext
//-----------------------------------------------------------------------------------

glShaderProgDetails pxContext::createShaderProgram(const char* vShaderTxt, const char* fShaderTxt)
{
    struct glShaderProgDetails details = { 0,0,0 };
    GLint stat;
    
    details.fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(details.fragShader, 1, (const char **) &fShaderTxt, NULL);
    glCompileShader(details.fragShader);
    glGetShaderiv(details.fragShader, GL_COMPILE_STATUS, &stat);
    
    if (!stat)
    {
        rtLogError("Error: fragment shader did not compile: %d", glGetError());
        
        GLint maxLength = 0;
        glGetShaderiv(details.fragShader, GL_INFO_LOG_LENGTH, &maxLength);
        
        //The maxLength includes the NULL character
        std::vector<char> errorLog(maxLength);
        glGetShaderInfoLog(details.fragShader, maxLength, &maxLength, &errorLog[0]);
        
        rtLogWarn("%s", &errorLog[0]);
        //Exit with failure.
        glDeleteShader(details.fragShader); //Don't leak the shader.
        
        //TODO get rid of exit
        exit(1);
    }
    
    details.vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(details.vertShader, 1, (const char **) &vShaderTxt, NULL);
    glCompileShader(details.vertShader);
    glGetShaderiv(details.vertShader, GL_COMPILE_STATUS, &stat);
    
    if (!stat)
    {
        rtLogError("vertex shader did not compile: %d", glGetError());
        exit(1);
    }
    
    details.program = glCreateProgram();
    glAttachShader(details.program, details.fragShader);
    glAttachShader(details.program, details.vertShader);
    return details;
}

void pxContext::linkShaderProgram(GLuint program)
{
    GLint stat;
    
    glLinkProgram(program);  /* needed to put attribs into effect */
    glGetProgramiv(program, GL_LINK_STATUS, &stat);
    if (!stat)
    {
        char log[1000];
        GLsizei len;
        glGetProgramInfoLog(program, 1000, &len, log);
        rtLogError("faild to link:%s", log);
        // TODO purge all exit calls
        exit(1);
    }
}

pxContext::~pxContext()
{
    SAFE_DELETE(mgSolidShader);
}

void pxContext::init(void)
{
#if 0
    if (mGContextInit)
        return;
    else
        mGContextInit = true;
#endif
    
    glClearColor(255, 0, 0, 0);
    
    SAFE_DELETE(mgSolidShader);
    
    mgSolidShader = new solidShader();
    
    mgSolidShader->init("vShaderText", "fSolidShaderText");
}

void pxContext::apiName(void)
{
    
}
