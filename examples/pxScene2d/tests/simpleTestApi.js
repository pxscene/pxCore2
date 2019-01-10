"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

root.w = 800;
root.h = 300;

var basePackageUri = px.getPackageBaseFilePath();

var onTextChange = function(value) { 
  console.log("New value for child text is ",value);
}

var newSceneChild = scene.create({t:'scene',parent:root,url:basePackageUri+"/simpleTestApiChild.js"});
var theChildApi; 

module.exports.beforeStart = function() {
  console.log("simpleTestApi beforeStart()!");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
  
}

var tests = { 

  test1: function() {
    return new Promise(function(resolve, reject) {
      newSceneChild.ready.then(function(obj)  {
        theChildApi = obj.api;
        console.log("Test1 simpleTestApi!");
        // check value 
        resolve(assert(theChildApi.getText()==="My text value","wrong text on screen after init"));
      });
    });

  },

  test2: function() {
    return new Promise(function(resolve, reject) {
      newSceneChild.ready.then(function()  {

        console.log("Test2 simpleTestApi!");
        // Change value 
        theChildApi.myAPI("Something new on screen!");
        // assert
        resolve(assert(theChildApi.getText() ==="Something new on screen!", "wrong text on screen after set"));

      });
    });
  }
}
module.exports.tests = tests;

// When child scene is ready, get api and register for event
/*newSceneChild.ready.then(function(child)  {



  console.log("newSceneChild is ready ....... "+child);
  console.log("child is api ....... "+child.api);
  theChildApi = child.api;
  console.log("newSceneChild is api ....... "+newSceneChild.api);

  // Register listener for text value change in child
  child.api.on("onTextChange",onTextChange);
  
  // Invoke value change to test event handler
  tests.test2();

});
*/
if(manualTest === true) {

  manual.runTestsManually(tests, module.exports.beforeStart);

}

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for simpleTestApi.js failed: " + err)
});
