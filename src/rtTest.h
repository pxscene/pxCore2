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

// rtTest.h

#ifndef RT_TEST_H
#define RT_TEST_H

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
