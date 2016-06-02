/************************************************************************
 * Name         : httpnetworkaccessmanager.js
 * Created      : 19/6/2015
 * Description  : Handles the httprequest method
 *************************************************************************/
var XRElogger = require('../utils/xrelogger.js');
var defaultLogger = XRElogger.getLogger("default");
var http = require('http');

/*
 * Class of httpnetworkaccessmanager
 */
var HTTPNetworkAccessManager = function() {

};
HTTPNetworkAccessManager.prototype.getWithTimeout = function(appObj, url, verb) {
    var options = url;
    defaultLogger.log("info", "Printing the url" + url);
    var method = verb;
    defaultLogger.log("info", "Entered inside the getWithtimeout of HTTPNetworkAccessManager");
    var request = http.get(options, function(res) {
        defaultLogger.log("info", "Printing the params of HTTPGET Method");
        var params = {};
        var statusCode = res.statusCode;
        var contentType = res.headers['content-type'];
        params["statusCode"] = statusCode;
        params["contentType"] = contentType;
        var body;
        res.on("data", function(chunk) {
            body += chunk;
            params["body"] = body;
        });
        defaultLogger.log("info", "Printing the params of HTTPGET Method");
        defaultLogger.log("info", params);
        res.on('end', function(chunk) {
            clog("End of responses");
            defaultLogger.log("info", "Printing the params of HTTPGET Method");
            defaultLogger.log("info", "Printing the body..........!!!");
            clog(params[statusCode]);
            clog(params[contentType]);
            appObj.getAppEmitter().emit('networkReply', params, method);
        });
        res.on('error', function(e) {
            clog("Got error: " + e.message);
        });
    });
    process.on('uncaughtException', function(err) {
        clog("error============");
        params = null;
        appObj.getAppEmitter().emit('networkReply', params, method);
    });
    request.setTimeout(100000, function() {
        defaultLogger.log("info", "Entered inside setTimeout..........!!!");

    });
};

HTTPNetworkAccessManager.prototype.postWithTimeout = function() {
    defaultLogger.log("info", "Entered inside the postWithTimeout");
};

module.exports = new HTTPNetworkAccessManager();
