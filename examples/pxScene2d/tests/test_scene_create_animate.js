"use strict";

px.import("px:scene.1.js").then( function ready(scene) {

  var root = scene.root;

  var scenes = {};

  console.log("Creating scene1");
  scenes["scene1"] = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_rect_lines.js"});
  scenes["scene1"].ready.then(function(s1) {
      console.log("Promise for scene1");
      
      console.log("Creating scene2");
      scenes["scene2"] = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_xre2-41.js"});
      scenes["scene2"].ready.then(function(s2) {
          console.log("Promise for scene2"); 
          console.log("Starting animation to fade scene1");
          scenes["scene1"].animateTo({a:0},1.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then( function(o) {
          console.log("Setting scene1 = null");
          scenes["scene1"] = null;
            console.log("scene1.remove()");
            o.remove();
          });

          
          console.log("Creating scene3");
          scenes["scene3"] = scene.create({t:"scene", parent:root, url:"https://px-apps.sys.comcast.net/pxscene-samples/examples/px-reference/tests/test_xre2-47.js"});
          scenes["scene3"].ready.then(function(s3) {
              console.log("Promise for scene3");
              console.log("Starting animation to fade scene2");
              scenes["scene2"].animateTo({a:0},1.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then( function(o) {
                console.log("scene2.remove()");
                o.remove();
              });
              console.log("Setting scene2 = null");
              scenes["scene2"] = null;

              console.log("Starting animation to fade scene3");
              scenes["scene3"].animateTo({a:0},1.0,scene.animation.TWEEN_LINEAR,scene.animation.OPTION_LOOP,1).then( function(o) {
                console.log("scene3.remove()");
                o.remove();
              });
              console.log("Setting scene3 = null");
              scenes["scene3"] = null;
            });
        });
      });
  

 
  }).catch( function importFailed(err){
  console.error("Import for test_scene_create.js failed: " + err)
});
