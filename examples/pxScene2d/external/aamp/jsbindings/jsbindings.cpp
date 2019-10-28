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
 * @file jsbindings.cpp
 * @brief JavaScript bindings for AAMP
 */


#include <JavaScriptCore/JavaScript.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <cmath>

#include "jsutils.h"
#include "main_aamp.h"
#include "priv_aamp.h"

#define GLOBAL_AAMP_NATIVEBINDING_VERSION "2.6"

static class PlayerInstanceAAMP* _allocated_aamp = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

extern void ClearAAMPPlayerInstances();

extern "C"
{

	/**
	 * @brief Get the global JS execution context
	 * @param[in] JS execution context
	 * @retval global execution context
	 */
	JS_EXPORT JSGlobalContextRef JSContextGetGlobalContext(JSContextRef);

	JSObjectRef AAMP_JS_AddEventTypeClass(JSGlobalContextRef context);

	JSObjectRef AAMP_JS_CreateTimedMetadata(JSContextRef context, double timeMS, const char* szName, const char* szContent, const char* id, double durationMS=0);
}

/**
 * @struct AAMP_JS
 * @brief Data structure of AAMP object
 */
struct AAMP_JS
{
	JSGlobalContextRef _ctx;
	class PlayerInstanceAAMP* _aamp;
	class AAMP_JSListener* _listeners;

	JSObjectRef _eventType;
	JSObjectRef _subscribedTags;
	JSObjectRef _promiseCallback;	/* Callback function for JS promise resolve/reject.*/
};


/**
 * @brief callback invoked when AAMP is used along with 'new'
 * @param[in] context JS execution context
 * @param[in] constructor JSObject that is the constructor being called
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSObject that is the constructor's return value
 */
static JSObjectRef AAMP_class_constructor(JSContextRef context, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	*exception = aamp_GetException(context, AAMPJS_GENERIC_ERROR, "Cannot create an object of AAMP");
	return NULL;
}


/**
 * @brief Callback invoked from JS to get the closedCaptionEnabled property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_closedCaptionEnabled(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.closedCaptionEnabled on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set the closedCaptionEnabled property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_closedCaptionEnabled(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.closedCaptionEnabled on instances of AAMP");
		return false;
	}
	return true;
}


/**
 * @brief Callback invoked from JS to get the initialBufferTime property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_initialBufferTime(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.initialBufferTime on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set the initialBufferTime property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_initialBufferTime(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.initialBufferTime on instances of AAMP");
		return false;
	}
	return true;
}


/**
 * @brief Callback invoked from JS to get the trickPlayEnabled property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_trickPlayEnabled(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.trickPlayEnabled on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set the trickPlayEnabled property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_trickPlayEnabled(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.trickPlayEnabled on instances of AAMP");
		return false;
	}
	return true;
}


/**
 * @brief Callback invoked from JS to get the EventType property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_EventType(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.EventType on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	return pAAMP->_eventType;
}


/**
 * @brief Callback invoked from JS to get the mediaType property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_MediaType(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.mediaType on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (pAAMP->_aamp->IsLive())
	{
		return aamp_CStringToJSValue(context, "live");
	}
	else
	{
		return aamp_CStringToJSValue(context, "vod");
	}
}


/**
 * @brief Callback invoked from JS to get the version property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_Version(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	TRACELOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.version on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	return aamp_CStringToJSValue(context, GLOBAL_AAMP_NATIVEBINDING_VERSION);
}


/**
 * @brief Callback invoked from JS to get the audioLanguage property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_AudioLanguage(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	TRACELOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.audioLanguage on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	const char* language = pAAMP->_aamp->GetCurrentAudioLanguage();
	return aamp_CStringToJSValue(context, language);
}


/**
 * @brief Callback invoked from JS to get the currentDRM property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_CurrentDRM(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	TRACELOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.currentDRM on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	const char* drm = pAAMP->_aamp->GetCurrentDRM();
	return aamp_CStringToJSValue(context, drm);
}


/**
 * @brief Callback invoked from JS to get the timedMetadata property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMP_getProperty_timedMetadata(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.timedMetadata on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	PrivateInstanceAAMP* privAAMP = (pAAMP->_aamp != NULL) ? pAAMP->_aamp->aamp : NULL;
	if (privAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() privAAMP not initialized", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "AAMP.timedMetadata - initialization error");
		return JSValueMakeUndefined(context);
	}

	int32_t length = privAAMP->timedMetadata.size();

	JSValueRef* array = new JSValueRef[length];
	for (int32_t i = 0; i < length; i++)
	{
		TimedMetadata item = privAAMP->timedMetadata.at(i);
		JSObjectRef ref = AAMP_JS_CreateTimedMetadata(context, item._timeMS, item._name.c_str(), item._content.c_str(), item._id.c_str(), item._durationMS);
		array[i] = ref;
	}

	JSValueRef prop = JSObjectMakeArray(context, length, array, NULL);
	delete [] array;

	return prop;
}


/**
 * @brief Callback invoked from JS to set the stallErrorCode property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_stallErrorCode(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.stallErrorCode on instances of AAMP");
		return false;
	}

	pAAMP->_aamp->SetStallErrorCode(JSValueToNumber(context, value, exception));
	return true;
}


/**
 * @brief Callback invoked from JS to set the stallTimeout property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_stallTimeout(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.stallTimeout on instances of AAMP");
		return false;
	}

	pAAMP->_aamp->SetStallTimeout(JSValueToNumber(context, value, exception));
	return true;
}


/**
 * @brief Callback invoked from JS to set the reportInterval property value
 * @param[in] context JS exception context
 * @param[in] thisObject JSObject on which to set the property's value
 * @param[in] propertyName JSString containing the name of the property to set
 * @param[in] value JSValue to use as the property's value
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval true if the property was set, otherwise false
 */
static bool AAMP_setProperty_reportInterval(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef value, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.reportInterval on instances of AAMP");
		return false;
	}

	pAAMP->_aamp->SetReportInterval(JSValueToNumber(context, value, exception));
	return true;
}

/**
 * @brief Array containing the AAMP's statically declared value properties
 */
static const JSStaticValue AAMP_static_values[] =
{
	{"closedCaptionEnabled", AAMP_getProperty_closedCaptionEnabled, AAMP_setProperty_closedCaptionEnabled, kJSPropertyAttributeDontDelete },
	{"initialBufferTime", AAMP_getProperty_initialBufferTime, AAMP_setProperty_initialBufferTime, kJSPropertyAttributeDontDelete },
	{"trickPlayEnabled", AAMP_getProperty_trickPlayEnabled, AAMP_setProperty_trickPlayEnabled, kJSPropertyAttributeDontDelete },
	{"EventType", AAMP_getProperty_EventType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"mediaType", AAMP_getProperty_MediaType, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"version", AAMP_getProperty_Version, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"audioLanguage", AAMP_getProperty_AudioLanguage, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"currentDRM", AAMP_getProperty_CurrentDRM, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"timedMetadata", AAMP_getProperty_timedMetadata, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{"stallErrorCode", NULL, AAMP_setProperty_stallErrorCode, kJSPropertyAttributeDontDelete },
	{"stallTimeout", NULL, AAMP_setProperty_stallTimeout, kJSPropertyAttributeDontDelete },
	{"reportInterval", NULL, AAMP_setProperty_reportInterval, kJSPropertyAttributeDontDelete },
	{NULL, NULL, NULL, 0}
};


/**
 * @brief Callback invoked from JS to get the type property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef Event_getProperty_type(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	const AAMPEvent* pEvent = (const AAMPEvent*)JSObjectGetPrivate(thisObject); 
	if (pEvent == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call Event.type on instances of AAMPEvent");
		return JSValueMakeUndefined(context);
	}
	return JSValueMakeNumber(context, pEvent->type);
}


/**
 * @brief Array containing the Event's statically declared value properties
 */
static const JSStaticValue Event_staticprops[] =
{
	{ "type", Event_getProperty_type, NULL, kJSPropertyAttributeReadOnly },
	{ NULL, NULL, NULL, 0 }
};


/**
 * @brief Array containing the Event's statically declared functions
 */
static const JSStaticFunction Event_staticfunctions[] =
{
	{ NULL, NULL, 0 }
};


/**
 * @brief Callback invoked from JS when an object of Event is first created
 * @param[in] ctx JS execution context
 * @param[in] thisObject JSObject being created
 */
static void Event_init(JSContextRef ctx, JSObjectRef thisObject)
{
	//LOG("[AAMP_JS] %s()", __FUNCTION__);
}


/**
 * @brief Callback invoked when an object of Event is finalized
 * @param[in] thisObj JSObject being finalized
 */
static void Event_finalize(JSObjectRef thisObject)
{
	//noisy - large (>400) burst of logging seen during garbage collection
	//LOG("[AAMP_JS] %s()", __FUNCTION__);

	const AAMPEvent* pEvent = (const AAMPEvent*)JSObjectGetPrivate(thisObject); 
	JSObjectSetPrivate(thisObject, NULL);
}


static JSClassRef Event_class_ref();


/**
 * @brief callback invoked when Event is used along with 'new'
 * @param[in] context JS execution context
 * @param[in] constructor JSObject that is the constructor being called
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSObject that is the constructor's return value
 */
static JSObjectRef Event_constructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* execption)
{
	//LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSObjectMake(ctx, Event_class_ref(), NULL);
}


