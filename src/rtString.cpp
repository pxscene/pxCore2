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

// rtString.h

#include "rtString.h"
#include <string.h>
#include <stdlib.h>

#include <stdio.h>
#include "rtLog.h"
extern "C"
{
#include "utf8.h"
}

rtString::rtString(): mData(NULL) {}

rtString::rtString(const char* s): mData(NULL)
{
  if (s)
    mData = strdup(s);
}

rtString::rtString(const char* s, uint32_t byteLen): mData(NULL)
{
  if (s)
  {
    init(s, byteLen);
  }
}

rtString& rtString::init(const char* s, size_t byteLen)
{
  mData = NULL;
  
  if (s)
  {
    mData = (char*)malloc(byteLen+1);
    memcpy(mData, s, byteLen);
    mData[byteLen] = 0; // null terminate
  }

  return *this;
}

rtString::rtString(const rtString& s): mData(NULL)
{
  if (s.mData)
    mData = strdup(s.mData);
}

rtString& rtString::operator=(const rtString& s) 
{
  if (this != &s)
  {
    term();
    if (s.mData)
      mData = strdup(s.mData);
  }
  return *this;
}

rtString& rtString::operator=(const char* s) 
{
  if (s != mData)
  {
    term();
    if (s)
      mData = strdup(s);
  }
  return *this;
}

bool rtString::isEmpty() const
{
  return (!mData || !(*mData));
}

rtString::~rtString() { term(); }

void rtString::term() 
{
  if (mData)
    free(mData);
  mData = 0;
}

rtString& rtString::append(const char* s)
{
  size_t sl = s?strlen(s):0;
  size_t dl = mData?strlen(mData):0;
  mData = (char*)realloc((void*)mData, dl+sl+1);
  strcpy(mData+dl, s?s:"");
  
  return *this;
}

int rtString::compare(const char* s) const 
{
  const char *d = mData?mData:"";
  s = s?s:"";
 
  u_int32_t c1, c2;
  int i1 = 0, i2 = 0;
  
  do 
  {
    c1 = u8_nextchar((char*)d, &i1);
    c2 = u8_nextchar((char*)s, &i2);
  } while (c1 == c2 && c1 && c2);
  
  return c1==c2?0:c1<c2?-1:1;
}


const char* rtString::cString() const 
{
  return mData?mData:"";
}

int32_t rtString::length() const 
{
  return mData?u8_strlen(mData):0;
}

int32_t rtString::byteLength() const 
{
  return (int32_t)(mData?strlen(mData):0);
}

bool rtString::beginsWith(const char* s) const
{
  s = s?s:"";
  return (strncmp(cString(),s,strlen(s))==0);
}

bool rtString::endsWith(const char* s) const
{
  s = s?s:"";
  const char* t = mData?mData:"";
  int sl = u8_strlen((char*)s);
  int tl = u8_strlen((char*)t);

  bool result = false;
  if (tl-sl > 0)
  {
    t = t + u8_charnum((char*)t,tl-sl);
    result = (strncmp(t,s,sl) == 0);
  }
  return result;
}

int32_t rtString::find(size_t pos, const char* str) const
{
  int haystack = 0;
  int old;
  int needle = 0;
  size_t haystackPos = 0;
  char* s = (char*)str;

  old = haystack;
  uint32_t haystackChar = u8_nextchar(mData, &haystack);
  for(;haystackChar && (haystackPos < pos);old=haystack,haystackChar=u8_nextchar(mData,&haystack),++haystackPos)
  {
    // skipping
  }
  for(;haystackChar;old=haystack,haystackChar=u8_nextchar(mData,&haystack),++haystackPos)
  {
    int h = old;
    int n = needle;
    uint32_t hChar = u8_nextchar(mData,&h);
    uint32_t nChar = u8_nextchar(s,&n);
    for(;hChar && nChar && (hChar==nChar); hChar=u8_nextchar(mData,&h),nChar=u8_nextchar(s,&n))
    {
      // matching
    }
    if (nChar == 0)
    {
      // matched needle
      return (int32_t)haystackPos;
    }
    // keep looking further in haystack
  }
  return -1;
}

int32_t rtString::find(size_t pos, uint32_t codePoint) const
{
  int i = 0;
  size_t p = 0;
  u_int32_t c = u8_nextchar(mData, &i);
  while(p < pos && c)
  {
    c = u8_nextchar(mData, &i);
    p++;
  }
  
  if (c)
  {

    while(c && c != codePoint)
    {
      c = u8_nextchar(mData, &i);
      p++;
    }
    if (c == codePoint)
      return (int32_t)p;
  }

  return -1;
}

rtString rtString::substring(size_t pos, size_t len) const
{
  char* s = mData;
  if (pos>0)
    s = s + u8_offset(s,(int)pos);

  if (len == 0)
    return rtString(s);
  else
  {
    int byteEnd = u8_offset(s,(int)len);
    return rtString(s,byteEnd);
  }
}
