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

var ApplicationType = Object.freeze({
  "SPARK":1,
  "NATIVE":2,
  "WEB":3,
  "UNDEFINED":4
});

function Application(props) {
  this.id = undefined;
  this.priority	= 1;
  this.appState = "RUNNING";
  this.externalApp = undefined;
  this.appName = "";
  this.browser = undefined;
  this.type = ApplicationType.UNDEFINED;
  this.ready = undefined;

  var cmd = "";
  var w = 0;
  var h = 0;
  var uri = "";
  var launchParams;
  var _this = this;

  if ("launchParams" in props){
    launchParams = props.launchParams;
    if ("cmd" in launchParams){
      cmd = launchParams.cmd;
    }
    if ("uri" in launchParams){
      uri = launchParams.uri;
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

  this.log("cmd:",cmd,"uri:",uri,"w:",w,"h:",h);

  if (cmd){
    if (scene !== undefined) {
      if (cmd === "spark"){
        this.type = ApplicationType.SPARK;
        this.externalApp = scene.create({t:"scene", parent:root, url:uri});
        this.ready = this.externalApp.ready;
        this.externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
        this.externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); }); // is never called
        this.externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); }); // is never called
        this.externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // is never called
        this.externalApp.on("onClientStopped", function () { _this.log( "onClientStopped"); }); // is never called
        this.externalApp.ready.then(function() {
          _this.log("successfully created Spark app: " + _this.id);
          _this.applicationReady();
        }, function rejection() {
          _this.log("failed to launch Spark app: " + _this.id);
          _this.applicationClosed();
        });
        this.setProperties(props);
        this.applicationCreated();
      }
      else if (cmd === "WebApp"){
        this.type = ApplicationType.WEB;
        this.externalApp = scene.create( {t:"wayland", parent:root, server:"wl-rdkbrowser2-server", w:w, h:h, hasApi:true} );
        this.ready = this.externalApp.remoteReady;
        // The following doesn't work - causes black screen:
        //this.externalApp.on("onReady", function () { _this.log("onReady"); });
        //this.externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
        //this.externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); });
        //this.externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); });
        //this.externalApp.on("onClientStopped", function () { _this.log("onClientStopped"); });
        this.externalApp.remoteReady.then(function(obj) {
          if(obj) {
            _this.log("about to create browser window");
            _this.browser = _this.externalApp.api.createWindow(_this.externalApp.displayName, false);
            if (_this.browser) {
              _this.browser.url = uri;
              _this.log("launched WebApp uri:" + uri);
              _this.applicationCreated();
              _this.applicationReady();
            } else {
              _this.log("failed to create window for WebApp");
              _this.applicationClosed();
            }
          } else {
            _this.log("failed to create WebApp invalid waylandObj");
            _this.applicationClosed();
          }
        }, function rejection() {
          _this.log("failed to create WebApp");
          _this.applicationClosed();
        });
        this.setProperties(props);
      }
      else{
        this.type = ApplicationType.NATIVE;
        this.externalApp = scene.create( {t:"wayland", parent:root, cmd:cmd, w:w, h:h, hasApi:true} );
        this.ready = this.externalApp.ready;
        this.externalApp.on("onReady", function () { _this.log("onReady"); }); // is never called
        this.externalApp.on("onClientStarted", function () { _this.log("onClientStarted"); });
        this.externalApp.on("onClientConnected", function () { _this.log("onClientConnected"); }); // called multiple times
        this.externalApp.on("onClientDisconnected", function () { _this.log("onClientDisconnected"); }); // is never called
        this.externalApp.on("onClientStopped", function () { _this.log("onClientStopped"); }); // is never called
        this.externalApp.ready.then(function() {
          _this.log("successfully created: " + _this.id);
          _this.applicationReady();
        }, function rejection() {
          _this.log("failed to launch app: " + _this.id);
          _this.applicationClosed();
        });
        this.setProperties(props);
        this.applicationCreated();
      }
    } else {
      this.log("cannot create app because the scene is not set");
      this.applicationClosed();
    }
  }
  else{
    this.setProperties(props);
  }
}

Application.prototype.log = function() {
  var _args = ["optimus"];
  if (this.id) {
    _args.push("id="+this.id);
  }
  if (this.type) {
    for (var key in ApplicationType) {
      if (ApplicationType[key] === this.type) {
        _args.push("type="+key);
        break;
      }
    }
  }
  if (this.appState) {
    _args.push("state="+this.appState);
  }
  if (this.appName) {
    _args.push("name="+this.appName);
  }
  _args.push.apply(_args, arguments);
  console.log.apply(console, _args);
};

