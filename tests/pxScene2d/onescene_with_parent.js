"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;
  var scene1;

  console.log("Creating scene1");
  scene1 = scene.create({t:"scene", parent:root, url:"test.js"});
  scene1 = null; 
  }).catch( function importFailed(err){
  console.error("Import for test_scene_create.js failed: " + err)
});
