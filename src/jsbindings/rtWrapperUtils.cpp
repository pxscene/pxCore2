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
    sCurrentSceneThread = std::thread::id();
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

void rt2duk(duk_context *ctx, const rtValue& v)
{
  switch (v.getType())
  {
  case RT_int32_tType:
  {
    int32_t i = v.toInt32();
    duk_push_int(ctx, i);
  }
  break;
  case RT_uint32_tType:
  {
    uint32_t u = v.toUInt32();
    duk_push_uint(ctx, u);
  }
  break;
  case RT_int64_tType:
  {
    double d = v.toDouble();
    duk_push_number(ctx, d);
  }
  break;
  case RT_floatType:
  {
    float f = v.toFloat();
    duk_push_number(ctx, f);
  }
  break;
  case RT_doubleType:
  {
    double d = v.toDouble();
    duk_push_number(ctx, d);
  }
  break;
  case RT_uint64_tType:
  {
    double d = v.toDouble();
    duk_push_number(ctx, d);
  }
  break;
  case RT_functionType:
  {
    rtFunctionRef func = v.toFunction();
    if (!func) {
      duk_push_null(ctx);
      return;
    }
    rtFunctionWrapper::createFromFunctionReference(ctx, func);
  }
  break;
  case RT_rtObjectRefType:
  {
    rtObjectRef obj = v.toObject();
    if (!obj) {
      duk_push_null(ctx);
      return;
    }

    if (jsObjectWrapper::isJavaScriptObjectWrapper(obj)) {
      static_cast<jsObjectWrapper *>(obj.getPtr())->pushDukWrappedObject();
    } else {
      rtObjectWrapper::createFromObjectReference(ctx, obj);
    }
  }
  break;
  case RT_boolType:
  {
    bool b = v.toBool();
    duk_push_boolean(ctx, b);
    break;
  }
  case RT_rtStringType:
  {
    rtString s = v.toString();
    duk_push_string(ctx, s.cString());
  }
  break;
  case RT_voidPtrType:
    rtLogWarn("attempt to convert from void* to JS object");
    assert(0);
    break;
  case RT_voidType: // This is really a value rtValue() will set mType to zero
    duk_push_null(ctx);
    break;
  default:
    rtLogFatal("unsupported rtValue [(char value(%c) int value(%d)] to javascript conversion",
      v.getType(), v.getType());
    break;
  }
}


rtValue duk2rt(duk_context *ctx, rtWrapperError* error)
{
  if (duk_is_undefined(ctx, -1)) { return rtValue((void *)0); }
  if (duk_is_null(ctx, -1)) { return rtValue((char *)0); }
  if (duk_is_string(ctx, -1)) { return rtValue(duk_get_string(ctx, -1)); }
  if (duk_is_c_function(ctx, -1)) {
    duk_bool_t res = duk_get_prop_string(ctx, -1, "\xff""\xff""data");
    assert(res);
    rtIFunction *func = (rtIFunction*)duk_require_pointer(ctx, -1);
    duk_pop(ctx);
    return rtValue(rtFunctionRef(func));
  }
  if (duk_is_function(ctx, -1)) {
    duk_dup(ctx, -1);
    std::string id = rtDukPutIdentToGlobal(ctx);
    return rtValue(rtFunctionRef(new jsFunctionWrapper(ctx, id)));
  }
  if (duk_is_object(ctx, -1)) {
    duk_bool_t res = duk_get_prop_string(ctx, -1, "\xff""\xff""data");

    if (res) {
      rtIObject *obj = (rtIObject*)duk_require_pointer(ctx, -1);
      duk_pop(ctx);
      return rtValue(obj);
    }

    duk_pop(ctx);

    duk_dup(ctx, -1);
    std::string id = rtDukPutIdentToGlobal(ctx);

    bool isArray = duk_is_array(ctx, -1);
    return rtValue(new jsObjectWrapper(ctx, id, isArray));
  }

  if (duk_is_boolean(ctx, -1)) { return rtValue(duk_get_boolean(ctx, -1)); }
  if (duk_is_number(ctx, -1)) { return rtValue(duk_get_number(ctx, -1)); }

  rtLogFatal("unsupported javascript -> rtValue type conversion");
  return rtValue(0);
}



std::string rtAllocDukIdentId()
{
  static int dukIdentId = 0;
  char buf[128];
  sprintf(buf, "__ident%05d", dukIdentId++);
  return buf;
}


std::string rtDukPutIdentToGlobal(duk_context *ctx, const std::string &name)
{
  std::string id = name;
  if (id.empty()) {
    id = rtAllocDukIdentId();
  }
  duk_bool_t rc = duk_put_global_string(ctx, id.c_str());
  assert(rc);
  return id;
}