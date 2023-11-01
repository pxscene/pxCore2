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

// rtSoftwareUpdate.cpp

#include "rtSoftwareUpdate.h"

#ifdef WIN32
#include <winsparkle.h>
// todo: is this resource file needed?  if so, uncomment
//#include "../../../pxCore.vsbuild/pxScene2d/resource.h"
#endif

void rtSoftwareUpdate::init() {
#if 0
#ifdef WIN32

  // Initialize WinSparkle as soon as the app itself is initialized, right
  // before entering the event loop:
  win_sparkle_set_appcast_url(
      "https://github.com/pxscene/pxscene/tree/gh-pages/dist/windows/"
      "appcast.xml");
  win_sparkle_init();

#endif
#endif
}

void rtSoftwareUpdate::term() {
#ifdef WIN32
  win_sparkle_cleanup();
#endif
}
