/***************************************
 *Name: xreapplication.js
 *Date 9-jun-2015
 ***************************************/

var XREApplicationCanvas = require('../receiver/xreapplicationcanvas.js');
var XREConnection = require("./xreconnection.js");
var XRECommand = require("./xrecommand.js");
var Constants = require("../utils/config.js");
var CONSTANTS = require('../events/xreprotocol.js');
var EventEmitter = require('events').EventEmitter;
var RESULT = require('../utils/enum').RESULT;
var XRERectangleResource = require('../receiver/xrerectangleresource.js');
var XREImageResource = require('../receiver/xreimageresource.js');
var ApplicationResource = require('../receiver/xreapplicationresource.js');
var XRECommandSequence = require('./xrecommandsequence.js');
var serviceProxy = require('../receiver/xreServiceProxy.js').serviceProxy;
var EventDispatcher = require('../receiver/eventDispatcher.js');
var OnConnectEvent = require('../events/onconnectevent.js');
var URL = require("url");
var opaqueevent = require('../events/resourceEvent.js').COpaqueEvent;
var XRElogger = require('../utils/xrelogger.js');
var XREAlphaAnimation = require('../receiver/xreAnimation.js').XREAlphaAnimation;
var XREAbsoluteTranslationAnimation = require('../receiver/xreAnimation.js').XREAbsoluteTranslationAnimation;
var XREDimensionsAnimation = require('../receiver/xreAnimation.js').XREDimensionsAnimation;
var XREColorAnimation = require('../receiver/xreAnimation.js').XREColorAnimation;
var XREAbsoluteScaleAnimation = require('../receiver/xreAnimation.js').XREAbsoluteScaleAnimation;
var XRETransformAnimation = require('../receiver/xreAnimation.js').XRETransformAnimation;
var XRETextResource = require("../receiver/xretextresource.js").TextResource;
var XREFontResource = require("../receiver/xrefontresource.js");
var XREObject = require("../receiver/xreObject.js");
var onReadLocalObject = require('../events/resourceEvent.js').COnReadLocalObject;
var onTimerStopEvent = require('../events/resourceEvent.js').COnTimerStopEvent;
var fs = require('fs');
var XREViewActivateEvent = require('../events/resourceEvent.js').COnAppActivate;
var XREViewDeactivateEvent = require('../events/resourceEvent.js').COnAppDeactivate;
var XREHTTPResponseEvent = require('../events/resourceEvent.js').COnHTTPResponseEvent;
var XREGenericEvent = require('../events/resourceEvent.js').CGenericEvent;
var XRENineSliceImageResource = require('../receiver/nineSliceImageResource.js');
var XREHTMLText = require('../receiver/htmlTextResource.js');
var StylesheetResource = require('../receiver/stylesheetResource.js');
var HTTPNetworkAccessManager = require('../receiver/httpnetworkaccessmanager.js');
var XRETextInput = require('../receiver/textInputResource.js').TextInputResource;
var XREKeyMap = require("../receiver/keyMap.js");

var fs = require('fs');
/**
 *Class: XREApplication
 */
