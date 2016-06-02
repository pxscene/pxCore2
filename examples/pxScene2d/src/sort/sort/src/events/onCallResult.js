/*
 * File Name - onAllResult.js
 *
 *  This file defines class for oncallresult events
 *
 */
var XREEvent = require('../receiver/xreEvent.js');

/**
 * Class onCallResult
 **/
var onCallResult = function(status, paramsList, callGuid, msg) {
    var statusCode = status;
    var params = paramsList;
    var callGUID = callGuid;
    var message = msg;
    this.prototype = new XREEvent("onCallResult");
    onCallResult.prototype.constructor = XREEvent;
    //TODO: constructor overloading

    /*
     * addEventParams will create and return object 'eventParams'
     */
    this.addEventParams = function() {
        var eventParams = {};
        eventParams["statusCode"] = statusCode;
        eventParams["params"] = params;
        eventParams["callGUID"] = callGUID;
        eventParams["message"] = message;
        return eventParams;
    };

};


module.exports = onCallResult;