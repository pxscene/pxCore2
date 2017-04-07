#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#ifndef RUNINMAIN
extern uv_mutex_t threadMutex;
#endif
#include <rtMutex.h>

#ifdef __APPLE__
static pthread_mutex_t sSceneLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER; //PTHREAD_MUTEX_INITIALIZER;
static pthread_t sCurrentSceneThread;
#ifndef RUNINMAIN
static pthread_mutex_t sObjectMapMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER; //PTHREAD_MUTEX_INITIALIZER;
#endif //!RUNINMAIN
#elif defined(USE_STD_THREADS)
#include <thread>
#include <mutex>
static std::mutex sSceneLock;
static std::thread::id sCurrentSceneThread;
#else
static pthread_mutex_t sSceneLock = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
static pthread_t sCurrentSceneThread;
#ifndef RUNINMAIN
static pthread_mutex_t sObjectMapMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
#endif //!RUNINMAIN
#endif

using namespace std;

static int sLockCount;

const int HandleMap::kContextIdIndex = 2;

struct ObjectReference
{
  rtObjectRef        RTObject;
  Persistent<Object> PersistentObject;
  uint32_t           CreationContextId;
};

typedef std::map< rtIObject*, ObjectReference*  > ObjectReferenceMap;
static ObjectReferenceMap objectMap;

uint32_t
GetContextId(Local<Context>& ctx)
{
  if (ctx.IsEmpty()) return 0;

  HandleScope handleScope(ctx->GetIsolate());
  Local<Value> val = ctx->GetEmbedderData(HandleMap::kContextIdIndex);
  assert(!val.IsEmpty());
  return val->Uint32Value();
}
#ifdef ENABLE_NODE_V_6_9
static void WeakCallback(const WeakCallbackInfo<rtIObject>& data) {
  Locker locker(data.GetIsolate());
  Isolate::Scope isolateScope(data.GetIsolate());
  HandleScope handleScope(data.GetIsolate());
#ifndef RUNINMAIN
  rtObjectRef temp;
#endif
rtWrapperSceneUpdateEnter();
#ifndef RUNINMAIN
  pthread_mutex_lock(&sObjectMapMutex);
#endif
  ObjectReferenceMap::iterator j = objectMap.find(data.GetParameter());
  if (j != objectMap.end())
  {
    // TODO: Removing this temporarily until we understand how this callback works. I
    // would have assumed that this is a weak persistent since we called SetWeak() on it
    // before inserting it into the objectMap_rt2v8 map.
    // assert(p->IsWeak());
    //
    j->second->PersistentObject.ClearWeak();
    j->second->PersistentObject.Reset();
#ifndef RUNINMAIN
    temp = j->second->RTObject;
#endif
    delete j->second;
    objectMap.erase(j);
  }
  else
  {
    rtLogWarn("failed to find:%p in map", data.GetParameter());
  }
rtWrapperSceneUpdateExit();
#ifndef RUNINMAIN
  pthread_mutex_unlock(&sObjectMapMutex);
  rtObjectRef parentRef;
  rtError err = temp.get<rtObjectRef>("parent",parentRef);
  if (err == RT_OK)
  {
    if (NULL == parentRef)
    {
      temp.send("dispose");
    }
  }
#endif
}
#else
void weakCallback_rt2v8(const WeakCallbackData<Object, rtIObject>& data)
{
  Locker locker(data.GetIsolate());
  Isolate::Scope isolateScope(data.GetIsolate());
  HandleScope handleScope(data.GetIsolate());
#ifndef RUNINMAIN
  rtObjectRef temp;
#endif
  // rtLogInfo("ptr: %p", data.GetParameter());

  Local<Object> obj = data.GetValue();
  assert(!obj.IsEmpty());

  Local<Context> ctx = obj->CreationContext();
  assert(!ctx.IsEmpty());

  #if 0
  Local<String> s = obj->GetConstructorName();
  String::Utf8Value v(s);
  rtLogInfo("OBJ: %s", *v);

  int index = 0;
  rtIObject* rt = data.GetParameter();
  rtValue value;
  while (rt->Get(index++, &value) == RT_OK)
  {
    rtLogInfo("  type:%d", value.getType());
  }
  #endif

  // uint32_t contextId = GetContextId(ctx);
  // rtLogInfo("contextId: %u addr:%p", contextId, data.GetParameter());
rtWrapperSceneUpdateEnter();
#ifndef RUNINMAIN
  pthread_mutex_lock(&sObjectMapMutex);
#endif
  ObjectReferenceMap::iterator j = objectMap.find(data.GetParameter());
  if (j != objectMap.end())
  {
    // TODO: Removing this temporarily until we understand how this callback works. I
    // would have assumed that this is a weak persistent since we called SetWeak() on it
    // before inserting it into the objectMap_rt2v8 map.
    // assert(p->IsWeak());
    //
    j->second->PersistentObject.ClearWeak();
    j->second->PersistentObject.Reset();
#ifndef RUNINMAIN
    temp = j->second->RTObject;
#endif
#if 0
    if (!p->IsWeak())
      rtLogWarn("TODO: Why isn't this handle weak?");
    if (p)
    {
      // delete p;
    }
    else
    {
      rtLogError("null handle in map");
    }
#endif
    delete j->second;
    objectMap.erase(j);
  }
  else
  {
    rtLogWarn("failed to find:%p in map", data.GetParameter());
  }
rtWrapperSceneUpdateExit();
#ifndef RUNINMAIN
  pthread_mutex_unlock(&sObjectMapMutex);
  rtObjectRef parentRef;
  rtError err = temp.get<rtObjectRef>("parent",parentRef);
  if (err == RT_OK)
  {
    if (NULL == parentRef)
    {
      temp.send("dispose");
    }
  }
#endif
}
#endif

