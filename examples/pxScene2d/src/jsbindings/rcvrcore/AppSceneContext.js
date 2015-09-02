//"use strict";

var url = require('url');
var vm = require('vm');
var Logger = require('rcvrcore/Logger').Logger;
var SceneModuleLoader = require('rcvrcore/SceneModuleLoader');
var hasExtension = require('rcvrcore/utils/FileUtils').hasExtension;
var XModule = require('rcvrcore/XModule').XModule;
var xmodImportModule = require('rcvrcore/XModule').importModule;
var loadFile = require('rcvrcore/utils/FileUtils').loadFile;
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');


var log = new Logger('AppSceneContext');


function AppSceneContext(params) { //rootScene, container, innerscene, packageUrl) {
  this.rootScene = params.rootScene;
  this.xresys = params.xreSysApis;
  this.container = params.sceneContainer;
  this.innerscene = params.scene;
  if( params.packageUrl.indexOf('?') != -1 ) {
    var urlParts = url.parse(params.packageUrl, true);
    this.queryParams = urlParts.query;
    this.packageUrl = urlParts.pathname;
  } else {
    this.queryParams = {};
    this.packageUrl = params.packageUrl;
  }
  this.defaultBaseUri = "";
  this.basePackageUri = "";
  this.fileArchive = null;
  this.packageManifest = null;
  this.sandbox = {};
  this.scriptMap = {};
  this.xmoduleMap = {};
  this.asyncFileAcquisition = new AsyncFileAcquisition();
  this.publicClasses = {};
  this.importProtocolPaths = {};
  this.importRedirects = {};
  this.lastHrTime = process.hrtime();
  this.resizeTimer = null;
  log.message(4, "[[[NEW AppSceneContext]]]: " + this.packageUrl);
}

