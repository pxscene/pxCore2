#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#include "rtWrapperUtils.h"

#include <rtLog.h>

using namespace v8;

static const char* kClassName   = "rtObject";
static const char* kFuncAllKeys = "allKeys";
static const char* kPropLength = "length";

const char* jsObjectWrapper::kIsJavaScriptObjectWrapper = "8907a0a6-ef86-4c3d-aea1-c40c0aa2f6f0";

bool jsObjectWrapper::isJavaScriptObjectWrapper(const rtObjectRef& obj)
{
  rtValue value;
  return obj && obj->Get(jsObjectWrapper::kIsJavaScriptObjectWrapper, &value) == RT_OK;
}

static Handle<Value> makeStringFromKey(Isolate* isolate, rtObjectRef& keys, uint32_t index)
{
  return String::NewFromUtf8(isolate, keys.get<rtString>(index).cString());
}

static Handle<Value> makeIntegerFromKey(Isolate* isolate, rtObjectRef& , uint32_t index)
{
  return Number::New(isolate, index);
}

static Persistent<Function> ctor;

rtObjectWrapper::rtObjectWrapper(const rtObjectRef& ref)
  : rtWrapper(ref)
{
}

rtObjectWrapper::~rtObjectWrapper()
{
}

void rtObjectWrapper::destroyPrototype()
{
  if( !ctor.IsEmpty() )
  {
    // TODO: THIS LEAKS... need to free obj within persistent

    ctor.ClearWeak();
    ctor.Reset();
  }
}

void rtObjectWrapper::exportPrototype(Isolate* isolate, Handle<Object> exports)
{
  Local<FunctionTemplate> tmpl = FunctionTemplate::New(isolate, create);
  tmpl->SetClassName(String::NewFromUtf8(isolate, kClassName));

  Local<ObjectTemplate> inst = tmpl->InstanceTemplate();
  inst->SetInternalFieldCount(1);
  inst->SetNamedPropertyHandler(
      &getPropertyByName,
      &setPropertyByName,
      NULL,
      NULL,
      &getEnumerablePropertyNames);

  inst->SetIndexedPropertyHandler(
      &getPropertyByIndex,
      &setPropertyByIndex,
      NULL,
      NULL,
      &getEnumerablePropertyIndecies);

  ctor.Reset(isolate, tmpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, kClassName), tmpl->GetFunction());
}

Handle<Object> rtObjectWrapper::createFromObjectReference(v8::Local<v8::Context>& ctx, const rtObjectRef& ref)
{
  Isolate* isolate(ctx->GetIsolate());

  EscapableHandleScope scope(isolate);
  Context::Scope contextScope(ctx);

  // rtLogInfo("lookup:%u addr:%p", GetContextId(ctx), ref.getPtr());
  Local<Object> obj = HandleMap::lookupSurrogate(ctx, ref);

  if (!obj.IsEmpty())
    return scope.Escape(obj);

  // introspect for rtArrayValue
  // TODO: not sure this is good. Any object can have a 'length' property
  {
    rtValue length;
    if (ref && ref->Get("length", &length) != RT_PROP_NOT_FOUND)
    {
      const int n = length.toInt32();
      Local<Array> arr = Array::New(isolate, n);
      for (int i = 0; i < n; ++i)
      {
        rtValue item;
        rtError err = ref->Get(i, &item);
        if (err == RT_OK)
          arr->Set(Number::New(isolate, i), rt2js(ctx, item));
      }
      return scope.Escape(arr);
    }
  }

  {
    rtString desc;
    if (ref)
    {
      rtError err = const_cast<rtObjectRef &>(ref).sendReturns<rtString>("description", desc);
      if (err == RT_OK && strcmp(desc.cString(), "rtPromise") == 0)
      {
        Local<Promise::Resolver> resolver = Promise::Resolver::New(isolate);

        rtFunctionRef resolve(new rtResolverFunction(rtResolverFunction::DispositionResolve, ctx, resolver));
        rtFunctionRef reject(new rtResolverFunction(rtResolverFunction::DispositionReject, ctx, resolver));

        rtObjectRef newPromise;
        rtObjectRef promise = ref;

        Local<Object> jsPromise = resolver->GetPromise();

        // rtLogInfo("addp id:%u addr:%p", GetContextId(creationContext), ref.getPtr());
        HandleMap::addWeakReference(isolate, ref, jsPromise);

        err = promise.send("then", resolve, reject, newPromise);
        if (err == RT_OK)
          return scope.Escape(jsPromise);
        else
          rtLogError("failed to setup promise");

        return scope.Escape(Local<Object>());
      }
    }
  }

  Local<Value> argv[1] =
  {
    External::New(isolate, ref.getPtr())
  };

  Local<Function> func = PersistentToLocal(isolate, ctor);
  obj = func->NewInstance(1, argv);

  // Local<Context> creationContext = obj->CreationContext();
  // rtLogInfo("add id:%u addr:%p", GetContextId(creationContext), ref.getPtr());
  // assert(GetContextId(creationContext) == GetContextId(ctx));

  HandleMap::addWeakReference(isolate, ref, obj);
  return scope.Escape(obj);
}

