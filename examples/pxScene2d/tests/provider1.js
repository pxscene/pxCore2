"use strict";
px.import({scene:"px:scene.1.js"
          }).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;

/** Example DisplaySettings service */
var test_provider = function() {
    return "test_provider1";
}

module.exports.test_provider = test_provider;

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for provider1.js failed: " + err)
});