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
 * @file jsevent.cpp
 * @brief JavaScript Event Impl for AAMP_JSController and AAMPMediaPlayer_JS
 */


#include "jsevent.h"
#include "jsutils.h"
#include <stdio.h>

//#define _DEBUG

#ifdef _DEBUG
#define LOG(...)  logprintf(__VA_ARGS__);fflush(stdout);
#else
#define LOG(...)
#endif



/**
 * @brief AAMPJSEvent Constructor
 */
AAMPJSEvent::AAMPJSEvent()
	: _bubbles(false)
	, _cancelable(false)
	, _canceled(false)
	, _currentTarget(NULL)
	, _defaultPrevented(false)
	, _phase(pAtTarget)
	, _target(NULL)
	, _timestamp(0)
	, _typeName()
	, _isTrusted(false)
	, _ctx(NULL)
	, _stopImmediatePropagation(false)
	, _stopPropagation(false)
{

}


/**
 * @brief AAMPJSEvent Constructor
 * @param[in] type event type
 * @param[in] bubble true if event is a bubbling event
 * @param[in] cancelable true if event default operation can be cancelled
 */
AAMPJSEvent::AAMPJSEvent(const char *type, bool bubble, bool cancelable)
	: _bubbles(bubble)
	, _cancelable(cancelable)
	, _canceled(false)
	, _currentTarget(NULL)
	, _defaultPrevented(false)
	, _phase(pAtTarget)
	, _target(NULL)
	, _timestamp(0)
	, _typeName(type)
	, _isTrusted(false)
	, _ctx(NULL)
	, _stopImmediatePropagation(false)
	, _stopPropagation(false)
{

}


/**
 * @brief AAMPJSEvent Destructor
 */
AAMPJSEvent::~AAMPJSEvent()
{
	if(_target != NULL)
	{
		JSValueUnprotect(_ctx, _target);
	}

	if(_currentTarget != NULL)
	{
		JSValueUnprotect(_ctx, _currentTarget);
	}
}


/**
 * @brief Initialize event's properties
 * @param[in] type event type
 * @param[in] bubble true if event is a bubbling event
 * @param[in] cancelable true if event default operation can be cancelled
 */
void AAMPJSEvent::initEvent(const char *type, bool bubble, bool cancelable)
{
	_typeName = type;
	_bubbles = bubble;
	_cancelable = cancelable;
}



/**
 * @brief Initialize event's properties based on args passed from JavaScript code.
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject that is the 'this' variable in the function's scope.
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 */
static void initEvent(JSContextRef context, JSObjectRef thisObj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	char* evType;
	bool bubbles = false;
	bool cancelable = false;

	if (argumentCount >= 1 && JSValueIsString(context, arguments[0]))
	{
		evType = aamp_JSValueToCString(context, arguments[0], NULL);

		if (argumentCount >= 2 && JSValueIsObject(context, arguments[1]))
		{
			JSObjectRef eventParams = JSValueToObject(context, arguments[1], NULL);

			JSStringRef bubblesProp = JSStringCreateWithUTF8CString("bubbles");
			JSValueRef bubblesValue = JSObjectGetProperty(context, eventParams, bubblesProp, NULL);
			if (JSValueIsBoolean(context, bubblesValue))
			{
				bubbles = JSValueToBoolean(context, bubblesValue);
			}
			JSStringRelease(bubblesProp);

			JSStringRef cancelableProp = JSStringCreateWithUTF8CString("cancelable");
			JSValueRef cancelableValue = JSObjectGetProperty(context, eventParams, cancelableProp, NULL);
			if (JSValueIsBoolean(context, cancelableValue))
			{
				cancelable = JSValueToBoolean(context, cancelableValue);
			}
			JSStringRelease(cancelableProp);
		}

		AAMPJSEvent* ev = (AAMPJSEvent*) JSObjectGetPrivate(thisObj);

		if (ev && evType != NULL)
		{
			ev->initEvent(evType, bubbles, cancelable);
		}

		delete[] evType;
	}
}


