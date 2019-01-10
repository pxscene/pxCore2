"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();

var basePackageUri = px.getPackageBaseFilePath();

var urls = [
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/elephant.png", // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/cube.png",     // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/apng/spinfox.png",  // apng
  "http://www.pxscene.org/examples/px-reference/gallery/images/star.png",          // supports plain old pngs
  "http://www.pxscene.org/examples/px-reference/gallery/images/ajpeg.jpg",         // and single frame jpegs too!!
];

var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_pxImageA_2 beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testImageAResource: function() {
   console.log("Running testImageAResource");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageAResource = scene.create({t:"imageAResource", url:urls[0]});
      var imageA = scene.create({t:"imageA",resource:imageAResource,parent:container});

      Promise.all([imageAResource.ready,imageA.ready]).then(function(objs) {
        results.push(assert(objs[0].url == urls[0], "imageA url is not correct when created with resource"));
        results.push(assert(objs[1].url == urls[0], "imageAResource url is not correct: "+objs[1].url));
        console.log("imageA url is "+imageA.url);
        console.log("imageAResource url is "+imageAResource.url);

      }, function rejection() {
        results.push(assert(false, "imageAResource failed : "+exception));      
      }).then(function() {
        container.removeAll();
        resolve(results);
      });

    });
  } ,
  
 // Trying to load an imageAResource should fail for pxImage
 testImageResourceFailure: function() {
   console.log("Running testImageAResource");
    return new Promise(function(resolve, reject) {

      var results = [];

      var imageResource = scene.create({t:"imageResource", url:urls[0]});
      var imageA = scene.create({t:"imageA",resource:imageResource,parent:container});

      Promise.all([imageResource.ready,imageA.ready]).then(function(objs) {
        results.push(assert(objs[0].url == urls[0], "imageA url is not correct when created with resource"));
        results.push(assert(objs[1].url == urls[0], "imageResource url is not correct"));

      }, function rejection(obj) {
        results.push(assert(true, "imageResource was correctly rejected for imageA : "));      
      }).then(function() {
        container.removeAll();
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
  console.error("Import for test_pxImageA_2.js failed: " + err)
});