/**
 * @brief Structure contains properties and callbacks of Event object
 */
static const JSClassDefinition Event_object_def =
{
	0,
	kJSClassAttributeNone,
	"__Event__AAMP",
	NULL,
	Event_staticprops,
	Event_staticfunctions,
	Event_init,
	Event_finalize,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	Event_constructor,
	NULL,
	NULL
};


/**
 * @brief Creates a JavaScript class of Event for use with JSObjectMake
 * @retval singleton instance of JavaScript class created
 */
static JSClassRef Event_class_ref() {
	static JSClassRef _classRef = NULL;
	if (!_classRef) {
		_classRef = JSClassCreate(&Event_object_def);
	}
	return _classRef;
}

/**
 * @class AAMP_JSListener
 * @brief Event listener impl for AAMP events
 */
class AAMP_JSListener : public AAMPEventListener
{
public:

	static void AddEventListener(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback);

	static void RemoveEventListener(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback);

	/**
	 * @brief AAMP_JSListener Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_JSListener(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback)
		: _aamp(aamp)
		, _type(type)
		, _jsCallback(jsCallback)
		, _pNext(NULL)
	{
		LOG("[AAMP_JS] %s() ctx=%p, type=%d, jsCallback=%p", __FUNCTION__, _aamp->_ctx, _type, _jsCallback);
		JSValueProtect(_aamp->_ctx, _jsCallback);
	}


	/**
	 * @brief AAMP_JSListener Destructor
	 */
	virtual ~AAMP_JSListener()
	{
		LOG("[AAMP_JS] %s() ctx=%p, type=%d, jsCallback=%p", __FUNCTION__, _aamp->_ctx, _type, _jsCallback);
		JSValueUnprotect(_aamp->_ctx, _jsCallback);
	}

	/**
	 * @brief AAMP_JSListener Copy Constructor
	 */
	AAMP_JSListener(const AAMP_JSListener&) = delete;

	/**
	 * @brief AAMP_JSListener Assignment operator overloading
	 */
	AAMP_JSListener& operator=(const AAMP_JSListener&) = delete;

	/**
	 * @brief Dispatch JS event for the corresponding AAMP event
	 * @param[in] e AAMP event object
	 */
	void Event(const AAMPEvent& e)
	{
		if(e.type != AAMP_EVENT_PROGRESS && e.type != AAMP_EVENT_AD_PLACEMENT_PROGRESS)//log all events except progress which spams
			ERROR("[AAMP_JS] %s() ctx=%p, type=%d, jsCallback=%p", __FUNCTION__, _aamp->_ctx, e.type, _jsCallback);

		JSObjectRef eventObj = JSObjectMake(_aamp->_ctx, Event_class_ref(), NULL);
		if (eventObj) {
			JSValueProtect(_aamp->_ctx, eventObj);
			JSObjectSetPrivate(eventObj, (void*)&e);
			setEventProperties(e, _aamp->_ctx, eventObj);
			JSValueRef args[1] = { eventObj };
			if (e.type == AAMP_EVENT_AD_RESOLVED)
			{
				if (_aamp->_promiseCallback != NULL)
				{
					JSObjectCallAsFunction(_aamp->_ctx, _aamp->_promiseCallback, NULL, 1, args, NULL);
				}
				else
				{
					ERROR("[AAMP_JS] %s() No promise callback registered ctx=%p, jsCallback=%p", __FUNCTION__, _aamp->_ctx, _aamp->_promiseCallback);
				}
			}
			else
			{
				JSObjectCallAsFunction(_aamp->_ctx, _jsCallback, NULL, 1, args, NULL);
			}
			JSValueUnprotect(_aamp->_ctx, eventObj);
		}
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	virtual void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
	}

public:
	AAMP_JS*			_aamp;
	AAMPEventType		_type;
	JSObjectRef			_jsCallback;
	AAMP_JSListener*	_pNext;
};

/**
 * @class AAMP_JSListener_Progress
 * @brief Event listener impl for REPORT_PROGRESS AAMP event
 */
class AAMP_JSListener_Progress : public AAMP_JSListener
{
public:

	/**
	 * @brief AAMP_JSListener_Progress Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_JSListener_Progress(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef name;

		name = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.progress.durationMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("positionMiliseconds");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.progress.positionMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("playbackSpeed");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.progress.playbackSpeed), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("startMiliseconds");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.progress.startMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("endMiliseconds");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.progress.endMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
	}
};

/**
 * @class AAMP_JSListener_BitRateChanged
 * @brief Event listener impl for BITRATE_CHANGED AAMP event
 */
class AAMP_JSListener_BitRateChanged : public AAMP_JSListener
{
public:

	/**
	 * @brief AAMP_JSListener_BitRateChanged Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_JSListener_BitRateChanged(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef name;
		name = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.bitrateChanged.time), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("bitRate");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.bitrateChanged.bitrate), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("description");
		JSObjectSetProperty(context, eventObj, name, aamp_CStringToJSValue(context,e.data.bitrateChanged.description ), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("width");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.bitrateChanged.width), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("height");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.bitrateChanged.height), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

	}
};

/**
 * @class AAMP_JSListener_SpeedChanged
 * @brief Event listener impl for SPEED_CHANGED AAMP event
 */
class AAMP_JSListener_SpeedChanged : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_SpeedChanged Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_SpeedChanged(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef name;

		name = JSStringCreateWithUTF8CString("speed");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.speedChanged.rate), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("reason");
		JSObjectSetProperty(context, eventObj, name, aamp_CStringToJSValue(context, "unknown"), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
	}
};

/**
 * @class AAMP_JSListener_TuneFailed
 * @brief Event listener impl for TUNE_FAILED AAMP event
 */
class AAMP_JSListener_TuneFailed : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_TuneFailed Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_TuneFailed(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		int code = e.data.mediaError.code;
		const char* description = e.data.mediaError.description;

                JSStringRef name = JSStringCreateWithUTF8CString("code");
                JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, code), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

