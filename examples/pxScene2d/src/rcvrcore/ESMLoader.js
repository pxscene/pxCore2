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

var vm = require('vm')
var urlmain = require("url")
var fs = require('fs')
var path = require('path')
var {promisify} = require('util')
var readFileAsync = promisify(fs.readFile)
var ArrayJoin = Function.call.bind(Array.prototype.join);
var ArrayMap = Function.call.bind(Array.prototype.map);
var reqOrig = require;
var frameWorkCache = {}
var enableFrameworkCaching = undefined;
var keepFrameworksOnExit = undefined;

// Normalize the URL, for consistency. Remove the query part.
// All local URLs would have the 'file:' prefix.
var filename2url = function (loc) {
  var __dirname = process.cwd()
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

function initializeImportMeta(meta, { url }) {
  meta.url = url;
}

var fastFetch = require('node-fetch').fastFetch;

var loadHttpFile = function(scene, fileUri) {
  return new Promise(function(resolve, reject) {
    fastFetch(this.global, fileUri, {}).then( data => {
      if (data.statusCode !== 200) {
        console.error(`StatusCode Bad: FAILED to read file[${fileUri}] http file get`);
        reject(data.statusCode);
      } else {
        resolve(data.responseData);
      }
    }).catch(err => { console.error(`Error: FAILED to read file[${fileUri}] from web service`); reject(); });
  });
  /*return new Promise(function(resolve, reject) {
    scene.loadArchive(fileUri).ready.then(a => {
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
  });*/
}

function stripBOM(content) {
  if (content.charCodeAt(0) === 0xFEFF) {
    content = content.slice(1);
  }
  return content;
}

/* load json module and returns as ejs module */
var loadJsonModule = async function (source, specifier, ctx)
{
  if (specifier in ctx.modmap)
  {
    return ctx.modmap[specifier];
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
  ctx.modmap[specifier] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(function() {});
  }
  return mod;
}
    
/* load javascript file and returns as ejs module */
var loadJavaScriptModule = async function (source, specifier, ctx, version)
{
  if (specifier in ctx.modmap)
  {
    return ctx.modmap[specifier];
  }
  var mod;
  var wrapper = [
    '(function (module) { ',
    '\n});'
  ];
  var module = {};
  module.exports = {}
  source = wrapper[0] + source + wrapper[1];
  var moduleFunc = vm.runInContext(source, ctx, {'filename':specifier, 'displayErrors':true});
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
  mod.version = version;
  ctx.modmap[specifier] = mod; 
  if (mod.linkingStatus == 'unlinked')
  { 
    var result = await mod.link(function() {});
  }
  return mod;
}      
    
/* load node module and returns as ejs module */
var loadNodeModule = async function (specifier, ctx)
{     
  if (specifier in ctx.modmap)
  {
    return ctx.modmap[specifier];
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
        const pathname = urlmain.fileURLToPath(new urlmain.URL(url));
        process.dlopen(module, path._makeLong(pathname));
        meta.exports.default.set(module.exports);
  }}});
  ctx.modmap[specifier] = mod; 
  if (mod.linkingStatus == 'unlinked')
  { 
    var result = await mod.link(function() {});
  }
  return mod;
}
    
/* load commonjs module and returns as ejs module */
var loadCommonJSModule = async function (specifier, ctx)
{
  if (specifier in ctx.modmap)
  {
    return ctx.modmap[specifier];
  }
  var __dirname = process.cwd()
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
  ctx.modmap[specifier] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(function() {});
  }
  return mod;
}

var getFile = async function (scene, url) {
  if (/^http:|^https:/.test(url))
    return await loadHttpFile(scene, url);
  if (/^file:/.test(url))
    return await readFileAsync(new urlmain.URL(url), {'encoding': 'utf-8'});
  return await readFileAsync(url, {'encoding': 'utf-8'});
}

/* load mjs file and returns as ejs module */
var getModule = async function (specifier, referencingModule, version) {
  if (/\.node$/.test(specifier))
    return await loadNodeModule(specifier, referencingModule.context);
  if ((fs.existsSync("node_modules/" + specifier) && !/^(wpe-lightning|wpe-lightning-spark)/.test(specifier)) || /^(fs|http|https)$/.test(specifier))
    return await loadCommonJSModule(specifier, referencingModule.context);

  if (specifier.indexOf("wpe-lightning-spark") === 0) {
    specifier = path.resolve(process.cwd(), "node_modules/wpe-lightning-spark/src/lightning.mjs");
  } else if (specifier.indexOf("wpe-lightning") === 0) {
    specifier = path.resolve(process.cwd(), "node_modules/" + specifier);
  }

  // The following code is default.
  // Specifier can be a relative, absolute path, or URL. Convert to URL, for consistency.
  else {
    if (!/^(http:|https:|file:)/.test(specifier))
      specifier = urlmain.resolve(referencingModule.url, specifier);
    const ext = path.extname(specifier);
    if (!/^(\.js|\.mjs)$/.test(ext))
      specifier += '.mjs';
  }
  if (!/^(http:|https:|file:)/.test(specifier))
    specifier = `file://${specifier}`;

  // Load as .js, .mjs, or .json.
  // If the module has been loaded already, return it.
  if (specifier in referencingModule.context.modmap) {
    return referencingModule.context.modmap[specifier];
  }

  // Same file from different domains is shared if has 'version'.
  // URL is added to modmap anyway.
  if (version) {
    for (let k in referencingModule.context.modmap) {
      if (referencingModule.context.modmap.hasOwnProperty(k)) {
        let mod = referencingModule.context.modmap[k];
        if (mod.version === version) {
          referencingModule.context.modmap[specifier] = mod;
          return mod;
        }
      }
    }
  }

  const source = await getFile(referencingModule.context.global.sparkscene, specifier);
  if (/\.json$/.test(specifier))
    return await loadJsonModule(source, specifier, referencingModule.context);
  if (/\.js$/.test(specifier))
    return await loadJavaScriptModule(source, specifier, referencingModule.context, version);
  return await loadMjs(source, specifier, referencingModule.context, referencingModule.context.modmap, version);
};

