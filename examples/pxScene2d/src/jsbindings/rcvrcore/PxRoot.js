var px = require("px");
var fs = require("fs");
var AppSceneContext = require('rcvrcore/AppSceneContext');
var SceneManager = require('rcvrcore/SceneManager');

// The singleton instance of pxRoot
var pxRoot = null;

// The public XRE system APIs
var XreSysApis = {
  getFrameworkImagePath: getFrameworkImagePath,
  setRootKeyHandler: setRootKeyHandler,
  getIt: function()  {
    return "Hey: I'm the XRE system";
  }
}

var rootKeyHandler = null;

function PxRoot(baseUri) {
  this.rootScene = null;
  this.baseUri = baseUri
  this.sceneManager = new SceneManager(this);
  this.bundleManager = null;
}

PxRoot.prototype.initialize = function(x, y, width, height) {
  this.rootScene = px.getScene(x, y, width, height);
  this.rootScene.aName = "PxRoot";

  this.rootScene.root.on('onKeyDown', function (e) {
    //log.message(2, "PxRoot(root): keydown:" + e.keyCode);
    console.log("PxRoot: got key " + e.keyCode);
    if( rootKeyHandler !== null ) {
      rootKeyHandler(e);
    }
  });

  this.rootScene.root.on('onPreKeyDown', function (e) {
    //log.message(2, "PxRoot(root): keydown:" + e.keyCode);
    console.log("PxRoot: got Preview key " + e.keyCode);
    if( rootKeyHandler !== null ) {
      //rootKeyHandler(e);
    }
  });
  this.rootScene.setFocus(this.rootScene.root);


  var self = this;
// register a "global" hook that gets invoked whenever a child scene is created
  this.rootScene.onScene = function (container, innerscene, url) {
    // TODO part of an experiment to eliminate intermediate rendering of the scene
    // while it is being set up
    // container when returned here has it's painting property set to false.
    // it won't start rendering until we set painting to true which we do
    // after the script has loaded

    setTimeout(function () {
      if( container.parent === self.rootScene.root ) {
        console.log("\n\nTJC: New scene is top level")
      } else {
        console.log("\n\nTJC: New scene is second level")
      }

      self.sceneManager.createNewAppContext({
        rootScene: self.rootScene, sceneManager: self.sceneManager,
        sceneContainer: container, scene: innerscene, packageUrl: url, xreFrameworkBaseUri: self.baseUri,
        xreSysApis: XreSysApis
      });

    }, 10);
  }


  this.rootScene.on("onResize", function(e) {
    this.sceneManager.onResize(e);
  }.bind(this));

};

PxRoot.prototype.addScene = function(params) {
  if( this.rootScene == null ) {
    console.error("Root scene has not been created.  Has PxRoot been initialized?");
    return null;
  }

  if( typeof params.w === 'undefined') {
    params.w = this.rootScene.w;
  }
  if( typeof params.h === 'undefined') {
    params.h = this.rootScene.h;
  }

  params['parent'] = this.rootScene.root;

  return this.rootScene.createScene(params);
};

function setRootKeyHandler(callback) {
  rootKeyHandler = callback;
}

function getFrameworkImagePath(name) {
}

module.exports = function(x, y, width, height) {
  pxRoot = new PxRoot();
  pxRoot.initialize(x, y, width, height);
  return pxRoot;
};



