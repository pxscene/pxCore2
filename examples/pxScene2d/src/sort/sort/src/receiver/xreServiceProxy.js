/************************************************************************
 * Name         : xreServiceProxy.js
 * Created      : 9/7/2015
 * Description  :This file defines classes for service proxy
 *************************************************************************/
var Constants = require("../receiver/xresettings.js");
var XREObject = require("./xreObject.js");
var onCallResult = require('../events/onCallResult.js');
var XRElogger = require('../utils/xrelogger.js');
/*
 * Class of OnConnectEvent
 */

var defaultLogger = XRElogger.getLogger("default");

var serviceProxy = function(indexId, app, params) {
    //Need to Inherit the XRE object.
    XREObject.call(this, indexId, app);
    this.setProperties(params);

};
serviceProxy.prototype = Object.create(XREObject.prototype);
serviceProxy.prototype.constructor = serviceProxy;

/*
 * setProperties will set the properties for serviceproxy class
 */
serviceProxy.prototype.setProperties = function(params) {

    this.eventHandlers.install(params);

};

/*
 * callMethod will call the corresponding method
 */
serviceProxy.prototype.callMethod = function(method, params, callGUID) {
    if (method == "init") {

        defaultLogger.log("info", "Entered inside init init.......");
        this.init(params, callGUID);
    } else if (method == "getConnectedVideoDisplays") {
        var eventParams = {
            "connectedVideoDisplays": ["Component0", "HDMI0"],
            "success": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "getCurrentResolution") {
        var eventParams = {
            "resolution": "720p",
            "success": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "isUpnpEnabled") {
        var eventParams = {
            "powerState": null,
            "success": true,
            "enabled": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "getValues") {
        var eventParams = {
            "properties": [{
                "propertyName": "com.comcast.card.moto.entitlements",
                "value": 1
            }, {
                "propertyName": "com.comcast.card.moto.dac_init_timestamp",
                "value": ""
            }, {
                "propertyName": "com.comcast.card.cisco.status",
                "value": 0
            }, {
                "propertyName": "com.comcast.hdmi_edid_read",
                "value": 0
            }, {
                "propertyName": "com.comcast.ca_system",
                "value": 2
            }],
            "success": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "registerListeners") {
        defaultLogger.log("info", " params----->> " + params);
        if (params == "com.comcast.card.disconnected") {
            var eventParams = {
                "properties": [{
                    "propertyName": "com.comcast.card.disconnected",
                    "value": 0
                }],
                "success": true
            };
        } else {
            var eventParams = {
                "properties": [{
                    "propertyName": "com.comcast.tune_ready",
                    "value": 1
                }, {
                    "propertyName": "com.comcast.cmac",
                    "value": 2
                }, {
                    "propertyName": "com.comcast.time_source",
                    "value": 1
                }],
                "success": true
            };
        }
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "unregisterListeners") {
        var eventParams = {
            "success": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (method == "getDiscoveredDevices") {
        var eventParams = {
            "upnpResults": [{
                "recvDevType": "null",
                "baseTrmUrl": "ws://169.254.80.89:9988",
                "requiresTRM": "true",
                "gatewayip": "169.254.80.89",
                "timezone": "EST05",
                "systemids": "channelMapId:65535;controllerId:65535;plantId:0;vodServerId:17733",
                "playbackUrl": "http://169.254.80.89:8080/hnStreamStart?deviceId=P0102803520&DTCP1HOST=169.254.80.89&DTCP1PORT=5000",
                "isgateway": "yes",
                "buildVersion": "null",
                "dnsconfig": "nameserver 8.8.8.8;",
                "hostMacAddress": "14:d4:fe:a4:03:92",
                "baseStreamingUrl": "http://169.254.80.89:8080/videoStreamInit?recorderId=P0102803520",
                "hosts": "10.252.180.80 ccpapp-dt-v001-i.dt.ccp.cable.comcast.com;10.252.180.74 ccpapp-dt-v101-i.dt.ccp.cable.comcast.com;69.241.25.245 feynman.xcal.tv;10.252.180.213 ccplp2-dt-v001-q.dt.ccp.cable.comcast.com;10.252.180.38 proxy.dt.ccp.cable.comcast.com;162.150.27.161 next.xreguide.ccp.xcal.tv;162.150.27.161 xre.ccp.xcal.tv;162.150.27.161 current.recorder.ccp.xcal.tv;162.150.27.161 stg6.po.ccp.xcal.tv;162.150.27.161 horoscope.cvs6.ccp.xcal.tv;162.150.27.161 pandora.cvs6.ccp.xcal.tv;162.150.27.161 stocks.cvs5.ccp.xcal.tv;162.150.27.161 traffic.cvs5.ccp.xcal.tv;162.150.27.161 weather.cvs5.ccp.xcal.tv;162.150.27.161 current.xreweb.ccp.xcal.tv;69.252.117.134 edge.x1-app.xcr.comcast.net;162.150.27.161 stg8.po.ccp.xcal.tv;162.150.27.161 cvs6.po.ccp.xcal.tv;162.150.27.161 cvs6.br.ccp.xcal.tv;162.150.27.161 photos.cvs5.ccp.xcal.tv;162.150.27.161 rss.cvs6.ccp.xcal.tv;162.150.27.161 pandora.cvs5.ccp.xcal.tv;162.150.27.161 c7.dds2.xbo.ccp.xcal.tv;",
                "receiverid": "P0102803520",
                "sno": "PAPV00010132"
            }],
            "powerState": null,
            "success": true
        };
        statusCode = 0; //Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    }
    
};

serviceProxy.prototype.init = function(params, callGUID) {

    defaultLogger.log("trace", " Entered inside the init method ");
    var apiName = params;
    var statusCode;
    var eventParams = {};
    defaultLogger.log("debug", " Current Api is ...." + apiName);

    if ((apiName == "systemapi") ||
        (apiName == "System_1") ||
        (apiName == "System_2") ||

        (apiName == "System_3") ||

        (apiName == "System_4") ||

        (apiName == "System_5") ||
        (apiName == "System_6") ||

        (apiName == "System_7")) {
        // Registering is not required.
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        defaultLogger.log("info", " statusCode----->> " + statusCode);
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "DisplaySettings_1") ||
        (apiName == "DisplaySettings_3") ||
        (apiName == "DisplaySettings_4")) {

        statusCode = Constants.SERVICE_PROXY_INIT_SUCCESS;
        defaultLogger.log("info", " statusCode----->> " + statusCode);
        this.eventSend(statusCode, eventParams, callGUID);

    } else if (apiName == "StorageManager_1") {
        // Registering is not required.
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        defaultLogger.log("info", " statusCode----->> " + statusCode);
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "frontpanelapi") ||
        (apiName == "frontpanel_1") ||
        (apiName == "frontpanel_2") ||
        (apiName == "frontpanel_3") ||
        (apiName == "frontpanel_4")) {
        // Registering is not required.
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE; 
        defaultLogger.log("info", " statusCode----->> " + statusCode);
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "FrontPanelAPI") ||
        (apiName == "FrontPanel_1") ||
        (apiName == "FrontPanel_2") ||
        (apiName == "FrontPanel_3") ||
        (apiName == "FrontPanel_4")) {
        // Registering is not required.
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE; 
        defaultLogger.log("info", " statusCode----->> " + statusCode);
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "HomeNetworking_1") ||
        (apiName == "HomeNetworking_2") ||
        (apiName == "HomeNetworking_3") ||
        (apiName == "HomeNetworking_4")) {


        statusCode = Constants.SERVICE_PROXY_INIT_SUCCESS;
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "com.comcast.stateObserver_1") ||
        (apiName == "com.comcast.stateObserver_2")) {

        statusCode = Constants.SERVICE_PROXY_INIT_SUCCESS;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if ((apiName == "RFRemoteAPI") ||
        (apiName == "rfremote_1")) {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "msopairingapi") ||
        (apiName == "MsoPairing_2") ||
        (apiName == "MsoPairing_3") ||
        (apiName == "MsoPairing_4") ||
        (apiName == "MsoPairing_5")) {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE; // Registering is not required.
        this.eventSend(statusCode, eventParams, callGUID);
    } else if ((apiName == "HDCPProfile_1") ||
        (apiName == "HDCPProfile_2")) {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE; // Registering is not required.
        this.eventSend(statusCode, eventParams, callGUID);

    } else if ((apiName == "AVinputapi") ||
        (apiName == "AVinput_1")) {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (apiName == "org.openrdk.vrexmanagerservice") {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE;
        this.eventSend(statusCode, eventParams, callGUID);
    } else if (apiName == "screencapture_1") {
        statusCode =  Constants.SERVICE_PROXY_INIT_SUCCESS;
        this.eventSend(statusCode, eventParams, callGUID);
    } else {
        statusCode = Constants.SERVICE_PROXY_NON_ZERO_STATUS_CODE; // Registering is not required.
        this.eventSend(statusCode, eventParams, callGUID);
    }

};

serviceProxy.prototype.eventSend = function(statusCode, params, callGUID) {
    var param = params;
    var message = "";
    this.eventHandlers.install(params);
    onCallResultEvent = new onCallResult(statusCode, param, callGUID, message);
    this.getEmitter().emit('Event', onCallResultEvent, this);
    defaultLogger.log("info", " event send ...............>> ");

};

/**
 * Class onServiceProxyEvent
 **/
var onServiceProxyEvent = function(pEvent, paramsList) {
    var statusCode = 0;
    var event = pEvent;
    var params = paramsList;
    XREEvent.call(this, "onServiceProxyEvent");
    //TODO: onServiceProxyEvent constructor overloading
    this.AddEventParams = function() {
        var eventParams = {};
        eventParams["name"] = event;
        eventParams["params"] = params;
        return eventParams;
    };
};

module.exports = {
    serviceProxy: serviceProxy,
    onServiceProxyEvent: onServiceProxyEvent
};
