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

var scene;
var root;
var appManager = new Optimus();
module.exports = appManager;

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

  // Getters/setters
  var _externalAppPropsReadWrite = {
    x:"x",y:"y",w:"w",h:"h",cx:"cx",cy:"cy",sx:"sx",sy:"sy",r:"r",a:"a",
    interactive:"interactive",painting:"painting",clip:"clip",mask:"mask",draw:"draw",hasApi:"hasApi", url:"url"
  };
  var _externalAppPropsReadonly = {
    pid:"clientPID" // integer process id associated with the application
  };
  var _this = this;
  Object.keys(_externalAppPropsReadWrite).forEach(function(key) {
    Object.defineProperty(_this, key, {
      get: function() { return _externalApp[_externalAppPropsReadWrite[key]]; },
      set: function(v) {
           if (this.type === ApplicationType.SPARK_INSTANCE && _externalApp.api && key === "url") {
             // set the api property for spark instance
             _externalApp.api.url = v;
           }
           _externalApp[_externalAppPropsReadWrite[key]] = v;
         }
    });
  });
  Object.keys(_externalAppPropsReadonly).forEach(function(key) {
    Object.defineProperty(_this, key, {
      get: function() { return _externalApp[_externalAppPropsReadonly[key]]; }
    });
  });

  // Public variables
  this.id = undefined;
  this.priority = 1;
  this.name = "";
  this.createTime = 0;
  this.uri = "";
  this.type = ApplicationType.UNDEFINED;
  var _readyResolve = undefined;
  var _readyReject = undefined;
  this.ready = new Promise(function (resolve, reject) {
    _readyResolve = resolve;
    _readyReject = reject;
  });

  // Private variables
  var cmd = "";
  var w = 0;
  var h = 0;
  var uri = "";
  var serviceContext = {};
  var launchParams;
  var _externalApp;
  var _browser;
  var _state = ApplicationState.RUNNING;

  // Public functions that use _externalApp
  // Suspends the application. Returns true if the application was suspended, or false otherwise
  this.suspend = function(o) {
    var ret;
    if (_state === ApplicationState.DESTROYED){
      this.log("suspend on already destroyed app");
      return false;
    }
    if (_state === ApplicationState.SUSPENDED){
      this.log("suspend on already suspended app");
      return false;
    }
    if (!_externalApp || !_externalApp.suspend){
      this.log("suspend api not available on app");
      _state = ApplicationState.SUSPENDED;
      this.applicationSuspended();
      return false;
    }
    ret = _externalApp.suspend(o);
    if (ret === true) {
      _state = ApplicationState.SUSPENDED;
      this.applicationSuspended();
    } else {
      this.log("suspend returned:", ret);
    }
    return ret;
  };
  // Resumes a suspended application. Returns true if the application was resumed, or false otherwise
  this.resume = function(o) {
    var ret;
    if (_state === ApplicationState.DESTROYED){
      this.log("resume on already destroyed app");
      return false;
    }
    if (_state === ApplicationState.RUNNING){
      this.log("resume on already running app");
      return false;
    }
    if (!_externalApp || !_externalApp.resume){
      this.log("resume api not available on app");
      _state = ApplicationState.RUNNING;
      this.applicationResumed();
      return false;
    }
    ret = _externalApp.resume(o);
    if (ret === true) {
      _state = ApplicationState.RUNNING;
      this.applicationResumed();
    } else {
      this.log("resume returned:", ret);
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
    } else {
      this.log("destroy returned:", ret);
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
  // Sets the input focus to this application
  this.setFocus = function() {
    if (_externalApp){
      _externalApp.focus = true;
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
  if (cmd === "wpe" && uri){
    cmd = cmd + " " + uri;
  }
  if("serviceContext" in props) { 
    serviceContext = props["serviceContext"]
  }

  this.log("cmd:",cmd,"uri:",uri,"w:",w,"h:",h);

  if (!cmd) {
    this.log('cannot create app because cmd is not set');
    _readyReject(new Error('cmd is not set'));
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (!scene) {
    this.log('cannot create app because the scene is not set');
    _readyReject(new Error('scene is not set'));
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (appManager.getApplicationById(props.id)) {
    this.log('cannot create app with duplicate ID: ' + props.id);
    _readyReject(new Error('duplicate ID: ' + props.id));
    setTimeout(function () { // app not created yet
      _this.applicationClosed();
    });
  }
  else if (cmd === "spark"){
    this.type = ApplicationType.SPARK;
    _externalApp = scene.create({t:"scene", parent:root, url:uri, serviceContext:serviceContext});
    _externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
    _externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); }); // is never called
    _externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); }); // is never called
    _externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // is never called
    _externalApp.on("onClientStopped", function () { _this.log( "onClientStopped"); }); // is never called
    _externalApp.ready.then(function() {
      _this.log("successfully created Spark app: " + _this.id);
      _readyResolve();
      _this.applicationReady();
    }, function rejection() {
      _this.log("failed to launch Spark app: " + _this.id);
      _readyReject(new Error("failed to create"));
      _this.applicationClosed();
    });
    this.setProperties(props);
    this.applicationCreated();
  }
  else if (cmd === "sparkInstance"){
    this.type = ApplicationType.SPARK_INSTANCE;
    _externalApp = scene.create( {t:"external", parent:root, cmd:"spark " + uri, w:w, h:h, hasApi:true} );
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
      _readyReject(new Error("failed to create"));
      _this.applicationClosed();
    });
    _externalApp.remoteReady.then(function() {
      _this.log("spark instance ready");
      _readyResolve();
      _this.applicationReady();
    }, function rejection() {
      _this.log("failed to create Spark instance");
      _readyReject(new Error("failed to create"));
      _this.applicationClosed();
    });
    this.setProperties(props);
    this.applicationCreated();
  }
  else if (cmd === "WebApp"){
    this.type = ApplicationType.WEB;
    _externalApp = scene.create( {t:"external", parent:root, server:"wl-rdkbrowser2-server", w:w, h:h, hasApi:true} );
    // The following doesn't work - causes black screen:
    //_externalApp.on("onReady", function () { _this.log("onReady"); });
    //_externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
    //_externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); });
    //_externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); });
    //_externalApp.on("onClientStopped", function () { _this.log("onClientStopped"); });
    _externalApp.remoteReady.then(function(obj) {
      if(obj) {
        _this.log("about to create browser window");
        _browser = _externalApp.api.createWindow(_externalApp.displayName, false);
        if (_browser) {
          _browser.url = uri;
          _this.log("launched WebApp uri:" + uri);
          _this.applicationCreated();
          _readyResolve();
          _this.applicationReady();
        } else {
          _this.log("failed to create window for WebApp");
          _readyReject(new Error("failed to create"));
          _this.applicationClosed();
        }
      } else {
        _this.log("failed to create WebApp invalid waylandObj");
        _readyReject(new Error("failed to create"));
        _this.applicationClosed();
      }
    }, function rejection() {
      _this.log("failed to create WebApp");
      _readyReject(new Error("failed to create"));
      _this.applicationClosed();
    });
    this.setProperties(props);
  }
  else{
    this.type = ApplicationType.NATIVE;
    _externalApp = scene.create( {t:"external", parent:root, cmd:cmd, w:w, h:h, hasApi:true} );
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
      _readyResolve();
      _this.applicationReady();
    }, function rejection() {
      _this.log("failed to launch app: " + _this.id);
      _readyReject(new Error("failed to create"));
      _this.applicationClosed();
    });
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
   * // { "cmd":"spark","uri":"http://www.pxscene.org/examples/px-reference/gallery/picturepile.js"}
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
    root = scene.root;
    availableApplicationsArray.splice(0,availableApplicationsArray.length);
    var availableApps = scene.getAvailableApplications();
    if (availableApps.length > 0) {
      availableApplicationsArray = JSON.parse(availableApps);
    }
  };
}
