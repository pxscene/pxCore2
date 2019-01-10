"use strict";
// This test will use various uri schemes to load imported files and modules
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           shots:"../test-run/tools_screenshot.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var shots = imports.shots;
var manual = imports.manual;

var doScreenshot = false;
var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();

var newSceneChild = scene.create({t:'scene',parent:root,url:basePackageUri+"/test_httpsImport_child.js"});

var newSceneChild2 = scene.create({t:'scene',parent:root,url:basePackageUri+"/test_httpsImport_child2.js"});



var tests = {

  validateLoad: function() {
    var results = [];
    return new Promise( function(resolve, reject) {
      newSceneChild.ready.then( function success(obj) {
        results.push(assert(true, "newSceneChild page loaded successfully"));
        newSceneChild.api.validateImports().then( function(childResults) {
          results.push(childResults);
        });
      },
      function failure(obj) {
        results.push(assert(false, "newSceneChild page load failed"));

      }).catch(function(err) {
        results.push(assert(false, "newSceneChild page load failed with exception"));
      }).then(function () {
        resolve(results);
      });
    });
  },

  validateLoad2: function() {
    var results = [];
    return new Promise( function(resolve, reject) {
      newSceneChild2.ready.then( function success(obj) {
        results.push(assert(true, "newSceneChild2 page loaded successfully"));
        newSceneChild2.api.validateImports().then( function(childResults) {
          results.push(childResults);
        })
      },
      function failure(obj) {
        results.push(assert(false, "newSceneChild2 page load failed"));

      }).catch(function(err) {
        results.push(assert(false, "newSceneChild2 page load failed with exception"));
      }).then(function () {
        resolve(results);
      });
    });
  }, 
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_httpsImport.js failed: " + err)
});
