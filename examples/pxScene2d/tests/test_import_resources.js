"use strict";

px.import({scene:"px:scene.1.js",importFile:'import/importFile.js',assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;
  var importfile = imports.importFile;
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var results = [];
  var assert = imports.assert.assert;
  var manualTest = manual.getManualTestValue();


  module.exports.beforeStart = function() {
    console.log("test_import_resources beforeStart()!");
    var promise = new Promise(function(resolve,reject) {
      resolve("beforeStart");
    });
    return promise;
  }

  var tests = {
    test1: function() {
    return new Promise(function(resolve, reject) {
        if (undefined != importfile.image)
        {
          importfile.image.ready.then(function() { results.push(assert(true, "resource test from import file Success !!")); resolve(results); },
                                    function(o) { results.push(assert(false, "resource test from import file Failed !!")); reject(results); });
        }
        else
        {
          results.push(assert(false, "resource test from import file failed due to api error !!")); 
          reject(results);
        }
      });
    }
  }
  module.exports.tests = tests;
  
  if(manualTest === true) {
  
    manual.runTestsManually(tests, module.exports.beforeStart);
  
  }

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_import_resources.js failed: " + err)
});