/**
 * @brief Callback invoked from JS to initialize event's properties
 * @param[in] context JS execution context
 * @param[in] func JSObject that is the function being called
 * @param[in] thisObj JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value.
 */
static JSValueRef AAMPJSEvent_initEvent(JSContextRef context, JSObjectRef func, JSObjectRef thisObj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	initEvent(context, thisObj, argumentCount, arguments, exception);

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to cancel the event's default action
 * @param[in] context JS execution context
 * @param[in] func JSObject that is the function being called
 * @param[in] thisObj JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value.
 */
static JSValueRef AAMPJSEvent_preventDefault(JSContextRef context, JSObjectRef func, JSObjectRef thisObj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj != NULL)
	{
		eventObj->preventDefault();
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to prevent other listeners from being called
 * @param[in] context JS execution context
 * @param[in] func JSObject that is the function being called
 * @param[in] thisObj JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value.
 */
static JSValueRef AAMPJSEvent_stopImmediatePropagation(JSContextRef context, JSObjectRef func, JSObjectRef thisObj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj != NULL)
	{
		eventObj->stopImmediatePropagation();
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to prevent further propagation of event during event flow
 * @param[in] context JS execution context
 * @param[in] func JSObject that is the function being called
 * @param[in] thisObj JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value.
 */
static JSValueRef AAMPJSEvent_stopPropagation(JSContextRef context, JSObjectRef func, JSObjectRef thisObj, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj != NULL)
	{
		eventObj->stopPropagation();
	}

	return JSValueMakeUndefined(context);
}


/**
 * @brief Callback invoked from JS to get the bubble property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_bubbles(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeBoolean(context, eventObj->getBubbles());
}


/**
 * @brief Callback invoked from JS to get the cancelable property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_cancelable(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeBoolean(context, eventObj->getCancelable());
}


/**
 * @brief Callback invoked from JS to get the default prevented value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_defaultPrevented(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeBoolean(context, eventObj->getIsDefaultPrevented());
}


/**
 * @brief Callback invoked from JS to get the event phase property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_eventPhase(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeNumber(context, eventObj->getEventPhase());
}


/**
 * @brief Callback invoked from JS to get the target property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_target(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return eventObj->getTarget();
}


/**
 * @brief Callback invoked from JS to get the current target property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_currentTarget(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return eventObj->getCurrentTarget();
}


/**
 * @brief
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_timestamp(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeNumber(context, eventObj->getTimestamp());
}


/**
 * @brief Callback invoked from JS to get the event type
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_type(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return aamp_CStringToJSValue(context, eventObj->getType());
}


/**
 * @brief Callback invoked from JS to get the isTrusted property value
 * @param[in] context JS execution context
 * @param[in] thisObj JSObject to search for the property
 * @param[in] propertyName JSString containing the name of the property to get
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval property's value if object has the property, otherwise NULL
 */
static JSValueRef AAMPJSEvent_getproperty_isTrusted(JSContextRef context, JSObjectRef thisObj, JSStringRef propertyName, JSValueRef* exception)
{
	AAMPJSEvent *eventObj = (AAMPJSEvent *) JSObjectGetPrivate(thisObj);

	if (eventObj == NULL)
	{
		return JSValueMakeUndefined(context);
	}

	return JSValueMakeBoolean(context, eventObj->getIsTrusted());
}


/**
 * @brief Callback invoked from JS when an object is first created
 * @param[in] ctx JS execution context
 * @param[in] object JSObject being created
 */
void AAMPJSEvent_initialize (JSContextRef ctx, JSObjectRef object)
{
        AAMPJSEvent* ev = new AAMPJSEvent();
        JSObjectSetPrivate(object, ev);


}


/**
 * @brief Callback invoked from JS when an object is finalized
 * @param[in] object JSObject being finalized
 */
void AAMPJSEvent_finalize(JSObjectRef object)
{
	AAMPJSEvent *ev = (AAMPJSEvent *) JSObjectGetPrivate(object);

	delete ev;
}

/**
 * @brief Array containing the class's statically declared function properties
 */
static JSStaticFunction AAMPJSEvent_static_functions[] =
{
	{ "initEvent", AAMPJSEvent_initEvent, kJSPropertyAttributeReadOnly },
	{ "preventDefault", AAMPJSEvent_preventDefault, kJSPropertyAttributeReadOnly },
	{ "stopImmediatePropagation", AAMPJSEvent_stopImmediatePropagation, kJSPropertyAttributeReadOnly },
	{ "stopPropagation", AAMPJSEvent_stopPropagation, kJSPropertyAttributeReadOnly },
	{ NULL, NULL, 0 }
};

/**
 * @brief Array containing the class's statically declared value properties
 */
static JSStaticValue AAMPJSEvent_static_values[] =
{
	{ "bubbles", AAMPJSEvent_getproperty_bubbles, NULL, kJSPropertyAttributeReadOnly },
	{ "cancelable", AAMPJSEvent_getproperty_cancelable, NULL, kJSPropertyAttributeReadOnly },
	{ "defaultPrevented", AAMPJSEvent_getproperty_defaultPrevented, NULL, kJSPropertyAttributeReadOnly },
	{ "eventPhase", AAMPJSEvent_getproperty_eventPhase, NULL, kJSPropertyAttributeReadOnly },
	{ "target", AAMPJSEvent_getproperty_target, NULL, kJSPropertyAttributeReadOnly },
	{ "currentTarget", AAMPJSEvent_getproperty_currentTarget, NULL, kJSPropertyAttributeReadOnly },
	{ "timestamp", AAMPJSEvent_getproperty_timestamp, NULL, kJSPropertyAttributeReadOnly },
	{ "type", AAMPJSEvent_getproperty_type, NULL, kJSPropertyAttributeReadOnly },
	{ "isTrusted", AAMPJSEvent_getproperty_isTrusted, NULL, kJSPropertyAttributeReadOnly },
	{ NULL, NULL, NULL, 0 }
};

/**
 * @brief Structure contains properties and callbacks of Event object of AAMP_JSController
 */
static const JSClassDefinition AAMPJSEvent_object_def =
{
	0,
	kJSClassAttributeNone,
	"__Event_AAMPJS",
	NULL,
	AAMPJSEvent_static_values,
	AAMPJSEvent_static_functions,
	AAMPJSEvent_initialize,
	AAMPJSEvent_finalize,
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
 * @brief Callback invoked when an object is used as a constructor in a 'new' expression
 * @param[in] ctx JS execution context
 * @param[in] constructor JSObject that is the constructor being called
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of the args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSObject that is the constructor's return value
 */
static JSObjectRef AAMPJSEvent_class_constructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	JSClassRef classDef = JSClassCreate(&AAMPJSEvent_object_def);
	JSObjectRef eventObj = JSObjectMake(ctx, classDef, NULL);

	initEvent(ctx, eventObj, argumentCount, arguments, exception);
	JSClassRelease(classDef);

	return eventObj;
}

/**
 * @brief Structure contains properties and callbacks of Event class of AAMP_JSController
 */
static const JSClassDefinition AAMPJSEvent_class_def =
{
	0,
	kJSClassAttributeNone,
	"__Event_AAMPJS_class",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	AAMPJSEvent_class_constructor,
	NULL,
	NULL
};


/**
 * @brief To create a new JS event instance
 * @param[in] ctx JS execution context
 * @param[in] type event type
 * @param[in] bubbles denotes if event support bubbling
 * @param[in] cancelable denotes if event is cancelable
 * @retval JSObject of the new instance created
 */
JSObjectRef createNewAAMPJSEvent(JSGlobalContextRef ctx, const char *type, bool bubbles, bool cancelable)
{
        JSClassRef classDef = JSClassCreate(&AAMPJSEvent_object_def);
        JSObjectRef eventObj = JSObjectMake(ctx, classDef, NULL);
	JSClassRelease(classDef);

	AAMPJSEvent *eventPriv = (AAMPJSEvent *) JSObjectGetPrivate(eventObj);
	eventPriv->initEvent(type, bubbles, cancelable);

	return eventObj;
}
