// testSTring.cpp Copyright 2005-2015 John Robinson
// rtCore

#include "testString.h"

#include "rtTest.h"
#include "rtString.h"

void testString()
{
  printf("testString\n");
  rtString s;
  const char sz1[] = "Sample String";
  const char sz2[] = "String";
  const char sz3[] = "S";

  RT_TEST(s == "");
  RT_TEST(s != sz1);
  RT_TEST(s.length() == 0);

  s = sz1;
  RT_TEST(s != "");
  RT_TEST(s == sz1);
  RT_TEST(s.length() == 13);
  RT_TEST(s.find(0,sz2) == 7);
  RT_TEST(s.find(0,sz3) == 0);
  RT_TEST(s.find(1,sz3) == 7);
  RT_TEST(s.find(0,"Blah") == (size_t)-1);

  RT_TEST(s.find(0,'S') == 0);
  RT_TEST(s.find(1,'S') == 7);

  RT_TEST(s.substring(0) == sz1);
  RT_TEST(s.substring(0,1) == "S");
  RT_TEST(s.substring(7) == sz2);
  RT_TEST(s.substring(7,6) == sz2);
}
