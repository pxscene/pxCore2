"use strict";

var hasExtension = require('rcvrcore/utils/FileUtils').hasExtension;

var Logger = require('rcvrcore/Logger').Logger;
var log = new Logger('XModule');

function XModule(name, appSceneContext, basePath, jarName) {
  log.message(5, "Create new XModule for " + name);
  this.name = name;
  this.appSandbox = null;
  this.moduleReadyPromise = null;
  this.exports = {};
  this.pendingIncludes = {};
  this.moduleNameList = [];
  this.promises = [];
  this.moduleData = {};
  this.appSceneContext = appSceneContext;
  this.imports = {};
  this.log = null;
  this.basePath = basePath;
  this.jarName = jarName;
  this.importReplacementMap = {};
}

XModule.prototype.load = function(uri) {
  global.exports = self.exports;
  global.module = self;
};

XModule.prototype.freeResources = function() {
  this.name = null;
  this.appSandbox = null;
  this.moduleReadyPromise = null;
  for(var key in this.exports) {
    this.exports[key] = null;
    delete this.exports[key];
  }
  this.exports = null;
  for(var key in this.pendingIncludes) {
    this.pendingIncludes[key] = null;
    delete this.pendingIncludes[key];
  }
  this.pendingIncludes = null;
  if ((undefined != this.moduleNameList) && (null != this.moduleNameList))
  {
    var nmodules = this.moduleNameList.length;
    for (var i=0; i<nmodules; i++)
    {
      this.moduleNameList.pop();
    }
  }
  this.moduleNameList = null;
  if ((undefined != this.promises) && (null != this.promises))
  {
    var npromises = this.promises.length;
    for (var i=0; i<npromises; i++)
    {
      this.promises.pop();
    }
  }
  this.promises = null;
  this.moduleData = null;
  this.appSceneContext = null;
  this.imports = null;
  this.log = null;
  this.importReplacementMap = null;
  this.basePath = null;
  this.jarName = null;
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

XModule.prototype.include = function(filePath) {
  var rtnPromise = this.appSceneContext.include(filePath, this);
  this.pendingIncludes[filePath] = rtnPromise;
  return rtnPromise;
};

function importModule(requiredModuleSet, params) {
  return this.importModule(requiredModuleSet, params);
}

XModule.prototype.importModule = function(requiredModuleSet, params) {
  var _this = this;
  return new Promise(function(resolve, reject) {
    _this._importModule(requiredModuleSet, function readyCallback(importsArr) {
      resolve(importsArr);
    }) ,
      function failureCallback(error) {
        reject(error);
      };
  } );
};

XModule.prototype._importModule = function(requiredModuleSet, readyCallBack, failedCallback, params) {
  var isSingleStringImportType = false;

  if( readyCallBack === undefined ) {
    console.trace("WARNING: " + 'prepareModule was did not have resolutionCallback parameter: USAGE: prepareModule(requiredModules, readyCallback, [failedCallback])');
  }

  var pathToNameMap = {};
  var requiredModules = [];
  if( typeof requiredModuleSet === 'string' ) {
    requiredModules.push(requiredModuleSet);
    isSingleStringImportType = true;
  } else if( !Array.isArray(requiredModuleSet) ) {
    requiredModules = [];
    for(var key in requiredModuleSet) {
      requiredModules.push(requiredModuleSet[key]);
      pathToNameMap[requiredModuleSet[key]] = key;
    }
  } else {
    requiredModules = requiredModuleSet;
    for(var k = 0; k < requiredModuleSet.length; ++k) {
      var baseName = requiredModuleSet[k].substring(requiredModuleSet[k].lastIndexOf('/')+1);
      pathToNameMap[requiredModuleSet[k]] = baseName;
    }

  }

  if( requiredModules.length === 0 ) {
    log.message(5, "XModule:  No includes are required for " + this.name);
    if( readyCallBack !== null && readyCallBack !== undefined ) {
      readyCallBack();
    }
    this.moduleReadyPromise = null;
    return this.getInstance;
  }


  var bPath;
  if( hasExtension(this.name, '.js') ) {
    bPath = this.name.substring(0, this.name.lastIndexOf('.js'));
  } else {
    bPath = this.name;
  }


  var justAdded = false;
  if( !this.appSandbox.importTracking.hasOwnProperty(bPath) ) {
    this.appSandbox.importTracking[bPath] = [];
    justAdded = true;
  }
  for (var k = 0; k < requiredModules.length; ++k) {
    log.message(9, bPath + " REQUIRES " + requiredModules[k]);
    this.appSandbox.importTracking[bPath].push(requiredModules[k]);

    if( this.appSandbox.importTracking.hasOwnProperty(requiredModules[k]) ) {
      var reqArr = this.appSandbox.importTracking[requiredModules[k]];
      if( reqArr.length !== 0) {
        for(var j = 0; j < reqArr.length; ++j) {
          if( bPath === reqArr[j]) {
            console.trace("Found circular dependency: " + bPath + " requires " + requiredModules[k] + " which requires " + bPath);
            //process.exit(1);
          }
        }
      }
      //this.appSandbox.importTracking.bPath = [];
    }
  }

  var _this = this;

  // Create a promise that will be fulfilled when all includes/imports have been completed
  var promise = new Promise(function(moduleBuildResolve, moduleBuildReject) {
    if (requiredModules !== undefined) {
      for (var k = 0; k < requiredModules.length; ++k) {
        var ipromise = _this.include(requiredModules[k]);
        _this.moduleNameList[k] = requiredModules[k];
        _this.promises[k] = ipromise;
      }
    } else {
      console.trace("requiredModules undefined");
    }

    // Now wait for all the include/import promises to be fulfilled
    Promise.all(_this.promises).then(function (exports) {
      var exportsMap = {};
      var exportsArr = [];
      if( isSingleStringImportType ) {
        exportsMap = exports[0][0];
      } else {
        for (var k = 0; k < _this.moduleNameList.length; ++k) {
          ///_this.appSandbox[pathToNameMap[exports[k][1]]] = exports[k][0];
          //_this.moduleData[_this.moduleNameList[k]] = exports[k][0];
          //exportsArr[k] = exports[k][0];
          exportsMap[pathToNameMap[exports[k][1]]] = exports[k][0];
          //since include file is received, remove it from pendingincludes list
          _this.pendingIncludes[exports[k][1]] = null;
          delete _this.pendingIncludes[exports[k][1]];

          //console.log("TJC: " + _this.name + " gets: module[" + _this.moduleNameList[k] + "]: " + exports[k][0]);
        }
      }
      //remove the array of modules added for tracking in sandbox
      let bPath;
      if( hasExtension(_this.name, '.js') ) {
        bPath = _this.name.substring(0, _this.name.lastIndexOf('.js'));
      } else {
        bPath = _this.name;
      }
      let nmodules = _this.appSandbox.importTracking[bPath].length;
      for (let modindex=0; modindex<nmodules; modindex++)
      {
        _this.appSandbox.importTracking[bPath].pop();
      }
      delete _this.appSandbox.importTracking[bPath];

      log.message(7, "XMODULE ABOUT TO NOTIFY [" + _this.name + "] that all its imports are Ready");
      if( readyCallBack !== null && readyCallBack !== undefined ) {
        readyCallBack(exportsMap);
      }
      moduleBuildResolve();
      log.message(8, "XMODULE AFTER NOTIFY [" + _this.name + "] that all its imports are Ready");
    }).catch(function (error) {
      console.error("Error - failed to get Remote modules for: " + _this.name + ", error=" + error);
      // notify that the promise can't be kept
      if( failedCallback !== null && failedCallback !== undefined ) {
        failedCallback();
      }
      moduleBuildReject(error);
    }); // end of individual file promise
  }); // end of module promise

  this.moduleReadyPromise = promise;
  return this.getInstance;
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
    var found = path.match(regexp);
    if( path.match(regexp) ) {
      var newUri = path.replace(regexp, this.importReplacementMap[key]);
      return {fileUri:newUri, isJarFile:false};
    }
  }

  return null;
};

function getFile(filePath) {
  this.getFile(filePath);
}

XModule.prototype.getFile = function(filePath) {
  return this.appSceneContext.getModuleFile(filePath, this);
};

function resolveFilePath(filePath) {
  this.getFile(filePath);
}

XModule.prototype.resolveFilePath = function(filePath) {
  return this.appSceneContext.resolveModulePath(filePath, this);
};

module.exports = {
  importModule: importModule,
  XModule: XModule
};
