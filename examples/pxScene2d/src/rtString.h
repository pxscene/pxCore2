// rtCore CopyRight 2007-2015 John Robinson
// rtString.h

#ifndef _RT_STRING
#define _RT_STRING

#include <stdint.h>

#ifndef finline
#define finline
#endif

class rtString {
public:
  rtString();
  rtString(char* s);
  rtString(const char* s);
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
  int32_t length() const;

#if 0
  void subst(const char* before, const char* after) {
  }
#endif

  const char* cString() const;
  operator const char* () const { return mData; }

  //uint32_t operator[](uint32_t i) const {}
  finline bool operator== (const char* s) const { return compare(s) == 0; }
  finline bool operator!= (const char* s) const { return compare(s) != 0; }
  finline bool operator<  (const char* s) const { return compare(s) <  0; }
  finline bool operator<= (const char* s) const { return compare(s) <= 0; }
  finline bool operator>  (const char* s) const { return compare(s) >  0; }
  finline bool operator>= (const char* s) const { return compare(s) >= 0; }
  finline operator char* () const { return mData; }
private:
  char* mData;
};

#endif
