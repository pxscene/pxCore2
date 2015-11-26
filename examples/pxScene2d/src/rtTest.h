// rtTest.h CopyRight 2005-2006 John Robinson
// rtCore

#ifndef _H_RT_TEST
#define _H_RT_TEST

#include <stdio.h>

void rtResetTestCounters();
void rtDumpTestCounters();

extern long totalTestsRun;
extern long totalTestsFailed;

#define RT_TEST(x) {totalTestsRun++; if (!(x)) {totalTestsFailed++; printf("Test Failed File: %s Line: %d\n", __FILE__, __LINE__);}}
#define RT_TEST_OK(x) {totalTestsRun++; if ((x) != RT_OK) {totalTestsFailed++; printf("Test Failed File: %s Line: %d\n", __FILE__, __LINE__);}}
#define RT_TEST_FAIL(x) {totalTestsRun++; if ((x) == RT_OK) {totalTestsFailed++; printf("Test Failed File: %s Line: %d\n", __FILE__, __LINE__)}
#define RT_TEST_FAIL_CODE(x, code) {totalTestsRun++; if ((x) != code) {totalTestsFailed++; printf("Test Failed File: %s Line: %d\n", __FILE__, __LINE__);}}

#endif
