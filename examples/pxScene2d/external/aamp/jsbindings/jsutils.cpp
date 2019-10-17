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
 * @file jsutils.cpp
 * @brief JavaScript util functions for AAMP
 */


#include "jsutils.h"

#include <stdlib.h>
#include <stdio.h>

/**
 * @struct EventTypeMap
 * @brief Struct to map names of AAMP events and JS events
 */
struct EventTypeMap
{
	AAMPEventType eventType;
	const char* szName;
};


/**
 * @brief Map AAMP events to its corresponding JS event strings
 */
static EventTypeMap aamp_eventTypes[] =
{
	{ (AAMPEventType)0, "onEvent"},
	{ AAMP_EVENT_TUNED, "tuned"},
	{ AAMP_EVENT_TUNE_FAILED, "tuneFailed"},
	{ AAMP_EVENT_SPEED_CHANGED, "speedChanged"},
	{ AAMP_EVENT_EOS, "eos"},
	{ AAMP_EVENT_PLAYLIST_INDEXED, "playlistIndexed"},
	{ AAMP_EVENT_PROGRESS, "progress"},
	{ AAMP_EVENT_CC_HANDLE_RECEIVED, "decoderAvailable"},
	{ AAMP_EVENT_JS_EVENT, "jsEvent"},
	{ AAMP_EVENT_MEDIA_METADATA, "metadata"},
	{ AAMP_EVENT_ENTERING_LIVE, "enteringLive"},
	{ AAMP_EVENT_BITRATE_CHANGED, "bitrateChanged"},
	{ AAMP_EVENT_TIMED_METADATA, "timedMetadata"},
	{ AAMP_EVENT_STATE_CHANGED, "statusChanged"},
	{ AAMP_EVENT_SPEEDS_CHANGED, "speedsChanged"},
	{ AAMP_EVENT_DRM_METADATA, "drmMetadata"},
	{ AAMP_EVENT_REPORT_ANOMALY, "anomalyReport" },
	{ AAMP_EVENT_REPORT_METRICS_DATA, "metricsData" },
	{ AAMP_EVENT_AD_RESOLVED, "adResolved"},
	{ AAMP_EVENT_AD_RESERVATION_START, "reservationStart" },
	{ AAMP_EVENT_AD_RESERVATION_END, "reservationEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_START, "placementStart" },
	{ AAMP_EVENT_AD_PLACEMENT_END, "placementEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_PROGRESS, "placementProgress" },
	{ AAMP_EVENT_AD_PLACEMENT_ERROR, "placementError" },
	{ (AAMPEventType)0, "" }
};


/**
 * @brief Map AAMP events to its corresponding JS event strings (AAMPMediaPlayer)
 */
static EventTypeMap aampPlayer_eventTypes[] =
{
//TODO: Need separate event list to avoid breaking existing viper impl. Unify later.
	{ (AAMPEventType)0, "onEvent"},
	{ AAMP_EVENT_TUNED, "playbackStarted"},
	{ AAMP_EVENT_TUNE_FAILED, "playbackFailed"},
	{ AAMP_EVENT_SPEED_CHANGED, "playbackSpeedChanged"},
	{ AAMP_EVENT_EOS, "playbackCompleted"},
	{ AAMP_EVENT_PLAYLIST_INDEXED, "playlistIndexed"},
	{ AAMP_EVENT_PROGRESS, "playbackProgressUpdate"},
	{ AAMP_EVENT_CC_HANDLE_RECEIVED, "decoderAvailable"},
	{ AAMP_EVENT_JS_EVENT, "jsEvent"},
	{ AAMP_EVENT_MEDIA_METADATA, "mediaMetadata"},
	{ AAMP_EVENT_ENTERING_LIVE, "enteringLive"},
	{ AAMP_EVENT_BITRATE_CHANGED, "bitrateChanged"},
	{ AAMP_EVENT_TIMED_METADATA, "timedMetadata"},
	{ AAMP_EVENT_STATE_CHANGED, "playbackStateChanged"},
	{ AAMP_EVENT_SPEEDS_CHANGED, "speedsChanged"},
	{ AAMP_EVENT_BUFFERING_CHANGED, "bufferingChanged"},
	{ AAMP_EVENT_DURATION_CHANGED, "durationChanged"},
	{ AAMP_EVENT_AUDIO_TRACKS_CHANGED, "currentAudioTrackChanged"},
	{ AAMP_EVENT_TEXT_TRACKS_CHANGED, "currentTextTrackChanged"},
	{ AAMP_EVENT_AD_BREAKS_CHANGED, "contentBreaksChanged"},
	{ AAMP_EVENT_AD_STARTED, "contentStarted"},
	{ AAMP_EVENT_AD_COMPLETED, "contentCompleted"},
	{ AAMP_EVENT_DRM_METADATA, "drmMetadata"},
	{ AAMP_EVENT_REPORT_ANOMALY, "anomalyReport" },
	{ AAMP_EVENT_AD_RESOLVED, "adResolved"},
	{ AAMP_EVENT_AD_RESERVATION_START, "reservationStart" },
	{ AAMP_EVENT_AD_RESERVATION_END, "reservationEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_START, "placementStart" },
	{ AAMP_EVENT_AD_PLACEMENT_END, "placementEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_PROGRESS, "placementProgress" },
	{ AAMP_EVENT_AD_PLACEMENT_ERROR, "placementError" },
	{ (AAMPEventType)0, "" }
};


/**
 * @brief Convert C string to JSString
 * @param[in] context JS execution context
 * @param[in] sz C string
 * @retval JSValue that is the converted JSString
 */
JSValueRef aamp_CStringToJSValue(JSContextRef context, const char* sz)
{
	JSStringRef str = JSStringCreateWithUTF8CString(sz);
	JSValueRef value = JSValueMakeString(context, str);
	JSStringRelease(str);

	return value;
}


/**
 * @brief Convert JSString to C string
 * @param[in] context JS execution context
 * @param[in] value JSValue of JSString to be converted
 * @param[out] exception pointer to a JSValueRef in which to store an exception, if any
 * @retval converted C string
 */
char* aamp_JSValueToCString(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
	JSStringRef jsstr = JSValueToStringCopy(context, value, exception);
	size_t len = JSStringGetMaximumUTF8CStringSize(jsstr);
	char* src = new char[len];
	JSStringGetUTF8CString(jsstr, src, len);
	JSStringRelease(jsstr);
	return src;
}


/**
 * @brief Check if a JSValue object is array or not
 * @param[in] context JS exception context
 * @param[in] value JSValue to check if array or not
 * @retval true if JSValue is an array
 */
bool aamp_JSValueIsArray(JSContextRef context, JSValueRef value)
{
	JSObjectRef global = JSContextGetGlobalObject(context);
	JSStringRef arrayProp = JSStringCreateWithUTF8CString("Array");
	JSValueRef arrayVal = JSObjectGetProperty(context, global, arrayProp, NULL);
	JSStringRelease(arrayProp);

	if (JSValueIsObject(context, arrayVal))
	{
		JSObjectRef arrayObj = JSValueToObject(context, arrayVal, NULL);
		if (JSObjectIsFunction(context, arrayObj) || JSObjectIsConstructor(context, arrayObj))
		{
			return JSValueIsInstanceOfConstructor(context, value, arrayObj, NULL);
		}
	}

	return false;
}


/**
 * @brief Convert an array of JSString to an array of C strings
 * @param[in] context JS execution context
 * @param[in] arrayRef JSValue of an array of JSString
 * @retval converted array of C strings
 */
std::vector<std::string> aamp_StringArrayToCStringArray(JSContextRef context, JSValueRef arrayRef)
{
    std::vector<std::string> retval;
    JSValueRef exception = NULL;

    if(!arrayRef)
    {
	ERROR("[AAMP_JS] %s() Error: value is NULL.", __FUNCTION__);
        return retval;
    }
    if (!JSValueIsObject(context, arrayRef))
    {
	ERROR("[AAMP_JS] %s() Error: value is not an object.", __FUNCTION__);
        return retval;
    }
    if(!aamp_JSValueIsArray(context, arrayRef))
    {
	ERROR("[AAMP_JS] %s() Error: value is not an array.", __FUNCTION__);
        return retval;
    }
    JSObjectRef arrayObj = JSValueToObject(context, arrayRef, &exception);
    if(exception)
    {
	ERROR("[AAMP_JS] %s() Error: exception accesing array object.", __FUNCTION__);
        return retval;
    }

    JSStringRef lengthStrRef = JSStringCreateWithUTF8CString("length");
    JSValueRef lengthRef = JSObjectGetProperty(context, arrayObj, lengthStrRef, &exception);
    if(exception)
    {
	ERROR("[AAMP_JS] %s() Error: exception accesing array length.", __FUNCTION__);
        return retval;
    }
    int length = JSValueToNumber(context, lengthRef, &exception);
    if(exception)
    {
	ERROR("[AAMP_JS] %s() Error: exception array length in not a number.", __FUNCTION__);
        return retval;
    }

    retval.reserve(length);
    for(int i = 0; i < length; i++)
    {
        JSValueRef strRef = JSObjectGetPropertyAtIndex(context, arrayObj, i, &exception);
        if(exception)
            continue;

        char* str = aamp_JSValueToCString(context, strRef, NULL);
	LOG("[AAMP_JS] %s() array[%d] = '%s'.", __FUNCTION__, i, str);
        retval.push_back(str);
        delete [] str;
    }

    JSStringRelease(lengthStrRef);

    return retval;
}


/**
 * @brief Generate a JSValue object with the exception details
 * @param[in] context JS exception context
 * @param[in] error error/exception code
 * @param[in] additionalInfo additional error description
 * @retval JSValue object with exception details
 */
JSValueRef aamp_GetException(JSContextRef context, ErrorCode error, const char *additionalInfo)
{
	const char *str = "Generic Error";
	JSValueRef retVal;

	switch(error)
	{
		case AAMPJS_INVALID_ARGUMENT:
		case AAMPJS_MISSING_OBJECT:
			str = "TypeError";
			break;
		default:
			str = "Generic Error";
			break;
	}

	char exceptionMsg[EXCEPTION_ERR_MSG_MAX_LEN];
	memset(exceptionMsg, '\0', EXCEPTION_ERR_MSG_MAX_LEN);

	if(additionalInfo)
	{
		snprintf(exceptionMsg, EXCEPTION_ERR_MSG_MAX_LEN - 1, "%s: %s", str, additionalInfo);
	}
	else
	{
		snprintf(exceptionMsg, EXCEPTION_ERR_MSG_MAX_LEN - 1, "%s!!", str);
	}

	ERROR("[AAMP_JS] %s() Error=%s", __FUNCTION__, exceptionMsg);

	const JSValueRef arguments[] = { aamp_CStringToJSValue(context, exceptionMsg) };
	JSValueRef exception = NULL;
	retVal = JSObjectMakeError(context, 1, arguments, &exception);
	if (exception)
	{
		ERROR("[AAMP_JS] %s() Error: exception creating an error object", __FUNCTION__);
		return NULL;
	}

	return retVal;
}


/**
 * @brief Convert JS event name to AAMP event type
 * @param[in] szName JS event name
 * @retval AAMPEventType of corresponding AAMP event
 */
AAMPEventType aamp_getEventTypeFromName(const char* szName)
{
	AAMPEventType eventType = AAMP_MAX_NUM_EVENTS;
	int numEvents = sizeof(aamp_eventTypes) / sizeof(aamp_eventTypes[0]);

	for (int i=0; i<numEvents; i++)
	{
		if (strcmp(aamp_eventTypes[i].szName, szName) == 0)
		{
			eventType = aamp_eventTypes[i].eventType;
			break;
		}
	}

	return eventType;
}


/**
 * @brief To dispatch a JS event
 * @param[in] context JS execution context
 * @param[in] callback function to which event has to be dispatched as an arg
 * @param[in] event the JS event to be dispatched
 */
void aamp_dispatchEventToJS(JSContextRef context, JSObjectRef callback, JSObjectRef event)
{
	JSValueRef args[1] = { event };
	if (context != NULL && callback != NULL)
	{
		JSObjectCallAsFunction(context, callback, NULL, 1, args, NULL);
	}
}


/**
 * @brief Convert JS event name to AAMP event type (AAMPMediaPlayer)
 * @param[in] szName JS event name
 * @retval AAMPEventType of corresponding AAMP event
 */
AAMPEventType aampPlayer_getEventTypeFromName(const char* szName)
{
//TODO: Need separate event list for now to avoid breaking existing viper impl. Unify later
	AAMPEventType eventType = AAMP_MAX_NUM_EVENTS;
	int numEvents = sizeof(aampPlayer_eventTypes) / sizeof(aampPlayer_eventTypes[0]);

	for (int i=0; i<numEvents; i++)
	{
		if (strcmp(aampPlayer_eventTypes[i].szName, szName) == 0)
		{
			eventType = aampPlayer_eventTypes[i].eventType;
			break;
		}
	}

	return eventType;
}


/**
 * @brief Convert AAMP event type to JS event string (AAMPMediaPlayer)
 * @param[in] szName AAMP event type
 * @retval JS event name corresponding to AAMP event
 */
const char* aampPlayer_getNameFromEventType(AAMPEventType type)
{
//TODO: Need separate API to avoid breaking existing viper impl. Unify later.
	if (type > 0 && type < AAMP_MAX_NUM_EVENTS)
	{
		return aampPlayer_eventTypes[type].szName;
	}
	else
	{
		return NULL;
	}
}
