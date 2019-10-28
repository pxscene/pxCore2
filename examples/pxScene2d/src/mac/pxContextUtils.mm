#include "pxContextUtils.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#import <Cocoa/Cocoa.h>
#include <map>
#include "rtLog.h"
#include "rtMutex.h"

bool glContextIsCurrent = false;
rtMutex eglContextMutex;
int nextInternalContextId = 1;

extern NSOpenGLContext *openGLContext;

NSOpenGLPixelFormatAttribute attribs[] =
        {
            /*NSOpenGLPFADoubleBuffer,*/
            NSOpenGLPFAAllowOfflineRenderers, // lets OpenGL know this context is offline renderer aware
            NSOpenGLPFAMultisample, 1,
            NSOpenGLPFASampleBuffers, 1,
            NSOpenGLPFASamples, 4,
            NSOpenGLPFAColorSize, 32,
            NSOpenGLPFAOpenGLProfile,NSOpenGLProfileVersionLegacy/*, NSOpenGLProfileVersion3_2Core*/, // Core Profile is the future
            0
        };

NSOpenGLPixelFormatAttribute attribsWithDepth[] =
        {
            /*NSOpenGLPFADoubleBuffer,*/
            NSOpenGLPFADepthSize,32,
            NSOpenGLPFAAllowOfflineRenderers, // lets OpenGL know this context is offline renderer aware
            NSOpenGLPFAMultisample, 1,
            NSOpenGLPFASampleBuffers, 1,
            NSOpenGLPFASamples, 4,
            NSOpenGLPFADepthSize, 32,
            NSOpenGLPFAOpenGLProfile,NSOpenGLProfileVersionLegacy/*, NSOpenGLProfileVersion3_2Core*/, // Core Profile is the future
            0
        };

NSOpenGLPixelFormat *internalPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] retain];
NSOpenGLPixelFormat *internalPixelWithDepthFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribsWithDepth] retain];

std::map<int, NSOpenGLContext *> internalContexts;


pxError createGLContext(int id, bool depthBuffer)
{
    NSOpenGLContext *context = nil;
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    if ( internalContexts.find(id) != internalContexts.end() )
    {
        context = internalContexts.at(id);
    }
    if (context == nil)
    {
        if (depthBuffer)
            context = [[NSOpenGLContext alloc] initWithFormat:internalPixelFormat shareContext:openGLContext];
        else
            context = [[NSOpenGLContext alloc] initWithFormat:internalPixelWithDepthFormat shareContext:openGLContext];

        internalContexts[id] = context;
    }
    return PX_OK;
}

pxError createInternalContext(int &id, bool depthBuffer)
{
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    id = nextInternalContextId++;
  }
  createGLContext(id, depthBuffer);
  return PX_OK;
}

pxError deleteInternalGLContext(int id)
{
  rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
  if ( internalContexts.find(id) != internalContexts.end() )
  {
    internalContexts.erase(id);
  }
  return PX_OK;
}

pxError makeInternalGLContextCurrent(bool current, int id)
{
    if (current)
    {
        NSOpenGLContext *context = nil;
        {
          rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
          if (internalContexts.find(id) != internalContexts.end())
          {
            context = internalContexts.at(id);
          }
        }
        if (context != nil)
        {
            createGLContext(id, false);
            {
              rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
              context = internalContexts[id];
            }
            [context makeCurrentContext];

            #if 0
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            #endif
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        glContextIsCurrent = true;
    }
    else
    {
        glFlush();
        [openGLContext makeCurrentContext];
        glContextIsCurrent = false;
    }
    return PX_OK;
}
