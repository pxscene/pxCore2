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
 * @file StubsForAVEPlayer.cpp
 * @brief Various stubs required to compile AVE drm
 * @note To be removed once avedrm is used instead of aveplayer
 */

#ifdef AAMP_STUBS_FOR_JS

/*Stubs to link with libAVEPlayer.so*/
extern "C" {

void JSValueToObject(){ }
void JSStringCreateWithUTF8CString(){ }
void JSStringGetCharactersPtr(){ }
void JSValueMakeNull(){ }
void JSValueIsUndefined(){ }
void JSClassRelease(){ }
void JSValueIsObjectOfClass(){ }
void JSStringRelease(){ }
void JSObjectSetProperty(){ }
void JSValueMakeNumber(){ }
void JSValueToBoolean(){ }
void JSObjectIsFunction(){ }
void JSValueIsBoolean(){ }
void JSValueToStringCopy(){ }
void JSValueToNumber(){ }
void JSValueIsString(){ }
void JSValueUnprotect(){ }
void JSObjectSetPrivate(){ }
void JSStringGetUTF8CString(){ }
void JSContextGetGlobalObject(){ }
void JSValueMakeString(){ }
void JSGarbageCollect(){ }
void JSStringGetLength(){ }
void JSObjectMakeConstructor(){ }
void JSObjectCallAsFunction(){ }
void JSValueMakeBoolean(){ }
void JSValueIsObject(){ }
void JSObjectGetPropertyAtIndex(){ }
void JSObjectMakeFunction(){ }
void JSObjectCopyPropertyNames(){ }
void JSPropertyNameArrayRelease() { }
void JSObjectIsConstructor(){ }
void JSStringGetMaximumUTF8CStringSize(){ }
void JSValueMakeUndefined(){ }
void JSObjectMake(){ }
void JSPropertyNameArrayGetNameAtIndex(){ }
void JSValueIsInstanceOfConstructor(){ }
void JSContextGetGlobalContext(){ }
void JSValueIsNumber(){ }
void JSObjectHasProperty(){ }
void JSObjectGetPrivate(){ }
void JSPropertyNameArrayGetCount(){ }
void JSObjectMakeArray(){ }
void JSObjectGetProperty(){ }
void JSClassCreate(){ }
void JSValueProtect(){ }
void JSStringCreateWithCharacters(){ }
void JSObjectMakeDate(){ }
void JSObjectMakeError(){ }
}
#endif

#ifdef AVE_DRM

#include "psdk/PSDKEvents.h"
#include "psdkutils/PSDKError.h"

psdk::PSDKEventManager* GetPlatformCallbackManager()
{
    return NULL;
}


void* GetClosedCaptionSurface()
{
    return NULL;
}


void ClearClosedCaptionSurface()
{
}


void HideClosedCaptionSurface()
{
}

void ShowClosedCaptionSurface()
{
}

void* CreateSurface()
{
     return NULL;
}

void DestroySurface(void* surface)
{
}

void GetSurfaceScale(double *pScaleX, double *pScaleY)
{
}

void SetSurfaceSize(void* surface, int width, int height)
{
}

void SetSurfacePos(void* surface, int x, int y)
{
}

#endif
