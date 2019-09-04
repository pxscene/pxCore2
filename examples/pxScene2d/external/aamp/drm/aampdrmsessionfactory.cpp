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

/**
* @file aampdrmsessionfactory.cpp
* @brief Source file for AampDrmSessionFactory
*/

#include "aampdrmsessionfactory.h"
#if defined(USE_OPENCDM_ADAPTER)
#include "opencdmsessionadapter.h"
#elif defined(USE_OPENCDM)
#include "opencdmsession.h"
#else
#include "playreadydrmsession.h"
#endif

/**
 *  @brief		Creates appropriate DRM systems Session objects based
 *  			on the requested systemID, like PlayReady or WideVine
 *
 *  @param[in]	systemid - DRM systems uuid
 *  @return		Pointer to DrmSession.
 */
AampDrmSession* AampDrmSessionFactory::GetDrmSession(const char* systemid)
{
	AampDrmSession* drmSession = NULL;
	if(!strcmp(PLAYREADY_PROTECTION_SYSTEM_ID, systemid))
	{
#ifdef USE_OPENCDM
        std::string key_system = PLAYREADY_KEY_SYSTEM_STRING;
        drmSession = new AAMPOCDMSession(key_system);
#else
		drmSession = new PlayReadyDRMSession();
#endif
	} else if(!strcmp(WIDEVINE_PROTECTION_SYSTEM_ID, systemid)) 
    {
#ifdef USE_OPENCDM
        std::string key_system = WIDEVINE_KEY_SYSTEM_STRING;
        drmSession = new AAMPOCDMSession(key_system);
#endif
    }
	return drmSession;
}
