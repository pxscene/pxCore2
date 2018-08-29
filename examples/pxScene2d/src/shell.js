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
var isV8 = (typeof _isV8 != "undefined")?true:false;

px.import({ scene: 'px:scene.1.js',
             keys: 'px:tools.keys.js'
}).then( function ready(imports)
{
  var scene = imports.scene;
  var keys  = imports.keys;

  function uncaughtException(err) {
    if (!isDuk && !isV8) {
      console.log("Received uncaught exception " + err.stack);
    }
  }
  function unhandledRejection(err) {
    if (!isDuk && !isV8) {
      console.log("Received uncaught rejection.... " + err);
    }
  }
  if (!isDuk && !isV8) {
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

  var showFPS = false;
  var fpsBg      = scene.create({t:"rect",     fillColor:0x00000080,lineColor:0xffff0080,lineWidth:3,x:10,y:10,a:showFPS?1:0,parent:scene.root});
  var fpsCounter = scene.create({t:"text", x:5,textColor:0xffffffff,pixelSize:24,text:"0fps",parent:fpsBg});
  fpsBg.w = fpsCounter.w+16;
  fpsBg.h = fpsCounter.h;

  // Prevent interaction with scenes...
  fpsBg.interactive      = false;
  fpsCounter.interactive = false;

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

  scene.on("onFPS", function(e) {
    if(showFPS) {
      fpsCounter.text = ""+Math.floor(e.fps)+"fps";
      fpsBg.w = fpsCounter.w+16;
      fpsBg.h = fpsCounter.h;
    }
  });

////
if (false)
{
  // TODO Cursor emulation mostly for egl targets right now.

  // TODO hacky raspberry pi detection
  var os = require("os");
  var hostname = os.hostname();

  if (hostname == "raspberrypi") {
    var cursor = scene.create({t:"image", url:"cursor.png",parent:scene.root,
                               interactive:false});

    scene.on("onMouseMove", function(e) {
      cursor.x = e.x-23;
      cursor.y = e.y-10;
    });
  }
}
////
  scene.root.on("onPreKeyDown", function(e) {
    var code  = e.keyCode;
    var flags = e.flags;

    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      console.log("onPreKeyDown value hidden");
    } else {
      console.log("SHELL: onPreKeyDown:", code, " key: ", keys.name(code), ", ", flags);
    }

    if( keys.is_CTRL_ALT( flags ) )
    {
      if(code == keys.Y)  // ctrl-alt-y
      {
//        console.log("SHELL: onPreKeyDown: FPS !!!  ############# ");

        showFPS = !showFPS;
        fpsBg.a = (showFPS)?1.0:0;
        e.stopPropagation();
      }
      else
      if(code == keys.O)  // ctrl-alt-o
      {
//        console.log("SHELL: onPreKeyDown: showOutlines !!!  ############# ");

        scene.showOutlines = !scene.showOutlines;
        e.stopPropagation();
      }
      else
      if (code == keys.S)  // ctrl-alt-s
      {
        if (!isDuk && !isV8) {
        // This returns a data URI string with the image data
        var dataURI = scene.screenshot('image/png;base64');

        // convert the data URI by stripping off the scheme and type information
        // to a base64 encoded string with just the PNG image data
        var base64PNGData = dataURI.slice(dataURI.indexOf(',')+1);

        // decode the base64 data and write it to a file
        fs.writeFile("screenshot.png", new Buffer(base64PNGData, 'base64'), function(err)
        {
          if (err)
            console.log("Error creating screenshot.png");
          else
            console.log("Created screenshot.png");
        });
      }
        e.stopPropagation();
      }
      else
      if(code == keys.D)  // ctrl-alt-d
      {
        // console.log("SHELL: onPreKeyDown: show dirty rect !!!  ############# ");

        scene.showDirtyRect = !scene.showDirtyRect;
        e.stopPropagation();
      }
    }// ctrl-alt
    else
    if( keys.is_CTRL_ALT_SHIFT( flags ) )
    {
      if(code == keys.R)  // ctrl-alt-shft-r
      {
        // console.log("SHELL: onPreKeyDown: Reloading url [ "+originalURL+" ] !!!  ############# ");

        console.log("Reloading url: ", originalURL);
        childScene.url = originalURL;
        e.stopPropagation();
      }
      else
      if(code == keys.H)  // ctrl-alt-shft-h
      {
        // console.log("SHELL: onPreKeyDown: Loading HOME url [ "+"browser.js"+" ] !!!  ############# ");

        var homeURL = "browser.js";
        console.log("Loading home url: ", homeURL);
        childScene.url = homeURL;
        e.stopPropagation();
      }
      else
      if(code == keys.D)  // ctrl-alt-shft-d
      {
        scene.logDebugMetrics();
      }
    }// ctrl-alt-shift
  });

  scene.root.on("onPreKeyUp", function(e)
  {
    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      console.log("onPreKeyUp value hidden");
    } else {
      console.log("in onPreKeyUp", e.keyCode, e.flags);
    }
    var code  = e.keyCode;
    var flags = e.flags;

    if (loggingDisabled && loggingDisabled === '1'){
      console.log("onKeyUp value hidden");
    } else {
      console.log("onKeyUp:", code, ", ", flags);
    }

    // eat the ones we handle here
         if (code == keys.Y && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-y
    else if (code == keys.O && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-o
    else if (code == keys.S && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-s
    else if (code == keys.D && keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-d
    else if (code == keys.R && keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-r
    else if (code == keys.H && keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-h
  });

  if (true)
  {
    scene.root.on("onKeyDown", function(e)
    {
      var code = e.keyCode; var flags = e.flags;
      var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
      if (loggingDisabled && loggingDisabled === '1'){
        console.log("onKeyDown value hidden");
      } else {
        console.log("onKeyDown shell:", code, ", ", flags);
      }

      if( keys.is_CTRL_ALT( flags ) )
      {
        if(code == keys.R)   // ctrl-alt-r
        {
          console.log("(shell.js) Reloading url: ", originalURL);
          childScene.url = originalURL;
          e.stopPropagation();
        }
        else
        if (code == keys.H)  // ctrl-alt-h
        {
          var homeURL = "browser.js";
          console.log("Loading home url: ", homeURL);
          childScene.url = homeURL;
          e.stopPropagation();
        }
      }// ctrl-alt
    });
  }

  scene.root.on("onPreChar", function(e)
  {
    console.log("in onchar");
    var c = e.charCode;
    var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
    if (loggingDisabled && loggingDisabled === '1'){
      console.log("onChar value hidden");
    } else {
      console.log("onChar:", c);
    }
    // TODO eating some "undesired" chars for now... need to redo this
    if (c<32) {
      console.log("stop onChar");
      e.stopPropagation();
    }
  });

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
    if (!isDuk && !isV8) {
        process.removeListener("uncaughtException", uncaughtException);
        process.removeListener("unhandledRejection", unhandledRejection);
    }
  }

  scene.on("onClose",releaseResources);
});
