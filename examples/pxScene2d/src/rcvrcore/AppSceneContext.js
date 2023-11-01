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

//"use strict";

var isDuk=(typeof Duktape != "undefined")?true:false;
var isV8=(typeof _isV8 != "undefined")?true:false;

var url = require('url');
var path = require('path');
var vm = require('vm');
var Logger = require('rcvrcore/Logger').Logger;
var SceneModuleLoader = require('rcvrcore/SceneModuleLoader');
var XModule = require('rcvrcore/XModule');
var loadFile = require('rcvrcore/utils/FileUtils').loadFile;
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
var JarFileMap = require('rcvrcore/utils/JarFileMap');
var AsyncFileAcquisition = require('rcvrcore/utils/AsyncFileAcquisition');
var WrapObj = require('rcvrcore/utils/WrapObj');
var http_wrap = require('rcvrcore/http_wrap');
var processKeys = ['env' ,'binding', 'hrtime', 'memoryUsage']

var log = new Logger('AppSceneContext');
//overriding original timeout and interval functions
var SetTimeout = (isDuk || isV8)?timers.setTimeout:setTimeout;
var ClearTimeout = (isDuk || isV8)?timers.clearTimeout:clearTimeout;
var SetInterval = (isDuk || isV8)?timers.setInterval:setInterval;
var ClearInterval = (isDuk || isV8)?timers.clearInterval:clearInterval;
var console = (isDuk || isV8)?global.console:require('console_wrap');

function AppSceneContext(params) {

  this.getContextID = params.getContextID;
  this.makeReady = params.makeReady;
  this.innerscene = params.scene;
  this.rpcController = params.rpcController;
  if( params.packageUrl.indexOf('?') !== -1 ) {
    var urlParts = url.parse(params.packageUrl, true);
    this.queryParams = urlParts.query;
    this.packageUrl = params.packageUrl;
  } else {
    this.queryParams = {};
    this.packageUrl = params.packageUrl;
  }
  this.basePackageUri = "";
  this.sandbox = {};
  this.scriptMap = {};
  this.xmoduleMap = {};
  this.asyncFileAcquisition = new AsyncFileAcquisition(params.scene);
  this.lastHrTime = isDuk?uv.hrtime():(isV8?uv_hrtime():process.hrtime());
  this.resizeTimer = null;
  this.topXModule = null;
  this.jarFileMap = new JarFileMap();
  this.sceneWrapper = null;
  this.thunderWrapper = null;
  //array to store the list of pending timers
  this.timers = [];
  this.timerIntervals = [];
  this.webSocketManager = null;
  this.disableFilePermissionCheck = isV8?true:this.innerscene.sparkSetting("disableFilePermissionCheck");
  if (undefined == this.disableFilePermissionCheck)
  {
    this.disableFilePermissionCheck = false;
  }
  // event received indicators for close and terminate
  this.isCloseEvtRcvd = false;
  this.isTermEvtRcvd = false;
  this.termEvent = null;
  this.isTerminated = false;
  log.message(4, "[[[NEW AppSceneContext]]]: " + this.packageUrl);
}


