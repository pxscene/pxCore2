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

var isDuk=(typeof Duktape != "undefined")?true:false;

var includeDebug = true;
if (includeDebug) {
  px.import({ debug: 'debug.js'}).then( function ready(imports)
  {
    imports.debug.init();
  });
}

px.import({ scene: 'px:scene.1.js'}).then( function ready(imports)
{
  var scene = imports.scene;

  function uncaughtException(err) {
    if (!isDuk) {
      console.log("Received uncaught exception " + err.stack);
    }
  }
  function unhandledRejection(err) {
    if (!isDuk) {
      console.log("Received uncaught rejection.... " + err);
    }
  }
  if (!isDuk) {
    process.on('uncaughtException', uncaughtException);
    process.on('unhandledRejection', unhandledRejection);
  }


  // JRJR TODO had to add more modules
  var url = queryStringModule.parse(urlModule.parse(module.appSceneContext.packageUrl).query).url;
  var originalURL = (!url || url==="") ? "browser.js":url;
  console.log("url:",originalURL);

  var    blackBg = scene.create({t:"rect", fillColor:0x000000ff,x:0,y:0,w:1280,h:720,a:0,parent:scene.root});
  var childScene = scene.create({t:"scene", url: originalURL, parent:scene.root});
  childScene.focus = true;

  function updateSize(w, h)
  {
    childScene.w = w;
    childScene.h = h;
    blackBg.w    = w;
    blackBg.h    = h;
  }

  // TODO if I log out event object e... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});

  // updateSize(scene.w, scene.h);

  // TODO if I log out event object ... there is extra stuff??
  scene.on("onResize", function(e) { updateSize(e.w, e.h);});
  updateSize(scene.w, scene.h);

  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Test for Clearscreen delegate...
  childScene.ready.then( function()
  {
      // Use delegate if present...
      //
      if (typeof childScene.api                  !== undefined &&
          typeof childScene.api.wantsClearscreen === 'function')
      {
        if(childScene.api.wantsClearscreen()) // use delegate preference - returns bool
          blackBg.a = 1; 

      }
      else {
          blackBg.a = 1; 
      }
  });
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // Example of 'delegate' for childScene.
  //
  // Returns:   BOOL preference of 'shell' performing Clearscreen per frame.
  /*
  module.exports.wantsClearscreen = function()  // delegate
  {
    return false;
  };
  */
  // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  function releaseResources() {
    if (!isDuk) {
      process.removeListener("uncaughtException", uncaughtException);
      process.removeListener("unhandledRejection", unhandledRejection);
    }
  }

  scene.on("onClose",releaseResources);
});
