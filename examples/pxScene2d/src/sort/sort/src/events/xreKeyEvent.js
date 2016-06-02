/*
 * File Name - xreKeyEvent.js
 *
 *  This file defines class for xre Key events
 *
 */
var XREEvent = require('../receiver/xreEvent.js');

/**
 * Class XREKeyEvent
 **/
var XREKeyEvent = function(arg1, arg2, arg3, arg4, arg5, arg6) {
    var key = "";
    var rawKey = -1;
    var alt = false;
    var ctl = false;
    var shift = false;
    var meta = false;
    this.prototype = new XREEvent();
    XREKeyEvent.prototype.constructor = XREEvent;
    if (arg1 instanceof XREKeyEvent) {
        //clog("inside instanceof XREKeyEvent");
        key = arg1.key;
        rawKey = arg1.rawKey;
        alt = arg1.alt;
        ctl = arg1.ctl;
        shift = arg1.shift;
        meta = arg1.meta;
        this.prototype.setEventIndex(arg1.prototype.eventIdx);
        this.prototype.setSourceID(arg1.prototype.sourceIdx);
        this.prototype.setHandlerID(arg1.prototype.handlerIdx);
        this.prototype.setPhase(arg1.prototype.phase);
        this.prototype.setName(arg1.prototype.name);

    } else if (typeof arg2 != 'undefined' && arg3 != 'undefined' && arg4 != 'undefined' && arg5 != 'undefined' && arg6 != 'undefined') {
        key = arg1;
        rawKey = arg2;
        alt = arg3;
        ctl = arg4;
        shift = arg5;
        meta = arg6;
    }
    /*
     * getKey will return the key value
     */
    this.getKey = function() {
        return key;
    };

    /*
     * getEventParams will create and return object 'eventParams'
     */
    this.getEventParams = function() {
        var eventParams = {};
        eventParams["virtualKeyCode"] = key;
        eventParams["rawCode"] = rawKey;
        eventParams["alt"] = alt;
        eventParams["control"] = ctl;
        eventParams["shift"] = shift;
        eventParams["meta"] = meta;
        return eventParams;
    };
    /*
     * addEventParams will return object 'eventParams'
     */
    this.addEventParams = function() {
        return this.getEventParams();
    };


};

module.exports = XREKeyEvent;
