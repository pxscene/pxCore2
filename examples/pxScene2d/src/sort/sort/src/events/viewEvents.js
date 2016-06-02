/**
 * FileName    : viewEvents.js
 * Created     : 07/13/2015
 * Description : Defines classes to handle view wevents
 **/

var XREEvent = require("../receiver/xreEvent.js");
var CONSTANTS = require('./xreprotocol.js');
/**
 *	Class for XREView activate event
 **/
var XREViewActivateEvent = function(event) {
    if (event === undefined) {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_ACTIVATE);
    } else {
        this.prototype = new XREEvent(event);
    }

};
/**
 *	Class for XREView deactivate event
 **/
var XREViewDeactivateEvent = function() {
    if (event === undefined) {
        this.prototype = new XREEvent(CONSTANTS.EVT_ON_INACTIVATE);
    } else {
        this.prototype = new XREEvent(event);
    }
};

//Exporting objects
module.exports = {
    XREViewActivateEvent: XREViewActivateEvent,
    XREViewDeactivateEvent: XREViewDeactivateEvent
};