                name = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(context, eventObj, name, aamp_CStringToJSValue(context, description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

                name = JSStringCreateWithUTF8CString("shouldRetry");
                JSObjectSetProperty(context, eventObj, name, JSValueMakeBoolean(context, e.data.mediaError.shouldRetry), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);
	}
};



/**
 * @class AAMP_JSListener_DRMMetaData
 * @brief
 */
class AAMP_JSListener_DRMMetadata : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_DRMMetadata Constructor
         * @param aamp
         * @param type
         * @param jsCallback
         */
        AAMP_JSListener_DRMMetadata(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
        {
        }

        /**
         * @brief
         * @param e
         * @param context
         * @param eventObj
         * @retval None
         */
        void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
        {
                JSStringRef name;


                int code = e.data.dash_drmmetadata.accessStatus_value;
                const char* description = e.data.dash_drmmetadata.accessStatus;

                ERROR("AAMP_JSListener_DRMMetadata code %d Description %s\n",code,description);
                name = JSStringCreateWithUTF8CString("code");
                JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, code), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

                name = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(context, eventObj, name, aamp_CStringToJSValue(context, description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);
        }
};

/**
 * @class AAMP_JSListener_AnomalyReport
 * @brief
 */
class AAMP_JSListener_AnomalyReport : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_DRMMetadata Constructor
         * @param aamp
         * @param type
         * @param jsCallback
         */
        AAMP_JSListener_AnomalyReport(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
        {
        }

        /**
         * @brief
         * @param e
         * @param context
         * @param eventObj
         * @retval None
         */
        void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
        {
                JSStringRef name;


                int severity = e.data.anomalyReport.severity;
                const char* description = e.data.anomalyReport.msg;

                ERROR("AAMP_JSListener_AnomalyReport severity %d Description %s\n",severity,description);
                name = JSStringCreateWithUTF8CString("severity");
                JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, severity), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

                name = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(context, eventObj, name, aamp_CStringToJSValue(context, description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);
        }
};

/**
 * @class AAMP_JSListener_AnomalyReport
 * @brief
 */
class AAMP_JSListener_MetricsData : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_DRMMetadata Constructor
         * @param aamp
         * @param type
         * @param jsCallback
         */
		AAMP_JSListener_MetricsData(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
        {
        }

        /**
         * @brief
         * @param e
         * @param context
         * @param eventObj
         * @retval None
         */
        void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
        {
                JSStringRef strJSObj;

                strJSObj = JSStringCreateWithUTF8CString("metricType");
                JSObjectSetProperty(context, eventObj, strJSObj, JSValueMakeNumber(context, e.data.metricsData.type), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);

                strJSObj = JSStringCreateWithUTF8CString("traceID");
                JSObjectSetProperty(context, eventObj, strJSObj, aamp_CStringToJSValue(context, e.data.metricsData.metricUUID), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);

                strJSObj = JSStringCreateWithUTF8CString("metricData");
                JSObjectSetProperty(context, eventObj, strJSObj, aamp_CStringToJSValue(context, e.data.metricsData.data), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);
        }
};


/**
 * @class AAMP_JSListener_CCHandleReceived
 * @brief Event listener impl for CC_HANDLE_RECEIVED AAMP event
 */
class AAMP_JSListener_CCHandleReceived : public AAMP_JSListener
{
public:
        /**
         * @brief AAMP_JSListener_CCHandleReceived Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_JSListener_CCHandleReceived(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef name;

		name = JSStringCreateWithUTF8CString("decoderHandle");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.ccHandle.handle), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
	}
};

/**
 * @class AAMP_JSListener_VideoMetadata
 * @brief Event listener impl for VIDEO_METADATA AAMP event
 */
class AAMP_JSListener_VideoMetadata : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_VideoMetadata Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_VideoMetadata(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef name;
		name = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.metadata.durationMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		JSValueRef* array = new JSValueRef[e.data.metadata.languageCount];
		for (int32_t i = 0; i < e.data.metadata.languageCount; i++)
		{
			JSValueRef lang = aamp_CStringToJSValue(context, e.data.metadata.languages[i]);
			array[i] = lang;
			//JSValueRelease(lang);
		}
		JSValueRef prop = JSObjectMakeArray(context, e.data.metadata.languageCount, array, NULL);

		delete [] array;

		name = JSStringCreateWithUTF8CString("languages");
		JSObjectSetProperty(context, eventObj, name, prop, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		array = new JSValueRef[e.data.metadata.bitrateCount];
		for (int32_t i = 0; i < e.data.metadata.bitrateCount; i++)
		{
			array[i] = JSValueMakeNumber(context, e.data.metadata.bitrates[i]);
		}
		prop = JSObjectMakeArray(context, e.data.metadata.bitrateCount, array, NULL); 
		delete [] array;

		name = JSStringCreateWithUTF8CString("bitrates");
		JSObjectSetProperty(context, eventObj, name, prop, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		array = new JSValueRef[e.data.metadata.supportedSpeedCount];
		for (int32_t i = 0; i < e.data.metadata.supportedSpeedCount; i++)
		{
			array[i] = JSValueMakeNumber(context, e.data.metadata.supportedSpeeds[i]);
		}
		prop = JSObjectMakeArray(context, e.data.metadata.supportedSpeedCount, array, NULL);
		delete [] array;

		name = JSStringCreateWithUTF8CString("playbackSpeeds");
		JSObjectSetProperty(context, eventObj, name, prop, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("width");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.metadata.width), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("height");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeNumber(context, e.data.metadata.height), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("hasDrm");
		JSObjectSetProperty(context, eventObj, name, JSValueMakeBoolean(context, e.data.metadata.hasDrm), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
	}
};

/**
 * @class AAMP_JSListener_TimedMetadata
 * @brief Event listener impl for TIMED_METADATA AAMP event
 */
class AAMP_JSListener_TimedMetadata : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_TimedMetadata Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_TimedMetadata(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSObjectRef timedMetadata = AAMP_JS_CreateTimedMetadata(context, e.data.timedMetadata.timeMilliseconds, e.data.timedMetadata.szName, e.data.timedMetadata.szContent, e.data.timedMetadata.id, e.data.timedMetadata.durationMilliSeconds);
        	if (timedMetadata) {
                	JSValueProtect(context, timedMetadata);
			JSStringRef name = JSStringCreateWithUTF8CString("timedMetadata");
			JSObjectSetProperty(context, eventObj, name, timedMetadata, kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
        		JSValueUnprotect(context, timedMetadata);
		}
	}
};

/**
 * @class AAMP_JSListener_StatusChanged
 * @brief Event listener for STATUS_CHANGED AAMP event
 */
class AAMP_JSListener_StatusChanged : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_StatusChanged Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_StatusChanged(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("state");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.stateChanged.state), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

	}
};


/**
 * @class AAMP_JSListener_SpeedsChanged
 * @brief Event listener impl for SPEEDS_CHANGED AAMP event
 */
class AAMP_JSListener_SpeedsChanged : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_SpeedsChanged Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_SpeedsChanged(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSValueRef* array = new JSValueRef[e.data.speedsChanged.supportedSpeedCount];
		for (int32_t i = 0; i < e.data.speedsChanged.supportedSpeedCount; i++)
		{
			array[i] = JSValueMakeNumber(context, e.data.speedsChanged.supportedSpeeds[i]);
		}
		JSValueRef prop = JSObjectMakeArray(context, e.data.speedsChanged.supportedSpeedCount, array, NULL);
		delete [] array;

		JSStringRef name = JSStringCreateWithUTF8CString("playbackSpeeds");
		JSObjectSetProperty(context, eventObj, name, prop, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
	}
};


/**
 * @class AAMP_JSListener_AdResolved
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdResolved : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdResolved Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdResolved(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("resolvedStatus");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeBoolean(context, e.data.adResolved.resolveStatus), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("placementId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adResolved.adId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("placementStartTime");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adResolved.startMS), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("placementDuration");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adResolved.durationMs), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdReservationStart
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdReservationStart : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdReservationStart Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdReservationStart(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adbreakId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adReservation.adBreakId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adReservation.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdReservationEnd
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdReservationEnd : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdReservationEnd Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdReservationEnd(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adbreakId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adReservation.adBreakId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adReservation.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdPlacementStart
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdPlacementStart : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdPlacementStart Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdPlacementStart(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adPlacement.adId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adPlacement.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdPlacementEnd
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdPlacementEnd : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdPlacementEnd Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdPlacementEnd(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adPlacement.adId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adPlacement.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdProgress
 * @brief Event listener impl for REPORT_AD_PROGRESS AAMP event
 */
class AAMP_JSListener_AdProgress : public AAMP_JSListener
{
public:

	/**
	 * @brief AAMP_JSListener_AdProgress Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_JSListener_AdProgress(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
         *
	 * @param[in] e         AAMP event object
	 * @param[in] context   JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adPlacement.adId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adPlacement.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @class AAMP_JSListener_AdPlacementEror
 * @brief Event listener impl for AD_RESOLVED AAMP event
 */
class AAMP_JSListener_AdPlacementEror : public AAMP_JSListener
{
public:

        /**
         * @brief AAMP_JSListener_AdPlacementEror Constructor
         * @param[in] aamp instance of AAMP_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
         */
	AAMP_JSListener_AdPlacementEror(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback) : AAMP_JSListener(aamp, type, jsCallback)
	{
	}

