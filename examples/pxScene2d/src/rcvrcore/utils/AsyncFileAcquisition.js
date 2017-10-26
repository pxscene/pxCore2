"use strict";

var Logger = require('../Logger.js').Logger;
var SceneModuleLoader = require('../SceneModuleLoader.js');
var log = new Logger('AsyncFileAcquisition');

function AsyncFileAcquisition(scene) {
  this.scene = scene;
  this.requestMap = {};
}

AsyncFileAcquisition.prototype.acquire = function(uri) {
  var _this = this;
  return new Promise(function (resolve, reject) {
    if( _this.requestMap.hasOwnProperty(uri) ) {
      // already waiting on file
      log.message(4, "ACQUISITION: adding listener for existing request: " + uri);
      var requestData = _this.requestMap[uri];
      requestData.listeners.push(function(status, error){
        if( status === 'resolve' ) {
          resolve(requestData.moduleLoader);
        } else {
          reject(error);
        }
      });
    } else {
      var moduleLoader = new SceneModuleLoader();
      _this.requestMap[uri] = {status: "acquiring", moduleLoader: moduleLoader, listeners: []};
      var self = _this;
      log.message(4, "ACQUISITION: adding requestor for: " + uri);
      moduleLoader.loadScenePackage(_this.scene, {fileUri:uri})
        .then(function() {
          log.message(4, "---> ACQUIRED: " + uri);
          resolve(moduleLoader);
          var listeners = self.requestMap[uri].listeners;
          if( listeners !== null && listeners.length !== 0 ) {
            for(var k = 0; k < listeners.length; ++k) {
              listeners[k]('resolve');
            }
          }
          self.requestMap[uri].listeners = null;
          delete self.requestMap[uri];
        })
        .catch(function(error){
          console.error("Error");
          console.error("AsyncFileAcquisition - Error: could not load file " + uri  + ", error=" + error);
          reject(error);
          var listeners = self.requestMap[uri].listeners;
          if( listeners !== null && listeners.length !== 0 ) {
            for(var k = 0; k < listeners.length; ++k) {
              listeners[k]('reject', error);
            }
          }
          self.requestMap[uri].listeners = null;
          delete self.requestMap[uri];
        });
    }
  });
};

module.exports = AsyncFileAcquisition;