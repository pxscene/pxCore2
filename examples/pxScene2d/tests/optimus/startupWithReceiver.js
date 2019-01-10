/**
 * This test is meant to be run on a desktop within the ServiceManagerShell.js so that it  
 * can use the service providers provided on the desktop.
 * 
 * Example url would be something like: 
 * https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/ServiceManager/ServiceManagerShell.js?url=https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/optimus/startupWithReceiver.js
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

// Keep receiverApp handy by storing in a var
var receiverApp; 

appMgr.setScene(scene);

var container = scene.create({t:"object", parent:root, w:400, h:400, clip:false});
var text2 = scene.create({t:"textBox", text:"Trying optimus.... hit 0 to focus on fonts2; hit 1 to focus on answers",  wordWrap:true, x:0, y:0, w:400, h:400, parent:container, textColor:0xFF7000FF, pixelSize:20});

// Handle the ready for application creation
appMgr.on("create",function(app) {
  console.log("Got it");
  app.setFocus(); 
})
// Handle the suspend for an app
appMgr.on("suspend",function(app) {
  console.log("Got suspend for an app");
  //app.hasApi() is undefined
  if(  app.api() != undefined && app.api().suspend != undefined ) {//app.externalApp.api != undefined &&  app.externalApp.api.suspend != undefined) {
    app.api().suspend();
  }
})
// Handle the resume for an app
appMgr.on("resume",function(app) {
  console.log("Got resume for an app");
  //app.hasApi() is undefined
  if( app.api() != undefined && app.api().resume != undefined) {
    app.api().resume();
  }
})

root.on("onKeyDown",function(key){
  console.log("Got scene onKey");
})
root.on("onPreKeyDown",function(key){
  console.log("Got scene onPreKeyDown");
  //var event = {"keyCode":key.keyCode};
  if(receiverApp.api().onPreKeyDown(key) == true) {
    key.stopPropagation(); 
  }
})
root.on("onPreChar",function(key){
  console.log("Got scene onPreChar");
  //var event = {"keyCode":key.keyCode};
  if(receiverApp.api().onPreChar(key) == true) {
    key.stopPropagation();
  }
})
root.on("onChar",function(key){
  console.log("Got scene onChar");
  //newApp.externalApp.onChar(key);
})

scene.addServiceProvider(function(serviceName, serviceCtx) {

  if( serviceName == '.optimus')
    return appMgr;
  else 
    return "ALLOW";
  
})


  console.log("root.w is "+container.w)
  // Create Receiver
  receiverApp = appMgr.createApplication({x:500,y:0,w:container.w,h:container.h,serviceContext:{id:"myLovelyReceiver",newProp:'12345'},id:"receiver",launchParams:{cmd:"spark",uri:basePackageUri+"/receiverApp.js",}} );
  //receiverApp.externalApp.serviceContext = {test:"testing",hope:"I hope this doesn't work"};
  // Create fonts2
  appMgr.createApplication({x:0,y:100,w:container.w,h:container.h,id:"fonts2",launchParams:{cmd:"spark",uri:basePackageUri+"/fonts2_optimus.js"}} );
 


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