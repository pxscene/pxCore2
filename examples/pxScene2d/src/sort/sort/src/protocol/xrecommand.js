var CallCommand = require('../protocol/xrecallcommand.js');
var XRElogger = require('../utils/xrelogger.js');
var Constants = require("../utils/config.js");
var XRETimer = require("../utils/xretimer.js");
/**
 * Classname: XRECommand
 * Version information:
 * Date :
 * Author:
 */

var commandLogger = XRElogger.getLogger("command");
var XRECommand = function() {

};

XRECommand.XRE_TIMER_STOP = "STOP";
XRECommand.XRE_TIMER_START = "START";


//Method to get target id
XRECommand.getTargetId = function(json) {
    var id = json.targetId;
    if (!id) {
        id = json.id;
    }
    return id;
};

//Method to create command object
XRECommand.createCommand = function(data) {
    var cmdType = data.command;
    var cmd = undefined;
    switch (cmdType) {
        case "NEW":
            cmd = new NewCommand();
            break;
        case "SET":
            cmd = new SetCommand();
            break;
        case "CONNECT":
            cmd = new ConnectCommand();
            break;
        case "CALL":
            cmd = new CallCommand();
            break;
        case "GET":
            cmd = new GetCommand();
            break;
        case "DELETE":
            commandLogger.log("info","DELETE Command");
            cmd = new DeleteCommand();
            break;
        case "START_TIMER":
            cmd = new TimerCommand(XRECommand.XRE_TIMER_START);
            break;
        case "STOP_TIMER":
            cmd = new TimerCommand(XRECommand.XRE_TIMER_STOP);
            break;
        case "RESTART":
            cmd = new RestartCommand();
            break;
        case "REDIRECT":
            cmd = new RedirectCommand();
            break;
         case "SHUTDOWN":
            cmd = new ShutdownCommand();
            break;
         case "CONNECT_REJECTED":
            cmd = new ConnectRejectedCommand();
            break;   

        default:
            commandLogger.log("error","ERROR!-");
            return;
    }
    if (cmd) {
        cmd.setProperties(data);
    }
    return cmd;
};

