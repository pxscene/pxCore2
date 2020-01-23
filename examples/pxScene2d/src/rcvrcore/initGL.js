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

// TODO... 
// * Xuse passed in url/file
// * Xfix xsetInterval and xclearInterval
// * Xbug when having a working gl scene and load non-existent gl scene
// * load nonexistent gl scene blue context??
// * Xmultiple loadings of gl scenes
// * requestAnimationTimer
// * fix deletionn of gl context (swith to MIke's new api)
// * multiple opengl scenes at same time
// * key events, onSize etc... 
// * view events... event delegation?? to scene
// * why do I need to pass in beginDrawing and endDraving
// NOTE TO self... declarations with no "var" are in global namespace... danger
// are setInterval and clearInterval polluting global namespace... ?
// /Users/johnrobinson/code/pxgl/examples/pxScene2d/external/spark-webgl/examples/2-triangle/triangle.js

var _timers = require('timers')
var fs = require('fs')
var path = require('path')
var vm = require('vm')
var _module = require('module')
var _ws = require('ws')
var _http = require('http')
var _https = require('https')
var urlmain = require("url")
var console = require('console_wrap')
var process = require('process')
var ESMLoader = require('rcvrcore/ESMLoader')
var _sparkkeys = require('rcvrcore/tools/keys.js')
var reqOrig = require
var {promisify} = require('util')
var Buffer = require('buffer').Buffer
var cachedSource = {}

var sandboxKeys = ["vm", "process", "setTimeout", "console", "clearTimeout", "setInterval", "clearInterval", "setImmediate", "clearImmediate", "sparkview", "sparkscene", "sparkgles2", "beginDrawing", "endDrawing", "sparkwebgl", "sparkkeys", "sparkQueryParams", "require", "localStorage", "sparkHttp"]
var __dirname = process.cwd()

// Spark node-like module loader
const makeRequire = function(pathToParent) {
  return function(moduleName) {
    // TODO: if code runs via "runInContext(... sandbox", then required module runs in sandbox,
    //  but if via "vm.SourceTextModule(... sandbox", then required module runs in global context.
    if ((moduleName == 'iconv-lite') || (moduleName == 'safer-buffer') || (moduleName == 'is-stream'))
    {
      return reqOrig(moduleName)
    }
    const parentDir = path.dirname(pathToParent);
    // use Node's built-in module resolver here, but we could easily pass in our own
    var resolvedModule = _module._resolveLookupPaths(moduleName, {paths:[parentDir],id:pathToParent,filename:pathToParent});
    var id = resolvedModule[0];
    var paths = resolvedModule[1];
    var isLocalFile = moduleName.indexOf("http")==-1?true:false;
    const filename = (isLocalFile)?_module._resolveFilename(moduleName, {paths:[parentDir].concat(_module._nodeModulePaths(parentDir)),id:pathToParent,filename:pathToParent}):moduleName;
    // Spark Modules should be loaded a "singleton" per scene
    // If we've already loaded a module then return it's cached exports
    if (filename in this.bootStrapCache) {
      return this.bootStrapCache[filename]
    }

    // JRJR Hack to handle native modules give we don't have
    // access to NativeModule.require here.
    if (filename.indexOf('.node') != -1 || paths.length == 0 || filename.indexOf('package.json') != -1) {
      var m = _module._load(filename, {paths:[parentDir],id:pathToParent,filename:pathToParent})
      
      // Cache the module exports
      this.bootStrapCache[filename] = m
      return m
    }

    var source = "";
    if (isLocalFile) {
      var cachedData = false
      if (filename.indexOf('webgl.js') != -1) 
      {
        if (undefined == cachedSource['webgl.js'])
          cachedSource['webgl.js'] = fs.readFileSync('webgl.js', 'utf-8');
        source = cachedSource['webgl.js']
        cachedData = true
      }
      if (filename.indexOf('gles2.js') != -1)
      {
        if (undefined == cachedSource['gles2.js'])
          cachedSource['gles2.js'] = fs.readFileSync('gles2.js', 'utf-8');
        source = cachedSource['gles2.js']
        cachedData = true
      }
      if (false == cachedData) {    
        if (/^file:/.test(filename)) {
          source = fs.readFileSync(new urlmain.URL(filename), 'utf-8');
        } else {
          source = fs.readFileSync(filename, 'utf-8');
        }
     }
    } else {
      throw 'cannot require remote modules!';
    }
    // OUR own require, independent of node require
    const require = makeRequire(filename).bind(this);
    const wrapped = `(function(exports,require,module,__filename,__dirname) {${source}})`;
    let compiled = vm.runInContext(wrapped, this.contextifiedSandbox , {filename:filename,displayErrors:true})
    const exports = {};
    const module = {exports};
    try {
        compiled.call(exports, exports, require, module, filename, path.dirname(filename));
    }
    catch(e) {
      console.log(e);
    }

    this.bootStrapCache[filename] = module.exports
    return module.exports;
  }
};

