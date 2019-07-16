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
var _require = require;
var _ws = require('ws')
var _http = require('http')
var _https = require('https')
var urlmain = require("url")
const {promisify} = require('util')
const readFileAsync = promisify(fs.readFile)
const ArrayJoin = Function.call.bind(Array.prototype.join);
const ArrayMap = Function.call.bind(Array.prototype.map);
var reqOrig = require;

// JRJR not sure why Buffer is not already defined.
// Could look at adding to sandbox.js but this works for now
Buffer = require('buffer').Buffer

// Define global within the global namespace
var global = this
var _intervals = []
var _timeouts = []
var _immediates = []
var _websockets = []
var sandboxKeys = ["vm", "process", "setTimeout", "console", "clearTimeout", "setInterval", "clearInterval", "setImmediate", "clearImmediate", "sparkview", "sparkscene", "sparkgles2", "beginDrawing", "endDrawing", "sparkwebgl", "require"]
var sandbox = {}
/* holds loaded main mjs module reference */
var app = null;
var contextifiedSandbox = null;
var __dirname = process.cwd()
var modmap = {}
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
  const script = new vm.Script("global.sparkwebgl = sparkwebgl= require('webgl'); global.sparkgles2 = sparkgles2 = require('gles2.js');");
  global.sparkscene.on('onClose', onClose);
  sandbox.global = global
  sandbox.vm = vm;
  sandbox['dynamic'] = './b.mjs';
  for (var i=0; i<sandboxKeys.length; i++)
  {
    sandbox[sandboxKeys[i]] = global[sandboxKeys[i]];
  }
  var sandboxDir = __dirname;
  if (url.startsWith('gl:')) {
    var fn = url.substring(3);
    if (fn.indexOf('/') == 0)
    {
      sandboxDir = fn.substring(0,fn.lastIndexOf('/')+1);
    }
  }
  sandbox['__dirname'] = sandboxDir;
  sandbox['require'] = reqOrig;
  sandbox['Buffer'] = Buffer;
  contextifiedSandbox = vm.createContext(sandbox);
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

function initializeImportMeta(meta, { url }) {
  meta.url = url;
}

function loadHttpFile(fileUri) {
  return new Promise(function(resolve, reject) {
    var code = [];
    var options = urlmain.parse(fileUri);
    var req = null;
    var httpCallback = function (res) {
      res.on('data', function (data) {
        if (Buffer.isBuffer(data)) {
          code.push(data);
        } else {
          code.push(new Buffer(data));
        }
      });
      res.on('end', function () {
        if( res.statusCode === 200 ) {
          var data = Buffer.concat(code);
          resolve(data.toString('utf8'));
         } else {
          console.error("StatusCode Bad: FAILED to read file[" + fileUri + "] from web service");
          reject(res.statusCode);
          }
        });
    };
    var isHttps = fileUri.substring(0, 5).toLowerCase() === "https";
    req = (isHttps ? _https : _http).get(options, httpCallback);
    
    req.on('error', function (err) {
      console.error("Error: FAILED to read file[" + fileUri + "] from web service");
      reject(err);
    });
  });
}

function stripBOM(content) {
  if (content.charCodeAt(0) === 0xFEFF) {
    content = content.slice(1);
  }
  return content;
}

/* load json module and returns as ejs module */
async function loadJsonModule(source, specifier, ctx)
{
  if (specifier in modmap)
  { 
    return modmap[specifier];
  }
  var mod;
  var module = JSON.parse(stripBOM(source));
  const names = ArrayMap([...Object.keys(module), 'default'], (name) => `${name}`);
    const jsonsource = `
  ${ArrayJoin(ArrayMap(names, (name) =>
      `let $${name};
  export { $${name} as ${name} };
  import.meta.exports.${name} = {
    get: () => $${name},
    set: (v) => $${name} = v,
  };`), '\n')
  }
  import.meta.done();
  `
  mod = vm.CreateSourceTextModule(jsonsource , { url: specifier, context:ctx, initializeImportMeta:function(meta, url) {
      meta.exports = {}
      meta.done = () => {
       try {
        for (const key of Object.keys(module))
          meta.exports[key].set(module[key]);
        meta.exports['default'].set(module);
       } catch (err) {
         err.message = url + ': ' + err.message;
         throw err;
       }
   }}});
  modmap[specifier] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(function() {});
  }
  return mod;
}

