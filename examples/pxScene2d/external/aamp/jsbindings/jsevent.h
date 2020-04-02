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
 * @file jsevent.h
 * @brief JavaScript Event Impl for AAMP_JSController and AAMPMediaPlayer_JS
 */

#ifndef __AAMP_JSEVENT_H__
#define __AAMP_JSEVENT_H__

#include <JavaScriptCore/JavaScript.h>

/**
 * @enum EventPhase
 * @brief Phase of the event flow that is currently being evaluated.
 */
enum EventPhase
{
	pNone = 0,
	pCapturingPhase,  /** Event flow is in capturing phase **/
	pAtTarget,  /** Event flow is in target phase **/
	pBubblingPhase  /** Event flow is in bubbling phase **/
};

/**
 * @class AAMPJSEvent
 * @brief Class represents the native object for a AAMP JS Event
 */
class AAMPJSEvent
{

public:

	AAMPJSEvent();
	AAMPJSEvent(const char *type, bool bubble, bool cancelable);
	AAMPJSEvent(const AAMPJSEvent&) = delete;
	AAMPJSEvent& operator=(const AAMPJSEvent&) = delete;
	~AAMPJSEvent();

	void initEvent(const char *type, bool bubble, bool cancelable);

	/**
	 * @brief Returns the name of the event
	 * @retval event type
	 */
	const char* getType() 	{ return _typeName; }

	/**
	 * @brief Returns whether or not a specific event is a bubbling event
	 * @retval true if event is a bubbling event
	 */
	bool getBubbles()       { return _bubbles; }

	/**
	 * @brief Returns whether or not an event can have its default action prevented
	 * @retval true if default action of event can be prevented
	 */
	bool getCancelable()    { return _cancelable; }

	/**
	 * @brief Returns the element that triggered the event
	 * @retval instance of the element
	 */
	JSObjectRef getTarget() { return _target; }

	/**
	 * @brief Returns the element whose event listeners triggered the event
	 * @retval instance of the element
	 */
	JSObjectRef getCurrentTarget()  { return _currentTarget; }

	/**
	 * @brief Returns which phase of the event flow is currently being evaluated
	 * @retval event phase
	 */
	EventPhase getEventPhase()  { return _phase; }

	/**
	 * @brief Returns whether or not the preventDefault() method was called for the event
	 * @retval true if default was prevented
	 */
	bool getIsDefaultPrevented()  { return _canceled; }

	/**
	 * @brief Returns whether or not an event is trusted
	 * @retval true if event is trusted
	 */
	bool getIsTrusted()           { return _isTrusted; }

	/**
	 * @brief Returns the time (in milliseconds relative to the epoch) at which the event was created
	 * @retval timestamp
	 */
	double getTimestamp()       { return _timestamp; }

	/**
	 * @brief Cancels the event if it is cancelable
	 */
	void preventDefault()
	{
		if(_cancelable)
			_canceled = true;
	}

	/**
	 * @brief Prevents other listeners of the same event from being called
	 */
	void stopImmediatePropagation()
	{
		_stopImmediatePropagation = true;
	}

	/**
	 * @brief Prevents further propagation of an event during event flow
	 */
	void stopPropagation()
	{
		_stopPropagation = true;
	}

	/**
	 * @brief Set the target instance
	 * @param[in] context JS execution context
	 * @param[in] obj target instance
	 */
	void setTarget(JSContextRef context, JSObjectRef obj)
	{
		if (_ctx == NULL)
		{
			_ctx = context;
		}
		if (_target)
		{
			JSValueUnprotect(_ctx, _target);
		}
		_target = obj;
		JSValueProtect(_ctx, _target);
	}

	/**
	 * @brief Set the current target instance
	 * @param[in] context JS execution context
	 * @param[in] obj current target instance
	 */
	void setCurrentTarget(JSContextRef context, JSObjectRef obj)
	{
		if (_ctx == NULL)
		{
			_ctx = context;
		}
		if (_currentTarget)
		{
			JSValueUnprotect(_ctx, _currentTarget);
		}
		_currentTarget = obj;
		JSValueProtect(_ctx, _currentTarget);
	}

private:
	bool _bubbles;  /** denotes if event is a bubbling event **/
	bool _cancelable;  /** denotes if event default operation can be cancelled **/
	bool _canceled;  /** denotes if event default operation was cancelled **/
	JSObjectRef _currentTarget;  /** element whose event listeners triggered the event **/
	bool _defaultPrevented;
	EventPhase _phase;  /** phase of the event flow is currently being evaluated **/
	JSObjectRef _target;  /** element that triggered the event **/
	double _timestamp;  /** event timestamp **/
	const char * _typeName;  /** event type name **/
	bool _isTrusted;  /** denotes if event is trusted or not **/

	bool _stopImmediatePropagation;  /** denotes if other listeners need not be called **/
	bool _stopPropagation;  /** denotes if further propagation of an event to be prevented **/

	JSContextRef _ctx;  /** JS execution context **/
};


JSObjectRef createNewAAMPJSEvent(JSGlobalContextRef ctx, const char *type, bool bubbles, bool cancelable);

#endif // __AAMP_JSEVENT_H__
