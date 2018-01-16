//"use strict";

var isDuk=(typeof timers != "undefined")?true:false;

var url = require('url');
var path = require('path');
var vm = require('vm');
var Logger = require('rcvrcore/Logger').Logger;
var SceneModuleLoader = require('rcvrcore/SceneModuleLoader');
var XModule = require('rcvrcore/XModule').XModule;
var xmodImportModule = require('rcvrcore/XModule').importModule;
var loadFile = require('rcvrcore/utils/FileUtils').loadFile;
var SceneModuleManifest = require('rcvrcore/SceneModuleManifest');
var JarFileMap = require('rcvrcore/utils/JarFileMap');
var AsyncFileAcquisition = require('rcvrcore/utils/AsyncFileAcquisition');

var log = new Logger('AppSceneContext');
//overriding original timeout and interval functions
var SetTimeout = isDuk?timers.setTimeout:setTimeout;
var ClearTimeout = isDuk?timers.clearTimeout:clearTimeout;
var SetInterval = isDuk?timers.setInterval:setInterval;
var ClearInterval = isDuk?timers.clearInterval:clearInterval;

var http_wrap = require('rcvrcore/http_wrap');
var https_wrap = require('rcvrcore/https_wrap');

function AppSceneContext(params) {

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
  this.sandbox = {};
  this.scriptMap = {};
  this.xmoduleMap = {};
  this.asyncFileAcquisition = new AsyncFileAcquisition(params.scene);
  this.lastHrTime = isDuk?uv.hrtime():process.hrtime();
  this.resizeTimer = null;
  this.topXModule = null;
  this.jarFileMap = new JarFileMap();
  this.sceneWrapper = null;
  //array to store the list of pending timers
  this.timers = [];
  this.timerIntervals = [];

  log.message(4, "[[[NEW AppSceneContext]]]: " + this.packageUrl);
}