var xxsetInterval = function(f,i){
  var rest = Array.from(arguments).slice(2)
  var interval = _timers.setInterval(function() {
    return function() {
        try {
        this.global.beginDrawing();
        f.apply(null,rest);
        this.global.endDrawing(); }
         catch(e) {
          console.log("exception during draw in setInterval !!");
        }}.bind(this)
    }.bind(this)(),i)
  this._intervals.push(interval)
  return interval
}

var xxclearInterval = function(interval) {
  var index = this._intervals.indexOf(interval);
  if (index > -1) {
    this._intervals.splice(index, 1);
  }
  _timers.clearInterval(interval)
}

var xxsetTimeout = function(f,t){
  var rest = Array.from(arguments).slice(2)
  var timeout = _timers.setTimeout(function() {
      return function() {
        try {
          this.global.beginDrawing();
          f.apply(null,rest);
          this.global.endDrawing();
        } catch(e) {
          console.log(e);
          console.log("exception during draw in setTimeout !!");
        }
        //console.log('after end Drawing2')
        var index = this._timeouts.indexOf(timeout)
        if (index > -1) {
          this._timeouts.splice(index,1)
        }
      }.bind(this)
      }.bind(this)(), t)
  this._timeouts.push(timeout)
  return timeout
}

var xxclearTimeout = function(timeout) {
  var index = this._timeouts.indexOf(timeout);
  if (index > -1) {
    this._timeouts.splice(index, 1);
  }
  _timers.clearTimeout(timeout)
}

var xxsetImmediate = function(f){
  var rest = Array.from(arguments).slice(1)
  var timeout = _timers.setImmediate(function() {
      return function() {
        //console.log('before beginDrawing3')
        try {
          if (this.active) this.global.beginDrawing();
          f.apply(null,rest)
          if (this.active) this.global.endDrawing();
        } catch(e) {
          console.log(e);
          console.log("exception during draw in setImmediate !!");
        }
        //console.log('after end Drawing3')
        var index = this._immediates.indexOf(timeout)
        if (index > -1) {
          this._immediates.splice(index,1)
        }
      }.bind(this)
      }.bind(this)())
  this._immediates.push(timeout)
  return timeout
}


var xxclearImmediate = function(immediate) {
  var index = this._immediates.indexOf(immediate);
  if (index > -1) {
    this._immediates.splice(index, 1);
  }
  _timers.clearTimeout(immediate)
}

function _clearIntervals () {
  if (this._intervals.length) {
    console.log(`clear ${this._intervals.length} intervals`);

    for (var interval of this._intervals) {
      _timers.clearInterval(interval)
    }
  }
  this._intervals = []
}

function _clearTimeouts () {
  if (this._timeouts.length) {
    console.log(`clear ${this._timeouts.length} timeouts`);

    for (var timeout of this._timeouts) {
      _timers.clearTimeout(timeout)
    }
  }
  this._timeouts = []
}

function _clearImmediates() {
  if (this._immediates.length) {
    console.log(`clear ${this._immediates.length} immediates`);

    for (var timeout of this._immediates) {
      _timers.clearImmediate(timeout)
    }
  }
  this._immediates = []
}

function _clearWebsockets() {
  if (this._websockets.length) {
    console.log(`closing ${this._websockets.length} websockets`);

    for (let w of this._websockets) {
      w.close();
      w.closeimmediate();
      w.removeAllListeners();
    }

    console.log(`after close having ${this._websockets.length} websockets`);
  }
  this._websockets = []
}

function _clearSockets() {
  const httpSockets = [].concat.apply([], Object.values(_http.globalAgent.sockets));
  const httpsSockets = [].concat.apply([], Object.values(_https.globalAgent.sockets));

  for (let s of new Set(httpSockets.concat(httpsSockets))) {
    console.log(`ending socket ${s.remoteAddress||'<destroyed>'}`);
    s.end();
  }
}

