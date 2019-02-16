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
var keys = require("rcvrcore/tools/keys.js");
var queryStringModule = require("querystring");
var urlModule = require("url");

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

function resolveSceneUrl(url) {
  if (url && url.toLowerCase().indexOf('.js?') > 0) { // this is a js file with query params
    return url;
  }
  if (url && ((!url.match(/\.js$/)) && (!url.match(/\.jar$/)))) {
    url = 'mimeScene.js?url=' + url;
  }
  return url;
}

function Shell(scene, urlparams, packageUrl)
{
  this.showFPS = false;
  this.scene = scene;
  this.keys = keys;
  this.packageUrl = packageUrl;
  this.blackBg = scene.create({t:"rect", fillColor:0x000000ff,x:0,y:0,w:1280,h:720,a:0,parent:scene.root});
  this.childScene = null;
  this.url = "";

  this.releaseResources = function() {
    if (!isDuk && !isV8) {
        process.removeListener("uncaughtException", uncaughtException);
        process.removeListener("unhandledRejection", unhandledRejection);
    }
  }

  this.updateSize = function(w, h)
  {
    this.childScene.w = w;
    this.childScene.h = h;
    this.blackBg.w    = w;
    this.blackBg.h    = h;
  }

  this.registerSceneEvents = function() {
    if (!isDuk && !isV8) {                            
      process.on('uncaughtException', uncaughtException);
      process.on('unhandledRejection', unhandledRejection);
    }  
    this.scene.on("onClose",this.releaseResources);
    this.scene.on("onResize", function(e) { this.updateSize(e.w, e.h);}.bind(this));
    this.scene.on("onFPS", function(e) {
      if(this.showFPS) {
        this.fpsCounter.text = ""+Math.floor(e.fps)+"fps";
        this.fpsBg.w = this.fpsCounter.w+16;
        this.fpsBg.h = this.fpsCounter.h;
      }
    });
  }

  this.registerKeyEvents = function() {
    var _this = this;
    ////
    if( this.scene.capabilities != undefined && this.scene.capabilities.graphics != undefined && this.scene.capabilities.graphics.cursor == 1) {
      // TODO Cursor emulation mostly for egl targets right now.
        var fadeCursorTimer = undefined;
    
        var cursor = this.scene.create({t:"image", url:"cursor.png",parent:this.scene.root,
                                   interactive:false, a:0});
    
        this.scene.on("onMouseMove", function(e) {
          if (fadeCursorTimer != undefined) {
            clearTimeout(fadeCursorTimer);
          }
          fadeCursorTimer = setTimeout( function()
          {
            cursor.animate({a:0}, 2, _this.scene.animation.TWEEN_LINEAR, _this.scene.animation.LOOP,1);
          }, 5000);
          cursor.a = 1.0;
          cursor.x = e.x-40;
          cursor.y = e.y-16;
        });
    }
    
    ////
     if( this.scene.capabilities != undefined && this.scene.capabilities.graphics != undefined && this.scene.capabilities.graphics.dirtyRect == 1) {
         // TODO  emulation mostly for egl targets right now.
         var dirtyRectsTimer = undefined;
    
          var dirtyRects = this.scene.create({t:"image", url:"cursor.png",parent:this.scene.root,
                                       interactive:false, a:0});
    
          this.scene.on("onMouseMove", function(e) {
           if (dirtyRectsTimer != undefined) {
             clearTimeout(dirtyRectsTimer);
           }
         dirtyRectsTimer = setTimeout( function()
         {
             dirtyRects.animate({a:0}, 2, _this.scene.animation.TWEEN_LINEAR, _this.scene.animation.LOOP,1);
                                                   }, 5000);
                      dirtyRects.a = 1.0;
                      dirtyRects.x = e.x-23;
                      dirtyRects.y = e.y-10;
                      });
         }
     ////
    ////
      this.scene.root.on("onPreKeyDown", function(e) {
        var code  = e.keyCode;
        var flags = e.flags;
    
        var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
        if (loggingDisabled && loggingDisabled === '1'){
          console.log("onPreKeyDown value hidden");
        } else {
          console.log("SHELL: onPreKeyDown:", code, " key: ", keys.name(code), ", ", flags);
        }
    
        if( _this.keys.is_CTRL_ALT( flags ) )
        {
          if(code == _this.keys.Y)  // ctrl-alt-y
          {
    //        console.log("SHELL: onPreKeyDown: FPS !!!  ############# ");
    
            _this.showFPS = !_this.showFPS;
            _this.fpsBg.a = (_this.showFPS)?1.0:0;
            e.stopPropagation();
          }
          else
          if(code == _this.keys.O)  // ctrl-alt-o
          {
    //        console.log("SHELL: onPreKeyDown: showOutlines !!!  ############# ");
    
            this.scene.showOutlines = !this.scene.showOutlines;
            e.stopPropagation();
          }
          else
          if (code == _this.keys.S)  // ctrl-alt-s
          {
            if (!isDuk && !isV8) {
            // This returns a data URI string with the image data
            var dataURI = _this.scene.screenshot('image/png;base64');
    
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
          if(code == _this.keys.D)  // ctrl-alt-d
          {
            // console.log("SHELL: onPreKeyDown: show dirty rect !!!  ############# ");
    
            _this.scene.showDirtyRect = !_this.scene.showDirtyRect;
            e.stopPropagation();
          }
          else
          if(code == _this.keys.T)  // ctrl-alt-t
          {
            _this.scene.enableDirtyRect = !_this.scene.enableDirtyRect;
            e.stopPropagation();
          }
        }// ctrl-alt
        else
        if( _this.keys.is_CTRL_ALT_SHIFT( flags ) )
        {
          if(code == _this.keys.R)  // ctrl-alt-shft-r
          {
            // console.log("SHELL: onPreKeyDown: Reloading url [ "+originalURL+" ] !!!  ############# ");
    
            console.log("Reloading url: ", _this.originalURL);
            _this.childScene.url = _this.originalURL;
            e.stopPropagation();
          }
          else
          if(code == _this.keys.H)  // ctrl-alt-shft-h
          {
            // console.log("SHELL: onPreKeyDown: Loading HOME url [ "+"browser.js"+" ] !!!  ############# ");
    
            var homeURL = "browser.js";
            console.log("Loading home url: ", homeURL);
            _this.childScene.url = homeURL;
            e.stopPropagation();
          }
          else
          if(code == _this.keys.D)  // ctrl-alt-shft-d
          {
            _this.scene.logDebugMetrics();
          }
        }// ctrl-alt-shift
      });
    
      this.scene.root.on("onPreKeyUp", function(e)
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
             if (code == _this.keys.Y && _this.keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-y
        else if (code == _this.keys.O && _this.keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-o
        else if (code == _this.keys.S && _this.keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-s
        else if (code == _this.keys.D && _this.keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-d
        else if (code == _this.keys.T && _this.keys.is_CTRL_ALT( flags ) )       e.stopPropagation(); // ctrl-alt-t
        else if (code == _this.keys.R && _this.keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-r
        else if (code == _this.keys.H && _this.keys.is_CTRL_ALT_SHIFT( flags ) ) e.stopPropagation(); // ctrl-alt-shift-h
      });
    
      if (true)
      {
        this.scene.root.on("onKeyDown", function(e)
        {
          var code = e.keyCode; var flags = e.flags;
          var loggingDisabled = process.env.PXSCENE_KEY_LOGGING_DISABLED;
          if (loggingDisabled && loggingDisabled === '1'){
            console.log("onKeyDown value hidden");
          } else {
            console.log("onKeyDown shell:", code, ", ", flags);
          }
    
          if( _this.keys.is_CTRL_ALT( flags ) )
          {
            if(code == _this.keys.R)   // ctrl-alt-r
            {
              console.log("(shell.js) Reloading url: ", _this.originalURL);
              _this.childScene.url = _this.originalURL;
              e.stopPropagation();
            }
            else
            if (code == _this.keys.H)  // ctrl-alt-h
            {
              var homeURL = "browser.js";
              console.log("Loading home url: ", homeURL);
              _this.childScene.url = homeURL;
              e.stopPropagation();
            }
          }// ctrl-alt
        });
      }
    
      this.scene.root.on("onPreChar", function(e)
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
  }

  this.registerSceneEvents();
  this.registerKeyEvents();

  this.loadUrl(urlparams.url);
  this.updateSize(this.scene.w, this.scene.h);
}

Shell.prototype.loadUrl = function(url) {
    var url = queryStringModule.parse(urlModule.parse(this.packageUrl).query).url;
    url = resolveSceneUrl(url);
    this.originalURL = (!url || url==="") ? "browser.js":url;
    this.url = this.originalURL;
    this.childScene = this.scene.create({t:"scene", url:this.originalURL,parent:this.scene.root});
    this.childScene.focus = true;

    this.childScene.ready.then( function()
    {
        // Use delegate if present...
        //
        if (typeof this.childScene.api                  !== undefined &&
            typeof this.childScene.api.wantsClearscreen === 'function')
        {
          if(this.childScene.api.wantsClearscreen()) // use delegate preference - returns bool
            this.blackBg.a = 1;

        }
        else {
            this.blackBg.a = 1;
        }
    }.bind(this));

    this.fpsBg      = this.scene.create({t:"rect",     fillColor:0x00000080,lineColor:0xffff0080,lineWidth:3,x:10,y:10,a:this.showFPS?1:0,parent:this.scene.root});
    this.fpsCounter = this.scene.create({t:"text", x:5,textColor:0xffffffff,pixelSize:24,text:"0fps",parent:this.fpsBg});
    this.fpsBg.w = this.fpsCounter.w+16;
    this.fpsBg.h = this.fpsCounter.h;
    this.fpsBg.interactive      = false;
    this.fpsCounter.interactive = false;
}

module.exports = Shell;
