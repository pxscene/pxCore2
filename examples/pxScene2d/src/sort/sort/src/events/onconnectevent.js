/************************************************************************
 * Name         : onconnectevent.js
 * Created      : 19/6/2015
 * Description  : Handles the setting and configurations required
 *************************************************************************/

var Constants = require("../receiver/xresettings.js");
var OS = require('os');
var XREEvent = require("../receiver/xreEvent.js");



/*
 * Class of OnConnectEvent
 */
var OnConnectEvent = function(appName,
    sessionGUID,
    connectURL,
    authToken,
    sessionAuthToken,
    deviceId,
    authDeviceId,
    partnerId,
    screenSize,
    pixelDimensions,
    stretchedDimensions,
    appParams,
    videoPreferences,
    forceSource,
    customProperties,
    lastCommandIndex,
    heartbeatsEnabled,
    bRecconnect,
    reconnectReason) {

    
    
    this.applicationName = appName;
    this.authenticationToken = authToken;
    this.sessionAuthToken = sessionAuthToken;
    this.deviceId = deviceId;
    this.authDeviceId = authDeviceId;
    this.partnerId = partnerId;
    this.screenSize = screenSize;
    this.pixelDimensions = pixelDimensions;
    this.stretchedDimensions = stretchedDimensions;
    this.appParams = appParams;
    this.videoPreferences = videoPreferences;
    this.forceSource = forceSource;
    this.sessionGUID = sessionGUID;
    this.currentCommandIndex = lastCommandIndex;
    this.reconnect = bRecconnect;
    this.reconnectReason = reconnectReason;
    this.connectURL = connectURL;
    this.heartbeatsEnabled = heartbeatsEnabled;

};
OnConnectEvent.prototype = new XREEvent(Constants.EVT_ON_CONNECT);

/*
 * Invoked from XREEvent. 
 */

OnConnectEvent.prototype.addEventParams = function(eventParams) {

    eventParams.applicationName = this.applicationName;
    eventParams.minimumVersion = Constants.GET_APP_MINIMUM_VERSION;
    eventParams.authenticationToken = this.authenticationToken;
    eventParams.sessionAuthToken = this.sessionAuthToken;
    //TODO --> Check whether sessionGUID is null or not
    eventParams.sessionGUID = this.sessionGUID;
    eventParams.currentCommandIndex = this.currentCommandIndex;
    eventParams.reconnect = this.reconnect;

    this.AddDeviceCaps(eventParams);
    this.AddAppParams(eventParams);

    //TODO deviceId and authToken should come from the localhost whitebox service
    //Need a check to implement that.

    eventParams.deviceId = this.deviceId;
    eventParams["auth.authService"] = 2;
    eventParams.authDeviceId = this.authDeviceId;
    eventParams.partnerId = this.partnerId;
    eventParams.connectURL = this.connectURL;
    eventParams.reconnectReason = this.reconnectReason;
    return eventParams;


};

/* AppParams added to the Params list */

OnConnectEvent.prototype.AddAppParams = function(eventParams) {

    var appParams = {};
    appParams.PHPSESSID = Constants.GET_PHP_SESSID;
    if (this.heartbeatsEnabled) {
        appParams.heartbeatRequest = 1;
    }
    if (this.videoPreferences > 0) {
        appParams.videoPreferences = this.videoPreferences;
    }
    if (this.forceSource > 0) {
        appParams.forceSource = this.forceSource;
    }
    if (this.customProperties > 0) {
        appParams.customProperties = this.customProperties;
    }

    var videoNetworkBufferHash = {};
    videoNetworkBufferHash.min = Constants.MIN_VIDEO_NETWORK_BUFFER_IN_MS;
    videoNetworkBufferHash.max = Constants.MAX_VIDEO_NETWORK_BUFFER_IN_MS;
    videoNetworkBufferHash.step = Constants.STEP_VIDEO_NETWORK_BUFFER_IN_MS;

    appParams.networkBuffer = videoNetworkBufferHash;
    appParams.mac = "14:D4:FE:A4:03:92"; //TODO
    appParams.authVersion = "12"; //TODO
    eventParams.appParams = this.appParams;

};

/* Device Capabilities added to the params list */

