/**
 * Created by tcarro004
 * on 7/19/15.
 */
"use strict";

var AppSceneContext = require('rcvrcore/AppSceneContext');

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('XModule');

function SceneManager(pxRoot) {
  this.pxRoot = pxRoot;
  this.appSceneContextList = {};
  this.sceneMap = {};
  this.sceneStack = [];
  this.currentScene = null;
}

SceneManager.prototype.createNewAppContext = function(params) {
  var appSceneContext = new AppSceneContext(params);

  if( params.sceneContainer.parent === params.rootScene.root ) {
    // It's a top level app
    this.appSceneContextList[params.packageUrl] = appSceneContext;

    if( this.sceneMap.hasOwnProperty(params.packageUrl) ) {
      log.message(2, "  push app Scene context");
      console.log("sceneMap already has: " + params.packageUrl);
      console.log("isSceneEqual? " + (this.sceneMap[params.packageUrl] === params.scene) );
      console.log("isSceneContainerEqual? " + (this.sceneMap[params.packageUrl] === params.sceneContainer) );
      console.log("isRootEqual? " + (this.sceneMap[params.packageUrl] === this.pxRoot) );
      //this.sceneMap[params.packageUrl] = params.scene;
    }

    appSceneContext.loadScene();
    params.rootScene.setFocus(params.sceneContainer);
    log.message(2, "  push app Scene context");
    this.sceneStack.push(appSceneContext);
  } else {
    log.message(2, "  just load the scene");
    appSceneContext.loadScene();
  }
  //console.log("After push: " + appSceneContext.packageUrl);
}

SceneManager.prototype.onResize = function(resizeEvent) {
  for(var k = 0; k < this.sceneStack.length; ++k) {
    this.sceneStack[k].onResize(resizeEvent);
  }
}

SceneManager.prototype.launchScene = function(params) {
  //scene.createScene({parent:this.sceneView, url:"./apps/examples/dom.js",w:480,h:540});
  var scene = this.pxRoot.addScene(params);
  //this.sceneMap[params.name] = scene;
  this.sceneMap[params.url] = scene;
  this.currentScene = scene;
  return scene;
}

SceneManager.prototype.destroyCurrentScene = function() {
  this.destroyScene(this.currentScene);
}


SceneManager.prototype.destroySceneByName = function(name) {
  var scene = this.sceneMap[name];
  this.destroyScene(scene);
}

SceneManager.prototype.destroyScene = function(scene) {
  if( typeof scene == 'undefined' || scene == null ) {
    return;
  }
  if (scene) {
    this.destroyNodeFromScene(scene, scene.root);
    scene.visible = false;
    scene.remove();
  }

  this.sceneStack.pop();
  var appSceneContext = this.sceneStack[this.sceneStack.length-1];

  appSceneContext.setFocus();
}

SceneManager.prototype.destroyNodeFromScene = function(scene, node) {
  if (node) {
    var children = node.children;
    if (children) {
      var n = children.length;
      for (var i = 0; i < n; ++i) {
        this.destroyNodeFromScene(scene, node.getChild(0));
      }
    }
    node.remove();
  }
}

SceneManager.prototype.destroySceneByRef = function(scene) {
  if (scene) {
    this.destroyNodeFromScene(scene, scene.root);
    scene.url = "";
    scene.parent = null;
  }
}


module.exports = SceneManager;

