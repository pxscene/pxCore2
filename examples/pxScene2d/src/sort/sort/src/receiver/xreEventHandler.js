/*
 * File Name - xreKeyEvent.js
 *
 *  This file defines class for xre event handler
 *
 */
var XREEvent = require('./xreEvent.js');
var XRElogger = require('../utils/xrelogger.js');

/**
 * Class XREEventHandler
 **/
var XREEventHandler = function() {
    var eventLogger = XRElogger.getLogger("event");
    this.handlers = {};
    this.install = function(params) {
        //Optimization hack. Scan the params list once for anything starting with "on" and assume it's an event. This way we only have to iterate through the params map once
        for (var key in params) {
            if (key.substring(0, 2) == "on") {
                eventLogger.log("info","key ====  "+ key);
                if (this.handlers[key]) {
                    eventLogger.log("info","delete ========  "+ key);
                    delete this.handlers[key];
                }
                if (params[key] !== null && typeof params[key] !== "string") {
                    this.handlers[key] = params[key];
                }
            }

        }
    };
    /*
    *getConfigs will return handler value for particular event
    */
    this.getConfigs = function(event) {
        return this.handlers[event];
    };

    /*
    *isInstalled will return true when an event is registered
    */
    this.isInstalled = function(event) {
        return this.handlers.hasOwnProperty(event);
    };

    /*
    *getConfig will return handler value for particular event
    */
    this.getConfig = function(xreEvent) {
        var eventName = "";
        if (xreEvent instanceof XREEvent) {
            eventName = xreEvent.getName();
        } else {
            eventName = xreEvent.prototype.getName();
        }

        if (!this.isInstalled(eventName)) {
            return null;
        }

        // configs are never empty
        var configs = this.handlers[eventName];
        var eventParams = xreEvent.addEventParams();

        if (configs.hasOwnProperty("filter")) {
            filter = configs["filter"];
            if (filter.keys().length === 0) {
                handlerConfig = configs;
                return handlerConfig;
            }
            

            if (filterParamsMatch(eventParams, filter)) {
                handlerConfig = configs;
                return handlerConfig;
            }
        } else {
            handlerConfig = configs;
            return handlerConfig;
        }

        return null;
    };
    /*
     *add will add new events to handlers
     */
    this.add = function(other) {
        for (var handler in other.handlers) {
            this.handlers[handler.key()] = handler[key];
        }
    };
    /*
     *filterParamsMatch will compare each k
     */
    var filterParamsMatch = function(eventParams, preparedFilter) {
        for (var key in preparedFilter) {
            var value = preparedFilter[key].virtualKeyCode;
            if (eventParams.virtualKeyCode == value) {
                return true;
            }
        }
        return false;
    };


};


module.exports = XREEventHandler;
