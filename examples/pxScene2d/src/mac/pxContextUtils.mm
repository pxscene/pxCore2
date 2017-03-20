#include "pxContextUtils.h"

#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#import <Cocoa/Cocoa.h>

bool glContextIsCurrent = false;

extern NSOpenGLContext *openGLContext;

NSOpenGLContext *internalGLContext = nil;
NSOpenGLPixelFormat *internalPixelFormat = nil;


pxError createGLContext()
{
    if (internalGLContext == nil)
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
        
        NSOpenGLPixelFormat *pf = [[NSOpenGLPixelFormat alloc] initWithAttributes:attribs];
        internalPixelFormat   = [pf retain];
        internalGLContext = [[NSOpenGLContext alloc] initWithFormat:internalPixelFormat shareContext:openGLContext];
    }
    return PX_OK;
}

pxError makeInternalGLContextCurrent(bool current)
{
    if (current)
    {
        if (internalGLContext == nil)
        {
            createGLContext();
            [internalGLContext makeCurrentContext];
            
            glEnable(GL_BLEND);
            glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        else
        {
            //TODO
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
