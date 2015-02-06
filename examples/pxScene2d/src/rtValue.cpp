
#include <stdio.h>

#include "rtDefs.h"
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
rtValue::rtValue(const rtFunctionRef& v):mType(0) { setFunction(v); }
rtValue::rtValue(const rtValue& v)      :mType(0) { setValue(v);  }
rtValue::rtValue(voidPtr v)             :mType(0) { setVoidPtr(v); }

rtObjectRef rtValue::toObject() const  { 
  rtObjectRef v; 
  getObject(v); 
  return v; 
}

rtFunctionRef rtValue::toFunction() const {
  rtFunctionRef f;
  getFunction(f);
  return f;
}

void rtValue::setEmpty() {
  if (mType == RT_objectType) 
  {
    if (mValue.objectValue) 
    {
      mValue.objectValue->Release();
      mValue.objectValue = NULL;
    }
    else if (mType == RT_stringType) 
    {
      if (mValue.stringValue) 
      {
        delete mValue.stringValue;
        mValue.stringValue = NULL;
      }
    }
  }
  // TODO setting this to '0' makes node wrappers unhappy
  mType = 0;
  // TODO do we really need thi
  mValue.uint64Value = 0;
}

void rtValue::setValue(const rtValue& v) {
  setEmpty();
  mType = v.mType; mValue = v.mValue;
}

void rtValue::setBool(bool v) {
  setEmpty();
  mType = RT_boolType; mValue.boolValue = v;
}

void rtValue::setInt8(int8_t v) {
  setEmpty();
  mType = RT_int8_tType; mValue.int8Value = v;
}

void rtValue::setUInt8(uint8_t v) {
  setEmpty();
  mType = RT_uint8_tType; mValue.uint8Value = v;
}

void rtValue::setInt32(int32_t v) {
  setEmpty();
  mType = RT_int32_tType; mValue.int32Value = v;
}

void rtValue::setUInt32(uint32_t v) {
  setEmpty();
  mType = RT_uint32_tType; mValue.uint32Value = v;
}

void rtValue::setInt64(int64_t v) {
  setEmpty();
  mType = RT_int64_tType; mValue.int64Value = v;
}

void rtValue::setUInt64(uint64_t v) {
  setEmpty();
  mType = RT_uint64_tType; mValue.uint64Value = v;
}

void rtValue::setFloat(float v) {
  setEmpty();
  mType = RT_floatType; mValue.floatValue = v;
}

void rtValue::setDouble(double v) {
  setEmpty();
  mType = RT_doubleType; mValue.doubleValue = v;
}

void rtValue::setString(const rtString& v) {
  setEmpty();
  mType = RT_stringType; mValue.stringValue = new rtString(v);
}

void rtValue::setObject(const rtIObject* v) {
  setEmpty();
  mType = RT_objectType; 
  mValue.objectValue = (rtIObject*)v;
  if (v) mValue.objectValue->AddRef(); 
}

void rtValue::setObject(const rtObjectRef& v) {
  setEmpty();
  mType = RT_objectType; 
  mValue.objectValue = v.getPtr();
  if (v) mValue.objectValue->AddRef(); 
}

void rtValue::setFunction(const rtIFunction* v) {
  setEmpty();
  mType = RT_functionType;
  mValue.functionValue = (rtIFunction*)v;
  if (v) mValue.functionValue->AddRef();
}

void rtValue::setFunction(const rtFunctionRef& v) {
  setFunction(v.getPtr());
}

void rtValue::setVoidPtr(voidPtr v) {
  setEmpty();
  mType = RT_voidPtrType;
  mValue.voidPtrValue = v;
}

rtError rtValue::getBool(bool& v) const {
  if (mType == RT_boolType) v = mValue.boolValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_boolType);
    v = t.mValue.boolValue;
  }
  return RT_OK;
}

rtError rtValue::getInt8(int8_t& v)  const {
  if (mType == RT_int8_tType) v = mValue.int8Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_int8_tType);
    v = t.mValue.int8Value;
  }
  return RT_OK;
}