OnConnectEvent.prototype.AddDeviceCaps = function(eventParams) {

    var deviceCaps = {};
    deviceCaps.supportsTrueSD = this.supportsTrueSD();
    deviceCaps.platform = this.getPlatform();
    deviceCaps.platformOS = this.getPlatformOS();
    deviceCaps.protocolVersion = Constants.XRE_PROTOCOL_VERSION_SETTING;
    deviceCaps.receiverType = Constants.RECEIVER_TYPE;
    deviceCaps.platformVersion = '5.1.1'; //TODO
    deviceCaps.deviceType = 'ipstb'; //TODO --> Startup Argument
    deviceCaps.mimeTypes = this.getMimetypes();
    deviceCaps.quirks = this.getQuirks();
    deviceCaps.receiverPlatform = 'pace_XG1_2.0s39'; //TODO --> Startup Argument
    deviceCaps.fontFamilies = this.getFontFamilies();
    deviceCaps.authTypes = this.getAuthTypes();
    deviceCaps.ocap = "mediaserver";
    deviceCaps.dvrTypes = this.dvrType();
    deviceCaps.receiverVersion = "2.0s39";

    this.setDimensions(deviceCaps);


    var deviceCapsFeatures = {};
    deviceCapsFeatures["htmlview.headers"] = Constants.GET_HTML_VIEW_HEADERS;
    deviceCapsFeatures["htmlview.cookies"] = Constants.GET_HTML_VIEW_COOKIES;
    deviceCapsFeatures["htmlview.httpCookies"] = Constants.GET_HTML_VIEW_HTTP_COOKIES;
    deviceCapsFeatures["htmlview.evaluateJavaScript"] = Constants.GET_HTML_VIEW_EVALUATE_JAVASCRIPT;
    deviceCapsFeatures["htmlview.callJavaScriptWithResult"] = Constants.GET_HTML_VIEW_JAVASCRIPT_WITH_RESULT;
    deviceCapsFeatures["htmlview.urlpatterns"] = Constants.GET_HTML_VIEW_URL_PATTERNS;
    deviceCapsFeatures["htmlview.disableCSSAnimations"] = Constants.GET_HTML_VIEW_DISABLE_CSS_ANIMATION;
    deviceCapsFeatures["htmlview.postMessage"] = Constants.GET_HTML_VIEW_POST_MESSAGE;
    deviceCapsFeatures["htmlview.headers2"] = Constants.GET_HTML_VIEW_HEADERS_2;
    deviceCapsFeatures["allowSelfSignedWithIPAddress"] = Constants.GET_ALLOW_SELF_SIGNED_WITH_IPADDRESS;

    deviceCapsFeatures["video.persistent"] = Constants.VIDEO_PERSISTENT; //TODO A CHECK
    deviceCapsFeatures["connection.supportsSecure"] = Constants.CONNECTION_SUPPORTS_SECURE;
    deviceCapsFeatures["video.aveLiveFetchHoldTime"] = Constants.AVE_LIVE_FETCH_HOLD_TIME;

    deviceCaps.features = deviceCapsFeatures;
    eventParams.deviceCaps = deviceCaps;

};

/* To get the type of dvr */

OnConnectEvent.prototype.dvrType = function() {
    var dvrType = [];
    var dvr = "mdvr";
    dvrType.push(dvr);
    return dvrType;
};

/* To get the different fontfamilies */

OnConnectEvent.prototype.getFontFamilies = function() {

    var fontList = [Constants.FAMILY_TAG_XFINITY_SANS_LGT,
        Constants.FAMILY_TAG_XFINITY_SANS_MED_COND,
        Constants.FAMILY_TAG_XFINITY_SANS_MED,
        Constants.FAMILY_TAG_XFINITY_SANS_COND,
        Constants.FAMILY_TAG_XFINITY_SANS_REG,
        Constants.FAMILY_TAG_XFINITY_SANS_EX_LGT,
        Constants.FAMILY_TAG_XFINITY_SANS_ICON
    ];

    return fontList;
};

/* Different quirks are added to the list */

OnConnectEvent.prototype.getQuirks = function() {

    var quirksList = ["XRE-4621",
        "XRE-4826",
        "XRE-5553",
        "XRE-5743",
        "XRE-6350",
        "XRE-6827",
    ];
    return quirksList;
};

/* Different MimeTypes required are added to the list */

OnConnectEvent.prototype.getMimetypes = function() {

    var mimeTypes = ["image/jpeg",
        "image/jpg",
        "image/png",
        "image/gif",
        "video/mp4",
        "video/mov",
        "video/mp4v",
        "video/vnd.dlna",
        "audio/vnd.dlna",
        "audio/eac3",
        "audio/mpeg",
        "audio/mp3",
        "video/mp2ts",
        "video/x-motion-jpeg",
        "video/rtsp+h264",
        "video/MP2T",
        "application/x-mpegURL",
        "application/x-netzyn"
    ];

    return mimeTypes;
};

/* Check and returns the value of supportsTrueSD */

OnConnectEvent.prototype.supportsTrueSD = function() {

    var supportsTrueSD = Constants.TRUE_SD ? "true" : "false";
    return supportsTrueSD;

};

/* Get the platfrom */

OnConnectEvent.prototype.getPlatform = function() {

    var platform = OS.type();

    if (platform == "WIN32") {
        return Constants.PLATFORM_NAME_WINDOWS;
    } else if (platform == "MAC") {
        return Constants.PLATFORM_NAME_MAC;
    } else {
        return Constants.PLATFORM_NAME_LINUX;
    }
};

/* Get the current Platform OS */
OnConnectEvent.prototype.getPlatformOS = function() {


    var platform = OS.platform();

    if (platform == "WIN32") {
        return Constants.PLATFORM_OS_NAME_WINDOWS;
    } else if (platform == "MAC") {
        return Constants.PLATFORM_OS_NAME_MAC;
    } else {
        return Constants.PLATFORM_OS_NAME_LINUX;
    }
};

/* Function Invoke d to set the dimensions */

OnConnectEvent.prototype.setDimensions = function(deviceCaps) {
    var dims = [];
    var x = this.screenSize.x;
    var y = this.screenSize.y;
    dims.push(x);
    dims.push(y);
    deviceCaps.nativeDimensions = dims;

    var pixDims = [];
    pixDims.push((this.pixelDimensions).width);
    pixDims.push((this.pixelDimensions).height);
    deviceCaps.pixelDimensions = pixDims;


    var strDims = [];
    strDims.push((this.stretchedDimensions).width);
    strDims.push((this.stretchedDimensions).height);
    deviceCaps.stretchedDimensions = strDims;

};

/* Returns the value of authtypes */

OnConnectEvent.prototype.getAuthTypes = function(deviceCaps) {
    var authType = [];
    authType.push("whitebox");
    //authType.push("secureAuth");
    return authType;
};

module.exports = OnConnectEvent;

/* End of File */
