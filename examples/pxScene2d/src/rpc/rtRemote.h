#ifndef __RT_RPC_H__
#define __RT_RPC_H__

#include <rtError.h>
#include <rtObject.h>

rtError rtRemoteInit();
rtError rtRemoteInitNs();
rtError rtRemoteRegisterObject(char const* id, rtObjectRef const& obj);
rtError rtRemoteLocateObject(char const* id, rtObjectRef& obj);
rtError rtRemoteShutdown();
rtError rtRemoteShutdownNs();

#endif
