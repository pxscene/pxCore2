"use strict";
px.import({scene:"px:scene.1.js"
          }).then( function ready(imports) {

var scene = imports.scene;
var root = imports.scene.root;

/** Example DisplaySettings service */
var provider4 = function() {

  return {
    test_provider: function() {
      return "test_provider4";
    }
  } 
}
function myServiceProvider(serviceName, serviceCtx) {

  if( serviceName == "provider4")
    return provider4();
  else if(serviceName == "provider5")
    return "allow"; 
  else if(serviceName == "provider8")
    return null; 
          
  else 
    return "deny";
  
}

// Add the service provider
scene.addServiceProvider(myServiceProvider);

module.exports.test_childProvider = function() {

  return scene.getService("provider4");
};
module.exports.test_childBubbledProvider = function() {

  return scene.getService("provider5");
};
module.exports.test_childDeniedProvider = function() {

  return scene.getService("provider7");
};
module.exports.test_childNullProvider = function() {

  return scene.getService("provider8");
};
module.exports.test_childRemoveProvider = function() {

  return scene.removeServiceProvider(myServiceProvider);
};

}).catch( function importFailed(err){
  console.log("err: "+err);
  console.error("Import for provider4.js failed: " + err)
});