Application.prototype.applicationManager = function() {
  return module.exports;
};
Application.prototype.applicationCreated = function(){
  this.log("applicationCreated");
  var appManager = module.exports;
  if (appManager){
    appManager.onCreate(this);
  }
};
Application.prototype.applicationReady = function(){
  this.log("applicationReady");
  var appManager = module.exports;
  if (appManager){
    appManager.onReady(this);
  }
};
Application.prototype.applicationClosed = function(){
  this.log("applicationClosed");
  var appManager = module.exports;
  if (appManager){
    appManager.onDestroy(this);
  }
};
Application.prototype.applicationSuspended = function(){
  this.log("applicationSuspended");
  var appManager = module.exports;
  if (appManager){
    appManager.onDestroy(this);
  }
};
Application.prototype.applicationResumed = function(){
  this.log("applicationResumed");
  var appManager = module.exports;
  if (appManager){
    appManager.onResume(this);
  }
};
Application.prototype.applicationDestroyed = function(){
  this.log("applicationDestroyed");
  var appManager = module.exports;
  if (appManager){
    appManager.onDestroy(this);
  }
};

Application.prototype.suspend = function() {
  if (this.externalApp && this.externalApp.suspend){
    this.externalApp.suspend();
  }
  if (this.appState !== "DESTROYED"){
    this.appState = "SUSPENDED";
  }
  this.applicationSuspended();
  return true;
};
Application.prototype.resume = function() {
  if (this.externalApp && this.externalApp.resume){
    this.externalApp.resume();
  }
  if (this.appState !== "DESTROYED"){
    this.appState = "RUNNING";
  }
  this.applicationResumed();
  return true;
};
Application.prototype.destroy = function() {
  if (this.externalApp){
    try {
      this.log("about to remove");
      if (this.externalApp.remove) {
        this.externalApp.remove();
      }
    } catch (e) {
      this.log("failed to remove",e);
    }
    try {
      this.log("about to destroy");
      if (this.externalApp.destroy) {
        this.externalApp.destroy();
      }
    } catch (e) {
      this.log("failed to destroy",e);
    }
    this.externalApp = null;
  }
  this.appState = "DESTROYED";
  this.applicationDestroyed();
  return true;
};

Application.prototype.state = function() {
  return this.appState;
}
Application.prototype.name = function() {
  return this.appName;
}
Application.prototype.moveToFront = function() {
  if (this.externalApp){
    this.externalApp.moveToFront();
  }
  return true;
}
Application.prototype.moveToBack = function() {
  if (this.externalApp){
    this.externalApp.moveToBack();
  }
  return true;
}
Application.prototype.setFocus = function() {
  if (this.externalApp){
    this.externalApp.focus = true;
  }
  return true;
}
Application.prototype.animateTo = function(animationProperties, duration, tween, type, count) {
  if (this.externalApp){
    return this.externalApp.animateTo(animationProperties, duration, tween, type, count);
  } else {
    var promise = new Promise(function(resolve,reject) {
      reject("app");
    });
    return promise;
  }
}
Application.prototype.animate = function(animationProperties, duration, tween, type, count) {
  if (this.externalApp){
    this.externalApp.animate(animationProperties, duration, tween, type, count);
  }
}

Application.prototype.on = function(e,fn) {
  if (this.externalApp){
    this.externalApp.on(e,fn);
  }
}


Application.prototype.api = function() {
  if (this.type === ApplicationType.WEB){
    if (this.browser){
      return this.browser;
    } else {
      return null;
    }
  }
  else if (this.externalApp){
    return this.externalApp.api;
  } else {
    return null;
  }
}

Application.prototype.setProperties = function(props) {
  var app = this.externalApp;
  for (var key in props)
  {
    switch(key){
      case "id":
        this.id = props[key];
        break;
      case "name":
        this.appName = props[key];
        break;
      case "priority":
        this.priority = props[key];
        break;
      case "x":
        if (app) {app.x = props[key];}
        break;
      case "y":
        if (app) {app.y = props[key];}
        break;
      case "w":
        if (app) {app.w = props[key];}
        break;
      case "h":
        if (app) {app.h = props[key];}
        break;
      case "cx":
        if (app) {app.cx = props[key];}
        break;
      case "cy":
        if (app) {app.cy = props[key];}
        break;
      case "sx":
        if (app) {app.sx = props[key];}
        break;
      case "sy":
        if (app) {app.sy = props[key];}
        break;
      case "r":
        if (app) {app.r = props[key];}
        break;
      case "a":
        if (app) {app.a = props[key];}
        break;
      case "interactive":
        if (app) {app.interactive = props[key];}
        break;
      case "painting":
        if (app) {app.painting = props[key];}
        break;
      case "clip":
        if (app) {app.clip = props[key];}
        break;
      case "mask":
        if (app) {app.mask = props[key];}
        break;
      case "draw":
        if (app) {app.draw = props[key];}
        break;
      default:
        this.log("unknown property " + key);
        break;
    }
  }
  return true;
}