function XREApplication(scene, sessionGUID, res) {
    //TODO move to Settings
    var args = JSON.parse(fs.readFileSync('receiverConfig.json', 'utf8'));
    var _this = this;
    this.Aname; //TODO for testing
    var appCanvas = undefined;
    var activeView = undefined;
    var containingView = undefined;
    //Event emitter for emit signals
    var eventEmitter = new EventEmitter();
    var connection = undefined;

    var authToken = undefined;
    var sessionAuthToken = "";
    var deviceId = "";
    var authDeviceId = null;
    var partnerId = null;

    var customProperties;
    var heartbeatsEnabled = false;

    var launchParams;
    //TODO move to Settings
    var screenSize = {
        x: args.width,
        y: args.height
    };

    // Default Logger to set default logs.
    var defaultLogger = XRElogger.getLogger("default");
    var appURL = undefined;

    var authURL = undefined;
    var authBasePath = undefined;
    //Buffer to store high priority services
    var highPriorityServices = [];
    //command queue
    var jsonCommands = [];
    var writeObjectQueue = [];
    var readObjectQueue = [];

    //command arrays
    var resources = {};
    var resource = res;
    var serviceProxies = {};
    var commandSequences = {};
    //flag to determine que is under processing
    var isProcessing = false;
    var isWritting = false;
    var isReading = false;
    //last command processed
    var lastCommandIndex = 0;
    this.eventDispatcher = new EventDispatcher();
    //TODO For testing
    var ONCONNECT = Constants.SHELL;


    var keyMap;
    var receiverConfig = JSON.parse(fs.readFileSync('receiverConfig.json', 'utf8'));
    var objectFilePath;
    if (receiverConfig.objectFilePath) {
        objectFilePath = args.objectFilePath;
    } else {
        objectFilePath = Constants.OBJECT_FILE_PATH;
    }

    XREApplication.oldActiveApp = undefined;

    this.getRootXREApplication = function (){
        return XREApplication.getRootApp();
    };

    //method data when connection is establised
    var onConnected = function() {
        var appName = URL.parse(appURL).pathname.split('/').pop();
        _this.Aname = appName;
        var token;
        if (!authToken && !XREApplication.isWhiteboxAuthenticationEnabled()) {
            token = appURL.queryItemValue("authenticationToken ");
        } else {
            token = authToken;
        }
        //TODO move to Settings
        var pixelViewSize = {
            width: args.width,
            height: args.height
        };
        //TODO move to Settings
        var stretchedViewSize = {
            width: args.width,
            height: args.height
        };
        var videoPreferences;
        var forceSource;
        var customProperties;
        var heartbeatsEnabled = false;
        var reconnect = false;
        var reconnectReason = 6;

        var connectEvent = new OnConnectEvent(appName,
            sessionGUID,
            appURL,
            token,
            sessionAuthToken,
            deviceId,
            authDeviceId,
            partnerId,
            screenSize,
            pixelViewSize,
            stretchedViewSize,
            launchParams,
            videoPreferences,
            forceSource,
            customProperties,
            lastCommandIndex,
            heartbeatsEnabled,
            reconnect,
            reconnectReason);

        connection.getEmitter().emit('sendEvent', connectEvent.toJson(connectEvent));
        

    };

    //call back from connection on connect lost
    var onConnectionLost = function() {
        //TODO
        defaultLogger.log("debug", "connection Lost");
    };

    //call back on connection error
    var onConnectionError = function() {
        //TODO
        defaultLogger.log("debug", "connection Error");
    };

    //method add signals for connection
    var allocateConnection = function() {
        eventEmitter.on('connected', onConnected);
        eventEmitter.on('SendEvent', connection.sendEvent);
        eventEmitter.on('networkReply', replyFinished);

        //TODO add all signals
    };
    //Method to check command is high priority or not
    var isHighPriorityServices = function(targetId) {
        if (highPriorityServices.indexOf(targetId) > -1) {
            return true;
        } else {
            return false;
        }
    };

    var getObject = function(id) {
        clog("Inside getObject " + id);
        var target_object;
        if (id != Constants.XRE_ID_NULL) {
            if (id == Constants.XRE_ID_ROOT_APPLICATION) {
                target_object = _this;
            } else if (resources[id]) {
                target_object = resources[id];
            } else if (appCanvas.getView(id)) {
                target_object = appCanvas.getView(id);
            } else if (commandSequences[id]) {
                target_object = commandSequences[id];
            } else if (serviceProxies[id]) {
                target_object = serviceProxies[id];
            }
        }
        return target_object;
    };

    this.generateEvent = function(param) {
        //TODO
        //emit Event( COpaqueEvent(params), this );
        var event = new opaqueevent(param);
        this.emitSendEvent(event);
        var result = RESULT.XRE_S_OK;
        return result;
    };

    //method to generate event between apps
    var generateAppEvent = function(params) {
        var eventParams = params[0];
        var destinationGUID = params[1];
        if (destinationGUID) {
            var result = XREApplication.apps[destinationGUID].generateEvent(eventParams);
            return result;
        }

        return Constants.XRE_E_INVALID_ID;
    };

    //Method toread object file
    var readObject = function(app, callback) {
        var object = JSON.parse('{}');
        if (!app && app === "") {
            app = URL.parse(appURL).pathname.split('/').pop();
        }
        var objectFile = objectFilePath + "/" + app + '.object';
        fs.readFile(objectFile, "utf-8", function(err, data) {
            if (!err) {
                try {
                    var buffer = new Buffer(data);
                    object = JSON.parse(buffer.toString());
                } catch (e) {
                    defaultLogger("error", "Unable to open file " + objectFile);
                }
            }
            callback(object);

        });

    };

    //method to write object to file
    var writeObject = function(paramList, callback) {
        var app = URL.parse(appURL).pathname.split('/').pop();
        var params;
        if (paramList[0]) {
            params = paramList[0];
        }
        if (paramList[1]) {
            app = paramList[1];
        }

        readObject(app, function(localObject) {

            try {
                if (!fs.existsSync(objectFilePath)) {
                    fs.mkdirSync(objectFilePath);
                }
            } catch (e) {
                clog("Unable to create folder " + objectFilePath);
                isWritting = false;
                triggerWriteObject();
                return;
            }
            var fd = fs.open(objectFilePath + "/" + app + '.object', 'w', function(err, fd) {
                if (!err) {
                    for (var key in params) {
                        localObject[key] = params[key];
                    }
                    fs.write(fd, JSON.stringify(localObject), function(err, written, buffer) {
                        if (err) {
                            defaultLogger.log("error", "Object write failed");
                        }
                        fs.close(fd, function(err) {
                            if (err) {
                                defaultLogger.log("error", "Error on closing file");

                            }
                            isWritting = false;
                            triggerWriteObject();
                        }); //End file close
                    }); //End of write
                } else {
                    defaultLogger.log("error", "Unable to open/create file");
                    isWritting = false;
                    triggerWriteObject();
                }
            }); //End of file open
        }); // end of read object
    };

    //trigger file writting
    var triggerWriteObject = function() {
        defaultLogger.log("debug", "triggerWriteObject");
        if (!isWritting && writeObjectQueue.length > 0) {
            isWritting = true;
            writeObject(writeObjectQueue.shift());
        }
    };

    //trigger file reading
    var triggerReadObject = function() {
        var app = "";
        if (!isReading && readObjectQueue.length > 0) {
            isReading = true;
            paramList = readObjectQueue.shift();
            if (paramList[0]) {
                app = paramList[0];
            }

            readObject(app, function(localObject) {
                var readLocalObjectEvent = new onReadLocalObject(localObject);
                _this.getEmitter().emit('Event', readLocalObjectEvent, _this);
                isReading = false;
            });
        }
    };

    var writeLocalObject = function(paramList) {
        writeObjectQueue.push(paramList);
        eventEmitter.emit('writeObject');
    };

    var readLocalObject = function(paramList) {
        readObjectQueue.push(paramList);
        eventEmitter.emit('readObject');
    };

    var httpRequest = function(paramList) {
        //paramList = params.params;
        var url = paramList[3];
        var verb = paramList[1];
        // Update requests to whitebox

        if (true /*XRE::Settings::authEnabled()*/ &&
            (URL.parse(url).hostname === "localhost" || URL.parse(url).hostname === "127.0.0.1") &&
            URL.parse(url).port === 50050 && URL.parse(url).pathname.indexOf("/device") === 0) {
            //TODO
            // QUrl whitboxURL(m_authURL);
            //whitboxURL.setPath(authBasePath + qurl.path().mid(qstrlen("/device")));
            //XRELOG_TRACE("http request to whitebox, url updated: '%s' => '%s'", QC_STR(url), QC_STR(whitboxURL.toString()));
            //url = whitboxURL.toString();
        }

        if (verb == "GET") {
            
            defaultLogger.log("info", " Entered inside the get method of httpRequest");
            HTTPNetworkAccessManager.getWithTimeout(_this, url, verb);
        } else if (verb == "POST") {
            defaultLogger.log("info", "paramlist[4]--------------->>", paramList[4]);
           
            defaultLogger.log("info", " Entered inside the post method");

            HTTPNetworkAccessManager.postWithTimeout();
        }
        return RESULT.XRE_S_OK;
    };

    //method to call application methods
    var callAppMethod = function(method, params) {
        if (method === "generateAppEvent") {
            return generateAppEvent(params);
        } else if (method === "generateEvent") {
            return _this.generateEvent(params[0]);
        } else if (method == "generateMouseWheelEvent") {
            //TODO    
            return Constants.XRE_S_OK;

        } else if (method == "generateMouseEvent") {

            //TODO
            return RESULT.XRE_S_OK;

        } else if (method == "generateKeyEvent") {
            //TODO
            return RESULT.XRE_S_OK;
        } else if (method == "execExternal") {
            return; //execExternal(params);
        } else if (method == "httpRequest") {
            defaultLogger.log("info", "Printing httpRequest Params....>>");
            defaultLogger.log("info", params);
            return httpRequest(params);

        } else if (method == "readLocalObject") {
            readLocalObject(params);

            return RESULT.XRE_S_OK;
        } else if (method == "writeLocalObject") {

            return writeLocalObject(params);
        } else if (method == "log") {
            return; //writeLog(params);
        } else if (method == "ping") {
            return; //ping(params);
        } else {
            //TODO
        }

        return Constants.XRE_E_OBJECT_DOES_NOT_IMPLEMENT_METHOD;
    };


    //process each command
    this.processCommand = function(command) {
        if (command) {
            lastCommandIndex = command.getCommandIndex();
            command.execute(_this);
        }
    };

    //Method get commands from queue and process it
    var processCommands = function() {
        isProcessing = true;
        while (jsonCommands.length > 0) {
            var command = jsonCommands.shift();
            _this.processCommand(command);
        }
        isProcessing = false;
    };

    //method to trigger cammand processing
    var triggerCommandProcessing = function() {
        if (!isProcessing) {
            eventEmitter.emit('commands');
        }
    };

    //methode to initialize application
    var allocateMembers = function() {
        //TODO 
        appCanvas = new XREApplicationCanvas(scene, _this);
        activeView = appCanvas.getView(Constants.XRE_ID_ROOT_VIEW);
        keyMap = new XREKeyMap();
    };

    //create view
    var createView = function(id, params) {
        var result = appCanvas.createView(id, params);
        return result;
    };

    //Set properties for root app
    var setProperties = function(params) {
        _this.eventHandlers.install(params);
        //TODO need to check EAS
        /*
    eventHandlers->Install( params );
    if( params.contains("easEnabled") )
    {
        var newEasEnabled = params["easEnabled"];
        if(newEasEnabled!=bEasEnabled)      {
        if(newEasEnabled) {
                EnterEasMode();
            } else {
                ExitEasMode(); 
            }
        }
    */
        return RESULT.XRE_S_OK;
    };

    //method to create resource object and add it to array 
    var createResource = function(id, klass, params) {

        if (klass == Constants.ks_XRERectangle) {
            resources[id] = new XRERectangleResource(id, _this, params, scene);
        } else if (klass == Constants.ks_XREImage) {
            resources[id] = new XREImageResource(id, _this, params, scene);
        } else if (klass == Constants.ks_XRENineSliceImage) {
            resources[id] = new XRENineSliceImageResource(id, _this, params, scene);
            
        } else if (klass == Constants.ks_XREText) {
            resources[id] = new XRETextResource(id, _this, params, scene);
        } else if (klass == Constants.ks_XREFont) {
            resources[id] = new XREFontResource(id, _this, params);
        } else if (klass == Constants.ks_XRETransformAnimation) {
            resources[id] = new XRETransformAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREAlphaAnimation) {
           resources[id] = new XREAlphaAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREDimensionsAnimation) {
            resources[id] = new XREDimensionsAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREAbsoluteTranslationAnimation) {
            resources[id] = new XREAbsoluteTranslationAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREAbsoluteScaleAnimation) {
            resources[id] = new XREAbsoluteScaleAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREColorAnimation) {
            resources[id] = new XREColorAnimation(id, _this, params);
        } else if (klass == Constants.ks_XREApplication) {

            resources[id] = new ApplicationResource(id, _this, params, scene, authToken);
            if (resources[id]) {
                resources[id].setProperties(params);
            }

        } else if (klass == Constants.ks_XREHTMLText) {
            resources[id] = new XREHTMLText(id, _this, params, scene);
            clog("=========resource[id]123");
            clog(resources[id]);
        } else if (klass == Constants.ks_XREStylesheet) {
            clog("Inside StylesheetResource======================");
            resources[id] = new StylesheetResource(id, _this, params);
        } else if (klass == Constants.ks_XRETextInput) {
            clog("Inside XRE TextInput======================");
            resources[id] = new XRETextInput(id, _this, params, scene);
        }
        /*else if(klass == Constants.ks_XRETextInput)
        {
            resources[id] = new CTextInputResource(id,this,params);
        }
        else
        {
            return RESULT.XRE_E_UNSUPPORTED_OBJECT_TYPE;
        }
    */
        return RESULT.XRE_S_OK;

    };

    var replyFinished = function(params, method) {
        authToken = "AHuptmh1TGoykHen"; //data["authToken"].toString();
        deviceId = "P0102803520"; //data["deviceId"].toString();
        if (method == 'GET') {
            defaultLogger.log("info", "Entered inside the function reply finished");
            if (params == null) {
                var params = {};
                defaultLogger.log("info", "Entered inside HTTPResponseEvent ");
                _this.eventHandlers.install(params);
                var GenericEvent = new XREGenericEvent(CONSTANTS.EVT_ON_HTTP_ERROR, params);
                _this.getEmitter().emit('Event', GenericEvent, _this);
            } else {
                defaultLogger.log("info", "Entered inside HTTPResponseEvent ");
                _this.eventHandlers.install(params);
                defaultLogger.log("info", params.statusCode);
                defaultLogger.log("info", params.contentType);
                var HTTPResponseEvent = new XREHTTPResponseEvent(params.statusCode, params.contentType, params.body);
                _this.getEmitter().emit('Event', HTTPResponseEvent, _this);
            }
        }
    };

    this.getKeyMap = function() {
        return keyMap;
    };
    this.getName = function() {
        return appURL.substring(2);
    };
    //Event emitter object
    this.getAppEmitter = function() {
        return eventEmitter;
    };

    this.getContainingView = function() {
        return containingView;
    };

    this.setContainingView = function(view) {
        containingView = view;
    };

    this.setParentItem = function(parent) {
        if (appCanvas) {
            //appCanvas.setParentItem(parent);
        }
    };
    this.getCommandSequence = function(id) {
        if (id !== RESULT.XRE_ID_NULL || !commandSequences.id) {
            return commandSequences[id];
        }
        return;
    };

    //Method to return the connection
    this.getConnection = function() {
        return connection;
    };

    this.getActiveView = function() {
        return activeView;
    };
    //Method to return the resource
    this.getResourceContainer = function() {
        return resource;
    };
    //Method to used to create command object from json and added to queue

    this.pushJSON = function(data) {
        var targetId = XRECommand.getTargetId(data);
        var command = XRECommand.createCommand(data);
        if (command) {
            if (isHighPriorityServices()) {
                jsonCommands.unshift(command);
            } else {
                jsonCommands.push(command);
            }
            triggerCommandProcessing();
        }

    };


    //get for resorce      
    this.getResource = function(id) {
        return resources[id];
    };

    //Method to handle connect command
    this.connect = function(sesGUID,
        kMURL,
        ver,
        scopeLocalObject,
        hbTimeout,
        hbJitter,
        hbWarning,
        reconPolicy,
        reconTimeout,
        reconRetries,
        maxKeyRep,
        imageTypeSet) {

        //TODO  implement key map url
        sessionGUID = sesGUID;
        XREApplication.apps[sessionGUID] = undefined;

        //TODO reconnect

        if (sessionGUID) {
            XREApplication.apps[sessionGUID] = this;
        }

        this.getAppEmitter().emit('appConnected');


        //TODO if( !GetName().isEmpty() ) m_appsbyname[GetName()] = this;

        //TODO

    };

    //Method to handle new command
    this.new = function(id, klass, params) {
        var result = RESULT.XRE_S_OK;
        switch (klass) {
            case Constants.ks_XREView:
                result = createView(id, params);
                break;
            case Constants.ks_XRECommandSequence:
                defaultLogger.log("info", "command sequence");
                commandSequences[id] = new XRECommandSequence(id, this, params);
                break;
            case Constants.ks_XRECachedCommandSequence:
                defaultLogger.log("info", "CachedCommandSequence");
                //commandSequences[id] = new XRECachedCommandSequence(id, this, params);
                break;
            case Constants.ks_XREServiceProxy:
                defaultLogger.log("info", "ks_XREServiceProxy");
                defaultLogger.log("info", "Entered Inside the service Proxxxy......!!!");
                clog(params);
                serviceProxies[id] = new serviceProxy(id, this, params);

                break;
            default: //Resource
                result = createResource(id, klass, params);
                break;
        }
        return result;
    };

    //Method to handle set command
    this.set = function(id, params) {
        defaultLogger.log("info", "setting properties");
        var result = RESULT.RESXRE_S_OK;
        if (id == Constants.XRE_ID_ROOT_APPLICATION) {
            result = setProperties(params);
        } else if (resources[id]) {
            defaultLogger.log("info", "setting resources  ");
            var res = resources[id];
            res.installEventHandlers(params);
            result = res.setProperties(params);
        } else if (serviceProxies[id]) {

            serviceProxies[id].setProperties(params);
        } else {
            if (appCanvas) {
                appCanvas.setViewProperties(id, params);
            } else {
                result = RESULT.XRE_E_FAILED;
            }
        }
        return result;
    };

    //Method to handle call command
    this.call = function(id, method, params, callGUID) {
        var resulr = RESULT.XRE_S_OK;
        if (resources[id]) {
            var res = resources[id];
            result = res.callMethod(method, params);
        } else if (commandSequences[id]) {
            var cs = commandSequences[id];
            result = cs.callMethod(method, params);
        } else if (serviceProxies[id]) {
            //TODO need to implement

            result = serviceProxies[id].callMethod(method, params, callGUID);
            clog("Call Method Invoked..........!!!");
           /* if (method == "init" && isHighPriorityService(params))
            {
            highPriorityServices.insert(id);
            }*/

        } else if (method == "activate") {
            if (!appCanvas) {
                return;
            }
            var prevActiveView = activeView;
            activeView = appCanvas.getView(id);

            //TODO need to check
            /*
               if (m_activeViewDeleting)
               {
               prevActiveView = NULL;
               m_activeViewDeleting = false;
               }
             */

            if (prevActiveView !== activeView) {
                if (prevActiveView) {
                    //TODO
                    //emit Event(CXREViewDeactivateEvent(), prevActiveView);
                    var viewDeactivateEvent = new XREViewDeactivateEvent();
                    this.getEmitter().emit('Event', prevActiveView);

                }
                if (activeView) {
                    activeView.activate();
                    //TODO
                    //emit Event(CXREViewActivateEvent(), activeView);
                    var viewActivateEvent = new XREViewActivateEvent();
                    this.getEmitter().emit('Event', activeView);

                    //emit Event(COnAppActivate(), this);
                }
            }

            var activeApp = XREApplication.getRootApp();
            while (activeApp) {
                var view = activeApp.getActiveView();
                if (view) {
                    var app = undefined; //TODO view.getApplication();
                    if (app) {
                        activeApp = app;
                    } else break;
                } else break;
            }
            if (XREApplication.oldActiveApp && XREApplication.oldActiveApp !== activeApp) {
                //TODO
                var deactivateEvent = new XREViewDeactivateEvent();
                this.getEmitter().emit('Event', deactivateEvent, this);
                //emit Event(COnAppDeactivate(), oldActiveApp);
            }

            XREApplication.oldActiveApp = activeApp;
            if (activeApp) {
                //TODO
                var activateEvent = new XREViewActivateEvent();
                this.getEmitter().emit('Event', activateEvent, this);
                //emit Event(COnAppActivate(), activeApp);
            }
        } else if (id == Constants.XRE_ID_ROOT_APPLICATION) {
            result = callAppMethod(method, params);

        } else {
            if (appCanvas) {
                result = appCanvas.callViewMethod(id, method, params);
            } else {
                result = RESULT.XRE_E_FAILED;
            }
        }
        //TODO
    };

    //app.get(targetId, targetPropNames, context);
    this.get = function(target, targetPropNames, context) {
        var targetObject;
        if (typeof target === 'string' || target instanceof String) {
            //TODO need to implement
        } else {
            targetObject = getObject(target);
        }

        if (!targetObject) {
            var props = {
                "contentType": null,
                "blocked": false,
                "closedCaptioningEnabled": false,
                "volume": 0,
                "speed": 1,
                "closedCaptionOptions": {},
                "preferredZoomSetting": 0,
                "preferredAudioLanguage": null,
                "position": 0,
                "autoPlay": false,
                "persistent": true,
                "isPersisted": false,
                "contentOptions": [],
                "networkBuffer": 0,
                "url": null,
                "availableClosedCaptionTracks": [],
                "closedCaptionsTrack": null,
                "playSpeed": 1
            };

            var onGetComplete = new XREgetCompletedEvent(props,context);
            this.getEmitter().emit('Event', onGetComplete, this);

            /*XREResult xr = targetObject - > GetProperties(prop_names, props);
            if (!XRE_FAILED(xr)) {
                emit Event(COnGetComplete(props, context), this);
            } else {
                XRELOG_DEBUG("%s", QC_STR(XREGetErrorString(xr)));
            }
            if (debug) {
                XRELOG_DEBUG("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
                XRELOG_DEBUG(" ");
            }*/
        }


    };

    //Restart the application.
    this.restart = function(reason, statusCode) {
        defaultLogger.log("info", "Entered inside the restart function");
        url = appURL;
        this.reconnect(url);
    };

    //Redirect  the application.
    this.redirect = function(url, preserveSession, reason, statusCode) {
        defaultLogger.log("info", "Entered inside the redirect function");
        defaultLogger.log("info", "Url is------------------>");
        defaultLogger.log("info", url );

        this.reconnect(url,preserveSession);
    };

    //Shutdown  the application.
    this.shutdown = function(url, preserveSession, reason, statusCode) {
        defaultLogger.log("info", "Going to shutdown........!!!!");
        process.exit();
    };

    this.connectRejected = function(statusCode, redirectURL, reason) {
        defaultLogger.log("info", "connectRejected........!!!!");
        url = appURL;
        switch (statusCode) {
            case CONSTANTS.STATUS_CODE_REDIRECT:
                url = redirectURL;
                defaultLogger.log("info", "STATUS_CODE_REDIRECT........!!!!");
            case CONSTANTS.STATUS_CODE_COMMAND_INDEX_MISMATCH:
            case CONSTANTS.STATUS_CODE_UNKNOWN_SESSION:
                this.reconnect(url); //preserveSession=false
                break;
            case CONSTANTS.STATUS_CODE_OVERLOADED:               
                this.reconnect(url);
                break;
            default: 
                this.reconnect(url);
        }
    };


    this.reconnect = function(url) {
        var view = appCanvas.getView(Constants.XRE_ID_ROOT_VIEW);
        if (view.getViewObj().parent == scene.root) {
            defaultLogger.log("info","reconnect root app");
            XREApplication.rootApp = null;
        }
        appCanvas.deleteView(Constants.XRE_ID_ROOT_VIEW);

        resources = {};
        allocateMembers();
        defaultLogger.log("info", "Entered inside the reconnect function");
        this.start(url, authToken, launchParams, lastCommandIndex, heartbeatsEnabled);

    };

    //Method used start an xre application
    this.start = function(url, token, params, commandIndex, heartbeat) {


        appURL = url;
        var host = URL.parse(appURL).hostname;
        var port = URL.parse(appURL).port;
        authToken = token;
        launchParams = params;
        lastCommandIndex = commandIndex;
        heartbeatsEnabled = heartbeat;

        replyFinished();
        connection = new XREConnection(this);
        allocateConnection();
        connection.connect(host, port);
    };

    this.getCanvas = function() {
        return appCanvas;
    };

    //Method to return event disptcher
    this.getEventDispatcher = function() {
        return this.eventDispatcher;
    };
    //Method to return active view
    this.getActiveView = function() {
        return activeView;
    };
    /** Process delete command **/
    this.delete = function(id) {
        if (resources[id]) {
            //TODO remove resource from Soundmixer instance
            if (resources[id].parentView) {
                resources[id].parentView.removeResource();
            }
            delete resources[id];
        } else if (commandSequences[id]) {
            delete commandSequences[id];
        } else
        if (serviceProxies[id]) {
            delete serviceProxies[id];
        } else {
            if (appCanvas) {
                appCanvas.deleteView(id);
            } else {
                defaultLogger.log("info", "DeleteView on empty canvas");
            }
        }
        if (highPriorityServices[id]) {
            delete highPriorityServices[id];
        }
        defaultLogger.log("info", "Deleted<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
    };
    //Method used to emit a signal to send an event to  server
    this.emitSendEvent = function(event) {
        eventEmitter.emit('SendEvent', event);


    };

    this.emitTimerStop = function(timerName, duration) {
        clog("====================" + timerName);
        var timerStopEvent = new onTimerStopEvent(timerName, duration);
        //_this.getEmitter().emit('Event', readLocalObjectEvent, _this);
        this.getEmitter().emit('Event', timerStopEvent, this);
    };

    //Calling parent constructor
    XREObject.call(this, Constants.XRE_ID_ROOT_APPLICATION, this);
    allocateMembers();
    //TODO connect signals
    eventEmitter.on('commands', processCommands);
    eventEmitter.on('writeObject', triggerWriteObject);
    eventEmitter.on('readObject', triggerReadObject);


};

XREApplication.apps = {};
XREApplication.rootApp = undefined;
XREApplication.oldActiveApp = undefined;

XREApplication.setRootApp = function(rootApp) {
    XREApplication.rootApp = rootApp;
};

XREApplication.getRootApp = function() {
    return XREApplication.rootApp;
};

XREApplication.getApps = function() {
    return XREApplication.apps;
};

XREApplication.enableWhiteboxAuthentication = function(url) {
    if (url) {
        authURL = url;
        var sl = URL.parse(url).pathname.split('/');
        sl.pop();
        authBasePath = sl.join("/");
    }

};

XREApplication.isWhiteboxAuthenticationEnabled = function() {
    if (authURL) {
        return true;
    }
    return false;
};
//Inheriting XREObject
XREApplication.prototype = Object.create(XREObject.prototype);
XREApplication.prototype.constructor = XREApplication;
module.exports = XREApplication;
