/*

pxCore Copyright 2005-2021 John Robinson

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

// pxSharedContextNative.cpp

#include "rtLog.h"
#include "rtMutex.h"
#include "pxSharedContext.h"

#include "windows.h"

#include "pxSharedContextNative.h"

void pxSharedContext::makeCurrent(bool f) {
  pxSharedContextNative::makeCurrent(f);
}

pxSharedContextNative::pxSharedContextNative() {
    mHDC = wglGetCurrentDC();
    mOriginalGLContext = wglGetCurrentContext();
    mGLContext = wglCreateContext(mHDC);
    wglShareLists(mOriginalGLContext, mGLContext);
}

pxSharedContextNative::~pxSharedContextNative() {
  if (mGLContext)
    wglDeleteContext(mGLContext);
}

void pxSharedContextNative::makeCurrent(bool f) {
  wglMakeCurrent(mHDC, f?mGLContext:mOriginalGLContext);
}
