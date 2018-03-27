var isDuk=(typeof Duktape != "undefined")?true:false;
var keys = require("rcvrcore/tools/keys.js");
var fs = require("fs");

function uncaughtException(err) {
  if (!isDuk) {
    console.log("Received uncaught exception " + err.stack);
  }
}

function Shell(scene, urlparams)
{
  this.showFPS = false;
  this.scene = scene;
  this.keys = keys;
  this.url = "";
  this.originalURL = "";
  this.blackBg = this.scene.create({t:"rect", fillColor:0x000000ff,lineColor:0xffff0080,lineWidth:0,x:0,y:0,w:1280,h:720,a:0,parent:this.scene.root});
  this.childScene = null;
  
  this.releaseResources = function() {
    console.log("release resources called !!!!!!!!!!!!!!!!!!!!!!!!!!!");
    if (!isDuk) {
        process.removeListener("uncaughtException", uncaughtException);
    }
  }

  this.updateSize = function (w, h) {
    this.childScene.w = w;
    this.childScene.h = h;
    this.blackBg.w = w;
    this.blackBg.h = h;
  }

  this.registerSceneEvents = function() {
    if (!isDuk) {
      process.on('uncaughtException', uncaughtException);
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
    _this.scene.root.on("onPreKeyDown", function(e) {
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
          _this.showFPS = !_this.showFPS;
          _this.fpsBg.a = (_this.showFPS)?1.0:0;
          e.stopPropagation();
        }
        else
        if(code == keys.O)  // ctrl-alt-o
        {
//          console.log("SHELL: onPreKeyDown: showOutlines !!!  ############# ");

          _this.scene.showOutlines = !_this.scene.showOutlines;
          e.stopPropagation();
        }
        else
        if (code == keys.S)  // ctrl-alt-s
        {
          if (!isDuk) {
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
        if(code == keys.D)  // ctrl-alt-d
        {
          // console.log("SHELL: onPreKeyDown: show dirty rect !!!  ############# ");

          _this.scene.showDirtyRect = !_this.scene.showDirtyRect;
          e.stopPropagation();
        }
      }// ctrl-alt
      else
      if( keys.is_CTRL_ALT_SHIFT( flags ) )
      {
        if(code == keys.R)  // ctrl-alt-shft-r
        {
          // console.log("SHELL: onPreKeyDown: Reloading url [ "+originalURL+" ] !!!  ############# ");

          console.log("Reloading url: ", _this.url);
          _this.childScene.url = _this.url;
          e.stopPropagation();
        }
        else
        if(code == keys.H)  // ctrl-alt-shft-h
        {
          // console.log("SHELL: onPreKeyDown: Loading HOME url [ "+"browser.js"+" ] !!!  ############# ");

          var homeURL = "browser.js";
          console.log("Loading home url: ", homeURL);
          _this.childScene.url = homeURL;
          e.stopPropagation();
        }
        else
        if(code == keys.D)  // ctrl-alt-shft-d
        {
          _this.scene.logDebugMetrics();
        }
      }// ctrl-alt-shift
    });

    _this.scene.root.on("onPreKeyUp", function(e)
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

    _this.scene.root.on("onKeyDown", function(e)
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
          console.log("(shell.js) Reloading url: ", _this.originalURL);
          _this.childScene.url = _this.originalURL;
          e.stopPropagation();
        }
        else
        if (code == keys.H)  // ctrl-alt-h
        {
          var homeURL = "browser.js";
          console.log("Loading home url: ", homeURL);
          _this.childScene.url = homeURL;
          e.stopPropagation();
        }
      }// ctrl-alt
    });

    _this.scene.root.on("onPreChar", function(e)
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

  //register the key events and scene related events for this shell
  this.registerSceneEvents();
  this.registerKeyEvents();

  this.loadUrl(urlparams.url);
  this.updateSize(this.scene.w, this.scene.h);

}

Shell.prototype.loadUrl = function(url) {
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

    this.fpsBg      = this.scene.create({t:"rect", fillColor:0x00000080,lineColor:0xffff0080,lineWidth:3,x:10,y:10,a:this.showFPS?1:0,parent:this.scene.root});
    this.fpsCounter = this.scene.create({t:"text", x:5,textColor:0xffffffff,pixelSize:24,text:"0fps",parent:this.fpsBg});
    this.fpsBg.w = this.fpsCounter.w+16;
    this.fpsBg.h = this.fpsCounter.h;
    // Prevent interaction with scenes...
    this.fpsBg.interactive = false;
    this.fpsCounter.interactive = false;
}

module.exports = Shell;
