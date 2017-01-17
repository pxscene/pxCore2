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

function AppSceneContext(params) { // container, innerscene, packageUrl) {
  //  this.container = params.sceneContainer;

  this.getContextID = params.getContextID;
  this.makeReady = params.makeReady;
  this.innerscene = params.scene;
  this.rpcController = params.rpcController;
  if( params.packageUrl.indexOf('?') != -1 ) {
    var urlParts = url.parse(params.packageUrl, true);
    this.queryParams = urlParts.query;
    if (params.packageUrl.substring(0, 4) === "http") {
      this.packageUrl = urlParts.protocol + "//" + urlParts.host + urlParts.pathname;
    } else {
      this.packageUrl = params.packageUrl;
    }
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
  this.asyncFileAcquisition = new AsyncFileAcquisition(params.scene);
  this.lastHrTime = process.hrtime();
  this.resizeTimer = null;
  this.topXModule = null;
  this.jarFileMap = {};
  this.sceneWrapper = null;
  log.message(4, "[[[NEW AppSceneContext]]]: " + this.packageUrl);
}

AppSceneContext.prototype.loadScene = function() {
  //log.info("loadScene() - begins    on ctx: " + getContextID() );
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

if (false) {
if (false) {
  // This no longer has access to the container
  this.container.on('onKeyDown', function (e) {
    log.message(2, "container(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));

}
  this.innerscene.on('onKeyDown', function (e) {
    log.message(2, "innerscene(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));

  this.innerscene.root.on('onKeyDown', function (e) {
    log.message(2, "innerscene root(" + this.packageUrl + "): keydown:" + e.keyCode);
  }.bind(this));
}

if (false) {
  // JRJRJR No longer get this event...
  this.innerscene.on('onComplete', function (e) {
//    this.container = null;
    this.innerscene = null;
    this.sandbox = null;
    for(var key in this.scriptMap) {
      this.scriptMap[key].scriptObject = null;
      this.scriptMap[key].readyListeners = null;
/* JRJRJR
      delete this.scriptMap[key].scriptObject;
      delete this.scriptMap[key].readyListeners;
*/
    }
    this.scriptMap = null;
    for(var xmodule in this.xmoduleMap) {
      this.xmoduleMap[xmodule].freeResources();
    }
    this.xmoduleMap = null;
    this.topXModule = null;
    this.jarFileMap = null;
    this.sceneWrapper = null;
    global.gc();
  }.bind(this));
}

  //log.info("loadScene() - ends    on ctx: " + getContextID() );

}

AppSceneContext.prototype.loadPackage = function(packageUri) {
  var _this = this;

  var moduleLoader = new SceneModuleLoader();

  moduleLoader.loadScenePackage(this.innerscene, {fileUri:packageUri})
    .then(function processScenePackage() {
      _this.fileArchive = moduleLoader.getFileArchive();
      _this.packageManifest = moduleLoader.getManifest();
      var main = _this.packageManifest.getMain();

      var configImport = {};
      if( moduleLoader.isDefaultManifest() ) {
        _this.getFile("package.json").then( function(packageFileContents) {
          var manifest = new SceneModuleManifest();
          manifest.loadFromJSON(packageFileContents);

          configImport = manifest.getConfigImport();

          var mainCode = _this.fileArchive.getFileContents(main);
          _this.runScriptInNewVMContext(mainCode, main, false, configImport);
        }).catch(function(e){
          var mainCode = _this.fileArchive.getFileContents(main);
          _this.runScriptInNewVMContext(mainCode, main, false, null);
        });
      } else {
        configImport = _this.packageManifest.getConfigImport();
        _this.jarFileMap[packageUri] = _this.fileArchive;
        var moduleUri = packageUri + "?module=" + main;

        var mainCode = _this.fileArchive.getFileContents(main);
        _this.runScriptInNewVMContext(mainCode, moduleUri, true, configImport);
      }

    })
    .catch(function(err) {
      this.makeReady(false, {});
      console.error("AppSceneContext#loadScenePackage: Error: Did not load fileArchive: Error=" + err );
    });

}

AppSceneContext.prototype.getModuleBasePath = function(moduleUri) {
  var questionMarkIndex = moduleUri.lastIndexOf('?');
  if( questionMarkIndex != -1 ) {
    // might be a jar file uri.  Let's split it up and see if there is a module param
    var urlParts = url.parse(moduleUri, true);
    if( urlParts.query !== 'undefined' && urlParts.query.hasOwnProperty('module')) {
      var relativeModulePath = urlParts.query.module;
      var relativePath = relativeModulePath.substring(0, relativeModulePath.lastIndexOf('/'));
      return {baseUri:moduleUri.substring(0, questionMarkIndex), relativePath:relativePath, isJarFile:true};
    }
  }

  return {baseUri:moduleUri.substring(0, moduleUri.lastIndexOf('/')), isJarFile:false};
}

function createModule_pxScope(xModule) {
  return {
    log: xModule.log,
    import: xmodImportModule.bind(xModule),
    configImport: xModule.configImport.bind(xModule),
    resolveFilePath: xModule.resolveFilePath.bind(xModule),
    appQueryParams: this.queryParams,
    getPackageBaseFilePath: getPackageBaseFilePath.bind(this),
    getFile: getFile.bind(this),
    getModuleFile: xModule.getFile.bind(xModule),
  };

}

function onAppModuleReady(callback) {
}

AppSceneContext.prototype.runScriptInNewVMContext = function (code, uri, fromJarFile, configImport) {
  var sceneForChild = this.innerscene;
  var apiForChild = this;

  // TODO: This is the name that will show up in stack traces. We should
  // resolve ./ to full paths (maybe).
  var fname = uri;
  //var fakeUri = "http://localhost:8000/yo?abc=xyz";
  var urlParts = url.parse(uri, true);

  var thisAppSceneContext = this;
  var xModule = new XModule(urlParts.pathname, this, fromJarFile, this.getModuleBasePath(uri));
  this.topXModule = xModule;
  if( configImport !== null ) {
    xModule.configImport(configImport);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  var self = this;
  var newSandbox;
  try {
    newSandbox = {
      sandboxName: "InitialSandbox",
      xmodule: xModule,
      console: console,
      runtime: apiForChild,
      process: process,
      urlModule: require("url"),
      queryStringModule: require("querystring"),
      theNamedContext: "Sandbox: " + uri,
      Buffer: Buffer,
      require: function (pkg) {
        log.message(3, "old use of require not supported: " + pkg);
        // TODO: remove
        return requireIt(pkg);

      },
      setTimeout: setTimeout,
      setInterval: setInterval,
      clearInterval: clearInterval,
      importTracking: {}
    } // end sandbox

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    xModule.initSandbox(newSandbox);
    thisAppSceneContext.sandbox = newSandbox; //xModule.sandbox;

    try {
      //JRJRJRJR  This line causing a garbage collection leak...
      // LEAKLEAK
//      this.innerscene.api = {isReady:false, onModuleReady:onAppModuleReady.bind(this) };

      var sourceCode = AppSceneContext.wrap(code);
      //var script = new vm.Script(sourceCode, fname);
      //var moduleFunc = script.runInNewContext(newSandbox, {filename:fname, displayErrors:true});

      var moduleFunc = vm.runInNewContext(sourceCode, newSandbox, {filename:fname, displayErrors:true});

      if (process._debugWaitConnect) {
        // Set breakpoint on module start
        delete process._debugWaitConnect;
        const Debug = vm.runInDebugContext('Debug');
        Debug.setBreakPoint(moduleFunc, 0, 0);
      }

      var px = createModule_pxScope.call(this, xModule);
      var rtnObject = moduleFunc(px, xModule, fname, this.basePackageUri);
      rtnObject = xModule.exports;
/*
if (false) {
      // TODO do the old scenes context get released when we reload a scenes url??
      // TODO part of an experiment to eliminate intermediate rendering of the scene - from original load.js
      // while it is being set up
      if (true) { // enable to fade scenes in
        this.container.a = 0;
        this.container.painting = true;
        this.container.animateTo({a: 1}, 0.2, this.innerscene.animation.TWEEN_LINEAR,this.innerscene.animation.OPTION_LOOP,1);
      }
      else {
        this.container.painting = true;
      }
}
*/

      //console.log("Main Module: readyPromise=" + xModule.moduleReadyPromise);
      if( !xModule.hasOwnProperty('moduleReadyPromise') || xModule.moduleReadyPromise === null ) {
//        this.container.makeReady(true); // DEPRECATED ?

//        this.innerscene.api = {isReady:true};
        this.makeReady(true,{});
      }
      else
      {
        var modulePromise = xModule.moduleReadyPromise;
        var thisMakeReady = this.makeReady; // NB:  capture for async then() closure.

        modulePromise.then( function(i)
        {
          self.innerscene.api = xModule.exports;

          thisMakeReady(true,xModule.exports);

        }).catch( function(err)
        {
          console.error("Main module[" + self.packageUrl + "]" + " load has failed - on failed imports: " + ", err=" + err);
//          self.container.makeReady(false); // DEPRECATED ?

          this.makeReady(false,{});
        } );
      }

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

AppSceneContext.prototype.buildFullFilePath = function(filePath) {
  var urlParts = url.parse(filePath, true);
  var fullPath = filePath;

  if( urlParts.hasOwnProperty('protocol') && urlParts['protocol'] != null ) {
    var protocol = urlParts['protocol'];

    if( protocol !== 'undefined' && protocol.length > 0 ) {
      var proto = protocol.substr(0,protocol.length-1);
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

AppSceneContext.prototype.getModuleFile = function(filePath, xModule) {
  var resolvedModulePath;
  resolvedModulePath = this.resolveModulePath(filePath, xModule);
  if( resolvedModulePath.isJarFile ) {
    var theFileArchive = this.jarFileMap[resolvedModulePath.fileUri];
    if( theFileArchive.hasFileContents(resolvedModulePath.relativePath) ) {
      return new Promise(function(resolve, reject) {
        resolve(theFileArchive.getFileContents(resolvedModulePath.relativePath));
      });
      return;
    } else {
      console.error("getModuleFile(): The file was not found in the archive: " + resolvedModulePath.relativePath);
    }
  } else {
    log.message(3, "TJC: getModuleFile("+filePath + "): resolves to " + resolvedModulePath.fileUri);
    return this.getFile(resolvedModulePath.fileUri);
  }
}

AppSceneContext.prototype.getFile_deprecated = function(filePath) {
  var _this = this;
  log.message(4, "getFile: requestedFile=" + filePath);

  var fullPath = this.buildFullFilePath(filePath);
  log.message(4, "getFile: fullPath=" + fullPath);
  if( this.fileArchive.hasFileContents(fullPath) ) {
    return new Promise(function(resolve, reject) {
      resolve(_this.fileArchive.getFileContents(fullPath));
    });
  }
  return loadFile(fullPath);
}

AppSceneContext.prototype.getFile = function(filePath) {
  var _this = this;
  log.message(4, "getFile: requestedFile=" + filePath);

  return loadFile(filePath);
}

AppSceneContext.prototype.resolveModulePath = function(filePath, currentXModule) {
  var replacementMatch = currentXModule.findImportReplacementMatch(filePath);
  if( replacementMatch === null && this.topXModule !== currentXModule) {
    replacementMatch = this.topXModule.findImportReplacementMatch(filePath);
  }
  if( replacementMatch !== null ) {
    log.info(filePath + " ==> " + replacementMatch.fileUri);
    return replacementMatch;
  }
  var normPath;
  var currModBasePath = currentXModule.getBasePath();
  if( currModBasePath.isJarFile ) {
    normPath = url.resolve(currentXModule.getBasePath().relativePath + "/", filePath);
    if( normPath.charAt(0) == '/' ) {
      normPath = normPath.substring(1);
    }
    return {fileUri:currModBasePath.baseUri, relativePath:normPath, isJarFile:true};
  } else {
    var fileUri = "";
    if( filePath.charAt(0) == '/' ) {
      // temporary for now
      fileUri = this.basePackageUri + filePath;
    } else {
      fileUri = url.resolve(currentXModule.getBasePath().baseUri + "/", filePath);
    }

    return {fileUri:fileUri, isJarFile:false};
  }
}

AppSceneContext.prototype.include = function(filePath, currentXModule) {
  log.message(4, ">>> include(" + filePath + ") for " + currentXModule.name + " <<<");
  var _this = this;
  var origFilePath = filePath;
  return new Promise(function (onImportComplete, reject) {
    if( filePath === 'fs' || filePath === 'px' || filePath === 'http' || filePath === 'url' || filePath === 'os'
      || filePath === 'events' || filePath === 'net' || filePath === 'querystring' || filePath === 'htmlparser'
      || filePath === 'ws') {
      // built-ins
      var modData = require(filePath);
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath.substring(0, 9) === "px:scene.") {
      filePath = 'rcvrcore/' + filePath.substring(3);
      var Scene = require(filePath);
      if( _this.sceneWrapper === null ) {
        _this.sceneWrapper = new Scene();
      }
      _this.sceneWrapper._setNativeScene(_this.innerscene, currentXModule.name);
      _this.sceneWrapper._setRPCController(_this.rpcController);
      onImportComplete([_this.sceneWrapper, origFilePath]);
      return;
    } else if( filePath.substring(0,9) === "px:tools.") {
      filePath = 'rcvrcore/tools/' + filePath.substring(9);
      var modData = require(filePath);
      onImportComplete([modData, origFilePath]);
      return;
    }
    var fullIncludeUri;
    var resolvedModulePath;
    if( filePath.substring(0, 9) === "px:scene.") {
      resolvedModulePath = {isJarFile:false, fileUri:__dirname+'/' + filePath.substring(3)};
    } else {
      resolvedModulePath = _this.resolveModulePath(filePath, currentXModule);
    }
    
    if( resolvedModulePath.isJarFile ) {
      fullIncludeUri = resolvedModulePath.fileUri + "?module=" + resolvedModulePath.relativePath;
    } else {
      fullIncludeUri = resolvedModulePath.fileUri;
    }

    if( _this.isScriptDownloading(fullIncludeUri) ) {
      log.message(4, "Script is downloading for " + fullIncludeUri);
      _this.addModuleReadyListener(fullIncludeUri, function(moduleExports) {
        onImportComplete([moduleExports, origFilePath]);

        return;
      });
    }

    var haveFile = _this.isScriptLoaded(fullIncludeUri);

    if (haveFile == false) {
      if( resolvedModulePath.isJarFile ) {
        var theFileArchive = _this.jarFileMap[resolvedModulePath.fileUri];
        if( theFileArchive.hasFileContents(resolvedModulePath.relativePath) ) {
          var moduleBasePath = _this.getModuleBasePath(fullIncludeUri);
          _this.processCodeBuffer(origFilePath, fullIncludeUri, moduleBasePath, currentXModule, theFileArchive.getFileContents(resolvedModulePath.relativePath), true, onImportComplete, reject);
          return;
        } else {
          console.error("The file was not found in the archive: " + resolvedModulePath.relativePath);
        }
      }

      var fullPath = filePath;

      fullPath = fullIncludeUri;

      // acquire file
      _this.setScriptStatus(fullIncludeUri, 'downloading');
      _this.asyncFileAcquisition.acquire(fullPath)
        .then(function(moduleLoader){
          var xModule;
          log.message(4, "PROCESS RCVD MODULE: " + fullIncludeUri);
          // file acquired
          // is it still needed - another one may have already arrived from a different module
          if( _this.isScriptReady(fullIncludeUri) ) {
            var modExports = _this.getScriptContents(fullIncludeUri);
            onImportComplete([modExports, origFilePath]);
          } else if( _this.isScriptLoaded((fullIncludeUri))) {
            log.message(4, "It looks like module script is already loaded -- no need to run it");
            _this.addModuleReadyListener(fullIncludeUri, function(moduleExports) {
              log.message(7, "Received moduleExports from other download" );
              onImportComplete([moduleExports, origFilePath]);
            });
          } else {
            log.message(4, "Need to run script: " + fullIncludeUri);
            var currentFileArchive;
            var currentFileManifest;
            currentFileArchive = moduleLoader.getFileArchive();
            currentFileManifest = moduleLoader.getManifest();
            var main = currentFileManifest.getMain();

            var mainCode = currentFileArchive.getFileContents(main);

            var baseUri = fullPath.substring(0, fullPath.lastIndexOf('/'));
            _this.processCodeBuffer(origFilePath, fullPath, {baseUri:baseUri, isJarFile:false}, currentXModule, mainCode, false, onImportComplete, reject);

          }

        }).catch(function(err){
          console.error("Error: could not load file " + fullIncludeUri  + ", err=" + err);
          reject("include failed");
        });

    } else {
      log.message(4, "Already have file loaded and ready, just return the existing content: " + fullIncludeUri);
      onImportComplete([_this.getScriptContents(fullIncludeUri), origFilePath]);
    }
  });

}

AppSceneContext.prototype.processCodeBuffer = function(origFilePath, filePath, moduleBasePath, currentXModule, codeBuffer, fromJarFile, onImportComplete, onImportRejected) {
  var _this = this;
  var xModule;
  var haveFile = _this.isScriptLoaded(filePath);

  _this.setScriptStatus(filePath, 'downloading');

  xModule = this.getXModule(filePath);
  if( xModule === 'undefined' ) {
    log.message(7, "cb Creating new XModule for " + filePath);

    xModule = new XModule(filePath, _this, fromJarFile, moduleBasePath);
    xModule.initSandbox(_this.sandbox);
    var sourceCode = AppSceneContext.wrap(codeBuffer);

    //var script = new vm.Script(sourceCode, filePath);
    //var moduleFunc = script.runInContext(_this.sandbox);
    moduleFunc = vm.runInContext(sourceCode, _this.sandbox, {filename:filePath, displayErrors:true});

    var px = createModule_pxScope.call(this, xModule);

    log.message(4, "RUN " + filePath);
    moduleFunc(px, xModule, filePath, origFilePath);
    log.message(4, "RUN DONE: " + filePath);

    this.setXModule(filePath, xModule);


    // Set up a async wait until module indicates it's completly ready
    var modReadyPromise = xModule.moduleReadyPromise;
    if( modReadyPromise == null ) {
      // No use of px.import or it's possible that these exports have already been added
      _this.addScript(filePath, 'ready', xModule.exports);

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

/*
AppSceneContext.prototype.setFocus = function() {
  log.info("setFocus");
  this.rootScene.setFocus(this.container);
}
*/



AppSceneContext.prototype.onResize = function(resizeEvent) {
  var hrTime = process.hrtime(this.lastHrTime);
  var deltaMillis = (hrTime[0] * 1000 + hrTime[1] / 1000000);
  this.lastHrTime = process.hrtime();
  if( deltaMillis > 300 ) {
    if( this.resizeTimer != null ) {
      clearTimeout(this.resizeTimer);
      this.resizeTimer = null;
    }
//    this.container.w = resizeEvent.w;
//    this.container.h = resizeEvent.h;
  } else {
    var lastWidth = resizeEvent.w;
    var lastHeight = resizeEvent.h;
    this.resizeTimer = setTimeout(function() {
//      this.container.w = lastWidth;
//      this.container.h = lastHeight;
    }.bind(this), 500);
  }
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
          if( listeners != null && listeners.length != 0 ) {
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
