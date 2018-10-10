/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

#ifndef __RT_RPC_H__
#define __RT_RPC_H__

#include <rtError.h>
#include <rtObject.h>
#include <stdint.h>

#define RT_REMOTE_TIMEOUT_INFINITE UINT32_MAX
#define RT_REMOTE_API_VERSION 2.0
#define RT_REMOTE_OLDSTYLE_API 1

class   rtRemoteEnvironment;
typedef void(*remoteDisconnectedCallback)(void *data);

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

typedef void (*rtRemoteQueueReady)(void*);
/**
 * Register a handler to be called when item is ready to be processed
 * @param handler the handler to be called by rtRemote
 * @param argp context to pass to the handler
 * @returns RT_OK for success
 */
rtError
rtRemoteRegisterQueueReadyHandler ( rtRemoteEnvironment* env, rtRemoteQueueReady handler, void* argp);

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
 * @returns RT_OK for success
 */
rtError
rtRemoteRegisterObject(rtRemoteEnvironment* env, char const* id, rtObjectRef const& obj);

rtError
rtRemoteUnregisterObject(rtRemoteEnvironment* env, char const* id);

/**
 * Locate a remote object by id.
 * @param id The id of the object to locate.
 * @param obj The object reference if found.
 * @returns RT_OK for success
 */
rtError
rtRemoteLocateObject(rtRemoteEnvironment* env, char const* id, rtObjectRef& obj, int timeout=3000,
        remoteDisconnectedCallback cb=NULL, void *cbdata=NULL);

/**
 * Removes the pair {cb, data} from a client's disconnected callback list.
 * @param cb callback pointer
 * @param cbdata callback data pointer
 * @returns RT_OK for success, RT_ERROR_INVALID_ARG if {cb, cbdata} could not be found
 */
rtError
rtRemoteUnregisterDisconnectedCallback( rtRemoteEnvironment* env, remoteDisconnectedCallback cb=NULL, void *cbdata=NULL );

/**
 * Shutdown rtRemote sub-system
 * @param immediate ignore RefCount
 * @returns RT_OK for success
 */
rtError
rtRemoteShutdown(rtRemoteEnvironment* env, bool immediate = false);

/**
 * Processes a single queue item. This is an API to be called from main loop from queue callback.
 * @returns RT_OK for success
 */
rtError
rtRemoteProcessSingleItem(rtRemoteEnvironment* env);

/**
 * Same as rtRemoteRunOnce, except that it will dispatch messages until the timout
 * is expired. Timeout duration is in milliseconds.
 * @param timeout The amount of time to run this function. Use RT_REMOTE_TIMEOUT_INIFINITE
 * to run forever, or until another thread calls rtRemoteShutdown();
 * @param wait If true will block the thread until an event comes or timeout reached
 */
rtError
rtRemoteRun(rtRemoteEnvironment* env, uint32_t timeout, bool wait);

rtError rtRemoteInitNs(rtRemoteEnvironment* env);
rtError rtRemoteShutdownNs(rtRemoteEnvironment* env);

#ifdef RT_REMOTE_OLDSTYLE_API
rtError
rtRemoteInit();

rtError
rtRemoteRegisterObject(char const* id, rtObjectRef const& obj);

rtError
rtRemoteUnregisterObject(char const* id);

rtError
rtRemoteLocateObject(char const* id, rtObjectRef& obj, int timeout = 3000,
        remoteDisconnectedCallback cb=NULL, void *cbdata=NULL);

rtError
rtRemoteUnregisterDisconnectedCallback( remoteDisconnectedCallback cb=NULL, void *cbdata=NULL );

rtError
rtRemoteShutdown();

rtError
rtRemoteProcessSingleItem();

rtError
rtRemoteRunUntil(rtRemoteEnvironment* env, uint32_t millisecondsFromNow, bool wait);

#endif

#endif
