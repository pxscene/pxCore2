
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

#include <stdio.h>

#include "rtCore.h"
#include "rtString.h"
#include "rtObject.h"
#include "rtValue.h"

rtValue::rtValue()                      :mType(0) { setEmpty();   }
rtValue::rtValue(bool v)                :mType(0) { setBool(v);   }
rtValue::rtValue(int8_t v)              :mType(0) { setInt8(v);   }
rtValue::rtValue(uint8_t v)             :mType(0) { setUInt8(v);  }
rtValue::rtValue(int32_t v)             :mType(0) { setInt32(v);  }
rtValue::rtValue(uint32_t v)            :mType(0) { setUInt32(v); }
rtValue::rtValue(int64_t v)             :mType(0) { setInt64(v);  }
rtValue::rtValue(uint64_t v)            :mType(0) { setUInt64(v); }
rtValue::rtValue(float v)               :mType(0) { setFloat(v);  }
rtValue::rtValue(double v)              :mType(0) { setDouble(v); }
rtValue::rtValue(const char* v)         :mType(0) { setString(v); }
rtValue::rtValue(const rtString& v)     :mType(0) { setString(v); }
rtValue::rtValue(const rtIObject* v)    :mType(0) { setObject(v); }
rtValue::rtValue(const rtObjectRef& v)  :mType(0) { setObject(v); }
rtValue::rtValue(const rtIFunction* v)  :mType(0) { setFunction(v); }
rtValue::rtValue(const rtFunctionRef& v):mType(0) { setFunction(v); }
rtValue::rtValue(const rtValue& v)      :mType(0) { setValue(v);  }
rtValue::rtValue(voidPtr v)             :mType(0) { setVoidPtr(v); }

rtValue::~rtValue()
{
  term();
}

bool rtValue::operator==(const rtValue& rhs) const
{
  return compare(*this, rhs);
}

bool rtValue::compare(const rtValue& lhs, const rtValue& rhs)
{
  bool result = false;
  if (lhs.getType() == rhs.getType())
  {
    switch(lhs.getType())
    {
    case RT_voidType:     result = true; break;
    case RT_boolType:     result = (lhs.mValue.boolValue == rhs.mValue.boolValue); break;
    case RT_int8_tType:   result = (lhs.mValue.int8Value == rhs.mValue.int8Value); break;
    case RT_uint8_tType:  result = (lhs.mValue.uint8Value == rhs.mValue.uint8Value); break;
    case RT_int32_tType:  result = (lhs.mValue.int32Value == rhs.mValue.int32Value); break;
    case RT_uint32_tType: result = (lhs.mValue.uint32Value == rhs.mValue.uint32Value); break;
    case RT_int64_tType:  result = (lhs.mValue.int64Value == rhs.mValue.int64Value); break;
    case RT_uint64_tType: result = (lhs.mValue.uint64Value == rhs.mValue.uint64Value); break;
    case RT_floatType:    result = (lhs.mValue.floatValue == rhs.mValue.floatValue); break;
    case RT_doubleType:   result = (lhs.mValue.doubleValue == rhs.mValue.doubleValue); break;
    case RT_stringType:
    {
      if (lhs.mValue.stringValue && rhs.mValue.stringValue)
        result = (*lhs.mValue.stringValue == *rhs.mValue.stringValue);
      else
        result = (lhs.mValue.stringValue == rhs.mValue.stringValue);
    }
    break;
    case RT_objectType:   result = (lhs.mValue.objectValue == rhs.mValue.objectValue); break;
    case RT_functionType: result = (lhs.mValue.functionValue == rhs.mValue.functionValue); break;
    }
  }
  return result;
}

rtObjectRef rtValue::toObject() const
{
  rtObjectRef v;
  getObject(v);
  return v;
}

rtFunctionRef rtValue::toFunction() const
{
  rtFunctionRef f;
  getFunction(f);
  return f;
}

