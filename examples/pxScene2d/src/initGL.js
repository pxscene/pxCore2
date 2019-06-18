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
var _https = require('http')

// JRJR not sure why Buffer is not already defined.
// Could look at adding to sandbox.js but this works for now
Buffer = require('buffer').Buffer

// Define global within the global namespace
var global = this

var _intervals = []
var _timeouts = []
var _immediates = []
var _websockets = []

var __dirname = process.cwd()

var loadUrl = function(url, _beginDrawing, _endDrawing, _view) {

  // JRJR review this... if we don't draw outside of the timers
  // then no need for this... 
  // general todo... in terms of sandboxing webgl operations.
  // webgl operations outside of the timer callbacks are unsafe
  // since the gl context will not be set up correctly
  global.beginDrawing = _beginDrawing
  global.endDrawing = _endDrawing

  var succeeded = false
  active = true

  var xxsetInterval = function(f,i){
    var rest = Array.from(arguments).slice(2)
    var interval = _timers.setInterval(function() {
      return function() {
        beginDrawing();
        f.apply(null,rest);
        endDrawing(); }
      }(),i)
    _intervals.push(interval)
    return interval
  }

  var xxclearInterval = function(interval) {
    var index = _intervals.indexOf(interval);
    if (index > -1) {
      _intervals.splice(index, 1);
    }
    _timers.clearInterval(interval)
  }

  var xxsetTimeout = function(f,t){
    var rest = Array.from(arguments).slice(2)
    var timeout = _timers.setTimeout(function() {
        return function() {
          //console.log('before beginDrawing2')
          beginDrawing();
          f.apply(null,rest)
          endDrawing();
          //console.log('after end Drawing2')
          var index = _timeouts.indexOf(timeout)
          if (index > -1) {
            _timeouts.splice(index,1)
          }
        }
        }(), t)
    _timeouts.push(timeout)
    return timeout
  }

  var xxclearTimeout = function(timeout) {
    var index = _timeouts.indexOf(timeout);
    if (index > -1) {
      _timeouts.splice(index, 1);
    }
    _timers.clearTimeout(timeout)
  }

  var xxsetImmediate = function(f){
    var rest = Array.from(arguments).slice(1)
    var timeout = _timers.setImmediate(function() {
        return function() {
          //console.log('before beginDrawing3')
          if (active) beginDrawing();
          f.apply(null,rest)
          if (active) endDrawing();
          //console.log('after end Drawing3')
          var index = _immediates.indexOf(timeout)
          if (index > -1) {
            _immediates.splice(index,1)
          }
        }
        }())
    _immediates.push(timeout)
    return timeout

   console.log('setImmediate called')
  }


  var xxclearImmediate = function(immediate) {
    var index = _immediates.indexOf(immediate);
    if (index > -1) {
      _immediates.splice(index, 1);
    }
    _timers.clearTimeout(immediate)
  }
  
  // (Re)define a few globals for our wrappers
  global.setTimeout = xxsetTimeout
  global.clearTimeout = xxclearTimeout
  global.setInterval = xxsetInterval
  global.clearInterval = xxclearInterval
  global.setImmediate = xxsetImmediate
  global.clearImmediate = xxclearImmediate
  global.sparkview = _view
  global.sparkscene = getScene("scene.1")
  global.sparkgles2 = require('gles2.js');

  global.sparkscene.on('onClose', onClose);

// JRJR todo make into a map
var bootStrapCache = {}

  // Add wrapped standard modules here...
  bootStrapCache[_module._resolveFilename('ws')] = function WebSocket(address, protocols, options) {
    let client = new _ws(address, protocols, options);
    _websockets.push(client);
    client.on('close', () => {
      console.log(`websocket ${address} closed`);
      let index = _websockets.indexOf(client);
      if (index !== -1) {
        _websockets.splice(index, 1);
      }
    });
    return client;
  };

// Spark node-like module loader
const bootStrap = (moduleName, from, request) => {
  const makeRequire = pathToParent => {
    return moduleName => {
      const parentDir = path.dirname(pathToParent);
      // use Node's built-in module resolver here, but we could easily pass in our own
      var resolvedModule = _module._resolveLookupPaths(moduleName, {paths:[parentDir],id:pathToParent,filename:pathToParent});
      var id = resolvedModule[0];
      var paths = resolvedModule[1];

      const filename = _module._resolveFilename(moduleName, {paths:[parentDir].concat(_module._nodeModulePaths(parentDir)),id:pathToParent,filename:pathToParent});

      // Spark Modules should be loaded a "singleton" per scene
      // If we've already loaded a module then return it's cached exports
      if (filename in bootStrapCache) {
        console.log('Using cached module:', filename)
        return bootStrapCache[filename]
      }

      // JRJR Hack to handle native modules give we don't have
      // access to NativeModule.require here.
      if (filename.indexOf('.node') != -1 || paths.length == 0 || filename.indexOf('package.json') != -1) {
        console.log('Loading native module: ', filename)
        var m = _module._load(filename, {paths:[parentDir],id:pathToParent,filename:pathToParent})
        
        // Cache the module exports
        bootStrapCache[filename] = m
        return m
      }

      console.log('Loading source module:', filename)
      const source = fs.readFileSync(filename, 'utf-8');

      const wrapped = `(function(exports,require,module,__filename,__dirname) {${source}})`;

      let compiled
      if (false) {
          var sandbox = {}
          for (var k of Object.getOwnPropertyNames(global)) { sandbox[k] = global[k]}
          sandbox.setTimeout = xxsetTimeout
          sandbox.clearTimeout = xxclearTimeout
          sandbox.setInterval = xxsetInterval
          sandbox.clearInterval = xxclearInterval
          sandbox.setImmediate = xxsetImmediate
          sandbox.clearImmediate = xxclearImmediate
          sandbox.sparkview = _view

          beginDrawing = _beginDrawing
          endDrawing = _endDrawing

          compiled = vm.runInNewContext(wrapped, sandbox, {filename:filename});
      }
      else {
        compiled = vm.runInThisContext(wrapped, {filename:filename,displayErrors:true})
      }
      
      const exports = {};
      // OUR own require, independent of node require
      const require = makeRequire(filename);
      const module = {exports};

      try {
          compiled.call(exports, exports, require, module, filename, path.dirname(filename));
      }
      catch(e) {
        console.log(e)
      }

      bootStrapCache[filename] = module.exports
      return module.exports;
    };
  };
  return makeRequire(from)(moduleName);
};  

  var filename = ''

  if (url.startsWith('gl:'))
    filename = url.substring(3)

  var initGLPath = __dirname+'/initGL.js'

  try {
    // bootStrap into the spark module system
    bootStrap(filename,initGLPath,'blah')
    succeeded = true
  }
  catch(e) {
    console.log(e)
  }

  try {
    var module = require.resolve(initGLPath)
    if (typeof require.cache[module] != undefined)
      delete require.cache[module]
  }
  catch(e) {}

  try {
    var module = require.resolve(filename)
    if (typeof require.cache[module] != undefined)
      delete require.cache[module]
  }
  catch(e) {}

  return succeeded
}

