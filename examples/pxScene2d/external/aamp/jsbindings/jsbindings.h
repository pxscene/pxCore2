/*
 * If not stated otherwise in this file or this component's license file the
 * following copyright and licenses apply:
 *
 * Copyright 2018 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

#ifndef __JSBINDINGS_H__
#define __JSBINDINGS_H__

#include <JavaScriptCore/JavaScript.h>
#include "main_aamp.h"
#include <map>
/*
 * @file jsbindings.h
 * @brief APIs exposed by the AAMP JS to inject different bindings.
 */

/**
 * @struct PrivAAMPStruct_JS
 * @brief Private data structure for JS binding object
 */
struct PrivAAMPStruct_JS {
	PrivAAMPStruct_JS() : _ctx(), _aamp(NULL), _listeners()
	{
	}
	PrivAAMPStruct_JS(const PrivAAMPStruct_JS&) = delete;
	PrivAAMPStruct_JS& operator=(const PrivAAMPStruct_JS&) = delete;
	JSGlobalContextRef _ctx;
	PlayerInstanceAAMP* _aamp;

	std::multimap<AAMPEventType, void*> _listeners;
};

/**
 *   @brief  Load aamp JS bindings.
 *
 *   @param[in]  context - JS Core context.
 *   @param[in]  playerInstanceAAMP - AAMP instance. NULL creates new aamp instance.
 */
void aamp_LoadJS(void* context, void* playerInstanceAAMP);

/**
 *   @brief  Unload aamp JS bindings.
 *
 *   @param[in]  context - JS Core context.
 */
void aamp_UnloadJS(void* context);

/**
 * @brief Loads AAMPMediaPlayer JS constructor into JS context
 *
 * @param[in] context JS execution context
 */
void AAMPPlayer_LoadJS(void* context);

/**
 * @brief Removes the AAMPMediaPlayer constructor from JS context
 *
 * @param[in] context JS execution context
 */
void AAMPPlayer_UnloadJS(void* context);

#endif// __JSBINDINGS_H__
