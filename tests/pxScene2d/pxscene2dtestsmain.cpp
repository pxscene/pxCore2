#include "gtest/gtest.h"
#include "pxContext.h"

pxContext context;
#ifdef ENABLE_DEBUG_MODE
extern int g_argc;
extern char** g_argv;
#endif

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
