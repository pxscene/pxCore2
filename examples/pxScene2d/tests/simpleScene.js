
"use strict";
px.import({scene:"px:scene.1.js",assert:"../test-run/assert.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;

var text = scene.create({t:'text', parent:root, clip:false, text:"Just a simple test page."});



}).catch( function importFailed(err){
  console.error("Import for simpleScene.js failed: " + err)
});