var _clearIntervals = function() {
  for(var interval of _intervals) {
    _timers.clearInterval(interval)
  }
  _intervals = []
}

var _clearTimeouts = function() {
  for(var timeout of _timeouts) {
    _timers.clearTimeout(timeout)
  }
  _timeouts = []
}

var _clearImmediates = function() {
  for(var timeout of _immediates) {
    _timers.clearTimeout(timeout)
  }
  _immediates = []
}

var _clearWebsockets = function() {
  if (_websockets.length) {
    console.log(`closing ${_websockets.length} websockets`);

    for (let w of _websockets) {
      w.close();
      w.closeimmediate();
      w.removeAllListeners();
    }

    console.log(`after close having ${_websockets.length} websockets`);
  }
  _websockets = []
}

var _clearSockets = function() {
  const httpSockets = [].concat.apply([], Object.values(_http.globalAgent.sockets));
  const httpsSockets = [].concat.apply([], Object.values(_https.globalAgent.sockets));

  for (let s of new Set(httpSockets.concat(httpsSockets))) {
    console.log(`ending socket ${s.remoteAddress}`);
    s.end();
  }
}

var onClose = function() {
  console.log(`onClose`);

  _clearIntervals()
  _clearTimeouts()
  _clearImmediates()
  _clearWebsockets()
  _clearSockets()

  // JRJR something is invoking setImmediate after this and causing problems
  active = false
}

