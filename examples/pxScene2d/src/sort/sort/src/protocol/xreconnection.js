/**
 * Classname: XREConnection
 * Version information:
 * Date :
 * Author:
 * Copyright (C) 2015 - 2016 by TATA Elxsi. All rights reserved.
 */

var XREapplication = require("./xreapplication.js");
var net = require('net');
var events = require('events');
var XREKeyEvent = require('../events/xreKeyEvent.js');
var XRElogger = require('../utils/xrelogger.js');
var XRE_LEN_HEADER_SIZE = 4;


/**
 * The class provides functionality to handle the
 * connections with XRE server. It sends the onConnect event to the
 * server and receive server response
 */

function XREConnection(app) {

    var client = undefined;
    var eventEmitter = new events.EventEmitter();
    var eventCount = 0;
    var isSocketWritable = false;
    var commandArray = [];
    // var commandBuffer = new Buffer(XRE_LEN_HEADER_SIZE);
    var jsonArray = [];
    var j = 0;
    this.app = app;
    var commandLogger = XRElogger.getLogger("command");
    var eventLogger = XRElogger.getLogger("event");
    var defaultLogger = XRElogger.getLogger("default");
    var writeInt = function(value) {
        var bytes = new Array(4);
        bytes[0] = value >> 24;
        bytes[1] = value >> 16;
        bytes[2] = value >> 8;
        bytes[3] = value;

        return bytes;
    };
    //Method to check whether an event is Key event
    var isKeyEvent = function(event) {
        if (event instanceof XREKeyEvent) {
            return true;
        } else {
            return false;
        }

    };
    this.getEmitter = function() {
        return eventEmitter;
    };

    this.connect = function(HOST, PORT) {
        client = new net.Socket();
        client.connect(PORT, HOST, function() {
            
            defaultLogger.log("info",'\n' + 'CONNECTED TO: ' + HOST + ':' + PORT + '\n');
            client.write('XRE\r\n');
            app.getAppEmitter().emit('connected', client);
            isSocketWritable = true;
        });

        client.on('data', function(data) {

            var readBuf = new Buffer(data);
            var readBuf1 = new Buffer(data);


            for (var i = 0; i < readBuf.length; i++) {
                if (commandArray.length != XRE_LEN_HEADER_SIZE) {
                    //FIX
                    var header_read_length = commandArray.length;
                    var bufferCounter = i;
                    for (var count = 0; count < (XRE_LEN_HEADER_SIZE - header_read_length) && (bufferCounter < readBuf1.length); count++) {
                        commandArray[header_read_length + count] = readBuf1[bufferCounter];
                        bufferCounter++;
                    }
                    i = (bufferCounter - 1);

                    // FIX END
                    if (commandArray.length == XRE_LEN_HEADER_SIZE) {

                        var buff = new Buffer(commandArray);
                        var command = buff.toString('hex', 0, XRE_LEN_HEADER_SIZE);
                        var commandSize = parseInt(command, 16);
                        commandLogger.log("info", "command size== " + commandSize);
                        if (commandSize === 0) {
                            commandLogger.log("info","Got heart beat");
                            commandArray.length = 0;
                        }
                    }
                } else {

                    var buff = new Buffer(commandArray);
                    var command = buff.toString('hex', 0, XRE_LEN_HEADER_SIZE);
                    var commandSize = parseInt(command, 16);
                    jsonArray[j] = readBuf[i];
                    j++;
                    if (jsonArray.length === commandSize) {
                        try {
                            commandLogger.log("info",app.Aname.toUpperCase() + '  ' + new Buffer(jsonArray).toString());
                            app.pushJSON(JSON.parse(new Buffer(jsonArray).toString()));
                            commandArray.length = 0;
                            jsonArray.length = 0;
                            //commandBuffer = new Buffer(XRE_LEN_HEADER_SIZE);
                            j = 0;
                        } catch (e) {
                            commandLogger.log("error",e);
                        }
                    }
                }
            }
	     readBuf = null;
            readBuf1 = null;
        });
        client.on('error', function(data){
            isSocketWritable = false;
            //TODO: callback for reconnecting
        });
    };

    /**
     * After sending the connect header XRE receiver sends the onConnect
     * event. But before each event a four byte data is send indicating the
     * size of the event. So first send the event size.
     */

    var sendEventToServer = function(jsonString) {
        eventLogger.log("info" , "Send event to server  ");
        var flag = 0;
        var bytes = new Array(XRE_LEN_HEADER_SIZE);
        if (client) {
            bytes = writeInt(jsonString.length);
            /*Sent json Message*/
            try {
                if (client.write(new Buffer(bytes))) {
                    var buf = new Buffer(jsonString.length);
                    len = buf.write(jsonString, 0);
                    eventLogger.log("debug","Message " + len + " bytes: " + buf.toString('utf8', 0, len));
                    client.write(buf.toString('utf8', 0, len));
                    flag = 1;
                } else {
                    eventLogger.log("info" ,"failed to send length to server.");
                }
            } catch (err) {
                eventLogger.log("info" ,"cannot send Message Header.");
            }
        } else {
            eventLogger.log("info" ,"trying to send event to null device");
        }
    };

    eventEmitter.on('sendEvent', sendEventToServer);

    //Method to send event to server
    this.sendEvent = function(event) {
        eventLogger.log("info" ,"########### inside connection send event ######");
        event.prototype.setEventIndex(eventCount);
        var jsonData = event.prototype.toJson(event);
        if (isSocketWritable) {
            if (isKeyEvent(event)) {
                //ECMLOG("Keyevent sent to server: %s", QC_STR(eventJSON));
            }
            sendEventToServer(jsonData);
            eventCount++;
	}
    };
}

module.exports = XREConnection;
