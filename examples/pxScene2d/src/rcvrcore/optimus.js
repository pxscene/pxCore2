'use strict';

//application manager variables
var applicationsArray = [];
var availableApplicationsArray = [];
var eventListenerHash = {}

var scene = undefined;
var root = undefined;

function Application(props) {
  this.id;
  this.priority	= 1;
  this.appState = "RUNNING";
  this.externalApp = undefined;
  this.appName = "";

  var cmd = "";
  var w = 0;
  var h = 0;

  if ("launchParams" in props){
    var launchParams = props["launchParams"];
    if ("cmd" in launchParams){
      cmd = launchParams["cmd"];
    }
    console.log("cmd: " + cmd);
  }
  if ("w" in props){
    w = props["w"];
  }
  if ("h" in props){
    h = props["h"];
  }
  if (cmd){
    if (scene !== undefined) {
      if (cmd.includes(".js")){
        this.externalApp = scene.create({t:"scene", parent:root, url:cmd});
        this.externalApp.on("onClientStopped", this.applicationClosed);
        var sparkApp = this;
        this.externalApp.ready.then(function(o) {
            console.log("successfully created Spark app: " + sparkApp.id);
            this.applicationReady();
          }, function rejection(o) {
          console.log("failed to launch Spark app: " + sparkApp.id);
          module.exports.onDestroy(sparkApp);
        });
        this.setProperties(props);
        module.exports.onCreate(this);
      }
      else{
        this.externalApp = scene.create( {t:"wayland", parent:root, cmd:cmd, w:w, h:h, hasApi:true} );
        this.externalApp.on("onReady", this.applicationReady);
        this.externalApp.on("onClientStopped", this.applicationClosed);
        this.externalApp.on("onClientDisconnected", this.applicationClosed);
        this.setProperties(props);
        var thisApp = this;
        this.externalApp.ready.then(function(o) {
            console.log("successfully created: " + thisApp.id);
            module.exports.onCreate(thisApp);
          }, function rejection(o) {
          console.log("failed to launch app: " + thisApp.id);
          module.exports.onDestroy(thisApp);
        });
      }
    }
    else
    {
      console.log("cannot create app because the scene is not set");
      var destroyedApp = this;
      module.exports.onDestroy(destroyedApp);
    }
  }
  else{
    this.setProperties(props);
  }
}
Application.prototype.applicationManager = function() {
  return module.exports;
}
Application.prototype.applicationReady = function(e){
  var appManager = module.exports;
  if (appManager){
    appManager.onReady(this);
  }
}
Application.prototype.applicationClosed = function(e){
  var appManager = module.exports;
  if (appManager){
    appManager.onDestroy(this);
  }
}
Application.prototype.suspend = function() {
  if (this.externalApp){
    this.externalApp.suspend();
  }
  var appManager = module.exports;
  if (appManager){
    appManager.onSuspend(this);
  }
  if (this.appState !== "DESTROYED"){
    this.appState = "SUSPENDED";
  }
  return true;
}
Application.prototype.resume = function() {
  if (this.externalApp){
    this.externalApp.resume();
  }
  var appManager = module.exports;
  if (appManager){
    appManager.onResume(this);
  }
  if (this.appState !== "DESTROYED"){
    this.appState = "RUNNING";
  }
  return true;
}
Application.prototype.destroy = function() {
  if (this.externalApp){
    if (this.externalApp.destroy) {
      this.externalApp.destroy();
    }
    this.externalApp.remove();
    this.externalApp = null;
  }
  var appManager = module.exports;
  if (appManager){
    appManager.onDestroy(this);
  }
  this.appState = "DESTROYED";
  return true;
}
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
Application.prototype.setFocus = function(focus) {
  if (this.externalApp){
    this.externalApp.focus = focus;
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
  if (this.externalApp){
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
        console.log("unknown property " + key);
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


