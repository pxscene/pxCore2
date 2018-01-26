#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#include <rtLog.h>
#include <string>
#include <map>

extern "C" {
#include "duv.h"
}

static const char* kClassName   = "rtObject";
static const char* kFuncAllKeys = "allKeys";
static const char* kPropLength = "length";

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
  dukObjectFunctionInfo(void) : mIsVoid(true), mNext(NULL), mType(dukObjectFunctionInfo::eMethod) {}

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

static duk_ret_t dukObjectMethodGetStub(duk_context *ctx)
{
  duk_push_this(ctx);
  bool res = duk_get_prop_string(ctx, -1, "\xff""\xff""data");
  assert(res);

  rtObject *obj = (rtObject*)duk_require_pointer(ctx, -1);

  // [this, pointer]
  duk_pop(ctx);
  // [this]
  duk_pop(ctx);
  // []

  duk_push_current_function(ctx);
  res = duk_get_prop_string(ctx, -1, "\xff""\xff""finfo");
  assert(res);

  dukObjectFunctionInfo *funcInfo = (dukObjectFunctionInfo*)duk_require_pointer(ctx, -1);

  // [curfunc prop]
  duk_pop(ctx);
  // [curfunc]
  duk_pop(ctx);
  // []

  int numArgs = duk_get_top(ctx);

  if (funcInfo->mType == dukObjectFunctionInfo::eGetProp) 
  {
    rtValue val;
    obj->Get(funcInfo->mMethodName.c_str(), &val);
    rt2duk(ctx, val);
    return 1;
  }

  if (funcInfo->mType == dukObjectFunctionInfo::eSetProp) 
  {
    duk_dup(ctx, 0);
    rtValue val = duk2rt(ctx);
    duk_pop(ctx);
    obj->Set(funcInfo->mMethodName.c_str(), &val);
    return 0;
  }

  assert(funcInfo->mType == dukObjectFunctionInfo::eMethod);

  assert(numArgs < 16);
  rtValue args[16];

  for (int i = 0; i < numArgs; ++i) 
  {
    duk_dup(ctx, i);
    args[i] = duk2rt(ctx);
    duk_pop(ctx);
  }

  rtValue func;
  obj->Get(funcInfo->mMethodName.c_str(), &func);

  assert(func.getType() == RT_functionType);
  rtFunctionRef funcObj = func.toFunction();

  rtValue result;
  funcObj->Send(numArgs, &args[0], &result);

  if (funcInfo->mIsVoid) {
    return 0;
  }

  rt2duk(ctx, result);
  return 1;
}


