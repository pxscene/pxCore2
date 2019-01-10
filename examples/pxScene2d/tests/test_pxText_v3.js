"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var fontUrl = "http://www.pxscene.org/examples/px-reference/fonts/DejaVuSans.ttf";

var fontResource = scene.create({t:'fontResource',url:fontUrl});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxText start.....");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {

  testFontUrlGetter: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var text = scene.create({t:"text",
                               fontUrl:fontUrl,
                               parent:root, 
                               text:"Simple Text",
                               x:40, y:80 });

      text.ready.then( function(obj) {
        results.push(assert(text.fontUrl===fontUrl,"Text fontUrl property was not as expected."));
      }, function failure() {
        results.push(assert(false,"Text got rejected promise, so test could not proceed."));
      }).then(function(error) {
        text = null;
        resolve(results);
      });
    });
  },
  testFontUrlGetter2: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var text2 = scene.create({t:"text",
                               fontUrl:fontUrl,
                               parent:root, 
                               text:"Simple Text",
                               x:40, y:120 });

      text2.ready.then( function(obj) {
        results.push(assert(text2.fontUrl!==fontUrl+"blah","Text fontUrl property was not as expected."));
      }, function failure() {
        results.push(assert(false,"Text got rejected promise, so test could not proceed."));
      }).then(function(error) {
        text2 = null;
        resolve(results);
      });
    });
  }, 
  testFontUrlGetter3: function() {
    return new Promise(function(resolve, reject) {
      fontResource.ready.then(function() { // Make sure fontResource is ready
        var results = [];
        var text3 = scene.create({t:"text",
                                font:fontResource,
                                parent:root, 
                                text:"Simple Text",
                                x:40, y:160 });

        text3.ready.then( function(obj) {
          results.push(assert(text3.fontUrl===fontUrl,"Text fontUrl property was not as expected when using fontResource."));
        }, function failure() {
          results.push(assert(false,"Text got rejected promise, so test could not proceed."));
        }).then(function(error) {
          text3 = null;
          resolve(results);
        });
      });
    });
  },
  testFontUrlGetter4: function() {
    return new Promise(function(resolve, reject) {
      fontResource.ready.then(function() { // Make sure fontResource is ready
        var results = [];
        var text4 = scene.create({t:"text",
                                font:fontResource,
                                parent:root, 
                                text:"Simple Text",
                                x:40, y:200 });

        text4.ready.then( function(obj) {
          results.push(assert(text4.fontUrl!==fontUrl+"blah","Text fontUrl property was not as expected when using fontResource."));
        }, function failure() {
          results.push(assert(false,"Text got rejected promise, so test could not proceed."));
        }).then(function(error) {
          text4 = null;
          resolve(results);
        });
      });
    });
  },

  testTextColor: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var text5 = scene.create({t:"text",
                              font:fontResource,
                              parent:root, 
                              text:"Simple Text",
                              x:40, y:240, 
                              textColor:0xDDCCBBFF});

      text5.ready.then( function(obj) {
        results.push(assert(text5.textColor==0xDDCCBBFF,"Text textColor property was not as expected when using fontResource."));
      }, function failure() {
        results.push(assert(false,"Text got rejected promise, so test could not proceed."));
      }).then(function(error) {
        text5 = null;
        resolve(results);
      });
    });
  },

  testTextColorSetGet: function() {
    return new Promise(function(resolve, reject) {
      var results = [];
      var text6 = scene.create({t:"text",
                              font:fontResource,
                              parent:root, 
                              text:"Simple Text",
                              x:40, y:240, 
                              textColor:0xDDCCBBFF});

      text6.ready.then( function(obj) {
        results.push(assert(text6.textColor==0xDDCCBBFF,"Text textColor property was not as expected when using fontResource."));

        text6.textColor=0x884422FF;
        results.push(assert(text6.textColor==0x884422FF,"Updated Text textColor property was not as expected when using fontResource."));
      }, function failure() {
        results.push(assert(false,"Text got rejected promise, so test could not proceed."));
      }).then(function(error) {
        text6 = null;
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
  console.error("Import for test_pxText_v2.js failed: " + err)
});