rtError rtValue::getUInt8(uint8_t& v) const {
  if (mType == RT_uint8_tType) v = mValue.uint8Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_uint8_tType);
    v = t.mValue.uint8Value;
  }
  return RT_OK;
}

rtError rtValue::getInt32(int32_t& v) const {
  if (mType == RT_int32_tType) v = mValue.int32Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_int32_tType);
    v = t.mValue.int32Value;
  }
  return RT_OK;
}

rtError rtValue::getUInt32(uint32_t& v) const {
  if (mType == RT_uint32_tType) v = mValue.uint32Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_uint32_tType);
    v = t.mValue.uint32Value;
  }
  return RT_OK;
}


rtError rtValue::getInt64(int64_t& v) const {
  if (mType == RT_int64_tType) v = mValue.int64Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_int64_tType);
    v = t.mValue.int64Value;
  }
  return RT_OK;
}

rtError rtValue::getUInt64(uint64_t& v) const {
  if (mType == RT_uint64_tType) v = mValue.uint64Value;
  else {
    rtValue t = *this;
    t.coerceType(RT_uint64_tType);
    v = t.mValue.int64Value;
  }
  return RT_OK;
}
rtError rtValue::getFloat(float& v) const {
  if (mType == RT_floatType) v = mValue.floatValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_floatType);
    v = t.mValue.floatValue;
  }
  return RT_OK;
}

rtError rtValue::getDouble(double& v) const {
  if (mType == RT_doubleType) v = mValue.doubleValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_doubleType);
    v = t.mValue.doubleValue;
  }
  return RT_OK;
}

rtError rtValue::getString(rtString& v) const {
#if 1
  if (mType == RT_stringType && mValue.stringValue) 
    v = *mValue.stringValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_stringType);
    v = *t.mValue.stringValue;
  }
#endif
  return RT_OK;
}

rtError rtValue::getObject(rtObjectRef& v) const {
  if (mType == RT_objectType) v = mValue.objectValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_objectType);
    v = t.mValue.objectValue;
  }
  return RT_OK;
}

rtError rtValue::getFunction(rtFunctionRef& v) const {
  if (mType == RT_functionType) v = mValue.functionValue;
  else {
    rtValue t = *this;
    t.coerceType(RT_functionType);
    v = t.mValue.functionValue;
  }
  return RT_OK;
}

rtError rtValue::getVoidPtr(voidPtr& v) const {
  if (mType == RT_voidPtrType) v = mValue.voidPtrValue;
  else v = NULL;
  return RT_OK;
}

rtError rtValue::getValue(rtValue& v) const {
  v = *this;
  return RT_OK;
}

