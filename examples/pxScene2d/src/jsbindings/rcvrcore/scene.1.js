var RPCContext = require('rcvrcore/rpcContext');

function Scene() {
  var nativeScene = null;
  var stylePatterns = [];
  var rpcContext = new RPCContext(this);

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
      this.w = scene.w;
      this.h = scene.h;
    }
  }

  this._setRPCController = function(rpcController) {
    rpcContext._setRPCController(rpcController);
  }

  this.getRPCContext = function getRPCContext() {
    return rpcContext;
  }

  this.loadArchive = function(u) {
    return nativeScene.loadArchive(u);
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

    return nativeScene.create(params);
  }

  //this.createRectangle = function(params) {
    //return nativeScene.createRectangle(params)
  //}

  //this.createText = function(params) {
    //return nativeScene.createText(params)
  //}

  //this.createImage = function(params) {
    //return nativeScene.createImage(params)
  //}

  //this.createImage9 = function(params,b1, b2) {
    //return nativeScene.createImage9(params, b1, b2);
  //}

  ////TODO - what is createExternal used for?  Testing only?
  //this.createExternal = function(params) {
    //if( params.parent === undefined ) {
      //params.parent = nativeScene.root;
    //}
    //return nativeScene.createExternal(params);
  //}

  //this.createScene = function(params) {
    //if( params.parent === undefined ) {
      //params.parent = nativeScene.root;
    //}
    //return nativeScene.createScene(params);
  //}

  this.getFocus = function(element) {
    return nativeScene.getFocus(element);
  }

  this.on = function(eventType, func) {
    return nativeScene.on(eventType, func);
  }

  this.defineStyle = function(paramMatchSet, styleParams) {
    var entry = [paramMatchSet,styleParams];
    stylePatterns.push([paramMatchSet,styleParams]);
  }

  this.clock = function clock() {
    return nativeScene.clock();
  }

  //this.getFont = function getFont(url) {
    //return nativeScene.getFont(url);
  //}

  this.screenshot = function screenshot(type, pngData) {
    return nativeScene.screenshot(type, pngData);
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
        if( !createParams.hasOwnProperty(patternKey) ) {
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

