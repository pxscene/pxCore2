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

// rtPhoneHome.cpp


#include "rtPhoneHome.h"

#include "rtLog.h"
#include "rtPathUtils.h"
#include "pxTimer.h"

#include <stdlib.h>
#include <signal.h>
#include <fstream>

#ifdef HAS_LINUX_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#elif HAS_WINDOWS_BREAKPAD
#include "client/linux/handler/exception_handler.h"
#if 1
#include <client/windows/handler/exception_handler.h>
#include <wchar.h>
#include <windows.h>
#endif

#endif

#ifdef HAS_LINUX_BREAKPAD
static bool dumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                         void *context, bool succeeded) {
  UNUSED_PARAM(descriptor);
  UNUSED_PARAM(context);
  return succeeded;
}
#elif HAS_WINDOWS_BREAKPAD
bool dumpCallback(const wchar_t *dump_path, const wchar_t *minidump_id,
                  void *context, EXCEPTION_POINTERS *exinfo,
                  MDRawAssertionInfo *assertion, bool succeeded) {
  return succeeded;
}
#endif

static void handleSegv(int) {
  signal(SIGSEGV, SIG_DFL);
  FILE *fp = fopen("/tmp/pxscenecrash", "w");
  fclose(fp);
  rtLogInfo("Signal SEGV received. sleeping to collect data");
  pxSleepMS(30000);  // TODO review length and necessity
}

static void handleAbrt(int) {
  FILE *fp = fopen("/tmp/pxscenecrash", "w");
  fclose(fp);
  rtLogInfo("Signal ABRT received. sleeping to collect data");
  pxSleepMS(30000);  // TODO review length and necessity
}

void rtPhoneHome::init() {
  //signal(SIGTERM, handleTerm);
  if (rtEnv<bool>("HANDLE_SIGNALS",false)) {

    #ifdef HAS_LINUX_BREAKPAD
      google_breakpad::MinidumpDescriptor descriptor("/tmp");
      google_breakpad::ExceptionHandler eh(descriptor, NULL, dumpCallback, NULL,
                                          true, -1);
    #elif HAS_WINDOWS_BREAKPAD
      // register exception handler for breakpad
      google_breakpad::ExceptionHandler *handler = NULL;
      handler = new google_breakpad::ExceptionHandler(
          L"C:\\dumps\\", NULL, dumpCallback, NULL,
          google_breakpad::ExceptionHandler::HANDLER_ALL, MiniDumpNormal, L"",
          NULL);
    #endif

    signal(SIGSEGV, handleSegv);
    signal(SIGABRT, handleAbrt);
    mInitialized = true;
  }
}

void rtPhoneHome::term() {
  if (mInitialized) {

  }
}