var linker = async function (specifier, referencingModule) {
  try {
  var mod = await getModule(specifier, referencingModule);
  return mod;
  } catch(err) {
    console.log("linker error !!!!!!!!!!!!!!!!!");
    console.log(err);
  }
}

// NOTE: 'version' is not a standard arg. It's used in our code. See getModule.
var importModuleDynamically = async function (specifier, referencingModule, version) {
  var mod = await getModule(specifier,referencingModule,version);
  mod.instantiate();
  await mod.evaluate();
  return mod;
}

var loadMjs = async function (source, url, context, modmap, version)
{
  if (url in modmap)
  {
    return modmap[url];
  }
  var mod = new vm.SourceTextModule(source , { context: context, initializeImportMeta:initializeImportMeta, importModuleDynamically:importModuleDynamically, url:url });
  mod.version = version;
  modmap[url] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(linker);
  }
  return mod;
}

function eligibleForFrameWorkCaching(scene, url, name, hash)
{
  if (undefined == enableFrameworkCaching)
  {
    enableFrameworkCaching = scene.sparkSetting("enableFrameWorkCache");
    if (undefined == enableFrameworkCaching)
    {
      enableFrameworkCaching = true;
    }
    if (false == enableFrameworkCaching) {
      keepFrameworksOnExit = false;
    }
  }
  if (enableFrameworkCaching == true)
  {
    // currently enable only for .js file
    if ((/\.js$/.test(url)) && (hash != undefined) && ((url != undefined) || (name != undefined)))
      return true;
  }
  return false;
}

function ensureUniqueFramework(url)
{
  var noentries = frameWorkCache[url].length;
  if (noentries > 1) {
    var entriestoremove = []
    for (var i=0; i<noentries; i++) {
      if (frameWorkCache[url][i].numAppsUsing == 0)
      {
        entriestoremove.push(i)
      }
    }
    for (var i=0; i<entriestoremove.length-1; i++) {
      var index = entriestoremove[i];
      for (key in frameWorkCache[url][index]) {
        delete frameWorkCache[url][index][key]
      }
    }
    for (var i=0; i<entriestoremove.length-1; i++) {
      var index = entriestoremove[i];
      index = index-i;
      frameWorkCache[url].splice(entriestoremove[index], 1);
    }
    entriestoremove = []
  }
}

function getCachePosition(url, hash)
{
  var cacheposition = -1;
  if (frameWorkCache[url] != undefined)
  {
    for (var i=0; i<frameWorkCache[url].length; i++) {
      if (frameWorkCache[url][i].hash == hash)
      {
        cacheposition = i;
        break;
      }
    }
/*
    // means we got a requirement for a framework version not in cache
    // removing unsed earlier to handle any scenarios to unwanted memory removal
    ensureUniqueFramework(url);
*/
  }
  return cacheposition;
}

