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
var sandboxKeys = ["vm", "process", "setTimeout", "console", "clearTimeout", "setInterval", "clearInterval", "setImmediate", "clearImmediate", "sparkview", "sparkscene", "sparkgles2", "beginDrawing", "endDrawing", "sparkwebgl", "sparkkeys", "sparkQueryParams", "require", "localStorage", "sparkHttp"]
var sandbox = {}
/* holds loaded main mjs module reference */
var app = null;
var contextifiedSandbox = null;
var __dirname = process.cwd()
/* holds map of depenedent module name and its reference */
var modmap = {}
var loadUrl = function(url, _beginDrawing, _endDrawing, _view, _frameworkURL, _options, _sparkHttp) {

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
          try {
          beginDrawing();
          f.apply(null,rest);
          endDrawing(); }
           catch(e) {
            console.log("exception during draw in setInterval !!");
          }}
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
          try {
            beginDrawing();
            f.apply(null,rest);
            endDrawing();
          } catch(e) {
            console.log("exception during draw in setTimeout !!");
          }
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
          try {
            if (active) beginDrawing();
            f.apply(null,rest)
            if (active) endDrawing();
          } catch(e) {
            console.log("exception during draw in setImmediate !!");
          }
          //console.log('after end Drawing3')
          var index = _immediates.indexOf(timeout)
          if (index > -1) {
            _immediates.splice(index,1)
          }
        }
        }())
    _immediates.push(timeout)
    return timeout
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
  if (!global.sparkscene.api) {
    global.sparkscene.api = {}
  }
  global.localStorage = global.sparkscene.storage;
  global.sparkHttp = _sparkHttp;
  const script = new vm.Script("global.sparkwebgl = sparkwebgl= require('webgl'); global.sparkgles2 = sparkgles2 = require('gles2.js'); global.sparkkeys = sparkkeys = require('rcvrcore/tools/keys.js');");
  global.sparkscene.on('onSceneTerminate', () => {
    for (let key in bootStrapCache) {
      delete bootStrapCache[key];
    }
    onSceneTerminate();
  });
  global.sparkQueryParams = urlmain.parse(url, true).query;
  sandbox.global = global
  sandbox.vm = vm;
  for (var i=0; i<sandboxKeys.length; i++)
  {
    sandbox[sandboxKeys[i]] = global[sandboxKeys[i]];
  }
  sandbox['Buffer'] = Buffer;
