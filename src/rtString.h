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

#ifndef _RT_STRING_H
#define _RT_STRING_H

//#include "rtCore.h"
#include <stdlib.h>
#include <stdint.h>

#ifndef finline
#define finline
#endif


/**
  A lightweight utf-8 string class.
*/
class rtString 
{
public:

  rtString();
  
  // Assumes utf-8 compatible encoding and null-terminated
  rtString(const char* s);

  // Assumes utf-8 compatible encoding
  // internal copy will be null-terminated
  rtString(const char* s, uint32_t byteLen);

  rtString(const rtString& s);
  
  ~rtString();

  rtString& operator=(const rtString& s);
  rtString& operator=(const char* s);

  friend
  rtString operator+(const rtString& lhs, const char *rhs)
  {
    rtString ans(lhs);
    ans = ans.append(rhs);
    return ans;
  }

  rtString& operator +(const char* s)     { return append(s);           };
  rtString& operator +(const rtString& s) { return append(s.cString()); };
  rtString& operator+=(const char* s)     { return append(s);           };
  rtString& operator+=(const rtString& s) { return append(s.cString()); };
  
  /**
   * Determines if the string is empty.
   * @returns true if string length is zero or is null
   */
  bool isEmpty() const;

  rtString& init(const char* s, size_t byteLen);

  void term();

  rtString&  append(const char* s);

  int compare(const char* s) const;

  /**
   * The length of the string in utf8 characters.
   * @returns The number of utf8 characters.
   */
  int32_t length() const;
  
  /**
   * The length of the string in bytes.
   * @returns The number of bytes in the string.
   */
  int32_t byteLength() const;

#if 0
  void subst(const char* before, const char* after) 
  {
  }
#endif

  const char* cString() const;
  operator const char* () const { return mData?mData:""; }

  //uint32_t operator[](uint32_t i) const {}

  finline bool operator== (const char* s) const { return compare(s) == 0; }
  finline bool operator!= (const char* s) const { return compare(s) != 0; }
  finline bool operator<  (const char* s) const { return compare(s) <  0; }
  finline bool operator<= (const char* s) const { return compare(s) <= 0; }
  finline bool operator>  (const char* s) const { return compare(s) >  0; }
  finline bool operator>= (const char* s) const { return compare(s) >= 0; }

  bool beginsWith(const char* s) const;
  bool endsWith(const char* s) const;

  rtString substring(size_t pos, size_t len = 0) const;

#if 0
  pos_t find(size_t pos, const char* s, size_t n) const;
#endif

  int32_t find(size_t pos, const char* s) const;
  int32_t find(size_t pos, uint32_t codePoint) const;

private:
  char* mData;
};

#endif
