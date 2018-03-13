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

// pxEventLoopNative.cpp

#include "../pxEventLoop.h"
#include "../pxOffscreen.h"
#include "pxWindowNative.h"

void pxEventLoop::run()
{
  // For now we delegate off to the pxWindowNative class
  pxWindowNative::runEventLoop();
}

void pxEventLoop::runOnce()
{
  pxWindowNative::runEventLoopOnce();
}

void pxEventLoop::exit()
{
  // For now we delegate off to the pxWindowNative class
  pxWindowNative::exitEventLoop();
}


///////////////////////////////////////////
// Entry Point 

int main(int argc, char* argv[])
{
  return pxMain(argc, argv);
}
