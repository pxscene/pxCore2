#ifndef __RT_RPC_H__
#define __RT_RPC_H__

#include <rtError.h>
#include <rtObject.h>

rtError rtRpcInit();
rtError rtRpcRegisterObject(char const* id, rtObjectRef const& obj);
rtError rtRpcLocateObject(char const* id, rtObjectRef& obj);
rtError rtRpcShutdown();

#endif
