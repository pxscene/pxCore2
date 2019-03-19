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
#include "win/WindowsGLContext.hpp"
#include <map>
#include "rtLog.h"
#include "rtMutex.h"

bool glContextIsCurrent = false;
rtMutex eglContextMutex;
int nextInternalContextId = 1;


std::map<int, HGLRC> internalContexts;


pxError createGLContext(int id)
{
  HGLRC hrc = nullptr;
  rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
  if (internalContexts.find(id) != internalContexts.end())
  {
    hrc = internalContexts.at(id);
  }
  if (!hrc)
  {
    hrc = WindowsGLContext::instance()->createContext();
    internalContexts[id] = hrc;
    rtLogInfo("createGLContext id = %d", id);
  }
  return PX_OK;
}

pxError createInternalContext(int &id)
{
  {
    rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
    id = nextInternalContextId++;
  }
  createGLContext(id);
  return PX_OK;
}

pxError deleteInternalGLContext(int id)
{
  rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
  if (internalContexts.find(id) != internalContexts.end())
  {
    HGLRC hrc = internalContexts[id];
    internalContexts.erase(id);
    WindowsGLContext::instance()->deleteContext(hrc);
  }
  return PX_OK;
}

pxError makeInternalGLContextCurrent(bool current, int id)
{
  if (current)
  {
    HGLRC context = nullptr;
    {
      rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
      if (internalContexts.find(id) != internalContexts.end())
      {
        context = internalContexts.at(id);
      }
    }
    if (!context)
    {
      createGLContext(id);
      {
        rtMutexLockGuard eglContextMutexGuard(eglContextMutex);
        context = internalContexts[id];
      }
      rtLogInfo("makeInternalGLContextCurrent id = %d", id);
    }
    WindowsGLContext::instance()->makeCurrent(context);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glContextIsCurrent = true;
  }
  else
  {
    glFlush();
    WindowsGLContext::instance()->makeCurrent(WindowsGLContext::instance()->rootContext);
    glContextIsCurrent = false;
    rtLogInfo("makeInternalGLContextCurrent -> switch to root gl context");
  }
  return PX_OK;
}
