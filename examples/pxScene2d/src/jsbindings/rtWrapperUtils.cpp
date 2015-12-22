#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"

#include <rtMutex.h>

#ifdef __APPLE__
static pthread_mutex_t sSceneLock = PTHREAD_MUTEX_INITIALIZER;
static pthread_t sCurrentSceneThread;
#elif defined(USE_STD_THREADS)
#include <thread>
#include <mutex>
static std::mutex sSceneLock;
static std::thread::id sCurrentSceneThread;
#else
static pthread_mutex_t sSceneLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_t sCurrentSceneThread;
#endif

static int sLockCount;

static rtMutex objectMapMutex;

struct ObjectReference
{
  Persistent<Object>* v8;
  rtObjectRef         rt;
};

//typedef std::map< rtIObject*, Persistent<Object>* > maptype_rt2v8;
typedef std::map< rtIObject*, ObjectReference > maptype_rt2v8;

maptype_rt2v8 objectMap_rt2v8;

void weakCallback_rt2v8(const WeakCallbackData<Object, rtIObject>& data)
{
  rtMutexLockGuard lock(objectMapMutex);
  maptype_rt2v8::iterator itr = objectMap_rt2v8.find(data.GetParameter());
  if (itr != objectMap_rt2v8.end())
  {
    Persistent<Object>* p = itr->second.v8;
    // TODO: Removing this temproarily until we understand how this callback works. I
    // would have assumed that this is a weak persistent since we called SetWeak() on it
    // before inserting it into the objectMap_rt2v8 map.
    // assert(p->IsWeak());
    //
// Jake says that this situation is ok...
#if 0
    if (!p->IsWeak())
      rtLogWarn("TODO: Why isn't this handle weak?");
#endif
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
  rtMutexLockGuard lock(objectMapMutex);

  ObjectReference entry;
  entry.v8 = new Persistent<Object>(isolate, to);
  entry.v8->SetWeak(from.getPtr(), &weakCallback_rt2v8);
  entry.rt = from;

  std::pair< maptype_rt2v8::iterator, bool > ret =
    objectMap_rt2v8.insert(std::make_pair(from.getPtr(), entry));

  if (!ret.second)
  {
    entry.v8->Reset();
    delete entry.v8;
  }

  #if 0
  static FILE* f = NULL;
  if (!f)
    f = fopen("/tmp/handles.txt", "w+");
  if (f)
  {
    rtString desc;
    const_cast<rtObjectRef &>(from).sendReturns<rtString>("description", desc);
    fprintf(f, "%p (%s) => %p\n", from.getPtr(), desc.cString(), h);
  }
  #endif
}

Local<Object> HandleMap::lookupSurrogate(v8::Isolate* isolate, const rtObjectRef& from)
{
  Local<Object> obj;
  EscapableHandleScope scope(isolate);

  Persistent<Object>* p = NULL;

  rtMutexLockGuard lock(objectMapMutex);
  maptype_rt2v8::iterator itr = objectMap_rt2v8.find(from.getPtr());
  if (itr != objectMap_rt2v8.end())
  {
    p = itr->second.v8;
    if (p)
      obj = PersistentToLocal(isolate, *p);
  }

  #if 1
  if (!obj.IsEmpty())
  {
    // JR sanity check
    if ((from.getPtr() != NULL) && (from.get<rtFunctionRef>("animateTo") != NULL) &&
        (!obj->Has(v8::String::NewFromUtf8(isolate,"animateTo"))))
    {
      // TODO change description to a property
      rtString desc;
      const_cast<rtObjectRef &>(from).sendReturns<rtString>("description", desc);
      printf("type mismatch in handle map %p (%s) != %p\n", from.getPtr(), desc.cString(), p);
      assert(false);
    }
  }
  #endif

  return scope.Escape(obj);
}

bool rtIsPromise(const rtValue& v)
{
  if (v.getType() != RT_rtObjectRefType)
    return false;

  rtObjectRef ref = v.toObject();
  if (!ref)
    return false;

  rtString desc;
  rtError err = ref.sendReturns<rtString>("description", desc);
  if (err != RT_OK)
    return false;

  return strcmp(desc.cString(), "rtPromise") == 0;
}

bool rtWrapperSceneUpdateHasLock()
{
#ifdef USE_STD_THREADS
  return std::this_thread::get_id() == sCurrentSceneThread;
#else
  return pthread_self() == sCurrentSceneThread;
#endif
}

void rtWrapperSceneUpdateEnter()
{
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(sSceneLock);
  sCurrentSceneThread = std::this_thread::get_id();
#else
  assert(pthread_mutex_lock(&sSceneLock) == 0);
  sCurrentSceneThread = pthread_self();
#endif
  sLockCount++;
}

void rtWrapperSceneUpdateExit()
{
#ifndef RT_USE_SINGLE_RENDER_THREAD
  assert(rtWrapperSceneUpdateHasLock());
#endif //RT_USE_SINGLE_RENDER_THREAD

  sLockCount--;
#ifdef USE_STD_THREADS
  if (sLockCount == 0)
    sCurrentSceneThread = std::thread::id()
#else
  if (sLockCount == 0)
    sCurrentSceneThread = 0;
#endif

#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(sSceneLock);
#else
  assert(pthread_mutex_unlock(&sSceneLock) == 0);
#endif
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
