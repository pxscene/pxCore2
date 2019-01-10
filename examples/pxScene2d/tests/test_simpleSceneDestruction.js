"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"
          }).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var basePackageUri = px.getPackageBaseFilePath();

var scene1 = scene.create({t:'scene',parent:root,url:"http://pxscene.org/examples/px-reference/gallery/fancy.js",x:0, y:0, w:scene.w, h:scene.h});
var scene2 = scene.create({t:'scene',parent:root,url:"http://pxscene.org/examples/px-reference/gallery/picturepile.js",x:0, y:0, w:scene.w, h:scene.h});
var scene3 = scene.create({t:'scene',parent:root,url:"http://pxscene.org/examples/px-reference/gallery/gallery.js",x:0, y:0, w:scene.w, h:scene.h});
var scene4 = scene.create({t:'scene',parent:root,url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/gallery/coverflowtest_v2.js",x:0, y:0, w:scene.w, h:scene.h});

var title = scene.create({t:'text',parent:root,x:50, y:0, text:'Test Scene Destruction',pixelSize:25});

setTimeout(function() {
  console.log("Set all test scenes to null");
  scene1.remove();
  scene1 = null;

  scene2.remove();
  scene2 = null;

  scene3.remove();
  scene3 = null;

  scene4.remove();
  scene4 = null;
}, 60000);


}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for testOptimus.js failed: " + err)
});