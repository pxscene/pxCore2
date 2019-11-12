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

var loadHttpFile = function(scene, fileUri) {
  return new Promise(function(resolve, reject) {
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
  });
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
var loadJavaScriptModule = async function (source, specifier, ctx)
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
var getModule = async function (specifier, referencingModule) {
   var __dirname = process.cwd()
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
           let result = await loadHttpFile(referencingModule.context.global.sparkscene, specifier);
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
           let result = await loadHttpFile(referencingModule.context.global.sparkscene, specifier);
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
       if (specifier in referencingModule.context.modmap)
       {
         mod = referencingModule.context.modmap[specifier];
       }
       else {
       var source;
       try {
         if (isLocalApp || (true == treatAsLocal)) {
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadMjs(source, specifier, referencingModule.context, referencingModule.context.modmap);
         }
         else
         {
           specifier = "http://" + specifier;
           let result = await loadHttpFile(referencingModule.context.global.sparkscene, specifier);
           mod = loadMjs(result, specifier, referencingModule.context, referencingModule.context.modmap);
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
     if (specifier in referencingModule.context.modmap)
     {
       mod = referencingModule.context.modmap[specifier];
     }
     else {
       // search for ux.mjs or ux.js
       var source;
       try {
         if (isLocalApp) {
           source = await readFileAsync(specifier, {'encoding' : 'utf-8'})
           specifier = "file://" + specifier;
           mod = loadMjs(source, specifier, referencingModule.context, referencingModule.context.modmap);
         }
         else
         {
           //http module read
           specifier = "http://" + specifier;
           let result = await loadHttpFile(referencingModule.context.global.sparkscene, specifier);
           mod = loadMjs(result, specifier, referencingModule.context, referencingModule.context.modmap);
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

var linker = async function (specifier, referencingModule) {
  try {
  var mod = await getModule(specifier, referencingModule);
  return mod;
  } catch(err) {
    console.log("linker error !!!!!!!!!!!!!!!!!");
    console.log(err);
  }
}

var importModuleDynamically = async function (specifier, { url }) {
  var mod = await getModule(specifier,{ url });
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
    const url = filename2url(filename);
    const loc = /^file:/.test(url) ? url.substring(7) : url;
  
    // override require to our own require to load files relative to file path
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
          if (loadCtx._frameworkURL) {
            const url2 = filename2url(loadCtx._frameworkURL);
            const loc2 = /^file:/.test(url2) ? url2.substring(7) : url2;
            const source2 = await getFile(loadCtx.global.sparkscene,url2);
            // use paths for frameworkURL
            loadCtx.sandbox.require = loadCtx.makeRequire(loc2).bind(loadCtx);
            loadCtx.sandbox['__dirname'] = path.dirname(loc2);
            vm.runInContext(source2, loadCtx.contextifiedSandbox, {filename:loc2});
            // restore previous values
            loadCtx.sandbox.require = loadCtx.makeRequire(loc).bind(loadCtx);
            loadCtx.sandbox['__dirname'] = path.dirname(loc);
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
  
          if (typeof loadCtx.app.namespace.default === 'function') {
            try {
              if (params._options) {
                new loadCtx.app.namespace.default(params._options);
              } else {
                new loadCtx.app.namespace.default();
              }
            } catch (err) {
              console.log(err);
            }
          }
  
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
    this.ctx = null
  }
}
module.exports = ESMLoader
