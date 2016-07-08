#ifndef __RT_RPC_H__
#define __RT_RPC_H__

#include <rtError.h>
#include <rtObject.h>
#include <stdint.h>

#define RT_REMOTE_TIMEOUT_INFINITE UINT32_MAX

typedef struct rtRemoteEnvironment rtRemoteEnvironment;

rtRemoteEnvironment* rtGlobalEnvironment();


/**
 * Initailize the rtRemote sub-system
 * @returns RT_OK for success
 */
rtError
rtRemoteInit(rtRemoteEnvironment* env);

/**
 * Register a remote object with the rtRemote sub-system for consumption
 * by clients.
 * @param id The id of the object
 * @param obj The reference to the object
 * @returns RT_OK for sucess
 */
rtError
rtRemoteRegisterObject(rtRemoteEnvironment* env, char const* id, rtObjectRef const& obj);

/**
 * Locate a remote object by id.
 * @param id The id of the object to locate.
 * @param obj The object reference if found.
 * @returns RT_OK for success
 */
rtError
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj);

/**
 * Shutdown rtRemote sub-system
 * @returns RT_OK for success
 */
rtError
rtRemoteShutdown(rtRemoteEnvironment* env);

/**
 * Use this when not running with a dedicated sub-system thread.
 * All incoming requests will execute on the thread calling this
 * function. It will execute a single incoming message. The timeout
 * is used to control the wait time for incoming messages, and cannot 
 * be used to accurately control how long before this function returns.
 * @param timeout The amount of time to wait for an incoming message to arrive
 * @returns RT_OK success
 */
rtError
rtRemoteRunOnce(rtRemoteEnvironment* env, uint32_t timeout);

/**
 * Same as rtRemoteRunOnce, except that it will dispatch messages until the timout
 * is expired. Timeout duration is in milliseconds.
 * @param timeout The amount of time to run this function. Use RT_REMOTE_TIMEOUT_INIFINITE
 * to run forever, or until another thread calls rtRemoteShutdown();
 */
rtError
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout);

rtError rtRemoteInitNs();
rtError rtRemoteShutdownNs();

#endif
