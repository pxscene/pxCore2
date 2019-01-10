"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var goodUrl = "http://pxscene.org/examples/px-reference/fonts/DejaVuSans.ttf";
var badUrl = "http://pxscene.org/examples/px-reference/fonts/NotThere.ttf";


var beforeStart = function() {
  // Nothing to do here...
  console.log("test_fontFailure start.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {

  testLoadFailure: function() {
  
    return new Promise(function(resolve, reject) {

      var results = [];
      var textBox = scene.create({t:"textBox",fontUrl:badUrl,parent:root});

      textBox.ready.then( function(obj) {
        results.push(assert(false,"TextBox got resolved promise, but it should have failed due to bad font url."));
      }, function failure() {
        // Failure is expected here, so this result is a pass
        results.push(assert(true,"TextBox got rejected promise as expected."));
      }).then(function(error) {
        resolve(results);
         });
      
    });
  },

  testLoadSuccess: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var textBox = scene.create({t:"textBox",fontUrl:goodUrl,parent:root});

      textBox.ready.then( function(obj) {
        results.push(assert(true,"TextBox got resolved promise as expected."));
      }, function failure() {
        results.push(assert(false,"TextBox got rejected promise but expected success."));
      }).then(function(error) {
        resolve(results);
      });
    });
  }


}
module.exports.tests = tests;
module.exports.beforeStart = beforeStart;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import for test_fontFailure.js failed: " + err)
});
