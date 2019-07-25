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


#include "pxContext.h"
#include <rtScript.h>
#include <rtFileDownloader.h>
#include <signal.h>
#include <unistd.h>

#include "test_includes.h" // Needs to be included last

#ifdef ENABLE_CODE_COVERAGE
extern "C" void __gcov_flush();
#endif

pxContext context;
extern rtScript script;

void handleSegv(int)
{
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal SEGV received. sleeping to collect data");
  sleep(1800);
}

void handleAbrt(int)
{
  FILE* fp = fopen("/tmp/pxscenecrash","w");
  fclose(fp);
  rtLogInfo("Signal ABRT received. sleeping to collect data");
  sleep(1800);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  char const* handle_signals = getenv("HANDLE_SIGNALS");
  if (handle_signals && (strcmp(handle_signals,"1") == 0))
  {
    signal(SIGSEGV, handleSegv);
    signal(SIGABRT, handleAbrt);
  }
  script.init();
  int retTests = RUN_ALL_TESTS();
  rtLogInfo("Tests executed with return code [%d]", retTests);
  #ifdef ENABLE_CODE_COVERAGE
  __gcov_flush();
  #endif
  rtFileDownloader::deleteInstance();
}
