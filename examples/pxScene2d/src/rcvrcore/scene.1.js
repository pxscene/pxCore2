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

var isDuk=(typeof Duktape != "undefined")?true:false;

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
      // TODO JRJR try to get rid of this stuff... 

      this.animation = scene.animation;
      this.stretch   = scene.stretch;
      this.maskOp    = scene.maskOp;
      this.alignVertical = scene.alignVertical;
      this.alignHorizontal = scene.alignHorizontal;
      this.truncation = scene.truncation;
      this.root = scene.root;
      this.info = scene.info;
      this.capabilities = scene.capabilities;
      this.filePath = filePath;
      this.addServiceProvider = scene.addServiceProvider;
      this.removeServiceProvider = scene.removeServiceProvider;
      if (!isDuk) { 
        this.__defineGetter__("w", function() { return scene.w; });
        this.__defineGetter__("h", function() { return scene.h; });
        this.__defineGetter__("showOutlines", function() { return scene.showOutlines; });
        this.__defineSetter__("showOutlines", function(v) { scene.showOutlines = v; });
        this.__defineGetter__("enableDirtyRect", function() { return scene.enableDirtyRect; });
        this.__defineSetter__("enableDirtyRect", function(v) { scene.enableDirtyRect = v; });
        this.__defineGetter__("showDirtyRect", function() { return scene.showDirtyRect; });
        this.__defineSetter__("showDirtyRect", function(v) { scene.showDirtyRect = v; });
        this.__defineSetter__("customAnimator", function(v) { scene.customAnimator = v; });
      }
      else {
        this.w = scene.w;
        this.h = scene.h;
        this.showOutlines = false;
        this.showDirtyRect = false;       
      }
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

  this.textureMemoryUsage = function() {
    return nativeScene.textureMemoryUsage();
  };

  this.collectGarbage = function() {
    return nativeScene.collectGarbage();
  };

  this.suspend = function() {
    return nativeScene.suspend({});
  };

  this.resume = function() {
    return nativeScene.resume({});
  };

  this.loadArchive = function(u) {
    return nativeScene.loadArchive(u);
  };

  this.customAnimator = function( f ) {
    return nativeScene.customAnimator( f );
  }

  this.getX = function() { return nativeScene.x; };
  this.getY = function() {
    return nativeScene.y; };

  this.getWidth = function() {
    return nativeScene.w; };
  this.getHeight = function() {
    return nativeScene.h; };

  this.create = function create(params) {
    applyStyle.call(this, params);

      function getColor(val)
      {
        clr = "" + val + "";
        
        var ans = 0x00000000; // transparent (default)
        
        // Support #RRGGBB web style color syntax
        if(clr.match(/#([0-9a-f]{6})/i) )
        {
          clr = clr.replace(/#([0-9a-f]{6})/i, "0x$1FF");  // tack on ALPHA at lsb to ($1) amtch
          ans = parseInt(clr, 16);
        }
        else
        // Support #RGB web style color syntax
        if(clr.match(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i) )
        {
          clr = clr.replace(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i, "0x$1$1$2$2$3$3FF"); //  #rgb >>> 0xrrggbb
          ans = parseInt(clr, 16);
        }
        else
        {
          ans = val; // pass-through
        }
        
        return ans;
      }
        
      // Support for Web colors using #RGB  or #RRGGBB syntax
      if(params.hasOwnProperty("textColor") )
      {
        params.textColor = getColor(params.textColor);
      }
      
      // Support for Web colors using #RGB  or #RRGGBB syntax
      if(params.hasOwnProperty("lineColor") )
      {
        params.lineColor = getColor(params.lineColor);
      }
  
      // Support for Web colors using #RGB  or #RRGGBB syntax
      if(params.hasOwnProperty("fillColor") )
      {
        params.fillColor = getColor(params.fillColor);
      }
   
      var component = null;
      if( componentDefinitions !== null && params.hasOwnProperty("t") )
      {
        component = createComponent(params);
      }

      if( component !== null ) {
        return component;
      } else {
        return nativeScene.create(params);
      }
  }; // ENDIF - create()
  
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

  this.getAvailableApplications = function getAvailableApplications(appNames) {
      return nativeScene.getAvailableApplications(appNames);
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

  this.close = function() {
    rpcContext._setRPCController(null);
    rpcContext = null;
  }

  return this;
}

module.exports = Scene;

