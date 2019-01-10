"use strict";
/** This test is for XRE2-597 - test reassigning url to image that had rejected promise, then url="", then url= valid url */

px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var badImageUrl = "https://px-apps.sys.comcast.net/pxscene-samples/notthere.png";
var goodImageUrl = "https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png";
// Use a bogus url to cause promise rejection
var image = scene.create({t:"imageA",parent:root, x:0,y:100, url:badImageUrl});

var beforeStart = function() {
  // Ensure rejected promise on first, invalid url 
  // before beginning the rest of the test
  console.log("test_promiseCreationImageA start.....");

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.ready.then(function(o) {
        results.push(assert(false, "Promise rejection expected, but resolution was received"));
      }, function rejection(o) {
        console.log("image rejection received");
        results.push(assert(true, "Promise rejection received"));
      }).then( function(obj) {
        resolve(results);
      },function(obj) { reject(results);} );
    });
 
}

var tests = {

  // Test that no promise is resolved/rejected when setting url to ""
  testSetEmptyUrl: function() {

    return new Promise(function(resolve, reject) {
      var promiseThen = false;
      var results = []; 
      image.url = "";
      // No promise should be resolved or rejected
      image.ready.then(function(o) {
        promiseThen = true;
      }, function rejection(o) {
        promiseThen = true;
        console.log("testSetEmptyUrl: image rejection received");
      }).then( function(obj) {
        promiseThen = true;
      });

      setTimeout(function() {
        if( promiseThen === true) {
          results.push(assert(false, "testSetEmptyUrl: No promise resolution or rejection was expected but was received"));
        } else {
          results.push(assert(true, "testSetEmptyUrl: No promise resolution or rejection was received"));
        }
        resolve(results);
      }, 1500);
    });
  },

  // Test that promise is received when resetting url to valid value
  testResetToGoodImageUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.url = goodImageUrl;
      image.ready.then(function(o) {
        results.push(assert(true, "testResetToGoodImageUrl: imageA promise resolution received"));
      }, function rejection(o) {
        console.log("testResetToGoodImageUrl: imageA rejection received");
        results.push(assert(false, "testResetToGoodImageUrl: imageA promise resolution expected, but rejection was received"));
      }).then( function(obj) {
        resolve(results);
      },function(obj) { reject(results);} );
    });
  },

  testResetToBadImageUrl: function() {
    return new Promise(function(resolve, reject) {
      var results = []; 
      image.url = badImageUrl;
      image.ready.then(function(o) {
        results.push(assert(false, "Promise rejection expected, but resolution was received"));
      }, function rejection(o) {
        console.log("image rejection received");
        results.push(assert(true, "Promise rejection received"));
      }).then( function(obj) {
        resolve(results);
      },function(obj) { reject(results);} );
    });
  }
}
module.exports.beforeStart = beforeStart;
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests, beforeStart);

}

}).catch( function importFailed(err){
  console.error("Import failed for test_promiseCreationImageA.js: " + err)
});
