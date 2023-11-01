/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

'use strict';

//application manager variables
var applicationsArray = [];
var availableApplicationsArray = [];
var eventListenerHash = {};
var html5_suspend_whitelist = [];
var html5_suspend_delay_seconds = 5;

var scene;
var root;
var appManager = new Optimus();
module.exports = appManager;

var node_url = require('url');

var ApplicationType = Object.freeze({
  SPARK:"SPARK",
  SPARK_INSTANCE:"SPARK_INSTANCE",
  NATIVE:"NATIVE",
  WEB:"WEB",
  UNDEFINED:"UNDEFINED"
});

var ApplicationState = Object.freeze({
  RUNNING:"RUNNING",
  SUSPENDED:"SUSPENDED",
  DESTROYED:"DESTROYED"
});

/**
 * @param props
 * @example
 * id - (String) the id of the application
 * priority - (integer) value between 1 and 10 inclusive
 * x – (float) x-coordinate used as input into the object's transform function in pixel units
 * y – (float) y-coordinate used as input into the object's transform function in pixel units
 * w – (float) pixel unit width of the object
 * h – (float) pixel unit height of the object
 * cx - (float) x offset used as the center of rotation and scale
 * cy - (float) y offset used as the center of rotation and scale
 * sx – (float) scale factor in the x dimension
 * sy – (float) scale factor in the y dimension
 * r - (float) angle of rotation in degrees
 * a – (float) alpha or opacity of the object [0-1]
 * interactive - (boolean) determines whether the application is mouse interactive. defaults to true.
 * painting - (boolean) when set to false the application will be snapshotted. when set to true the application will immediately reflect any changes
 * clip - (boolean) determines whether the drawing done by the object and it's children will be clipped by the objects w and h properties. defaults to false.
 * mask - (boolean) determines whether this application will be used to define an alpha layer mask for the siblings of this application. defaults to false.
 * draw - (boolean) determines whether this object will be drawn. defaults to true.
 * launchParams  (Object) - a set of k/v pairs to be passed to the application
 * @constructor
 */
