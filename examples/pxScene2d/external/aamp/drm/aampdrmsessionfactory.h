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
* @file aampdrmsessionfactory.h
* @brief Header file for AampDrmSessionFactory
*/

#ifndef AampDrmSessionFactory_h
#define AampDrmSessionFactory_h

#include "AampDrmSession.h"

/**
 * @class AampDrmSessionFactory
 * @brief Factory class to create DRM sessions based on
 *        requested system ID
 */
class AampDrmSessionFactory
{
public:

	static AampDrmSession* GetDrmSession(const char* systemId);
};
#endif