/* load js file and returns as ejs module */
async function loadJsModule(source, specifier, ctx)
{
  if (specifier in modmap)
  { 
    return modmap[specifier];
  }
  var mod;
  var wrapper = [
    '(function (module) { ',
    '\n});'
  ];
  var module = {};
  module.exports = {}
  source = wrapper[0] + source + wrapper[1];
  var moduleFunc = vm.runInContext(source, sandbox, {'filename':specifier, 'displayErrors':true});
  moduleFunc(module);
  const names = ArrayMap([...Object.keys(module.exports), 'default'], (name) => `${name}`);
    const jssource = `
  ${ArrayJoin(ArrayMap(names, (name) =>
      `let $${name};
  export { $${name} as ${name} };
  import.meta.exports.${name} = {
    get: () => $${name}, 
    set: (v) => $${name} = v,
  };`), '\n')
  }
  import.meta.done();
  `
  mod = vm.CreateSourceTextModule(jssource , { url: specifier, context:ctx, initializeImportMeta:function(meta, url) {
      meta.exports = {} 
      meta.done = () => {
       try {
        for (const key of Object.keys(module.exports))
          meta.exports[key].set(module.exports[key]);
        meta.exports['default'].set(module.exports);
       } catch (err) { 
         err.message = url + ': ' + err.message;
         throw err;
       }
   }}});
  modmap[specifier] = mod; 
  if (mod.linkingStatus == 'unlinked')
  { 
    var result = await mod.link(function() {});
  }
  return mod;
}      

/* load node module and returns as ejs module */
async function loadNodeModule(specifier, ctx)
{     
  if (specifier in modmap)
  { 
    return modmap[specifier];
  }
  var mod;
  var module; 
  const names = ArrayMap(['default'], (name) => `${name}`);
    const source = `
  ${ArrayJoin(ArrayMap(names, (name) =>
      `let $${name};
  export { $${name} as ${name} };
  import.meta.exports.${name} = {
    get: () => $${name}, 
    set: (v) => $${name} = v,
  };`), '\n')
  }
  import.meta.done();
  `
  mod = vm.CreateSourceTextModule(source , { url: "file://" + __dirname + specifier, context:ctx, initializeImportMeta:function(meta, url) {
      meta.exports = {} 
      meta.done = () => {
        const module = { exports: {} };
        const pathname = internalURLModule.fileURLToPath(new URL(url));
        process.dlopen(module, _makeLong(pathname));
        meta.exports.default.set(module.exports);
  }}});
  modmap[specifier] = mod; 
  if (mod.linkingStatus == 'unlinked')
  { 
    var result = await mod.link(function() {});
  }
  return mod;
}

/* load cjs module and returns as ejs module */
async function loadCjsModule(specifier, ctx)
{
  if (specifier in modmap)
  {
    return modmap[specifier];
  }
  var mod;
  var module = reqOrig(specifier);
  const names = ArrayMap([...Object.keys(module)], (name) => `${name}`);
  if (false == names.includes("default"))
  {
    names.push("default");
  }
    const source = `
  ${ArrayJoin(ArrayMap(names, (name) =>
      `let $${name};
  export { $${name} as ${name} };
  import.meta.exports.${name} = {
    get: () => $${name},
    set: (v) => $${name} = v,
  };`), '\n')
  }
  import.meta.done();
  `
  mod = vm.CreateSourceTextModule(source , { url: "file://" + __dirname + "/node_modules/" + specifier, context:ctx, initializeImportMeta:function(meta, url) {
      meta.exports = {}
      meta.done = () => {
        for (const key of Object.keys(module))
          meta.exports[key].set(module[key]);
        meta.exports['default'].set(module);
      };
  }});
  modmap[specifier] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(function() {});
  }
  return mod;
}

