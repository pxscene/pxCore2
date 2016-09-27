#ifndef __RT_RPC_H__
#define __RT_RPC_H__

#include <rtError.h>
#include <rtObject.h>
#include <stdint.h>

#define RT_REMOTE_TIMEOUT_INFINITE UINT32_MAX
#define RT_REMOTE_API_VERSION 2.0
#define RT_REMOTE_OLDSTYLE_API 1

typedef struct rtRemoteEnvironment rtRemoteEnvironment;

/**
 * Get a pointer the global rtRemoteEnvironment
 * Don't delete this!
 */
rtRemoteEnvironment*
rtEnvironmentGetGlobal();

/**
 * Create a new rtRemoteEnvironment from a file
 * @param configFile The rtremote configuration file to create the environment from
 * @returns a newly allocated rtRemoteEnvironment. Caller must delete.
 */
rtRemoteEnvironment*
rtEnvironmentFromFile(char const* configFile);


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
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj, int timeout=3000);

/**
 * Shutdown rtRemote sub-system
 * @returns RT_OK for success
 */
rtError
rtRemoteShutdown(rtRemoteEnvironment* env);

/**
 * Same as rtRemoteRunOnce, except that it will dispatch messages until the timout
 * is expired. Timeout duration is in milliseconds.
 * @param timeout The amount of time to run this function. Use RT_REMOTE_TIMEOUT_INIFINITE
 * to run forever, or until another thread calls rtRemoteShutdown();
 */
rtError
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout);

rtError rtRemoteInitNs(rtRemoteEnvironment* env);
rtError rtRemoteShutdownNs(rtRemoteEnvironment* env);

#ifdef RT_REMOTE_OLDSTYLE_API
rtError
rtRemoteInit();

rtError
rtRemoteRegisterObject(char const* id, rtObjectRef const& obj);

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj, int timeout = 3000);

rtError
rtRemoteShutdown();
#endif

#endif
