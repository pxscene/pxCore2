/**
 * Classname: Resourceevent.js 
 * Version information: 
 * Date : 
 * Author: 
 * Copyright (C) 2015 - 2016 by TATA Elxsi. All rights reserved.
 */

var XREEvent = require("../receiver/xreEvent.js");
var CONSTANTS = require('./xreprotocol.js');

function ResourceEvent(viewId, eventName) {
    clog("Inside ResourceEvent=========");
    /*if (undefined === name) {
        var other = view;
        this.prototype = new XREEvent(other);
        this.view = other.view;
    } else {
        this.view = view;
        this.prototype = new XREEvent(name);
    }*/
    var view = viewId;
    this.prototype = new XREEvent(eventName);

    this.addEventParams = function() {
        var eventParams = {};
        eventParams["view"] = view;
        return eventParams;
    };
}

function COnReadyEvent() {
    this.prototype = new XREEvent(CONSTANTS.EVT_ON_READY);
    this.eventClass = null;
    //TODO Constructor overloading not implemented
    /*if (typeof eventClass == 'string') {
        this.eventClass = eventClass;
    } else {
        var other = eventClass;
        this.prototype = new XREEvent(other);
        this.eventClass = other.eventClass;
    }*/

    this.addEventParams = function() {
        var eventParams = {};
        eventParams["class"] = this.eventClass;
        return eventParams;
    };
}

/*Timer stop evnt */
function COnTimerStopEvent(timerName, duration) {
    this.prototype = new XREEvent(CONSTANTS.EVT_ON_TIMER_STOP);
    this.timerName = timerName;
    this.duration = duration;
    this.addEventParams = function(eventParams) {
        var eventParams = {};
        eventParams["timerName"] = this.timerName;
        eventParams["duration"] = this.duration;
        return eventParams;
    };

}

function COnHTTPResponseEvent(statuscode, contentType , body) {
     this.prototype = new XREEvent(CONSTANTS.EVT_ON_HTTP_RESPONSE);
     this.statuscode = statuscode;
     this.contentType = contentType;
     this.responseBody = body;
     this.addEventParams = function(eventParams) {
        var eventParams = {};
        eventParams["statuscode"] = this.statuscode;
        clog("##############");
        clog(this.statuscode);
        eventParams["contentType"] = this.contentType;
        eventParams["responseBody"] = this.responseBody;
        return eventParams;
    };
}




/*.......COnGetComplete.......*/

function COnGetComplete(params, context) {
    this.prototype = new XREEvent(CONSTANTS.EVT_ON_GET_COMPLETE);
    COnGetComplete.prototype.constructor = XREEvent;
    this.params = params;
    this.context = context;

    /*this.copy = function() {
        return new COnGetComplete(this.params, this.context);
    };*/

    this.addEventParams = function(eventParams) { 
        clog("addEventParams");
        var eventParams = {};
        eventParams["props"] = this.params;
        eventParams["context"] = this.context;
        return eventParams; 
    };
}

function COnAppDeactivate(other) {
    if (other === undefined) {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_APP_INACTIVATE);
    } else {
        new XREEvent(other);
    }
    this.addEventParams = function() {
        var eventParams = {};
        return eventParams;
    };
}


function COnAppActivate(other) {
    if (undefined === other) {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_APP_ACTIVATE);
    } else {
        this.prototype = new XREEvent(other);
    }

    this.addEventParams = function() {
        var eventParams = {};
        return eventParams;
    };
}

function COnReadLocalObject(params) {
    /*if (params instanceof COnReadLocalObject) {
        var other = params;
        this.prototype = new XREEvent(other);
        this.localObject = other.localObject;
    }
    else {
        var objectMap = params;
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_READ_LOCAL_OBJECT);
        this.localObject = objectMap;
    }*/
    //TODO Changed for testing
    this.prototype = new XREEvent(CONSTANTS.EVT_ON_READ_LOCAL_OBJECT);
    this.localObject = params;
    this.addEventParams = function() {
        var eventParams = {};
        eventParams["localObject"] = this.localObject;
        return eventParams;
    };
}

function COnErrorEvent(errorType, errorDesc) {
    if (undefined === errorDesc) {
        var other = errorType;
        this.prototype = new XREEvent(other);
        this.errorType = other.errorType;
        this.errorDesc = other.errorDesc;
    } else {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_ERROR);
        this.errorType = errorType;
        this.errorDesc = errorDesc;
    }

    this.addEventParams = function(eventParams) {
        eventParams.errorType   = m_errorType;
        eventParams.description = m_errorDesc;
        return  eventParams;
    };
}

var HTMLLinkClickedEvent = function(value) {
    if (value instanceof HTMLLinkClickedEvent) {
        var other = value;
        this.prototype = new XREEvent(other);
        this.value = other.value;
    } else {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_HTML_LINK_CLICKED);
        this.value = value;
    }
    this.addEventParams = function(eventParams) {
        eventParams.value = this.value;
        return eventParams;
    };
};