function Application(props) {

  // Public variables
  this.id = undefined;
  this.priority = 5;
  this.name = "";
  this.createTime = 0;
  this.uri = "";
  this.type = ApplicationType.UNDEFINED;
  this.expectedMemoryUsage = -1;
  this.actualMemoryUsage = -1;
  var _readyBaseResolve = function(){};
  var _readyBaseReject = function(){};
  var _readyResolve = function(){};
  var _readyReject = function(){};
  var _uiReadyResolve = function(){};
  var _uiReadyReject = function(){};
  var _urlChangeResolve = function(){};
  var _urlChangeReject = function(){};
  this._metaData = {}

  // Getters/setters
  var _externalAppPropsReadWrite = {
    x:"x",y:"y",w:"w",h:"h",cx:"cx",cy:"cy",sx:"sx",sy:"sy",r:"r",a:"a",
    interactive:"interactive",painting:"painting",clip:"clip",mask:"mask",draw:"draw",hasApi:"hasApi", url:"url", displayName:"displayName"
  };
  var _externalAppPropsReadonly = {
    pid:"clientPID" // integer process id associated with the application
  };
  var _this = this;
  Object.keys(_externalAppPropsReadWrite).forEach(function(key) {
    Object.defineProperty(_this, key, {
      get: function() { return _externalApp[_externalAppPropsReadWrite[key]]; },
      set: function(v) {
           if (key === "url") {
             if ((this.type === ApplicationType.SPARK_INSTANCE || this.type === ApplicationType.WEB) && typeof(this.api()) != "undefined") {
               
               this.api().url = v;
               
               this.urlChangeUiReady = new Promise(function (resolve, reject) 
               {
                 _urlChangeResolve = resolve;
                 _urlChangeReject = reject;
               });
               
               this.urlChangeUiReady.then( 
                 function() { _this.logTelemetry("urlChangeUiReady", true); },
                 function() { _this.logTelemetry("urlChangeUiReady", false); }
                 );
             }
             else if (this.type === ApplicationType.SPARK) {
               console.log("setting url on spark app is not permitted");
             }
             else {
               _externalApp[_externalAppPropsReadWrite[key]] = v;
             }
           }
           else {
             _externalApp[_externalAppPropsReadWrite[key]] = v;
           }
         }
    });
  });
  Object.keys(_externalAppPropsReadonly).forEach(function(key) {
    Object.defineProperty(_this, key, {
      get: function() { return _externalApp[_externalAppPropsReadonly[key]]; }
    });
  });

  Object.defineProperty(_this, "metaData", {
      get: function() { return _metaData; }
  });

  this.readyBase = new Promise(function (resolve, reject) {
    _readyBaseResolve = resolve;
    _readyBaseReject = reject;
  });
  this.ready = new Promise(function (resolve, reject) {
    _readyResolve = resolve;
    _readyReject = reject;
  });
  this.uiReady = new Promise(function (resolve, reject) 
  {
    _uiReadyResolve = resolve;
    _uiReadyReject = reject;
  });
  this.urlChangeUiReady = null;
  
  //only fire newReady and ready and uiReady have succeeded
  Promise.all([this.readyBase, this.uiReady]).then(function() 
  {
    _readyResolve();
  }, function() 
  {
    _readyReject();
  });
  
  this.logTelemetry = function(log_id, is_success)
  {
    var timestamp = "timestamp unknown";
      
    if(typeof(scene) != "undefined")
      timestamp = scene.clock().toFixed(2);
    
    if(is_success)
      console.log("OPTIMUS_uiReady:" + timestamp + "," + log_id + "," + _this.id + ",success");
    else
      console.log("OPTIMUS_uiReady:" + timestamp + "," + log_id + "," + _this.id + ",failure");
  }
  
  this.uiReady.then( 
    function() { _this.logTelemetry("uiReady", true); },
    function() { _this.logTelemetry("uiReady", false); }
    );
  
  this.readyBase.then( 
    function() { _this.logTelemetry("readyBase", true); },
    function() { _this.logTelemetry("readyBase", false); }
    );
  
  this.ready.then( 
    function() { _this.logTelemetry("ready", true); },
    function() { _this.logTelemetry("ready", false); }
    );

  // Private variables
  var cmd = "";
  var w = 0;
  var h = 0;
  var uri = "";
  var hasApi = true;
  var serviceContext = {};
  var launchParams;
  var _externalApp;
  var _browser;
  var _state = ApplicationState.RUNNING;
  var displayName;
  var userAgent = null;
  var localStorage = false;
  var appParent = null;
  var suspendDelayTimeout = null;

  // Internal function needed for suspend
  var do_suspend_internal = function(o)
  {
    //basic failure conditions
    if (_state === ApplicationState.DESTROYED){
      _this.log("suspend on already destroyed app");
      return false;
    }
    if (_state === ApplicationState.SUSPENDED){
      _this.log("suspend on already suspended app");
      return false;
    }
    if (_this.type === ApplicationType.WEB){
      if (_browser !== undefined && _browser.suspend){
        _this.log("Suspending Web app");
         
        if (_this.urlDelaysSuspend(_this.api().url) && html5_suspend_delay_seconds >= 1)
        {
          _this.log("delaying suspend and setting visibility to hidden");
          _this.api().visibility = 'hidden';
          
          if(suspendDelayTimeout != null)
          {
            _this.log("WARNING! suspendDelayTimeout already set, canceling and restarting anew");
            clearTimeout(suspendDelayTimeout);
            suspendDelayTimeout = null;
          }
          
          suspendDelayTimeout = setTimeout(function ()
          {
              _this.log("doing delayed suspend now");
              _browser.suspend();
              suspendDelayTimeout = null;
          }, html5_suspend_delay_seconds * 1000);
        }
        else
        {
          _this.log("suspending immediately");
          _browser.suspend();
        }
        
        _state = ApplicationState.SUSPENDED;
        _this.applicationSuspended();
        return true;
      }
      return false;
    }
    if (!_externalApp || !_externalApp.suspend){
      _this.log("suspend api not available on app");
      _state = ApplicationState.SUSPENDED;
      _this.applicationSuspended();
      return false;
    }
    
    var ret = true;
    _externalApp.suspend(o);
      
    if (ret === true) {
      _state = ApplicationState.SUSPENDED;
      _this.applicationSuspended();
    }
    
    _this.log("suspend returned:", ret);
    
    return ret;
  }

  // Public functions that use _externalApp
  // Suspends the application. Returns promise if the application was suspended or not.
  this.suspend = function(o) {

    //setup return promise
    var ret_promise_resolve;
    var ret_promise_reject;
    var ret_promise = new Promise(function (resolve, reject) {
      ret_promise_resolve = resolve;
      ret_promise_reject = reject;
    });
    
    //telemetry
    ret_promise.then(
      function()
      {
        _this.logTelemetry("suspend", true);
      },
      function()
      {
        _this.logTelemetry("suspend", false);
      });
    
    this.readyBase.then( 
      function() 
      { 
        //needs also remoteReady?
        if(_externalApp && _externalApp.hasApi)
        {
          _externalApp.remoteReady.then(
            function()
            {
              //readyBase and remoteReady succeeded so suspend
              var suspend_ret = do_suspend_internal(o);
              
              if(suspend_ret)
                ret_promise_resolve();
              else
                ret_promise_reject();
            },
            function()
            {
              _this.log("suspend remoteReady failed");
              ret_promise_reject();
            });
        }
        else
        {
          //otherwise attempt suspend now
          var suspend_ret = do_suspend_internal(o);

          if(suspend_ret)
            ret_promise_resolve();
          else
            ret_promise_reject();
        }
      },
      function() 
      { 
        _this.log("suspend readyBase failed");
        ret_promise_reject();
      }
    );
    
    return ret_promise;
  };
  
  // Resumes a suspended application. Returns true if the application was resumed, or false otherwise
  this.resume = function(o) {
    var ret = true;
    if (_state === ApplicationState.DESTROYED){
      this.log("resume on already destroyed app");
      return false;
    }
    if (_state === ApplicationState.RUNNING){
      this.log("resume on already running app");
      return false;
    }
    if (this.type === ApplicationType.WEB){
      if (_browser !== undefined && _browser.resume){
        this.log("Resuming Web app");
         
        if(suspendDelayTimeout != null)
        {
          _this.log("suspendDelayTimeout set, canceling");
          clearTimeout(suspendDelayTimeout);
          suspendDelayTimeout = null;
        }
         
        _browser.resume();
        _state = ApplicationState.RUNNING;
        this.applicationResumed();
        return true;
      }
      return false;
    }
    if (!_externalApp || !_externalApp.resume){
      this.log("resume api not available on app");
      _state = ApplicationState.RUNNING;
      this.applicationResumed();
      return false;
    }
    _externalApp.resume(o);
    if (ret === true) {
      _state = ApplicationState.RUNNING;
      this.applicationResumed();
      this.logTelemetry("resume", true);
    } else {
      this.log("resume returned:", ret);
      this.logTelemetry("resume", false);
    }
    return ret;
  };
  // Destroys the application. Returns true if destroyed or false if not destroyed
  this.destroy = function() {
    var ret = false;
    if (_state === ApplicationState.DESTROYED){
      this.log("destroy on already destroyed app");
      return false;
    }
    if (!_externalApp || (!_externalApp.destroy && !_externalApp.dispose)){
      this.log("destroy not supported");
      return false;
    }
    try {
      this.log("about to remove");
      if (_externalApp.remove) {
        _externalApp.remove();
      }
    } catch (e) {
      this.log("failed to remove",e);
    }
    try {
      this.log("about to destroy");
      
      if(suspendDelayTimeout != null)
      {
        _this.log("suspendDelayTimeout set, canceling");
        clearTimeout(suspendDelayTimeout);
        suspendDelayTimeout = null;
      }
      
      if (_externalApp.destroy) {
        ret = _externalApp.destroy();
      } else if (this.type === ApplicationType.SPARK && _externalApp.api && _externalApp.api.destroy) {
        ret = _externalApp.api.destroy();
      } else if (_externalApp.dispose) {
        _externalApp.dispose();
        ret = true;
      }
    } catch (e) {
      this.log("failed to destroy",e);
    }
    if (ret === true) {
      _externalApp = null;
      _state = ApplicationState.DESTROYED;
      this.applicationDestroyed();
      this.logTelemetry("destroy", true);
    } else {
      this.log("destroy returned:", ret);
      this.logTelemetry("destroy", false);
    }
    return ret;
  };
  // Sets the Z-order of this application to the highest value
  this.moveToFront = function() {
    if (_externalApp){
      _externalApp.moveToFront();
    }
  };
  // Sets the Z-order of this application to the lowest value
  this.moveToBack = function() {
    if (_externalApp){
      _externalApp.moveToBack();
    }
  };
  // Move this child in front of its next closest sibling in z-order
  this.moveForward = function() {
    if (_externalApp){
      _externalApp.moveForward();
    }
  };
  // Move this child behind its next closest sibling in z-order
  this.moveBackward = function() {
    if (_externalApp){
      _externalApp.moveBackward();
    }
  };
  // Sets the parent
  this.setParent = function(p) {
    if (_externalApp){
      _externalApp.parent = p;
    }
  };
  // Check if a link would have a delayed suspend
  this.urlDelaysSuspend = function(url_val)
  {
    var parsedURL = node_url.parse(url_val);
    var url_hostname = parsedURL.hostname;
    
    for(var i=0;i<html5_suspend_whitelist.length;i++)
      if (url_hostname.toLowerCase().indexOf(html5_suspend_whitelist[i]) != -1)
        return true;
      
    return false;
  };
  // takes a screenshot of the application
  this.screenshot = function(mimeType) {
    if (_externalApp && _externalApp.screenshot && typeof _externalApp.screenshot === "function"){
      return _externalApp.screenshot(mimeType);
    }
    return null;
  };
  // Sets the input focus to this application
  this.setFocus = function(b) {
    if (_externalApp){
      if (typeof b === 'boolean') {
        _externalApp.focus = b;
      } else {
        _externalApp.focus = true;
      }
    }
  };
  // Returns true if this application currently has focus, false if it does not
  this.isFocused = function() {
    if (_externalApp){
      return _externalApp.focus;
    } else {
      return false;
    }
  };
  this.onKeyDown = function(e) {
    if (_externalApp && _externalApp.onKeyDown !== undefined && typeof _externalApp.onKeyDown === "function" ){
      _externalApp.onKeyDown(e);
    }
  };
  this.onKeyUp = function(e) {
    if (_externalApp && _externalApp.onKeyUp !== undefined && typeof _externalApp.onKeyUp === "function" ){
      _externalApp.onKeyUp(e);
    }
  };
  this.onChar = function(e) {
    if (_externalApp && _externalApp.onChar !== undefined && typeof _externalApp.onChar === "function" ){
      _externalApp.onChar(e);
    }
  };
  // Returns a promise that will fulfill when the animation is complete.
  this.animateTo = function(animationProperties, duration, tween, type, count) {
    if (_externalApp){
      return _externalApp.animateTo(animationProperties, duration, tween, type, count);
    } else {
      return new Promise(function(resolve,reject) {
        reject("not supported");
      });
    }
  };
  // This version of animate does not return a promise.
  this.animate = function(animationProperties, duration, tween, type, count) {
    if (_externalApp){
      return _externalApp.animate(animationProperties, duration, tween, type, count);
    }
  };
  this.on = function(e,fn) {
    if (_externalApp){
      _externalApp.on(e,fn);
    }
  };
  // Returns an object containing this application's external API
  this.api = function() {
    if (this.type === ApplicationType.WEB){
      return _browser;
    } else if (_externalApp){
      return _externalApp.api;
    }
  };
  this.setProperties = function(props) {
    Object.keys(props).forEach(function(k) {
      if (k.match(/^(id|name|priority)$/g) || Object.keys(_externalAppPropsReadWrite).indexOf(k) >= 0) {
        var v = props[k];
        _this.log("set " + k + "=" + v);
        _this[k] = v;
      } else {
        _this.log("unknown property " + k);
      }
    });
  };
  this.state = function () {
    return _state;
  };
  this.paint = function(x, y, color, translateOnly) {
    if (_externalApp){
      return _externalApp.paint(x, y, color, translateOnly);
    }
  };
  this.description = function() {
    if (_externalApp){
      return _externalApp.description();
    }
  };

  // Constructor
  if ("launchParams" in props){
    launchParams = props.launchParams;
    if ("cmd" in launchParams){
      cmd = launchParams.cmd;
    }
    if ("uri" in launchParams){
      this.uri = uri = launchParams.uri;
    }
  }
  if ("w" in props){
    w = props.w;
  }
  if ("h" in props){
    h = props.h;
  }
  if ("hasApi" in props){
    hasApi = props.hasApi;
  }
  if ("metaData" in props){
    _metaData = props.metaData;
  }
  if ("userAgent" in props){
    userAgent = props.userAgent;
  }
  if ("localStorage" in props){
    localStorage = props.localStorage;
  }
  if ("parent" in props){
    appParent = props.parent;
  } else {
    appParent = root;
  }
  if (cmd === "wpe" && uri){
    cmd = cmd + " " + uri;
  }
  if("serviceContext" in props) { 
    serviceContext = props["serviceContext"]
  }
  if ("expectedMemoryUsage" in props) {
    this.expectedMemoryUsage = props.expectedMemoryUsage;
  }
  if ("displayName" in props) {
    displayName = props.displayName;
  }

  this.log("cmd:",cmd,"uri:",uri,"w:",w,"h:",h,"hasApi:",hasApi);

  if (!cmd) {
    this.log('cannot create app because cmd is not set');
    _readyBaseReject(new Error('cmd is not set'));
    _uiReadyReject();
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (!scene && !appParent) {
    this.log('cannot create app because the scene is not set');
    _readyBaseReject(new Error('scene is not set'));
    _uiReadyReject();
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (appManager.getApplicationById(props.id)) {
    this.log('cannot create app with duplicate ID: ' + props.id);
    _readyBaseReject(new Error('duplicate ID: ' + props.id));
    _uiReadyReject();
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (cmd === "spark"){
    this.type = ApplicationType.SPARK;
    _externalApp = scene.create({t:"scene", parent:appParent, url:uri, serviceContext:serviceContext});
    _externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
    _externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); }); // is never called
    _externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); }); // is never called
    _externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // is never called
    _externalApp.on("onClientStopped", function () { _this.log( "onClientStopped"); }); // is never called
    _externalApp.ready.then(function() {
      _this.log("successfully created Spark app: " + _this.id);
      _readyBaseResolve();
      _this.applicationReady();

      if(typeof(_externalApp.api.uiReady) == "object")
      {
        _externalApp.api.uiReady.then(
          function(result) { _uiReadyResolve(result); }, 
          function(err) { _uiReadyReject(err); }
        );
      }
      else
        _uiReadyResolve("no uiReady promise");
      
    }, function rejection() {
      var msg = "Failed to load uri: " + uri;
      _this.log("failed to launch Spark app: " + _this.id + ". " + msg);
      _readyBaseReject(new Error("failed to create. " + msg));
      _uiReadyReject();
      _this.applicationClosed();
    });
    this.setProperties(props);
    this.applicationCreated();
  }
  else if (cmd === "sparkInstance"){
    if (process.env.SPARK_DEBUGGER_PORT) {
      process.env.SPARK_DEBUGGER_PORT++;
    }
    this.type = ApplicationType.SPARK_INSTANCE;
    process.env.PXCORE_ESSOS_WAYLAND=1;
    process.env.WESTEROS_FAST_RENDER=0;
    if (uri === ""){
      uri = "preloadSparkInstance.js";
    }
    _externalApp = scene.create( {t:"external", parent:appParent, cmd:"spark " + uri, w:w, h:h, hasApi:true} );
    _externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
    _externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
    _externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); });
    _externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // called on client crash
    _externalApp.on("onClientStopped", function () { // called on client crash
      _this.log("onClientStopped");
      setTimeout(function () {
        _this.destroy();
      });
    });
    _externalApp.ready.then(function() {
    }, function rejection() {
      _this.log("failed to launch Spark instance app: " + _this.id);
      _readyBaseReject(new Error("failed to create"));
      _uiReadyReject();
      _this.applicationClosed();
    });
    _externalApp.remoteReady.then(function() {
      _this.log("spark instance ready");
      
      _externalApp.api.on("onApplicationLoaded", function(e)
      {
        if(e.success)
          _urlChangeResolve();
        else
          _urlChangeReject();
      });
      
      _readyBaseResolve();
      _uiReadyResolve();
      _this.applicationReady();
    }, function rejection() {
      _this.log("failed to create Spark instance");
      _readyBaseReject(new Error("failed to create"));
      _uiReadyReject();
      _this.applicationClosed();
    });
    this.setProperties(props);
    this.applicationCreated();
  }
  else if (cmd === "WebApp"){
    this.type = ApplicationType.WEB;
    _externalApp = scene.create( {t:"external", parent:appParent, server:"wl-rdkbrowser2-server", w:w, h:h, hasApi:true} );
    _externalApp.on("onReady", function () { _this.log("onReady"); });
    _externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
    _externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); });
    _externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); });
    _externalApp.on("onClientStopped", function () { _this.log("onClientStopped"); });
    _externalApp.remoteReady.then(function(obj) {
      if(obj) {
        _this.log("about to create browser window");
        _browser = _externalApp.api.createWindow(_externalApp.displayName, false);
        if (_browser) {
          
          function handleOnHTMLDocumentLoadedEvent(e)
          {
            if(e.success)
            {
              _uiReadyResolve();
              
              if(typeof(_urlChangeResolve) != "undefined")
                _urlChangeResolve();
            }
            else
            {
              _uiReadyReject();
              
              if(typeof(_urlChangeReject) != "undefined")
                _urlChangeReject();
            }
          }
          
          _browser.on("onHTMLDocumentLoaded",handleOnHTMLDocumentLoadedEvent);
          if (userAgent){
            _browser.userAgent = userAgent;
          }
          if (localStorage){
            _browser.localStorageEnabled = localStorage;
          }

          _browser.url = uri;
          _this.log("launched WebApp uri:" + uri);
          _this.applicationCreated();
          _readyBaseResolve();
          _this.applicationReady();
        } else {
          _this.log("failed to create window for WebApp");
          _readyBaseReject(new Error("failed to create"));
          _uiReadyReject();
          _this.applicationClosed();
        }
      } else {
        _this.log("failed to create WebApp invalid waylandObj");
        _readyBaseReject(new Error("failed to create"));
        _uiReadyReject();
        _this.applicationClosed();
      }
    }, function rejection() {
      _this.log("failed to create WebApp");
      _readyBaseReject(new Error("failed to create"));
      _uiReadyReject();
      _this.applicationClosed();
    });
    this.setProperties(props);
  }
  else{
    this.type = ApplicationType.NATIVE;
    _externalApp = scene.create( {t:"external", parent:appParent, cmd:cmd, w:w, h:h, hasApi:hasApi, displayName:displayName} );
    _externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
    _externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
    _externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); });
    _externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // called on client crash
    _externalApp.on("onClientStopped", function () { // called on client crash
      _this.log("onClientStopped");
      setTimeout(function () {
        _this.destroy();
      });
    });

    _externalApp.ready.then(function() {
      _this.log("successfully created: " + _this.id);
      _readyBaseResolve();
      _this.applicationReady();
    }, function rejection() {
      _this.log("failed to launch app: " + _this.id);
      _readyBaseReject(new Error("failed to create"));
      _uiReadyReject();
      _this.applicationClosed();
    });
    
    if(_externalApp.hasApi === true)
    {
      _externalApp.remoteReady.then(function(wayland)
      {
        if(typeof(wayland.api.uiReady) == "object")
        {
          wayland.api.uiReady.then( 
            function() { _uiReadyResolve("uiReady succeeded"); },
            function() { _uiReadyReject("uiReady failed"); }
            );
        }
        else
        {
          _uiReadyResolve("no uiReady promise");
        }
      },
      function()
      {
        console.log("native remoteReady error");
        console.log("uiReady state or existance unknown because of remoteReady error. Defaulting to resolved.");
        _uiReadyResolve("native remoteReady error");
      }
      );
    }
    else
    {
      _uiReadyResolve("no api");
    }
    
    this.setProperties(props);
    this.applicationCreated();
  }
}

