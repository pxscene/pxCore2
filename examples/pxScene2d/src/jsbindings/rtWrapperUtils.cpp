#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

#include <rtMutex.h>

static pthread_mutex_t sSceneLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t sCurrentSceneThread;
static rtMutex objectMapMutex;

typedef std::map< rtIObject*, Persistent<Object>* > maptype_rt2v8;

maptype_rt2v8 objectMap_rt2v8;

void weakCallback_rt2v8(const WeakCallbackData<Object, rtIObject>& data)
{
  rtMutexLockGuard lock(objectMapMutex);
  maptype_rt2v8::iterator itr = objectMap_rt2v8.find(data.GetParameter());
  if (itr != objectMap_rt2v8.end())
  {
    Persistent<Object>* p = itr->second;
    // TODO: Removing this temproarily until we understand how this callback works. I
    // would have assumed that this is a weak persistent since we called SetWeak() on it
    // before inserting it into the objectMap_rt2v8 map.
    // assert(p->IsWeak());
    //
    if (!p->IsWeak())
      rtLogWarn("TODO: Why isn't this handle weak?");
    if (p)
    {
      p->Reset();
      delete p;
    }
    objectMap_rt2v8.erase(itr);
  }
}

void HandleMap::addWeakReference(Isolate* isolate, const rtObjectRef& from, Local<Object>& to)
{
  Persistent<Object>* h(new Persistent<Object>(isolate, to));
  h->SetWeak(from.getPtr(), &weakCallback_rt2v8);

  rtMutexLockGuard lock(objectMapMutex);
  objectMap_rt2v8.insert(std::make_pair(from.getPtr(), h));
}

Local<Object> HandleMap::lookupSurrogate(v8::Isolate* isolate, const rtObjectRef& from)
{
  Local<Object> obj;

  rtMutexLockGuard lock(objectMapMutex);
  maptype_rt2v8::iterator itr = objectMap_rt2v8.find(from.getPtr());
  if (itr != objectMap_rt2v8.end())
  {
    Persistent<Object>* p = itr->second;
    if (p)
      obj = PersistentToLocal(isolate, *p);
  }

  return obj;
}

bool rtWrapperSceneUpdateHasLock()
{
  return pthread_self() == sCurrentSceneThread;
}

void rtWrapperSceneUpdateEnter()
{
  pthread_mutex_lock(&sSceneLock);
  sCurrentSceneThread = pthread_self();
}

void rtWrapperSceneUpdateExit()
{
  pthread_mutex_unlock(&sSceneLock);
  sCurrentSceneThread = 0;
}

using namespace v8;

Handle<Value> rt2js(Isolate* isolate, const rtValue& v)
{
  switch (v.getType())
  {
    case RT_int32_tType:
      {
        int32_t i = v.toInt32();
        return Integer::New(isolate, i);
      }
      break;
    case RT_uint32_tType:
      {
        uint32_t u = v.toUInt32();
        return Integer::NewFromUnsigned(isolate, u);
      }
      break;
    case RT_int64_tType:
      {
        double d = v.toDouble();
        return Number::New(isolate, d);
      }
      break;
    case RT_floatType:
      {
        float f = v.toFloat();
        return Number::New(isolate, f);
      }
      break;
    case RT_doubleType:
      {
        double d = v.toDouble();
        return Number::New(isolate, d);
      }
      break;
    case RT_uint64_tType:
      {
        double d = v.toDouble();
        return Number::New(isolate, d);
      }
      break;
    case RT_functionType:
      return rtFunctionWrapper::createFromFunctionReference(isolate, v.toFunction());
      break;
    case RT_rtObjectRefType:
      return jsObjectWrapper::isJavaScriptObjectWrapper(v.toObject())
        ? static_cast<jsObjectWrapper *>(v.toObject().getPtr())->getWrappedObject()
        : rtObjectWrapper::createFromObjectReference(isolate, v.toObject());
      break;
    case RT_boolType:
      return Boolean::New(isolate, v.toBool());
      break;
    case RT_rtStringType:
      {
        rtString s = v.toString();
        return String::NewFromUtf8(isolate, s.cString());
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

  return Undefined(isolate);
}

rtValue js2rt(v8::Isolate* isolate, const Handle<Value>& val, rtWrapperError* )
{
  if (val->IsUndefined()) { return rtValue((void *)0); }
  if (val->IsNull())      { return rtValue((char *)0); }
  if (val->IsString())    { return toString(val); }
  if (val->IsFunction())  { return rtValue(rtFunctionRef(new jsFunctionWrapper(isolate, val))); }
  if (val->IsArray() || val->IsObject())
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
      return rtValue(new jsObjectWrapper(isolate, obj->ToObject(), val->IsArray()));
    }
  }

  if (val->IsBoolean())   { return rtValue(val->BooleanValue()); }
  if (val->IsNumber())    { return rtValue(val->NumberValue()); }
  if (val->IsInt32())     { return rtValue(val->Int32Value()); }
  if (val->IsUint32())    { return rtValue(val->Uint32Value()); }

  rtLogFatal("unsupported javascript -> rtValue type conversion");
  return rtValue(0);
}