/* load mjs file and returns as ejs module */
async function getMod(specifier, referencingModule) {
   try {
   var isLocalApp = (referencingModule.url.indexOf("file:") == 0)?true:false;
   var baseString = "";
   if (isLocalApp) {
     baseString = referencingModule.url.indexOf("file:") == 0?referencingModule.url.substring(7):referencingModule.url;
   }
   else
   {
     baseString = referencingModule.url.indexOf("http:") == 0?referencingModule.url.substring(7):referencingModule.url;
   }
   var mod;
   if (specifier.indexOf('.node') != -1) // native node module
   {
     mod = loadNodeModule(specifier, referencingModule.context);
   }
   else if (specifier.indexOf('.json') != -1) // json module
   {
     var source;
     try {
       if (isLocalApp) {
           specifier = path.resolve(baseString.substring(0, baseString.lastIndexOf("/")+1), specifier);
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadJsonModule(source, specifier, referencingModule.context);
         }
         else
         {
           specifier = "http://" + baseString.substring(0, baseString.lastIndexOf("/")+1) + specifier;
           result = await loadHttpFile(specifier);
           mod = loadJsonModule(result, specifier, referencingModule.context);
         }
       } catch(err) {
         console.log(err);
       }
   }
   else if (specifier.indexOf('.js') != -1) // js module used by app
   {
     var source;
     try {
       if (isLocalApp) {
           specifier = path.resolve(baseString.substring(0, baseString.lastIndexOf("/")+1), specifier);
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadJsModule(source, specifier, referencingModule.context);
         }
         else
         {
           specifier = "http://" + baseString.substring(0, baseString.lastIndexOf("/")+1) + specifier;
           result = await loadHttpFile(specifier);
           mod = loadJsModule(result, specifier, referencingModule.context);
         }
       } catch(err) {
         console.log(err);
       }
   }
   // may be relative or non-relative
   else if ((specifier.startsWith(".") == false) && (specifier.startsWith("/") == false)) {
     // node modules
     if (fs.existsSync("node_modules/" + specifier) && (specifier.indexOf("wpe-lightning") == -1))
     {
       mod = loadCjsModule(specifier, referencingModule.context);
     }
     else if(specifier == 'fs' || specifier == 'http' || specifier == 'https') {
       mod = loadCjsModule(specifier, referencingModule.context);
     }
     else
     {
       // mjs module
       var treatAsLocal = false;
       if(specifier.indexOf("wpe-lightning-spark") == 0)
       {
         treatAsLocal = true;
         specifier = path.resolve(__dirname ,"node_modules/wpe-lightning-spark/src/lightning.mjs") ;
       }
       else if(specifier.indexOf("wpe-lightning") == 0)
       {
         treatAsLocal = true;
         specifier = path.resolve(__dirname ,"node_modules/" + specifier);
       }
       else if (specifier.indexOf(".mjs") != -1)
       {
         if (isLocalApp) {
           specifier = path.resolve(baseString.substring(0, baseString.lastIndexOf("/")+1), specifier);
         }
         else
         {
           specifier = baseString.substring(0, baseString.lastIndexOf("/")+1) + specifier;
         }
       }
       if (specifier in modmap)
       {
         mod = modmap[specifier];
       }
       else {
       var source;
       try {
         if (isLocalApp || (true == treatAsLocal)) {
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadMjs(source, specifier, referencingModule.context);
         }
         else
         {
           specifier = "http://" + specifier;
           result = await loadHttpFile(specifier);
           mod = loadMjs(result, specifier, referencingModule.context);
         }
       } catch(err) {
         console.log(err);
       }
      }
     }
   }
   else if (specifier.indexOf(".") == 0) {
     if (specifier.indexOf("./") == 0) {
       specifier = specifier.substring(2);
     }
     if (isLocalApp) {
       specifier = path.resolve(baseString.substring(0, baseString.lastIndexOf("/")+1), specifier);
     }
     else
     { 
       specifier = baseString.substring(0, baseString.lastIndexOf("/")+1) + specifier;
     }
     if (specifier.indexOf(".") == -1)
     {
       specifier = specifier + ".mjs";
     } 
     if (specifier in modmap)
     {
       mod = modmap[specifier];
     }
     else {
       // search for ux.mjs or ux.js
       var source;
       try {
         if (isLocalApp) {
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadMjs(source, specifier, referencingModule.context);
         }
         else
         {
           //http module read
           specifier = "http://" + specifier;
           result = await loadHttpFile(specifier);
           mod = loadMjs(result, specifier, referencingModule.context);
         }
       } catch(err) {
         console.log(err);
       }
     }
  }
  return mod;
  } catch(err) {
    console.log("Exception in linker " + err);
 }
}