Application.prototype.log = function() {
  var _args = ["optimus"];
  if (this.id) {
    _args.push("id="+this.id);
  }
  if (this.type) {
    _args.push("type="+this.type);
  }
  if (this.state()) {
    _args.push("state="+this.state());
  }
  if (this.name) {
    _args.push("name="+this.name);
  }
  _args.push.apply(_args, arguments);
  console.log.apply(console, _args);
};

// Returns the ApplicationManager
Application.prototype.applicationManager = function() {
  return appManager;
};

Application.prototype.applicationCreated = function(){
  this.log("applicationCreated");
  this.createTime = scene.clock();
  appManager.onCreate(this);
};
Application.prototype.applicationReady = function(){
  this.log("applicationReady");
  appManager.onReady(this);
  var loadTime = scene.clock() - this.createTime;;
  console.log("url : "+this.uri+", type : " +this.type+", "+"load time : "+loadTime.toFixed(2)+"ms");
};
Application.prototype.applicationClosed = function(){
  this.log("applicationClosed");
  appManager.onDestroy(this);
};
Application.prototype.applicationSuspended = function(){
  this.log("applicationSuspended");
  appManager.onSuspend(this);
};
Application.prototype.applicationResumed = function(){
  this.log("applicationResumed");
  appManager.onResume(this);
};
Application.prototype.applicationDestroyed = function(){
  this.log("applicationDestroyed");
  appManager.onDestroy(this);
};

