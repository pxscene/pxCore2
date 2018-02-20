var isDuk = (typeof timers != "undefined")?true:false;

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
      this.alignVertical = scene.alignVertical;
      this.alignHorizontal = scene.alignHorizontal;
      this.truncation = scene.truncation;
      this.root = scene.root;
      this.info = scene.info;
      this.filePath = filePath;
      this.addServiceProvider = scene.addServiceProvider;
      this.removeServiceProvider = scene.removeServiceProvider;
      if (!isDuk) { 
        this.__defineGetter__("w", function() { return scene.w; });
        this.__defineGetter__("h", function() { return scene.h; });
        this.__defineGetter__("showOutlines", function() { return scene.showOutlines; });
        this.__defineSetter__("showOutlines", function(v) { scene.showOutlines = v; });
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

    if(params.t === "path")
    {
      if(params.hasOwnProperty("strokeColor") )
      {
        var clr = "" + params.strokeColor + "";
        
        // Support #RRGGBB  and #RGB web style color syntax
        if(clr.match(/#([0-9a-f]{6})/i) )
        {
          clr = clr.replace(/#([0-9a-f]{6})/i, "0x$1FF");
          params.strokeColor = parseInt(clr, 16);
        }
        else
          if(clr.match(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i) )
          {
            clr = clr.replace(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i, "0x0$10$20$3FF");
            params.strokeColor = parseInt(clr, 16);
          }
        
      }
      
      if(params.hasOwnProperty("fillColor") )
      {
        var clr = "" + params.fillColor + "";
        
        // Support #RRGGBB  and #RGB web style color syntax
        if(clr.match(/#([0-9a-f]{6})/i) )
        {
          clr = clr.replace(/#([0-9a-f]{6})/i, "0x$1FF");
          params.fillColor = parseInt(clr, 16);
        }
        else
          if(clr.match(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i) )
          {
            clr = clr.replace(/#([0-9a-f]{1})([0-9a-f]{1})([0-9a-f]{1})/i, "0x0$10$20$3FF");
            params.fillColor = parseInt(clr, 16);
          }
      }
      
      if(params.hasOwnProperty("d") )
    {
        if(params.d.match(/rect/i) )
        {
          params.d = params.d.replace(/rect/gi, "RECT");
          
          // normalize the path
          params.d = params.d.replace(/,/g," ")
          .replace(/-/g," -")
          .replace(/ +/g," ");
        }
        else
        if(params.d.match(/circle/i) )
        {
          params.d = params.d.replace(/circle/gi, "CIRCLE");
          
          // normalize the path
          params.d = params.d.replace(/,/g," ")
          .replace(/-/g," -")
          .replace(/ +/g," ");
        }
        else
        if(params.d.match(/ellipse/i))
        {
          params.d = params.d.replace(/ellipse/gi, "ELLIPSE");
          
          // normalize the path
          params.d = params.d.replace(/,/g," ")
          .replace(/-/g," -")
          .replace(/ +/g," ");
        }
        else
        if(params.d.match(/polygon/i))
        {
          params.d = params.d.replace(/polygon/gi, "POLYGON");
          
          // normalize the path
          params.d = params.d.replace(/,/g," ")
          .replace(/-/g," -")
          .replace(/ +/g," ");
        }
        else
        {
          // normalize the path
          params.d = params.d.replace(/\s*([mlvhqczastTSAMLVHQCZ])\s*/g,"\n$1 ")
          .replace(/,/g," ")
          .replace(/-/g," -")
          .replace(/ +/g," ");
        }
      } // 'd' path
    }//"path"
 
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

  this.close = function() {
    rpcContext._setRPCController(null);
    rpcContext = null;
  }

  return this;
}

module.exports = Scene;

