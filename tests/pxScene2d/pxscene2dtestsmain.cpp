
#include "pxContext.h"
#include <rtNode.h>
#include <signal.h>
#include <unistd.h>

#include "test_includes.h" // Needs to be included last

#ifdef ENABLE_CODE_COVERAGE
extern "C" void __gcov_flush();
#endif

pxContext context;
int gargc;
char** gargv;
#ifdef ENABLE_DEBUG_MODE
extern int g_argc;
extern char** g_argv;
char *nodeInput = NULL;
extern rtNode script;
#endif

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
  gargc = argc;
  gargv = argv;
#ifdef ENABLE_DEBUG_MODE
  int urlIndex  = -1;
  bool isDebugging = false;
  g_argv = (char**)malloc((argc+2) * sizeof(char*));
  int size  = 0;
  for (int i=1;i<argc;i++)
  {
    if (strstr(argv[i],"--"))
    {
      size += strlen(argv[i])+1;
      if (strstr(argv[i],"--debug"))
      {
        isDebugging = true;
      }
    }
    else
    {
      if (strstr(argv[i],".js"))
      {
        urlIndex = i;
      }
    }
  }
  if (isDebugging == true)
  {
    nodeInput = (char *)malloc(size+8);
    memset(nodeInput,0,size+8);
  }
  else
  {
    nodeInput = (char *)malloc(size+46);
    memset(nodeInput,0,size+46);
  }
  int curpos = 0;
  strcpy(nodeInput,"pxscene\0");
  g_argc  = 0;
  g_argv[g_argc++] = &nodeInput[0];
  curpos += 8;

  for (int i=1;i<argc;i++)
  {
    if (strstr(argv[i],"--"))
    {
      strcpy(nodeInput+curpos,argv[i]);
      *(nodeInput+curpos+strlen(argv[i])) = '\0';
      g_argv[g_argc++] = &nodeInput[curpos];
      curpos = curpos + strlen(argv[i]) + 1;
    }
  }
  if (false == isDebugging)
  {
      strcpy(nodeInput+curpos,"-e\0");
      g_argv[g_argc++] = &nodeInput[curpos];
      curpos = curpos + 3;
      strcpy(nodeInput+curpos,"console.log(\"rtNode Initialized\");\0");
      g_argv[g_argc++] = &nodeInput[curpos];
      curpos = curpos + 35;
  }
  script.initializeNode();
#endif
  int retTests = RUN_ALL_TESTS();
  rtLogInfo("Tests executed with return code [%d]", retTests);
  #ifdef ENABLE_CODE_COVERAGE
  __gcov_flush();
  #endif
}