function Optimus() {

  /**
   * @example
   * // create optimus app with one of the following launchParams:
   * // { "cmd":"spark","uri":"http://www.sparkui.org/examples/gallery/picturepile.js"}
   * // { "cmd":"receiver"}
   * // { "cmd":"WebApp","uri":"https://google.com"}
   * var app = optimus.createApplication(...);
   * app.ready.then(() => { app.log("ready promise!"); });
   * optimus.on("create", (app) => { app.log("create callback!"); });
   * optimus.on("ready", (app) => { app.log("ready callback!"); });
   * optimus.on("ready", (app) => { app.log("one more ready callback!"); });
   * optimus.on("suspend", (app) => { app.log("suspend callback!"); });
   * optimus.on("resume", (app) => { app.log("resume callback!"); });
   * var destroyCallback = (app) => { app.log("destroy callback!"); };
   * optimus.on("destroy", destroyCallback);
   * var animateFn = () => { return app.animateTo({ x: 600 }, 1, scene.animation.TWEEN_LINEAR, scene.animation.OPTION_FASTFORWARD, 1); };
   * setTimeout(() => { animateFn().then(() => { app.log("animated!"); }, () => { app.log("not animated!"); }); }, 30000);
   * setTimeout(() => { app.x=0; app.log("moved!"); }, 40000);
   * setTimeout(() => { app.suspend(); }, 50000);
   * setTimeout(() => { app.resume(); }, 60000);
   * setTimeout(() => { app.destroy(); }, 70000);
   * var removeListeners = () => { optimus.removeListener("suspend"); optimus.removeListener("destroy", destroyCallback); };
   * setTimeout(() => { removeListeners(); app.log("unsubscribed!"); app.suspend(); app.destroy(); }, 80000);
   * // grep log for 'optimus'
   * @param props
   * @returns {Application}
   */
  this.createApplication = function(props){
    //set the expected memory usage
    props.expectedMemoryUsage = this.getExpectedMemoryUsage(props);
    
    var app = new Application(props);
    applicationsArray.push(app);
    return app;
  };
  this.getApplications = function(){
    return applicationsArray;
  };
  this.getApplicationById = function(id){
    for (var index = 0; index < applicationsArray.length; index++){
      if (applicationsArray[index].id === id){
        return applicationsArray[index];
      }
    }
    return null;
  };
  this.getAvailableApplications = function(){
    return availableApplicationsArray;
  };
  function notifyListeners(eventName, app) {
    if (eventName in eventListenerHash){
      var _array = eventListenerHash[eventName];
      for (var i = 0; i < _array.length; i++) {
        _array[i](app);
      }
    }
  }
  this.on = function(eventName, handler){
    if (eventName in eventListenerHash){
      var _array = eventListenerHash[eventName];
      _array.push(handler);
    } else {
      eventListenerHash[eventName] = [handler];
    }
  };
  this.removeListener = function(eventName, handler){
    if (eventName in eventListenerHash){
      if (handler) {
        var _array = eventListenerHash[eventName];
        var _index = _array.indexOf(handler);
        while (_index !== -1) {
          _array.splice(_index, 1);
          _index = _array.indexOf(handler);
        }
      } else {
        delete eventListenerHash[eventName];
      }
    }
  };
  this.onCreate = function(app){
    notifyListeners("create",app);
  };
  this.onSuspend = function(app){
    notifyListeners("suspend",app);
  };
  this.onResume = function(app){
    notifyListeners("resume",app);
  };
  this.onDestroy = function(app){
    if (app){
      for (var index = 0; index < applicationsArray.length; index++){
        if (applicationsArray[index] && applicationsArray[index].id === app.id){
          applicationsArray[index] = null;
          applicationsArray.splice(index, 1);
          break;
        }
      }
    }
    notifyListeners("destroy",app);
  };
  this.onReady = function(app){
    notifyListeners("ready",app);
  };
  this.setScene = function(s){
    scene = s;
    // remove reference to scene by passing null
    if (null != s) {
      root = scene.root;
      availableApplicationsArray.splice(0,availableApplicationsArray.length);
      var availableApps = scene.getAvailableApplications();
      if (availableApps.length > 0) {
        availableApplicationsArray = JSON.parse(availableApps);
      }
    }
    else {
      root = null;
    }
  };
  this.getExpectedMemoryUsage = function(props){
    
    if(typeof(props) != "object")
      return -1;
     
    if ("expectedMemoryUsage" in props)
      return props.expectedMemoryUsage;

    if ("launchParams" in props && "cmd" in props.launchParams){
      if (props.launchParams.cmd === "WebApp")
         return 130;
      else if (props.launchParams.cmd === "spark")
         return 90;
      else if (props.launchParams.cmd === "sparkInstance")
         return 90;
      else
         return 75;
    }
  };
  this.getFreeID = function(starting_from){
    var i=0;
    
    if(typeof(starting_from) == "number")
      i = starting_from;
    
    while(this.getApplicationById(i.toString()) != null)
      i++;
    
    return i.toString();
  };
 
  function loadHTML5SuspendWhitelist()
  {
    //OPTIMUS_HTML5_DELAY_SUSPEND_FILE not set?
    if(typeof(process.env.OPTIMUS_HTML5_DELAY_SUSPEND_FILE) == "undefined")
    {
      console.log("OPTIMUS_HTML5_DELAY_SUSPEND_FILE undefined. Not loading HTML5 Delay Suspend Whitelist.");
      return;
    }

    //set / read whitelist
    try
    {
      html5_suspend_whitelist = require(process.env.OPTIMUS_HTML5_DELAY_SUSPEND_FILE);
    }
    catch(err)
    {
      console.log("loading html5_suspend_whitelist with '" + process.env.OPTIMUS_HTML5_DELAY_SUSPEND_FILE + "' failed with error '" + err.message + "'");
    }
    
    //set delay in seconds
    {
      var default_html5_suspend_delay_seconds = html5_suspend_delay_seconds;
      
      if(typeof(process.env.OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS) == "number")
        html5_suspend_delay_seconds = process.env.OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS;
      if(typeof(process.env.OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS) == "string")
        html5_suspend_delay_seconds = Number(process.env.OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS);
      else
        console.log("OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS is undefined. Will use default time of " + html5_suspend_delay_seconds);
      
      //check if nan
      if(isNaN(html5_suspend_delay_seconds))
      {
        console.log("html5_suspend_whitelist_delay_in_seconds was NaN. Possibly bad OPTIMUS_HTML5_DELAY_SUSPEND_SECONDS string?");
        html5_suspend_delay_seconds = default_html5_suspend_delay_seconds;
      }

      //disable suspend delay?
      if(html5_suspend_delay_seconds < 1)
      {
        console.log("html5_suspend_whitelist_delay_in_seconds set to less than 1, disabling html5 delay suspend.");
        html5_suspend_delay_seconds = 0;
      }
    }

    console.log("html5_suspend_whitelist loaded:");
    console.log(html5_suspend_whitelist);
    console.log("html5_suspend_whitelist_delay_in_seconds: " + html5_suspend_delay_seconds);
  }
  
  function checkLoadHTML5SuspendWhitelist()
  {
    if(process.env.OPTIMUS_HTML5_DELAY_SUSPEND === "true")
    {
      console.log("OPTIMUS_HTML5_DELAY_SUSPEND set to true. Loading HTML5 Delay Suspend Whitelist file.");
      loadHTML5SuspendWhitelist();
    }
    else if(process.env.OPTIMUS_HTML5_DELAY_SUSPEND === "false")
      console.log("OPTIMUS_HTML5_DELAY_SUSPEND set to false. Not loading HTML5 Delay Suspend Whitelist file.");
    else if(typeof(process.env.OPTIMUS_HTML5_DELAY_SUSPEND) == "undefined")
    {
      console.log("OPTIMUS_HTML5_DELAY_SUSPEND undefined. Defaulting to loading HTML5 Delay Suspend Whitelist file.");
      loadHTML5SuspendWhitelist();
    }
  }
  
  checkLoadHTML5SuspendWhitelist();
}
