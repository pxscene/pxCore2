#include "gtest/gtest.h"


// PURPOSE:  Test that 'exampleValue()' return value is expected value. 

int exampleTest(int value)
{
  return value * 2;
}
 
TEST(pxScene2dTests, exampleTest)
{ 
    EXPECT_EQ (2, exampleTest(1));
    EXPECT_EQ (4, exampleTest(2));
    EXPECT_EQ (6, exampleTest(3));
}



/*
SOURCE: https://github.com/google/googletest/blob/master/googletest/docs/Primer.md

Basic Assertions
================

These assertions do basic true/false condition testing.

Fatal assertion             Nonfatal assertion          Verifies
ASSERT_TRUE(condition);     EXPECT_TRUE(condition);     condition is true
ASSERT_FALSE(condition);    EXPECT_FALSE(condition);    condition is false

Remember, when they fail, ASSERT_* yields a fatal failure and returns from the current function,
while EXPECT_* yields a nonfatal failure, allowing the function to continue running.
In either case, an assertion failure means its containing test fails.

Availability: Linux, Windows, Mac.


Binary Comparison
=================

This section describes assertions that compare two values.

Fatal assertion         Nonfatal assertion      Verifies
ASSERT_EQ(val1,val2);   EXPECT_EQ(val1,val2);   val1 == val2
ASSERT_NE(val1,val2);   EXPECT_NE(val1,val2);   val1 != val2
ASSERT_LT(val1,val2);   EXPECT_LT(val1,val2);   val1 <  val2
ASSERT_LE(val1,val2);   EXPECT_LE(val1,val2);   val1 <= val2
ASSERT_GT(val1,val2);   EXPECT_GT(val1,val2);   val1 >  val2
ASSERT_GE(val1,val2);   EXPECT_GE(val1,val2);   val1 >= val2

In the event of a failure, Google Test prints both val1 and val2.

Value arguments must be comparable by the assertion's comparison operator or you'll get a compiler error.
We used to require the arguments to support the << operator for streaming to an ostream, but it's no longer
necessary since v1.6.0 (if << is supported, it will be called to print the arguments when the assertion fails;
otherwise Google Test will attempt to print them in the best way it can.

For more details and how to customize the printing of the arguments, see this Google Mock recipe.).

These assertions can work with a user-defined type, but only if you define the corresponding comparison
operator (e.g. ==, <, etc). If the corresponding operator is defined, prefer using the ASSERT_*() macros because
they will print out not only the result of the comparison, but the two operands as well.

Arguments are always evaluated exactly once. Therefore, it's OK for the arguments to have side effects. However,
as with any ordinary C/C++ function, the arguments' evaluation order is undefined (i.e. the compiler is free to
choose any order) and your code should not depend on any particular argument evaluation order.

ASSERT_EQ() does pointer equality on pointers. If used on two C strings, it tests if they are in the same memory
location, not if they have the same value. Therefore, if you want to compare C strings (e.g. const char*) by value,
use ASSERT_STREQ() , which will be described later on. In particular, to assert that a C string is NULL,
use ASSERT_STREQ(NULL, c_string) . However, to compare two string objects, you should use ASSERT_EQ.

Macros in this section work with both narrow and wide string objects (string and wstring).

Availability: Linux, Windows, Mac.

Historical note: Before February 2016 *_EQ had a convention of calling it as ASSERT_EQ(expected, actual), so lots of
existing code uses this order. Now *_EQ treats both parameters in the same way.


String Comparison
=================

The assertions in this group compare two C strings. If you want to compare two string objects, use EXPECT_EQ, 
EXPECT_NE, and etc instead.

Fatal assertion	Nonfatal assertion	Verifies

ASSERT_STREQ(str1,str2);	EXPECT_STREQ(str1,_str_2);	the two C strings have the same content
ASSERT_STRNE(str1,str2);	EXPECT_STRNE(str1,str2);	the two C strings have different content
ASSERT_STRCASEEQ(str1,str2);	EXPECT_STRCASEEQ(str1,str2);	the two C strings have the same content, ignoring case
ASSERT_STRCASENE(str1,str2);	EXPECT_STRCASENE(str1,str2);	the two C strings have different content, ignoring case

Note that "CASE" in an assertion name means that case is ignored.

*/

