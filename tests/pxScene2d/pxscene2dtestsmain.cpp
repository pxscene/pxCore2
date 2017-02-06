#include "gtest/gtest.h"
#include "pxContext.h"

pxContext context;
#ifdef ENABLE_DEBUG_MODE
char *g_debugPort = NULL;
int g_argc = 0;
char** g_argv;
char *nodeInput = NULL;
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
