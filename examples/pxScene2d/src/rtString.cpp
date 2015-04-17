// rtCore CopyRight 2007-2015 John Robinson
// rtString.h
// A simple utf8 string class

#include "rtString.h"
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include "rtLog.h"
extern "C" {
#include "utf8.h"
}

rtString::rtString(): mData(0)  {
  //  printf("default constructor\n");
}

rtString::rtString(char* s): mData(0) {
  if (s)
    mData = strdup(s);
  //  printf("char* constructor\n");
}

rtString::rtString(const char* s): mData(0) {
  if (s)
    mData = strdup(s);
}

rtString::rtString(const rtString& s): mData(0) {
  if (s.mData)
    mData = strdup(s.mData);
  //  printf("copy constructor\n");
}

rtString& rtString::operator=(const rtString& s) {
  if (s.mData) // Aliases
    mData = strdup(s.mData);
  return *this;
}

rtString& rtString::operator=(const char* s) {
  if (s) // Aliases
    mData = strdup(s);
  return *this;
}

bool rtString::isEmpty()
{
  return (!mData || !(*mData));
}

rtString::~rtString() { term(); }

void rtString::term() {
  if (mData)
      free(mData);
  mData = 0;
}

void rtString::append(const char* s) {
  int sl = strlen(s);
  int dl = strlen(mData);
  mData = (char*)realloc((void*)mData, dl+sl+1);
    strcpy(mData+dl, s);
}

int rtString::compare(const char* s) const {
  u_int32_t c1, c2;
  int i1 = 0, i2 = 0;
  
  do {
    c1 = u8_nextchar((char*)s, &i1);
    c2 = u8_nextchar((char*)mData, &i2);
  } while (c1 == c2 && c1 && c2);
  
  return c1-c2;
}


const char* rtString::cString() const {
  // TODO const cast 
  return mData?(const char*)mData:"";
}

//HACK: missing symbol. Is this utf8?
int32_t rtString::length() const {
  return mData ? strlen(mData) : 0;
}

