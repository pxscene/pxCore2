#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#include <rtLog.h>
#include <string>
#include <map>

extern "C" {
#include "duv.h"
}

static const char* kFuncAllKeys = "allKeys";

const char* jsObjectWrapper::kIsJavaScriptObjectWrapper = "8907a0a6-ef86-4c3d-aea1-c40c0aa2f6f0";

bool jsObjectWrapper::isJavaScriptObjectWrapper(const rtObjectRef& obj)
{
  rtValue value;
  return obj && obj->Get(jsObjectWrapper::kIsJavaScriptObjectWrapper, &value) == RT_OK;
}

rtObjectWrapper::rtObjectWrapper(const rtObjectRef& ref)
  : rtWrapper(ref)
{
}

rtObjectWrapper::~rtObjectWrapper()
{
}

struct dukObjectFunctionInfo
{
  dukObjectFunctionInfo(void) : mIsVoid(true), mType(dukObjectFunctionInfo::eMethod), mNext(NULL) {}

  std::string mMethodName;
  bool        mIsVoid;

  enum eType {
    eMethod = 0,
    eGetProp = 1,
    eSetProp = 2,
  };

  eType       mType;

  dukObjectFunctionInfo *mNext;
};

static duk_ret_t rtObjectFinalizerProc(duk_context *ctx)
{
  bool res = duk_get_prop_string(ctx, -1, "\xff""\xff""data");
  assert(res);

  rtIObject *obj = (rtIObject*)duk_require_pointer(ctx, -1);
  obj->Release();

  duk_pop(ctx);

  return 0;
}

static void wrapObjToDuk(duk_context *ctx, const rtObjectRef& ref)
{
  duk_push_object(ctx);

  const_cast<rtObjectRef &>(ref)->AddRef();

  void *ptr = (void*)ref.getPtr();
  duk_push_pointer(ctx, ptr);
  duk_put_prop_string(ctx, -2, "\xff""\xff""data");

  duk_push_c_function(ctx, &rtObjectFinalizerProc, 1);
  duk_set_finalizer(ctx, -2);
}

void rtObjectWrapper::createFromObjectReference(duk_context *ctx, const rtObjectRef& ref)
{
  assert(ref.getPtr() != NULL);

  // array
  {
    rtValue length;
    if (ref && ref->Get("length", &length) != RT_PROP_NOT_FOUND)
    {
      duk_bool_t rt = duk_get_global_string(ctx, "constructProxy");
      // [func] 
      assert(rt);

      wrapObjToDuk(ctx, ref);
    
      if (duk_pcall(ctx, 1) != 0) {
        duv_dump_error(ctx, -1);
        assert(0);
      }
    
      assert(duk_is_object(ctx, -1));

      return;
    }
  }

  // map
  if (ref)
  {
    rtString desc;
    rtError err = const_cast<rtObjectRef &>(ref).sendReturns<rtString>("description", desc);
    if (err == RT_OK && strcmp(desc.cString(), "rtMapObject") == 0)
    {
      duk_bool_t rt = duk_get_global_string(ctx, "constructProxy");
      // [func] 
      assert(rt);

      wrapObjToDuk(ctx, ref);
    
      if (duk_pcall(ctx, 1) != 0) {
        duv_dump_error(ctx, -1);
        assert(0);
      }
    
      assert(duk_is_object(ctx, -1));

      return;
    }
  }
   
  // promise
  {
    rtString desc;
    if (ref)
    {
      rtError err = const_cast<rtObjectRef &>(ref).sendReturns<rtString>("description", desc);

      if (err == RT_OK && strcmp(desc.cString(), "rtPromise") == 0)
      {
        #if 1
        rtString val;
        ref.get("promiseId", val);

        if (!val.isEmpty()) {
          duk_bool_t rt = duk_get_global_string(ctx, val.cString());

          assert(rt);

          // [js-promise]
          assert(duk_is_object(ctx, -1));
          return;
        }
        #else
        if (!ref.get<rtString>("promiseId").isEmpty())
          return;
        #endif

        duk_bool_t rt = duk_get_global_string(ctx, "constructPromise");
        // [func]
        assert(rt);

        rt = duk_get_global_string(ctx, "constructProxy");
        // [func] 
        assert(rt);

        wrapObjToDuk(ctx, ref);

        if (duk_pcall(ctx, 1) != 0) {
          duv_dump_error(ctx, -1);
          assert(0);
        }

        assert(duk_is_object(ctx, -1));

        // [func obj] 

        if (duk_pcall(ctx, 1) != 0) {
          duv_dump_error(ctx, -1);
          assert(0);
        }

#if 1
#if 1
        // [js-promise]
        assert(duk_is_object(ctx, -1));

#else
        const_cast<rtObjectRef &>(ref).set("promiseId", "blah");
#endif
#endif
        return;
      }
    }
  }

  duk_bool_t rt = duk_get_global_string(ctx, "constructProxy");
  // [func] 
  assert(rt);

  wrapObjToDuk(ctx, ref);

  if (duk_pcall(ctx, 1) != 0) {
    duv_dump_error(ctx, -1);
    assert(0);
  }

  assert(duk_is_object(ctx, -1));
}