void rtValue::setEmpty()
{
  mIsEmpty = true;

  if (mType == RT_objectType)
  {
    if (mValue.objectValue)
    {
      mValue.objectValue->Release();
      mValue.objectValue = NULL;
    }
  }
  else if (mType == RT_functionType)
  {
    if (mValue.functionValue)
    {
       mValue.functionValue->Release();
       mValue.functionValue = NULL;
    }
  }
  else if (mType == RT_stringType)
  {
    if (mValue.stringValue)
    {
      delete mValue.stringValue;
      mValue.stringValue = NULL;
    }
  }

  // TODO setting this to '0' makes node wrappers unhappy
  mType = 0;
  // TODO do we really need thi
  mValue.uint64Value = 0;
  //memset(&mValue, 0, sizeof(mValue));
}

void rtValue::setValue(const rtValue& v)
{
  if (this != &v)
  {
    setEmpty();
    mType = v.mType;
    if (mType == RT_objectType && v.mValue.objectValue != NULL)
    {
      mValue.objectValue = v.mValue.objectValue;
      mValue.objectValue->AddRef();
    }
    else if (mType == RT_functionType && v.mValue.functionValue != NULL)
    {
      mValue.functionValue = v.mValue.functionValue;
      mValue.functionValue->AddRef();
    }
#if 1
    else if (mType == RT_stringType && v.mValue.stringValue != NULL)
    {
      setString(*(v.mValue.stringValue));
    }
#endif
    else
      mValue = v.mValue;
    mIsEmpty = v.mIsEmpty;
  }
}

void rtValue::setBool(bool v)
{
  setEmpty();
  mType    = RT_boolType; mValue.boolValue = v;
  mIsEmpty = false;
}

void rtValue::setInt8(int8_t v)
{
  setEmpty();
  mType    = RT_int8_tType; mValue.int8Value = v;
  mIsEmpty = false;
}

void rtValue::setUInt8(uint8_t v)
{
  setEmpty();
  mType    = RT_uint8_tType; mValue.uint8Value = v;
  mIsEmpty = false;
}

void rtValue::setInt32(int32_t v)
{
  setEmpty();
  mType     = RT_int32_tType; mValue.int32Value = v;
  mIsEmpty = false;
}

void rtValue::setUInt32(uint32_t v)
{
  setEmpty();
  mType    = RT_uint32_tType; mValue.uint32Value = v;
  mIsEmpty = false;
}

void rtValue::setInt64(int64_t v)
{
  setEmpty();
  mType    = RT_int64_tType; mValue.int64Value = v;
  mIsEmpty = false;
}

void rtValue::setUInt64(uint64_t v)
{
  setEmpty();
  mType    = RT_uint64_tType; mValue.uint64Value = v;
  mIsEmpty = false;
}

void rtValue::setFloat(float v)
{
  setEmpty();
  mType    = RT_floatType; mValue.floatValue = v;
  mIsEmpty = false;
}

void rtValue::setDouble(double v)
{
  setEmpty();
  mType    = RT_doubleType; mValue.doubleValue = v;
  mIsEmpty = false;
}

void rtValue::setString(const rtString& v)
{
  setEmpty();
  mType    = RT_stringType; mValue.stringValue = new rtString(v);
  mIsEmpty = false;
}

void rtValue::setObject(const rtIObject* v)
{
  setEmpty();
  mType    = RT_objectType;
  mIsEmpty = false;
  mValue.objectValue = (rtIObject*)v;
  if (v)
    mValue.objectValue->AddRef();
}

void rtValue::setObject(const rtObjectRef& v)
{
#if 0
  setEmpty();
  mType = RT_objectType;
  mValue.objectValue = v.getPtr();
  if (v)
    mValue.objectValue->AddRef();
#else
  setObject(v.getPtr());
#endif
  mIsEmpty = false;
}

void rtValue::setFunction(const rtIFunction* v)
{
  setEmpty();
  mType    = RT_functionType;
  mIsEmpty = false;
  mValue.functionValue = (rtIFunction*)v;
  if (v) mValue.functionValue->AddRef();
}

void rtValue::setFunction(const rtFunctionRef& v)
{
  setFunction(v.getPtr());
}

