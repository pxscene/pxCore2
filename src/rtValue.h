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

// rtValue.h
#ifndef RT_VALUE_H
#define RT_VALUE_H

#include <stdio.h>

#include "rtCore.h"
#include "rtString.h"

#define RT_voidType               '\0'
#define RT_valueType              'v'
#define RT_rtValueType            'v'
#define RT_boolType               'b'
#define RT_int8_tType             '1'
#define RT_uint8_tType            '2'
#define RT_intType                '4'
#define RT_int32_tType            '4'
#define RT_uint32_tType           '5'
#define RT_int64_tType            '6'
#define RT_uint64_tType           '7'
#define RT_floatType              'e'
#define RT_doubleType             'd'
#define RT_stringType             's'
#define RT_rtStringType           's'
#define RT_objectType             'o'
#define RT_rtObjectRefType        'o'
#define RT_functionType           'f'
#define RT_rtFunctionRefType      'f'
#define RT_voidPtrType            'z'

// TODO JR Hack Only needed for reflection... method signature
//Try #define CHARIZE(x) #x[0]
//http://www.complete-concrete-concise.com/programming/c/preprocessor-%E2%80%93-understanding-the-stringizing-operator

#define RT_voidType2               "\0"
#define RT_valueType2              "v"
#define RT_rtValueType2            "v"
#define RT_boolType2               "b"
#define RT_int8_tType2             "1"
#define RT_uint8_tType2            "2"
#define RT_intType2                "4"
#define RT_int32_tType2            "4"
#define RT_uint32_tType2           "5"
#define RT_int64_tType2            "6"
#define RT_uint64_tType2           "7"
#define RT_floatType2              "e"
#define RT_doubleType2             "d"
#define RT_rtStringType2           "s"
#define RT_rtObjectRefType2        "o"
#define RT_rtFunctionRefType2      "f"
#define RT_voidPtrType2            "z"

class rtIObject;
class rtIFunction;
class rtObject;
class rtObjectRef;
class rtFunctionRef;

const char* rtStrType(char t); //fwd

typedef void* voidPtr;

union rtValue_
{
  bool        boolValue;
  int8_t      int8Value;
  uint8_t     uint8Value;
  int32_t     int32Value;
  int64_t     int64Value;
  uint64_t    uint64Value;
  uint32_t    uint32Value;
  float       floatValue;
  double      doubleValue;
  rtString    *stringValue;
  rtIObject   *objectValue;
  rtIFunction *functionValue;
  voidPtr     voidPtrValue;  // For creating mischief
};

typedef char rtType;

class rtValue
{
 public:
  rtValue();
  rtValue(bool v);
  rtValue(int8_t v);
  rtValue(uint8_t v);
  rtValue(int32_t v);
  rtValue(uint32_t v);
  rtValue(int64_t v);
  rtValue(uint64_t v);
  rtValue(float v);
  rtValue(double v);
  rtValue(const char* v);
  rtValue(const rtString& v);
  rtValue(const rtIObject* v);
  rtValue(const rtObjectRef& v);
  rtValue(const rtIFunction* v);
  rtValue(const rtFunctionRef& v);
  rtValue(const rtValue& v);
  rtValue(voidPtr v);
  ~rtValue();

  void term() { setEmpty(); }

  finline rtValue& operator=(bool v)                { setBool(v);     return *this; }
  finline rtValue& operator=(int8_t v)              { setInt8(v);     return *this; }
  finline rtValue& operator=(uint8_t v)             { setUInt8(v);    return *this; }
  finline rtValue& operator=(int32_t v)             { setInt32(v);    return *this; }
  finline rtValue& operator=(uint32_t v)            { setUInt32(v);   return *this; }
  finline rtValue& operator=(int64_t v)             { setInt64(v);    return *this; }
  finline rtValue& operator=(uint64_t v)            { setUInt64(v);   return *this; }
  finline rtValue& operator=(float v)               { setFloat(v);    return *this; }
  finline rtValue& operator=(double v)              { setDouble(v);   return *this; }
  finline rtValue& operator=(const char* v)         { setString(v);   return *this; }
  finline rtValue& operator=(const rtString& v)     { setString(v);   return *this; }
  finline rtValue& operator=(const rtIObject* v)    { setObject(v);   return *this; }
  finline rtValue& operator=(const rtObjectRef& v)  { setObject(v);   return *this; }
  finline rtValue& operator=(const rtIFunction* v)  { setFunction(v); return *this; }
  finline rtValue& operator=(const rtFunctionRef& v){ setFunction(v); return *this; }
  finline rtValue& operator=(const rtValue& v)      { setValue(v);    return *this; }
  finline rtValue& operator=(voidPtr v)             { setVoidPtr(v);  return *this; }

  bool operator!=(const rtValue& rhs) const { return !(*this == rhs); }
  bool operator==(const rtValue& rhs) const;


