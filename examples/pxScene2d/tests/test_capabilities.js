px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js"}).then( function ready(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;
  var assert = imports.assert.assert;
  var manual = imports.manual;

  var manualTest = manual.getManualTestValue();

  var basePackageUri = px.getPackageBaseFilePath();

  var tests = {

    test_capabilities: function() {
      
      return new Promise(function(resolve, reject) {
        
        var results = [];
        if( scene.capabilities != undefined) {
          results.push(assert(true, "scene.capabilities property is accessible"));
        }
        else {
          results.push(assert(false, "scene.capabilities property is not accessible"));
        }
        
        resolve(results);
      });
    },
    test_graphics: function() {
      
      return new Promise(function(resolve, reject) {
        
        var results = [];
        if( scene.capabilities != undefined && scene.capabilities.graphics != undefined) {
          results.push(assert(true, "scene.capabilities.graphics property is accessible"));
        }
        else {
          results.push(assert(false, "scene.capabilities.graphics property is not accessible"));
        }
        
        resolve(results);
      });
    },
    test_graphics_svg: function() {
        
      return new Promise(function(resolve, reject) {
      
        var results = [];
        if( scene.capabilities != undefined && scene.capabilities.graphics != undefined && scene.capabilities.graphics.svg != undefined) {
          results.push(assert(true, "scene.capabilities.graphics.svg property is accessible"));
        }
        else {
          results.push(assert(false, "scene.capabilities.graphics.svg property is not accessible"));
        }

        // Verify version 
        if( scene.capabilities != undefined && scene.capabilities.graphics != undefined && scene.capabilities.graphics.svg >= 1) {
          results.push(assert(true, "scene.capabilities.graphics.svg property value is >= 1"));
        }
        else {
          results.push(assert(false, "scene.capabilities.graphics.svg property value is not >= 1"));
        }      
        resolve(results);
      });
   }
  }
  module.exports.tests = tests;

  if(manualTest === true) {

    manual.runTestsManually(tests);

  }

  }).catch( function importFailed(err){
    console.error("Import failed for test_capabilities.js: " + err)
  });