rtValue rtObjectWrapper::unwrapObject(const Local<Object>& obj)
{
  return rtValue(unwrap(obj));
}

template<typename T>
void rtObjectWrapper::getProperty(const T& prop, const PropertyCallbackInfo<Value>& info)
{
  HandleScope handle_scope(info.GetIsolate());
  Local<Context> ctx = info.This()->CreationContext();

  rtObjectWrapper* wrapper = node::ObjectWrap::Unwrap<rtObjectWrapper>(info.This());
  if (!wrapper)
    return;

  rtObjectRef ref = wrapper->mWrappedObject;
  if (!ref)
    return;

  rtValue value;
  rtWrapperSceneUpdateEnter();
  rtError err = ref->Get(prop, &value);
  rtWrapperSceneUpdateExit();

  if (err != RT_OK)
  {
    if (err == RT_PROP_NOT_FOUND)
      return;
    else
      info.GetIsolate()->ThrowException(Exception::Error(String::NewFromUtf8(info.GetIsolate(),
        rtStrError(err))));
  }
  else
  {
    Local<Value> v;
    EscapableHandleScope scope(info.GetIsolate());
    v = rt2js(ctx, value);
//    info.GetReturnValue().Set(rt2js(info.GetIsolate(), value));
    scope.Escape(v);
    info.GetReturnValue().Set(v);
  }
}

template<typename T>
void rtObjectWrapper::setProperty(const T& prop, Local<Value> val, const PropertyCallbackInfo<Value>& info)
{
  Locker locker(info.GetIsolate());
  Isolate::Scope isolateScope(info.GetIsolate());
  HandleScope handleScope(info.GetIsolate());
  Local<Context> creationContext = info.This()->CreationContext();

  rtWrapperError error;
  rtValue value = js2rt(creationContext, val, &error);
  if (error.hasError())
    info.GetIsolate()->ThrowException(error.toTypeError(info.GetIsolate()));

  rtWrapperSceneUpdateEnter();
  rtError err = unwrap(info)->Set(prop, &value);
  rtWrapperSceneUpdateExit();
  if (err == RT_OK)
    info.GetReturnValue().Set(val);
}

void rtObjectWrapper::getEnumerable(const PropertyCallbackInfo<Array>& info, enumerable_item_creator_t create)
{
  rtObjectWrapper* wrapper = node::ObjectWrap::Unwrap<rtObjectWrapper>(info.This());
  if (!wrapper)
    return;

  rtObjectRef ref = wrapper->mWrappedObject;
  if (!ref)
    return;

  rtObjectRef keys = ref.get<rtObjectRef>(kFuncAllKeys);
  if (!keys)
    return;

  uint32_t length = keys.get<uint32_t>(kPropLength);
  Local<Array> props = Array::New(info.GetIsolate(), length);

  for (uint32_t i = 0; i < length; ++i)
    props->Set(Number::New(info.GetIsolate(), i), create(info.GetIsolate(), keys, i));

  info.GetReturnValue().Set(props);
}

void rtObjectWrapper::getEnumerablePropertyNames(const PropertyCallbackInfo<Array>& info)
{
  getEnumerable(info, makeStringFromKey);
}

void rtObjectWrapper::getEnumerablePropertyIndecies(const PropertyCallbackInfo<Array>& info)
{
  getEnumerable(info, makeIntegerFromKey);
}

void rtObjectWrapper::getPropertyByName(Local<String> prop, const PropertyCallbackInfo<Value>& info)
{
  rtString name = toString(prop);
  getProperty(name.cString(), info);
}

void rtObjectWrapper::getPropertyByIndex(uint32_t index, const PropertyCallbackInfo<Value>& info)
{
  getProperty(index, info);
}

void rtObjectWrapper::setPropertyByName(Local<String> prop, Local<Value> val, const PropertyCallbackInfo<Value>& info)
{
  rtString name = toString(prop);
  setProperty(name.cString(), val, info);
}

void rtObjectWrapper::setPropertyByIndex(uint32_t index, Local<Value> val, const PropertyCallbackInfo<Value>& info)
{
  setProperty(index, val, info);
}

void rtObjectWrapper::create(const FunctionCallbackInfo<Value>& args)
{
  assert(args.IsConstructCall());

  HandleScope scope(args.GetIsolate());
  rtObject* obj = static_cast<rtObject*>(args[0].As<External>()->Value());
  rtObjectWrapper* wrapper = new rtObjectWrapper(obj);
  wrapper->Wrap(args.This());
}

