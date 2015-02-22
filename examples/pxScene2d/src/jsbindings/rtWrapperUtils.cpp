#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

// #include <rtMutex.h> // non-recusrive

static pthread_mutex_t sSceneLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

void rtWrapperSceneUpdateEnter()
{
  pthread_mutex_lock(&sSceneLock);
}

void rtWrapperSceneUpdateExit()
{
  pthread_mutex_unlock(&sSceneLock);
}

using namespace v8;

Handle<Value> rt2js(const rtValue& v)
{
  switch (v.getType())
  {
    case RT_int32_tType:
      {
        int32_t i = v.toInt32();
        return Integer::New(i);
      }
      break;
    case RT_uint32_tType:
      {
        uint32_t u = v.toUInt32();
        return Integer::NewFromUnsigned(u);
      }
      break;
    case RT_int64_tType:
      {
        double d = v.toDouble();
        return Number::New(d);
      }
      break;
    case RT_floatType:
      {
        float f = v.toFloat();
        return Number::New(f);
      }
      break;
    case RT_doubleType:
      {
        double d = v.toDouble();
        return Number::New(d);
      }
      break;
    case RT_uint64_tType:
      {
        double d = v.toDouble();
        return Number::New(d);
      }
      break;
    case RT_functionType:
      return rtFunctionWrapper::createFromFunctionReference(v.toFunction());
      break;
    case RT_rtObjectRefType:
      return rtObjectWrapper::createFromObjectReference(v.toObject());
      break;
    case RT_boolType:
      return Boolean::New(v.toBool());
      break;
    case RT_rtStringType:
      {
        rtString s = v.toString();
        return String::New(s.cString(), s.length());
      }
      break;
    case RT_voidPtrType:
      rtLogWarn("attempt to convert from void* to JS object");
      return Handle<Value>(); // TODO
      break;
    case 0: // This is really a value rtValue() will set mType to zero
      return Handle<Value>();
      break;
    default:
      rtLogFatal("unsupported rtValue [(char value(%c) int value(%d)] to javascript conversion",
          v.getType(), v.getType());
      break;
  }

  return Undefined();
}

rtValue js2rt(const Handle<Value>& val, rtWrapperError* )
{
  if (val->IsUndefined()) { return rtValue((void *)0); }
  if (val->IsNull())      { return rtValue((char *)0); }
  if (val->IsString())    { return toString(val); }
  if (val->IsArray())     { assert(false); return rtValue(0); } // TODO: rtValue support collections
  if (val->IsFunction())  { return rtValue(rtFunctionRef(new jsFunctionWrapper(val))); }
  if (val->IsObject())
  {
    // This is mostly a heuristic. We should probably set a second internal
    // field and use a uuid as a magic number to ensure this is one of our
    // wrapped objects.
    // It's very possible that someone is trying to use another wrapped object
    // from some other native addon. Maybe that would actuall work and fail
    // at a lower level?
    Local<Object> obj = val->ToObject();
    if (obj->InternalFieldCount() > 0)
    {
      return rtObjectWrapper::unwrapObject(obj);
    }
    else
    {
      // this is a regular JS object. i.e. one that does not wrap an rtObject.
      // in this case, we'll provide the necessary adapter.
      return rtValue(new jsObjectWrapper(obj->ToObject()));
    }
  }

  if (val->IsBoolean())   { return rtValue(val->BooleanValue()); }
  if (val->IsNumber())    { return rtValue(val->NumberValue()); }
  if (val->IsInt32())     { return rtValue(val->Int32Value()); }
  if (val->IsUint32())    { return rtValue(val->Uint32Value()); }

  rtLogFatal("unsupported javascript -> rtValue type conversion");
  return rtValue(0);
}
