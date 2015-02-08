var px = require("./build/Debug/px");
var http = require('http');
var util = require('util');

function Api(scene) {
  this._scene = scene;
}

Api.prototype.destroyNodeFromScene = function(scene, node) {
  if (node) {
    if (node.children) {
      var n = node.children.length;
      for (var i = 0; i < n; ++i) {
        this.destroyNodeFromScene(scene, node.getChild(0));
      }
    }
    if (node.remove) {
      node.remove();
    }
  }
}

Api.prototype.destroyScene = function(scene) {
  if (scene) {
    this.destroyNodeFromScene(scene, scene.root);
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

  if (uri == null) {
    this.destroyScene(scene);
  }

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

        var app;
        try {
          app = vm.runInNewContext(code, sandbox);
          // TODO do the old scenes context get released when we reload a scenes url??
            
            scene.ctx = app;

            // TODO part of an experiment to eliminate intermediate rendering of the scene
            // while it is being set up
            container.painting = true;
            container.a = 0;
            container.animateTo({a:1}, 0.3, 0, 0);
        }
        catch (err) {
          // console.log('dumping context');
          // sanbox was turned into a context via vm.runInNewContext(...);
          // console.log(util.inspect(sandbox.scene));

          apiForChild.destroyScene(sandbox.scene);

          console.log("failed to run app:" + uri);
          console.log(err);

          // TODO: scene.onError(err); ???
          // TODO: at this point we need to destroy the child scene
          scene.url = "";  // This destroys the child scene and releases scene.ctx
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
    var childScene = scene.createScene();
    var originalURL = argv[2];
    childScene.url = originalURL;
    childScene.parent = scene.root;
    

    function updateSize(w, h) {
        childScene.w = w;
        childScene.h = h;
    }

    scene.on("keydown", function(code, flags) {
        console.log("keydown:", code, ", ", flags);
        if (code == 48)      //'0'
            childScene.url = "gallery.js";
        else if (code == 57) //'9'
            childScene.url = "fancy.js";
        else if (code == 49) {  //'1'
            console.log("Reloading url: ", originalURL);
            childScene.url = originalURL;
        }
    });
    scene.on("resize", updateSize);
    updateSize(scene.w, scene.h);
}
else
    console.log("Usage: ./load.sh <js file>");
