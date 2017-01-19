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
  /**
   * Default constructor
   */
  rtString();

  /**
   * Constructor
   * @param s A c-style string that must be null-terminated
   */
  rtString(const char* s);

  rtString(const char* s, uint32_t byteLen);
  rtString(const rtString& s);
  
  ~rtString();
  
  rtString& operator=(const rtString& s);
  rtString& operator=(const char* s);

  /**
   * Determines if the string is empty.
   * @returns true if string length is zero or is null
   */
  bool isEmpty() const;

  void term();

  void append(const char* s);

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
