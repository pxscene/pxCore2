var RPCContext = require('rcvrcore/rpcContext');

function Scene() {
  var nativeScene = null;
  var stylePatterns = [];
  var rpcContext = new RPCContext(this);
  var appContextMap = {};
  var componentDefinitions = null;

  this._setNativeScene = function(scene, filePath) {
    if( nativeScene === null ) {
      nativeScene = scene;
      this.animation = scene.animation;
      this.stretch   = scene.stretch;
      this.alignVertical = scene.alignVertical;
      this.alignHorizontal = scene.alignHorizontal;
      this.truncation = scene.truncation;
      this.root = scene.root;
      this.filePath = filePath;
      this.__defineGetter__("w", function() { return scene.w; });
      this.__defineGetter__("h", function() { return scene.h; });
      this.__defineGetter__("showOutlines", function() { return scene.showOutlines; });
      this.__defineSetter__("showOutlines", function(v) { scene.showOutlines = v; });
      this.__defineGetter__("showDirtyRect", function() { return scene.showDirtyRect; });
      this.__defineSetter__("showDirtyRect", function(v) { scene.showDirtyRect = v; });
      //this.w = scene.w;
      //this.h = scene.h;
    }
  };

  this._setRPCController = function(rpcController) {
    rpcContext._setRPCController(rpcController);
  };

  this.getRPCContext = function getRPCContext() {
    return rpcContext;
  };

  this.logDebugMetrics = function() {
    return nativeScene.logDebugMetrics();
  };

  this.loadArchive = function(u) {
    return nativeScene.loadArchive(u);
  };

  this.getX = function() { return nativeScene.x; };
  this.getY = function() {
    return nativeScene.y; };

  this.getWidth = function() {
    return nativeScene.w; };
  this.getHeight = function() {
    return nativeScene.h; };

  this.create = function create(params) {
    applyStyle.call(this, params);

    var component = null;
    if( componentDefinitions !== null && params.hasOwnProperty("t") ) {
      component = createComponent(params);
    }

    if( component !== null ) {
      return component;
    } else {
      return nativeScene.create(params);
    }
  };
  
  this.stopPropagation = function() {
    return nativeScene.stopPropagation();
  };
  
  this.getFocus = function(element) {
    return nativeScene.getFocus(element);
  };

  this.on = function(eventType, func) {
    return nativeScene.on(eventType, func);
  };

  this.defineStyle = function(paramMatchSet, styleParams) {
    var entry = [paramMatchSet,styleParams];
    stylePatterns.push([paramMatchSet,styleParams]);
  };

  this.clock = function clock() {
    return nativeScene.clock();
  };

  this.screenshot = function screenshot(type, pngData) {
    return nativeScene.screenshot(type, pngData);
  };
    
  this.clipboardGet = function clipboardGet(type) {
      return nativeScene.clipboardGet(type);
  };

  this.clipboardSet = function clipboardSet(type, clip) {
      return nativeScene.clipboardSet(type, clip);
  };

  this.getService = function getService(name, serviceObject) {
    return nativeScene.getService(name, serviceObject);
  };
    
  this.setAppContext = function(appContextName, appContext) {
    if( !appContextMap.hasOwnProperty(appContextName) ) {
      appContextMap[appContextName] = appContext;
      return true;
    }

    return false;
  };

  this.getAppContext = function(appContextName) {
    return appContextMap[appContextName];
  };

  this.addComponentDefinitions = function(appComponentDefinitions) {
    componentDefinitions = appComponentDefinitions;
  };

  function createComponent(params) {
    if( componentDefinitions === null ) {
      return null;
    }
    if( componentDefinitions.hasOwnProperty(params.t) && typeof(componentDefinitions[params.t]) === "function") {
      return new componentDefinitions[params.t](params);
    } else {
      return null;
    }
  }

  function applyStyle(createParams) {
    var currentMatch = null;
    var currentKeysMatched = 0;

    for(var currentPatternSet=0; currentPatternSet < stylePatterns.length; ++currentPatternSet) {
      var patternSet = stylePatterns[currentPatternSet];
      var patternMatchSet = patternSet[0];
      var patternParams = patternSet[1];
      var allKeysMatched = true;
      var totalKeysMatched = 0;
      for (var patternKey in patternMatchSet) {
        if( !createParams.hasOwnProperty(patternKey)  || patternMatchSet[patternKey] !== createParams[patternKey] ) {
          // don't consider this set anymore
          allKeysMatched = false;
          break;
        } else {
          ++totalKeysMatched;
        }
      }
      if( allKeysMatched ) {
        if( totalKeysMatched > currentKeysMatched ) {
          currentMatch = patternParams;
        }
      }
    }

    if( currentMatch !== null ) {
      for(var key in currentMatch ) {
        if( !createParams.hasOwnProperty(key) ) {
          createParams[key] = currentMatch[key];
        }
      }
    }

  }

  return this;
}

module.exports = Scene;