	/**
	 * @brief Set JS event properties
	 * @param[in] e AAMP event object
	 * @param[in] context JS execution context
	 * @param[out] eventObj JS event object
	 */
	void setEventProperties(const AAMPEvent& e, JSContextRef context, JSObjectRef eventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, eventObj, prop, aamp_CStringToJSValue(context, e.data.adPlacement.adId), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adPlacement.position), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("error");
		JSObjectSetProperty(context, eventObj, prop, JSValueMakeNumber(context, e.data.adPlacement.errorCode), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};

/**
 * @brief Callback invoked from JS to add an event listener for a particular event
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_addEventListener(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);

	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.addEventListener on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount >= 2)
	{
		JSObjectRef callbackObj = JSValueToObject(context, arguments[1], NULL);

		if (callbackObj != NULL && JSObjectIsFunction(context, callbackObj))
		{
			char* type = aamp_JSValueToCString(context, arguments[0], NULL);
			AAMPEventType eventType = aamp_getEventTypeFromName(type);
			LOG("[AAMP_JS] %s() eventType='%s', %d", __FUNCTION__, type, eventType);

			if ((eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
			{
				AAMP_JSListener::AddEventListener(pAAMP, eventType, callbackObj);
			}

			delete[] type;
		}
		else
		{
			ERROR("[AAMP_JS] %s() callbackObj=%p, JSObjectIsFunction(context, callbackObj)=%d", __FUNCTION__, callbackObj, JSObjectIsFunction(context, callbackObj));
			*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.addEventListener' - parameter 2 is not a function");
		}
	}
	else
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.addEventListener' - 2 arguments required.");
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Adds a JS function as listener for a particular event
 * @param[in] jsObj instance of AAMP_JS
 * @param[in] type event type
 * @param[in] jsCallback callback to be registered as listener
 */
void AAMP_JSListener::AddEventListener(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback)
{
	LOG("[AAMP_JS] %s(%p, %d, %p)", __FUNCTION__, aamp, type, jsCallback);

	AAMP_JSListener* pListener = 0;

	if(type == AAMP_EVENT_PROGRESS)
	{
		pListener = new AAMP_JSListener_Progress(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_SPEED_CHANGED)
	{
		pListener = new AAMP_JSListener_SpeedChanged(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_CC_HANDLE_RECEIVED)
	{
		pListener = new AAMP_JSListener_CCHandleReceived(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_MEDIA_METADATA)
	{
		pListener = new AAMP_JSListener_VideoMetadata(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_TUNE_FAILED)
	{
		pListener = new AAMP_JSListener_TuneFailed(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_BITRATE_CHANGED)
	{
		pListener = new AAMP_JSListener_BitRateChanged(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_TIMED_METADATA)
	{
		pListener = new AAMP_JSListener_TimedMetadata(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_STATE_CHANGED)
	{
		pListener = new AAMP_JSListener_StatusChanged(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_SPEEDS_CHANGED)
	{
		pListener = new AAMP_JSListener_SpeedsChanged(aamp, type, jsCallback);
	}
	else if (type == AAMP_EVENT_REPORT_ANOMALY)
	{
		pListener = new AAMP_JSListener_AnomalyReport(aamp, type, jsCallback);
	}
	else if (type == AAMP_EVENT_REPORT_METRICS_DATA)
	{
		pListener = new AAMP_JSListener_MetricsData(aamp, type, jsCallback);
	}
	else if (type == AAMP_EVENT_DRM_METADATA)
	{
		pListener = new AAMP_JSListener_DRMMetadata(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_RESOLVED)
	{
		pListener = new AAMP_JSListener_AdResolved(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_RESERVATION_START)
	{
		pListener = new AAMP_JSListener_AdReservationStart(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_RESERVATION_END)
	{
		pListener = new AAMP_JSListener_AdReservationEnd(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_PLACEMENT_START)
	{
		pListener = new AAMP_JSListener_AdPlacementStart(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_PLACEMENT_END)
	{
		pListener = new AAMP_JSListener_AdPlacementEnd(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_PLACEMENT_PROGRESS)
	{
		pListener = new AAMP_JSListener_AdProgress(aamp, type, jsCallback);
	}
	else if(type == AAMP_EVENT_AD_PLACEMENT_ERROR)
	{
		pListener = new AAMP_JSListener_AdPlacementEror(aamp, type, jsCallback);
	}
	else
	{
		pListener = new AAMP_JSListener(aamp, type, jsCallback);
	}

	pListener->_pNext = aamp->_listeners;
	aamp->_listeners = pListener;
	aamp->_aamp->AddEventListener(type, pListener);
}


/**
 * @brief Callback invoked from JS to remove an event listener
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_removeEventListener(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);

	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.removeEventListener on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount >= 2)
	{
		JSObjectRef callbackObj = JSValueToObject(context, arguments[1], NULL);

		if (callbackObj != NULL && JSObjectIsFunction(context, callbackObj))
		{
			char* type = aamp_JSValueToCString(context, arguments[0], NULL);
			AAMPEventType eventType = aamp_getEventTypeFromName(type);
			LOG("[AAMP_JS] %s() eventType='%s', %d", __FUNCTION__, type, eventType);

			if ((eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
			{

				AAMP_JSListener::RemoveEventListener(pAAMP, eventType, callbackObj);
			}

			delete[] type;
		}
		else
		{
			ERROR("[AAMP_JS] %s() InvalidArgument: callbackObj=%p, JSObjectIsFunction(context, callbackObj)=%d", __FUNCTION__, callbackObj, JSObjectIsFunction(context, callbackObj));
			*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.removeEventListener' - parameter 2 is not a function");
		}
	}
	else
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.removeEventListener' - 2 arguments required.");
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Removes a JS listener for a particular event
 * @param[in] aamp instance of AAMP_JS
 * @param[in] type event type
 * @param[in] jsCallback callback to be removed as listener
 */
void AAMP_JSListener::RemoveEventListener(AAMP_JS* aamp, AAMPEventType type, JSObjectRef jsCallback)
{
	LOG("[AAMP_JS] %s(%p, %d, %p)", __FUNCTION__, aamp, type, jsCallback);

	AAMP_JSListener** ppListener = &aamp->_listeners;
	while (*ppListener != NULL)
	{
		AAMP_JSListener* pListener = *ppListener;
		if ((pListener->_type == type) && (pListener->_jsCallback == jsCallback))
		{
			*ppListener = pListener->_pNext;
			aamp->_aamp->RemoveEventListener(type, pListener);
			delete pListener;
			return;
		}
		ppListener = &pListener->_pNext;
	}
}


/**
 * @brief Callback invoked from JS to set AAMP object's properties
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setProperties(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to get AAMP object's properties
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_getProperties(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to start playback for requested URL
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_tune(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.tune on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	char* contentType = NULL;
	bool bFinalAttempt = false;
	bool bFirstAttempt = true;
	switch(argumentCount)
	{
		case 4:
			{
				bFinalAttempt = JSValueToBoolean(context, arguments[3]);
			}
		case 3:
			{
				bFirstAttempt = JSValueToBoolean(context, arguments[2]);
			}
		case 2:
			{
				contentType = aamp_JSValueToCString(context, arguments[1], exception);
			}
		case 1:
			{
				char* url = aamp_JSValueToCString(context, arguments[0], exception);
				pAAMP->_aamp->Tune(url, contentType, bFirstAttempt, bFinalAttempt);
				delete [] url;
			}
			if(NULL != contentType)
			{
				delete [] contentType;
			}
			break;
		default:
			ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1 to 4", __FUNCTION__, argumentCount);
			*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.tune' - 1 argument required");
			break;
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to start playback for requested URL
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_load(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.load on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount == 1 || argumentCount == 2)
	{
		char* contentType = NULL;
		char* strTraceId = NULL;
		bool bFinalAttempt = false;
		bool bFirstAttempt = true;
		if (argumentCount == 2 && JSValueIsObject(context, arguments[1]))
		{
			JSObjectRef argument = JSValueToObject(context, arguments[1], NULL);
			JSStringRef paramName = JSStringCreateWithUTF8CString("contentType");
			JSValueRef paramValue = JSObjectGetProperty(context, argument, paramName, NULL);
			if (JSValueIsString(context, paramValue))
			{
				contentType = aamp_JSValueToCString(context, paramValue, NULL);
			}
			JSStringRelease(paramName);

			paramName = JSStringCreateWithUTF8CString("traceId");
			paramValue = JSObjectGetProperty(context, argument, paramName, NULL);
			if (JSValueIsString(context, paramValue))
			{
				strTraceId = aamp_JSValueToCString(context, paramValue, NULL);
			}
			JSStringRelease(paramName);

			paramName = JSStringCreateWithUTF8CString("isInitialAttempt");
			paramValue = JSObjectGetProperty(context, argument, paramName, NULL);
			if (JSValueIsBoolean(context, paramValue))
			{
				bFirstAttempt = JSValueToBoolean(context, paramValue);
			}
			JSStringRelease(paramName);

			paramName = JSStringCreateWithUTF8CString("isFinalAttempt");
			paramValue = JSObjectGetProperty(context, argument, paramName, NULL);
			if (JSValueIsBoolean(context, paramValue))
			{
				bFinalAttempt = JSValueToBoolean(context, paramValue);
			}
			JSStringRelease(paramName);
		}

		char* url = aamp_JSValueToCString(context, arguments[0], exception);
		pAAMP->_aamp->Tune(url, contentType, bFirstAttempt, bFinalAttempt,strTraceId);

		delete [] url;
		if (contentType)
		{
			delete[] contentType;
		}

		if (strTraceId)
		{
			delete[] strTraceId;
		}
	}
	else
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1 or 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.load' - 1 or 2 argument required");
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to stop active playback
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_stop(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.stop on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	pAAMP->_aamp->Stop();
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set playback rate
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setRate(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setRate on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount < 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setRate' - 2 arguments required");
	}
	else
	{
		int overshoot = 0;
		int rate = (int)JSValueToNumber(context, arguments[0], exception);
		// present JS doesnt support overshoot , check for arguement count and store.
		if(argumentCount > 1)
		{
			overshoot = (int)JSValueToNumber(context, arguments[1], exception);
		}
		LOG("[AAMP_JS] %s () rate=%d, overshoot=%d", __FUNCTION__, rate, overshoot);
		pAAMP->_aamp->SetRate(rate,overshoot);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to perform seek to a particular playback position
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_seek(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.seek on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.seek' - 1 argument required");
	}
	else
	{
		double position = JSValueToNumber(context, arguments[0], exception);
		LOG("[AAMP_JS] %s () position=%g", __FUNCTION__, position);
		pAAMP->_aamp->Seek(position);
	}
	return JSValueMakeUndefined(context);
}



/**
 * @brief Callback invoked from JS to perform seek to live point
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_seekToLive(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.seekToLive on instances of AAMP");
		return JSValueMakeUndefined(context);
	}
	pAAMP->_aamp->SeekToLive();
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set video rectangle co-ordinates
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setRect(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setRect on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 4)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 4", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setRect' - 4 arguments required");
	}
	else
	{
		int x = JSValueToNumber(context, arguments[0], exception);
		int y = JSValueToNumber(context, arguments[1], exception);
		int w = JSValueToNumber(context, arguments[2], exception);
		int h = JSValueToNumber(context, arguments[3], exception);
		pAAMP->_aamp->SetVideoRectangle(x,y,w,h);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set video mute status
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setVideoMute(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__); 
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setVideoMute on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setVideoMute' - 1 argument required");
	}
	else
	{
		bool muted = JSValueToBoolean(context, arguments[0]);
		pAAMP->_aamp->SetVideoMute(muted);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set audio volume
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setAudioVolume(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setAudioVolume on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setAudioVolume' - 1 argument required");
	}
	else
	{
		unsigned int volume = JSValueToNumber(context, arguments[0], exception);
		pAAMP->_aamp->SetAudioVolume(volume);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set preferred zoom setting
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setZoom(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setZoom on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setZoom' - 1 argument required");
	}
	else
	{
		VideoZoomMode zoom;
		char* zoomStr = aamp_JSValueToCString(context, arguments[0], exception);
		LOG("[AAMP_JS] %s() zoomStr %s", __FUNCTION__, zoomStr);
		if (0 == strcmp(zoomStr, "none"))
		{
			zoom = VIDEO_ZOOM_NONE;
		}
		else
		{
			zoom = VIDEO_ZOOM_FULL;
		}
		pAAMP->_aamp->SetVideoZoom(zoom);
		delete[] zoomStr;
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set preferred audio language
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setLanguage(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setLanguage on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setLanguage' - 1 argument required");
	}
	else
	{
		char* lang = aamp_JSValueToCString(context, arguments[0], exception);
		pAAMP->_aamp->SetLanguage(lang);
		delete [] lang;
	}
	return JSValueMakeUndefined(context);
}
 

/**
 * @brief Callback invoked from JS to set list of subscribed tags
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setSubscribeTags(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.subscribedTags on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setSubscribeTags' - 1 argument required");
	}
	else if (!aamp_JSValueIsArray(context, arguments[0]))
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: aamp_JSValueIsArray=%d", __FUNCTION__, aamp_JSValueIsArray(context, arguments[0]));
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setSubscribeTags' - parameter 1 is not an array");
	}
	else
	{
		if(pAAMP->_subscribedTags)
		{
			JSValueUnprotect(pAAMP->_ctx, pAAMP->_subscribedTags);
		}
		pAAMP->_subscribedTags = JSValueToObject(context, arguments[0], NULL);
		if(pAAMP->_subscribedTags)
		{
			JSValueProtect(pAAMP->_ctx, pAAMP->_subscribedTags);
			std::vector<std::string> subscribedTags = aamp_StringArrayToCStringArray(context, arguments[0]);
			pAAMP->_aamp->SetSubscribedTags(subscribedTags);
		}
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to add a custom HTTP header/s
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_addCustomHTTPHeader(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.addCustomHTTPHeader on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 2)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.addCustomHTTPHeader' - 2 argument required");
	}
	else
	{
		char *name = aamp_JSValueToCString(context, arguments[0], exception);
		std::string headerName(name);
		std::vector<std::string> headerVal;

		delete[] name;

		if (aamp_JSValueIsArray(context, arguments[1]))
		{
			headerVal = aamp_StringArrayToCStringArray(context, arguments[1]);
		}
		else if (JSValueIsString(context, arguments[1]))
		{
			headerVal.reserve(1);
			char *value =  aamp_JSValueToCString(context, arguments[1], exception);
			headerVal.push_back(value);
			delete[] value;
		}

		// Don't support empty values now
		if (headerVal.size() == 0)
		{
			ERROR("[AAMP_JS] %s() InvalidArgument: Custom header's value is empty", __FUNCTION__, argumentCount);
			*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.addCustomHTTPHeader' - 2nd argument should be a string or array of strings");
			return JSValueMakeUndefined(context);
		}

		pAAMP->_aamp->AddCustomHTTPHeader(headerName, headerVal);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to remove custom HTTP headers
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_removeCustomHTTPHeader(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.removeCustomHTTPHeader on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.removeCustomHTTPHeader' - 1 argument required");
	}
	else
	{
		char *name = aamp_JSValueToCString(context, arguments[0], exception);
		std::string headerName(name);
		pAAMP->_aamp->AddCustomHTTPHeader(headerName, std::vector<std::string>());
		delete[] name;

	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set an ad
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setAds(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to get list of audio tracks
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_getAudioTrackList(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to get current audio track
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_getAudioTrack(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set audio track
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setAudioTrack(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set CC track
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setClosedCaptionTrack(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return JSValueMakeUndefined(context);
}

/**
 * @brief Callback invoked from JS to set license server URL
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setLicenseServerURL(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setLicenseServerURL on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setLicenseServerURL' - 1 argument required");
	}
	else
	{
		const char *url = aamp_JSValueToCString(context, arguments[0], exception);
		pAAMP->_aamp->SetLicenseServerURL(url);
		delete [] url;
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set the preferred DRM
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setPreferredDRM(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setPreferredDRM on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setPreferredDRM' - 1 argument required");
	}
	else
	{
		const char *drm = aamp_JSValueToCString(context, arguments[0], exception);
		if (strncasecmp(drm, "widevine", 8) == 0)
		{
			pAAMP->_aamp->SetPreferredDRM(eDRM_WideVine);
		}
		if (strncasecmp(drm, "playready", 9) == 0)
		{
			pAAMP->_aamp->SetPreferredDRM(eDRM_PlayReady);
		}
		delete [] drm;
	}
	return JSValueMakeUndefined(context);
}

/**
 * @brief Callback invoked from JS to en/disable anonymous request
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setAnonymousRequest(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setAnonymousRequest on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setAnonymousRequest' - 1 argument required");
	}
	else
	{
		bool isAnonymousReq = JSValueToBoolean(context, arguments[0]);
		pAAMP->_aamp->SetAnonymousRequest(isAnonymousReq);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set vod trickplay FPS
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setVODTrickplayFPS(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
        LOG("[AAMP_JS] %s()", __FUNCTION__);
        AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
        if (!pAAMP)
        {
                ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
                *exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setAnonymousRequest on instances of AAMP");
                return JSValueMakeUndefined(context);
        }

        if (argumentCount != 1)
        {
                ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
                *exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setAnonymousRequest' - 1 argument required");
        }
        else
        {
                int vodTrickplayFPS = (int)JSValueToNumber(context, arguments[0], exception);
                pAAMP->_aamp->SetVODTrickplayFPS(vodTrickplayFPS);
        }
        return JSValueMakeUndefined(context);
}

/**
 * @brief Callback invoked from JS to set alternate playback content URLs
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setAlternateContent(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	ERROR("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.SetAlternateContents() on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 2)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.SetAlternateContent' - 1 argument required");
	}
	else
	{
		/*
		 * Parmater format
		 "reservationObject": object {
		 	"reservationId": "773701056",
			"reservationBehavior": number
			"placementRequest": {
			 "id": string,
		         "pts": number,
		         "url": "",
		     },
		 },
		 "promiseCallback": function
	 	 */
		char *reservationId = NULL;
		int reservationBehavior = -1;
		char *adId = NULL;
		long adPTS = -1;
		char *adURL = NULL;
		if (JSValueIsObject(context, arguments[0]))
		{
			//Parse the ad object
			JSObjectRef reservationObject = JSValueToObject(context, arguments[0], NULL);
			if (reservationObject == NULL)
			{
				ERROR("[AAMP_JS] %s() Unable to convert argument to JSObject", __FUNCTION__);
				return JSValueMakeUndefined(context);
			}
			JSStringRef propName = JSStringCreateWithUTF8CString("reservationId");
			JSValueRef propValue = JSObjectGetProperty(context, reservationObject, propName, NULL);
			if (JSValueIsString(context, propValue))
			{
				reservationId = aamp_JSValueToCString(context, propValue, NULL);
			}
			JSStringRelease(propName);

			propName = JSStringCreateWithUTF8CString("reservationBehavior");
			propValue = JSObjectGetProperty(context, reservationObject, propName, NULL);
			if (JSValueIsNumber(context, propValue))
			{
				reservationBehavior = JSValueToNumber(context, propValue, NULL);
			}
			JSStringRelease(propName);

			propName = JSStringCreateWithUTF8CString("placementRequest");
			propValue = JSObjectGetProperty(context, reservationObject, propName, NULL);
			if (JSValueIsObject(context, propValue))
			{
				JSObjectRef adObject = JSValueToObject(context, propValue, NULL);

				JSStringRef adPropName = JSStringCreateWithUTF8CString("id");
				JSValueRef adPropValue = JSObjectGetProperty(context, adObject, adPropName, NULL);
				if (JSValueIsString(context, adPropValue))
				{
					adId = aamp_JSValueToCString(context, adPropValue, NULL);
				}
				JSStringRelease(adPropName);

				adPropName = JSStringCreateWithUTF8CString("pts");
				adPropValue = JSObjectGetProperty(context, adObject, adPropName, NULL);
				if (JSValueIsNumber(context, adPropValue))
				{
					adPTS = (long) JSValueToNumber(context, adPropValue, NULL);
				}
				JSStringRelease(adPropName);

				adPropName = JSStringCreateWithUTF8CString("url");
				adPropValue = JSObjectGetProperty(context, adObject, adPropName, NULL);
				if (JSValueIsString(context, adPropValue))
				{
					adURL = aamp_JSValueToCString(context, adPropValue, NULL);
				}
				JSStringRelease(adPropName);
			}
			JSStringRelease(propName);
		}

		JSObjectRef callbackObj = JSValueToObject(context, arguments[1], NULL);

		if (callbackObj != NULL && JSObjectIsFunction(context, callbackObj))
		{
			if (pAAMP->_promiseCallback)
			{
				JSValueUnprotect(context, pAAMP->_promiseCallback);
			}
			pAAMP->_promiseCallback = callbackObj;
			JSValueProtect(context, pAAMP->_promiseCallback);
			std::string adBreakId(reservationId);
			std::string adIdStr(adId);
			std::string url(adURL);
			ERROR("[AAMP_JS] Calling pAAMP->_aamp->SetAlternateContents with promiseCallback:%p", callbackObj);
			pAAMP->_aamp->SetAlternateContents(adBreakId, adIdStr, url);
		}
		else
		{
			ERROR("[AAMP_JS] %s() Unable to parse the promiseCallback argument", __FUNCTION__);
		}
		if (reservationId)
		{
			delete[] reservationId;
		}
		if (adURL)
		{
			delete[] adURL;
		}
		if (adId)
		{
			delete[] adId;
		}
	}
	return JSValueMakeUndefined(context);
}

/**
 * @brief Notify AAMP that the reservation is complete
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_notifyReservationCompletion(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	ERROR("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if(!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.notifyReservationCompletion() on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 2)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 2", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.notifyReservationCompletion' - 1 argument required");
	}
	else
	{
		const char * reservationId = aamp_JSValueToCString(context, arguments[0], exception);
		long time = (long) JSValueToNumber(context, arguments[1], exception);
		//Need an API in AAMP to notify that placements for this reservation are over and AAMP might have to trim
		//the ads to the period duration or not depending on time param
		ERROR("[AAMP_JS] %s(): Called reservation close for periodId:%s and time:%d", __FUNCTION__, reservationId, time);
		delete[] reservationId;
	}
	return JSValueMakeUndefined(context);
}

/**
 * @brief Callback invoked from JS to set linear trickplay FPS
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setLinearTrickplayFPS(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
        LOG("[AAMP_JS] %s()", __FUNCTION__);
        AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
        if (!pAAMP)
        {
                ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
                *exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setAnonymousRequest on instances of AAMP");
                return JSValueMakeUndefined(context);
        }

        if (argumentCount != 1)
        {
                ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
                *exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setAnonymousRequest' - 1 argument required");
        }
        else
        {
                int linearTrickplayFPS = (int)JSValueToNumber(context, arguments[0], exception);
                pAAMP->_aamp->SetLinearTrickplayFPS(linearTrickplayFPS);
        }
        return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set live offset
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setLiveOffset(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setLiveOffset on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setLiveOffset' - 1 argument required");
	}
	else
	{
		int liveOffset = (int)JSValueToNumber(context, arguments[0], exception);
		pAAMP->_aamp->SetLiveOffset(liveOffset);
	}
	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set download stall timeout value
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setDownloadStallTimeout(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setDownloadStallTimeout on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setDownloadStallTimeout' - 1 argument required");
	}
	else
	{
		long stallTimeout = (long)JSValueToNumber(context, arguments[0], exception);
		pAAMP->_aamp->SetDownloadStallTimeout(stallTimeout);
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set download start timeout value
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setDownloadStartTimeout(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setDownlodStartTimeout on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setDownloadStartTimeout' - 1 argument required");
	}
	else
	{
		long startTimeout = (long)JSValueToNumber(context, arguments[0], exception);
		pAAMP->_aamp->SetDownloadStartTimeout(startTimeout);
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to set network timeout value
 *
 * @param[in] context JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 *
 * @retval JSValue that is the function's return value
 */
static JSValueRef AAMP_setNetworkTimeout(JSContextRef context, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject);
	if (!pAAMP)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		*exception = aamp_GetException(context, AAMPJS_MISSING_OBJECT, "Can only call AAMP.setNetworkTimeout on instances of AAMP");
		return JSValueMakeUndefined(context);
	}

	if (argumentCount != 1)
	{
		ERROR("[AAMP_JS] %s() InvalidArgument: argumentCount=%d, expected: 1", __FUNCTION__, argumentCount);
		*exception = aamp_GetException(context, AAMPJS_INVALID_ARGUMENT, "Failed to execute 'AAMP.setNetworkTimeout' - 1 argument required");
	}
	else
	{
		long networkTimeout = (long)JSValueToNumber(context, arguments[0], exception);
		pAAMP->_aamp->SetNetworkTimeout(networkTimeout);
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Array containing the AAMP's statically declared functions
 */
static const JSStaticFunction AAMP_staticfunctions[] =
{
	{ "addEventListener", AAMP_addEventListener, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "removeEventListener", AAMP_removeEventListener, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setProperties", AAMP_setProperties, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "getProperties", AAMP_getProperties, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "tune", AAMP_tune, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "load", AAMP_load, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "stop", AAMP_stop, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setRate", AAMP_setRate, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "seek", AAMP_seek, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "seekToLive", AAMP_seekToLive, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setRect", AAMP_setRect, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setZoom", AAMP_setZoom, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setLanguage", AAMP_setLanguage, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setSubscribeTags", AAMP_setSubscribeTags, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setAds", AAMP_setAds, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "subscribeTimedMetadata", AAMP_setSubscribeTags, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "getAudioTrackList", AAMP_getAudioTrackList, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "getAudioTrack", AAMP_getAudioTrack, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setAudioTrack", AAMP_setAudioTrack, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setClosedCaptionTrack", AAMP_setClosedCaptionTrack, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setVideoMute", AAMP_setVideoMute, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setAudioVolume", AAMP_setAudioVolume, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "addCustomHTTPHeader", AAMP_addCustomHTTPHeader, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "removeCustomHTTPHeader", AAMP_removeCustomHTTPHeader, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setLicenseServerURL", AAMP_setLicenseServerURL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setPreferredDRM", AAMP_setPreferredDRM, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setAnonymousRequest", AAMP_setAnonymousRequest, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },

	{ "setVODTrickplayFPS", AAMP_setVODTrickplayFPS, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },

	{ "setLinearTrickplayFPS", AAMP_setLinearTrickplayFPS, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setLiveOffset", AAMP_setLiveOffset, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setDownloadStallTimeout", AAMP_setDownloadStallTimeout, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setDownloadStartTimeout", AAMP_setDownloadStartTimeout, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setNetworkTimeout", AAMP_setNetworkTimeout, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setAlternateContent", AAMP_setAlternateContent, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "notifyReservationCompletion", AAMP_notifyReservationCompletion, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ NULL, NULL, 0 }
};


/**
 * @brief Callback invoked when an object of AAMP is finalized
 * @param[in] thisObj JSObject being finalized
 */
static void AAMP_finalize(JSObjectRef thisObject)
{
	LOG("[AAMP_JS] %s(): object=%p", __FUNCTION__, thisObject);

	AAMP_JS* pAAMP = (AAMP_JS*)JSObjectGetPrivate(thisObject); 
	if (pAAMP == NULL)
	{
		ERROR("[AAMP_JS] %s() Error: JSObjectGetPrivate returned NULL!", __FUNCTION__);
		return;
	}
	JSObjectSetPrivate(thisObject, NULL);

	while (pAAMP->_listeners != NULL)
	{
		AAMPEventType eventType = pAAMP->_listeners->_type;
		JSObjectRef   jsCallback  = pAAMP->_listeners->_jsCallback;

		AAMP_JSListener::RemoveEventListener(pAAMP, eventType, jsCallback);
	}

	if (pAAMP->_ctx != NULL) {
		JSValueUnprotect(pAAMP->_ctx, pAAMP->_eventType);
		if(pAAMP->_subscribedTags) {
			JSValueUnprotect(pAAMP->_ctx, pAAMP->_subscribedTags);
		}
		if(pAAMP->_promiseCallback) {
			JSValueUnprotect(pAAMP->_ctx, pAAMP->_promiseCallback);
		}
	}

	pthread_mutex_lock(&mutex);
	if (NULL != _allocated_aamp)
	{
		_allocated_aamp->Stop();
		LOG("[AAMP_JS] %s:%d delete aamp %p\n", __FUNCTION__, __LINE__, _allocated_aamp);
		delete _allocated_aamp;
		_allocated_aamp = NULL;
	}
	pthread_mutex_unlock(&mutex);
	delete pAAMP;
}


/**
 * @brief Structure contains properties and callbacks of AAMP object
 */
static const JSClassDefinition AAMP_class_def =
{
	0,
	kJSClassAttributeNone,
	"__AAMP__class",
	NULL,
	AAMP_static_values,
	AAMP_staticfunctions,
	NULL,
	AAMP_finalize,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	AAMP_class_constructor,
	NULL,
	NULL
};


/**
 * @brief Creates a JavaScript class of AAMP for use with JSObjectMake
 * @retval singleton instance of JavaScript class created
 */
static JSClassRef AAMP_class_ref() {
	static JSClassRef _classRef = NULL;
	if (!_classRef) {
		_classRef = JSClassCreate(&AAMP_class_def);
	}
	return _classRef;
}


/**
 * @brief Callback invoked from JS to get the AD_PLAYBACK_STARTED property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_AD_PLAYBACK_STARTED(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "adPlaybackStarted");
}


/**
 * @brief Callback invoked from JS to get the AD_PLAYBACK_COMPLETED property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_AD_PLAYBACK_COMPLETED(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "adPlaybackCompleted");
}


/**
 * @brief Callback invoked from JS to get the AD_PLAYBACK_INTERRUPTED property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_AD_PLAYBACK_INTERRUPTED(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "adPlaybackInterrupted");
}


/**
 * @brief Callback invoked from JS to get the BUFFERING_BEGIN property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_BUFFERING_BEGIN(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "bufferingBegin");
}


/**
 * @brief Callback invoked from JS to get the BUFFERING_END property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_BUFFERING_END(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "bufferingEnd");
}


/**
 * @brief Callback invoked from JS to get the DECODER_AVAILABLE property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_DECODER_AVAILABLE(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "decoderAvailable");
}


/**
 * @brief Callback invoked from JS to get the DRM_METADATA_INFO_AVAILABLE property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_DRM_METADATA_INFO_AVAILABLE(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "drmMetadataInfoAvailable");
}





/**
 * @brief Callback invoked from JS to get the DRM_METADATA property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_DRM_METADATA(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
        LOG("[AAMP_JS] %s()", __FUNCTION__);
        return aamp_CStringToJSValue(context, "drmMetadata");
}




/**
 * @brief Callback invoked from JS to get the ENTERING_LIVE property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_ENTERING_LIVE(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "enteringLive");
}


/**
 * @brief Callback invoked from JS to get the MEDIA_OPENED property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_MEDIA_OPENED(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "mediaOpened");
}


/**
 * @brief Callback invoked from JS to get the MEDIA_STOPPED property value
 * @param[in] context JS execution context
 * @param[in] thisObject JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef EventType_getproperty_MEDIA_STOPPED(JSContextRef context, JSObjectRef thisObject, JSStringRef propertyName, JSValueRef* exception)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
	return aamp_CStringToJSValue(context, "mediaStopped");
}


/**
 * @brief Array containing the EventType's statically declared value properties
 */
static const JSStaticValue EventType_staticprops[] =
{
	{ "AD_PLAYBACK_STARTED", EventType_getproperty_AD_PLAYBACK_STARTED, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "AD_PLAYBACK_COMPLETED", EventType_getproperty_AD_PLAYBACK_COMPLETED, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "AD_PLAYBACK_INTERRUPTED", EventType_getproperty_AD_PLAYBACK_INTERRUPTED, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "BUFFERING_BEGIN", EventType_getproperty_BUFFERING_BEGIN, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "BUFFERING_END", EventType_getproperty_BUFFERING_END, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "DECODER_AVAILABLE", EventType_getproperty_DECODER_AVAILABLE, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "DRM_METADATA_INFO_AVAILABLE", EventType_getproperty_DRM_METADATA_INFO_AVAILABLE, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "ENTERING_LIVE", EventType_getproperty_ENTERING_LIVE, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "MEDIA_OPENED", EventType_getproperty_MEDIA_OPENED, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "MEDIA_STOPPED", EventType_getproperty_MEDIA_STOPPED, NULL, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ NULL, NULL, NULL, 0 }
};


/**
 * @brief Callback invoked from JS when an object of EventType is first created
 * @param[in] ctx JS execution context
 * @param[in] thisObject JSObject being created
 */
static void EventType_init(JSContextRef ctx, JSObjectRef thisObject)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
}


/**
 * @brief Callback invoked when an object of EventType is finalized
 * @param[in] thisObj JSObject being finalized
 */
static void EventType_finalize(JSObjectRef thisObject)
{
	LOG("[AAMP_JS] %s()", __FUNCTION__);
}


/**
 * @brief Structure contains properties and callbacks of EventType object
 */
static const JSClassDefinition EventType_object_def =
{
	0,
	kJSClassAttributeNone,
	"__EventType",
	NULL,
	EventType_staticprops,
	NULL,
	EventType_init,
	EventType_finalize,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


/**
 * @brief Creates a JavaScript class of EventType for use with JSObjectMake
 * @retval singleton instance of JavaScript class created
 */
static JSClassRef EventType_class_ref() {
	static JSClassRef _classRef = NULL;
	if (!_classRef) {
		_classRef = JSClassCreate(&EventType_object_def);
	}
	return _classRef;
}


/**
 * @brief Create a EventType JS object
 * @param[in] context JS execute context
 * @retval JSObject of EventType
 */
JSObjectRef AAMP_JS_AddEventTypeClass(JSGlobalContextRef context)
{
	LOG("[AAMP_JS] %s() context=%p", __FUNCTION__, context);

	JSObjectRef obj = JSObjectMake(context, EventType_class_ref(), context);

	return obj;
}


/**
 * @brief Create a TimedMetadata JS object with args passed
 * @param[in] context JS execution context
 * @param[in] timeMS time in milliseconds
 * @param[in] szName name of the metadata tag
 * @param[in] szContent metadata associated with the tag
 * @retval JSObject of TimedMetadata generated
 */
JSObjectRef AAMP_JS_CreateTimedMetadata(JSContextRef context, double timeMS, const char* szName, const char* szContent, const char* id, double durationMS)
{
	JSStringRef name;

	JSObjectRef timedMetadata = JSObjectMake(context, NULL, NULL);
	
	if (timedMetadata) {
		JSValueProtect(context, timedMetadata);
		bool bGenerateID = true;

		name = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, std::round(timeMS)), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		if(!strcmp(szName,"SCTE35") && id && *id != '\0')
		{
			name = JSStringCreateWithUTF8CString("reservationId");
			JSObjectSetProperty(context, timedMetadata, name, aamp_CStringToJSValue(context, id), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
			bGenerateID = false;
		}

		name = JSStringCreateWithUTF8CString("duration");
		JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, (int)durationMS), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("name");
		JSObjectSetProperty(context, timedMetadata, name, aamp_CStringToJSValue(context, szName), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("content");
		JSObjectSetProperty(context, timedMetadata, name, aamp_CStringToJSValue(context, szContent), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		// Force type=0 (HLS tag) for now.
		// Does type=1 ID3 need to be supported?
		name = JSStringCreateWithUTF8CString("type");
		JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, 0), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		// Force metadata as empty object
		JSObjectRef metadata = JSObjectMake(context, NULL, NULL);
		if (metadata) {
                	JSValueProtect(context, metadata);
			name = JSStringCreateWithUTF8CString("metadata");
			JSObjectSetProperty(context, timedMetadata, name, metadata, kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);

			// Parse CUE metadata and TRICKMODE-RESTRICTION metadata
			if ((strcmp(szName, "#EXT-X-CUE") == 0) ||
			    (strcmp(szName, "#EXT-X-TRICKMODE-RESTRICTION") == 0)) {
				const char* szStart = szContent;

				// Advance past #EXT tag.
				for (; *szStart != ':' && *szStart != '\0'; szStart++);
				if (*szStart == ':')
					szStart++;

				// Parse comma seperated name=value list.
				while (*szStart != '\0') {
					char* szSep;
					// Find the '=' seperator.
					for (szSep = (char*)szStart; *szSep != '=' && *szSep != '\0'; szSep++);

					// Find the end of the value.
					char* szEnd = (*szSep != '\0') ? szSep + 1 : szSep;
					for (; *szEnd != ',' && *szEnd != '\0'; szEnd++);

					// Append the name / value metadata.
					if ((szStart < szSep) && (szSep < szEnd)) {
						JSValueRef value;
						char chSave = *szSep;

						*szSep = '\0';
						name = JSStringCreateWithUTF8CString(szStart);
						*szSep = chSave;

						chSave = *szEnd;
						*szEnd = '\0';
						value = aamp_CStringToJSValue(context, szSep+1);
						*szEnd = chSave;

						JSObjectSetProperty(context, metadata, name, value, kJSPropertyAttributeReadOnly, NULL);
						JSStringRelease(name);

						// If we just added the 'ID', copy into timedMetadata.id
						if (szStart[0] == 'I' && szStart[1] == 'D' && szStart[2] == '=') {
							bGenerateID = false;
							name = JSStringCreateWithUTF8CString("id");
							JSObjectSetProperty(context, timedMetadata, name, value, kJSPropertyAttributeReadOnly, NULL);
							JSStringRelease(name);
						}
					}

					szStart = (*szEnd != '\0') ? szEnd + 1 : szEnd;
				}
			}

			// Parse TARGETDURATION and CONTENT-IDENTIFIER metadata
			else {
				const char* szStart = szContent;
				// Advance past #EXT tag.
				for (; *szStart != ':' && *szStart != '\0'; szStart++);
				if (*szStart == ':')
					szStart++;

				// Stuff all content into DATA name/value pair.
				JSValueRef value = aamp_CStringToJSValue(context, szStart);
				if (strcmp(szName, "#EXT-X-TARGETDURATION") == 0) {
					// Stuff into DURATION if EXT-X-TARGETDURATION content.
					name = JSStringCreateWithUTF8CString("DURATION");
				} else {
					name = JSStringCreateWithUTF8CString("DATA");
				}
				JSObjectSetProperty(context, metadata, name, value, kJSPropertyAttributeReadOnly, NULL);
				JSStringRelease(name);
			}
			JSValueUnprotect(context, metadata);
		}

		// Generate an ID
		if (bGenerateID) {
			int hash = (int)timeMS;
			const char* szStart = szName;
			for (; *szStart != '\0'; szStart++) {
				hash = (hash * 33) ^ *szStart;
			}

			char buf[32];
			sprintf(buf, "%d", hash);
			name = JSStringCreateWithUTF8CString("id");
			JSObjectSetProperty(context, timedMetadata, name, aamp_CStringToJSValue(context, buf), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
		}
		JSValueUnprotect(context, timedMetadata);
	}

        return timedMetadata;
}


/**
 *   @brief  Loads AAMP JS object into JS execution context
 *
 *   @param[in]  context - JS execution context
 *   @param[in]  playerInstanceAAMP - Instance of PlayerInstanceAAMP, if to be re-used
 *   @return void
 */
void aamp_LoadJS(void* context, void* playerInstanceAAMP)
{
	INFO("[AAMP_JS] %s() context=%p, aamp=%p", __FUNCTION__, context, playerInstanceAAMP);

	JSGlobalContextRef jsContext = (JSGlobalContextRef)context;

	AAMP_JS* pAAMP = new AAMP_JS();
	pAAMP->_ctx = jsContext;
	if (NULL != playerInstanceAAMP)
	{
		pAAMP->_aamp = (PlayerInstanceAAMP*)playerInstanceAAMP;
	}
	else
	{
		pthread_mutex_lock(&mutex);
		if (NULL == _allocated_aamp )
		{
			_allocated_aamp = new PlayerInstanceAAMP();
			LOG("[AAMP_JS] %s:%d create aamp %p\n", __FUNCTION__, __LINE__, _allocated_aamp);
		}
		else
		{
			LOG("[AAMP_JS] %s:%d reuse aamp %p\n", __FUNCTION__, __LINE__, _allocated_aamp);
		}
		pAAMP->_aamp = _allocated_aamp;
		pthread_mutex_unlock(&mutex);
	}

	pAAMP->_listeners = NULL;

	pAAMP->_eventType = AAMP_JS_AddEventTypeClass(jsContext);
	JSValueProtect(jsContext, pAAMP->_eventType);

	pAAMP->_subscribedTags = NULL;
	pAAMP->_promiseCallback = NULL;
	AAMP_JSListener::AddEventListener(pAAMP, AAMP_EVENT_AD_RESOLVED, NULL);

	JSObjectRef classObj = JSObjectMake(jsContext, AAMP_class_ref(), pAAMP);
	JSObjectRef globalObj = JSContextGetGlobalObject(jsContext);
	JSStringRef str = JSStringCreateWithUTF8CString("AAMP");
	JSObjectSetProperty(jsContext, globalObj, str, classObj, kJSPropertyAttributeReadOnly, NULL);
	JSStringRelease(str);
}


/**
 *   @brief  Removes the AAMP instance from JS context
 *
 *   @param[in]  context - JS execution context
 *   @return void
 */
void aamp_UnloadJS(void* context)
{
	INFO("[AAMP_JS] %s() context=%p", __FUNCTION__, context);

	JSGlobalContextRef jsContext = (JSGlobalContextRef)context;

	JSObjectRef globalObj = JSContextGetGlobalObject(jsContext);
	JSStringRef str = JSStringCreateWithUTF8CString("AAMP");
	JSValueRef aamp = JSObjectGetProperty(jsContext, globalObj, str, NULL);

	if (aamp == NULL)
	{
		JSStringRelease(str);
		return;
	}

	JSObjectRef aampObj = JSValueToObject(jsContext, aamp, NULL);
	if (aampObj == NULL)
	{
		JSStringRelease(str);
		return;
	}

	AAMP_finalize(aampObj);

	JSObjectSetProperty(jsContext, globalObj, str, JSValueMakeUndefined(jsContext), kJSPropertyAttributeReadOnly, NULL);
	JSStringRelease(str);

	// Force a garbage collection to clean-up all AAMP objects.
	LOG("[AAMP_JS] JSGarbageCollect() context=%p", context);
	JSGarbageCollect(jsContext);
}

#ifdef __GNUC__

/**
 * @brief Stops any prevailing AAMP instances before exit of program
 */
void __attribute__ ((destructor(101))) _aamp_term()
{
	LOG("[AAMP_JS] %s:%d\n", __FUNCTION__, __LINE__);
	pthread_mutex_lock(&mutex);
	if (NULL != _allocated_aamp)
	{
		LOG("[AAMP_JS] %s:%d stopping aamp\n", __FUNCTION__, __LINE__);
		_allocated_aamp->Stop();
		LOG("[AAMP_JS] %s:%d stopped aamp\n", __FUNCTION__, __LINE__);
		delete _allocated_aamp;
		_allocated_aamp = NULL;
	}
	pthread_mutex_unlock(&mutex);

	//Clear any active js mediaplayer instances on term
	ClearAAMPPlayerInstances();
}
#endif