function Optimus() {

  this.createApplication = function(props){
      /*id - (String) the id of the application
      priority - (integer) value between 1 and 10 inclusive
      x – (float) x-coordinate used as input into the object's transform function in pixel units
      y – (float) y-coordinate used as input into the object's transform function in pixel units
      w – (float) pixel unit width of the object
      h – (float) pixel unit height of the object
      cx - (float) x offset used as the center of rotation and scale
      cy - (float) y offset used as the center of rotation and scale
      sx – (float) scale factor in the x dimension
      sy – (float) scale factor in the y dimension
      r - (float) angle of rotation in degrees
      a – (float) alpha or opacity of the object [0-1]
      interactive - (boolean) determines whether the application is mouse interactive. defaults to true.
      painting - (boolean) when set to false the application will be snapshotted. when set to true the application will immediately reflect any changes
      clip - (boolean) determines whether the drawing done by the object and it's children will be clipped by the objects w and h properties. defaults to false.
      mask - (boolean) determines whether this application will be used to define an alpha layer mask for the siblings of this application. defaults to false.
      draw - (boolean) determines whether this object will be drawn. defaults to true.
      launchParams  (Object) - a set of k/v pairs to be passed to the application */

      var app = new Application(props);
      applicationsArray.push(app);

      Object.defineProperties(app, {
        'x' : { get: function() { return this.externalApp.x;}, set: function(v) { this.externalApp.x = v;}},
        'y' : { get: function() { return this.externalApp.y;}, set: function(v) { this.externalApp.y = v;}},
        'w' : { get: function() { return this.externalApp.w;}, set: function(v) { this.externalApp.w = v;}},
        'h' : { get: function() { return this.externalApp.h;}, set: function(v) { this.externalApp.h = v;}},
        'cx' : { get: function() { return this.externalApp.cx;}, set: function(v) { this.externalApp.cx = v;}},
        'cy' : { get: function() { return this.externalApp.cy;}, set: function(v) { this.externalApp.cy = v;}},
        'sx' : { get: function() { return this.externalApp.sx;}, set: function(v) { this.externalApp.sx = v;}},
        'sy' : { get: function() { return this.externalApp.sy;}, set: function(v) { this.externalApp.sy = v;}},
        'r' : { get: function() { return this.externalApp.r;}, set: function(v) { this.externalApp.r = v;}},
        'a' : { get: function() { return this.externalApp.a;}, set: function(v) { this.externalApp.a = v;}},
        'interactive' : { get: function() { return this.externalApp.interactive;}, set: function(v) { this.externalApp.interactive = v;}},
        'painting' : { get: function() { return this.externalApp.painting;}, set: function(v) { this.externalApp.painting = v;}},
        'clip' : { get: function() { return this.externalApp.clip;}, set: function(v) { this.externalApp.clip = v;}},
        'mask' : { get: function() { return this.externalApp.mask;}, set: function(v) { this.externalApp.mask = v;}},
        'draw' : { get: function() { return this.externalApp.draw;}, set: function(v) { this.externalApp.draw = v;}},
        'hasApi' : { get: function() { return this.externalApp.hasApi;}},
        'pid' : { get: function() { return this.externalApp.clientPID;}},
      });
      return app;
  }

  this.getApplications = function(){
    return applicationsArray;
  }

  this.getApplicationById = function(id){
    for (var index = 0; index < applicationsArray.length; index++){
      if (applicationsArray[index].id === id){
        return applicationsArray[index];
      }
    }
    return null;
  }

  this.getAvailableApplications = function(){
    return availableApplicationsArray;
  }

  this.on = function(eventName, handler){
    eventListenerHash[eventName] = handler;
  }

  this.onCreate = function(app){
    if ("create" in eventListenerHash && typeof eventListenerHash["create"] === 'function'){
      eventListenerHash["create"](app);
    }
  }
  this.onSuspend = function(app){
    if ("suspend" in eventListenerHash && typeof eventListenerHash["suspend"] === 'function'){
      eventListenerHash["suspend"](app);
    }
  }
  this.onResume = function(app){
    if ("resume" in eventListenerHash && typeof eventListenerHash["resume"] === 'function'){
      eventListenerHash["resume"](app);
    }
  }
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
    if ("destroy" in eventListenerHash && typeof eventListenerHash["destroy"] === 'function'){
      eventListenerHash["destroy"](app);
    }
  }
  this.onReady = function(app){
    if ("ready" in eventListenerHash && typeof eventListenerHash["ready"] === 'function'){
      eventListenerHash["ready"](app);
    }
  }

  this.setScene = function(s){
    scene = s;
    root = scene.root;
    availableApplicationsArray.splice(0,availableApplicationsArray.length);
    var availableApps = scene.getAvailableApplications();
    if (availableApps.length > 0)
    {
      availableApplicationsArray = JSON.parse(availableApps);
    }
  }
}

module.exports = new Optimus();