AppSceneContext.prototype.loadScene = function() {

  const thisPackageUrl = this.packageUrl.split('?')[0];
  this.basePackageUri = path.dirname(thisPackageUrl);

  this.loadPackage(thisPackageUrl);

function terminateScene() {
    var e = this.termEvent;
    if (null != this.webSocketManager)
    {
       this.webSocketManager.clearConnections();
       delete this.webSocketManager;
    }
    this.webSocketManager = null;
    //clear the timers and intervals on close
    var ntimers = this.timers.length;
    for (var i=0; i<ntimers; i++)
    {
      clearTimeout(this.timers.pop());
    }
    var ntimerIntervals = this.timerIntervals.length;
    for (var i=0; i<ntimerIntervals; i++)
    {
      clearInterval(this.timerIntervals.pop());
    }
    if (this.innerscene.api !== undefined)
    {
      for(var k in this.innerscene.api) { delete this.innerscene.api[k]; }
    }

    if ((undefined != this.innerscene) && (null != this.innerscene))
    {
      this.innerscene.api = null;
    }
    this.innerscene = null;
    if (this.sandbox)
    {
      this.sandbox.sandboxName = null;
      this.sandbox.console = null;
      this.sandbox.process = null;
      this.sandbox.theNamedContext = null;
      this.sandbox.Buffer = null;
      this.sandbox.require = null;
      this.sandbox.global = null;
      this.sandbox.setTimeout = null;
      this.sandbox.setInterval = null;
      this.sandbox.clearTimeout = null;
      this.sandbox.clearInterval = null;
      for(var k in this.sandbox.importTracking) { delete this.sandbox.importTracking[k]; }
      this.sandbox.importTracking = null;
      for(var k in this.sandbox) { delete this.sandbox[k]; }
    }
    this.sandbox = null;
    for(var xmodule in this.xmoduleMap) {
      this.xmoduleMap[xmodule].freeResources();
      delete this.xmoduleMap[xmodule];
    }
    this.xmoduleMap = null;
    if (this.asyncFileAcquisition) {
      this.asyncFileAcquisition.scene = null;
      this.asyncFileAcquisition = null;
    }
    if (null != this.topXModule)
      this.topXModule.freeResources();
    this.topXModule = null;
    this.jarFileMap = null;
    for(var key in this.scriptMap) {
      this.scriptMap[key].scriptObject = null;
      this.scriptMap[key].readyListeners = null;
      delete this.scriptMap[key];
    }
    this.scriptMap = null;
    if (null != this.sceneWrapper)
      this.sceneWrapper.close();
    this.sceneWrapper = null;
    if (null != this.thunderWrapper)
      this.thunderWrapper.close();
    this.thunderWrapper = null;
    this.rpcController = null;
    this.isCloseEvtRcvd = false;
    this.isTermEvtRcvd = false;
    this.termEvent = null;
    this.isTerminated = true;
}

this.innerscene.on('onSceneTerminate', function(e) { 
     this.isTermEvtRcvd = true;
     this.termEvent = e;
     // make sure we are sending terminate event only after close event
     if (true == this.isCloseEvtRcvd) {
       terminateScene.bind(this)(); 
     }
  }.bind(this));

this.innerscene.on('onClose', function() {
    // make sure terminate event is sent after immediately if onClose comes after it
    if (true == this.isTermEvtRcvd)
    {
      terminateScene.bind(this)();
    }
    this.isCloseEvtRcvd = true;
  }.bind(this));

  //log.info("loadScene() - ends    on ctx: " + getContextID() );
};

AppSceneContext.prototype.loadPackage = function(packageUri) {
  var _this = this;
  var moduleLoader = new SceneModuleLoader();
  // Fixed scene loading promise rejection
  var thisMakeReady = this.makeReady;

  moduleLoader.loadScenePackage(this.innerscene, {fileUri:packageUri})
    .then(function processScenePackage() {
      if( moduleLoader.isDefaultManifest() ) {
        _this.getFile("package.json").then( function(packageFileContents) {
          var manifest = new SceneModuleManifest();
          manifest.loadFromJSON(packageFileContents);
          //console.info("AppSceneContext#loadScenePackage0");
          _this.runScriptInNewVMContext(packageUri, moduleLoader, manifest.getConfigImport());
          //console.info("AppSceneContext#loadScenePackage0 done");
        }).catch(function (e) {
            //console.info("AppSceneContext#loadScenePackage1");
            _this.runScriptInNewVMContext(packageUri, moduleLoader, null);
            //console.info("AppSceneContext#loadScenePackage1 done");
        });
      } else {
        var manifest = moduleLoader.getManifest();
        //console.info("AppSceneContext#loadScenePackage2");
        _this.runScriptInNewVMContext(packageUri, moduleLoader, manifest.getConfigImport());
        //console.info("AppSceneContext#loadScenePackage2 done");
      }
    })
    .catch(function (err) {
      //console.info("AppSceneContext#loadScenePackage3");
      thisMakeReady(false, {});
      console.error("AppSceneContext#loadScenePackage: Error: Did not load fileArchive: Error=" + JSON.stringify(err));
    });
};

var setTimeoutCallback = function() {
  // gets the timers list and remove the callback timer from the list of pending lists
  var contextTimers = arguments[0];
  var callback = arguments[1];
  callback();
  callback = null;

  var index = contextTimers.indexOf(this);
  if (index != -1)
  {
    contextTimers.splice(index,1);
  }
  ClearTimeout(this);
};

function getBaseFilePath()
{
  return this;
}