static void wrapObjToDuk(duk_context *ctx, const rtObjectRef& ref)
{
  static std::map<rtMethodMap *, dukObjectFunctionInfo *> methodCache;

  duk_push_object(ctx);

  const_cast<rtObjectRef &>(ref)->AddRef();

  duk_push_pointer(ctx, (void*)ref.getPtr());
  duk_put_prop_string(ctx, -2, "\xff""\xff""data");

  dukObjectFunctionInfo *prevInfo = NULL, *firstInfo = NULL;

  rtMethodMap* mOrig = ref->getMap();

  if (mOrig == NULL) 
  {
    duk_push_pointer(ctx, NULL);
    duk_put_prop_string(ctx, -2, "\xff""\xff""funcInfoList");
    return;
  }

  if (methodCache.count(mOrig) == 0) 
  {
    rtMethodMap* m = mOrig;

    while (m) 
    {
      rtMethodEntry *e = m->getFirstMethod();
      while (e) 
      {
        dukObjectFunctionInfo *funcInfo = new dukObjectFunctionInfo();
        funcInfo->mMethodName = e->mMethodName;
        funcInfo->mIsVoid = e->mReturnType == RT_voidType;
        funcInfo->mType = dukObjectFunctionInfo::eMethod;

        if (prevInfo == NULL) 
        {
          firstInfo = funcInfo;
          prevInfo = funcInfo;
        } 
        else 
        {
          prevInfo->mNext = funcInfo;
          prevInfo = funcInfo;
        }

        e = e->mNext;
      }

      m = m->parentsMap;
    }

    m = mOrig;
    while (m) 
    {
      rtPropertyEntry* e = m->getFirstProperty();
      while (e) 
      {
        if (!e->mGetThunk && !e->mSetThunk) 
        {
          e = e->mNext;
          continue;
        }

        if (e->mGetThunk) 
        {
          dukObjectFunctionInfo *funcInfo = new dukObjectFunctionInfo();
          funcInfo->mMethodName = e->mPropertyName;
          funcInfo->mIsVoid = false;
          funcInfo->mType = dukObjectFunctionInfo::eGetProp;

          if (prevInfo == NULL) 
          {
            firstInfo = funcInfo;
            prevInfo = funcInfo;
          } 
          else 
          {
            prevInfo->mNext = funcInfo;
            prevInfo = funcInfo;
          }
        }

        if (e->mSetThunk) 
        {
          dukObjectFunctionInfo *funcInfo = new dukObjectFunctionInfo();
          funcInfo->mMethodName = e->mPropertyName;
          funcInfo->mIsVoid = false;
          funcInfo->mType = dukObjectFunctionInfo::eSetProp;

          if (prevInfo == NULL) 
          {
            firstInfo = funcInfo;
            prevInfo = funcInfo;
          } 
          else 
          {
            prevInfo->mNext = funcInfo;
            prevInfo = funcInfo;
          }
        }

        e = e->mNext;
      }

      m = m->parentsMap;
    }

    methodCache[mOrig] = firstInfo;
  } 
  else 
  {
    firstInfo = methodCache[mOrig];
  }

  for (prevInfo = firstInfo; prevInfo != NULL; prevInfo = prevInfo->mNext) 
  {
    if (prevInfo->mType == dukObjectFunctionInfo::eMethod)
    {
      duk_push_c_function(ctx, &dukObjectMethodGetStub, DUK_VARARGS);

      duk_push_pointer(ctx, (void*)prevInfo);
      duk_put_prop_string(ctx, -2, "\xff""\xff""finfo");

      // [ obj, func ]
      duk_put_prop_string(ctx, -2, prevInfo->mMethodName.c_str());

      // [ obj ]

      continue;
    }

    {
      duk_bool_t rc = duk_get_prop_string(ctx, -1, prevInfo->mMethodName.c_str());
      duk_pop(ctx);

      if (rc)
      {
        continue;
      }
    }

    duk_push_string(ctx, prevInfo->mMethodName.c_str());

    int offs = 2;
    int flags = 0;

    dukObjectFunctionInfo *infoPtr[2] = { prevInfo, prevInfo->mNext };

    int objFuncIdx = 0;
    for (objFuncIdx = 0; objFuncIdx < 2; ++objFuncIdx) 
    {
      dukObjectFunctionInfo *info = infoPtr[objFuncIdx];
      if (info == NULL || info->mMethodName != prevInfo->mMethodName) 
      {
        break;
      }


      if (info->mType == dukObjectFunctionInfo::eGetProp || info->mType == dukObjectFunctionInfo::eSetProp)
      {
        duk_push_c_function(ctx, &dukObjectMethodGetStub, DUK_VARARGS);

        flags |= (info->mType == dukObjectFunctionInfo::eGetProp ? DUK_DEFPROP_HAVE_GETTER : DUK_DEFPROP_HAVE_SETTER);
        offs++;

        duk_push_pointer(ctx, (void*)info);
        duk_put_prop_string(ctx, -2, "\xff""\xff""finfo");
      }
    }

    if (objFuncIdx == 2) 
    {
      prevInfo = prevInfo->mNext;
    }

    assert(flags != 0);

    // [ obj, name, getter, setter ]
    duk_def_prop(ctx, -offs, flags);

    // [ obj ]
  }

  // [ obj ]

  duk_push_pointer(ctx, (void*)firstInfo);
  duk_put_prop_string(ctx, -2, "\xff""\xff""funcInfoList");
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

#if 0
      if (err == RT_OK && strcmp(desc.cString(), "rtPromise") == 0)
      {
        duk_bool_t rt = duk_get_global_string(ctx, "Promise");

        // [func] 
        assert(rt);

        uint32_t isResolved = 0;
        ref.get("isResolved", isResolved);

        if (isResolved != 0)
        {
          duk_push_string(ctx, isResolved == 1 ? "resolve" : "reject");

          rtValue val;
          ref.get("val", val);

          rt2duk(ctx, val);

          // [ Promise method obj ]
          duk_int_t rc = duk_pcall_prop(ctx, -3, 1);

          if (rc != 0) {
            duv_dump_error(ctx, -1);
            assert(0);
          }

          // [Promise js-promise]
          assert(duk_is_object(ctx, -1));

          return;
        }
      }
#endif

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

        wrapObjToDuk(ctx, ref);

        // [func obj] 

        if (duk_pcall(ctx, 1) != 0) {
          duv_dump_error(ctx, -1);
          assert(0);
        }

#if 1
#if 1
        // [js-promise]
        assert(duk_is_object(ctx, -1));

        duk_dup(ctx, -1);

        // [ obj ]
        std::string promiseObjName = rtDukPutIdentToGlobal(ctx);
        //std::string promiseObjName = "blah";

        const_cast<rtObjectRef &>(ref).set("promiseId", rtString(promiseObjName.c_str()));
       // const_cast<rtObjectRef &>(ref).set("promiseContext", (void*)ctx);
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
  , mDukCtx(ctx)
  , mDukName(name)
  , mIsArray(isArray)
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