function onSceneTerminate() {
  console.log(`onSceneTerminate `);
  this.active = false
  _clearIntervals.bind(this)()
  _clearTimeouts.bind(this)()
  _clearImmediates.bind(this)()
  _clearWebsockets.bind(this)()
  _clearSockets.bind(this)()

  // memory leak fix
  delete this.sandbox.sparkwebgl.gl;
  delete this.sandbox.sparkwebgl;
  // memory leak fix
  this.sandbox.sparkscene.api = null;

  for (var k in this.sandbox)
  {
    delete this.sandbox[k];
  }

  this.contextifiedSandbox = null;
  for (var key in this.modmap)
  {
    delete this.modmap[key].context;
   delete this.modmap[key];
  }
  this.modmap = {};
  this.app = null;
  this.sandbox = {};
  for (var key in this.global)
  {
    delete this.global[key];
  }
  this.succeeded = false;
  this.active = true;
  this.makeRequire = null
  this.makeReady = null
  // JRJR something is invoking setImmediate after this and causing problems
}

function stripBOM(content) {
  if (content.charCodeAt(0) === 0xFEFF) {
    content = content.slice(1);
  }
  return content;
}
  
function initializeImportMeta(meta, { url }) {
  meta.url = url;
}
  
function LightningApp(params) {
  this.url = params.url
  this.bootstrap = params.bootstrap;
  
  this._intervals = []
  this._timeouts = []
  this._immediates = []
  this._websockets = []
  this.global = {}
  this.sandbox = {}
  this.app = null;
  this.contextifiedSandbox = null;
  this.modmap = {}

  var tmpGlobal = this.global
  tmpGlobal.beginDrawing = params._beginDrawing
  tmpGlobal.endDrawing = params._endDrawing
  tmpGlobal.sparkscene = params.sparkscene
  tmpGlobal.sparkHttp = params._sparkHttp
  // JRJR review this... if we don't draw outside of the timers
  // then no need for this... 
  // general todo... in terms of sandboxing webgl operations.
  // webgl operations outside of the timer callbacks are unsafe
  // since the gl context will not be set up correctly
  if (!tmpGlobal.sparkscene.api) {
    tmpGlobal.sparkscene.api = {}
  }
  tmpGlobal.localStorage = tmpGlobal.sparkscene.storage;
  tmpGlobal.sparkview = params._view;
  tmpGlobal.sparkkeys = _sparkkeys
  tmpGlobal.console = console
  tmpGlobal.process = process
  tmpGlobal.vm = vm
  tmpGlobal.setTimeout = xxsetTimeout.bind(this)
  tmpGlobal.clearTimeout = xxclearTimeout.bind(this)
  tmpGlobal.setInterval = xxsetInterval.bind(this)
  tmpGlobal.clearInterval = xxclearInterval.bind(this)
  tmpGlobal.setImmediate = xxsetImmediate.bind(this)
  tmpGlobal.clearImmediate = xxclearImmediate.bind(this)
  tmpGlobal.sparkQueryParams = urlmain.parse(this.url, true).query;
  tmpGlobal.thisIsTmpGlobal = true;

  var tmpSandbox = this.sandbox
  tmpSandbox.makeReady = this.makeReady = params.makeReady
  tmpSandbox.global = tmpGlobal
  tmpSandbox['Buffer'] = Buffer;
  tmpSandbox.thisIsSandbox = true;
  for (var i=0; i<sandboxKeys.length; i++)
  {
    tmpSandbox[sandboxKeys[i]] = tmpGlobal[sandboxKeys[i]];
  }
  var _this = this;
  this.bootStrapCache = {}
  this.bootStrapCache[_module._resolveFilename('ws', {paths:[__dirname].concat(_module._nodeModulePaths(__dirname))})] = function WebSocket(address, protocols, options) {
    let client = new _ws(address, protocols, options);
    _this._websockets.push(client);
    client.on('close', () => {
      console.log(`websocket ${address} closed`);
      let index = _this._websockets.indexOf(client);
      if (index !== -1) {
        _this._websockets.splice(index, 1);
      }
    });
    return client;
  };

  tmpGlobal.sparkscene.on('onSceneTerminate', function () {
    for (let key in this.bootStrapCache) {
      delete this.bootStrapCache[key];
    }
    this.esmloader.clearResources()
    this.esmloader = null
    onSceneTerminate.bind(this)();
  }.bind(this));

  this.esmloader = null
  this.makeRequire = makeRequire.bind(this)
  this.succeeded = false;
  this.active = true;
}

LightningApp.prototype.loadUrl = function() {
  var filename = this.url

  if (filename.startsWith('gl:'))
    filename = filename.substring(3)

  try {
    this.esmloader = new ESMLoader(this) 
    this.esmloader.loadESM(filename);
  }
  catch(e) {
    console.log(e)
  }
  return this.succeeded
}
module.exports.app = LightningApp;
