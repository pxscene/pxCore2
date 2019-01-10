"use strict";
px.import({scene:"px:scene.1.js",
           assert:"../test-run/assert.js",
           manual:"../test-run/tools_manualTests.js",
           provider1:"provider1.js"
          }).then( function ready(imports) {

  var scene = imports.scene;
  var root = imports.scene.root;
  var assert = imports.assert.assert;
  var manual = imports.manual;
  var provider1 = imports.provider1;

  var manualTest = manual.getManualTestValue();

  var basePackageUri = px.getPackageBaseFilePath();

  var provider2 =  {

    test_provider: function() {
      return "test_provider2";
    }

  }

  var provider4 = function() {

    return {
      test_provider: function() {
        return "ERROR - test page should not be provider4";
      }
    } 
  }

  var provider5 =  {

    test_provider: function() {
      return "test_provider5";
    }

  }

  scene.addServiceProvider(function(serviceName, serviceCtx) {

    if( serviceName == "provider1")
      return provider1;
    else if(serviceName == "provider2")
      return provider2;  
    else if(serviceName == "provider3")
      return "allow";
    else if(serviceName == "provider5")
      return provider5;  
    else 
      return "deny";
    
  })

  // Now, start the url that was passed in
  var child = scene.create({t:'scene',parent:root,url:basePackageUri+"/provider4.js"});

  var beforeStart = function() {
    console.log("test_serviceProvider beforeStart.....");
    var promise = new Promise(function(resolve,reject) {
      child.ready.then(function() {
        resolve(assert(true,"child scene is ready"));
      }, function() {
        resolve(assert(false,"child scene was rejected"));
      }).catch( function(error) {
        resolve(assert(false,"child scene ready got exception: "+error));
      });
      
    });
    return promise;
  }
  // beforeStart ensures that tests are not called until the child scene is ready
  var tests = {

    testImportedProvider: function() {
      console.log("Running test_serviceProvider testImportedProvider");
      return new Promise(function(resolve, reject) {

        var results = [];

        try {
          var tmpProvider = scene.getService("provider1");
          if((typeof tmpProvider) == 'object') {
            results.push(assert(tmpProvider.test_provider() == "test_provider1"), "testImportedProvider: provider1 was wrong!")
          } else {
            results.push(assert(false,"testImportedProvider: provider1 was not as expected"));
          }
          resolve(results);
        }catch(error) {
          results.push(assert(false,"testImportedProvider: Error occurred when trying to get provider1: "+error));
          resolve(results);
        }
        
      });
    },

    testLocalProvider: function() {
      console.log("Running test_serviceProvider testLocalProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = scene.getService("provider2");
          if((typeof tmpProvider) == 'object') {
            results.push(assert(tmpProvider.test_provider() == "test_provider2"), "testLocalProvider: provider2 was wrong!")
          } else {
            results.push(assert(false,"testLocalProvider: provider2 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testLocalProvider: Error occurred when trying to get provider2: "+error));
          resolve(results);
        }
        
      });
    },

    testLocalAllow: function() {
      console.log("Running test_serviceProvider testLocalAllow");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = scene.getService("provider3");
          if((typeof tmpProvider) == 'object') {
            results.push(assert(tmpProvider.test_provider() == "test_provider3"), "testLocalAllow: provider3 was wrong!")
          } else {
            results.push(assert(false,"testLocalAllow: provider3 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testLocalAllow: Error occurred when trying to get provider3: "+error));
          resolve(results);
        }
        
      });
    },

    testChildProvider: function() {
      console.log("Running test_serviceProvider testChildProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = child.api.test_childProvider();
          if((typeof tmpProvider) == 'object') {
            results.push(assert(tmpProvider.test_provider() == "test_provider4"), "testChildProvider: provider4 was wrong!")
          } else {
            results.push(assert(false,"testChildProvider: provider4 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testChildProvider: Error occurred when trying to get provider4: "+error));
          resolve(results);
        }
        
      });
    },

    testChildBubbledProvider: function() {
      console.log("Running test_serviceProvider testChildBubbledProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = child.api.test_childBubbledProvider();
          if((typeof tmpProvider) == 'object') {
            results.push(assert(tmpProvider.test_provider() == "test_provider5"), "testChildBubbledProvider: provider5 was wrong!")
          } else {
            results.push(assert(false,"testChildBubbledProvider: provider5 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testChildBubbledProvider: Error occurred when trying to get provider5: "+error));
          resolve(results);
        }
        
      });
    },

    testDeniedProvider: function() {
      console.log("Running test_serviceProvider testDeniedProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = scene.getService("provider6");
          if(tmpProvider == null) {
            results.push(assert(true, "testDeniedProvider: provider6 was wrong!"))
          } else {
            results.push(assert(false,"testDeniedProvider: provider6 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testDeniedProvider: Error occurred when trying to get provider6: "+error));
          resolve(results);
        }
        
      });
    },

    testChildDeniedProvider: function() {
      console.log("Running test_serviceProvider testChildDeniedProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = child.api.test_childDeniedProvider();
          if(tmpProvider == null) {
            results.push(assert(true, "testChildDeniedProvider: provider7 was wrong!"))
          } else {
            results.push(assert(false,"testChildDeniedProvider: provider7 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testChildDeniedProvider: Error occurred when trying to get provider7: "+error));
          resolve(results);
        }
        
      });
    },

    testChildNullProvider: function() {
      console.log("Running test_serviceProvider testChildNullProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          var tmpProvider = child.api.test_childNullProvider();
          if(tmpProvider == null) {
            results.push(assert(true, "testChildNullProvider: provider8 was wrong!")) 
          } else {
            results.push(assert(false,"testChildNullProvider: provider8 was not as expected"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testChildNullProvider: Error occurred when trying to get provider8: "+error));
          resolve(results);
        }
        
      });
    },

    testChildRemoveProvider: function() {
      console.log("Running test_serviceProvider testChildRemoveProvider");
      return new Promise(function(resolve, reject) {

        var results = [];
        try {
          // First remove the provider
          child.api.test_childRemoveProvider();
          // Now try getting a service that used to be provided
          var tmpProvider = child.api.test_childProvider();
          if(tmpProvider == null) {
            results.push(assert(true, "testChildRemoveProvider: provider was wrong!")) 
          } else {
            results.push(assert(false,"testChildRemoveProvider: removed provider was provided"));
          }
          resolve(results);
        } catch(error) {
          results.push(assert(false,"testChildRemoveProvider: Error occurred when trying to get removed provider: "+error));
          resolve(results);
        }
        
      });
    }
  }
        

        

  module.exports.tests = tests;
  module.exports.beforeStart = beforeStart;

  if(manualTest === true) {

    manual.runTestsManually(tests, beforeStart);

  }



}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for test_serviceProvider.js failed: " + err)
});