function createModule_pxScope(xModule, isImported) {
  var params  = {     log: xModule.log,
    import: xModule.importModule.bind(xModule),
    configImport: xModule.configImport.bind(xModule),
    resolveFilePath: xModule.resolveFilePath.bind(xModule),
    appQueryParams: this.queryParams,
    getPackageBaseFilePath: this.getPackageBaseFilePath.bind(this),
    getFile: this.getFile.bind(this),
    getModuleFile: xModule.getFile.bind(xModule)
  };

  //xModule.basePath is used for imported files, as xModule is per javascript file basis
  if (true == isImported)
  {
    if (xModule.basePath == "")
    {
      params.getBaseFilePath = getBaseFilePath.bind("./");
    }
    else
    {
      params.getBaseFilePath = getBaseFilePath.bind(xModule.basePath+"/");
    }
  }
  else
  {
    params.getBaseFilePath = params.getPackageBaseFilePath;
  }
  return params;
}

AppSceneContext.prototype.runScriptInNewVMContext = function (packageUri, moduleLoader, configImport) {
  var apiForChild = this;
  var isJar = moduleLoader.jarFileWasLoaded();
  var currentFileArchive = moduleLoader.getFileArchive();
  var currentFileManifest = moduleLoader.getManifest();
  var main = currentFileManifest.getMain();
  var code = currentFileArchive.getFileContents(main);

  // TODO: This is the name that will show up in stack traces. We should
  // resolve ./ to full paths (maybe).
  var fname = main;
  var urlParts = url.parse(main, true);
  var moduleName = urlParts.pathname;
  var uri = main;
  var basePath, jarName;
  if (isJar) {
    basePath = main.substring(0, main.lastIndexOf('/'));
    jarName = moduleName;
  } else {
    basePath = packageUri.substring(0, packageUri.lastIndexOf('/'));
  }

  var thisAppSceneContext = this;
  log.message(4, "runScriptInNewVMContext: create XModule(" + moduleName + ") basePath=" + basePath + " packageUri=" + packageUri);
  var xModule = new XModule(moduleName, this, basePath, jarName);
  this.topXModule = xModule;
  if( configImport !== null ) {
    xModule.configImport(configImport);
  }
  if (isJar) {
    this.jarFileMap.addArchive(xModule.name,currentFileArchive);
    log.message(4, "JAR added: " + xModule.name);
  }

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  var self = this;
  var newSandbox;
  try {
    if (!isDuk && !isV8) {
      var requireMethod = function (pkg) {
        if (typeof requireIt === "function") { 
          // TODO: remove
          log.message(1, "old use of require not supported: " + pkg);
          return requireIt(pkg);
        }
        else{
          log.message(1, "old use of require not supported: " + pkg);
        }
      };

      var requireFileOverridePath = process.env.PXSCENE_REQUIRE_ENABLE_FILE_PATH;
      var requireEnableFilePath = "/tmp/";
      if (process.env.HOME && process.env.HOME !== '') {
        requireEnableFilePath = process.env.HOME;
      }
      if (requireFileOverridePath && requireFileOverridePath !== '') {
        requireEnableFilePath = requireFileOverridePath;
      }

      var fs = require("fs");
      var requireEnableFile = requireEnableFilePath + "/.pxsceneEnableRequire";
      if (fs.existsSync(requireEnableFile)) {
        console.log("enabling pxscene require support");
        requireMethod = require;
      }
    }

    if (!isDuk && !isV8) {
      var processWrap = WrapObj(process, {"binding":function() { throw new Error("process.binding is not supported"); }}, false, processKeys);
      var globalWrap = WrapObj(global, {"process":processWrap, "console":console});

      // TODO: app runs in new context (vm.runInNewContext),
      //  while px (px.imports) is in parent context.
      //  Hence in imported module Function isn't the same object as Function in app,
      //  'instanceof Function' won't work.
      //  Propagating Function: Function here solves the problem only partially
      //  (not for lowercase 'function').

      newSandbox = {
        sandboxName: "InitialSandbox",
        console: console,
        theNamedContext: "Sandbox: " + uri,
        Buffer: Buffer,
        process: processWrap,
        require: requireMethod,
        global: globalWrap,
        //Function: Function,
        //Uint8Array: Uint8Array,
        setTimeout: function (callback, after, arg1, arg2, arg3) {
          //pass the timers list to callback function on timeout
          var timerId = SetTimeout(setTimeoutCallback, after, this.timers, function() { callback(arg1, arg2, arg3)});
          this.timers.push(timerId);
          return timerId;
        }.bind(this),
        clearTimeout: function (timer) {
          var index = this.timers.indexOf(timer);
          if (index != -1)
          {
            this.timers.splice(index,1);
          }
          ClearTimeout(timer);
        }.bind(this),
        setInterval: function (callback, repeat, arg1, arg2, arg3) {
          var intervalId = SetInterval(callback, repeat, arg1, arg2, arg3);
          this.timerIntervals.push(intervalId);
          return intervalId;
        }.bind(this),
        clearInterval: function (timer) {
          var index = this.timerIntervals.indexOf(timer);
          if (index != -1)
          {
            this.timerIntervals.splice(index,1);
          }
          ClearInterval(timer);
        }.bind(this),
        importTracking: {}
      };
    }
    else if (isV8) 
    {
      newSandbox = {
        sandboxName: "InitialSandbox",
        console: console,
        timers: timers,
        global: global,
        isV8: isV8,
        setTimeout: setTimeout,
        clearTimeout: clearTimeout,
        setInterval: setInterval,
        clearInterval: clearInterval,
        require: require,
        loadUrl: loadUrl,
        theNamedContext: "Sandbox: " + uri,
        //Buffer: Buffer,
        importTracking: {},
        print: print,
        getScene: getScene,
        makeReady: makeReady,
        getContextID: getContextID
      }; // end sandbox
    }
    else
    {
      newSandbox = {
        sandboxName: "InitialSandbox",
        console: console,
        theNamedContext: "Sandbox: " + uri,
        //Buffer: Buffer,
        importTracking: {}
      }; // end sandbox
    }

    // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

    xModule.initSandbox(newSandbox);
    thisAppSceneContext.sandbox = newSandbox; //xModule.sandbox;


    try {
      //JRJRJRJR  This line causing a garbage collection leak...
      // LEAKLEAK
//      this.innerscene.api = {isReady:false, onModuleReady:onAppModuleReady.bind(this) };

      var sourceCode = AppSceneContext.wrap(code);
      log.message(4, "createModule_pxScope.call()");
      var px = createModule_pxScope.call(this, xModule);
      log.message(4, "createModule_pxScope.call() done");

      var xModule_wrap = WrapObj(xModule, {
        appSceneContext: WrapObj(xModule.appSceneContext, {
          packageUrl: xModule.appSceneContext.packageUrl
        }, true, [
          'getPackageBaseFilePath',
          'getFile'
        ])
      }, true, [
        'exports'
      ]);

      if (isDuk) {
        vm.runInNewContext(sourceCode, newSandbox, {
          filename: path.normalize(fname),
          displayErrors: true
        }, px, xModule_wrap, fname, this.basePackageUri);
      } else if (isV8) {
        var moduleFunc = vm.runInNewContext(sourceCode, newSandbox, {
          filename: path.normalize(fname),
          displayErrors: true
        }, px, xModule_wrap, fname, this.basePackageUri);

        moduleFunc(px, xModule_wrap, fname, this.basePackageUri);

      } else {
        // Set breakpoint on module start
        if (process.env.BREAK_ON_SCRIPTSTART == 1 && (packageUri.indexOf("shell.js") == -1))
        {
          sourceCode = "debugger;\n" + sourceCode;
        }
        var moduleFunc = vm.runInNewContext(sourceCode, newSandbox, {
          filename: path.normalize(fname),
          displayErrors: true
        });
        moduleFunc(px, xModule_wrap, fname, this.basePackageUri);
      }
      log.message(4, "vm.runInNewContext done");

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
        //console.log("Main module[" + self.packageUrl + "] about to notify. xModule.exports:"+(typeof xModule.exports));
        self.innerscene.api = xModule.exports;
        this.makeReady(true, xModule.exports);
        //console.log("Main module[" + self.packageUrl + "] about to notify done");
      } else {
        xModule.moduleReadyPromise.then( function() {
          //console.log("Main module[" + self.packageUrl + "] about to notify. xModule.exports:"+(typeof xModule.exports));
          self.innerscene.api = xModule.exports;
          self.makeReady(true, xModule.exports);
          //console.log("Main module[" + self.packageUrl + "] about to notify done");
        }).catch( function(err) {
          console.error("Main module[" + self.packageUrl + "]" + " load has failed - on failed imports: " + ", err=" + err);
          self.makeReady(false, {});
        });
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
};

AppSceneContext.prototype.getPackageBaseFilePath = function() {
  return this.basePackageUri.replace('%20', '\ '); // replace HTML escaped spaces with C/C++ escaping
};

AppSceneContext.prototype.getModuleFile = function(filePath, xModule) {
  var promise = this.jarFileMap.getArchiveFileAsync(xModule.getJarName(), filePath);
  if (promise) {
    log.message(4, "Found file '" + filePath+"' in JAR: "+xModule.getJarName());
    return promise;
  }
  var resolvedModulePath;
  resolvedModulePath = this.resolveModulePath(filePath, xModule);
  log.message(3, "TJC: getModuleFile("+filePath + "): resolves to " + resolvedModulePath.fileUri);
  return this.getFile(resolvedModulePath.fileUri);
};

AppSceneContext.prototype.getFile = function(filePath) {
  log.message(4, "getFile: requestedFile=" + filePath);
  if ("true" === this.disableFilePermissionCheck || true === this.disableFilePermissionCheck) {
    return loadFile(filePath);
  }
  return loadFile(filePath, this);
};

AppSceneContext.prototype.resolveModulePath = function(filePath, currentXModule) {
  var replacementMatch = currentXModule.findImportReplacementMatch(filePath);
  if( replacementMatch === null && this.topXModule !== currentXModule) {
    replacementMatch = this.topXModule.findImportReplacementMatch(filePath);
  }
  if( replacementMatch !== null ) {
    log.info(filePath + " ==> " + replacementMatch.fileUri);
    return replacementMatch;
  }
  var fileUri;
  if (filePath.charAt(0) == '/') {
    // temporary for now
    fileUri = filePath.substring(1);
    if (this.basePackageUri) {
      fileUri = url.resolve(this.basePackageUri + "/", fileUri);
    }
  } else {
    // relative to current module's folder
    fileUri = filePath;
    if (currentXModule.getBasePath()) {
      fileUri = url.resolve(currentXModule.getBasePath() + "/", fileUri);
    }
  }
  return {fileUri:fileUri};
};

AppSceneContext.prototype.include = function(filePath, currentXModule) {
  log.message(4, ">>> include(" + filePath + ") for " + currentXModule.name + " <<<");
  var _this = this;
  var origFilePath = filePath;

  return new Promise(function (onImportComplete, reject) {
    if (/^(px|url|querystring|htmlparser|crypto|oauth|grpc|google-protobuf|thunderJS)$/.test(filePath)) {
      if (isDuk && filePath === 'htmlparser') {
        console.log("Not permitted to use the module " + filePath);
        reject("include failed due to module not permitted");
        return;
      }
      // built-ins
      var moduleName = filePath;
      if (filePath === 'grpc')
        moduleName = '@grpc/grpc-js';
      var modData = require(moduleName);
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath === 'fs' || filePath === 'os' || filePath === 'events') {
      console.log("Not permitted to use the module " + filePath);
      reject("include failed due to module not permitted");
      return;
    } else if(filePath === 'ws') {
      if (isDuk) {
        console.log("creating websocket instance")
        modData = websocket;
        onImportComplete([modData, origFilePath]);
        return;
      } else {
        var wsdata = require('rcvrcore/ws_wrap');
        _this.webSocketManager = new wsdata();
        var WebSocket = (function() {
          var context = this;
          function WebSocket(address, protocol, options) {
            var client = context.webSocketManager.WebSocket(address, protocol, options);
            return client;
          }
          return WebSocket;
         }.bind(_this))();
        modData = WebSocket;
        onImportComplete([modData, origFilePath]);
        return;
      }
    } else if(/^(http|https|http2)$/.test(filePath)) {
      modData = new http_wrap(filePath, _this);
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath.substring(0, 9) === "px:scene.") {
      var Scene = require('rcvrcore/' + filePath.substring(3));
      if( _this.sceneWrapper === null ) {
        _this.sceneWrapper = new Scene();
      }
      _this.sceneWrapper._setNativeScene(_this.innerscene, currentXModule.name);
      _this.sceneWrapper._setRPCController(_this.rpcController);
      onImportComplete([_this.sceneWrapper, origFilePath]);
      return;
    } else if( filePath.substring(0,9) === "px:tools.") {
      modData = require('rcvrcore/tools/' + filePath.substring(9));
      onImportComplete([modData, origFilePath]);
      return;
    }
    else if( filePath.substring(0,7) === "optimus") {
      modData = require('rcvrcore/optimus.js');
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath.substring(0, 7) === "thunder") {
      var thunder = require('rcvrcore/thunder.js');
      if( _this.thunderWrapper === null ) {
        _this.thunderWrapper = new thunder();
      }
      _this.thunderWrapper._setScene(_this.innerscene);
      onImportComplete([_this.thunderWrapper, origFilePath]);
      return;
    }

    filePath = _this.resolveModulePath(filePath, currentXModule).fileUri;

    log.message(4, "filePath=" + filePath);
    if( _this.isScriptDownloading(filePath) ) {
      log.message(4, "Script is downloading for " + filePath);
      _this.addModuleReadyListener(filePath, function(moduleExports) {
        onImportComplete([moduleExports, origFilePath]);
      });
      return;
    }
    if (_this.isScriptLoaded(filePath)) {
      log.message(4, "Already have file loaded and ready, just return the existing content: " + filePath);
      modData = _this.getScriptContents(filePath);
      onImportComplete([modData, origFilePath]);
      return;
    }

    _this.setScriptStatus(filePath, 'downloading');

    var file = _this.jarFileMap.getArchiveFile(currentXModule.getJarName(), filePath);
    if (file) {
      // FIXME: no support for jars in jar because nativeFileArchive uses getFileAsString which is not ok for jar
      log.message(4, "Found file '" + filePath+"' in JAR: "+currentXModule.getJarName());
      var moduleLoader = new SceneModuleLoader();
      moduleLoader.processFileData(filePath, file);
      moduleLoader.loadedJarFile = false;
      moduleLoader.manifest = new SceneModuleManifest();
      moduleLoader.manifest.loadFromJSON(moduleLoader.getFileArchive().getFileContents('package.json'));
      _this.processCodeBuffer(origFilePath, filePath, currentXModule, moduleLoader, onImportComplete, reject);
      return;
    }

    _this.asyncFileAcquisition.acquire(filePath)
      .then(function(moduleLoader){
        log.message(4, "PROCESS RCVD MODULE: " + filePath);
        // file acquired
        _this.processCodeBuffer(origFilePath, filePath, currentXModule, moduleLoader, onImportComplete, reject);
      }).catch(function(err){
        console.error("Error: could not load file ", filePath, ", err=", err);
        reject("include failed");
      });
  });
};

AppSceneContext.prototype.processCodeBuffer = function(origFilePath, filePath, currentXModule, moduleLoader, onImportComplete, onImportRejected) {
  var _this = this;
  if( _this.isScriptReady(filePath) ) {
    var modExports = _this.getScriptContents(filePath);
    onImportComplete([modExports, origFilePath]);
    return;
  } else if( _this.isScriptLoaded((filePath))) {
    log.message(4, "It looks like module script is already loaded -- no need to run it");
    _this.addModuleReadyListener(filePath, function(moduleExports) {
      log.message(7, "Received moduleExports from other download" );
      onImportComplete([moduleExports, origFilePath]);
    });
    return;
  }

  log.message(4, "Need to run script: " + filePath);

  // FIXME: XModule names are not unique
  var xModule = this.getXModule(filePath);
  if( xModule !== 'undefined' ) {
    log.message(4, "xModule already exists: " + filePath);
    return;
  }

  var isJar = moduleLoader.jarFileWasLoaded();
  var currentFileArchive = moduleLoader.getFileArchive();
  var currentFileManifest = moduleLoader.getManifest();
  var main = currentFileManifest.getMain();
  var codeBuffer = currentFileArchive.getFileContents(main);
  var basePath, jarName;
  if (isJar) {
    basePath = main.substring(0, main.lastIndexOf('/'));
    jarName = filePath;
  } else {
    basePath = filePath.substring(0, filePath.lastIndexOf('/'));
    jarName = currentXModule.getJarName();
  }

  log.message(7, "cb Creating new XModule for " + filePath + " basePath="+basePath);
  xModule = new XModule(filePath, _this, basePath, jarName);
  xModule.initSandbox(_this.sandbox);

  if (isJar) {
    _this.jarFileMap.addArchive(xModule.name,currentFileArchive);
    log.message(4, "JAR added: " + xModule.name);
  }

  var sourceCode = AppSceneContext.wrap(codeBuffer);
  log.message(4, "RUN " + filePath);
  var px = createModule_pxScope.call(this, xModule, true);

  var xModule_wrap = WrapObj(xModule, {
    appSceneContext: WrapObj(xModule.appSceneContext, {
      packageUrl: xModule.appSceneContext.packageUrl
    }, true, [
      'getPackageBaseFilePath',
      'getFile'
    ])
  }, true, [
    'exports'
  ]);

  if (isDuk) {
    vm.runInNewContext(sourceCode, _this.sandbox, { filename: filePath, displayErrors: true },
                         px, xModule_wrap, filePath, filePath);
  } else if (isV8) {
    var moduleFunc = vm.runInNewContext(sourceCode, _this.sandbox, { filename: filePath, displayErrors: true },
                         px, xModule_wrap, filePath, filePath);

    moduleFunc(px, xModule_wrap, filePath, filePath);
  } else {
    var moduleFunc = vm.runInContext(sourceCode, _this.sandbox, {filename:filePath, displayErrors:true});
    moduleFunc(px, xModule_wrap, filePath, filePath);
  }
  log.message(4, "RUN DONE: " + filePath);
  this.setXModule(filePath, xModule);

  // Set up a async wait until module indicates it's completly ready
  if( !xModule.moduleReadyPromise ) {
    // No use of px.import or it's possible that these exports have already been added
    log.message(4, "["+xModule.name+"]: <" + filePath + "> MODULE INDICATES IT'S FULLY READY. xModule.exports:" + (typeof xModule.exports));
    _this.addScript(filePath, 'ready', xModule.exports);
    log.message(4, "is about to notify [" + currentXModule.name + "] that <" + filePath + "> has been imported and is ready");
    onImportComplete([xModule.exports, origFilePath]);
    log.message(4, "after notifying [:" + currentXModule.name + "] about import <" + filePath + ">");
    _this.callModuleReadyListeners(filePath, xModule.exports);
  } else {
    // Now wait for module to indicate that it's fully ready to go
    xModule.moduleReadyPromise.then(function () {
      log.message(4, "["+xModule.name+"]: <" + filePath + "> MODULE INDICATES IT'S FULLY READY. xModule.exports:" + (typeof xModule.exports));
      _this.addScript(filePath, 'loaded', xModule.exports);
      _this.setScriptStatus(filePath, 'ready');
      log.message(4, "is about to notify [" + currentXModule.name + "] that <" + filePath + "> has been imported and is ready");
      onImportComplete([xModule.exports, origFilePath]);
      log.message(4, "after notifying [:" + currentXModule.name + "] about import <" + filePath + ">");
      _this.callModuleReadyListeners(filePath, xModule.exports);
    }).catch(function (error) {
      onImportRejected("include(2): failed while waiting for module <" + filePath + "> to be ready for [" + currentXModule.name + "] - error=" + error);
    });
  }
};

AppSceneContext.prototype.onResize = function(resizeEvent) {
  var hrTime = isDuk?uv.hrtime():(isV8?uv_hrtime():process.hrtime(this.lastHrTime));
  var deltaMillis = (hrTime[0] * 1000 + hrTime[1] / 1000000);
  this.lastHrTime = isDuk?uv.hrtime():(isV8?uv_hrtime():process.hrtime());
  if( deltaMillis > 300 ) {
    if( this.resizeTimer !== null ) {
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
};

AppSceneContext.prototype.addModuleReadyListener = function(moduleName, callback) {
  if( this.scriptMap.hasOwnProperty(moduleName) ) {
    this.scriptMap[moduleName].readyListeners.push(callback);
  } else {
    console.trace('AppSceneContext#addModuleReadyListener: no entry in map for module [' + moduleName + ']');
  }
};

AppSceneContext.prototype.callModuleReadyListeners = function(moduleName, moduleExports) {
  log.message(4, "Call ModuleReadyListeners for module: " + moduleName);
  if( this.scriptMap.hasOwnProperty(moduleName) ) {
    var listeners = this.scriptMap[moduleName].readyListeners;
    if( listeners !== null && listeners.length !== 0 ) {
      for(var k = 0; k < listeners.length; ++k) {
        listeners[k](moduleExports);
      }
      for(var k = 0; k < listeners.length; ++k) {
        listeners.pop();
      }
    }
    this.scriptMap[moduleName].readyListeners = null;
  } else {
    console.trace('AppSceneContext#callModuleReadyListeners: no entry in map for module [' + moduleName + ']');
  }
};

AppSceneContext.prototype.setXModule = function(name, xmod) {
  this.xmoduleMap[name] = xmod;
};

AppSceneContext.prototype.getXModule = function(name) {
  if( this.xmoduleMap.hasOwnProperty(name) ) {
    return this.xmoduleMap[name].xmod;
  }

  return 'undefined';
};

AppSceneContext.prototype.addScript = function(name, status, scriptObject) {
    if( this.scriptMap.hasOwnProperty(name) ) {
    var curData = this.scriptMap[name];
    curData.status = status;
    if( status == 'ready' && (typeof scriptObject == 'undefined' || scriptObject === null) ) {
      console.trace("Whoa: seting Ready state but there is no scriptObject");
    }
    if( scriptObject !== null && scriptObject !== 'undefined' ) {
      curData.scriptObject = scriptObject;
      log.message(4, "ADDED UPDATED script: " + name + ", status=" + status);
    }

    var oldScriptObject = this.scriptMap[name].scriptObject;
    if( oldScriptObject === null && scriptObject !== null ) {
      console.trace("Script object changing from null, but isn't being set");
    }

    log.message(8, "AddScript " + name + ", status=" + status + ", object=" + typeof(scriptObject));

  } else {
    this.scriptMap[name] = {status: status, scriptObject: scriptObject, readyListeners:[]};
    log.message(4, "ADDED NEW script: " + name + ", status=" + status);
  }
};

AppSceneContext.prototype.getScriptContents = function(name) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    return this.scriptMap[name].scriptObject;
  } else {
    return null;
  }
};

AppSceneContext.prototype.setScriptStatus = function(name, status) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    this.scriptMap[name].status = status;
    var scriptObject = this.scriptMap[name].scriptObject;
    if( status == 'ready' && (typeof scriptObject == 'undefined' || scriptObject === null) ) {
      console.trace("Whoa: seting Ready state but there is no scriptObject");
    }

    log.message(8, "1) SetScriptStatus " + name + ", status=" + status + ", object=" + typeof(scriptObject));

  } else {
    log.message(8, "0) SetScriptStatus " + name + ", status=" + status + ", null");
    this.addScript(name, status, null);
  }
};

