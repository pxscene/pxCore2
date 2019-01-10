/** 
 * This test is to validate changes made for XRE2-579 to no longer 
 * reject promises when animation.
 */

"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = scene.root;
var assert = imports.assert.assert;
var manual = imports.manual;

var manualTest = manual.getManualTestValue();


var basePackageUri = px.getPackageBaseFilePath();
console.log("base uri is "+basePackageUri);

    
var parent = scene.create({t:'object', parent:root, y: 50, clip:false});

var rect1 = scene.create({t:'rect', x:298, y:298, w:304, h:304,  parent:parent, lineWidth:2});



var tests = {
  
  testReload: function() 
  {
    var results = [];
    return new Promise(function(resolve, reject) {
      
        var newScene = scene.create({t:"scene", url:basePackageUri+"/simpleScene.js",x:50,y:100,w:300, h:300});
        newScene.ready.then(function() {
          console.log("promise from first load; READY and resetting url");
          newScene.url = basePackageUri+"/simpleScene.js";

          newScene.ready.then(function() {
            console.log("Promise from second load");
            results.push(assert(true, "Promise 2 resolution received"));

          }, function reject(obj) {
            results.push(assert(false, "Promise 2 rejection received"));
          }).catch(error=>{
            results.push(assert(false, "Promise 2 error received"));
            console.log("REJECTION2");
            
          }).then(function() {
            resolve(results);
          });

          console.log("DONE RESETTING URL");
          results.push(assert(true, "Promise 1 resolution received"));

        }, function reject(obj) {

          results.push(assert(false, "Promise 1 rejection received"));
          console.log("REJECTION");

        }).catch(error=>{
          results.push(assert(false, "Promise 1 error received"));
          console.log("REJECTION");
          
        });

          
      }); //end Promise
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests);

}

}).catch( function importFailed(err){
  console.error("Import for test_promiseRejectionReload.js failed: " + err)
});
