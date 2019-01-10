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
  "http://www.pxscene.org/examples/px-reference/gallery/images/gold_star.png", 
  "http://www.pxscene.org/examples/px-reference/gallery/images/banana.png",     
  "http://www.pxscene.org/examples/px-reference/gallery/images/grapes.png",  
  "http://www.pxscene.org/examples/px-reference/gallery/images/star.png",          
  "http://www.pxscene.org/examples/px-reference/gallery/images/flower1.jpg",         
];

var container = scene.create({t:'object',parent:root});

var beforeStart = function() {
  // Nothing to do here...
  console.log("test_textureMemoryUsage beforeStart.....");
  var promise = new Promise(function(resolve,reject) {
    resolve(assert(true,"beforeStart succeeded"));
  });
  return promise;
}

var tests = {

  testTextureMemoryUsage: function() {
    console.log("Running testTextureMemoryUsage");
    return new Promise(function(resolve, reject) {

      var promises = [];
      var results = [];
      
      for( var i = 0; i < urls.length; i++) {
        promises.push( scene.create({t:"image",url:urls[i],parent:container}).ready);
      }

 
      Promise.all(promises).then(function(objs) {
        if (scene.capabilities != undefined && scene.capabilities.metrics != undefined && scene.capabilities.metrics.textureMemory 
          && scene.textureMemoryUsage() > 0) {
           results.push(assert(true, "texture memory usage success"));
        } else {
          results.push(assert(false, "texture memory usage failed"));
        }
      }, function(obj) {//rejected
        results.push(assert(false, "image load failed : "+obj));
      }).catch(function(obj) {
          results.push(assert(false, "image load failed : "+obj));       
      }).then(function() {
        container.removeAll();
        resolve(results);
        console.log("Resolving testTextureMemoryUsage");
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
  console.error("Import for test_textureMemoryUsage.js failed: " + err)
});