jsObjectWrapper::jsObjectWrapper(duk_context *ctx, const std::string &name, bool isArray)
  : mRefCount(0)
  , mIsArray(isArray)
  , mDukCtx(ctx)
  , mDukName(name)
{
  duk_bool_t rt = duk_get_global_string(ctx, name.c_str());
  assert(rt);

  duk_push_int(ctx, 1);
  duk_put_prop_string(ctx, -2, jsObjectWrapper::kIsJavaScriptObjectWrapper);

  duk_pop(ctx);
}

jsObjectWrapper::~jsObjectWrapper()
{
  if (!mDukName.empty()) {
    rtDukDelGlobalIdent(mDukCtx, mDukName);
  }
}

unsigned long jsObjectWrapper::AddRef()
{
  return rtAtomicInc(&mRefCount);
}

unsigned long jsObjectWrapper::Release()
{
  unsigned long l = rtAtomicDec(&mRefCount);
  if (l == 0) {
    delete this;
  }
  return l;
}

rtError jsObjectWrapper::getAllKeys(rtValue* value) const
{
  duk_get_global_string(mDukCtx, "Object");
  duk_push_string(mDukCtx, "keys");
  duk_bool_t res = duk_get_global_string(mDukCtx, mDukName.c_str());
  assert(res);

  duk_call_prop(mDukCtx, -3, 1);

  int n = duk_get_length(mDukCtx, -1);

  rtRefT<rtArrayObject> result(new rtArrayObject);
  for (int i = 0; i < n; i++) {
    duk_get_prop_index(mDukCtx, -1, i);
    rtWrapperError error;
    rtValue val = duk2rt(mDukCtx, &error);
    if (error.hasError()) {
      return RT_FAIL;
    } else {
      result->pushBack(val);
    }
    duk_pop(mDukCtx);
  }

  duk_pop(mDukCtx);

  *value = rtValue(result);
  return RT_OK;
}

rtError jsObjectWrapper::Get(const char* name, rtValue* value) const
{
  if (!name)
    return RT_ERROR_INVALID_ARG;
  if (!value)
    return RT_ERROR_INVALID_ARG;

  // TODO: does array support this?
  if (strcmp(name, kFuncAllKeys) == 0) {
    return getAllKeys(value);
  }

  rtError err = RT_OK;

#if 0
  if (mIsArray)
  {
    // unsupported yet
    assert(0);
  }
  else
#endif
  {
    // TODO perf warning do we really have to do Has before a Get?
    if (!dukHasProp(name)) {
      err = RT_PROPERTY_NOT_FOUND;
    } else {
      rtWrapperError error;
      *value = dukGetProp(name);
      if (error.hasError())
        err = RT_ERROR_INVALID_ARG;
    }
  }
  return err;
}

bool jsObjectWrapper::dukHasProp(const std::string &name) const
{
  assert(mDukCtx != NULL);
  duk_bool_t res = duk_get_global_string(mDukCtx, mDukName.c_str());
  assert(res);
  res = duk_get_prop_string(mDukCtx, -1, name.c_str());
  duk_pop(mDukCtx);
  duk_pop(mDukCtx);
  return res;
}

rtValue jsObjectWrapper::dukGetProp(const std::string &name, rtWrapperError *error) const
{
  assert(mDukCtx != NULL);
  duk_bool_t res = duk_get_global_string(mDukCtx, mDukName.c_str());
  assert(res);
  res = duk_get_prop_string(mDukCtx, -1, name.c_str());
  assert(res);
  rtValue rt = duk2rt(mDukCtx, error);
  duk_pop(mDukCtx);
  duk_pop(mDukCtx);
  return rt;
}

rtValue jsObjectWrapper::dukGetProp(uint32_t i, rtWrapperError *error) const
{
    assert(mDukCtx != NULL);
    duk_bool_t res = duk_get_global_string(mDukCtx, mDukName.c_str());
    assert(res);
    //res = duk_get_prop_string(mDukCtx, -1, name.c_str());
    res = duk_get_prop_index(mDukCtx, -1, i);
    assert(res);
    rtValue rt = duk2rt(mDukCtx, error);
    duk_pop(mDukCtx);
    duk_pop(mDukCtx);
    return rt;
}

rtError jsObjectWrapper::Get(uint32_t i, rtValue* value) const
{
  // unsupported yet
  //assert(0);
  // TODO no error propogation..
  *value = dukGetProp(i);
  return RT_OK;
}

rtError jsObjectWrapper::Set(const char* name, const rtValue* value)
{
  // unsupported yet
  assert(0);

  return RT_OK;
}

rtError jsObjectWrapper::Set(uint32_t i, const rtValue* value)
{
  // unsupported yet
  assert(0);

  return RT_OK;
}

void jsObjectWrapper::pushDukWrappedObject()
{
  duk_bool_t res = duk_get_global_string(mDukCtx, mDukName.c_str());
  assert(res);

  AddRef();
}
