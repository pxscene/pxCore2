/**
 * This test is meant to be run on a desktop within the ServiceManagerShell.js so that it  
 * can use the service providers provided on the desktop.
 * 
 * Example url would be something like: 
 * https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/ServiceManager/ServiceManagerShell.js?url=https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/optimus/startup.js
 */

"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../../test-run/assert.js",
           manual:"../../test-run/tools_manualTests.js",
           optimus:"optimus.js"
          }).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;
var appMgr = imports.optimus;

var basePackageUri = px.getPackageBaseFilePath();

appMgr.setScene(scene);

var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});
var text2 = scene.create({t:"textBox", text:"Trying optimus....",  x:0, y:0, w:400, h:400, parent:container, textColor:0xFF7000FF, pixelSize:20});

appMgr.on("create",function(app) {
  console.log("Got it");
  console.log("app w is "+app.externalApp.w)
  app.externalApp.w = 400;
  app.setFocus();
})
appMgr.on("onKeyDown", function(key) {
  console.log("Got onKey");
});
appMgr.on("keyDown", function(key) {
  console.log("Got key");
});

scene.root.on("onKeyDown",function(key){
  console.log("Got scene onKey");
})
//text2.ready.then(function(text2) { 
  console.log("root.w is "+container.w)
  var newApp = appMgr.createApplication({x:0,y:40,w:container.w,h:container.h,launchParams:{cmd:"spark",uri:"http://www.pxscene.org/examples/px-reference/gallery/gallery3.js"}} );

  newApp.on("onKeyDown",function(key){
    console.log("Got newApp onKey");
  })

  var displaySettings = scene.getService("org.openrdk.DisplaySettings");
  if(displaySettings != undefined && displaySettings != null) {
    displaySettings.setApiVersionNumber(5);
  }
  
  var loggingPreferences = scene.getService("org.openrdk.LoggingPreferences");
  if(loggingPreferences != undefined && loggingPreferences != null) {
    var masking = loggingPreferences.isKeystrokeMaskEnabled();
    console.log("KeystrokeMasking is "+masking);
  }   

//});
}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for startup.js failed: " + err)
});