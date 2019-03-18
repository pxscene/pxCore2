/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#include "pxContextUtils.h"
#include "rtLog.h"
#include "rtMutex.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <map>

EGLContext defaultEglContext = 0;
EGLDisplay defaultEglDisplay = 0;
EGLSurface defaultEglSurface = 0;

rtMutex eglContextMutex;


int nextInternalContextId = 1;

struct eglContextDetails
{
  eglContextDetails() : eglDisplay(0), eglDrawSurface(0), eglReadSurface(0), eglContext(0) {}
  eglContextDetails(EGLDisplay display, EGLSurface drawSurface, EGLSurface readSurface, EGLContext context) :
               eglDisplay(display), eglDrawSurface(drawSurface), eglReadSurface(readSurface), eglContext(context) {}
  EGLDisplay eglDisplay;
  EGLSurface eglDrawSurface;
  EGLSurface eglReadSurface;
  EGLContext eglContext;
};

struct eglContextInfo
{
  eglContextInfo(): contextDetails(), previousContextDetails(), onDisplayStack(false) {}

  eglContextDetails contextDetails;
  eglContextDetails previousContextDetails;
  bool onDisplayStack;
};


std::map<int, eglContextInfo> internalContexts;

int pxCreateEglContext(int id)
{
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    if (internalContexts.find(id) != internalContexts.end())
    {
      rtLogDebug("context with this id already exists");
      return PX_FAIL;
    }
  }
  rtLogDebug("creating new context\n");
  EGLDisplay egl_display      = 0;
  EGLSurface egl_surface      = 0;
  EGLContext egl_context      = 0;
  EGLConfig *egl_config;
  EGLint     major_version;
  EGLint     minor_version;
  int        config_select    = 0;
  int        configs;

  egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (egl_display == EGL_NO_DISPLAY)
  {
    rtLogError("eglGetDisplay() failed, did you register any exclusive displays\n");
    return PX_FAIL;
  }

  if (!eglInitialize(egl_display, &major_version, &minor_version))
  {
     rtLogError("eglInitialize() failed\n");
     return PX_FAIL;
  }

  if (!eglGetConfigs(egl_display, NULL, 0, &configs))
  {
     rtLogError("eglGetConfigs() failed\n");
     return PX_FAIL;
  }

  egl_config = (EGLConfig *)alloca(configs * sizeof(EGLConfig));

  {
    const int   NUM_ATTRIBS = 21;
    EGLint      *attr = (EGLint *)malloc(NUM_ATTRIBS * sizeof(EGLint));
    int         i = 0;

    attr[i++] = EGL_RED_SIZE;        attr[i++] = 8;
    attr[i++] = EGL_GREEN_SIZE;      attr[i++] = 8;
    attr[i++] = EGL_BLUE_SIZE;       attr[i++] = 8;
    attr[i++] = EGL_ALPHA_SIZE;      attr[i++] = 8;
    attr[i++] = EGL_DEPTH_SIZE;      attr[i++] = 24;
    attr[i++] = EGL_STENCIL_SIZE;    attr[i++] = 0;
    attr[i++] = EGL_SURFACE_TYPE;    attr[i++] = EGL_PBUFFER_BIT;
    attr[i++] = EGL_RENDERABLE_TYPE; attr[i++] = EGL_OPENGL_ES2_BIT;

    attr[i++] = EGL_NONE;

    if (!eglChooseConfig(egl_display, attr, egl_config, configs, &configs) || (configs == 0))
    {
      rtLogError("eglChooseConfig() failed");
      return PX_FAIL;
    }

    free(attr);
  }

  EGLint attribList[] =
  {
      EGL_WIDTH, 2,
      EGL_HEIGHT, 2,
      EGL_LARGEST_PBUFFER, EGL_TRUE,
      EGL_NONE
  };

  for (config_select = 0; config_select < configs; config_select++)
  {
    EGLint red_size, green_size, blue_size, alpha_size, depth_size;

    eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_RED_SIZE,   &red_size);
    eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_GREEN_SIZE, &green_size);
    eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_BLUE_SIZE,  &blue_size);
    eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_ALPHA_SIZE, &alpha_size);
    eglGetConfigAttrib(egl_display, egl_config[config_select], EGL_DEPTH_SIZE, &depth_size);

    if ((red_size == 8) && (green_size == 8) && (blue_size == 8) && (alpha_size == 8))
    {
      rtLogDebug("Selected config: R=%d G=%d B=%d A=%d Depth=%d\n", red_size, green_size, blue_size, alpha_size, depth_size);
      break;
    }
  }

  if (config_select == configs)
  {
    rtLogError("No suitable configs found\n");
    return PX_FAIL;
  }

  egl_surface = eglCreatePbufferSurface(egl_display, egl_config[config_select], attribList);
  if (egl_surface == EGL_NO_SURFACE)
  {
    eglGetError(); /* Clear error */
    egl_surface = eglCreateWindowSurface(egl_display, egl_config[config_select], (EGLNativeWindowType)NULL, NULL);
  }

  if (egl_surface == EGL_NO_SURFACE)
  {
    rtLogError("eglCreateWindowSurface() failed\n");
    return PX_FAIL;
  }

  {
    EGLint     ctx_attrib_list[3] =
    {
      EGL_CONTEXT_CLIENT_VERSION, 2, /* For ES2 */
      EGL_NONE
    };

    egl_context = eglCreateContext(egl_display, egl_config[config_select], defaultEglContext /*EGL_NO_CONTEXT*/, ctx_attrib_list);
    if (egl_context == EGL_NO_CONTEXT)
    {
      rtLogError("eglCreateContext() failed");
      return PX_FAIL;
    }
  }

  eglContextInfo contextInfo;
  contextInfo.contextDetails.eglDisplay = egl_display;
  contextInfo.contextDetails.eglDrawSurface = egl_surface;
  contextInfo.contextDetails.eglReadSurface = egl_surface;
  contextInfo.contextDetails.eglContext = egl_context;
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    internalContexts[id] = contextInfo;
  }
  rtLogDebug("display: %p surface: %p context: %p created\n", egl_display, egl_surface, egl_context);

  return PX_OK;
}
  