// JRJR todo make into a map
var bootStrapCache = {}

  // Add wrapped standard modules here...
  bootStrapCache[_module._resolveFilename('ws', {paths:[__dirname].concat(_module._nodeModulePaths(__dirname))})] = function WebSocket(address, protocols, options) {
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
    global.sparkscene.loadArchive(fileUri).ready.then(a => {
      if (a.loadStatus.httpStatusCode !== 200) {
        console.error(`StatusCode Bad: FAILED to read file[${fileUri}] from web service`);
        reject(a.loadStatus.httpStatusCode);
      } else {
        resolve(a.getFileAsString(""));
      }
    }, () => {
      console.error(`Error: FAILED to read file[${fileUri}] from web service`);
      reject();
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
  mod = new vm.SourceTextModule(jsonsource , { url: specifier, context:ctx, initializeImportMeta:function(meta, url) {
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

/* load javascript file and returns as ejs module */
async function loadJavaScriptModule(source, specifier, ctx)
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
  mod = new vm.SourceTextModule(jssource , { url: specifier, context:ctx, initializeImportMeta:function(meta, url) {
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
  mod = new vm.SourceTextModule(source , { url: "file://" + __dirname + specifier, context:ctx, initializeImportMeta:function(meta, url) {
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

/* load commonjs module and returns as ejs module */
async function loadCommonJSModule(specifier, ctx)
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
  mod = new vm.SourceTextModule(source , { url: "file://" + __dirname + "/node_modules/" + specifier, context:ctx, initializeImportMeta:function(meta, url) {
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
async function getModule(specifier, referencingModule) {
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
           let result = await loadHttpFile(specifier);
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
           mod = loadJavaScriptModule(source, specifier, referencingModule.context);
         }
         else
         {
           specifier = "http://" + baseString.substring(0, baseString.lastIndexOf("/")+1) + specifier;
           let result = await loadHttpFile(specifier);
           mod = loadJavaScriptModule(result, specifier, referencingModule.context);
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
       mod = loadCommonJSModule(specifier, referencingModule.context);
     }
     else if(specifier == 'fs' || specifier == 'http' || specifier == 'https') {
       mod = loadCommonJSModule(specifier, referencingModule.context);
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
           let result = await loadHttpFile(specifier);
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
     // making sure we are not appending extension with files already having extension
     if ((specifier.endsWith(".js") == false) && (specifier.endsWith(".mjs") == false))
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
           let result = await loadHttpFile(specifier);
           mod = loadMjs(result, specifier, referencingModule.context);
         }
       } catch(err) {
         console.log(err);
       }
     }
  }
  return mod;
  } catch(err) {
    console.log(err);
 }
}

async function linker(specifier, referencingModule) {
  try {
  var mod = await getModule(specifier, referencingModule);
  return mod;
  } catch(err) {
    console.log("linker error !!!!!!!!!!!!!!!!!");
    console.log(err);
  }
}

async function importModuleDynamically(specifier, { url }) {
  var mod = await getModule(specifier,{ url });
  mod.instantiate();
  await mod.evaluate();
  return mod;
}

async function loadMjs(source, url, context)
{
  if (url in modmap)
  {
    return modmap[url];
  }
  var mod = new vm.SourceTextModule(source , { context: context, initializeImportMeta:initializeImportMeta, importModuleDynamically:importModuleDynamically, url:url });
  modmap[url] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(linker);
  }
  return mod;
}

// Spark node-like module loader
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

      var source = "";
      if (isLocalFile) {
        if (/^file:/.test(filename)) {
          source = fs.readFileSync(new urlmain.URL(filename), 'utf-8');
        } else {
          source = fs.readFileSync(filename, 'utf-8');
        }
      } else {
        throw 'cannot require remote modules!';
      }

      const wrapped = `(function(exports,require,module,__filename,__dirname) {${source}})`;
      let compiled = vm.runInThisContext(wrapped, {filename:filename,displayErrors:true})
      const exports = {};
      // OUR own require, independent of node require
      const require = makeRequire(filename);
      const module = {exports};
      try {
          compiled.call(exports, exports, require, module, filename, path.dirname(filename));
      }
      catch(e) {
        console.log(e);
      }

      bootStrapCache[filename] = module.exports
      return module.exports;
    };
  };

  function filename2url(loc) {
    let pos = loc.indexOf('#');
    if (pos !== -1)
      loc = loc.substring(0, pos);
    pos = loc.indexOf('?');
    if (pos !== -1)
      loc = loc.substring(0, pos);

    if (/^http:|^https:|^file:/.test(loc))
      return loc; //already a URL

    if (loc.charAt(0) !== '/' && __dirname)
      loc = urlmain.resolve(__dirname + '/', loc);
    if (!/^file:/.test(loc))
      loc = `file://${loc}`;
    return loc;
  }

  async function getFile(url) {
    if (/^http:|^https:/.test(url))
      return await loadHttpFile(url);
    if (/^file:/.test(url))
      return await readFileAsync(new urlmain.URL(url), {'encoding': 'utf-8'});
    return await readFileAsync(url, {'encoding': 'utf-8'});
  }

  function loadESM(filename) {
    const url = filename2url(filename);
    const loc = /^file:/.test(url) ? url.substring(7) : url;

    // override require to our own require to load files relative to file path
    sandbox.require = makeRequire(loc);
    sandbox['__dirname'] = path.dirname(loc);

    contextifiedSandbox = vm.createContext(sandbox);
    script.runInContext(contextifiedSandbox);

    try {
      (async () => {
        let instantiated = false;
        try {
          if (_frameworkURL) {
            const url2 = filename2url(_frameworkURL);
            const loc2 = /^file:/.test(url2) ? url2.substring(7) : url2;
            const source2 = await getFile(url2);

            // use paths for frameworkURL
            sandbox.require = makeRequire(loc2);
            sandbox['__dirname'] = path.dirname(loc2);

            vm.runInContext(source2, contextifiedSandbox, {filename:loc2});

            // restore previous values
            sandbox.require = makeRequire(loc);
            sandbox['__dirname'] = path.dirname(loc);
          }

          const source = await getFile(url);
          app = await loadMjs(source, url, contextifiedSandbox);
          app.instantiate();
          instantiated = true;
          succeeded = true;
          makeReady(true, app.namespace);
          beginDrawing();
          await app.evaluate();

          if (typeof app.namespace.default === 'function') {
            try {
              if (_options) {
                new app.namespace.default(_options);
              } else {
                new app.namespace.default();
              }
            } catch (err) {
              console.log(err);
            }
          }

          endDrawing();
        } catch (err) {
          console.log("load mjs module failed "+err);
          console.log(err);
          if (false === instantiated) {
            makeReady(false, {});
          }
        }
      })();
    } catch (err) {
      console.log(err);
    }
  }

  var filename = url

  if (filename.startsWith('gl:'))
    filename = filename.substring(3)

  var initGLPath = __dirname+'/initGL.js'

  try {
    loadESM(filename);
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

var onSceneTerminate  = function() {
  console.log(`onSceneTerminate `);
  active = false

  _clearIntervals()
  _clearTimeouts()
  _clearImmediates()
  _clearWebsockets()
  _clearSockets()

  // memory leak fix
  delete sandbox.sparkwebgl.instance.gl;
  delete sandbox.sparkwebgl.instance;

  for (var k in sandbox)
  {
    delete sandbox[k];
  }

  contextifiedSandbox = null;
  for (var key in modmap)
  {
    delete modmap[key].context;
   delete modmap[key];
  }
  modmap = {};
  app = null;
  sandbox = {};
  for (var key in global)
  {
    delete global[key];
  }
  // JRJR something is invoking setImmediate after this and causing problems
}
