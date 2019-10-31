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
 * @file jseventlistener.cpp
 * @brief Event Listner impl for PrivAAMPStruct_JS object
 */


#include "jseventlistener.h"
#include "jsevent.h"
#include "jsutils.h"


/**
 * @class AAMP_Listener_PlaybackStateChanged
 * @brief Event listener impl for AAMP_EVENT_STATE_CHANGED event.
 */
class AAMP_Listener_PlaybackStateChanged : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_PlaybackStateChanged Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_PlaybackStateChanged(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("state");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.stateChanged.state), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}

};


/**
 * @class AAMP_Listener_MediaEndReached
 * @brief Event listener impl for AAMP_EVENT_EOS event.
 */
class AAMP_Listener_MediaEndReached : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_MediaEndReached Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_MediaEndReached(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}
};


/**
 * @class AAMP_Listener_ProgressUpdate
 * @brief Event listener impl for AAMP_EVENT_PROGRESS event.
 */
class AAMP_Listener_ProgressUpdate : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_ProgressUpdate Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_ProgressUpdate(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.progress.durationMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("positionMiliseconds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.progress.positionMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("playbackSpeed");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.progress.playbackSpeed), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("startMiliseconds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.progress.startMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("endMiliseconds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.progress.endMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_SpeedChanged
 * @brief Event listener impl for AAMP_EVENT_SPEED_CHANGED event.
 */
class AAMP_Listener_SpeedChanged : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_SpeedChanged Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_SpeedChanged(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("speed");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.speedChanged.rate), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("reason");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, aamp_CStringToJSValue(p_obj->_ctx, "unknown"), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_BufferingChanged
 * @brief Event listener impl for AAMP_EVENT_BUFFERING_CHANGED event.
 */
class AAMP_Listener_BufferingChanged : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_BufferingChanged Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_BufferingChanged(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("buffering");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeBoolean(p_obj->_ctx, ev.data.bufferingChanged.buffering), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_PlaybackFailed
 * @brief Event listener impl for AAMP_EVENT_TUNE_FAILED event.
 */
class AAMP_Listener_PlaybackFailed : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_PlaybackFailed Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_PlaybackFailed(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

                prop = JSStringCreateWithUTF8CString("recoveryEnabled");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeBoolean(p_obj->_ctx, false), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

                prop = JSStringCreateWithUTF8CString("code");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.mediaError.code), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

                prop = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, aamp_CStringToJSValue(p_obj->_ctx, ev.data.mediaError.description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_MediaMetadata
 * @brief Event listener impl for AAMP_EVENT_MEDIA_METADATA event.
 */
class AAMP_Listener_MediaMetadata : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_MediaMetadata Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_MediaMetadata(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;
		prop = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.metadata.durationMiliseconds), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		JSValueRef* array = new JSValueRef[ev.data.metadata.languageCount];
		for (int32_t i = 0; i < ev.data.metadata.languageCount; i++)
		{
			JSValueRef lang = aamp_CStringToJSValue(p_obj->_ctx, ev.data.metadata.languages[i]);
			array[i] = lang;
		}
		JSValueRef propValue = JSObjectMakeArray(p_obj->_ctx, ev.data.metadata.languageCount, array, NULL);
		delete [] array;

		prop = JSStringCreateWithUTF8CString("languages");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		array = new JSValueRef[ev.data.metadata.bitrateCount];
		for (int32_t i = 0; i < ev.data.metadata.bitrateCount; i++)
		{
			array[i] = JSValueMakeNumber(p_obj->_ctx, ev.data.metadata.bitrates[i]);
		}
		propValue = JSObjectMakeArray(p_obj->_ctx, ev.data.metadata.bitrateCount, array, NULL);
		delete [] array;

		prop = JSStringCreateWithUTF8CString("bitrates");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		array = new JSValueRef[ev.data.metadata.supportedSpeedCount];
		for (int32_t i = 0; i < ev.data.metadata.supportedSpeedCount; i++)
		{
			array[i] = JSValueMakeNumber(p_obj->_ctx, ev.data.metadata.supportedSpeeds[i]);
		}
		propValue = JSObjectMakeArray(p_obj->_ctx, ev.data.metadata.supportedSpeedCount, array, NULL);
		delete [] array;

		prop = JSStringCreateWithUTF8CString("playbackSpeeds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("width");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.metadata.width), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("height");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.metadata.height), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("hasDrm");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeBoolean(p_obj->_ctx, ev.data.metadata.hasDrm), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_SpeedsChanged
 * @brief Event listener impl for AAMP_EVENT_SPEEDS_CHANGED event.
 */
class AAMP_Listener_SpeedsChanged : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_SpeedsChanged Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_SpeedsChanged(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSValueRef* array = new JSValueRef[ev.data.speedsChanged.supportedSpeedCount];
		for (int32_t i = 0; i < ev.data.speedsChanged.supportedSpeedCount; i++)
		{
			array[i] = JSValueMakeNumber(p_obj->_ctx, ev.data.speedsChanged.supportedSpeeds[i]);
		}
		JSValueRef propValue = JSObjectMakeArray(p_obj->_ctx, ev.data.speedsChanged.supportedSpeedCount, array, NULL);
		delete [] array;

		JSStringRef prop = JSStringCreateWithUTF8CString("playbackSpeeds");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}
};


/**
 * @class AAMP_Listener_CCHandleAvailable
 * @brief Event listener impl for AAMP_EVENT_CC_HANDLE_RECEIVED, event.
 */
class AAMP_Listener_CCHandleAvailable : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_CCHandleAvailable Constructor
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_CCHandleAvailable(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
	 * @param[in] e AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("decoderHandle");
		JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, ev.data.ccHandle.handle), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
	}

};


/**
 * @class AAMP_Listener_DRMMetadata
 *
 * @brief Event listener impl for AAMP_EVENT_DRM_METADATA event.
 */
class AAMP_Listener_DRMMetadata : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_DRMMetadata Constructor
	 *
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_DRMMetadata(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
         *
	 * @param[in]  e        AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
                JSStringRef prop;

                int code = ev.data.dash_drmmetadata.accessStatus_value;
                const char* description = ev.data.dash_drmmetadata.accessStatus;

                ERROR("AAMP_Listener_DRMMetadata code %d Description %s\n", code, description);
                prop = JSStringCreateWithUTF8CString("code");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, code), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

                prop = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, aamp_CStringToJSValue(p_obj->_ctx, description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
	}

};


/**
 * @class AAMP_Listener_AnomalyReport
 *
 * @brief Event listener impl for AAMP_EVENT_REPORT_ANOMALY event.
 */
class AAMP_Listener_AnomalyReport : public AAMP_JSEventListener
{
public:
	/**
	 * @brief AAMP_Listener_AnomalyReport Constructor
	 *
         * @param[in] aamp instance of PrivAAMPStruct_JS
         * @param[in] type event type
         * @param[in] jsCallback callback to be registered as listener
	 */
	AAMP_Listener_AnomalyReport(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
		: AAMP_JSEventListener(obj, type, jsCallback)
	{
	}

	/**
	 * @brief Set properties to JS event object
         *
	 * @param[in]  e        AAMP event object
	 * @param[out] eventObj JS event object
	 */
	void SetEventProperties(const AAMPEvent& ev, JSObjectRef jsEventObj)
	{
                JSStringRef prop;

                int severity = ev.data.anomalyReport.severity;
                const char* description = ev.data.anomalyReport.msg;

                ERROR("AAMP_Listener_AnomalyReport severity %d Description %s\n", severity, description);
                prop = JSStringCreateWithUTF8CString("severity");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, JSValueMakeNumber(p_obj->_ctx, severity), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

                prop = JSStringCreateWithUTF8CString("description");
                JSObjectSetProperty(p_obj->_ctx, jsEventObj, prop, aamp_CStringToJSValue(p_obj->_ctx, description), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
	}

};


/**
 * @brief AAMP_JSEventListener Constructor
 * @param[in] obj instance of PrivAAMPStruct_JS
 * @param[in] type event type
 * @param[in] jsCallback callback for the event type
 */
AAMP_JSEventListener::AAMP_JSEventListener(PrivAAMPStruct_JS *obj, AAMPEventType type, JSObjectRef jsCallback)
	: p_obj(obj)
	, p_type(type)
	, p_jsCallback(jsCallback)
{
	JSValueProtect(p_obj->_ctx, p_jsCallback);
}


/**
 * @brief AAMP_JSEventListener Destructor
 */
AAMP_JSEventListener::~AAMP_JSEventListener()
{
	JSValueUnprotect(p_obj->_ctx, p_jsCallback);
}


/**
 * @brief Callback invoked for dispatching event
 * @param[in] e event object
 */
void AAMP_JSEventListener::Event(const AAMPEvent& e)
{
	LOG("%s() type=%d, jsCallback=%p", __FUNCTION__, e.type, p_jsCallback);
	if (e.type < 0 || e.type >= AAMP_MAX_NUM_EVENTS)
	{
		return;
	}

	JSObjectRef event = createNewAAMPJSEvent(p_obj->_ctx, aampPlayer_getNameFromEventType(e.type), false, false);
	if (event)
	{
		JSValueProtect(p_obj->_ctx, event);
		SetEventProperties(e, event);
		aamp_dispatchEventToJS(p_obj->_ctx, p_jsCallback, event);
		JSValueUnprotect(p_obj->_ctx, event);
	}
}



/**
 * @brief Adds a JS function as listener for a particular event
 * @param[in] jsObj instance of PrivAAMPStruct_JS
 * @param[in] type event type
 * @param[in] jsCallback callback to be registered as listener
 */
void AAMP_JSEventListener::AddEventListener(PrivAAMPStruct_JS* obj, AAMPEventType type, JSObjectRef jsCallback)
{
	LOG("AAMP_JSEventListener::%s (%p, %d, %p)", __FUNCTION__, obj, type, jsCallback);

	AAMP_JSEventListener* pListener = NULL;

	switch(type)
	{
		case AAMP_EVENT_EOS:
			pListener = new AAMP_Listener_MediaEndReached(obj, type, jsCallback);
			break;
		case AAMP_EVENT_STATE_CHANGED:
			pListener = new AAMP_Listener_PlaybackStateChanged(obj, type, jsCallback);
			break;
		case AAMP_EVENT_PROGRESS:
			pListener = new AAMP_Listener_ProgressUpdate(obj, type, jsCallback);
			break;
		case AAMP_EVENT_SPEED_CHANGED:
			pListener = new AAMP_Listener_SpeedChanged(obj, type, jsCallback);
			break;
		case AAMP_EVENT_BUFFERING_CHANGED:
			pListener = new AAMP_Listener_BufferingChanged(obj, type, jsCallback);
			break;
		case AAMP_EVENT_TUNE_FAILED:
			pListener = new AAMP_Listener_PlaybackFailed(obj, type, jsCallback);
			break;
		case AAMP_EVENT_MEDIA_METADATA:
			pListener = new AAMP_Listener_MediaMetadata(obj, type, jsCallback);
			break;
		case AAMP_EVENT_SPEEDS_CHANGED:
			pListener = new AAMP_Listener_SpeedsChanged(obj, type, jsCallback);
			break;
		case AAMP_EVENT_CC_HANDLE_RECEIVED:
			pListener = new AAMP_Listener_CCHandleAvailable(obj, type, jsCallback);
			break;
		case AAMP_EVENT_DRM_METADATA:
			pListener = new AAMP_Listener_DRMMetadata(obj, type, jsCallback);
			break;
		case AAMP_EVENT_REPORT_ANOMALY:
			pListener = new AAMP_Listener_AnomalyReport(obj, type, jsCallback);
			break;
		default:
			pListener = new AAMP_JSEventListener(obj, type, jsCallback);
			break;
	}

	if (obj->_aamp != NULL)
	{
		obj->_aamp->AddEventListener(type, pListener);
	}

	obj->_listeners.insert({type, (void *)pListener});
}


/**
 * @brief Removes a JS listener for a particular event
 * @param[in] jsObj instance of PrivAAMPStruct_JS
 * @param[in] type event type
 * @param[in] jsCallback callback to be removed as listener
 */
void AAMP_JSEventListener::RemoveEventListener(PrivAAMPStruct_JS* obj, AAMPEventType type, JSObjectRef jsCallback)
{
	LOG("AAMP_JSEventListener::%s (%p, %d, %p)", __FUNCTION__, obj, type, jsCallback);

	if (obj->_listeners.count(type) > 0)
	{

		typedef std::multimap<AAMPEventType, void*>::iterator listenerIter_t;
		std::pair<listenerIter_t, listenerIter_t> range = obj->_listeners.equal_range(type);
		for(listenerIter_t iter = range.first; iter != range.second; iter++)
		{
			if (iter->second == jsCallback)
			{
				AAMP_JSEventListener *listener = (AAMP_JSEventListener *)iter->second;
				if (obj->_aamp != NULL)
				{
					obj->_aamp->RemoveEventListener(iter->first, listener);
				}
				obj->_listeners.erase(iter);
				delete listener;
			}
		}
	}
}


/**
 * @brief Remove all JS listeners registered
 * @param[in] jsObj instance of PrivAAMPStruct_JS
 */
void AAMP_JSEventListener::RemoveAllEventListener(PrivAAMPStruct_JS * obj)
{
	LOG("AAMP_JSEventListener::%s obj(%p) listeners remaining(%d)", __FUNCTION__, obj, obj->_listeners.size());
	std::multimap<AAMPEventType, void*>::iterator listenerIter;

	for (listenerIter = obj->_listeners.begin(); listenerIter != obj->_listeners.end();)
	{
		AAMP_JSEventListener *listener = (AAMP_JSEventListener *)listenerIter->second;
		if (obj->_aamp != NULL)
		{
			obj->_aamp->RemoveEventListener(listenerIter->first, listener);
		}
		listenerIter = obj->_listeners.erase(listenerIter);
		delete listener;
	}

	obj->_listeners.clear();

}