AppSceneContext.prototype.loadScene = function() {
  //log.info("loadScene() - begins    on ctx: " + getContextID() );
  var urlParts = url.parse(this.packageUrl, true);
  var fullPath = this.packageUrl;
  var platform = (isDuk)?uv.platform:process.platform;
  if (fullPath.substring(0, 4) !== "http") {
    if( fullPath.charAt(0) === '.' ) {
      // local file system
      this.defaultBaseUri = ".";
    } else if( platform === 'win32' && fullPath.charAt(1) === ':' ) {
        // Windows OS, so take the url as the whole file path
        urlParts.pathname = this.packageUrl;
    }
    fullPath = urlParts.pathname;
    if( fullPath !== null) {
      this.basePackageUri = fullPath.substring(0, fullPath.lastIndexOf('/'));
      //var fileName = this.packageUrl.substring(fullPath.lastIndexOf('/'));
    }
  } else {
    this.basePackageUri = fullPath.substring(0, fullPath.lastIndexOf('/'));
  }

if( fullPath !== null)
  this.loadPackage(fullPath);

this.innerscene.on('onSceneTerminate', function (e) {
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

    this.innerscene.api = null;
    this.innerscene = null;
    this.sandbox.xmodule = null;
    this.sandbox.require = null;
    this.sandbox.sandboxName = null;
    this.sandbox.runtime = null;
    this.sandbox.theNamedContext = null;
    this.sandbox.Buffer = null;
    this.sandbox.setTimeout = null;
    this.sandbox.setInterval = null;
    this.sandbox.clearTimeout = null;
    this.sandbox.clearInterval = null;
    this.sandbox.importTracking = {};
    this.sandbox = null;
    this.scriptMap = null;
    for(var xmodule in this.xmoduleMap) {
      this.xmoduleMap[xmodule].freeResources();
    }
    this.xmoduleMap = null;
    this.topXModule = null;
    this.jarFileMap = null;
    for(var key in this.scriptMap) {
      this.scriptMap[key].scriptObject = null;
      this.scriptMap[key].readyListeners = null;
    }
    this.scriptMap = null;
    this.sceneWrapper = null;
  }.bind(this));

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
          console.info("AppSceneContext#loadScenePackage0");
          _this.runScriptInNewVMContext(packageUri, moduleLoader, manifest.getConfigImport());
          console.info("AppSceneContext#loadScenePackage0 done");
        }).catch(function (e) {
            console.info("AppSceneContext#loadScenePackage1");
            _this.runScriptInNewVMContext(packageUri, moduleLoader, null);
            console.info("AppSceneContext#loadScenePackage1 done");
        });
      } else {
        var manifest = moduleLoader.getManifest();
        console.info("AppSceneContext#loadScenePackage2");
        _this.runScriptInNewVMContext(packageUri, moduleLoader, manifest.getConfigImport());
        console.info("AppSceneContext#loadScenePackage2 done");
      }
    })
    .catch(function (err) {
      console.info("AppSceneContext#loadScenePackage3");
      thisMakeReady(false, {});
      console.error("AppSceneContext#loadScenePackage: Error: Did not load fileArchive: Error=" + err );
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

function createModule_pxScope(xModule) {
  return {
    log: xModule.log,
    import: xmodImportModule.bind(xModule),
    configImport: xModule.configImport.bind(xModule),
    resolveFilePath: xModule.resolveFilePath.bind(xModule),
    appQueryParams: this.queryParams,
    getPackageBaseFilePath: getPackageBaseFilePath.bind(this),
    getFile: getFile.bind(this),
    getModuleFile: xModule.getFile.bind(xModule)
  };
}


//duktape merge hack

if (isDuk) {
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
            newSandbox = {
            sandboxName: "InitialSandbox",
            xmodule: xModule,
            console: console,
            runtime: apiForChild,
            urlModule: require("url"),
            queryStringModule: require("querystring"),
            theNamedContext: "Sandbox: " + uri,
            Buffer: Buffer,
            importTracking: {}
            }; // end sandbox
            
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
                vm.runInNewContext(sourceCode, newSandbox, { filename: path.normalize(fname), displayErrors: true },
                                   px, xModule, fname, this.basePackageUri);
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
                
                console.log("Main Module: readyPromise=" + xModule.moduleReadyPromise);
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
                                       
                                       console.log("Main module[" + self.packageUrl + "] about to notify");
                                       thisMakeReady(true, xModule.exports);
                                       console.log("Main module[" + self.packageUrl + "] about to notify done");
                                       
                                       }).catch( function(err)
                                                {
                                                console.error("Main module[" + self.packageUrl + "]" + " load has failed - on failed imports: " + ", err=" + err);
                                                thisMakeReady(false,{});
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
    };
    
}
else {
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
    var requireMethod = function (pkg) {
      log.message(3, "old use of require not supported: " + pkg);
      // TODO: remove
      return requireIt(pkg);
    };

    var requireFileOverridePath = process.env.PXSCENE_REQUIRE_ENABLE_FILE_PATH;
    var requireEnableFilePath = "/tmp/";
    if (process.env.HOME && process.env.HOME !== '') {
      requireEnableFilePath = process.env.HOME;
    }
    if (requireFileOverridePath && requireFileOverridePath !== ''){
      requireEnableFilePath = requireFileOverridePath;
    }

    var fs = require("fs");
    var requireEnableFile = requireEnableFilePath + "/.pxsceneEnableRequire";
    if (fs.existsSync(requireEnableFile)) {
      console.log("enabling pxscene require support");
      requireMethod = require;
    }
    
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
      require: requireMethod,
      global: global,
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
    }; // end sandbox

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
      // fix debug under windows issue
      var moduleFunc = vm.runInNewContext(sourceCode, newSandbox, {filename:path.normalize(fname), displayErrors:true});

      if (process._debugWaitConnect) {
        // Set breakpoint on module start
        if (process.env.BREAK_ON_SCRIPTSTART != 1)
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
          thisMakeReady(false,{});
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
};
}

AppSceneContext.prototype.getPackageBaseFilePath = function() {
  var fullPath;
  var pkgPart;
  var platform = (isDuk)?uv.platform:process.platform;
  if (this.basePackageUri.substring(0, 4) !== "http") {
    if (this.basePackageUri.charAt(0) == '.') {
      pkgPart = this.basePackageUri.substring(1);
    } else {
      pkgPart = this.basePackageUri;
    }
    if (pkgPart.charAt(0) == '/') {
      fullPath = this.defaultBaseUri + pkgPart;
    } else if(platform === 'win32' && pkgPart.charAt(1) === ':' ) {
      // Windows OS and using drive name, take the pkg part as the file path
      fullPath = pkgPart;   
    } else {
      fullPath = this.defaultBaseUri + "/" + pkgPart;
    }
  } else {
    fullPath = this.basePackageUri;
  }

  return fullPath;
};

function getPackageBaseFilePath() {
  return this.getPackageBaseFilePath();
}