async function linker(specifier, referencingModule) {
  try {
  var mod = await getMod(specifier, referencingModule);
  return mod;
  } catch(err) {
    console.log("linker error !!!!!!!!!!!!!!!!! "  +err);
  }
}

async function importModuleDynamically(specifier, { url }) {
  var mod = await getMod(specifier,{ url });
  mod.instantiate();
  await mod.evaluate();
  return mod;
}

async function loadMjs(source, url, context)
{
  var mod = vm.CreateSourceTextModule(source , { context: context, initializeImportMeta:initializeImportMeta, importModuleDynamically:importModuleDynamically, url:url });
  modmap[url] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(linker);
  }
  return mod;
}

// Spark node-like module loader
const bootStrap = (moduleName, from, request) => {
  const makeRequire = pathToParent => {
    return moduleName => {
      const parentDir = path.dirname(pathToParent);
      // use Node's built-in module resolver here, but we could easily pass in our own
      var resolvedModule = _module._resolveLookupPaths(moduleName, {paths:[parentDir],id:pathToParent,filename:pathToParent});
      var id = resolvedModule[0];
      var paths = resolvedModule[1];
      var isLocalFile = moduleName.indexOf("http")==-1?true:false;
      const filename = (isLocalFile)?_module._resolveFilename(moduleName, {paths:[parentDir].concat(_module._nodeModulePaths(parentDir)),id:pathToParent,filename:pathToParent}):moduleName;
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

                console.log("About to load http file !!");
      //it will be good if we have some way to tell, we are loading esm module in js file
      if ((filename.indexOf('.mjs') != -1)) {
        script.runInContext(contextifiedSandbox);
        try {
          (async () => {
            var instantiated = false;
            try
            {
              var source, rpath;
              if (filename.indexOf('http') == 0) {
                result = await loadHttpFile(filename);
                app = await loadMjs(result, filename, contextifiedSandbox);
                app.instantiate();
                instantiated = true;
                makeReady(true, {});
                await app.evaluate();
              }
              else
              {
                if (filename.indexOf("/") != 0) {
                  rpath = path.resolve(__dirname, filename);
                }
                else
                {
                  rpath = filename;
                }
                source = await readFileAsync(rpath, {'encoding' : 'utf-8'})
                rpath = "file://" + rpath;
                app = await loadMjs(source, rpath, contextifiedSandbox);
                app.instantiate();
                instantiated = true;
                makeReady(true, {});
                await app.evaluate();
              }
            }
            catch(err) {
              console.log("load mjs module failed " + err);
              if (false == instantiated) {
                makeReady(false, {});
              } 
            }
          })();
        }
        catch(err) {
          console.log(err);
        }
        return;
      }

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
      global.sparkwebgl = sparkwebgl= reqOrig('webgl'); global.sparkgles2 = sparkgles2 = reqOrig('gles2.js');
      const exports = {};
      // OUR own require, independent of node require
      const require = makeRequire(filename);
      const module = {exports};

      try {
          compiled.call(exports, exports, require, module, filename, path.dirname(filename));
          makeReady(true, {});
      }
      catch(e) {
        console.log(e);
        makeReady(false, {});
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
  if (_intervals.length) {
    console.log(`clear ${_intervals.length} intervals`);

    for (var interval of _intervals) {
      _timers.clearInterval(interval)
    }
  }
  _intervals = []
}

var _clearTimeouts = function() {
  if (_timeouts.length) {
    console.log(`clear ${_timeouts.length} timeouts`);

    for (var timeout of _timeouts) {
      _timers.clearTimeout(timeout)
    }
  }
  _timeouts = []
}

var _clearImmediates = function() {
  if (_immediates.length) {
    console.log(`clear ${_immediates.length} immediates`);

    for (var timeout of _immediates) {
      _timers.clearImmediate(timeout)
    }
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
    console.log(`ending socket ${s.remoteAddress||'<destroyed>'}`);
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
  for (var i=0; i<sandboxKeys.length; i++)
  {
    sandbox[sandboxKeys[i]] = null;
  }
  sandbox['global'] = null;
  contextifiedSandbox = null;
  vm.ClearSourceTextModules();
  for (var key in modmap)
  {
   delete modmap[key]; 
   modmap[key] = null;
  }
  modmap = {};
  app = null;
  sandbox = {};
  // JRJR something is invoking setImmediate after this and causing problems
  active = false
}