  finline bool       toBool()     const { bool v;        getBool(v);   return v; }
  finline int8_t     toInt8()     const { int8_t v;      getInt8(v);   return v; }
  finline uint8_t    toUInt8()    const { uint8_t v;     getUInt8(v);  return v; }
  finline int32_t    toInt32()    const { int32_t v;     getInt32(v);  return v; }
  finline uint32_t   toUInt32()   const { uint32_t v(0); getUInt32(v); return v; }
  finline int64_t    toInt64()    const { int64_t v(0);  getInt64(v);  return v; }
  finline uint64_t   toUInt64()   const { uint64_t v(0); getUInt64(v); return v; }
  finline float      toFloat()    const { float v;       getFloat(v);  return v; }
  finline double     toDouble()   const { double v;      getDouble(v); return v; }
  finline rtString   toString()   const { rtString v;    getString(v); return v; }
  rtObjectRef        toObject()   const;
  rtFunctionRef      toFunction() const;
  voidPtr            toVoidPtr()  const { voidPtr v;     getVoidPtr(v);return v; }

  rtType getType() const { return mType; }

  const char *getTypeStr() const { return ::rtStrType(mType); }

  finline bool isEmpty() const { return mIsEmpty; };

  void setEmpty();
  void setValue(const rtValue& v);
  void setBool(bool v);
  void setInt8(int8_t v);
  void setUInt8(uint8_t v);
  void setInt32(int32_t v);
  void setUInt32(uint32_t v);
  void setInt64(int64_t v);
  void setUInt64(uint64_t v);
  void setFloat(float v);
  void setDouble(double v);
  void setString(const rtString& v);
  void setObject(const rtIObject* v);
  void setObject(const rtObjectRef& v);
  void setFunction(const rtIFunction* v);
  void setFunction(const rtFunctionRef& v);
  void setVoidPtr(voidPtr v);

  rtError getValue(rtValue& v)          const;
  rtError getBool(bool& v)              const;
  rtError getInt8(int8_t& v)            const;
  rtError getUInt8(uint8_t& v)          const;
  rtError getInt32(int32_t& v)          const;
  rtError getInt64(int64_t& v)          const;
  rtError getUInt64(uint64_t& v)        const;
  rtError getUInt32(uint32_t& v)        const;
  rtError getFloat(float& v)            const;
  rtError getDouble(double& v)          const;
  rtError getString(rtString& v)        const;
  rtError getObject(rtObjectRef& v)     const;
  rtError getFunction(rtFunctionRef& v) const;
  rtError getVoidPtr(voidPtr& v)        const;

  // TODO rework this so we avoid a copy if the type matches
  template <typename T>
    T convert() const { T t; cvt(t); return t; }

  template <typename T>
  rtError tryConvert (T& t) { return cvt(t); }

  template <typename T>
    void assign(const T t) { asn(t); }

 protected:

  // Both values must have the same type
  static bool compare(const rtValue& lhs, const rtValue& rhs);

   rtError cvt(rtValue& v)                const { return getValue(v);    }
   rtError cvt(bool& v)                   const { return getBool(v);     }
   rtError cvt(int8_t& v)                 const { return getInt8(v);     }
   rtError cvt(uint8_t& v)                const { return getUInt8(v);    }
   rtError cvt(int32_t& v)                const { return getInt32(v);    }
   rtError cvt(uint32_t& v)               const { return getUInt32(v);   }
   rtError cvt(int64_t& v)                const { return getInt64(v);    }
   rtError cvt(uint64_t& v)               const { return getUInt64(v);   }
   rtError cvt(float& v)                  const { return getFloat(v);    }
   rtError cvt(double& v)                 const { return getDouble(v);   }
   rtError cvt(rtString& v)               const { return getString(v);   }
   rtError cvt(rtObjectRef& v)            const { return getObject(v);   }
   rtError cvt(rtFunctionRef& v)          const { return getFunction(v); }
   rtError cvt(voidPtr& v)                const { return getVoidPtr(v);  }

  void asn(const rtValue& v)                { setValue(v);    }
  void asn(bool v)                          { setBool(v);     }
  void asn(int8_t v)                        { setInt8(v);     }
  void asn(uint8_t v)                       { setUInt8(v);    }
  void asn(int32_t v)                       { setInt32(v);    }
  void asn(uint32_t v)                      { setUInt32(v);   }
  void asn(int64_t v)                       { setInt64(v);    }
  void asn(uint64_t v)                      { setUInt64(v);   }
  void asn(float v)                         { setFloat(v);    }
  void asn(double v)                        { setDouble(v);   }
  void asn(const char* v)                   { setString(v);   }
  void asn(const rtString& v)               { setString(v);   }
  void asn(const rtIObject* v)              { setObject(v);   }
  void asn(const rtObjectRef& v)            { setObject(v);   }
  void asn(const rtIFunction* v)            { setFunction(v); }
  void asn(const rtFunctionRef& v)          { setFunction(v); }
  void asn(voidPtr v)                       { setVoidPtr(v);  }

  rtError coerceType(rtType newType);

  rtType   mType;
  rtValue_ mValue;

  bool     mIsEmpty;
};

#define RT_TYPE_CASE(t) case t: s = # t; break;

#define RT_TYPE_NAME(t) (# t)

//const char* rtStrType(char t);

#endif
