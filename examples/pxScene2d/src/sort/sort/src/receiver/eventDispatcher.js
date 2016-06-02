/*
 * File Name - eventDispatcher.js
 *
 *  This file defines class for event dispatcher
 *
 */

var Constants = require("../utils/config.js");
var XREView = require("./xreView.js");
var XREEvent = require("./xreEvent.js");

/**
 * Class EventDispatcher
 **/
var EventDispatcher = function() {
    this.parents = [];
    this.SendEvent = function(event, porigin, bPreviewOnly) {
        if (typeof bPreviewOnly === 'undefined') {
            bPreviewOnly = false;
        }
        if (porigin instanceof XREView) {
            dispatchViewEvent(porigin, event, bPreviewOnly);
        } else {
            dispatchResourceEvent(porigin, event);
        }
    };

    var deepCopy = function(obj) {
        if (Object.prototype.toString.call(obj) === '[object Array]') {
            var out = [];
            var i;
            len = obj.length;
            for (; i < len; i++) {
                out[i] = arguments.callee(obj[i]);
            }
            return out;
        }
        if (typeof obj === 'object') {
            var out = {};
            var i;
            for (i in obj) {
                out[i] = arguments.callee(obj[i]);
            }
            return out;
        }
        return obj;
    };

    /*
     *function to dispatch the view events
     */
    var dispatchViewEvent = function(origin, pXREEvent, bPreviewOnly) {
        if (origin && origin !== "null" && origin !== "undefined") {
            var standardEvent = deepCopy(pXREEvent);
            dispatchPreviewViewEvent(origin, pXREEvent);
            if (!bPreviewOnly) dispatchStandardViewEvent(origin, standardEvent);
        }
    };

    /*
     *function to dispatch the resource events
     */
    var dispatchResourceEvent = function(origin, xreEvent) {
        clog("Inside dispatchResourceEvent");
        if (origin && origin !== "null" && origin !== "undefined") {
            var object = origin;
            while (object) {
                //Send events to the owning app's connection
                var app = object.GetApp();
                var connection = app === null ? null : app.getConnection();

                if (null === connection){
                    continue;
                }
                var handlerConfig = object.GetHandlerConfig(xreEvent);
                if (handlerConfig !== null) {
                    var pXREEvent = xreEvent;
                    pXREEvent.prototype.setSourceID(object.GetId());
                    pXREEvent.prototype.setHandlerID(object.GetId());
                    sendEvent(object, pXREEvent, handlerConfig); 
                    break;
                }

                if (app !== null) {
                    handlerConfig = app.GetHandlerConfig(xreEvent);
                    if (handlerConfig !== null) {
                        var pXREEvent = xreEvent;
                        pXREEvent.prototype.setSourceID(object.GetId());
                        pXREEvent.prototype.setHandlerID(app.GetId());
                        sendEvent(object, pXREEvent, handlerConfig); 
                        break;
                    }
                }

                object = object.GetApp().getResourceContainer();
            }
        }
    };
    /*
     *function to send event to server , if no command sequnce registered for particular event
     */
    var sendEvent = function(handler, event, config) {
        if (handler && handler !== "null" && handler !== "undefined") {
            var seqId = 0;
            if (config.hasOwnProperty("commandSequence")) {
                seqId = parseInt(config.commandSequence);
            }

            //If we have a client-side event handler, execute that instead of sending the event to the host
            if (seqId !== 0) {
                var pEvent = new XREEvent(event);

                var seq = handler.prototype.GetApp().getCommandSequence(seqId);
                if (seq) {
                    seq.callMethod("execute");
                } else { //TODO

                    /*CScriptResource * script = dynamic_cast < CScriptResource * > ((handler - > GetApp() - > GetResource(seqId)));
                    if (script)
                        script - > Execute(event);*/
                }
            } else {
                //TODO: modify inheritace.. in xreView 
                if(handler instanceof XREView){
                    handler.prototype.GetApp().emitSendEvent(event); // will delete ptr
                }else{
                    handler.GetApp().emitSendEvent(event);
                }
            }
        }
    };
    /*getParentList:
     *Recursivly populates a list of a particular view's parents
     */
    
    var getParentList = function(view, source) {
        if (!view) {
            return;
        }

        var element = {};
        element["view"] = view;
        element["source"] = source;
        this.parents.unshift(element);
        var nextview = null;
        var nextsource = 0;
        //Create a list up through child apps via their containing views
        if (view.prototype.GetId() == Constants.XRE_ID_ROOT_VIEW) {
            nextview = view.prototype.GetApp().getContainingView();
            if (nextview == null) { //Stop when we hit the root app (which has no containing view)
                return;
            }
            nextsource = nextview.prototype.GetId();
        } else {
            nextview = view.getParentObject();
            nextsource = source;
        }
        getParentList(nextview, nextsource);
    };

    /*
     *function to dispatch Preview view events
     */
    var dispatchPreviewViewEvent = function(origin, xreEvent) {
        var eventName = xreEvent.prototype.getName();
        var previewEventName = "onPreview" + eventName.substring(2);
        this.parents = [];
        getParentList(origin, origin.prototype.GetId());
        var eventHandlerRegistered = false;
        for (var parent in this.parents) {
            var previewHandlerView = this.parents[parent].view;
            if (origin && origin !== "null" && origin !== "undefined") {
                var pPreviewEvent = xreEvent;
                pPreviewEvent.prototype.setName(previewEventName);
                pPreviewEvent.prototype.setHandlerID(previewHandlerView.prototype.GetId());
                pPreviewEvent.prototype.setSourceID(this.parents[parent].source);
                pPreviewEvent.prototype.setPhase("PREVIEW");
                var handlerConfig = previewHandlerView.prototype.GetHandlerConfig(pPreviewEvent);
                if (handlerConfig !== null) {
                    eventHandlerRegistered = true;
                    sendEvent(previewHandlerView, pPreviewEvent, handlerConfig);
                } else {
                    // The below code added for XRE-870
                    /*if (XREGraphicsScene::GetInstance() - > getRootApp() - > getIsRootAppNotConnected() && \
                        XREGraphicsScene::GetInstance() - > getRootApp() - > shouldShowDisconnect()) {
                        XREGraphicsScene::GetInstance() - > getRootApp() - > onSentEventDisconnected();
                    }*/
                }
            }
        }
        /*if (!eventHandlerRegistered && eventName.contains("Key")) {
            XRELOG_WARN("There is no keyevent handler registered for the Preview view. Dropping the key events.");
        }*/
    };

    /*
     *function to dispatch the Standard view events
     */
    var dispatchStandardViewEvent = function(origin, xreEvent) {
        var currentview = origin;
        var sourceID = origin.prototype.GetId();

        var eventHandlerRegistered = true;
        while (currentview) {
            eventHandlerRegistered = false;
            var handlerConfig = currentview.prototype.GetHandlerConfig(xreEvent);
            if (handlerConfig) {
                var pStandardEvent = xreEvent;
                pStandardEvent.prototype.setHandlerID(currentview.prototype.GetId());
                if (xreEvent.prototype.getSourceID() < 2048) { // if the event was created from the view's generateEvent call, don't change the source
                    pStandardEvent.prototype.setSourceID(sourceID);
                }
                pStandardEvent.prototype.setPhase("STANDARD");
                sendEvent(currentview, pStandardEvent, handlerConfig);
                eventHandlerRegistered = true;
                break;
            }

            //If we've hit the top of our heirachy for this app, bubble up throught the app that contians us
            if (currentview.getParentObject() === null) {
                currentview = currentview.prototype.GetApp().getContainingView();
                if (currentview !== null) {
                    sourceID = currentview.prototype.GetId();
                }
            } else {
                currentview = currentview.getParentObject();
            }

        }
        /*if (!eventHandlerRegistered && xreEvent.getName().contains("Key")) {
            XRELOG_WARN("There is no keyevent handler registered for the Standard view. Dropping the key events.");
        }*/
    };
};

module.exports = EventDispatcher;