AppSceneContext.prototype.loadScene = function() {
  log.info("loadScene() - begins");
  var urlParts = url.parse(this.packageUrl, true);
  var fullPath = this.packageUrl;
  if (fullPath.substring(0, 4) !== "http") {
    if( fullPath.charAt(0) === '.' ) {
      // local file system
      this.defaultBaseUri = ".";
    }
    fullPath = urlParts.pathname;
    this.basePackageUri = fullPath.substring(0, fullPath.lastIndexOf('/'));
    var fileName = this.packageUrl.substring(fullPath.lastIndexOf('/'));
  } else {
    this.basePackageUri = fullPath.substring(0, fullPath.lastIndexOf('/'));
  }

  this.loadPackage(fullPath);

  this.container.on('onKeyDown', function (e) {
    log.message(2, "container(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));
  this.innerscene.on('onKeyDown', function (e) {
    log.message(2, "innerscene(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));
  this.innerscene.root.on('onKeyDown', function (e) {
    log.message(2, "innerscene root(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));

  log.info("loadScene() - ends");

}

AppSceneContext.prototype.setFocus = function() {
  log.info("setFocus");
  this.rootScene.setFocus(this.container);
}

AppSceneContext.prototype.onResize = function(resizeEvent) {
  var hrTime = process.hrtime(this.lastHrTime);
  var deltaMillis = (hrTime[0] * 1000 + hrTime[1] / 1000000);
  this.lastHrTime = process.hrtime();
  if( deltaMillis > 300 ) {
    if( this.resizeTimer != null ) {
      clearTimeout(this.resizeTimer);
      this.resizeTimer = null;
    }
    this.container.w = resizeEvent.w;
    this.container.h = resizeEvent.h;
  } else {
    var lastWidth = resizeEvent.w;
    var lastHeight = resizeEvent.h;
    this.resizeTimer = setTimeout(function() {
      this.container.w = lastWidth;
      this.container.h = lastHeight;
    }.bind(this), 500);
  }
}


AppSceneContext.prototype.loadPackage = function(packageUri) {
  var _this = this;

  var moduleLoader = new SceneModuleLoader();

  moduleLoader.loadScenePackage({fileUri:packageUri})
    .then(function() {
      _this.fileArchive = moduleLoader.getFileArchive();
      _this.packageManifest = moduleLoader.getManifest();
      var main = _this.packageManifest.getMain();

      if( moduleLoader.isDefaultManifest() ) {
        // let's see if there's one on the server
        _this.getFile("package.json").then( function(packageFileContents) {
          var manifest = new SceneModuleManifest();
          manifest.loadFromJSON(packageFileContents);
          _this.importProtocolPaths = manifest.getNamespaceImportPaths();

          var mainCode = _this.fileArchive.getFileContents(main);
          _this.runScriptInVMContext(mainCode, main);
        }).catch(function(e){
          var mainCode = _this.fileArchive.getFileContents(main);
          _this.runScriptInVMContext(mainCode, main);
        });
      } else {
        _this.importProtocolPaths = _this.packageManifest.getNamespaceImportPaths();
        var mainCode = _this.fileArchive.getFileContents(main);
        _this.runScriptInVMContext(mainCode, main);
      }

    })
    .catch(function(err) {
      console.error("Error: Did not load fileArchive: Error=" + err );
    });

}

AppSceneContext.prototype.runScriptInVMContext = function (code, uri) {
  var sceneForChild = this.innerscene;
  var apiForChild = this;

  // TODO: This is the name that will show up in stack traces. We should
  // resolve ./ to full paths (maybe).
  var fname = uri;
  //var fakeUri = "http://localhost:8000/yo?abc=xyz";
  var urlParts = url.parse(uri, true);

  var thisAppSceneContext = this;
  var xModule = new XModule(urlParts.pathname, this);

  var self = this;
  var newSandbox;
  try {
    newSandbox = {
      sandboxName: "InitialSandbox",
      xresys: this.xresys,
      xmodule: xModule,
      console: console,
      rootScene: this.rootScene,
      scene: sceneForChild,
      runtime: apiForChild,
      process: process,
      sandboxContext1: "true",
      theNamedContext: "Sandbox: " + uri,
      Buffer: Buffer,
      require: function (pkg) {
        log.message(3, "SecureRequire2: " + pkg);
        // TODO: do whitelist/validate import here
        return requireIt(pkg);

      },
      setTimeout: setTimeout,
      setInterval: setInterval,
      clearInterval: clearInterval,
      baseXreModuleDirectory: __dirname,
      basePackageUri: this.basePackageUri,
      createInstance: createComponent.bind(this),
      getFile: getFile.bind(this),
      buildAbsoluteFilePath: buildAbsoluteFilePath.bind(this),
      framework: {
        getNamespaceBaseFilePath: getNamespaceBaseFilePath.bind(this),
        installBaseFramework: installBaseFramework.bind(this)
      },
      app: {
        getNamespaceBaseFilePath: getNamespaceBaseFilePath.bind(this),
        installAppFramework: installAppFramework.bind(this),
        getPackageBaseFilePath: getPackageBaseFilePath.bind(this),
        getPackageFile: getFile.bind(this),
        inputParams: this.queryParams
      },
      importTracking: {}
    } // end sandbox
    xModule.initSandbox(newSandbox);
    thisAppSceneContext.sandbox = newSandbox; //xModule.sandbox;

    try {
      // TODO: This form will not work until nodejs >= 0.12.x
      // var opts = { filename: fname, displayError: true };
      var sourceCode = AppSceneContext.wrap(code);
      var script = new vm.Script(sourceCode, fname);
      var moduleFunc = script.runInNewContext(newSandbox); //xModule.sandbox);
      var px = {
        log: xModule.log,
        import: xmodImportModule.bind(xModule),
        addImportProtocolPath: addImportProtocolPath.bind(self),
        //addImportRedirect: addImportRedirect.bind(this)
      };
      moduleFunc(px, xModule, fname, this.basePackageUri);

      // TODO do the old scenes context get released when we reload a scenes url??

      // TODO part of an experiment to eliminate intermediate rendering of the scene
      // while it is being set up
      if (true) { // enable to fade scenes in
        this.container.a = 0;
        this.container.painting = true;
        this.container.animateTo({a: 1}, 0.2, 0, 0);
        //this._scene.setFocus(container);
      }
      else
        this.container.painting = true;
    }
    catch (err) {
      console.error("failed to run app:" + uri);
      console.error(err);

      // TODO: scene.onError(err); ???
      // TODO: at this point we need to destroy the child scene
      scene.url = "";  // This destroys the child scene and releases scene.ctx
      apiForChild.destroyScene(sandbox.scene);

      sandbox.console = null;
      sandbox.scene = null;
      sandbox.runtime = null;
      sandbox.process = null;

      // log.message(4, util.inspect(sandbox));
    }
  }
  catch (err) {
    console.error("failed to load script:" + uri + "; error=" + err);
    console.error(err);
    // TODO: scene.onError(err); ???
  }
}

function addImportRedirect(basePath, redirect) {
  this.addImportRedirect(basePath, redirect);
}

function addImportProtocolPath(protocol, redirect) {
  this.addImportProtocolPath(protocol, redirect);
}

AppSceneContext.prototype.addImportRedirect = function(basePath, redirect) {
  this.importRedirects[basePath] = redirect;
}

AppSceneContext.prototype.addImportProtocolPath = function(protocol, redirect) {
  this.importProtocolPaths[protocol] = redirect;
}


AppSceneContext.prototype.getPackageBaseFilePath = function() {
  var fullPath;
  var pkgPart;
  if (this.basePackageUri.substring(0, 4) !== "http") {
    if (this.basePackageUri.charAt(0) == '.') {
      pkgPart = this.basePackageUri.substring(1);
    } else {
      pkgPart = this.basePackageUri;
    }
    if (pkgPart.charAt(0) == '/') {
      fullPath = this.defaultBaseUri + pkgPart;
    } else {
      fullPath = this.defaultBaseUri + "/" + pkgPart;
    }
  } else {
    fullPath = this.basePackageUri;
  }

  return fullPath;
}

function getPackageBaseFilePath() {
  return this.getPackageBaseFilePath();
}

AppSceneContext.prototype.getNamespaceBaseFilePath = function() {
  return this.defaultBaseUri;
}

function getNamespaceBaseFilePath() {
  return this.getNamespaceBaseFilePath();
}


AppSceneContext.prototype.buildFullFilePath = function(filePath) {
  var urlParts = url.parse(filePath, true);
  var fullPath = filePath;

  if( urlParts.hasOwnProperty('protocol') && urlParts['protocol'] != null ) {
    var protocol = urlParts['protocol'];

    if( protocol !== 'undefined' && protocol.length > 0 ) {
      var proto = protocol.substr(0,protocol.length-1);
      if( this.importProtocolPaths.hasOwnProperty(proto) ) {
        var protoPath = this.importProtocolPaths[proto];
        fullPath = this.importProtocolPaths[proto] + "/" + fullPath.substring(6);
      }
    }
  } else {
    var sbase = this.basePackageUri;
    var fileToLoad = filePath;
    if( filePath.charAt(0) === '/' ) {
      fileToLoad = filePath.substring(1);
    }


    if( this.fileArchive.hasFileContents(fileToLoad) ) {
      log.message(10, "buildFullFilePath(): The file is in the archive: " + fileToLoad);
    } else {
      if (sbase.substring(0, 4) !== "http") {
        if( sbase.charAt(0) == '/' ) {
          fullPath = sbase + "/" + filePath; //this.defaultBaseUri + sbase + "/" + filePath;
        } else {
          fullPath = sbase + "/" + filePath; //this.defaultBaseUri + "/" + sbase + "/" + filePath;
        }
      } else {
        fullPath = sbase + "/" + filePath;
      }
    }


  }

  return fullPath;

}

function getFile(filePath) {
  return this.getFile(filePath);
}

function buildAbsoluteFilePath(filePath) {
  return this.buildFullFilePath(filePath);
}

AppSceneContext.prototype.getFile = function(filePath) {
  var _this = this;
  log.message(4, "getFile: requestedFile=" + filePath);

  var fullPath = this.buildFullFilePath(filePath);
  log.message(4, "getFile: fullPath=" + fullPath);
  if( this.fileArchive.hasFileContents(fullPath) ) {
    return new Promise(function(resolve, reject) {
      //console.log("getFileContents of " + fullPath);
      resolve(_this.fileArchive.getFileContents(fullPath));
    });
  }
  return loadFile(fullPath);
}

AppSceneContext.prototype.include = function(filePath, currentXModule) {
  log.message(4, ">>> include(" + filePath + ") for " + currentXModule.name + " <<<");
  var _this = this;
  var origFilePath = filePath;
  return new Promise(function (onImportComplete, reject) {
    if( filePath === 'fs' || filePath === 'px' || filePath === 'http' || filePath === 'url' || filePath === 'os'
      || filePath === 'events' || filePath === 'net') {
      // built-ins
      var modData = require(filePath);
      onImportComplete([modData, origFilePath]);
      return
    }

    if( _this.isScriptDownloading(filePath) ) {
      log.message(4, "Script is downloading for " + filePath);
      _this.addModuleReadyListener(filePath, function(moduleExports) {
        onImportComplete([moduleExports, origFilePath]);

        return;
      });
    }
    var haveFile = _this.isScriptLoaded(filePath);

    if (haveFile == false) {
      var urlParts = url.parse(filePath, true);
      var fullPath = filePath;

      if( urlParts.hasOwnProperty('protocol') &&urlParts['protocol'] !== null ) {
        var protocol = urlParts['protocol'];
        if( protocol !== 'undefined' && protocol.length > 0 ) {
          var proto = protocol.substr(0,protocol.length-1);
          if( _this.importProtocolPaths.hasOwnProperty(proto) ) {
            var protoPath = _this.importProtocolPaths[proto];
            fullPath = _this.importProtocolPaths[proto] + "/" + fullPath.substring(6);
          }
        }
      } else {
        var fileToLoad = filePath;
        if( filePath.charAt(0) === '/' ) {
          fileToLoad = filePath.substring(1);
        }
        if( _this.fileArchive.hasFileContents(fileToLoad) ) {
          _this.processCodeBuffer(origFilePath, filePath, currentXModule, _this.fileArchive.getFileContents(fileToLoad), onImportComplete, reject);
          return;
        } else {
          fullPath = _this.basePackageUri + "/" + filePath;
        }
      }

      // acquire file
      _this.setScriptStatus(filePath, 'downloading');
      _this.asyncFileAcquisition.acquire(fullPath)
        .then(function(moduleLoader){
          var xModule;
          log.message(4, "PROCESS RCVD MODULE: " + filePath);
          // file acquired
          // is it still needed - another one may have already arrived from a different module
          if( _this.isScriptReady(filePath) ) {
            var modExports = _this.getScriptContents(filePath);
            onImportComplete([modExports, origFilePath]);
          } else if( _this.isScriptLoaded((filePath))) {
            log.message(4, "It looks like module script is already loaded -- no need to run it");
            _this.addModuleReadyListener(filePath, function(moduleExports) {
              log.message(7, "Received moduleExports from other download" );
              onImportComplete([moduleExports, origFilePath]);
            });
          } else {
            log.message(4, "Need to run script: " + filePath);
            var currentFileArchive;
            var currentFileManifest;
            ///_this.fileArchive = moduleLoader.getFileArchive();
            ///currentFileArchive = _this.fileArchive;
            currentFileArchive = moduleLoader.getFileArchive();
            currentFileManifest = moduleLoader.getManifest();
            var main = currentFileManifest.getMain();

            var mainCode = currentFileArchive.getFileContents(main);

            _this.processCodeBuffer(origFilePath, filePath, currentXModule, mainCode, onImportComplete, reject);

          }

        }).catch(function(err){
          console.error("Error: could not load file " + filePath  + ", err=" + err);
          reject("include failed");
        });

    } else {
      log.message(4, "Already have file loaded and ready, just return the existing content: " + filePath);
      onImportComplete([_this.getScriptContents(filePath), origFilePath]);
    }
  });

}

AppSceneContext.prototype.processCodeBuffer = function(origFilePath, filePath, currentXModule, codeBuffer, onImportComplete, onImportRejected) {
  var _this = this;
  var xModule;

  var haveFile = _this.isScriptLoaded(filePath);

  _this.setScriptStatus(filePath, 'downloading');

  xModule = this.getXModule(filePath);
  if( xModule === 'undefined' ) {
    log.message(7, "cb Creating new XModule for " + filePath);

    xModule = new XModule(filePath, _this);
    xModule.initSandbox(_this.sandbox);
    var sourceCode = AppSceneContext.wrap(codeBuffer);
    var script = new vm.Script(sourceCode, filePath);

    var moduleFunc = script.runInContext(_this.sandbox);
    var px = {
      log: xModule.log,
      import: xmodImportModule.bind(xModule)
    };

    log.message(4, "cbRUN " + filePath);
    moduleFunc(px, xModule, filePath, origFilePath);
    log.message(4, "cbRUN DONE: " + filePath);

    this.setXModule(filePath, xModule);


    // Set up a async wait until module indicates it's completly ready
    var modReadyPromise = xModule.moduleReadyPromise;
    if( modReadyPromise == null ) {

      // It's possible that these exports have already been added
      _this.addScript(filePath, 'ready', xModule.exports);

      _this.addImportedPublicClasses(xModule.exports, xModule.publicClasses);

      onImportComplete([xModule.exports, origFilePath]);
      log.message(4, "AppSceneContext after notifying[:" + currentXModule.name + "] about import<" + filePath + ">");
      // notify 'ready' listeners
      _this.callModuleReadyListeners(filePath, xModule.exports);
    } else {
      // Now wait for module to indicate that it's fully ready to go
      modReadyPromise.then(function () {
        log.message(7, "AppSceneContext[xModule=" + xModule.name + "]: is notified that <" + filePath + "> MODULE INDICATES IT'S FULLY READY");
        _this.addScript(filePath, 'loaded', xModule.exports);
        _this.setScriptStatus(filePath, 'ready');
        log.message(7, "AppSceneContext: is about to notify [" + currentXModule.name + "] that <" + filePath + "> has been imported and is ready");
        _this.addImportedPublicClasses(xModule.exports, xModule.publicClasses);

        onImportComplete([xModule.exports, origFilePath]);
        log.message(8, "AppSceneContext after notifying[:" + currentXModule.name + "] about import<" + filePath + ">");
        // notify 'ready' listeners
        _this.callModuleReadyListeners(filePath, xModule.exports);
      }).catch(function (error) {
        onImportRejected("include(2): failed while waiting for module <" + filePath + "> to be ready for [" + currentXModule.name + "] - error=" + error);
      });

    }

  }

}

AppSceneContext.prototype.addImportedPublicClasses = function(exports, classMap) {
  if( typeof classMap !== 'undefined' ) {
    for ( var key in classMap) {
      if (classMap.hasOwnProperty(key)) {
        if (!this.publicClasses.hasOwnProperty(key)) {
          this.publicClasses[key] = classMap[key];
          log.message(4, "Added public class: " + key);
        } else {
          console.trace("Wait a second... ClassName<" + key + "> is already listed in the public class list");
        }
      }
    }
  } else {
    log.message(10, "addImportPublicClasses() classMap undefined");
  }

  if( typeof(exports) === 'function') {
    var exp = exports;
    log.message(4, "Module Exporting a function: " + exp.name);
    this.publicClasses[exp.name] = exports;
  }

}

AppSceneContext.prototype.getConstructor = function(className) {
  if( this.publicClasses.hasOwnProperty(className) ) {
    log.message(4, "getConstructor(" + className + "): returns " + this.publicClasses[className] + ", typeof " + typeof(this.publicClasses[className]));
    return this.publicClasses[className];
  } else {
    console.trace("Unknown className[" + className + "]");
    return null;
  }
}

AppSceneContext.prototype.createClass = function(className, params) {
  if( this.publicClasses.hasOwnProperty(className) ) {
    log.message(10, "createClass(" + className + "): returns " + ", typeof " + typeof(this.publicClasses[className])); // + this.publicClasses[className]);
    return new this.publicClasses[className](params);
  } else {
    console.trace("Unknown className[" + className + "]");
    return null;
  }
}

function createComponent(className, params) {
  log.message(8, "ASC#createComponent(" + className + ", " + params + ")");
  return this.createClass(className, params);
}

AppSceneContext.prototype.addModuleReadyListener = function(moduleName, callback) {
  if( this.scriptMap.hasOwnProperty(moduleName) ) {
    this.scriptMap[moduleName].readyListeners.push(callback);
  } else {
    console.trace('AppSceneContext#addModuleReadyListener: no entry in map for module [' + moduleName + ']');
  }
}

AppSceneContext.prototype.callModuleReadyListeners = function(moduleName, moduleExports) {
  log.message(4, "Call ModuleReadyListeners for module: " + moduleName);
  if( this.scriptMap.hasOwnProperty(moduleName) ) {
    var listeners = this.scriptMap[moduleName].readyListeners;
    if( listeners != null && listeners.length != 0 ) {
      for(var k = 0; k < listeners.length; ++k) {
        listeners[k](moduleExports);
      }
    }
    this.scriptMap[moduleName].readyListeners = null;
  } else {
    console.trace('AppSceneContext#callModuleReadyListeners: no entry in map for module [' + moduleName + ']');
  }
}

AppSceneContext.prototype.setXModule = function(name, xmod) {
  this.xmoduleMap[name] = xmod;
}

AppSceneContext.prototype.getXModule = function(name) {
  if( this.xmoduleMap.hasOwnProperty(name) ) {
    return this.xmoduleMap[name].xmod;
  }

  return 'undefined';
}

AppSceneContext.prototype.addScript = function(name, status, scriptObject) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    var curData = this.scriptMap[name];
    curData.status = status;
    if( status == 'ready' && (typeof scriptObject == 'undefined' || scriptObject == null) ) {
      console.trace("Whoa: seting Ready state but there is no scriptObject");
    }
    if( scriptObject != null && scriptObject !== 'undefined' ) {
      curData.scriptObject = scriptObject;
      log.message(4, "ADDED UPDATED script: " + name + ", status=" + status)
    }

    var oldScriptObject = this.scriptMap[name].scriptObject;;
    if( oldScriptObject == null && scriptObject != null ) {
      console.trace("Script object changing from null, but isn't being set");
    }

    log.message(8, "AddScript " + name + ", status=" + status + ", object=" + typeof(scriptObject));

  } else {
    this.scriptMap[name] = {status: status, scriptObject: scriptObject, readyListeners:[]};
    log.message(4, "ADDED NEW script: " + name + ", status=" + status)
  }
}

AppSceneContext.prototype.getScriptContents = function(name) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    return this.scriptMap[name].scriptObject;
  } else {
    return null;
  }
}

AppSceneContext.prototype.setScriptStatus = function(name, status) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    this.scriptMap[name].status = status;
    var scriptObject = this.scriptMap[name].scriptObject;
    if( status == 'ready' && (typeof scriptObject == 'undefined' || scriptObject == null) ) {
      console.trace("Whoa: seting Ready state but there is no scriptObject");
    }

    log.message(8, "1) SetScriptStatus " + name + ", status=" + status + ", object=" + typeof(scriptObject));

  } else {
    log.message(8, "0) SetScriptStatus " + name + ", status=" + status + ", null");
    this.addScript(name, status, null);
  }
}

AppSceneContext.prototype.getScriptStatus = function(name) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    return this.scriptMap[name].status;
  }

  return 'undefined';
}

AppSceneContext.prototype.isScriptDownloading = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && this.scriptMap[name].status === "downloading" ) {
    log.message(4, "isScriptDownloading(" + name + ")? Yes");
    return true;
  }

  log.message(4, "isScriptDownloading(" + name + ")? NOT DOWNLOADED YET");
  return false;
}