void
HandleMap::clearAllForContext(uint32_t contextId)
{
  typedef ObjectReferenceMap::iterator iterator;

  int n = 0;
rtWrapperSceneUpdateEnter();
#ifndef RUNINMAIN
  pthread_mutex_lock(&sObjectMapMutex);
#endif
  rtLogInfo("clearing all persistent handles for: %u size:%u", contextId,
    static_cast<unsigned>(objectMap.size()));
  vector<iterator> refs;
  for (iterator begin = objectMap.begin(), end = objectMap.end(); begin != end;)
  {
      ObjectReference* ref = begin->second;
      //rtLogInfo("looking at:%d == %d", ref->CreationContextId, contextId);

      if (ref->CreationContextId == contextId)
      {
        refs.push_back(begin);
        ref->PersistentObject.ClearWeak();
        ref->PersistentObject.Reset();
        delete ref;
        ref = NULL;
        begin++;
        n++;
      }
      else
      {
        //rtLogInfo("looping :%d == %d", ref->CreationContextId, contextId);
        ++begin;
      }
  }

  for(unsigned int i=0; i<refs.size();i++)
  {
    objectMap.erase(refs[i]);
  }
  refs.clear();
  //rtLogInfo("clear complete for id[%d] . removed:%d size:%u", contextId, n,
      //static_cast<unsigned>(objectMap.size()));
rtWrapperSceneUpdateExit();
#ifndef RUNINMAIN
  pthread_mutex_unlock(&sObjectMapMutex);
#endif
}

