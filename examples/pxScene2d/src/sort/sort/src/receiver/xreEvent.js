/*
 * File Name - xreEvent.js
 *
 *  This file defines class for xreEvents
 *
 */
var Constants = require("../utils/config.js");


/**
 * Class XREEvent
 **/
var XREEvent = function(arg) {
    this.name = "";
    this.eventIdx = 0;
    this.sourceIdx = Constants.XRE_ID_ROOT_APPLICATION;
    this.handlerIdx = Constants.XRE_ID_ROOT_APPLICATION;
    this.phase = "STANDARD";
    if (arg) {
        if (arg instanceof XREEvent) {
            this.name = arg.name;
            this.eventIdx = arg.eventIdx;
            //this.sourceIdx = arg.sourceIdx;
            //this.handlerIdx = arg.handlerIdx;
            this.phase = arg.phase;
            this.writer = arg.write;
        } else if (typeof arg == 'string') {
            this.name = arg;
        } else if (typeof arg === "object") {
            this.name = arg.name;
            //this.sourceIdx = parseInt(arg.source);
            //this.handlerIdx = parseInt(arg.handler);
            this.phase = arg.phase;
        }
    }
    /*
     * setEventIndex will set the value of eventIdx with idx
     */
    this.setEventIndex = function(idx) {
        this.eventIdx = idx;
    };
    /*
     * getEventIndex will return the value of eventIdx
     */
    this.getEventIndex = function() {

        return this.eventIdx;
    };
    /*
     * getSourceID will return the value of sourceIdx
     */
    this.getSourceID = function() {
        return this.sourceIdx;
    };
    /*
     * setSourceID will set the value of sourceIdx with ID
     */
    this.setSourceID = function(ID) {
        this.sourceIdx = ID;
    };

    /*
     * getHandlerID will return the value of handlerIdx
     */
    this.getHandlerID = function() {
        return this.handlerIdx;
    };
    /*
     * setHandlerID will set the value of handlerIdx with ID
     */
    this.setHandlerID = function(ID) {
        this.handlerIdx = ID;
    };
    /*
     * getPhase will return the value of phase
     */
    this.getPhase = function() {
        return this.phase;
    };
    /*
     * setPhase will set the value of phase
     */
    this.setPhase = function(phase) {
        this.phase = phase;
    };
    /*
     * getName will return the value of name
     */
    this.getName = function() {
        return this.name;
    };
    /*
     * setName will set the value of name
     */
    this.setName = function(name) {
        this.name = name;
    };

    /*
     * Returns the json Value
     */

    this.toJson = function(connectEvent) {
        var connectJson = this.toMap(connectEvent);
        return connectJson;
    };

    /*
     * Calls the toMap function.
     */

    this.toMap = function(connectEvent) {

        var map = {};
        map["name"] = this.name;
        map["source"] = this.sourceIdx;
        map["handler"] = this.handlerIdx;
        map["event"] = this.eventIdx;
        map["phase"] = this.phase;
		var eventParams ={};
        eventParamsJson = connectEvent.addEventParams(eventParams);
        map["params"] = eventParamsJson;
        var json = JSON.stringify(map);
        return json;
    };


    //virtual CXREEvent * copy() const = 0;

    /*void getEventParams(QVariantHash & map) {
        AddEventParams(map);
    }
    protected:

    virtual void AddEventParams(QVariantHash & map) = 0;
*/
};
module.exports = XREEvent;