AppSceneContext.prototype.getScriptStatus = function(name) {
  if( this.scriptMap.hasOwnProperty(name) ) {
    return this.scriptMap[name].status;
  }

  return 'undefined';
};

AppSceneContext.prototype.isScriptDownloading = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && this.scriptMap[name].status === "downloading" ) {
    log.message(4, "isScriptDownloading(" + name + ")? Yes");
    return true;
  }

  log.message(4, "isScriptDownloading(" + name + ")? NOT DOWNLOADED YET");
  return false;
};

AppSceneContext.prototype.isScriptLoaded = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && (this.scriptMap[name].status === "loaded" || this.scriptMap[name].status === "ready") ) {
    log.message(4, "isScriptLoaded(" + name + ")? Yes");
    return true;
  }

  log.message(4, "isScriptLoaded(" + name + ")? NOT LOADED YET");
  return false;
};

AppSceneContext.prototype.isScriptReady = function(name) {
  if( this.scriptMap.hasOwnProperty(name) && this.scriptMap[name].status === "ready" ) {
    log.message(4, "isScriptReady(" + name + ")?  Yes it's READY");
    return true;
  }

  log.message(4, "isScriptReady(" + name + ")?  NOT READY YET");
  return false;
};

AppSceneContext.wrap = function(script) {
  return AppSceneContext.wrapper[0] + script + AppSceneContext.wrapper[1];
};

AppSceneContext.wrapper = [
  '(function (px, module, __filename, __dirname) { ',
  '\n});'
];

module.exports = AppSceneContext;
