/*

 pxCore Copyright 2005-2021 John Robinson
 Spark Copyright 2016-2021 John Robinson

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

// sparkWindow.h

#include "pxViewWindow.h"
#include "pxScene2d.h"

#include <string>

#define MAX_URL_SIZE 8000

extern pxEventLoop eventLoop;
extern pxContext context;
extern rtScript script;
extern bool gDumpMemUsage;
extern bool gApplicationIsClosing;
extern int pxObjectCount;
extern double gCollectionTime;

class sparkWindow: public pxViewWindow {
 public:
  sparkWindow(): mClosed(false) {}
  virtual ~sparkWindow() { mView = NULL; }

  void init(int x, int y, int w, int h, const char *url = NULL) {
    pxWindow::init(x, y, w, h);
    setUrl(url);
  }

  void *getInterface(const char * /*name*/) { return NULL; }

  rtError setUrl(const char *url) {
    // escape url begin
    std::string escapedUrl;
    std::string origUrl = url;
    for (std::string::iterator it = origUrl.begin(); it != origUrl.end(); ++it) {
      char currChar = *it;
      if ((currChar == '"') || (currChar == '\\')) {
        escapedUrl.append(1, '\\');
      }
      escapedUrl.append(1, currChar);
    }
    if (escapedUrl.length() > MAX_URL_SIZE) {
      rtLogWarn(
          "url size greater than MAX_URL_SIZE, reset to empty url to get "
          "default behavior");
      escapedUrl = "";
    }
    // escape url end
    char buffer[MAX_URL_SIZE + 50];
    memset(buffer, 0, sizeof(buffer));

    if (std::string::npos != escapedUrl.find("http")) {
      snprintf(buffer, sizeof(buffer), "shell.js?url=%s",
               rtUrlEncodeParameters(escapedUrl.c_str()).cString());
    } else {
      snprintf(buffer, sizeof(buffer), "shell.js?url=%s", escapedUrl.c_str());
    }

    setView(new pxScriptView(buffer, "javascript/node/v8"));

    return RT_OK;
  }

 protected:

  virtual void onCloseRequest() {
    if (mClosed)
      return;

    mClosed = true;

    if (gDumpMemUsage)
      gApplicationIsClosing = true;

    rtLogInfo(__FUNCTION__);
    fflush(stdout);
    if (mView) {
      rtLogInfo("onClose request started");
      fflush(stdout);
      mView->onCloseRequest();
      rtLogInfo("onClose request completed");
      fflush(stdout);
    }

    // pxScene.cpp:104:12: warning: deleting object of abstract class type
    // ‘pxIView’ which has non-virtual destructor will cause undefined behaviour
    // [-Wdelete-non-virtual-dtor]

    mView = NULL;

    context.term();

    script.pump();

    rtLogInfo("about to call garbage collect during close");
    fflush(stdout);
    script.collectGarbage();
    rtLogInfo("called garbage collect during close");
    fflush(stdout);

    if (gDumpMemUsage) {
      script.pump();
      script.collectGarbage();

      rtLogInfo("pxobjectcount is [%d]", pxObjectCount);

      rtLogInfo("texture memory usage is [%" PRId64 "]",
                context.currentTextureMemoryUsageInBytes());

      fflush(stdout);
    }
#ifdef ENABLE_CODE_COVERAGE
    __gcov_flush();
#endif

    close();
    eventLoop.exit();
  }

  virtual void onAnimationTimer() {
    static double lastCollectionTime = pxSeconds();

    if (mView && !mClosed)
      mView->onUpdate(pxSeconds());

    script.pump();

    double currentTime = pxSeconds();
    if ((gCollectionTime > 0) &&
        (currentTime - lastCollectionTime > gCollectionTime)) {
      script.collectGarbage();
      lastCollectionTime = currentTime;
    }
  }

 protected:
  bool mClosed;
};
