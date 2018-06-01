#include "rtWrapperUtils.h"
#include "rtObjectWrapper.h"
#include "rtFunctionWrapper.h"
#ifndef RUNINMAIN
extern uv_mutex_t threadMutex;
#endif
#include <rtMutex.h>


#include <set>
#include <sstream>

using namespace std;

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
    duk_push_null(ctx);
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


rtValue duk2rt(duk_context *ctx, duk_idx_t idx, rtWrapperError* error)
{
  if (duk_is_undefined(ctx, idx)) { return rtValue((void *)0); }
  if (duk_is_null(ctx, idx)) { return rtValue((char *)0); }
  if (duk_is_string(ctx, idx)) { return rtValue(duk_get_string(ctx, idx)); }
  if (duk_is_c_function(ctx, idx)) {
    duk_bool_t res = duk_get_prop_string(ctx, idx, "\xff""\xff""data");
    assert(res);
    rtIFunction *func = (rtIFunction*)duk_require_pointer(ctx, idx);
    duk_pop(ctx);
    return rtValue(rtFunctionRef(func));
  }
  if (duk_is_function(ctx, idx)) {
    duk_dup(ctx, idx);
    std::string id = rtDukPutIdentToGlobal(ctx);
    return rtValue(rtFunctionRef(new jsFunctionWrapper(ctx, id)));
  }
  if (duk_is_object(ctx, idx)) {
    //duk_bool_t res = duk_get_prop_string(ctx, idx, "\xff""\xff""data");
    duk_bool_t res = duk_get_prop_string(ctx, idx, "zpointer");

    if (res) {
      //printf("recovered rtObject pointer\n");
      rtIObject *obj = (rtIObject*)duk_require_pointer(ctx, -1);
      //printf("recovered rtObject pointer %p\n", obj);
      duk_pop(ctx);
      return rtValue(obj);
    }

    duk_pop(ctx);

    duk_dup(ctx, idx);
    std::string id = rtDukPutIdentToGlobal(ctx);

    bool isArray = duk_is_array(ctx, idx);
    return rtValue(new jsObjectWrapper(ctx, id, isArray));
  }

  if (duk_is_boolean(ctx, idx)) { return rtValue(duk_get_boolean(ctx, idx)); }
  if (duk_is_number(ctx, idx)) { return rtValue(duk_get_number(ctx, idx)); }

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


static std::set<std::string> g_globalIdsSet;
static std::map<duk_context*, std::set<std::string>> g_dukRtObjectSet;


std::string rtDukPutIdentToGlobal(duk_context *ctx, const std::string &name)
{
  std::string id = name;
  if (id.empty()) {
    id = rtAllocDukIdentId();
  }
  duk_bool_t rc = duk_put_global_string(ctx, id.c_str());
  assert(rc);
  g_globalIdsSet.insert(id);
  return id;
}


void rtDukDelGlobalIdent(duk_context *ctx, const std::string &name)
{
  duk_push_global_object(ctx);
  duk_del_prop_string(ctx, -1, name.c_str());
  duk_pop(ctx);
  g_globalIdsSet.erase(name);
}


void rtClearAllGlobalIdents(duk_context *ctx)
{
  std::set<std::string> copySet(g_globalIdsSet);
  for (std::set<std::string>::iterator it = copySet.begin(); it != copySet.end(); ++it) {
    if ((*it).substr(0, 7) == "__ident") {
      rtDukDelGlobalIdent(ctx, *it);
    }
  }
}

void rtDukAllocObjectIdent(duk_context *ctx, rtIObject* o, duk_idx_t index)
{
  rtString objReferenceKey = "rtObject_";
  std::stringstream ss;
  ss << o;
  objReferenceKey.append(ss.str().c_str());
  duk_push_thread_stash(ctx, ctx);
  duk_push_string(ctx,objReferenceKey.cString());
  duk_dup(ctx, index);
  duk_bool_t rc = duk_put_prop(ctx, -3);
  duk_pop(ctx);
  g_dukRtObjectSet[ctx].insert(objReferenceKey.cString());
}

bool rtDukCheckObjectIdent(duk_context *ctx, rtIObject* o)
{
  rtString objReferenceKey = "rtObject_"; 
  std::stringstream ss;
  ss << o;
  objReferenceKey.append(ss.str().c_str());

  duk_push_thread_stash(ctx, ctx);
  duk_push_string(ctx,objReferenceKey.cString());
  duk_bool_t isPresent = duk_get_prop(ctx, -2);
  if (isPresent)
  {
    duk_swap(ctx, -1, -2);
    duk_pop(ctx);
  }
  else
  {
    duk_pop(ctx);
    duk_pop(ctx);
  }
  return isPresent;
}

void rtDukDelObjectIdent(duk_context *ctx, rtIObject* o)
{
  rtString objReferenceKey = "rtObject_";
  std::stringstream ss;
  ss << o;
  objReferenceKey.append(ss.str().c_str());
  duk_push_thread_stash(ctx,ctx);
  duk_del_prop_string(ctx, -1, objReferenceKey.cString());
  duk_pop(ctx);
  g_dukRtObjectSet[ctx].erase(objReferenceKey.cString());
}

void rtClearAllObjectIdents(duk_context *ctx)
{
  std::set<std::string> copySet(g_dukRtObjectSet[ctx]);
  for (std::set<std::string>::iterator it = copySet.begin(); it != copySet.end(); ++it) {
    duk_push_thread_stash(ctx,ctx);
    duk_del_prop_string(ctx, -1, (*it).c_str());
    duk_pop(ctx);
    g_dukRtObjectSet[ctx].erase((*it));
  }
  g_dukRtObjectSet.erase(ctx);
}
