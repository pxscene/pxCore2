/************************************************************************
 * Name         : xreconsoleogger.js
 * Created      : 1/7/2015
 * Description  : Handles the Logs levels.
 *************************************************************************/


var fs = require('fs');
var json = require('../../receiverConfig.json');
var levels = ['all', 'debug', 'info', 'warn', 'error', 'off'];


/*
 * Class XRElogs
 */
var CXRElogs = function(categoryName) {

	/* Set the Default log level as info */
    this.logLevel = "info";
    switch (categoryName) {
    	case "command":
        	 this.logLevel = json.commandLogger.level;
        	 break;
        case "event":
            this.logLevel = json.eventLogger.level;
            break;
        case "default":
            this.logLevel = json.defaultLogger.level;
            break;
    }


    /*
     * Log Message is displayed with loglevel,date and time.
     */
    this.log = function(level, message) {

       
        if (levels.indexOf(level) >= levels.indexOf(this.logLevel)) {
            /*if (typeof message !== 'string') {
                message = JSON.stringify(message);
            }*/
            var currentdate = new Date();
            var datetime = currentdate.getDate() + "-" + (currentdate.getMonth() + 1) + "-" + currentdate.getFullYear() + " " + currentdate.getHours() + ":" + currentdate.getMinutes() + ":" + currentdate.getSeconds();
            var logMessage = '[' + datetime + ']' + '[' + level + ']' + message;
            clog(logMessage);
            writeLogsToFile(logMessage + '\n');
        }
    };

    /*
     * Logs are redirected to file.
     */
    var writeLogsToFile = function(logMessage) {

        fs.appendFile('logs/debug.log', logMessage, function(err, data) {
            if (err) {
                return clog(err);
            }
        });

    };

};

/*
 *  Class XRElogger
 */   
var XRElogger = function() {
    this.commandLogger = new CXRElogs("command");
    this.eventLogger = new CXRElogs("event");
    this.defaultLogger = new CXRElogs("default");

};

/*
 *  Current XRElogger is obtained
 */  
XRElogger.prototype.getLogger = function(categoryName) {
    switch (categoryName) {
        case "command":
            return this.commandLogger;

        case "event":
            return this.eventLogger;

        case "default":
            return this.defaultLogger;
    }

};

module.exports = new XRElogger();
