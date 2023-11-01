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

// pxSharedContextNative.h
#ifndef PX_SHAREDCONTEXTNATIVE_H
#define PX_SHAREDCONTEXTNATIVE_H

#include "windows.h"

class pxSharedContextNative {
public:

  pxSharedContextNative();
  virtual ~pxSharedContextNative();

  void makeCurrent(bool f);

protected:
  HDC mHDC;
  HGLRC mGLContext;
  HGLRC mOriginalGLContext;
};

#endif
