"use strict";
px.import({scene:"px:scene.1.js",assert:"../test-run/assert.js"}).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;
var assert = imports.assert.assert;

root.w = 800;
root.h = 300;

var basePackageUri = px.getPackageBaseFilePath();


module.exports.beforeStart = function() {
  console.log("test_pxWayland beforeStart()!");
  var promise = new Promise(function(resolve,reject) {
    resolve("beforeStart");
  });
  return promise;
}

var tests = {
  test1: function() {
    return new Promise(function(resolve, reject) {
      var wl = scene.create({t:"wayland",parent:root,w:100,h:200});
      wl.ready.then(function()  {
        var results = [];
        results.push(assert(null !== wl,"wayland component not created"));
        resolve(results);
  });
  });
  }
}

module.exports.tests = tests;

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_pxWayland.js failed: " + err)
});
