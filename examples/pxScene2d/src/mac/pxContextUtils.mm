#include "pxContextUtils.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#import <Cocoa/Cocoa.h>
#include <map>

bool glContextIsCurrent = false;

extern NSOpenGLContext *openGLContext;

NSOpenGLContext *internalGLContext = nil;
NSOpenGLPixelFormat *internalPixelFormat = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attribs] retain];

std::map<int, NSOpenGLContext *> internalContexts;


pxError createGLContext(int id)
{
    NSOpenGLContext *context = nil;
    if ( internalContexts.find(i) != m.end() )
    {
        context = internalContexts.at(id);
    }
    if (context == nil)
    {
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
        context = [[NSOpenGLContext alloc] initWithFormat:internalPixelFormat shareContext:openGLContext];
        internalContexts[id] = context;
    }
    return PX_OK;
}

pxError makeInternalGLContextCurrent(bool current, int id)
{
    if (current)
    {
        NSOpenGLContext *context = nil;
        if ( internalContexts.find(i) != m.end() )
        {
            context = internalContexts.at(id);
        }
        if (context == nil)
        {
            createGLContext(id);
            context = internalContexts[id];
            [context makeCurrentContext];

            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            [context makeCurrentContext];
        }
        glContextIsCurrent = true;
    }
    else
    {
        [openGLContext makeCurrentContext];
        glContextIsCurrent = false;
    }
    return PX_OK;
}
