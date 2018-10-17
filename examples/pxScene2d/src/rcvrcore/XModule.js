/*

pxCore Copyright 2005-2018 John Robinson

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/

"use strict";

var hasExtension = require('rcvrcore/utils/FileUtils').hasExtension;
var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('XModule');

function XModule(name, appSceneContext, basePath, jarName) {
  log.message(5, "Create new XModule for " + name);
  this.name = name;
  this.appSandbox = null;
  this.moduleReadyPromise = null;
  this.exports = {}; // module.exports is an object by default
  this.appSceneContext = appSceneContext;
  this.log = null;
  this.basePath = basePath;
  this.jarName = jarName;
  this.importReplacementMap = {};
}

XModule.prototype.freeResources = function() {
  this.name = null;
  this.appSandbox = null;
  this.moduleReadyPromise = null;
  this.exports = null;
  this.appSceneContext = null;
  this.log = null;
  this.basePath = null;
  this.jarName = null;
  this.importReplacementMap = null;
};

XModule.prototype.getBasePath = function() {
  return this.basePath;
};

XModule.prototype.getJarName = function() {
  return this.jarName;
};

XModule.prototype.initSandbox = function(otherSandbox) {
  this.appSandbox = otherSandbox;
  this.log = new Logger(this.name);
};

XModule.prototype.importModule = function(requiredModuleSet) {
  var _this = this;

  return new Promise(function(readyCallBack, failedCallback) {
    // parse input
    var isSingleStringImportType = false;
    var pathToNameMap = {};
    var requiredModules = [];
    var k, key;
    if( typeof requiredModuleSet === 'string' ) {
      requiredModules.push(requiredModuleSet);
      isSingleStringImportType = true;
    } else if( !Array.isArray(requiredModuleSet) ) {
      for(key in requiredModuleSet) {
        if (requiredModuleSet.hasOwnProperty(key)) {
          var modulePath = requiredModuleSet[key];
          requiredModules.push(modulePath);
          pathToNameMap[modulePath] = key;
        }
      }
    } else {
      for(k = 0; k < requiredModuleSet.length; ++k) {
        key = requiredModuleSet[k];
        requiredModules.push(key);
        pathToNameMap[key] = key.substring(key.lastIndexOf('/')+1);
      }
    }
    // if empty set return
    if( requiredModules.length === 0 ) {
      log.message(5, "XModule:  No includes are required for " + _this.name);
      readyCallBack();
      _this.moduleReadyPromise = null;
      return;
    }
    // get this module's path
    var bPath;
    if( hasExtension(_this.name, '.js') ) {
      bPath = _this.name.substring(0, _this.name.lastIndexOf('.js'));
    } else {
      bPath = _this.name;
    }
    // add for tracking
    if (_this.appSandbox && _this.appSandbox.importTracking) {
      if( !_this.appSandbox.importTracking.hasOwnProperty(bPath) ) {
        _this.appSandbox.importTracking[bPath] = [];
      }
      for (k = 0; k < requiredModules.length; ++k) {
        log.message(9, bPath + " REQUIRES " + requiredModules[k]);
        _this.appSandbox.importTracking[bPath].push(requiredModules[k]);
      }
    }

    _this.moduleReadyPromise = new Promise(function(moduleBuildResolve, moduleBuildReject) {
      // get a promise for each import
      var moduleNameList = [];
      var promises = [];
      var k, key;
      for (k = 0; k < requiredModules.length; ++k) {
        key = requiredModules[k];
        var promise = _this.appSceneContext.include(key, _this);
        moduleNameList[k] = key;
        promises[k] = promise;
      }
      // Now wait for all the include/import promises to be fulfilled
      Promise.all(promises).then(function (exports) {
        var exportsMap = {};
        if( isSingleStringImportType ) {
          key = exports[0][1];
          log.message(9, bPath + " GOT " + key);
          exportsMap = exports[0][0];
        } else {
          for (k = 0; k < moduleNameList.length; ++k) {
            key = pathToNameMap[exports[k][1]];
            log.message(9, bPath + " GOT " + key);
            exportsMap[key] = exports[k][0];
          }
        }
        // remove the array of modules added for tracking in sandbox
        delete _this.appSandbox.importTracking[bPath];
        log.message(7, "XMODULE ABOUT TO NOTIFY [" + _this.name + "] that all its imports are Ready");
        readyCallBack(exportsMap);
        moduleBuildResolve();
        log.message(8, "XMODULE AFTER NOTIFY [" + _this.name + "] that all its imports are Ready");
      }).catch(function (error) {
        log.error("Error - failed to get Remote modules for: " + _this.name + ", error=" + error);
        // notify that the promise can't be kept
        failedCallback();
        moduleBuildReject(error);
      });
    });

  });
};

XModule.prototype.configImport = function(replacementMap) {
  this.importReplacementMap = replacementMap;
};

XModule.prototype.findImportReplacementMatch = function(path) {
  if( this.importReplacementMap === null ) {
    return null;
  }
  // look for direct matches or partial matches at beginning of uri
  for(var key in this.importReplacementMap) {
    var regexp = new RegExp('^' + key);
    if( path.match(regexp) ) {
      var newUri = path.replace(regexp, this.importReplacementMap[key]);
      return {fileUri:newUri, isJarFile:false};
    }
  }
  return null;
};

XModule.prototype.getFile = function(filePath) {
  return this.appSceneContext.getModuleFile(filePath, this);
};

XModule.prototype.resolveFilePath = function(filePath) {
  return this.appSceneContext.resolveModulePath(filePath, this);
};

module.exports = XModule;