void HandleMap::addWeakReference(v8::Isolate* isolate, const rtObjectRef& from, Local<Object>& to)
{
  HandleScope handleScope(isolate);
  Local<Context> creationContext = to->CreationContext();

  uint32_t const contextIdCreation = GetContextId(creationContext);
  assert(contextIdCreation != 0);
rtWrapperSceneUpdateEnter();
#ifndef RUNINMAIN
  pthread_mutex_lock(&sObjectMapMutex);
#endif
  ObjectReferenceMap::iterator i = objectMap.find(from.getPtr());
  assert(i == objectMap.end());

  if (i == objectMap.end())
  {
    // rtLogInfo("add id:%u addr:%p", contextIdCreation, from.getPtr());
    ObjectReference* entry(new ObjectReference());
    entry->PersistentObject.Reset(isolate, to);
#ifdef ENABLE_NODE_V_6_9
    entry->PersistentObject.SetWeak(from.getPtr(), WeakCallback, v8::WeakCallbackType::kParameter);
#else
    entry->PersistentObject.SetWeak(from.getPtr(), &weakCallback_rt2v8);
#endif
    entry->RTObject = from;
    entry->CreationContextId = contextIdCreation;
    objectMap.insert(std::make_pair(from.getPtr(), entry));
  }
rtWrapperSceneUpdateExit(); 
#ifndef RUNINMAIN 
  pthread_mutex_unlock(&sObjectMapMutex);
#endif

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

Local<Object> HandleMap::lookupSurrogate(v8::Local<v8::Context>& ctx, const rtObjectRef& from)
{
  Isolate* isolate = ctx->GetIsolate();
  EscapableHandleScope scope(isolate);
  Local<Object> obj;
rtWrapperSceneUpdateEnter();
#ifndef RUNINMAIN
  pthread_mutex_lock(&sObjectMapMutex);
#endif
  ObjectReferenceMap::iterator i = objectMap.find(from.getPtr());
  if (i == objectMap.end())
  {
    rtWrapperSceneUpdateExit();
#ifndef RUNINMAIN
    pthread_mutex_unlock(&sObjectMapMutex);
#endif
    return scope.Escape(obj);
  }
  obj = PersistentToLocal(isolate, i->second->PersistentObject);
rtWrapperSceneUpdateExit();
#ifndef RUNINMAIN
  pthread_mutex_unlock(&sObjectMapMutex);
#endif

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
      rtLogError("type mismatch in handle map %p (%s)", from.getPtr(), desc.cString());
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
  return pthread_equal(pthread_self(),sCurrentSceneThread);
#endif
}

void rtWrapperSceneUpdateEnter()
{
#ifndef RUNINMAIN
  //printf("rtWrapperSceneUpdateEnter() pthread_self= %x\n",pthread_self());
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(sSceneLock);
  sCurrentSceneThread = std::this_thread::get_id();
  sLockCount++;
#else
  //assert(pthread_mutex_lock(&sSceneLock) == 0);
  //sCurrentSceneThread = pthread_self();
  // Main thread is now NOT the node thread
  if(rtIsMainThread())
  {
    // Check if this thread already has a lock 
    if(rtWrapperSceneUpdateHasLock()) 
    {
      //printf("increment lock count only\n");
      sLockCount++;
    } 
    else 
    {
      //printf("rtWrapperSceneUpdateEnter locking\n");
      uv_mutex_lock(&threadMutex);
      //printf("rtWrapperSceneUpdateEnter GOT LOCK!!!\n");
      sCurrentSceneThread = pthread_self();
      sLockCount++;
    }
  }
#endif //USE_STD_THREADS

#else // RUNINMAIN
#ifdef USE_STD_THREADS
  std::unique_lock<std::mutex> lock(sSceneLock);
  sCurrentSceneThread = std::this_thread::get_id();
#else
  assert(pthread_mutex_lock(&sSceneLock) == 0);
  sCurrentSceneThread = pthread_self();
#endif
  sLockCount++;
#endif // RUNINMAIN
  
}

void rtWrapperSceneUpdateExit()
{ 
#ifndef RUNINMAIN
  //printf("rtWrapperSceneUpdateExit() pthread_self= %x\n",pthread_self());
  if(rtIsMainThread()) {
 
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
  //assert(pthread_mutex_unlock(&sSceneLock) == 0);
  // Main thread is now NOT the node thread
  if (sLockCount == 0) {
    //printf("rtWrapperSceneUpdateExit unlocking\n");
    uv_mutex_unlock(&threadMutex);
  }

#endif
  }

#else //RUNINMAIN
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
#endif // RUNINMAIN
}

using namespace v8;

Handle<Value> rt2js(Local<Context>& ctx, const rtValue& v)
{
  Context::Scope contextScope(ctx);

  Isolate* isolate = ctx->GetIsolate();

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
#ifdef ENABLE_NODE_V_6_9
      return rtFunctionWrapper::createFromFunctionReference(ctx, isolate, v.toFunction());
#else
      return rtFunctionWrapper::createFromFunctionReference(isolate, v.toFunction());
#endif
      break;
    case RT_rtObjectRefType:
      return jsObjectWrapper::isJavaScriptObjectWrapper(v.toObject())
        ? static_cast<jsObjectWrapper *>(v.toObject().getPtr())->getWrappedObject()
        : rtObjectWrapper::createFromObjectReference(ctx, v.toObject());
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

rtValue js2rt(v8::Local<v8::Context>& ctx, const Handle<Value>& val, rtWrapperError* )
{
  v8::Isolate* isolate = ctx->GetIsolate();

  if (val->IsUndefined()) { return rtValue((void *)0); }
  if (val->IsNull())      { return rtValue((char *)0); }
  if (val->IsString())    { return toString(val); }
  if (val->IsFunction())  { return rtValue(rtFunctionRef(new jsFunctionWrapper(ctx, val))); }
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
