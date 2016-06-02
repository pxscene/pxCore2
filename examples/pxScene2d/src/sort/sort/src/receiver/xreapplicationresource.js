var RESULT = require('../utils/enum').RESULT;
var XREApplication = require('../protocol/xreapplication.js');
var EventEmitter = require('events').EventEmitter;
var XREResource = require("./xreresource.js");
var OnReady = require('../events/resourceEvent.js').COnReadyEvent;

var ApplicationItem = function(view, app){
    var xreapp = app;
    //QObject::connect(view,SIGNAL(gotEvent(QEvent*)),SLOT(handleEvent(QEvent *)));


    //QObject::connect(app,SIGNAL(teardown()),SLOT(setAppParent()));
    


    this.cleanUp = function(){
        if (xreapp){
            xreapp.setParentItem();
            xreapp = undefined;
        }
    };

    this.setAppParent = function(){
        if(xreapp){
            xreapp.setParentItem(this);
            
        }
    };

    this.handleEvent = function(event){
        if(xreapp || !xreapp.getActiveView()){
            return;
        }
        xreapp.getActiveView().raiseEvent(event);
    };
    view.getEventEmitter().on("gotEvent", this.handleEvent);
};
var ApplicationResource = function(id, app, params, scene, authToken) {
    var _this = this;
    var appItem = undefined;
    var ChildAppConnected = false;
    var ChildAppFirstConnectAttempt = true;
    var childApp = undefined;
    var url = undefined;
    var minimumVersion = undefined;
    var sessionGUID = undefined;
    var launchParams = undefined;
    
    //Calling parent constructor
    XREResource.call(this, id, app);
    

    this.handleEvent = function(event, modifiers) {
        //clog("XREApplication resource handle event==========");
        if (childApp && childApp.getActiveView()) {
            //clog("XREApplication resource handle event========== ifff" + childApp.getActiveView().getId());
            childApp.getActiveView().raiseEvent(event, modifiers);
        }
        
    };
    this.getEmitter().on("gotEvent", this.handleEvent);

    this.onAppConnected = function() {
        clog("onAppConnected");
        var readyEvent = new OnReady();
        _this.getEmitter().emit('Event', readyEvent, _this, false);
        clog("onAppConnected=====");
    };

    this.setProperties = function(params) {
        if (childApp) {
            return RESULT.XRE_S_OK; //FIXME: Setting runtime app properties via the resource will not work after construction!
        }
        url = params.url;
        minimumVersion = params.minimumVersion;
        sessionGUID = params.sessionGUID;
        if (params.launchParams) {
            launchParams = params.launchParams;
        }
        var XREApplication = require('../protocol/xreapplication.js');
        childApp = new XREApplication(scene, sessionGUID, this);
        XREApplication.apps[sessionGUID] = childApp;
        clog(this.onAppConnected);
        childApp.getAppEmitter().on('appConnected', this.onAppConnected);
        //QObject::connect(m_childApp, SIGNAL(onDisconnectedSignal()),SLOT(onAppDisconnected()));

        childApp.start(url, authToken, launchParams, 0);
        return RESULT.XRE_S_OK;
    };
    /**
     *  Function to set application resource to the parent view
     **/
    this.createPXItem = function(view) {
        childApp.setContainingView(view);
        appItem =  new ApplicationItem(view, childApp);
        //view.resourceItem = childApp.;
        var resItem = childApp.getCanvas().getView(Constants.XRE_ID_ROOT_VIEW).getViewObj();
        resItem.parent = view.getViewObj();
        view.setResourceItem(resItem);
    };

    this.callMethod = function(method, params) {
        if (childApp) {
            childApp.call(XRE_ID_ROOT_APPLICATION, method, params, "");
        }
        return XRE_S_OK;
    };
    this.handlesViewEvents= function(){
        return true;
    }

}
ApplicationResource.prototype = Object.create(XREResource.prototype); 
ApplicationResource.prototype.constructor = ApplicationResource;
module.exports = ApplicationResource;