function COpaqueEvent(params) {

    if (params instanceof COpaqueEvent) {
        other = params;
        this.prototype = new XREEvent(other);
        this.params = other.params;
    } else {
        this.prototype = new XREEvent(params);
        this.params = params.params;
    }

    this.addEventParams = function() {
        var eventParams = {};
        clog("Inside AddEventParams " + this.params);
        for (var item in (this.params)) {
            if ((this.params).hasOwnProperty(item)) {
                eventParams[item] = this.params[item];
                clog("Inside AddEventParams " + eventParams);
            }
        }
        return eventParams;
    };
}

function CGenericEvent (name, params) {
    if (undefined === params ){
        var other = name;
        this.prototype = new XREEvent(other);
        this.params = other.params;
    }
    else {
        this.prototype = new XREEvent(name);
        this.params = params;
    }

    this.addEventParams = function ( ) {     
        var eventParams = {};
        for (var item in (this.params) ) {
            if ((this.params).hasOwnProperty(item)) {
                eventParams [ item ] =  this.params [ item ];
            }
        }
        return eventParams;
    };
}

function COnImageMetadataEvent(width, height) {
    if ( undefined === height) {
        var other = width;
        this.prototype = new XREEvent(other);
        this.width = other.W;
        this.height = other.H;
    } else {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_IMAGE_METADATA);
        this.width = width;
        this.height = height;
    }

    this.addEventParams = function() {
        var eventParams = {};
        eventParams["width"]   = this.width;
        eventParams["height"] = this.height;
        return eventParams;
    };
}

function COnTextMetadataEvent(width, height, ascent, descent, leading) {
    if (height === undefined) {
        var other = width;
        this.prototype = new XREEvent(other);
        this.width = other.width;
        this.height = other.height;
        this.ascent = other.ascent;
        this.descent = other.ascent;
        this.leading = other.leading;
    } else {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_TEXT_METADATA);
        this.width = width;
        this.height = height;
        this.ascent = ascent;
        this.descent = ascent;
        this.leading = leading;
    }

    this.addEventParams = function() {
        var eventParams ={}; 
        eventParams["width"]   = this.width;
        eventParams["height"] = this.height;
        eventParams["ascent"] = this.ascent;
        eventParams["descent"] = this.descent;
        eventParams["leading"] = this.leading;
        return eventParams;
        
    };
}

function COnResourceMeasuredEvent (xvalue, yvalue, width, height) {
    if (yvalue === undefined) {
        var other = xvalue;
        this.prototype = new XREEvent(other);
        this.xvalue = other.X;
        this.yvalue = other.Y;
        this.width = other.W;
        this.height = other.H;
        clog("COnResourceMeasuredEvent : (x:" + xvalue + 'y:' + yvalue + "w:" + width + "h:" + height + ")");
    } else {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_RESOURCE_MEASURED);
        this.xvalue = xvalue;
        this.yvalue = yvalue;
        this.width = width;
        this.height = height;
        clog("COnResourceMeasuredEvent : (x:" + xvalue + 'y:' + yvalue + "w:" + width + "h:" + height + ")");
    }
    this.addEventParams = function(eventParams) {
        eventParams.width = this.width;
        eventParams.height = this.height;
        eventParams.xvalue = this.xvalue;
        eventParams.yvalue = this.yvalue;
        return eventParams;
    };

}

function COnChangedEvent(text){
    clog("Inside COnChangedEvent=====");
    if(text instanceof COnChangedEvent){
        clog("Inside COnChangedEvent=====");
        var other = text;
        this.prototype = new XREEvent(other);
        this.text = other.text;
    }
    else {
        clog("Inside else=====");
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_CHANGED);
        this.text = text;
    }
    this.addEventParams = function() {
        var eventParams= {};
        clog("Inside Add eventparams");
        eventParams["text"] = this.text;
        clog(eventParams);
        return eventParams;
    };

}

module.exports = {
    COpaqueEvent: COpaqueEvent,
    COnResourceMeasuredEvent: COnResourceMeasuredEvent,
    COnImageMetadataEvent: COnImageMetadataEvent,
    ResourceEvent: ResourceEvent,
    COnReadyEvent: COnReadyEvent,
    COnGetComplete: COnGetComplete,
    COnAppDeactivate: COnAppDeactivate,
    COnAppActivate: COnAppActivate,
    COnReadLocalObject: COnReadLocalObject,
    COnErrorEvent: COnErrorEvent,            
    CGenericEvent: CGenericEvent,
    COnTextMetadataEvent: COnTextMetadataEvent,
    COnTimerStopEvent: COnTimerStopEvent,
    COnHTTPResponseEvent:COnHTTPResponseEvent,
    COnChangedEvent: COnChangedEvent
};