void pxMakeEglCurrent(int id)
{
  eglContextInfo contextInfo;
  {
    eglContextMutex.lock();
    if (internalContexts.find(id) != internalContexts.end())
    {
      contextInfo = internalContexts.at(id);
    }
    else
    {
      eglContextMutex.unlock();
      pxCreateEglContext(id);
      eglContextMutex.lock();
      contextInfo = internalContexts[id];
    }
    eglContextMutex.unlock();
  }

  eglContextDetails previousContextDetails;
  previousContextDetails.eglDisplay = eglGetCurrentDisplay();
  previousContextDetails.eglDrawSurface = eglGetCurrentSurface(EGL_DRAW);
  previousContextDetails.eglReadSurface = eglGetCurrentSurface(EGL_READ);
  previousContextDetails.eglContext = eglGetCurrentContext();
  contextInfo.previousContextDetails = previousContextDetails;
  contextInfo.onDisplayStack = true;
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    internalContexts[id] = contextInfo;
  }
  bool success = eglMakeCurrent(contextInfo.contextDetails.eglDisplay, contextInfo.contextDetails.eglDrawSurface,
                                contextInfo.contextDetails.eglReadSurface, contextInfo.contextDetails.eglContext);

  if (!success)
  {
    int eglError = eglGetError();
    rtLogWarn("make current error: %d\n", eglError);
  }
}

void pxDoneEglCurrent(int id)
{
  rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
  if ( internalContexts.find(id) != internalContexts.end() )
  {
    eglContextInfo contextInfo;
    eglContextDetails previousContextDetails;
    contextInfo = internalContexts.at(id);
    if (contextInfo.onDisplayStack)
    {
      previousContextDetails = contextInfo.previousContextDetails;
      eglMakeCurrent(previousContextDetails.eglDisplay, previousContextDetails.eglDrawSurface,
                     previousContextDetails.eglReadSurface, previousContextDetails.eglContext);
      contextInfo.previousContextDetails = eglContextDetails(0,0,0,0);
      contextInfo.onDisplayStack = false;
      internalContexts[id] = contextInfo;
    }
  }
  else
  {
    eglMakeCurrent(defaultEglDisplay, defaultEglSurface, defaultEglSurface, defaultEglContext);
  }
}

void pxDeleteEglContext(int id)
{
  eglContextInfo contextInfo;
  rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
  if ( internalContexts.find(id) != internalContexts.end() )
  {
    contextInfo = internalContexts[id];
    internalContexts.erase(id);
    eglDestroySurface(contextInfo.contextDetails.eglDisplay, contextInfo.contextDetails.eglDrawSurface);
    eglDestroyContext(contextInfo.contextDetails.eglDisplay, contextInfo.contextDetails.eglContext);
  }
}

pxError createInternalContext(int &id)
{
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    id = nextInternalContextId++;
  }
  pxCreateEglContext(id);
  return PX_OK;
}


pxError deleteInternalGLContext(int id)
{
  pxDeleteEglContext(id);
  return PX_OK;
}

pxError makeInternalGLContextCurrent(bool current, int id)
{
  if (current)
  {
    pxMakeEglCurrent(id);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  else
  {
    glFlush();
    pxDoneEglCurrent(id);
  }
  return PX_OK;
}