rtError rtValue::coerceType(rtType newType) {
  if (mType != newType) {
    //    rtLog("In coerceType\n");
    switch(mType) {
    case RT_boolType:
    {
      switch(newType) {
      case RT_boolType: setBool(mValue.boolValue); break;
      case RT_int8_tType: setInt8(mValue.boolValue?0:1); break;
      case RT_uint8_tType: setUInt8(mValue.boolValue?0:1); break;
      case RT_int32_tType: setInt32(mValue.boolValue?0:1); break;
      case RT_uint32_tType: setUInt32(mValue.boolValue?0:1); break;
      case RT_floatType: setFloat(mValue.boolValue?0.0f:1.0f); break;
      case RT_doubleType: setDouble(mValue.boolValue?0.0:1.0); break;
      case RT_stringType: setString(mValue.boolValue?"true":"false"); break;
      case RT_objectType: setObject(NULL); break;
      case RT_functionType: setFunction(NULL); break;
      default:
        rtLogWarn("missed conversion");
        break;
      }
    }
//    case RT_int8_tType:
//    case RT_uint8_tType:
    case RT_int32_tType:
    {
      switch(newType) {
      case RT_boolType: setBool(mValue.int32Value?true:false); break;
      case RT_int8_tType: setInt8((int8_t)mValue.int32Value); break;
      case RT_uint8_tType: setUInt8((uint8_t)mValue.int32Value); break;
      case RT_int32_tType: setInt32((int32_t)mValue.int32Value); break;
      case RT_uint32_tType: setUInt32((uint32_t)mValue.int32Value); break;
      case RT_floatType: setFloat((float)mValue.int32Value); break;
      case RT_doubleType: setDouble((double)mValue.int32Value); break;
      case RT_stringType: 
      {
        char buffer[256];
        sprintf(buffer, "%d", mValue.int32Value);
        setString(buffer);
      }
      break;
      case RT_objectType: setObject(NULL); break;
      case RT_functionType: setFunction(NULL); break;
      default:
        rtLogWarn("missed conversion");
        break;
      }
    }
    break;
    case RT_uint32_tType:
    {
      switch(newType) {
      case RT_boolType: setBool(mValue.uint32Value?true:false); break;
      case RT_int8_tType: setInt8((int8_t)mValue.uint32Value); break;
      case RT_uint8_tType: setUInt8((uint8_t)mValue.uint32Value); break;
      case RT_int32_tType: setInt32((int32_t)mValue.uint32Value); break;
      case RT_uint32_tType: setUInt32((uint32_t)mValue.uint32Value); break;
      case RT_floatType: setFloat((float)mValue.uint32Value); break;
      case RT_doubleType: setDouble((double)mValue.uint32Value); break;
      case RT_stringType: 
      {
        char buffer[256];
        sprintf(buffer, "%d", mValue.int32Value);
        setString(buffer);
      }
      break;
      case RT_objectType: setObject(NULL); break;
      case RT_functionType: setFunction(NULL); break;
      default:
        rtLogWarn("missed conversion");
        break;
      }
    }
    break;
    case RT_floatType:
    {
      switch(newType) {
      case RT_boolType: setBool((mValue.floatValue==0.0)?false:true); break;
      case RT_int8_tType: setInt8((int8_t)mValue.floatValue); break;
      case RT_uint8_tType: setUInt8((uint8_t)mValue.floatValue); break;
      case RT_int32_tType: setInt32((int32_t)mValue.floatValue); break;
      case RT_uint32_tType: setUInt32((uint32_t)mValue.floatValue); break;
      case RT_floatType: setFloat((float)mValue.floatValue); break;
      case RT_doubleType: setDouble((double)mValue.floatValue); break;
      case RT_stringType: 
      {
        char buffer[256];
        sprintf(buffer, "%f", mValue.floatValue);
        setString(buffer);
      }
      break;
      case RT_objectType: setObject(NULL); break;
      case RT_functionType: setFunction(NULL); break;
      default:
        rtLogWarn("missed conversion");
        break;
      }
    }
    break;
    case RT_doubleType: 
    {
      switch(newType) {
      case RT_boolType: setBool((mValue.doubleValue==0.0)?false:true); break;
      case RT_int8_tType: setInt8((int8_t)mValue.doubleValue); break;
      case RT_uint8_tType: setUInt8((uint8_t)mValue.doubleValue); break;
      case RT_int32_tType: setInt32((int32_t)mValue.doubleValue); break;
      case RT_uint32_tType: setUInt32((uint32_t)mValue.doubleValue); break;
      case RT_floatType: setFloat((float)mValue.doubleValue); break;
      case RT_doubleType: setDouble((double)mValue.doubleValue); break;
      case RT_stringType: 
      {
        char buffer[256];
        sprintf(buffer, "%lf", mValue.doubleValue);
        setString(buffer);
      }
      break;
      case RT_objectType: setObject(NULL); break;
      case RT_functionType: setFunction(NULL); break;
      default:
        rtLogWarn("missed conversion");
        break;
      }
      }
    break;

    case RT_stringType:
    {
    }
    break;
    case RT_objectType:
    {
    }
    break;
    case 0:
      // Do Nothing
    break;
    default:
      rtLogError("XMissing conversion in coerceType: %c, %d", mType, mType);
      break;
    }
  }
  return RT_OK;
}