AppSceneContext.prototype.isScriptLoaded = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && (this.scriptMap[name].status === "loaded" || this.scriptMap[name].status === "ready") ) {
    log.message(4, "isScriptLoaded(" + name + ")? Yes");
    return true;
  }

  log.message(4, "isScriptLoaded(" + name + ")? NOT LOADED YET");
  return false;
}

AppSceneContext.prototype.isScriptReady = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && this.scriptMap[name].status === "ready" ) {
    log.message(4, "isScriptReady(" + name + ")?  Yes it's READY");
    return true;
  }

  log.message(4, "isScriptReady(" + name + ")?  NOT READY YET");
  return false;
}

function AsyncFileAcquisition() {
  this.requestMap = {};
}

AppSceneContext.prototype.installBaseFramework = function(params) {
  for (var key in params) {
    this.sandbox.framework[key] = params[key];
  }
}

function installBaseFramework(params) {
  this.installBaseFramework(params);
}

AppSceneContext.prototype.installAppFramework = function(params) {
  for (var key in params) {
    this.sandbox.app[key] = params[key];
  }
}

function installAppFramework(params) {
  this.installAppFramework(params);
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
      moduleLoader.loadScenePackage({fileUri:uri})
        .then(function() {
          log.message(4, "---> ACQUIRED: " + uri);
          resolve(moduleLoader);
          var listeners = self.requestMap[uri].listeners;
          if( listeners != null && listeners.length != 0 ) {
            for(var k = 0; k < listeners.length; ++k) {
              listeners[k]('resolve');
            }
          }
          self.requestMap[uri].listeners = null;
          delete self.requestMap[uri];
        })
        .catch(function(error){
          console.error("Error: could not load file " + filePath  + ", error=" + error);
          reject(error);
          var listeners = self.requestMap[uri].listeners;
          if( listeners != null && listeners.length != 0 ) {
            for(var k = 0; k < listeners.length; ++k) {
              listeners[k]('reject', error);
            }
          }
          self.requestMap[uri].listeners = null;
          delete self.requestMap[uri];
        });
    }
  });

}

AppSceneContext.wrap = function(script) {
  return AppSceneContext.wrapper[0] + script + AppSceneContext.wrapper[1];
};

AppSceneContext.wrapper = [
  '(function (px, module, __filename, __dirname) { ',
  '\n});'
];




module.exports = AppSceneContext;
