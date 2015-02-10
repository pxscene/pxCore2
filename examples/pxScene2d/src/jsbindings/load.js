var px = require("./build/Debug/px");
var http = require('http');
var util = require('util');

function Api(scene) {
  this._scene = scene;
}

Api.prototype.destroyNodeFromScene = function(scene, node) {
  if (node) {
    var children = node.children;
    if (children) {
      var n = children.length;
      for (var i = 0; i < n; ++i) {
        this.destroyNodeFromScene(scene, node.getChild(0));
      }
    }
    node.remove();
  }
}

Api.prototype.destroyScene = function(scene) {
  if (scene) {
    this.destroyNodeFromScene(scene, scene.root);
    scene.url = "";
    scene.parent = null;
  }
}

Api.prototype.loadScriptContents = function(uri, closure) {
  var fs = require('fs');
  var url = require('url');

  var code = '';

  if (uri.substring(0, 4) == "http") {
    var options = url.parse(uri);
    if (uri.substring(0, 5) == "https") {
      throw 'not supported'
    }
    else {
      var req = http.get(options, function(res) {
        res.on('data',  function(data)  { code += data; });
        res.on('end',   function()      { closure(code, null); });
      });
      req.on('error',   function(err)   { closure('', err); });
    }
  }
  else {
    var infile = fs.createReadStream(uri);
    infile.on('data',  function(data)   { code += data; });
    infile.on('end',   function()       { closure(code, null); });
    infile.on('error', function(err)    { closure('', err); });
  }
}

Api.prototype.loadScriptForScene = function(container, scene, uri) {
  var sceneForChild = scene;
  var apiForChild = this;

  var code;

  try {
    code = this.loadScriptContents(uri, function(code, err) {
      var vm = require('vm');
      var sandbox = {
        console   : console,
        scene     : sceneForChild,
        runtime   : apiForChild,
        process   : process
      };

      if (err) {
        console.log("failed to load script:" + uri);
        console.log(err);
        // TODO: scene.onError(err); ???
      }
      else {
        try {
          app = vm.runInNewContext(code, sandbox);
          // TODO do the old scenes context get released when we reload a scenes url??
            
//            scene.ctx = app;

            // TODO part of an experiment to eliminate intermediate rendering of the scene
            // while it is being set up
            if (true) { // enable to fade scenes in
                container.a = 0;
                container.painting = true;
                container.animateTo({a:1}, 0.2, 0, 0);
            }
            else
                container.painting = true;
        }
        catch (err) {
          console.log("failed to run app:" + uri);
          console.log(err);

          // TODO: scene.onError(err); ???
          // TODO: at this point we need to destroy the child scene
          scene.url = "";  // This destroys the child scene and releases scene.ctx
          apiForChild.destroyScene(sandbox.scene);

          sandbox.console = null;
          sandbox.scene = null;
          sandbox.runtime = null;
          sandbox.process = null;

          // console.log(util.inspect(sandbox));
        }
      }
    });
  }
  catch (err) {
    console.log("failed to load script:" + uri);
    console.log(err);
    // TODO: scene.onError(err); ???
  }
}

var scene = px.getScene(0, 0, 800, 400);
var api = new Api(scene);

// register a "global" hook that gets invoked whenever a child scene is created
scene.onScene = function(container, innerscene, url) {
    // TODO part of an experiment to eliminate intermediate rendering of the scene
    // while it is being set up
    // container when returned here has it's painting property set to false.
    // it won't start rendering until we set painting to true which we do 
    // after the script has loaded
    api.loadScriptForScene(container,innerscene, url);
};

var argv = process.argv;

if (argv.length >= 3) {
    var originalURL = argv[2];
    // TODO - WARNING root scene.create* doesn't allow passing in property bags
    var childScene = scene.createScene();
    childScene.url = originalURL;
    childScene.parent = scene.root;
    var fpsCounter = scene.createText();
    fpsCounter.parent = scene.root;
    fpsCounter.x = 20;
    fpsCounter.textColor = 0x000000ff;
    fpsCounter.a = 0;

    function updateSize(w, h) {
        childScene.w = w;
        childScene.h = h;
    }

    scene.on("fps", function(fps) { fpsCounter.text = ""+Math.floor(fps)+"fps"; });

    scene.on("keydown", function(code, flags) {
        console.log("keydown:", code, ", ", flags);
        if (code == 89 && (flags | 48)) {  // ctrl-alt-y
            fpsCounter.a = (fpsCounter.a==0)?1:0;
        }
        else if (code == 82 && (flags | 16)) {  // ctrl-r
            console.log("Reloading url: ", originalURL);
            childScene.url = originalURL;
        }
        else { 
            // forward event
// TODO would be nice if apply worked on rtFunctions so we could forward all arguments more automatically
//            var args = ["keydown"];
//            args.splice(0,0,[].slice.call(arguments));
//            childScene.emit.apply(args);
            childScene.emit("keydown", code, flags);
        }
    });
    scene.on("keyup", function(code, flags) {
        console.log("keyup:", code, ", ", flags);
        // eat the ones we handle here
        if (code == 89 && (flags | 48));
        else if (code == 82 && (flags | 16));
        else
            childScene.emit("keyup", code, flags);
        
    });

    scene.on("onchar", function(c) {
        console.log("onchar:", String.fromCharCode(c));
    });

    scene.on("mousedown", function(x, y) { childScene.emit("mousedown", x, y); });    
    scene.on("mouseup", function(x, y) { childScene.emit("mouseup", x, y); console.log("here");});    
    scene.on("mousemove", function(x, y) { childScene.emit("mousemove", x, y); });
    scene.on("mouseenter", function() { childScene.emit("mouseenter"); });
    scene.on("mouseleave", function() { childScene.emit("mouseleave"); });

    scene.on("resize", updateSize);
    updateSize(scene.w, scene.h);
}
else
    console.log("Usage: ./load.sh <js file>");