var ConnectCommand = function() {
    commandLogger.log("info","Connect command");
    var sessionGUID = undefined;
    var keyMapURL = undefined;
    var version = undefined;
    var scopeLocalObject = undefined;
    var heartbeatTimeout = 0;
    var heartbeatJitter = 0;
    var heartbeatWarning = 0;
    var reconnectPolicy = undefined; //(RP_STICKY)
    var reconnectTimeout = 0;
    var reconnectRetries = 0;
    var reconnectPolicyDefined = false;
    var imageCacheSettings = undefined;
    var maxKeyRepeats = 0;
    var appProxyParams = undefined;
    var quirks = undefined; //(CXREApplication::QUIRK_MAX + 1) 
    var id = undefined;
    var params = undefined;
    var commandIdx = 0;

    this.execute = function(app) {
        commandLogger.log("info","connect this.execute");
        app.connect(sessionGUID, keyMapURL, version, scopeLocalObject, heartbeatTimeout, heartbeatJitter, heartbeatWarning, reconnectPolicy, reconnectTimeout, reconnectRetries, maxKeyRepeats, imageCacheSettings);
        commandLogger.log("info","connect this.execute end");

    };

    this.setProperties = function(cmdObj) {

        sessionGUID = cmdObj.sessionGUID;
        keyMapURL = cmdObj.keyMapURL;
        version = cmdObj.version;


        if (cmdObj.httpProxyPatterns) {
            //TODO check settings
            if (true) {
                 commandLogger.log("info","Not using server's httpProxyPatterns settings");
            } else {
                proxyParams = JSON.parse(cmdObj.httpProxyPatterns);
                appProxyParams = proxyParams;
            }

        }

        if (cmdObj.scopeLocalObject) {
            scopeLocalObject = cmdObj.scopeLocalObject;

            if (!scopeLocalObject)
                scopeLocalObject = "";
        }


        if (cmdObj.heartbeatTimeout) {
            heartbeatTimeout = cmdObj.heartbeatTimeout;
        }

        if (cmdObj.heartbeatJitter) {
            heartbeatJitter = cmdObj.heartbeatJitter;
        }

        if (cmdObj.heartbeatWarning) {
            heartbeatWarning = cmdObj.heartbeatWarning;
        }

        //TODO
        //reconnectPolicy = RP_STICKY;

        if (cmdObj.reconnectTimeout) {
            reconnectTimeout = cmdObj.reconnectTimeout;
        }

        if (cmdObj.reconnectRetries) {
            reconnectRetries = cmdObj.reconnectRetries;

            if (cmdObj.imageTypeSettings) {
                imageCacheSettings = cmdObj.imageTypeSettings;
            }

            if (cmdObj.usingTrueSD) {}

            if (cmdObj.maxKeyRepeats) {
                maxKeyRepeats = cmdObj.maxKeyRepeats;
            }
            //TODO need to check quirks
        }

    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

};

var SetCommand = function() {
    var id = undefined;
    var params = undefined;
    var commandIdx = 0;
    this.execute = function(app) {
         commandLogger.log("info","Set Execute");
        app.set(id, params);

    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        params = json.props;
        commandIdx = json.commandIndex;
    };
};

var RestartCommand = function() {
    commandLogger.log("info"," Entered inside the restart command");
    var id = undefined;
    var reason = undefined;
    var status = undefined;
    var statusCode = undefined;
    var commandIdx = 0;
    this.execute = function(app) {
        commandLogger.log("info","Set Execute");
        app.restart(reason,statusCode);

    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        commandIdx = json.commandIndex;
        status = json.status;
        statusCode = json.statusCode;
        reason = json.reason;
    };
};
var ShutdownCommand = function() {
    commandLogger.log("info"," Entered inside the ShutdownCommand");
    var id = undefined; 
    var commandIdx = 0;
    this.execute = function(app) {
        commandLogger.log("info","Set Execute");
        app.shutdown();
    };
    this.getCommandIndex = function() {
        return commandIdx;
    };
    this.setProperties = function(json) {
        id = json.targetId;
        commandIdx = json.commandIndex;      
    };
};
var RedirectCommand = function() {
    commandLogger.log("info"," Entered inside the RedirectCommand ");
    var id = undefined;
    var reason = undefined;
    var status = undefined;
    var statusCode = undefined;
    var commandIdx = 0;
    var preserveSession;
    var url= undefined;
    this.execute = function(app) {
        commandLogger.log("info","Set Execute");
        app.redirect(url,preserveSession,reason,statusCode);

    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        commandIdx = json.commandIndex;
        status = json.status;
        statusCode = json.statusCode;
        reason = json.reason;
        preserveSession = json.preserveSession;
        url = json.url;
    };
}; 
var ConnectRejectedCommand = function() {
    commandLogger.log("info"," Entered inside the ConnectRejected command");
    var id = undefined;
    var reason = undefined;
    var status = undefined;
    var statusCode = undefined;
    var commandIdx = 0;
    var redirectURL= undefined;
    this.execute = function(app) {
        commandLogger.log("info"," ConnectRejectedCommand ");
        app.connectRejected(statusCode,redirectURL,reason);

    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        commandIdx = json.commandIndex;
        status = json.status;
        statusCode = json.statusCode;
        reason = json.reason;
        url = json.redirectURL;
    };
}; 

var NewCommand = function() {
    var id = undefined;
    var params = undefined;
    var commandIdx = 0;
    var klass = undefined;
    this.execute = function(app) {
        app.new(id, klass, params);
    };

    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        if (!id) {
            id = json.id;
        }
        params = json.params;
        commandIdx = json.commandIndex;
        klass = json.klass;
    };
};
var DeleteCommand = function() {
    commandLogger.log("info","Delete command const");
    var id = undefined;
    var commandIdx = 0;
    this.execute = function(app) {
         commandLogger.log("info","Delete Execute===");
        app.delete(id);
    };
    this.getCommandIndex = function() {
        return commandIdx;
    };

    this.setProperties = function(json) {
        id = json.targetId;
        commandIdx = json.commandIndex;
    };
};

var TimerCommand = function(action) {
    var timerName;
    var commandIdx = 0;
    this.getCommandIndex = function() {
        return commandIdx;
    };
    this.setProperties = function(cmdObj) {
        clog("Setting.. Timer properties" + cmdObj.reason);
        timerName = cmdObj.reason;
        clog("added Timer properties ");
        commandIdx = cmdObj.commandIndex;
    };

    this.execute = function(app) {
        clog("timer execute " + action);
        switch (action) {
            case XRECommand.XRE_TIMER_START:
                clog("timer start" + timerName);
                var timer = new XRETimer(timerName);
                timer.startTimer(timerName);
                clog("Timer started");
                break;
            case XRECommand.XRE_TIMER_STOP:
                clog("timer stop"+ timerName);
                var timer = XRETimer.getTimer(timerName);
                if (timer) {
                    timer.stopTimer();
                    clog(timer.getDuration());
                    app.emitTimerStop(timerName, timer.getDuration());
                }
                //Performance::XRETimer::stopTimer( m_timerName );*/
                break;

        }
    };

};


var GetCommand = function() {
    var targetPropNames;
    var targetPath;
    var context;
    var targetId = 0;
    var commandIdx = 0;
    this.getCommandIndex = function() {
        return commandIdx;
    };
    this.setProperties = function(cmdObj) {
        targetId = cmdObj.targetId;
        targetPath = cmdObj.targetPath;
        targetPropNames = cmdObj.props;
        context = cmdObj.context;
        commandIdx = cmdObj.commandIndex;
    };

    this.execute = function(app) {
        if (targetPath) {
            app.get(targetPath, targetPropNames, context);

        } else {
            app.get(targetId, targetPropNames, context);
        }
    };

};
module.exports = XRECommand;
