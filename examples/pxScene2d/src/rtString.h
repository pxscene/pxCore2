// rtCore Copyright 2007-2015 John Robinson
// rtString.h

#ifndef _RT_STRING
#define _RT_STRING

#include <stdlib.h>
#include <stdint.h>

#ifndef finline
#define finline
#endif

class rtString {
public:
  rtString();
  rtString(char* s);
  rtString(const char* s);
  rtString(const char* s, uint32_t byteLen);
  rtString(const rtString& s);
  
  ~rtString();
  
  rtString& operator=(const rtString& s);
  rtString& operator=(const char* s);
  
  bool isEmpty();
  void term();
  //void escape(rtString& s);
  //void unescape(rtString& s);
  void append(const char* s);

  int compare(const char* s) const;

  // number of utf8 characters
  int32_t length() const;
  
  // number of bytes
  int32_t byteLength() const;

#if 0
  void subst(const char* before, const char* after) 
  {
  }
#endif

  const char* cString() const;
  operator const char* () const { return mData?(const char*)mData:""; }

  //uint32_t operator[](uint32_t i) const {}
  finline bool operator== (const char* s) const { return compare(s) == 0; }
  finline bool operator!= (const char* s) const { return compare(s) != 0; }
  finline bool operator<  (const char* s) const { return compare(s) <  0; }
  finline bool operator<= (const char* s) const { return compare(s) <= 0; }
  finline bool operator>  (const char* s) const { return compare(s) >  0; }
  finline bool operator>= (const char* s) const { return compare(s) >= 0; }

  bool beginsWith(const char* s) const;

  rtString substring(size_t pos, size_t len = 0) const;

#if 0
  pos_t find(size_t pos, const char* s, size_t n);
#endif

  size_t find(size_t pos, const char* s) const;
  size_t find(size_t pos, uint32_t codePoint) const;

private:
  char* mData;
};

#endif
