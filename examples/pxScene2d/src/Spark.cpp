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

// Spark.cpp

#include "rtSettings.h"
#include "rtUrlUtils.h"
#include "rtPathUtils.h"
#include "rtPhoneHome.h"
#include "rtSoftwareUpdate.h"

#include "rtScript.h"

#include "pxCore.h"
#include "pxEventLoop.h"
//#include "pxViewWindow.h"
#include "pxWindowUtil.h"
#include "pxTimer.h"
#include "pxContext.h"

#include "sparkWindow.h"
#include "pxScene2d.h"
#include "pxUtil.h"

#include <inttypes.h>  // for PRId64
#include <stdint.h>    // for PRId64
//#include <stdlib.h>

#ifndef PX_SCENE_VERSION
#define PX_SCENE_VERSION dev
#endif

rtScript script;
pxEventLoop eventLoop;
pxContext context; // JRJR try to make this go away and be a parameter to onDraw

double gCollectionTime = 30.0;
bool gDumpMemUsage = false;

#ifdef ENABLE_CODE_COVERAGE
extern "C" void __gcov_flush();
#endif

sparkWindow win;
#define xstr(s) str(s)
#define str(s) #s

#if 0
void handleTerm(int) {
  rtLogInfo("Signal TERM received. closing the window");
  win.close();
#ifndef PX_PLATFORM_MAC
#ifdef WIN32
  win_sparkle_cleanup();
#endif
  base64_cleanup();
  signal(SIGTERM, SIG_DFL);
  raise(SIGTERM);
#endif
}
#endif

int pxMain(int argc, char *argv[]) {
  rtPhoneHome crash;
  crash.init();

#if 1
// JRJR TODO rather than this hackery add a proper command arg parser
  const char *url = "";
  for (int i = 1; i < argc; i++) {
    const char *arg = argv[i];
    if (arg && arg[0] != '-' &&
        arg[0] != 'Y')  // Xcode Debugger adds args of
                        // "-NSDocumentRevisionsDebugMode YES"
    {
      url = arg;
      break;
    }
  }
#endif

  // load settings
  rtSettings::instance()->loadFromFile();
  // overwrite settings from the command line
  rtSettings::instance()->loadFromArgs(argc, argv);

  script.init();
  gDumpMemUsage = rtEnv<bool>("PX_DUMP_MEMUSAGE",false);
  gCollectionTime = rtEnv<double>("SPARK_COLLECTION_TIME", 30);
  int32_t windowWidth = rtEnv<int32_t>("SPARK_WINDOW_WIDTH", 1280);
  int32_t windowHeight = rtEnv<int32_t>("SPARK_WINDOW_HEIGHT", 720);
  windowWidth = rtSettings::get("screenWidth", windowWidth);
  windowHeight = rtSettings::get("screenHeight", windowHeight);

  extern bool gDirtyRectsEnabled;
  gDirtyRectsEnabled = rtSettings::get<bool>("enableDirtyRects");
  rtLogInfo("dirty rectangles enabled: %s", gDirtyRectsEnabled ? "true" : "false");

  pxScene2d::enableOptimizedUpdate(rtSettings::get<bool>("enabledOptimiedUpdate"));

  win.init(10, 10, windowWidth, windowHeight, url);

  char buffer[256];
  sprintf(buffer, "Spark: %s", xstr(PX_SCENE_VERSION));

  win.setTitle(buffer);
  // JRJR TODO Why aren't these necessary for glut... pxCore bug
  win.setVisibility(true);

  uint32_t animationFPS = rtSettings::get<uint32_t>("animationFPS", 60);
  rtLogInfo("Animation FPS: %lu", (unsigned long)animationFPS);
  win.setAnimationFPS(animationFPS);

#if 0 // JRJR multiple windows broken on windows... likely opengl context is not correct.
  sparkWindow win2;
  //win2.init(50, 50, 1280, 720);
  win2.init(10, 10, windowWidth, windowHeight, url);
#endif

  // JRJR TODO this needs happen after GL initialization which right now only
  // happens after a pxWindow has been created. Likely will move this to
  // pxWindow...  as an option... a "context" type would like to decouple it
  // from pxScene2d specifically
  context.init();

  rtSoftwareUpdate update;
  update.init();

  eventLoop.run();

  base64_cleanup();

  return 0;
}
