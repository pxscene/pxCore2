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

duk_ret_t proxyWrapperFinalizer(duk_context *ctx)
{
  if (duk_get_prop_string(ctx, 0, "zpointer"))
  {
    rtIObject* p = (rtObject*)duk_require_pointer(ctx,-1);
    if (true == rtDukDelObjectIdent(ctx, p))
    {
      if (p)
        p->Release();
    }

    // in case we get called again
    duk_push_pointer(ctx,NULL);
    duk_put_prop_string(ctx,0,"zpointer");
  }
  return 0;
}

duk_ret_t objectHandleGet(duk_context *ctx)
{
  if (duk_get_prop_string(ctx, 0, "zpointer"))
  {
    rtIObject* p = (rtObject*)duk_require_pointer(ctx,-1);
    if (p)
    {
      duk_get_prop_string(ctx, 0, "isArray");
      bool isArray = duk_require_boolean(ctx, -1);

      rtValue v;
      if (!isArray || duk_is_string(ctx,1))
      {
        const char* key = duk_to_string(ctx, 1);
        if (!strcmp(key,"zpointer"))
        {
          rtString desc;
          rtObjectRef oo = p;
          rtError err = oo.sendReturns<rtString>("description", desc);
          duk_push_pointer(ctx, p);
          return 1;
        }
        else if (p->Get(key, &v) == RT_OK)
        {
          rt2duk(ctx, v);
          return 1;
        }
        else
        {
          // Handle retrieving the dynamic properties which are not part of rtObject
  	  if (duk_get_prop_string(ctx, 0, key))
  	  {
            if (duk_is_null(ctx, -1))
            {
              duk_push_null(ctx);
            }
            else if (duk_is_string(ctx, -1))
            {
              duk_push_string(ctx,duk_get_string(ctx, -1));
            }
            else if (duk_is_boolean(ctx, -1))
            {
                duk_push_boolean(ctx,duk_get_boolean(ctx, -1));
            }
            else if (duk_is_number(ctx, -1))
            {
                duk_push_number(ctx,duk_get_number(ctx, -1));
            }
            else if (duk_is_object(ctx, -1))
            {
      	      duk_dup(ctx,-1);
            }
	    return 1;
          }
        }
        #if 0
        else
          rtLogError("Failed to get property, %s, from rtObject", key);
        #endif
      }
      else
      {
        uint32_t index = duk_to_uint32(ctx, 1);
        if (p->Get(index, &v) == RT_OK)
        {
          rt2duk(ctx, v);
          return 1;
        }
        #if 1
        else
          rtLogError("Failed to get array property, %u, from rtObject", index);
        #endif
      }
    }
  }
  #if 1
  else rtLogError("failed to get rtObjectPointer");
  #endif

  return 0;
}

duk_ret_t objectHandleSet(duk_context *ctx)
{
  if (duk_get_prop_string(ctx, 0, "zpointer"))
  {
    rtIObject* p = (rtObject*)duk_require_pointer(ctx,-1);
    if (p)
    {
      duk_get_prop_string(ctx, 0, "isArray");
      bool isArray = duk_require_boolean(ctx, -1);

      rtValue v = duk2rt(ctx,2);
      if (!isArray || duk_is_string(ctx,1))
      {
        const char* key = duk_to_string(ctx, 1);
        if (p->Set(key, &v) == RT_OK)
        {
          duk_push_boolean(ctx, true);
          return 1;
        }
        else
        {
          rtLogError("Failed to set property, %s, to rtObject", key);
          // Handle setting the dynamic property which are not part of rtObject
          if (duk_is_null(ctx, 2)) 
          { 
            duk_push_null(ctx);
          }
          else if (duk_is_string(ctx, 2)) 
          { 
            duk_push_string(ctx,duk_get_string(ctx, 2));
          }
          else if (duk_is_boolean(ctx, 2))
          { 
              duk_push_boolean(ctx,duk_get_boolean(ctx, 2));
          }
          else if (duk_is_number(ctx, 2)) 
          { 
              duk_push_number(ctx,duk_get_number(ctx, 2));
          }
          else if (duk_is_object(ctx, 2))
          {
	    duk_dup(ctx,2);
          }
          duk_put_prop_string(ctx, 0, key);
          duk_push_boolean(ctx, true);
          return 1;
        }
      }
      else
      {
        uint32_t index = duk_to_uint32(ctx, 1);
        if (p->Set(index, &v) == RT_OK)
        {
          duk_push_boolean(ctx, true);          
          return 1;
        }
        #if 1
        else
          rtLogError("Failed to set array property, %u, from rtObject", index);
        #endif
      }
    }
  }
  #if 1
  else 
    rtLogError("Failed to get rtObject*");
  #endif

  duk_push_boolean(ctx, false);
  return 1;  
}