async function loadFrameWorks(loadCtx, bootstrapUrl) {
  const list = loadCtx.bootstrap.frameworks;
  var frameWorkUsageInfo = {}
  for (let i = 0; i < list.length; i++) {
    let _framework = list[i];
    let _url = _framework.url || _framework;
    let _name = _framework.name

    let _hash = _framework.md5
    let useFrameWorkCaching = eligibleForFrameWorkCaching(loadCtx.sandbox.global.sparkscene, _url, _name, _hash)

    // store framework compiled scripts only for js files
    if (true == useFrameWorkCaching)
    {
      let _cachekey = (undefined != _name)?_name:_url;
      let _cachePostion = getCachePosition(_cachekey, _hash);
      if (_cachePostion != -1)
      {
        frameWorkUsageInfo[_cachekey] = _cachePostion;
        //console.log("framework "+  _cachekey + " available in cache at postion " + _cachePostion + " , no need to reload !!!!!!!!!!!!!!!!!!!");
      }
      else
      {
        if (!/^(http:|https:|file:)/.test(_url))
          _url = urlmain.resolve(bootstrapUrl, _url);

        var frameWorkSource = await getFile(loadCtx.sandbox.global.sparkscene, _url);
        var frameWorkScript = new vm.Script(frameWorkSource);
        if (undefined == frameWorkCache[_cachekey]) {
          frameWorkCache[_cachekey] = []
        }
        frameWorkCache[_cachekey].push({'hash' : _hash, 'script' : frameWorkScript, 'numAppsUsing' : 0})
        frameWorkUsageInfo[_cachekey] = frameWorkCache[_cachekey].length - 1;
        //console.log("Newly loaded cache for " + _cachekey + " at position " + frameWorkUsageInfo[_cachekey]);
      }
    }
    else
    {
      await importModuleDynamically(_url, {url:bootstrapUrl, context:loadCtx.contextifiedSandbox}, _hash);
    }
  }
  for (var key in frameWorkUsageInfo)
  {
    var pos = frameWorkUsageInfo[key];
    frameWorkCache[key][pos].script.runInContext(loadCtx.contextifiedSandbox);
    frameWorkCache[key][pos].numAppsUsing = frameWorkCache[key][pos].numAppsUsing + 1;
  }
  loadCtx.frameWorkUsageInfo = frameWorkUsageInfo;
}

function ESMLoader(params) {
  this.ctx = params
  this.loadESM = function(filename) {
    var loadCtx = this.ctx;
    loadCtx.frameWorkUsageInfo = {}
    let url = filename2url(filename);
    const loc = /^file:/.test(url) ? url.substring(7) : url;
  
    // When URL is .spark, applicationURL is 'filename'.
    const bootstrapUrl = url;
    if (loadCtx.bootstrap) {
      let appURL = loadCtx.bootstrap.applicationURL;
      if (!/^(http:|https:|file:)/.test(appURL))
        appURL = urlmain.resolve(url, appURL);
      url = appURL;
    }

    // override require to our own require to load files relative to file path
    // TODO: __dirname doesn't change in modules imported dynamically. However, in modules loaded via 'require', it is set correctly.
    loadCtx.sandbox.require = loadCtx.makeRequire(loc).bind(loadCtx);
    loadCtx.sandbox['__dirname'] = path.dirname(loc);
    loadCtx.contextifiedSandbox = vm.createContext(loadCtx.sandbox);
    loadCtx.contextifiedSandbox.modmap = loadCtx.modmap;
    var script = new vm.Script("var sgl = require('webgl.js'); global.sparkwebgl = sparkwebgl = new sgl.WebGLRenderingContext(global.sparkscene); global.sparkgles2 = sparkgles2 = require('gles2.js');");
    script.runInContext(loadCtx.contextifiedSandbox);
    script = null; 
    try {
      (async () => {
        let instantiated = false;
        try {
          // When URL is .spark, frameworkURL-s are loaded like dynamic imports.
          if (loadCtx.bootstrap && loadCtx.bootstrap.frameworks) {
            await loadFrameWorks(loadCtx, bootstrapUrl);
          }
          var source = await getFile(loadCtx.global.sparkscene, url);
          loadCtx.app = await loadMjs(source, url, loadCtx.contextifiedSandbox, loadCtx.modmap);
          source = null;
          loadCtx.app.instantiate();
          instantiated = true;
          loadCtx.succeeded = true;
          loadCtx.makeReady(true, loadCtx.app.namespace);
          loadCtx.global.beginDrawing();
          await loadCtx.app.evaluate();
          loadCtx.global.endDrawing();
        } catch (err) {
          console.log("load mjs module failed ");
          console.log(err);
          if (false === instantiated) {
            loadCtx.makeReady(false, {});
          }
        }
      })();
    } catch (err) {
      console.log(err);
    }
  }
  this.clearResources = function() {
    if (enableFrameworkCaching == true)
    {
      if (undefined == keepFrameworksOnExit)
      {
        keepFrameworksOnExit = this.ctx.global.sparkscene.sparkSetting("keepFrameworksOnExit");
        if (undefined == keepFrameworksOnExit)
        {
          keepFrameworksOnExit = true;
        }
      }
      if (undefined != this.ctx.frameWorkUsageInfo) {
        for (key in this.ctx.frameWorkUsageInfo)
        {
          var pos = this.ctx.frameWorkUsageInfo[key];
          frameWorkCache[key][pos].numAppsUsing--;
          if (false == keepFrameworksOnExit) {
            // not deleting the cache entry if someone is using
            if (frameWorkCache[key][pos].numAppsUsing == 0)
            {
              for(var k in frameWorkCache[key][pos]) 
              { 
                delete frameWorkCache[key][pos][k]; 
              }
              frameWorkCache[key].splice(pos, 1);
            }
            if (frameWorkCache[key].length == 0)
            {
              delete frameWorkCache[key];
            }
          }
          else {
            ensureUniqueFramework(key);
          }
        }
        this.ctx.frameWorkUsageInfo = {}
        delete this.ctx.frameWorkUsageInfo;
      }
    }
    else {
      console.log("no framework manipulation !!!!!");
    }
    this.ctx = null
  }
}
module.exports = ESMLoader