jsObjectWrapper::jsObjectWrapper(Isolate* isolate, const Handle<Value>& obj, bool isArray)
  : mRefCount(0)
  , mObject(isolate, Handle<Object>::Cast(obj))
  , mIsolate(isolate)
  , mIsArray(isArray)
{
}

jsObjectWrapper::~jsObjectWrapper()
{
  mObject.Reset();
}

unsigned long jsObjectWrapper::AddRef()
{
  return rtAtomicInc(&mRefCount);
}

unsigned long jsObjectWrapper::Release()
{
  unsigned long l = rtAtomicDec(&mRefCount);
  if (l == 0) delete this;
  return l;
}

rtError jsObjectWrapper::getAllKeys(Isolate* isolate, rtValue* value) const
{
  HandleScope handleScope(isolate);
  Local<Object> self = PersistentToLocal(isolate, mObject);
  Local<Array> names = self->GetPropertyNames();
  Local<Context> ctx = self->CreationContext();

  rtRefT<rtArrayObject> result(new rtArrayObject);
  for (int i = 0, n = names->Length(); i < n; ++i)
  {
    rtWrapperError error;
    rtValue val = js2rt(ctx, names->Get(i), &error);
    if (error.hasError())
      return RT_FAIL;
    else
      result->pushBack(val);
  }

  *value = rtValue(result);
  return RT_OK;
}

rtError jsObjectWrapper::Get(const char* name, rtValue* value) const
{
  HandleScope handle_scope(mIsolate);

  if (!name)
    return RT_ERROR_INVALID_ARG;
  if (!value)
    return RT_ERROR_INVALID_ARG;

  if (strcmp(name, jsObjectWrapper::kIsJavaScriptObjectWrapper) == 0)
    return RT_OK;

  // TODO: does array support this?
  if (strcmp(name, kFuncAllKeys) == 0)
    return getAllKeys(mIsolate, value);

  rtError err = RT_OK;

  Local<Object> self = PersistentToLocal(mIsolate, mObject);
  Local<String> s = String::NewFromUtf8(mIsolate, name);
  Local<Context> ctx = self->CreationContext();

  if (mIsArray)
  {
    if (!strcmp(name, "length"))
      *value = rtValue(Array::Cast(*self)->Length());
    else
      err = Get(s->ToArrayIndex()->Value(), value);
  }
  else
  {
    if (!self->Has(s))
    {
      err = RT_PROPERTY_NOT_FOUND;
    }
    else
    {
      rtWrapperError error;
      *value = js2rt(ctx, self->Get(s), &error);
      if (error.hasError())
        err = RT_ERROR_INVALID_ARG;
    }
  }
  return err;
}

rtError jsObjectWrapper::Get(uint32_t i, rtValue* value) const
{
  if (!value)
    return RT_ERROR_INVALID_ARG;

  Locker locker(mIsolate);
  HandleScope handleScope(mIsolate);

  Local<Object> self = PersistentToLocal(mIsolate, mObject);
  if (!self->Has(i))
    return RT_PROPERTY_NOT_FOUND;

  Local<Context> ctx = self->CreationContext();

  rtWrapperError error;
  *value = js2rt(ctx, self->Get(i), &error);
  if (error.hasError())
    return RT_ERROR_INVALID_ARG;

  return RT_OK;
}

rtError jsObjectWrapper::Set(const char* name, const rtValue* value)
{
  if (!name)
    return RT_ERROR_INVALID_ARG;
  if (!value)
    return RT_ERROR_INVALID_ARG;

  Locker locker(mIsolate);
  HandleScope handleScope(mIsolate);
  Local<String> s = String::NewFromUtf8(mIsolate, name);
  Local<Object> self = PersistentToLocal(mIsolate, mObject);
  Local<Context> ctx = self->CreationContext();

  rtError err = RT_OK;

  if (mIsArray)
  {
    Local<Uint32> idx = s->ToArrayIndex();
    if (idx.IsEmpty())
      err = RT_ERROR_INVALID_ARG;
    else
      err = Set(idx->Value(), value);
  }
  else
  {
    err = self->Set(s, rt2js(ctx, *value));
  }

  return err;
}

rtError jsObjectWrapper::Set(uint32_t i, const rtValue* value)
{
  if (!value)
    return RT_ERROR_INVALID_ARG;

  Locker locker(mIsolate);
  HandleScope handleScope(mIsolate);
  Local<Object> self = PersistentToLocal(mIsolate, mObject);
  Local<Context> ctx = self->CreationContext();

  if (!self->Set(i, rt2js(ctx, *value)))
    return RT_FAIL;

  return RT_OK;
}

Local<Object> jsObjectWrapper::getWrappedObject()
{
  EscapableHandleScope scope(mIsolate);
  return scope.Escape(PersistentToLocal(mIsolate, mObject));
}