void rtValue::setVoidPtr(voidPtr v)
{
  setEmpty();
  mType    = RT_voidPtrType;
  mIsEmpty = false;
  mValue.voidPtrValue = v;
}

rtError rtValue::getBool(bool& v) const
{
  if (mType == RT_boolType)
    v = mValue.boolValue;
  else
  {
    v = false; // normalize to default
    switch(mType)
    {
    case RT_voidType:       /* do nothing */                          break;
    case RT_boolType:     v = mValue.boolValue;                       break;
    case RT_int8_tType:   v = (mValue.int8Value==0)     ? false:true; break;
    case RT_uint8_tType:  v = (mValue.uint8Value==0)    ? false:true; break;
    case RT_int32_tType:  v = (mValue.int32Value==0)    ? false:true; break;
    case RT_uint32_tType: v = (mValue.uint32Value==0)   ? false:true; break;
    case RT_int64_tType:  v = (mValue.int64Value==0)    ? false:true; break;
    case RT_uint64_tType: v = (mValue.uint64Value==0)   ? false:true; break;
    case RT_floatType:    v = (mValue.floatValue==0.0f) ? false:true; break;
    case RT_doubleType:   v = (mValue.doubleValue==0.0) ? false:true; break;
    case RT_stringType:
    {
      if (mValue.stringValue) {
        v = (*mValue.stringValue=="")?false:true;
      }
    }
    break;
    case RT_objectType: v = mValue.objectValue?     true:false; break;
    case RT_functionType: v = mValue.functionValue? true:false; break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to bool.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getInt8(int8_t& v)  const
{
  if (mType == RT_int8_tType)
    v = mValue.int8Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:       /* do nothing */                break;
    case RT_boolType:     v = (int8_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (int8_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (int8_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (int8_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (int8_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (int8_t)mValue.int64Value;    break;
    case RT_uint64_tType: v = (int8_t)mValue.uint64Value;   break;
// TODO look at faster float to fixed conversion
    case RT_floatType:    v = (int8_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (int8_t)mValue.doubleValue;   break;
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (int8_t)atol(mValue.stringValue->cString());
    }
    break;
    case RT_objectType: /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to int8_t.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getUInt8(uint8_t& v) const
{
  if (mType == RT_uint8_tType)
    v = mValue.uint8Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:      /* do nothing */                  break;
    case RT_boolType:     v = (uint8_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (uint8_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (uint8_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (uint8_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (uint8_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (uint8_t)mValue.int64Value;    break;
    case RT_uint64_tType: v = (uint8_t)mValue.uint64Value;   break;
// TODO look at faster float to fixed conversion
#ifdef PX_RTVALUE_CAST_UINT_BASIC
    case RT_floatType:    v = (uint8_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (uint8_t)mValue.doubleValue;   break;
#else
    case RT_floatType:    v = (uint8_t)((int8_t)(mValue.floatValue));    break;
    case RT_doubleType:   v = (uint8_t)((int8_t)(mValue.doubleValue));   break;
#endif //PX_RTVALUE_CAST_UINT_BASIC
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (uint8_t)atol(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to uint8_t.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getInt32(int32_t& v) const
{
  if (mType == RT_int32_tType)
    v = mValue.int32Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:       /* do nothing */                 break;
    case RT_boolType:     v = (int32_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (int32_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (int32_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (int32_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (int32_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (int32_t)mValue.int64Value;    break;
    case RT_uint64_tType: v = (int32_t)mValue.uint64Value;   break;
// TODO look at faster float to fixed conversion
    case RT_floatType:    v = (int32_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (int32_t)mValue.doubleValue;   break;
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (int32_t)atol(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to int32_t.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getUInt32(uint32_t& v) const
{
  if (mType == RT_uint32_tType)
    v = mValue.uint32Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:       /* do nothing */                  break;
    case RT_boolType:     v = (uint32_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (uint32_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (uint32_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (uint32_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (uint32_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (uint32_t)mValue.int32Value;    break;
    case RT_uint64_tType: v = (uint32_t)mValue.uint32Value;   break;
// TODO look at faster float to fixed conversion
#ifdef PX_RTVALUE_CAST_UINT_BASIC
    case RT_floatType:    v = (uint32_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (uint32_t)mValue.doubleValue;   break;
#else
    case RT_floatType:    v = (uint32_t)((int32_t)(mValue.floatValue));    break;
    case RT_doubleType:   v = (uint32_t)((int32_t)(mValue.doubleValue));   break;
#endif //PX_RTVALUE_CAST_UINT_BASIC
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (uint32_t)atol(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to uint32_t.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getInt64(int64_t& v) const
{
  if (mType == RT_int64_tType)
    v = mValue.int64Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:       /* do nothing */                 break;
    case RT_boolType:     v = (int64_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (int64_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (int64_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (int64_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (int64_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (int64_t)mValue.int64Value;    break;
    case RT_uint64_tType: v = (int64_t)mValue.uint64Value;   break;
// TODO look at faster float to fixed conversion
    case RT_floatType:    v = (int64_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (int64_t)mValue.doubleValue;   break;
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (int64_t)atoll(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to int64.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getUInt64(uint64_t& v) const
{
  if (mType == RT_uint64_tType)
    v = mValue.uint64Value;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:      /* do nothing */                   break;
    case RT_boolType:     v = (uint64_t)mValue.boolValue?1:0; break;
    case RT_int8_tType:   v = (uint64_t)mValue.int8Value;     break;
    case RT_uint8_tType:  v = (uint64_t)mValue.uint8Value;    break;
    case RT_int32_tType:  v = (uint64_t)mValue.int32Value;    break;
    case RT_uint32_tType: v = (uint64_t)mValue.uint32Value;   break;
    case RT_int64_tType:  v = (uint64_t)mValue.int64Value;    break;
    case RT_uint64_tType: v = (uint64_t)mValue.uint64Value;   break;
// TODO look at faster float to fixed conversion
#ifdef PX_RTVALUE_CAST_UINT_BASIC
    case RT_floatType:    v = (uint64_t)mValue.floatValue;    break;
    case RT_doubleType:   v = (uint64_t)mValue.doubleValue;   break;
#else
    case RT_floatType:    v = (uint64_t)((int64_t)(mValue.floatValue));    break;
    case RT_doubleType:   v = (uint64_t)((int64_t)(mValue.doubleValue));   break;
#endif
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (uint64_t)atoll(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to uint64_t.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getFloat(float& v) const
{
  if (mType == RT_floatType)
    v = mValue.floatValue;
  else
  {
    v = 0.0f; // normalize to default
    switch(mType)
    {
    case RT_voidType:      /* do nothing */               break;
    case RT_boolType:     v = mValue.boolValue?1.0f:0.0f; break;
    case RT_int8_tType:   v = (float)mValue.int8Value;    break;
    case RT_uint8_tType:  v = (float)mValue.uint8Value;   break;
    case RT_int32_tType:  v = (float)mValue.int32Value;   break;
    case RT_uint32_tType: v = (float)mValue.uint32Value;  break;
    case RT_int64_tType:  v = (float)mValue.int64Value;   break;
    case RT_uint64_tType: v = (float)mValue.uint64Value;  break;
//    case RT_floatType: break;
    case RT_doubleType: v = (float)mValue.doubleValue;    break;
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = (float)atof(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to float.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getDouble(double& v) const
{
  if (mType == RT_doubleType)
    v = mValue.doubleValue;
  else
  {
    v = 0; // normalize to default
    switch(mType)
    {
    case RT_voidType:      /* do nothing */               break;
    case RT_boolType:     v = mValue.boolValue?1.0:0.0;   break;
    case RT_int8_tType:   v = (double)mValue.int8Value;   break;
    case RT_uint8_tType:  v = (double)mValue.uint8Value;  break;
    case RT_int32_tType:  v = (double)mValue.int32Value;  break;
    case RT_uint32_tType: v = (double)mValue.uint32Value; break;
    case RT_int64_tType:  v = (double)mValue.int64Value;  break;
    case RT_uint64_tType: v = (double)mValue.uint64Value; break;
    case RT_floatType:    v = (double)mValue.floatValue;  break;
//    case RT_doubleType: break;
    case RT_stringType:
    {
      if (mValue.stringValue)
        v = atof(mValue.stringValue->cString());
    }
    break;
    case RT_objectType:   /* Leave as default */ break;
    case RT_functionType: /* Leave as default */ break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to double.", rtStrType(mType));
      break;
    }
  }
  return RT_OK;
}

rtError rtValue::getString(rtString& v) const
{
  if (mType == RT_stringType && mValue.stringValue)
    v = *mValue.stringValue;
  else
  {
    // TODO EVIL buffer on stack
    char buffer[256]; buffer[0] = 0;
    switch(mType)
    {
    case RT_voidType:     /* do nothing */               break;
    case RT_boolType:     sprintf(buffer, "%s", mValue.boolValue?"true":"false"); break;
    case RT_int8_tType:   sprintf(buffer, "%d", mValue.int8Value);   break;
    case RT_uint8_tType:  sprintf(buffer, "%u", mValue.uint8Value);  break;
    case RT_int32_tType:  sprintf(buffer, "%d", mValue.int32Value);  break;
    case RT_uint32_tType: sprintf(buffer, "%u", mValue.uint32Value); break;
    case RT_int64_tType:  sprintf(buffer, "%lld", (long long int)mValue.int64Value); break;
    case RT_uint64_tType: sprintf(buffer, "%llu", (unsigned long long int)mValue.uint64Value); break;
    case RT_floatType:    sprintf(buffer, "%g", mValue.floatValue);  break;
    case RT_doubleType:   sprintf(buffer, "%g", mValue.doubleValue); break;
      // TODO call toString or description on object
    case RT_objectType: break;
    case RT_functionType: break;
    case RT_voidPtrType:  /* Leave as default */ break;
    default:
      rtLogError("No conversion from %s to string.", rtStrType(mType));
      break;
    }
    v = buffer;
  }
  return RT_OK;
}

rtError rtValue::getObject(rtObjectRef& v) const
{
  if (mType == RT_objectType)
    v = mValue.objectValue;
  else
  {
    // No other types are convertable to object
    v = NULL;
    return RT_ERROR_TYPE_MISMATCH;;
  }
  return RT_OK;
}

rtError rtValue::getFunction(rtFunctionRef& v) const
{
  if (mType == RT_functionType)
    v = mValue.functionValue;
  else
  {
    // No other types are convertable to function
    v = NULL;
    return RT_ERROR_TYPE_MISMATCH;;
  }
  return RT_OK;
}

rtError rtValue::getVoidPtr(voidPtr& v) const
{
  if (mType == RT_voidPtrType)
    v = mValue.voidPtrValue;
  else
  {
    // No other types are convertable
    v = NULL;
    return RT_ERROR_TYPE_MISMATCH;
  }
  return RT_OK;
}

rtError rtValue::getValue(rtValue& v) const
{
  v = *this;
  return RT_OK;
}

const char* rtStrType(char t)
{
  const char* s = "UNKNOWN";
  switch(t)
  {
    RT_TYPE_CASE(RT_voidType);
    RT_TYPE_CASE(RT_boolType);
    RT_TYPE_CASE(RT_int8_tType);
    RT_TYPE_CASE(RT_uint8_tType);
    RT_TYPE_CASE(RT_int32_tType);
    RT_TYPE_CASE(RT_uint32_tType);
    RT_TYPE_CASE(RT_int64_tType);
    RT_TYPE_CASE(RT_uint64_tType);
    RT_TYPE_CASE(RT_floatType);
    RT_TYPE_CASE(RT_doubleType);
    RT_TYPE_CASE(RT_stringType);
    RT_TYPE_CASE(RT_objectType);
    RT_TYPE_CASE(RT_functionType);
    RT_TYPE_CASE(RT_voidPtrType);
    default:
    break;
  }
  return s;
}
