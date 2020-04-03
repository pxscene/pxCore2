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

class Framework {
  constructor(module, specifier) {
    this.module = module;
    this.filename = specifier.substring(specifier.lastIndexOf('/')+1);
    this.use = 1;
  }
}

// default value is true for below parameters if not defined in spark settings
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
var loadJavaScriptModule = async function (source, specifier, ctx)
{
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
  var mod;
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

/* iife, or umd */
var loadFrameworkModule = async function (source, specifier, ctx, version)
{
  // console.log(`loading framework '${specifier}' ver.'${version}'`);
  var mod = new vm.Script(source);
  mod.runInContext(ctx);
  if (typeof version === "string" && version !== "") {
    let framework = new Framework(mod, specifier);
    Object.keys(frameWorkCache).forEach(key => {
      if (frameWorkCache[key].filename === framework.filename && frameWorkCache[key].use <= 0) {
        // console.log(`replace framework '${framework.filename}' ver.'${key}' => '${version}'`);
        delete frameWorkCache[key];
      }
    });
    frameWorkCache[version] = framework;
  } else {
    ctx.modmap[specifier] = mod
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

class ModuleItem {
  constructor(specifier, referencingModule, version) {
    let isCommonJSModule = false
    let isFramework = version !== undefined && version !== null

    if (specifier.indexOf("wpe-lightning-spark") === 0) {
      specifier = path.resolve(process.cwd(), "node_modules/wpe-lightning-spark/src/lightning.mjs");
    } else if (specifier.indexOf("wpe-lightning") === 0) {
      specifier = path.resolve(process.cwd(), "node_modules/" + specifier);
    }

    // The following code is default.
    // Specifier can be a relative, absolute path, or URL. Convert to URL, for consistency.
    else {
      if (!/^(http:|https:|file:)/.test(specifier)) {
        if (fs.existsSync("node_modules/" + specifier)) {
          isCommonJSModule = true
        } else {
          specifier = urlmain.resolve(referencingModule.url, specifier);
          const ext = path.extname(specifier);
          if (!/^(\.js|\.mjs)$/.test(ext))
            specifier += '.mjs';
        }
      }
    }
    if (!/^(http:|https:|file:)/.test(specifier) && !isCommonJSModule)
      specifier = `file://${specifier}`;

    this.specifier = specifier;
    this.refModule = referencingModule;
    this.version = version;
    this.isCommonJSModule = isCommonJSModule;
    this.isFramework = isFramework;

    // If the module has been loaded already, return it.
    if (specifier in referencingModule.context.modmap) {
      // console.log(`found module '${specifier}' in cache`);
      this.module = referencingModule.context.modmap[specifier];
    }

    // Same file from different domains is shared if has 'version'.
    else if (isFramework) {
      if (undefined === enableFrameworkCaching) {
        enableFrameworkCaching = referencingModule.context.global.sparkscene.sparkSetting("enableFrameWorkCache");
        if (undefined === enableFrameworkCaching) {
          enableFrameworkCaching = true;
        }
        if (false === enableFrameworkCaching) {
          keepFrameworksOnExit = false;
        }
      }
      if (enableFrameworkCaching && version && (version in frameWorkCache)) {
        // console.log(`found framework '${specifier}' ver.'${version}' in cache`);
        this.module = frameWorkCache[version];
      }
    }
  }

  async download() {
    if (!this.module && !this.source && !/\.node$/.test(this.specifier) && !this.isCommonJSModule) {
      this.source = await getFile(this.refModule.context.global.sparkscene, this.specifier)
    }
  }

  async load() {
    if (this.module instanceof Framework) {
      this.module.module.runInContext(this.refModule.context);
      this.module.use++;
      return this.module.module;
    }

    if (/\.node$/.test(this.specifier))
      return await loadNodeModule(this.specifier, this.refModule.context);
    if (this.isCommonJSModule)
      return await loadCommonJSModule(this.specifier, this.refModule.context);

    // Load as .js, .mjs, or .json.
    if (/\.json$/.test(this.specifier))
      return await loadJsonModule(this.source, this.specifier, this.refModule.context);
    if (/\.js$/.test(this.specifier)) {
      if (this.isFramework) {
        if (enableFrameworkCaching)
          return await loadFrameworkModule(this.source, this.specifier, this.refModule.context, this.version);
        return await loadFrameworkModule(this.source, this.specifier, this.refModule.context)
      }
      return await loadJavaScriptModule(this.source, this.specifier, this.refModule.context);
    }
    return await loadMjs(this.source, this.specifier, this.refModule.context, this.refModule.context.modmap);
  }
}

/* load mjs file and returns as ejs module */
var getModule = async function (specifier, referencingModule, version) {
  let mod = new ModuleItem(specifier, referencingModule, version);
  await mod.download();
  return await mod.load();
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

var importModuleDynamically = async function (specifier, referencingModule) {
  var mod = await getModule(specifier,referencingModule);
  mod.instantiate();
  await mod.evaluate();
  return mod;
}

var loadMjs = async function (source, url, context, modmap)
{
  if (url in modmap)
  {
    return modmap[url];
  }
  if (null == context)
  {
    throw "Context is empty, app might got terminated !!!";
  }
  var mod = new vm.SourceTextModule(source , { context: context, initializeImportMeta:initializeImportMeta, importModuleDynamically:importModuleDynamically, url:url });
  modmap[url] = mod;
  if (mod.linkingStatus == 'unlinked')
  {
    var result = await mod.link(linker);
  }
  return mod;
}

function ESMLoader(params) {
  this.ctx = params
  this.loadESM = function(filename) {
    var loadCtx = this.ctx;
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
    loadCtx.sandbox.global.__dirname = loadCtx.sandbox.__dirname;
    loadCtx.contextifiedSandbox = vm.createContext(loadCtx.sandbox);
    loadCtx.contextifiedSandbox.modmap = loadCtx.modmap;
    var script = new vm.Script(
      "var sgl = require('webgl.js');" +
      "global.sparkwebgl = sparkwebgl = new sgl.WebGLRenderingContext(global.sparkscene);" +
      "global.sparkgles2 = sparkgles2 = require('gles2.js');" +
      "global.fetch = fetch = require('node-fetch');" +
      "global.Headers = Headers = global.fetch.Headers;" +
      "global.Request = Request = global.fetch.Request;" +
      "global.Response = Response = global.fetch.Response;" +
      "global.WebSocket = WebSocket = require('ws');" +
      "global.window = window = {};"
    );
    
    script.runInContext(loadCtx.contextifiedSandbox);
    script = null; 
    try {
      (async () => {
        let instantiated = false;
        try {
          // When URL is .spark, frameworkURL-s are loaded like dynamic imports.
          if (loadCtx.bootstrap && loadCtx.bootstrap.frameworks) {
            const list = loadCtx.bootstrap.frameworks;
            const modules = [];
            for (let i = 0; i < list.length; i++) {
              let _framework = list[i];
              let _url = _framework.url || _framework;
              let _version = _framework.md5 || "";
              modules.push(new ModuleItem(_url, {
                url:bootstrapUrl,
                context:loadCtx.contextifiedSandbox
              }, _version));
            }
            await Promise.all(modules.map(m => m.download()));
            for (let i = 0; i < modules.length; i++) {
              await modules[i].load();
            }
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
          if ((false === instantiated) && (null != loadCtx)) {
            if ((null != loadCtx) && (null != loadCtx.makeReady)) {
              loadCtx.makeReady(false, {});
            }
          }
        }
      })();
    } catch (err) {
      console.log(err);
    }
  }
  this.clearResources = function() {
    if (enableFrameworkCaching) {
      if (undefined === keepFrameworksOnExit) {
        keepFrameworksOnExit = this.ctx.global.sparkscene.sparkSetting("keepFrameworksOnExit")
        if (undefined === keepFrameworksOnExit) {
          keepFrameworksOnExit = true
        }
      }
      if (!keepFrameworksOnExit) {
        // console.log(`delete all frameworks on exit`);
        frameWorkCache = {}
      } else {
        if (this.ctx.bootstrap && this.ctx.bootstrap.frameworks) {
          this.ctx.bootstrap.frameworks.forEach(i => {
            if (i.md5 && i.md5 in frameWorkCache) {
              let framework = frameWorkCache[i.md5];
              if (--framework.use <= 0 && Object.values(frameWorkCache).some(j => j !== framework && j.filename === framework.filename)) {
                // console.log(`delete framework '${framework.filename}' ver.'${i.md5}'`);
                delete frameWorkCache[i.md5];
              }
            }
          });
        }
      }
    }
    this.ctx = null
  }
}
module.exports = ESMLoader
