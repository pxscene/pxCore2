"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

// Use a bogus url to cause promise rejection
var image = scene.create({t:"image",parent:root, x:0,y:100, url:"https://px-apps.sys.comcast.net/pxscene-samples/notthere.png"});

var image2 = scene.create({t:"image",parent:root, x:0,y:100, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/images/ball.png"});

var tests = {
  testBadImageUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image.ready.then(function(o) {
        results.push(assert(false, "Promise rejection expected, but resolution was received"));
      }, function rejection(o) {
        console.log("image rejection received");
        results.push(assert(true, "Promise rejection received"));
      }).then( function(obj) {
        resolve(results);
      });
    });
  },

  testGoodImageUrl: function() {

    return new Promise(function(resolve, reject) {
      var results = []; 
      image2.ready.then(function(o) {
        results.push(assert(true, "image2 promise resolution received"));
      }, function rejection(o) {
        console.log("image2 rejection received");
        results.push(assert(false, "image2 promise resolution expected, but rejection was received"));
      }).then( function(obj) {
        resolve(results);
      });
    });
  }

}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import failed for test_promiseRejectionImage.js: " + err)
});
