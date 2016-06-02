/***************************************
 *Name: xreCommandSequence.js
 *Date 16-jun-2015
 ***************************************/
var XRECommand = require("./xrecommand.js");
var RESULT = require('../utils/enum.js').RESULT;
var XREObject = require("../receiver/xreObject.js");
var GenericEvent = require('../events/resourceEvent.js').CGenericEvent;
var CONSTANTS = require('../events/xreprotocol.js');

var XRECommandSequence = function(id, app, params) {
    var _this = this;
    this.params = params;
    var commands = [];

    //Calling parent constructor
    XREObject.call(this, id, app);


    /*Method setProperties is used for setting commands */
    this.setProperties = function(params) {
        var commandList = params.commands;
        for (i = 0; i < commandList.length; i++) {
            var command = commandList[i];
            if (command) {
                commands.push(command);
            } 
        }
        return RESULT.XRE_S_OK;
    };

    /*callMethod is used for executing each command in 'commands'*/
    this.callMethod = function(method) {
        if (method == "execute") {
            for (i = 0; i < commands.length; ++i) {
                var commandJson = commands[i];
                var command = XRECommand.createCommand(commandJson);
                this.GetApp().processCommand(command);
            }
            var onCompleteEvent = new GenericEvent(CONSTANTS.EVT_ON_COMMAND_SEQUENCE_COMPLETE, {});
            this.getEmitter().emit('Event', onCompleteEvent, this);
            //emit Event( CGenericEvent(XREProtocol::EVT_ON_COMMAND_SEQUENCE_COMPLETE, XREParams()), this ); ::To do
            return RESULT.XRE_S_OK;
        }
        return RESULT.XRE_E_OBJECT_DOES_NOT_IMPLEMENT_METHOD;
    };

    /*getProperty is used to get the value of corresponding data member'*/
    var getProperty = function(name){
        if(name == "commands"){
            return commands;
        } else {
            return this.GetProperty(name);
        }
    };

    this.setProperties(params);
  };
  
//Inheriting XREObject
XRECommandSequence.prototype = Object.create(XREObject.prototype);
XRECommandSequence.prototype.constructor = XRECommandSequence;
module.exports = XRECommandSequence;