function getFile(filePath) {
  return this.getFile(filePath);
}

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
  return loadFile(filePath);
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

// duktape merge hack

if (isDuk) {
    AppSceneContext.prototype.include = function(filePath, currentXModule) {
        log.message(4, ">>> include(" + filePath + ") for " + currentXModule.name + " <<<");
        var _this = this;
        var origFilePath = filePath;
        
        return new Promise(function (onImportComplete, reject) {
                           if( filePath === 'px' || filePath === 'url' || filePath === 'querystring') {
                           // built-ins
                           var modData = require(filePath);
                           onImportComplete([modData, origFilePath]);
                           return;
                           } else if( filePath === 'fs' || filePath === 'os' || filePath === 'events') {
                           console.log("Not permitted to use the module " + filePath);
                           reject("include failed due to module not permitted");
                           return;
                           } else if( filePath === 'net' /*|| filePath === 'ws'*/ ||  filePath === 'htmlparser') {
                           //modData = require('rcvrcore/' + filePath + '_wrap');
                           //onImportComplete([modData, origFilePath]);
                           console.log("Not permitted to use the module " + filePath);
                           reject("include failed due to module not permitted");
                           return;
                           } 
                           else if (filePath === 'ws') {
                            console.log("creating websocket instance")
                            modData = websocket;
                            onImportComplete([modData, origFilePath]);
                            return;
                           } else if (filePath === 'http' || filePath === 'https') {
                           //console.log("Not permitted to use the module " + filePath);
                           //reject("include failed due to module not permitted");
                           if (filePath === 'http')
                           {
                           modData = new http_wrap();
                           }
                           else
                           {
                           modData = new https_wrap();
                           }
                           var localapp = (isLocalApp(_this.packageUrl) || isLocalIPV6App(_this.packageUrl));
                           modData.setLocalApp(localapp);
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
                           
                           //console.log("this path is temporarily disabled by akuts");
                           //reject("this path is temporarily disabled by akuts");
                           
                           
                           filePath = _this.resolveModulePath(filePath, currentXModule).fileUri;
                           
                           console.log("this path is by akuts: " + filePath);
                           
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
                                          console.error("Error: could not load file " + filePath  + ", err=" + err);
                                          reject("include failed");
                                          });
                           });
    };
    
    
}
else {
AppSceneContext.prototype.include = function(filePath, currentXModule) {
  log.message(4, ">>> include(" + filePath + ") for " + currentXModule.name + " <<<");
  var _this = this;
  var origFilePath = filePath;

  return new Promise(function (onImportComplete, reject) {
    if( filePath === 'px' || filePath === 'url' || filePath === 'querystring' || filePath === 'htmlparser') {
      // built-ins
      var modData = require(filePath);
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath === 'fs' || filePath === 'os' || filePath === 'events') {
      console.log("Not permitted to use the module " + filePath);
      reject("include failed due to module not permitted");
      return;
    } else if( filePath === 'net' || filePath === 'ws' ) {
      modData = require('rcvrcore/' + filePath + '_wrap');
      onImportComplete([modData, origFilePath]);
      return;
    } else if( filePath === 'http' || filePath === 'https' ) {
      modData = filePath === 'http' ? new http_wrap(_this.innerscene) : new https_wrap(_this.innerscene);
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
        console.error("Error: could not load file " + filePath  + ", err=" + err);
        reject("include failed");
      });
  });
};
}

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
  var px = createModule_pxScope.call(this, xModule);
  if (isDuk) {
    vm.runInNewContext(sourceCode, _this.sandbox, { filename: filePath, displayErrors: true },
                         px, xModule, filePath, filePath);
  }
  else {
    var moduleFunc = vm.runInContext(sourceCode, _this.sandbox, {filename:filePath, displayErrors:true});
    moduleFunc(px, xModule, filePath, filePath);
  }
  log.message(4, "RUN DONE: " + filePath);
  this.setXModule(filePath, xModule);

  // Set up a async wait until module indicates it's completly ready
  var modReadyPromise = xModule.moduleReadyPromise;
  if( modReadyPromise === null ) {
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
};

AppSceneContext.prototype.onResize = function(resizeEvent) {
  var hrTime = isDuk?uv.hrtime():process.hrtime(this.lastHrTime);
  var deltaMillis = (hrTime[0] * 1000 + hrTime[1] / 1000000);
  this.lastHrTime = isDuk?uv.hrtime():process.hrtime();
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