#if 1
duk_ret_t objectHandleOwnKeys(duk_context *ctx)
{
  if (duk_get_prop_string(ctx, 0, "zpointer"))
  {
    rtIObject* p = (rtObject*)duk_require_pointer(ctx,-1);
    if (p)
    {
      duk_get_prop_string(ctx, 0, "isArray");
      bool isArray = duk_require_boolean(ctx, -1);

      rtValue v;
      if (!isArray || duk_is_string(ctx,1))
      {
        if (p->Get("allKeys", &v) == RT_OK)
        {
          rt2duk(ctx, v);
          return 1;
       
        }
        rt2duk(ctx, v);
        return 1;
      }
      else
      {
        uint32_t index = duk_to_uint32(ctx, 1);
        if (p->Get(index, &v) == RT_OK)
        {
          rt2duk(ctx, v);
          return 1;
        }
        #if 1
        else
          rtLogError("Failed to get array property, %u, from rtObject", index);
        #endif
      }
    }
  }
  #if 1
  else rtLogError("failed to get rtObjectPointer");
  #endif
  return 0;
}
#endif


void pushProxyForObject(duk_context *ctx, const rtObjectRef& o) {

  // Create target object for the proxy.
  duk_idx_t objectIndex = duk_push_object(ctx);
  {
    //duk_push_pointer(ctx, new StringArrayRef(array));
    //duk_put_prop_string(ctx, objectIndex, HIDDEN_PREFIX "data");

    // Store a boolean flag to mark the object as deleted because the destructor may be called several times.
    bool isArray = false;
    rtValue discard;
    if (o && o->Get("length", &discard) != RT_PROP_NOT_FOUND)
    {
      // assume array-like object
      isArray = true;
    }
    duk_push_boolean(ctx, isArray);
    duk_put_prop_string(ctx, objectIndex, /*HIDDEN_PREFIX*/ "isArray");

    duk_push_boolean(ctx, false);
    duk_put_prop_string(ctx, objectIndex, /*HIDDEN_PREFIX*/ "deleted");

    rtIObject* p = o.getPtr();
    p->AddRef();  // Warning this should be released by object finalizer
    duk_push_pointer(ctx, o.getPtr());
    duk_put_prop_string(ctx, objectIndex, "zpointer");

    // Register finalizer.
    duk_push_c_function(ctx, proxyWrapperFinalizer, 1);
    duk_set_finalizer(ctx, objectIndex);
  }

  // Handler that allows us to virtualize the access to the object.
  duk_idx_t handlerIndex = duk_push_object(ctx);
  {
    duk_push_c_lightfunc(ctx, objectHandleGet, 3, 0, 0);
    duk_put_prop_string(ctx, handlerIndex, "get");
    duk_push_c_lightfunc(ctx, objectHandleSet, 4, 0, 0);
    duk_put_prop_string(ctx, handlerIndex, "set");
    #if 1
    // TODO JR enumeration
    duk_push_c_lightfunc(ctx, objectHandleOwnKeys, 1, 0, 0);
    duk_put_prop_string(ctx, handlerIndex, "ownKeys");
    #endif
  }

  duk_idx_t index = duk_push_proxy(ctx, 0);
  rtDukAllocObjectIdent(ctx, o.getPtr(), index);
}

void rtObjectWrapper::createFromObjectReference(duk_context *ctx, const rtObjectRef& ref)
{
  assert(ref.getPtr() != NULL);
  duk_bool_t isPropPresent = rtDukCheckObjectIdent(ctx, ref.getPtr());
  if (isPropPresent)
  {
  }
  else
  {
    // promise
    {
      rtString desc;
      if (ref)
      {
        rtError err = const_cast<rtObjectRef &>(ref).sendReturns<rtString>("description", desc);

        if (err == RT_OK && strcmp(desc.cString(), "rtPromise") == 0)
        {
          duk_bool_t rt = duk_get_global_string(ctx, "constructPromise");
          // [func] 
          assert(rt);

          pushProxyForObject(ctx, ref);

          // [func obj] 

          if (duk_pcall(ctx, 1) != 0) {
            duv_dump_error(ctx, -1);
            assert(0);
          }

          // [js-promise]
          assert(duk_is_object(ctx, -1));

          return;
        }
      }
    }
    pushProxyForObject(ctx, ref);
  }
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
    rtValue val = duk2rt(mDukCtx, -1, &error);
    if (error.hasError()) {
      return RT_FAIL;
    } else {
      rtString key = val.toString();
      if (key.compare(jsObjectWrapper::kIsJavaScriptObjectWrapper) != 0)
      {
        result->pushBack(val);
      }
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

  if (strcmp(name, jsObjectWrapper::kIsJavaScriptObjectWrapper) == 0)
    return RT_OK;

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
  rtValue rt = duk2rt(mDukCtx, -1, error);
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
    rtValue rt = duk2rt(mDukCtx, -1, error);
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
  //AddRef();
}
