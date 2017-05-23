
#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#endif

#undef ASSERT_GE  // avoid collision
#undef ASSERT_GT  // avoid collision
#undef ASSERT_LT  // avoid collision
#undef ASSERT_LE  // avoid collision
#undef ASSERT_NE  // avoid collision
#undef ASSERT_EQ  // avoid collision

#include "gtest/gtest.h"

#if !defined(WIN32) && !defined(ENABLE_DFB)
#pragma GCC diagnostic pop
#endif
