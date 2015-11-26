function Scene() {
  var nativeScene = null;
  var stylePatterns = [];

  this._setNativeScene = function(scene, filePath) {
    if( nativeScene === null ) {
      nativeScene = scene;
      this.PX_LINEAR = scene.PX_LINEAR;
      this.PX_EXP1 = scene.PX_EXP1;
      this.PX_EXP2 = scene.PX_EXP2;
      this.PX_EXP3 = scene.PX_EXP3;
      this.PX_LOOP = scene.PX_LOOP;
      this.PX_SEESAW = scene.PX_SEESAW;
      this.PX_STOP = scene.PX_STOP;
      this.PX_INQUAD = scene.PX_INQUAD;
      this.PX_INCUBIC = scene.PX_INCUBIC;
      this.PX_INBACK = scene.PX_INBACK;
      this.PX_EASEINELASTIC = scene.PX_EASEINELASTIC;
      this.PX_EASEOUTELASTIC = scene.PX_EASEOUTELASTIC;
      this.PX_EASEOUTBOUNCE = scene.PX_EASEOUTBOUNCE;
      this.PX_END = scene.PX_END;
      this.allInterpolators = scene.allInterpolators;
      this.root = scene.root;
      this.filePath = filePath;
      this.w = scene.w;
      this.h = scene.h;
    }
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

  this.createRectangle = function(params) {
    return nativeScene.createRectangle(params)
  }

  this.createText = function(params) {
    return nativeScene.createText(params)
  }

  this.createImage = function(params) {
    return nativeScene.createImage(params)
  }

  this.createImage9 = function(params) {
    return nativeScene.createImage9(params);
  }

  //TODO - what is createExternal used for?  Testing only?
  this.createExternal = function(params) {
    if( params.parent === undefined ) {
      params.parent = nativeScene.root;
    }
    return nativeScene.createExternal(params);
  }

  this.createScene = function(params) {
    if( params.parent === undefined ) {
      params.parent = nativeScene.root;
    }
    return nativeScene.createScene(params);
  }

  this.setFocus = function(element) {
    return nativeScene.setFocus(element);
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

  this.getFont = function getFont(url) {
    return nativeScene.getFont(url);
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

