/*
 * File Name - xreMouseEvent.js
 *
 *  This file defines class for xre Mouse events
 *
 */
var XREEvent = require('../receiver/xreEvent.js');

/**
 * Class XREKeyEvent
 **/

function XREMouseEvent(name, button, x, y, wheel, wheelOrientation) {
    this.button = button;
    this.x = x;
    this.y = y;
    this.wheel = wheel;
    this.wheelOrientation = wheelOrientation;
    this.prototype = new XREEvent(name);
    this.eventClass = null;
    this.addEventParams = function() {
        var eventParams = {};
        eventParams["button"] = this.button;
        eventParams["X"] = this.x;
        eventParams["Y"] = this.y;
        eventParams["wheelDelta"] = this.wheel;
        eventParams["wheelOrientation"] = this.wheelOrientation;
        return eventParams;
    };
}


var XREDragEvent = function(name, x, y) {
    this.x = x;
    this.y = y;
    this.prototype = new XREEvent(name);
    this.eventClass = null;
    this.addEventParams = function() {
        var eventParams = {};
        eventParams["X"] = this.x;
        eventParams["Y"] = this.y;
        return eventParams;
    };
};


module.exports = {
    XREMouseEvent: XREMouseEvent,
    XREDragEvent: XREDragEvent
};
