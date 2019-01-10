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


module.exports.beforeStart = function() {
  console.log("test_pxResource beforeStart()!");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {
  test1: function() {
  return new Promise(function(resolve, reject) {
    var imageRes = scene.create({t:"imageResource",parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg"});
    var results = []; 
    var tmpTimeout = setTimeout(function(){
      resolve(assert(false,">>>>> TIMEOUT ON test imageResource: waiting for promise"));
    }, 300000);
    imageRes.ready.then(function()  {
      clearTimeout(tmpTimeout);
      console.log("test1: imageRes ready");
    
        // check value 
        var loadStatus = imageRes.loadStatus;
        results.push(assert(loadStatus["statusCode"]==0,"status code is not correct"));
        results.push(assert(loadStatus["sourceType"]=="http","load type is not correct"));
        results.push(assert(imageRes.w != 0 ,"image width is 0"));
        results.push(assert(imageRes.h != 0,"image height is 0"));
        resolve(results);
      }, function(o){
        clearTimeout(tmpTimeout);
        console.log("test1: imageRes rejection");
        var loadStatus = imageRes.loadStatus;
        results.push(assert(loadStatus["statusCode"]==0,"status code is not correct; code is "+loadStatus["statusCode"]));
        results.push(assert(false,"image promise rejection was unexpected!"));
        reject(results);
      });
    });
  },

  test2: function() {
    return new Promise(function(resolve, reject) {
      var imageA = scene.create({t:"imageA",parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg"});
      var results = [];
      var tmpTimeout = setTimeout(function(){
        resolve(assert(false,">>>>> TIMEOUT ON test2 imageA: waiting for promise"));
      }, 300000);
      imageA.ready.then(function()  {
        clearTimeout(tmpTimeout);
        console.log("test2: imageA ready");
        var res = imageA.resource;
        var loadStatus = res.loadStatus;
        // check value 
        results.push(assert(imageA.url=="https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg","url is not correct"));
        results.push(assert(loadStatus["statusCode"]==0,"status code is not correct"));
        results.push(assert(loadStatus["sourceType"]=="http","load type is not correct"));
        resolve(results);
      }, function(o){
        clearTimeout(tmpTimeout);
        console.log("test2: imageA rejection");
        var res = imageA.resource;
        var loadStatus = res.loadStatus;
        results.push(assert(loadStatus["statusCode"]==0,"status code is not correct; code is "+loadStatus["statusCode"]));
        results.push(assert(false,"image promise rejection was unexpected!"));
        reject(results);
      });
    });
  },

  test3: function() {
    return new Promise(function(resolve, reject) {
      var imageA = scene.create({t:"imageA",parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg"});
      var results = [];
      var tmpTimeout = setTimeout(function(){
        resolve(assert(false,">>>>> TIMEOUT ON test3 imageA: waiting for promise"));
      }, 300000);
      imageA.ready.then(function()  {
        clearTimeout(tmpTimeout);
        console.log("test3: imageA ready");
        var res = imageA.resource;
        var loadStatus = res.loadStatus;
        // check value 
        results.push(assert(imageA.url=="https://px-apps.sys.comcast.net/pxscene-samples/images/dolphin.jpg","url is not correct"));
        resolve(results);
      }, function(o){
        clearTimeout(tmpTimeout);
        console.log("test3: imageA rejection");
        var res = imageA.resource;
        var loadStatus = res.loadStatus;
        results.push(assert(loadStatus["statusCode"]==0,"status code is not correct; code is "+loadStatus["statusCode"]));
        results.push(assert(false,"imageA promise rejection was unexpected!"));
        reject(results);
      });
    });
  }
}
module.exports.tests = tests;

if(manualTest === true) {

  manual.runTestsManually(tests, module.exports.beforeStart);

}

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_pxResource.js failed: " + err)
});
